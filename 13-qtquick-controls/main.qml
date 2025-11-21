import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    visible: true
    width: 400
    height: 300
    title: "Lesson 13: Qt Quick Controls"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        Label {
            text: "Qt Quick Controls Demo"
            font.pixelSize: 20
            font.bold: true
        }

        Label {
            text: "Name:"
        }

        TextField {
            id: nameField
            Layout.fillWidth: true
            placeholderText: "Enter your name"
            validator: RegularExpressionValidator {
                regularExpression: /[A-Za-z ]+/
            }
        }

        Label {
            text: "Favorite Color:"
        }

        ComboBox {
            id: colorCombo
            Layout.fillWidth: true
            model: ["Red", "Green", "Blue", "Yellow", "Purple"]
        }

        Button {
            text: "Submit"
            Layout.fillWidth: true
            highlighted: true
            onClicked: {
                if (nameField.text.length > 0) {
                    resultLabel.text = "Hello, " + nameField.text + 
                                     "! Your favorite color is " + 
                                     colorCombo.currentText + "."
                } else {
                    resultLabel.text = "Please enter your name."
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#f0f0f0"
            radius: 5

            Label {
                id: resultLabel
                anchors.centerIn: parent
                text: "Fill the form and click Submit"
                wrapMode: Text.WordWrap
                width: parent.width - 20
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
