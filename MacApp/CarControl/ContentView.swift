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
    @StateObject var controller: Controller
    
    var body: some View {
        VStack {
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
                        Text(String(format: "%05.2f°C", $measurements.humidity.wrappedValue))
                    }
                }
            }.padding(.bottom, 50)

            Button(/*@START_MENU_TOKEN@*/"ON"/*@END_MENU_TOKEN@*/) {
                controller.mode = .on
            }
                .buttonStyle(.borderedProminent)
                .accentColor(.green)
            
            HStack {
                Text("").frame(width: 50)
                
                Slider(value: $throttle, in: 1...20, step: 1) {
                    Text("Throttle")
                } onEditingChanged: { editing in
                    if (!editing) {
                        controller.mode = .throttle(UInt8($throttle.wrappedValue))
                    }
                }
                .frame(width: 200.0)
                
                Text(String(format: "%.0f", $throttle.wrappedValue))
                    .frame(width: 50, alignment: .leading)
            }

            Button(/*@START_MENU_TOKEN@*/"OFF"/*@END_MENU_TOKEN@*/) {
                controller.mode = .off
            }
                .buttonStyle(.borderedProminent)
                .accentColor(.red)

            Divider()
                .frame(width: 100.0)
                .padding()
            
            Button(/*@START_MENU_TOKEN@*/"AUTO"/*@END_MENU_TOKEN@*/) {
                controller.mode = .auto
            }
                .buttonStyle(.borderedProminent)
                .accentColor(.purple)
                    }
        .padding()
    }
}

#Preview {
    ContentView(measurements: Measurements.init(), controller: Controller.init())
}
