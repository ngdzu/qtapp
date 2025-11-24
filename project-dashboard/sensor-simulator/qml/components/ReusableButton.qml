import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import qml 1.0

Button {
    id: root

    // Reusable styling tokens (can be overridden by the caller)
    property color textColor: Theme.textPrimary
    property color backgroundColor: Theme.colorWithOpacity(Theme.textPrimary, 0.02)
    property color borderColor: Theme.border
    property int radius: Theme.radiusSm
    property int fontSize: Theme.fontSizeBase

    // sensible defaults so it behaves like previous buttons
    Layout.preferredWidth: Math.max(80, implicitWidth + (Theme.spacingMd * 2))
    Layout.preferredHeight: Theme.buttonHeightSm

    // Use Button's default text centering - just style the background
    background: Rectangle {
        anchors.fill: parent
        color: root.backgroundColor
        border.color: root.borderColor
        border.width: Theme.dividerHeight
        radius: root.radius
    }

    // Minimal contentItem override - preserve Button's default alignment behavior
    contentItem: Text {
        text: root.text
        color: root.textColor
        font.pixelSize: root.fontSize
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
