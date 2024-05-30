//
//  DataModel.swift
//  CarControl
//
//  Created by Roland Kuhn on 30.05.24.
//

import Foundation

class Measurements: ObservableObject {
    @Published var mode: UInt8 = 0
    @Published var throttle: UInt8 = 0
    @Published var temperature: Float = 0
    @Published var humidity: Float = 0
}

enum ControllerMode {
    case off
    case on
    case auto
    case throttle(UInt8)
}

class Controller: ObservableObject {
    @Published var mode: ControllerMode = .off
}
