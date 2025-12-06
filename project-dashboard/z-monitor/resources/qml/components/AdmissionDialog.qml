import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    title: "Admit Patient"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    ColumnLayout {
        spacing: 10
        
        Label {
            text: "Patient Admission"
            font.bold: true
            font.pixelSize: 16
        }

        GridLayout {
            columns: 2
            rowSpacing: 10
            columnSpacing: 10

            Label { text: "MRN:" }
            TextField { 
                id: mrnField
                placeholderText: "Medical Record Number"
                Layout.fillWidth: true
            }

            Label { text: "First Name:" }
            TextField {
                id: firstNameField
                placeholderText: "First Name"
                Layout.fillWidth: true
            }

            Label { text: "Last Name:" }
            TextField {
                id: lastNameField
                placeholderText: "Last Name"
                Layout.fillWidth: true
            }
        }
    }

    onAccepted: {
        console.log("Admitting patient: " + mrnField.text)
    }
}
