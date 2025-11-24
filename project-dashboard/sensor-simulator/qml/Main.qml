import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
Window {
    id: window
    width: 1000
    height: 700
    visible: true
    title: "Sensor Simulator"
    color: "#09090b"

    // Intercept window close and ask the user to confirm.
    onClosing: function(event) {
        event.accepted = false
        confirmExitDialog.open()
    }

    // Theme tokens (mapped from qt-style-telemetry-simulator)
    property color bg: "#0b0f12"
    property color cardBg: "#0f1720"
    property color cardBorder: "#27272a"
    property color slate900: "#0f1720"
    property color slate800: "#111827"
    property color slate700: "#1f2937"
    property color textPrimary: "#e6eef0"
    property color textMuted: "#9ca3af"
    property color emerald: "#10b981"
    property color indigo: "#6366f1"
    property color redAccent: "#ef4444"
    property color orangeAccent: "#f59e0b"
    property color skyAccent: "#38bdf8"

    property string filterLevel: "All"
    property int logRetention: 50
    property int messageCounter: 0

    // Live vitals populated from the Simulator C++ backend
    property int hr: 72
    property int spo2: 98
    property int rr: 16

    function pushLog(level, text, source) {
        var m = logView.model
        window.messageCounter += 1
        var id = window.messageCounter
        var now = new Date()
        var timeStr = now.toLocaleTimeString()
        m.append({ "id": id, "time": timeStr, "sensor": source ? source : "-", "message": text, "level": level })
        while (m.count > window.logRetention) m.remove(0)
        logView.positionViewAtEnd()
    }

    // Header + layout
    Rectangle {
        anchors.fill: parent
        color: bg

        // Top bar
        Rectangle {
            id: topBar
            height: 56
            anchors.left: parent.left
            anchors.right: parent.right
            color: Qt.rgba(0,0,0,0.25)
            border.color: cardBorder
            // subtle blur/glass can be simulated with opacity
            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 12
                
                // Logo & Title
                Row {
                    spacing: 12
                    Rectangle { width: 40; height: 40; radius: 10; color: emerald
                        // Icon placeholder (Activity)
                        Text { anchors.centerIn: parent; text: "⚡"; color: "white"; font.pixelSize: 20 }
                    }
                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        Text { text: "Qt-Style Telemetry Simulator"; color: textPrimary; font.pixelSize: 18; font.bold: true }
                        Text { text: "SIM-ID: 9000-X • PORT: 9002 • CONNECTED"; color: "#9ca3af"; font.pixelSize: 10; font.family: "monospace" }
                    }
                }

                Item { Layout.fillWidth: true }

                // Last Event Display
                Rectangle {
                    Layout.preferredWidth: 300
                    Layout.preferredHeight: 36
                    color: slate900
                    radius: 8
                    border.color: cardBorder
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        Text { text: "LAST EVENT"; color: textMuted; font.pixelSize: 10; font.bold: true }
                        Item { Layout.fillWidth: true }
                        Text { 
                            id: lastEvent
                            text: "-- No events --"
                            color: textMuted
                            font.pixelSize: 12
                            font.family: "monospace"
                            font.bold: true
                        }
                    }
                }
            }
        }

            RowLayout {
            anchors.top: topBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            spacing: 12
            anchors.margins: 12

            // Left column (controls)
            ColumnLayout {
                Layout.preferredWidth: 320
                Layout.maximumWidth: 320
                Layout.minimumWidth: 320
                Layout.fillWidth: false
                spacing: 12

                // Control panel (mirrors ControlPanel.tsx)
                Rectangle {
                    color: cardBg
                    radius: 12
                    border.color: cardBorder
                    Layout.fillWidth: true
                    Layout.preferredHeight: col1.implicitHeight + 24
                    ColumnLayout { 
                        id: col1
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 12
                        Text { text: "Scenarios"; color: textMuted; font.pixelSize: 12; font.bold: true }
                        Button {
                            text: "Play Demo"
                            enabled: true
                            font.bold: true
                            onClicked: simulator.playDemo()
                            background: Rectangle { color: indigo; radius: 10 }
                            Layout.preferredHeight: 44
                            Layout.fillWidth: true
                        }
                        Text { text: "Sequence: Critical → Notify → Warning"; font.pixelSize: 11; color: textMuted; horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true }
                    }
                }

                // Manual triggers (styled buttons)
                Rectangle {
                    color: cardBg
                    radius: 12
                    border.color: cardBorder
                    Layout.fillWidth: true
                    Layout.preferredHeight: col2.implicitHeight + 24
                    ColumnLayout { 
                        id: col2
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 8
                        Text { text: "Manual Events"; color: textMuted; font.pixelSize: 12; font.bold: true }

                        ColumnLayout { 
                            Layout.fillWidth: true
                            spacing: 8
                            // Critical
                            Button {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 50
                                text: "Trigger Critical"
                                onClicked: simulator.triggerCritical()
                                background: Rectangle { color: Qt.rgba(1,0.1,0.1,0.07); border.color: Qt.rgba(0.6,0,0,0.15); radius: 10 }
                                contentItem: Row { spacing: 8; anchors.centerIn: parent
                                    Rectangle { width: 32; height: 32; radius: 8; color: redAccent }
                                    Column { spacing: 0; Text { text: "Trigger Critical"; color: textPrimary; font.bold: true } Text { text: "Code Blue Alarm"; color: textMuted; font.pixelSize: 11 } }
                                }
                            }

                            // Warning
                            Button {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 50
                                text: "Trigger Warning"
                                onClicked: simulator.triggerWarning()
                                background: Rectangle { color: Qt.rgba(1,0.6,0.1,0.07); border.color: Qt.rgba(0.6,0.4,0,0.15); radius: 10 }
                                contentItem: Row { spacing: 8; anchors.centerIn: parent
                                    Rectangle { width: 32; height: 32; radius: 8; color: orangeAccent }
                                    Column { spacing: 0; Text { text: "Trigger Warning"; color: textPrimary; font.bold: true } Text { text: "Sensor Check"; color: textMuted; font.pixelSize: 11 } }
                                }
                            }

                            // Notify
                            Button {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 50
                                text: "Notify"
                                onClicked: simulator.triggerNotification("Manual notification")
                                background: Rectangle { color: Qt.rgba(0.2,0.6,1,0.07); border.color: Qt.rgba(0,0.4,0.6,0.15); radius: 10 }
                                contentItem: Row { spacing: 8; anchors.centerIn: parent
                                    Rectangle { width: 32; height: 32; radius: 8; color: skyAccent }
                                    Column { spacing: 0; Text { text: "Notify"; color: textPrimary; font.bold: true } Text { text: "General Alert"; color: textMuted; font.pixelSize: 11 } }
                                }
                            }
                        }
                    }
                }

                // System Info
                Rectangle {
                    color: cardBg
                    radius: 12
                    border.color: cardBorder
                    Layout.fillWidth: true
                    Layout.preferredHeight: col3.implicitHeight + 24
                    ColumnLayout { 
                        id: col3
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 8
                        
                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: "DEVICE STATUS"; color: textMuted; font.pixelSize: 11 }
                            Item { Layout.fillWidth: true }
                            Row { 
                                spacing: 6
                                Rectangle { width: 8; height: 8; radius: 4; color: emerald; anchors.verticalCenter: parent.verticalCenter }
                                Text { text: "ONLINE"; color: textPrimary; font.pixelSize: 11; anchors.verticalCenter: parent.verticalCenter }
                            }
                        }

                        Rectangle {
                            color: bg
                            radius: 8
                            border.color: cardBorder
                            Layout.fillWidth: true
                            Layout.preferredHeight: 64
                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 4
                                Text { text: "Proto: v2.1 (JSON)"; color: textMuted; font.pixelSize: 11 }
                                Text { text: "Rate: 5 Hz"; color: textMuted; font.pixelSize: 11 }
                                Text { text: "Mode: Interactive"; color: indigo; font.pixelSize: 11; font.bold: true }
                            }
                        }
                    }
                }
                
                // Spacer to push content up
                Item { Layout.fillHeight: true }
            }

            // Right Column (Visualization & Logs)
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 16

                // Vitals row (three cards)
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16
                    Rectangle {
                        color: slate900
                        radius: 12
                        border.color: cardBorder
                        Layout.fillWidth: true
                        Layout.minimumWidth: 140
                        Layout.preferredHeight: 120
                        Column { anchors.fill: parent; anchors.margins: 16; spacing: 8
                            Row { spacing: 8; Text { text: "HEART RATE"; color: textMuted; font.pixelSize: 11; font.bold: true } Item { Layout.fillWidth: true } }
                            Row { spacing: 8; Text { text: window.hr; color: emerald; font.pixelSize: 42; font.bold: true; font.family: "monospace" } Text { text: "BPM"; color: textMuted; verticalAlignment: Text.AlignBottom; bottomPadding: 6 } }
                        }
                    }
                    Rectangle {
                        color: slate900
                        radius: 12
                        border.color: cardBorder
                        Layout.fillWidth: true
                        Layout.minimumWidth: 140
                        Layout.preferredHeight: 120
                        Column { anchors.fill: parent; anchors.margins: 16; spacing: 8
                            Row { spacing: 8; Text { text: "SPO2"; color: textMuted; font.pixelSize: 11; font.bold: true } Item { Layout.fillWidth: true } }
                            Row { spacing: 8; Text { text: window.spo2; color: skyAccent; font.pixelSize: 42; font.bold: true; font.family: "monospace" } Text { text: "%"; color: textMuted; verticalAlignment: Text.AlignBottom; bottomPadding: 6 } }
                        }
                    }
                    Rectangle {
                        color: slate900
                        radius: 12
                        border.color: cardBorder
                        Layout.fillWidth: true
                        Layout.minimumWidth: 140
                        Layout.preferredHeight: 120
                        Column { anchors.fill: parent; anchors.margins: 16; spacing: 8
                            Row { spacing: 8; Text { text: "RESP RATE"; color: textMuted; font.pixelSize: 11; font.bold: true } Item { Layout.fillWidth: true } }
                            Row { spacing: 8; Text { text: window.rr; color: orangeAccent; font.pixelSize: 42; font.bold: true; font.family: "monospace" } Text { text: "RPM"; color: textMuted; verticalAlignment: Text.AlignBottom; bottomPadding: 6 } }
                        }
                    }
                }

                // Waveform Chart
                Rectangle {
                    color: slate900
                    radius: 12
                    border.color: cardBorder
                    Layout.fillWidth: true
                    Layout.preferredHeight: 180
                    
                    // Grid background pattern (simulated with repeated lines or just a subtle grid)
                    Column {
                        anchors.fill: parent
                        Repeater {
                            model: 9
                            Rectangle { width: parent.width; height: 1; color: Qt.rgba(1,1,1,0.03); y: index * 20 }
                        }
                    }
                    Row {
                        anchors.fill: parent
                        Repeater {
                            model: 40
                            Rectangle { width: 1; height: parent.height; color: Qt.rgba(1,1,1,0.03); x: index * 20 }
                        }
                    }

                    Text { anchors.centerIn: parent; text: "Waveform Visualization (Lead II)"; color: emerald; font.family: "monospace" }
                    
                    // Header
                    Row {
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 12
                        Text { text: "LEAD II"; color: emerald; font.pixelSize: 10; font.bold: true }
                        Item { width: 10 }
                        Text { text: "250 Hz / 25 mm/s"; color: textMuted; font.pixelSize: 10; font.family: "monospace" }
                    }
                }

                // Log Console
                Rectangle {
                    id: logConsole
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: slate900
                    radius: 12
                    border.color: cardBorder
                    clip: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0

                        // Header
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 48
                            color: Qt.rgba(0,0,0,0.2)
                            border.width: 0 // Bottom border handled by separator
                            
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 12
                                Text { text: "Telemetry Stream"; color: textPrimary; font.bold: true }
                                Rectangle {
                                    width: 60; height: 20; radius: 10; color: slate800
                                    Text { anchors.centerIn: parent; text: logView.count + " events"; color: textMuted; font.pixelSize: 10 }
                                }
                                Item { Layout.fillWidth: true }
                                
                                // Filter
                                Text { text: "Filter:"; color: textMuted; font.pixelSize: 12 }
                                ComboBox {
                                    id: filterCombo
                                    Layout.preferredWidth: 120
                                    Layout.preferredHeight: 30
                                    model: ["All","Critical","Warning","Info","Debug"]
                                    onCurrentTextChanged: window.filterLevel = currentText
                                }
                                
                                Button {
                                    text: "Clear"
                                    Layout.preferredHeight: 30
                                    onClicked: { logView.model.clear(); window.messageCounter = 0; }
                                    background: Rectangle { color: "transparent"; border.color: cardBorder; radius: 6 }
                                    contentItem: Text { text: "Clear"; color: textMuted; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                }
                            }
                            
                            Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: cardBorder }
                        }

                        // Log List
                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            
                            ScrollView {
                                anchors.fill: parent
                                anchors.margins: 0
                                ScrollBar.vertical: ScrollBar { }
                                ListView {
                                    id: logView
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    model: ListModel {}
                                    spacing: 4
                                    clip: true
                                    
                                    delegate: Rectangle {
                                        width: logView.width
                                        height: 32
                                        color: "transparent"
                                        visible: window.filterLevel === "All" || window.filterLevel === (typeof(level) !== 'undefined' ? level : "Info")
                                        
                                        RowLayout {
                                            anchors.fill: parent
                                            spacing: 12
                                            
                                            // Time
                                            Text { text: time; color: textMuted; font.pixelSize: 11; font.family: "monospace"; Layout.preferredWidth: 80 }
                                            
                                            // Level Badge
                                            Rectangle {
                                                Layout.preferredWidth: 70
                                                Layout.preferredHeight: 20
                                                radius: 4
                                                color: level === "Critical" ? Qt.rgba(0.93, 0.26, 0.26, 0.1) : 
                                                       level === "Warning" ? Qt.rgba(0.96, 0.62, 0.04, 0.1) : 
                                                       level === "Info" ? Qt.rgba(0.06, 0.72, 0.5, 0.1) : Qt.rgba(0.42, 0.45, 0.5, 0.1)
                                                border.color: level === "Critical" ? Qt.rgba(0.93, 0.26, 0.26, 0.3) : 
                                                              level === "Warning" ? Qt.rgba(0.96, 0.62, 0.04, 0.3) : 
                                                              level === "Info" ? Qt.rgba(0.06, 0.72, 0.5, 0.3) : Qt.rgba(0.42, 0.45, 0.5, 0.3)
                                                
                                                Text { 
                                                    anchors.centerIn: parent
                                                    text: level
                                                    color: level === "Critical" ? redAccent : level === "Warning" ? orangeAccent : level === "Info" ? emerald : textMuted
                                                    font.pixelSize: 10
                                                    font.bold: true
                                                }
                                            }
                                            
                                            // Message
                                            Text {
                                                text: message
                                                color: textPrimary
                                                font.pixelSize: 12
                                                font.family: "monospace"
                                                Layout.fillWidth: true
                                                elide: Text.ElideRight
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

    Connections {
        target: simulator
        function onAlarmTriggered(level) {
            lastEvent.text = "ALARM(" + level + ")"
            pushLog(level, "ALARM: " + level)
        }
        function onNotification(text) {
            lastEvent.text = "NOTIFY(" + text + ")"
            pushLog("Info", "NOTIFY: " + text)
        }
        function onVitalsUpdated(newHr, newSpo2, newRr) {
            // Update window-scoped vitals properties so UI cards refresh
            window.hr = newHr
            window.spo2 = newSpo2
            window.rr = newRr
        }
        function onLogEmitted(level, text) {
            pushLog(level, text)
        }
    }

    Dialog {
        id: confirmExitDialog
        title: "Exit Simulator"
        modal: true
        contentItem: Column {
            spacing: 8
            Text { text: "Are you sure you want to exit the simulator?" }
            Row { spacing: 12
                Button {
                    id: yesButton
                    text: "Yes"
                    onClicked: {
                        yesButton.enabled = false
                        confirmExitDialog.close()
                        simulator.quitApp()
                    }
                }
                Button { text: "No"; onClicked: confirmExitDialog.close() }
            }
        }
    }
}

}
