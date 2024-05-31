//
//  DataModel.swift
//  CarControl
//
//  Created by Roland Kuhn on 30.05.24.
//

import Foundation
import CoreBluetooth

class Measurements: ObservableObject {
    @Published var rssi: Int = -255
    
    @Published var mode: UInt8 = 0
    @Published var throttle: UInt8 = 0
    @Published var temperature: Float = 0
    @Published var humidity: Float = 0
}

enum ControllerMode {
    case off
    case on
    case auto
    case throttle((UInt8))
}
