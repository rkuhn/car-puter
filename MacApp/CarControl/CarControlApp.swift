//
//  CarControlApp.swift
//  CarControl
//
//  Created by Roland Kuhn on 30.05.24.
//

import SwiftUI

@main
class CarControlApp: App {
    var measurements: Measurements
    var bleController: BleController
    
    var body: some Scene {
        WindowGroup {
            ContentView(measurements: measurements, controller: bleController)
        }
    }
    
    required public init() {
        print("starting")
        measurements = Measurements.init()
        bleController = BleController.init(measurements: measurements)
        print("started")
    }
}
