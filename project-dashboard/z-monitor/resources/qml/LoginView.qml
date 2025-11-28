import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/**
 * LoginView.qml
 * @brief Login screen for user authentication.
 *
 * Provides user ID and secret code input fields, authentication status,
 * and error message display. Connects to AuthenticationController for
 * authentication logic.
 */
Item {
    id: loginView
    anchors.fill: parent

    // Connect to AuthenticationController (registered in C++ as "AuthenticationController")
    property var authController: null

    Rectangle {
        anchors.fill: parent
        color: "#101820"  // Dark background matching Main.qml

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 24
            width: 400

            // Title
            Text {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Z Monitor Login")
                font.pixelSize: 32
                font.bold: true
                color: "#ffffff"
            }

            // User ID Input
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: qsTr("User ID")
                    font.pixelSize: 14
                    color: "#a0b0c0"
                }

                TextField {
                    id: userIdField
                    Layout.fillWidth: true
                    placeholderText: qsTr("Enter user ID (e.g., NURSE001)")
                    font.pixelSize: 16
                    color: "#ffffff"
                    background: Rectangle {
                        color: "#1e293b"
                        border.color: "#475569"
                        border.width: 1
                        radius: 4
                    }
                    enabled: !authController || !authController.isAuthenticating

                    Keys.onReturnPressed: {
                        secretCodeField.forceActiveFocus()
                    }
                }
            }

            // Secret Code Input
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: qsTr("Secret Code")
                    font.pixelSize: 14
                    color: "#a0b0c0"
                }

                TextField {
                    id: secretCodeField
                    Layout.fillWidth: true
                    placeholderText: qsTr("Enter secret code/PIN")
                    font.pixelSize: 16
                    color: "#ffffff"
                    echoMode: TextInput.Password
                    background: Rectangle {
                        color: "#1e293b"
                        border.color: "#475569"
                        border.width: 1
                        radius: 4
                    }
                    enabled: !authController || !authController.isAuthenticating

                    Keys.onReturnPressed: {
                        if (loginButton.enabled) {
                            loginButton.clicked()
                        }
                    }
                }
            }

            // Error Message
            Text {
                id: errorText
                Layout.fillWidth: true
                visible: authController && authController.loginError !== ""
                text: authController ? authController.loginError : ""
                font.pixelSize: 14
                color: "#ef4444"  // Red for errors
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            // Login Button
            Button {
                id: loginButton
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                text: authController && authController.isAuthenticating 
                      ? qsTr("Authenticating...") 
                      : qsTr("Login")
                enabled: userIdField.text !== "" && 
                         secretCodeField.text !== "" && 
                         (!authController || !authController.isAuthenticating)

                background: Rectangle {
                    color: loginButton.enabled ? "#4f46e5" : "#475569"
                    radius: 4
                }

                contentItem: Text {
                    text: loginButton.text
                    font.pixelSize: 16
                    color: "#ffffff"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    if (authController) {
                        authController.login(userIdField.text, secretCodeField.text)
                    }
                }
            }

            // Loading Spinner (shown during authentication)
            BusyIndicator {
                Layout.alignment: Qt.AlignHCenter
                visible: authController && authController.isAuthenticating
                running: visible
            }

            // Test Users Hint (development only)
            Text {
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 32
                visible: true  // Show in development, hide in production
                text: qsTr("Test Users:\nNURSE001/1234\nPHYSICIAN001/5678\nTECH001/9999\nADMIN001/0000")
                font.pixelSize: 12
                color: "#64748b"
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    // Handle login success - navigation will be handled by Main.qml
    Connections {
        target: authController
        function onLoginSucceeded() {
            // Clear input fields
            userIdField.text = ""
            secretCodeField.text = ""
            // Navigation to dashboard will be handled by Main.qml based on isLoggedIn state
        }
    }
}

