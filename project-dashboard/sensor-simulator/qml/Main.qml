import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

Window {
    id: window
    width: 640
    height: 480
    visible: true

    // Intercept window close and ask the user to confirm.
    onClosing: function(event) {
        // Prevent the window from closing immediately
        event.accepted = false;
        confirmExitDialog.open();
    }
    title: "Sensor Simulator"

    property string filterLevel: "All"
    property int logRetention: 1000

    function pushLog(level, text) {
        var m = logView.model
        m.append({ "message": text, "level": level })
        if (m.count > window.logRetention) {
            // remove oldest until within retention
            while (m.count > window.logRetention) {
                m.remove(0)
            }
        }
        // keep view pinned to end so new entries are visible
        logView.positionViewAtEnd()
    }

    Column {
        anchors.centerIn: parent
        spacing: 12

        Text { id: header; text: "Sensor Simulator"; font.pixelSize: 24 }

        Row { spacing: 8
            Button { text: "Trigger Critical"; onClicked: simulator.triggerCritical() }
            Button { text: "Trigger Warning"; onClicked: simulator.triggerWarning() }
            Button { text: "Notify"; onClicked: simulator.triggerNotification("Manual notification") }
        }

        Row { spacing: 8
            Button { text: "Play Demo"; onClicked: simulator.playDemo() }
        }

        Row {
            spacing: 8
            Text { text: "Filter:" }
            ComboBox {
                id: filterCombo
                model: ["All", "Critical", "Warning", "Info", "Debug"]
                currentIndex: 0
                onCurrentTextChanged: window.filterLevel = currentText
            }
            Button { text: "Show All"; onClicked: filterCombo.currentIndex = 0 }
        }

        Text { text: "WebSocket server: ws://localhost:9002" }

        Rectangle {
            width: parent.width * 0.9
            height: parent.height * 0.5
            color: "#111"
            radius: 6
            Column {
                anchors.fill: parent
                anchors.margins: 8
                Text { id: lastEvent; text: "Last event: none"; color: "white" }

                ScrollView {
                    id: logScroll
                    width: parent.width
                    height: parent.height - lastEvent.height - 8
                    clip: true
                    ScrollBar.vertical: ScrollBar { }

                    contentItem: ListView {
                        id: logView
                        width: logScroll.width
                        model: ListModel {}
                        delegate: Item {
                            width: logView.width
                            height: textItem.contentHeight + 8
                            property string itemLevel: typeof(level) !== 'undefined' ? level : "Info"
                            visible: window.filterLevel === "All" || window.filterLevel === itemLevel
                            Text {
                                id: textItem
                                text: "[" + itemLevel + "] " + message
                                wrapMode: Text.Wrap
                                width: parent.width
                                color: itemLevel === "Critical" ? "#ff5555" : itemLevel === "Warning" ? "#ffcc00" : itemLevel === "Info" ? "#ffffff" : "#888888"
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
            lastEvent.text = "Last event: ALARM(" + level + ")"
            pushLog(level, "ALARM: " + level)
        }
        function onNotification(text) {
            lastEvent.text = "Last event: NOTIFY(" + text + ")"
            pushLog("Info", "NOTIFY: " + text)
        }
        function onLogEmitted(level, text) {
            // Append any generic logs emitted by the simulator
            pushLog(level, text)
        }
    }

    Dialog {
        id: confirmExitDialog
        title: "Exit Simulator"
        modal: true
        onOpened: { console.log("confirmExitDialog: opened") }
        onClosed: { console.log("confirmExitDialog: closed") }
        contentItem: Column {
            spacing: 8
            Text { text: "Are you sure you want to exit the simulator?" }
            Row {
                spacing: 12
                Button {
                    id: yesButton
                    text: "Yes"
                    onClicked: {
                        console.log("Yes button clicked — disabling and closing dialog")
                        // Close dialog and disable button to avoid duplicate clicks
                        yesButton.enabled = false
                        confirmExitDialog.close()
                        // Call quitApp() directly so the application quits immediately
                        // when invoked from the QML (main) thread.
                        simulator.quitApp()
                    }
                }
                Button { text: "No"; onClicked: confirmExitDialog.close() }
            }
        }
    }
}
