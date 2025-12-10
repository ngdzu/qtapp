/**
 * @file PatientProfileDialog.qml
 * @brief Patient profile modal dialog - matches React design
 *
 * Displays current patient information in a modal overlay
 * Allows discharge and other patient management actions
 *
 * @author Z Monitor Team
 * @date 2025-12-09
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Colors 1.0

// Modal overlay with patient info
Rectangle {
    id: modalRoot
    visible: false
    anchors.fill: parent
    color: Colors.black
    opacity: 0

    // Backdrop color
    Behavior on opacity {
        PropertyAnimation {
            duration: 200
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: modalRoot.close()
    }

    // Modal content
    Rectangle {
        id: modalContent
        width: Math.min(450, parent.width - 32)
        height: mainLayout.implicitHeight
        anchors.centerIn: parent

        color: Colors.zinc900
        border.color: Colors.zinc700
        border.width: 1
        radius: 12

        // Smooth entrance animation
        scale: modalRoot.visible ? 1.0 : 0.95
        Behavior on scale {
            PropertyAnimation {
                duration: 200
            }
        }

        ColumnLayout {
            id: mainLayout
            width: parent.width
            spacing: 0

            // Header
            Rectangle {
                id: header
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                color: Colors.zinc800
                border.color: Colors.zinc700
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 8

                    // Header icon
                    Rectangle {
                        width: 24
                        height: 24
                        color: Colors.transparent

                        Text {
                            anchors.centerIn: parent
                            text: "👤"
                            font.pixelSize: 16
                        }
                    }

                    // Header title
                    Text {
                        text: "Current Patient"
                        font.pixelSize: 16
                        font.bold: true
                        color: Colors.lightGray
                        Layout.fillWidth: true
                    }

                    // Close button
                    Rectangle {
                        width: 32
                        height: 32
                        color: Colors.transparent
                        radius: 6

                        Text {
                            anchors.centerIn: parent
                            text: "✕"
                            font.pixelSize: 18
                            color: Colors.zinc400
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            hoverEnabled: true
                            onClicked: modalRoot.close()
                            onEntered: parent.color = Colors.zinc700
                            onExited: parent.color = Colors.transparent
                        }
                    }
                }
            }

            // Content
            ColumnLayout {
                id: contentLayout
                Layout.fillWidth: true
                Layout.margins: 32
                Layout.topMargin: 24
                Layout.bottomMargin: 32
                spacing: 24

                // Patient info row (avatar + name/mrn)
                RowLayout {
                    spacing: 16
                    Layout.fillWidth: true

                    // Avatar circle
                    Rectangle {
                        width: 80
                        height: 80
                        color: Colors.zinc800
                        border.color: Colors.emerald500
                        border.width: 2
                        radius: 40

                        Text {
                            anchors.centerIn: parent
                            text: "JD"
                            font.pixelSize: 28
                            font.bold: true
                            color: Colors.lightGray
                        }
                    }

                    // Name and MRN
                    ColumnLayout {
                        spacing: 4
                        Layout.fillWidth: true

                        Text {
                            text: "DOE, JOHN"
                            font.pixelSize: 18
                            font.bold: true
                            color: Colors.lightGray
                        }

                        Text {
                            text: "MRN: 12345"
                            font.pixelSize: 12
                            font.family: "Courier New"
                            color: Colors.zinc400
                        }
                    }
                }

                // Info grid (2 columns)
                GridLayout {
                    columns: 2
                    rowSpacing: 12
                    columnSpacing: 12
                    Layout.fillWidth: true

                    // Assigned Location
                    Rectangle {
                        color: Colors.zinc950
                        border.color: Colors.zinc700
                        border.width: 1
                        radius: 6
                        Layout.fillWidth: true
                        Layout.preferredHeight: 60

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 4

                            Text {
                                text: "ASSIGNED LOCATION"
                                font.pixelSize: 10
                                font.bold: true
                                color: Colors.zinc500
                                font.capitalization: Font.AllUppercase
                            }

                            Text {
                                text: "BED-94"
                                font.pixelSize: 14
                                font.family: "Courier New"
                                font.bold: true
                                color: Colors.lightGray
                                Layout.fillHeight: true
                            }
                        }
                    }

                    // DOB / Sex
                    Rectangle {
                        color: Colors.zinc950
                        border.color: Colors.zinc700
                        border.width: 1
                        radius: 6
                        Layout.fillWidth: true
                        Layout.preferredHeight: 60

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 4

                            Text {
                                text: "DOB / SEX"
                                font.pixelSize: 10
                                font.bold: true
                                color: Colors.zinc500
                                font.capitalization: Font.AllUppercase
                            }

                            Text {
                                text: "1980-05-12 (M)"
                                font.pixelSize: 14
                                font.family: "Courier New"
                                font.bold: true
                                color: Colors.lightGray
                                Layout.fillHeight: true
                            }
                        }
                    }
                }

                // Divider
                Rectangle {
                    color: Colors.zinc700
                    height: 1
                    Layout.fillWidth: true
                }

                // Discharge button
                Rectangle {
                    id: dischargeBtn
                    color: Colors.red900_20
                    border.color: Colors.red900_50
                    border.width: 1
                    radius: 8
                    Layout.fillWidth: true
                    Layout.preferredHeight: 48

                    Text {
                        anchors.centerIn: parent
                        text: "⊕ Discharge Patient"  // Cross-ish shape
                        font.pixelSize: 14
                        font.bold: true
                        color: Colors.red500
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: {
                            if (confirm("Discharge patient and clear all data?")) {
                                modalRoot.close();
                                // Emit discharge signal
                                modalRoot.discharged();
                            }
                        }
                        onEntered: parent.color = Colors.red900_40
                        onExited: parent.color = Colors.red900_20
                    }
                }
            }
        }
    }

    // Public methods
    function open() {
        visible = true;
        opacity = 0.8;
        modalContent.forceActiveFocus();
    }

    function close() {
        opacity = 0;
        visible = false;
    }

    // Signals
    signal discharged

    // Handle ESC key
    Keys.onEscapePressed: close()
}
