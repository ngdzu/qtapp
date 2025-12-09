import QtQuick
import QtQuick.Controls
import "project-dashboard/z-monitor/resources/qml/components"
import "project-dashboard/z-monitor/resources/qml/theme"

ApplicationWindow {
    visible: true
    width: 1280
    height: 80
    title: "TopBar Demo"

    TopBar {
        id: topBar
        anchors.fill: parent

        patientName: "JOHN DOE"
        bedLocation: "ICU-A 101"
        currentTime: "14:35:22"
        currentDate: "Mon, Dec 9"
        isConnected: true
        signalQuality: 95
        networkType: "WiFi"
        batteryLevel: 85
        isCharging: false
        hasNotifications: true
        notificationCount: 3
    }
}
