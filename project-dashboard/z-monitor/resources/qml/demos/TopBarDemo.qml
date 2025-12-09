pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

ApplicationWindow {
    id: app
    width: 1280
    height: 80
    minimumWidth: 800
    minimumHeight: 64
    visible: true
    title: "TopBar Demo"

    TopBar {
        id: topBar
        anchors.fill: parent

        // Mock data
        patientName: "JOHN DOE"
        bedLocation: "ICU-A 101"
        isConnected: true
        batteryLevel: 85
        isCharging: false
        hasActiveAlarms: false
        showNotifications: true

        // Update time every second (for visual confirmation the demo is running)
        Timer {
            interval: 1000
            running: true
            repeat: true
            onTriggered: {
                // Toggle battery level for demo purposes
                topBar.batteryLevel = (topBar.batteryLevel + 1) % 100;
            }
        }

        // Signal handlers
        onPatientBannerClicked: {
            console.log("Patient banner clicked");
        }

        onNotificationClicked: {
            console.log("Notification clicked");
        }
    }
}
