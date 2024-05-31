//
//  BLE.swift
//  CarControl
//
//  Created by Roland Kuhn on 31.05.24.
//

import CoreBluetooth

class BleController : NSObject, CBPeripheralDelegate, CBCentralManagerDelegate {
    private var measurements: Measurements
    private var ble: CBCentralManager!
    private var peripheral: CBPeripheral?
    private var connected = false
    
    private var timer = DispatchSource.makeTimerSource()
    
    let serviceId = CBUUID.init(string: "4625E9D0-EA59-4E6C-A81D-282F46BC25A9")
    let modeId = CBUUID.init(string: "7356A709-0235-4976-91AB-49F8AC825442")
    let temperatureId = CBUUID.init(string: "9E762D6E-4034-4407-B4F3-94579F8658FE")
    let humidityId = CBUUID.init(string: "B56F03B4-D496-48BF-8BFC-A54D19FC1D0D")
    let throttleId = CBUUID.init(string: "9A76D379-96CD-4BC7-979E-15982AF7A1E9")
    
    private var modeService: CBCharacteristic?
    private var temperaturService: CBCharacteristic?
    private var humidityService: CBCharacteristic?
    private var throttleService: CBCharacteristic?

    public init(measurements: Measurements) {
        self.measurements = measurements
        super.init()
        ble = CBCentralManager(delegate: self, queue: nil, options: [CBCentralManagerOptionShowPowerAlertKey : true])
        timer.schedule(deadline: .now(), repeating: .seconds(1))
        timer.setEventHandler {
            if self.connected {
                self.peripheral?.readRSSI()
            }
        }
        timer.activate()
    }
    
    public func setMode(_ mode: ControllerMode) {
        if modeService == nil { return }
        switch mode {
        case .off:
            peripheral?.writeValue(Data([0]), for: modeService!, type: .withResponse)
        case .on:
            peripheral?.writeValue(Data([1]), for: modeService!, type: .withResponse)
        case .auto:
            peripheral?.writeValue(Data([2]), for: modeService!, type: .withResponse)
        case .throttle(let x):
            peripheral?.writeValue(Data([x]), for: throttleService!, type: .withResponse)
        }
    }
    
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        switch central.state {
        case .unknown:
            print("BLE: unknown")
        case .resetting:
            print("BLE: resetting")
        case .unsupported:
            print("BLE: unsupported")
        case .unauthorized:
            print("BLE: unauthorized")
        case .poweredOff:
            print("BLE: off")
        case .poweredOn:
            print("BLE: on")
            ble.scanForPeripherals(withServices: [serviceId], options: [CBCentralManagerScanOptionAllowDuplicatesKey : true])
        @unknown default:
            print("BLE: weird")
        }
    }

    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        if self.peripheral != nil { return }
        
        measurements.rssi = RSSI.intValue
        
        self.connected = false
        self.peripheral = peripheral
        peripheral.delegate = self
        
        ble.stopScan()
        ble.connect(peripheral)
        print("connecting")
    }
    
    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: Error?) {
        if self.peripheral == peripheral {
            connected = false
            self.peripheral = nil
            measurements.rssi = -255
            print("failed to connect (\(error.debugDescription))")
            ble.scanForPeripherals(withServices: [serviceId], options: [CBCentralManagerScanOptionAllowDuplicatesKey : true])
        }
    }
    
    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        if self.peripheral == peripheral {
            connected = false
            self.peripheral = nil
            
            measurements.rssi = -255

            modeService = nil
            temperaturService = nil
            humidityService = nil
            throttleService = nil
            
            print("disconnected (\(error.debugDescription))")
            ble.scanForPeripherals(withServices: [serviceId], options: [CBCentralManagerScanOptionAllowDuplicatesKey : true])
        }
    }
    
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        if self.peripheral == peripheral {
            print("connected")
            connected = true
            peripheral.discoverServices([serviceId])
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didReadRSSI RSSI: NSNumber, error: Error?) {
        if self.peripheral == peripheral {
            print("got RSSI")
            measurements.rssi = RSSI.intValue
        }
    }
    
    func peripheralDidUpdateRSSI(_ peripheral: CBPeripheral, error: Error?) {
        if self.peripheral != peripheral { return }
        print("got RSSI 2")
        measurements.rssi = peripheral.rssi?.intValue ?? -255
    }
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        if self.peripheral != peripheral { return }
        if error != nil {
            print("error discovering services: \(error.debugDescription)")
            return
        }
        if let services = peripheral.services {
            for service in services {
                if service.uuid == serviceId {
                    print("service found")
                    peripheral.discoverCharacteristics([modeId, temperatureId, humidityId, throttleId], for: service)
                    break
                }
            }
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        if self.peripheral != peripheral { return }
        if let characteristics = service.characteristics {
            print("\(service.characteristics?.count ?? -1) characteristics found")
            for characteristic in characteristics {
                switch characteristic.uuid {
                case modeId:
                    modeService = characteristic
                    peripheral.setNotifyValue(true, for: characteristic)
                    peripheral.readValue(for: characteristic)
                case temperatureId:
                    temperaturService = characteristic
                    peripheral.setNotifyValue(true, for: characteristic)
                    peripheral.readValue(for: characteristic)
                case humidityId:
                    humidityService = characteristic
                    peripheral.setNotifyValue(true, for: characteristic)
                    peripheral.readValue(for: characteristic)
                case throttleId:
                    throttleService = characteristic
                    peripheral.setNotifyValue(true, for: characteristic)
                    peripheral.readValue(for: characteristic)
                default:
                    print("found unknown characteristic \(characteristic.uuid)")
                }
            }
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        if self.peripheral != peripheral { return }
        print("got update for \(characteristic.uuid)")
        if characteristic.value == nil {
            print("    contains no data")
            return
        }
        switch characteristic.uuid {
        case modeId:
            measurements.mode = characteristic.value![0]
        case temperatureId:
            measurements.temperature = characteristic.value!.withUnsafeBytes { bytes in
                bytes.load(as: Float.self)
            }
        case humidityId:
            measurements.humidity = characteristic.value!.withUnsafeBytes { bytes in
                bytes.load(as: Float.self)
            }
        case throttleId:
            measurements.throttle = characteristic.value![0]
        default:
            print("notify for unknown characteristic \(characteristic.uuid)")
        }
    }
}
