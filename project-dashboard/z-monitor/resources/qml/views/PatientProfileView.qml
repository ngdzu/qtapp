import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components" as Components

Item {
    id: profileRoot
    anchors.fill: parent

    // Themeable colors (passed from Main or use sensible defaults)
    property color panelColor: "#1e1e1e"
    property color borderColor: "#404040"
    property color textColor: "#ffffff"
    property color accentColor: "#3daee9"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        // Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            color: panelColor
            border.color: borderColor
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                Text {
                    text: "Patient Profile"
                    font.pixelSize: 14
                    font.bold: true
                    color: textColor
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: "Back"
                    onClicked: root.currentView = root.viewState.Monitor
                }
            }
        }

        // Content
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: panelColor
            border.color: borderColor
            border.width: 1

            GridLayout {
                columns: 2
                rowSpacing: 12
                columnSpacing: 12
                anchors.fill: parent
                anchors.margins: 16

                // Left column: Identity & current location
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    GroupBox {
                        title: "Identity"
                        Layout.fillWidth: true
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 6
                            Label { text: "Name: " + patientController.displayName; color: textColor }
                            Label { text: "MRN: " + patientController.mrn; color: textColor }
                            Label { text: "DOB: " + patientController.dob; color: textColor }
                        }
                    }

                    GroupBox {
                        title: "Current Location"
                        Layout.fillWidth: true
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 6
                            Label { text: "Unit: " + patientController.unit; color: textColor }
                            Label { text: "Bed: " + patientController.bed; color: textColor }
                            Label { text: "Status: " + patientController.status; color: textColor }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        Button {
                            text: "Admit"
                            onClicked: admitDialog.open()
                        }
                        Button { text: "Transfer"; enabled: false }
                        Button { text: "Discharge"; enabled: false }
                    }
                }

                // Right column: Notes / allergies / placeholders
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    GroupBox {
                        title: "Notes"
                        Layout.fillWidth: true
                        TextArea {
                            anchors.fill: parent
                            placeholderText: "Clinical notes"
                            readOnly: true
                        }
                    }

                    GroupBox {
                        title: "Allergies"
                        Layout.fillWidth: true
                        TextArea {
                            anchors.fill: parent
                            placeholderText: "Allergies"
                            readOnly: true
                        }
                    }
                }
            }
        }

        // Admission modal
        Components.AdmissionDialog { id: admitDialog }
    }
}
