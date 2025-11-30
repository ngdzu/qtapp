/**
 * @file Panel.qml
 * @brief Reusable panel container component
 *
 * Provides consistent panel styling matching React bg-medical-panel, border-zinc-800
 * Used across all views for consistency
 *
 * @author Z Monitor Team
 * @date 2025-11-30
 */

import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    color: "#18181b"      // Zinc-900 (medical-panel)
    border.color: "#27272a" // Zinc-800
    border.width: 1
    radius: 8

    // Allow children to be added easily
    default property alias children: container.children

    Item {
        id: container
        anchors.fill: parent
        anchors.margins: 12
    }
}
