import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import qml 1.0
// Theme colors are passed in from parent; avoid hard dependency here

Rectangle {
    id: banner
    // Layout handled by parent
    // Colors provided by parent for flexibility
    property color bannerColor: "#18181b"
    property color bannerBorderColor: "#27272a"
    color: banner.bannerColor
    border.color: banner.bannerBorderColor
    border.width: 1
    radius: 6

    // Public API
    property string name: ""
    property string bedLabel: ""
    property color colorText: "#e5e7eb"
    property color accentColor: "#34d399"

    // Clickable area for future ADT actions
    signal bannerClicked()

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        // Avatar (User icon)
        Image {
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32
            source: "qrc:/qml/icons/user.svg"
            fillMode: Image.PreserveAspectFit
        }

        // Patient Info
        Column {
            Layout.fillWidth: true
            spacing: 2

            Text {
                text: banner.name
                font.pixelSize: 13
                font.bold: true
                color: banner.colorText
            }

            Text {
                text: banner.bedLabel
                font.pixelSize: 10
                font.family: "Monospace"
                color: banner.accentColor
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: banner.bannerClicked()
    }
}
