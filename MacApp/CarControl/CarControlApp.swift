//
//  CarControlApp.swift
//  CarControl
//
//  Created by Roland Kuhn on 30.05.24.
//

import SwiftUI

@main
struct CarControlApp: App {
    var measurements: Measurements
    var controller: Controller
    
    var body: some Scene {
        WindowGroup {
            ContentView(measurements: measurements, controller: controller)
        }
    }
    
    public init() {
        print("starting")
        measurements = Measurements.init()
        controller = Controller.init()
        print("started")
    }
}
