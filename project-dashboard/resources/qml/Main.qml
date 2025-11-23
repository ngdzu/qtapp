import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "components"

Window {
    width: 1024
    height: 768
    visible: true
    title: "Medical Device Dashboard"
    color: "#09090b" // Zinc-950

    GridLayout {
        anchors.fill: parent
        anchors.margins: 24
        columns: width < 800 ? 2 : 4
        rowSpacing: 16
        columnSpacing: 16
        
        // Card 1: Heart Rate
        StatCard {
            title: "Heart Rate"
            value: dashboard.heartRate + " BPM"
            accentColor: "#10b981" // Emerald
            
            visualization: Component {
                SparkLine {
                    dataPoints: dashboard.heartRateHistory
                    lineColor: "#10b981"
                }
            }
        }
        
        // Card 2: SpO2
        StatCard {
            title: "SpO2 Level"
            value: dashboard.oxygenLevel + "%"
            accentColor: "#3b82f6" // Blue
            
            visualization: Component {
                SparkLine {
                    dataPoints: dashboard.oxygenHistory
                    lineColor: "#3b82f6"
                }
            }
        }
        
        // Card 3: Battery
        StatCard {
            title: "Battery Level"
            value: dashboard.batteryLevel + "%"
            accentColor: "#a855f7" // Purple
            
            visualization: Component {
                Item {
                    // Custom Progress Bar
                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width
                        height: 8
                        radius: 4
                        color: "#27272a" // Zinc-800
                        
                        Rectangle {
                            height: parent.height
                            width: parent.width * (dashboard.batteryLevel / 100)
                            radius: 4
                            color: "#a855f7"
                            
                            Behavior on width { NumberAnimation { duration: 500 } }
                        }
                    }
                }
            }
        }
        
        // Card 4: Status
        StatCard {
            title: "Device Status"
            value: dashboard.temperature + "°C"
            accentColor: "#f59e0b" // Amber
            
            visualization: Component {
                ColumnLayout {
                    spacing: 4
                    Text {
                        text: dashboard.isConnected ? "Connected" : "Disconnected"
                        color: dashboard.isConnected ? "#10b981" : "#ef4444"
                        font.pixelSize: 12
                    }
                    Text {
                        text: "Stable Connection"
                        color: "#71717a" // Zinc-500
                        font.pixelSize: 10
                    }
                }
            }
        }
        
        // Spacer to push content up
        Item {
            Layout.columnSpan: parent.columns
            Layout.fillHeight: true
        }
    }
}
