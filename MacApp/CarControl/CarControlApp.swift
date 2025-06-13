//
//  CarControlApp.swift
//  CarControl
//
//  Created by Roland Kuhn on 30.05.24.
//

import SwiftUI

@main
struct CarControlApp: App {
    @StateObject private var measurements: Measurements
    let bleController: BleController
    
    init() {
        print("starting")
        let measurements = Measurements.init()
        bleController = BleController.init(measurements: measurements)
        self._measurements = StateObject(wrappedValue: measurements)
        print("started")
    }

    var body: some Scene {
        WindowGroup {
            ContentView(measurements: self.measurements, controller: self.bleController)
        }
    }
}
