/**
 * @file WaveformDisplayDemo.qml
 * @brief Standalone demo of WaveformDisplay component
 * 
 * Shows all three waveforms (ECG, Pleth, Resp) with simulated data
 * to demonstrate zoom, pan, freeze, and signal quality features.
 * 
 * Usage:
 *   /Users/dustinwind/Qt/6.9.2/macos/bin/qml /Users/dustinwind/Development/Qt/qtapp/project-dashboard/z-monitor/resources/qml/WaveformDisplayDemo.qml 2>&1
 * 
 * Or from project root:
 *   /Users/dustinwind/Qt/6.9.2/macos/bin/qml resources/qml/WaveformDisplayDemo.qml
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "./components"

ApplicationWindow {
    visible: true
    width: 1200
    height: 800
    title: "WaveformDisplay Demo - ECG, Pleth, Resp"
    color: "#09090b"

    // Simulate waveform data
    Timer {
        interval: 16  // ~60 FPS
        running: true
        repeat: true
        property int sampleCount: 0
        
        onTriggered: {
            // Generate ECG data (60 BPM)
            var ecgData = [];
            for (var i = 0; i < 250; i++) {  // 1 second window at 250 Hz
                var t = (sampleCount + i) / 250.0;
                var beatPhase = (t * 1.0) % 1.0; // 60 BPM
                
                var value = 0;
                if (beatPhase < 0.1) {
                    value = Math.sin(beatPhase * 10 * Math.PI) * 0.2;  // P wave
                } else if (beatPhase >= 0.15 && beatPhase < 0.25) {
                    if (beatPhase < 0.17) value = -0.3;  // Q
                    else if (beatPhase < 0.20) value = 1.0;  // R
                    else value = -0.2;  // S
                } else if (beatPhase >= 0.35 && beatPhase < 0.55) {
                    value = Math.sin((beatPhase - 0.35) * 5 * Math.PI) * 0.3;  // T wave
                }
                
                ecgData.push({time: t, value: value});
            }
            ecgDisplay.waveformData = ecgData;
            
            // Generate Pleth data (60 BPM sine wave)
            var plethData = [];
            for (var j = 0; j < 250; j++) {
                var t2 = (sampleCount + j) / 250.0;
                var value2 = Math.sin(t2 * 2 * Math.PI * 1.0) * 0.8;  // 60 BPM
                plethData.push({time: t2, value: value2});
            }
            plethDisplay.waveformData = plethData;
            
            // Generate Resp data (12 breaths/min = 0.2 Hz)
            var respData = [];
            for (var k = 0; k < 250; k++) {
                var t3 = (sampleCount + k) / 250.0;
                var value3 = Math.sin(t3 * 2 * Math.PI * 0.2) * 0.6;  // 12 breaths/min
                respData.push({time: t3, value: value3});
            }
            respDisplay.waveformData = respData;
            
            sampleCount += 10;  // Advance by 10 samples (~40ms)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // Title
        Text {
            Layout.fillWidth: true
            text: "WaveformDisplay Component Demo"
            font.pixelSize: 24
            font.bold: true
            color: "#ffffff"
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            Layout.fillWidth: true
            text: "Features: 60 FPS rendering, Touch gestures (pinch zoom, tap to freeze, drag to pan, double-tap reset), Mouse wheel zoom"
            font.pixelSize: 14
            color: "#71717a"
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        // ECG Panel
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#18181b"
            border.color: "#27272a"
            border.width: 1
            radius: 4

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: "II ECG"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#10b981"
                    }
                    Item { Layout.fillWidth: true }
                    Button {
                        text: ecgDisplay.frozen ? "FROZEN ❄️" : "RUNNING ▶"
                        onClicked: ecgDisplay.toggleFreeze()
                    }
                    Button {
                        text: "Reset View"
                        onClicked: ecgDisplay.resetView()
                    }
                }

                WaveformDisplay {
                    id: ecgDisplay
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    label: "ECG II"
                    waveformColor: "#10b981"
                    gridColor: "#27272a40"
                    showQuality: true
                    signalQuality: 0.95
                }
            }
        }

        // Pleth Panel
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#18181b"
            border.color: "#27272a"
            border.width: 1
            radius: 4

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: "PLETH"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#3b82f6"
                    }
                    Item { Layout.fillWidth: true }
                    Button {
                        text: plethDisplay.frozen ? "FROZEN ❄️" : "RUNNING ▶"
                        onClicked: plethDisplay.toggleFreeze()
                    }
                    Button {
                        text: "Reset View"
                        onClicked: plethDisplay.resetView()
                    }
                }

                WaveformDisplay {
                    id: plethDisplay
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    label: "PLETH"
                    waveformColor: "#3b82f6"
                    gridColor: "#27272a40"
                    showQuality: true
                    signalQuality: 0.75
                }
            }
        }

        // Resp Panel
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#18181b"
            border.color: "#27272a"
            border.width: 1
            radius: 4

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: "RESP"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#eab308"
                    }
                    Item { Layout.fillWidth: true }
                    Button {
                        text: respDisplay.frozen ? "FROZEN ❄️" : "RUNNING ▶"
                        onClicked: respDisplay.toggleFreeze()
                    }
                    Button {
                        text: "Reset View"
                        onClicked: respDisplay.resetView()
                    }
                }

                WaveformDisplay {
                    id: respDisplay
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    label: "RESP"
                    waveformColor: "#eab308"
                    gridColor: "#27272a40"
                    showQuality: true
                    signalQuality: 0.85
                }
            }
        }

        // Instructions
        Text {
            Layout.fillWidth: true
            text: "💡 Touch: Pinch to zoom • Tap to freeze • Drag to pan • Double-tap to reset | Mouse: Wheel to zoom • Drag to pan • Double-click to reset"
            font.pixelSize: 12
            color: "#52525b"
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }
}
