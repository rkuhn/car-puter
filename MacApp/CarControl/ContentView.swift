//
//  ContentView.swift
//  CarControl
//
//  Created by Roland Kuhn on 30.05.24.
//

import SwiftUI

struct ContentView: View {
    @State private var throttle: Double = 1.0
    @ObservedObject var measurements: Measurements
    var controller: BleController
    
    var body: some View {
        VStack {
            GroupBox(label: Text("Connection")) {
                Grid(alignment: .leading) {
                    GridRow {
                        Text("RSSI:")
                        Text("\($measurements.rssi.wrappedValue) dB")
                            .frame(width: 200, alignment: .leading)
                    }
                }
            }
            GroupBox(label: Text("Measurements:")) {
                Grid(alignment: .leading) {
                    GridRow {
                        Text("Mode:")
                        Text("\($measurements.mode.wrappedValue)")
                            .frame(width: 200, alignment: .leading)
                    }
                    GridRow {
                        Text("Throttle:")
                        Text("\($measurements.throttle.wrappedValue)")
                    }
                    GridRow {
                        Text("Temperature:")
                        Text(String(format: "%05.2f°C", $measurements.temperature.wrappedValue))
                    }
                    GridRow {
                        Text("Humidity:")
                        Text(String(format: "%05.2f%", $measurements.humidity.wrappedValue))
                    }
                }
            }.padding([.bottom, .top], 50)

            Button(/*@START_MENU_TOKEN@*/"ON"/*@END_MENU_TOKEN@*/) {
                controller.setMode(.on)
            }
                .buttonStyle(.borderedProminent)
                .accentColor(.green)
            
            HStack {
                Text("").frame(width: 50)
                
                Slider(value: $throttle, in: 1...20, step: 1) {
                    Text("Throttle")
                } onEditingChanged: { editing in
                    if (!editing) {
                        controller.setMode(.throttle(UInt8($throttle.wrappedValue)))
                    }
                }
                .frame(width: 200.0)
                
                Text(String(format: "%.0f", $throttle.wrappedValue))
                    .frame(width: 50, alignment: .leading)
            }.padding()

            Button(/*@START_MENU_TOKEN@*/"OFF"/*@END_MENU_TOKEN@*/) {
                controller.setMode(.off)
            }
                .buttonStyle(.borderedProminent)
                .accentColor(.red)

            Divider()
                .frame(width: 100.0)
                .padding()
            
            Button(/*@START_MENU_TOKEN@*/"AUTO"/*@END_MENU_TOKEN@*/) {
                controller.setMode(.auto)
            }
                .buttonStyle(.borderedProminent)
                .accentColor(.purple)
                    }
        .padding()
    }
}

#Preview {
    let m = Measurements.init()
    return ContentView(measurements: m, controller: BleController.init(measurements: m))
}
