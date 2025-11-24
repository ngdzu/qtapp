import QtQuick 2.15
import QtQuick.Layouts 1.12
import qml 1.0

RowLayout {
    id: root
    
    property int heartRate: 72
    property int spo2: 98
    property int respRate: 16
    
    spacing: Theme.spacingLg
    
    // Heart Rate Card
    VitalCard {
        Layout.fillWidth: true
        Layout.minimumWidth: Theme.vitalCardMinWidth
        Layout.preferredHeight: Theme.vitalCardHeight
        label: "HEART RATE"
        value: root.heartRate
        unit: "BPM"
        accentColor: Theme.accentEmerald
        icon: "⚡"
    }
    
    // SpO2 Card
    VitalCard {
        Layout.fillWidth: true
        Layout.minimumWidth: Theme.vitalCardMinWidth
        Layout.preferredHeight: Theme.vitalCardHeight
        label: "SPO2"
        value: root.spo2
        unit: "%"
        accentColor: Theme.accentSky
        icon: "💧"
    }
    
    // Resp Rate Card
    VitalCard {
        Layout.fillWidth: true
        Layout.minimumWidth: Theme.vitalCardMinWidth
        Layout.preferredHeight: Theme.vitalCardHeight
        label: "RESP RATE"
        value: root.respRate
        unit: "RPM"
        accentColor: Theme.accentAmber
        icon: "🌬"
    }
}

