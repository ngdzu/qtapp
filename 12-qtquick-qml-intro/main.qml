import QtQuick
import QtQuick.Controls

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: "Lesson 12: QML Intro"

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#4A90E2" }
            GradientStop { position: 1.0; color: "#357ABD" }
        }

        Column {
            anchors.centerIn: parent
            spacing: 20

            Text {
                text: "Hello Qt Quick!"
                font.pixelSize: 32
                font.bold: true
                color: "white"
            }

            Rectangle {
                id: box
                width: 200
                height: 200
                color: mouseArea.containsMouse ? "lightgreen" : "white"
                radius: 10

                Text {
                    anchors.centerIn: parent
                    text: "Hover over me!"
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: box.rotation += 45
                }
            }

            Button {
                text: "Click Me"
                onClicked: console.log("Button clicked in QML!")
            }
        }
    }
}
