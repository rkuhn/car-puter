//
//  DataModel.swift
//  CarControl
//
//  Created by Roland Kuhn on 30.05.24.
//

import CoreBluetooth

class Measurements: ObservableObject {
    @Published var rssi: Int = -255
    
    @Published var mode: UInt8 = 255
    @Published var throttle: UInt8 = 0
    
    @Published var temperature: Float = -Float.infinity {
        didSet {
            temperatureTimestamp = Date.now
        }
    }
    @Published var temperatureTimestamp = Date.now
    
    @Published var humidity: Float = -Float.infinity {
        didSet {
            humidityTimestamp = Date.now
        }
    }
    @Published var humidityTimestamp = Date.now
}

enum ControllerMode {
    case off
    case on
    case auto
    case autoOn
    case throttle((UInt8))
    case invalid((UInt8))
    
    static public func fromByte(_ byte: UInt8, withThrottle throttle: UInt8 ) -> ControllerMode {
        switch byte {
        case 0:
            return .off
        case 1:
            return .on
        case 2:
            return .auto
        case 3:
            return .autoOn
        case 4:
            return .throttle(throttle)
        default:
            return .invalid(byte)
        }
    }
}
