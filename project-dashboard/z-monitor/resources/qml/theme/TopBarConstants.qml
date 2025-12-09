/**
 * @file TopBarConstants.qml
 * @brief Constants for TopBar component dimensions and styling
 *
 * Centralized definitions for TopBar sizing to avoid hardcoded values
 *
 * @author Z Monitor Team
 * @date 2025-12-09
 */

pragma Singleton

import QtQuick

QtObject {
    // TopBar dimensions
    readonly property int height: 80
    readonly property int bannerHeight: 64
    readonly property int bannerWidth: 220

    // Spacing and padding
    readonly property int contentMargin: 8
    readonly property int contentSpacing: 4
    readonly property int bannerSpacing: 10

    // PatientBanner dimensions
    readonly property int avatarSize: 40
    readonly property int avatarRadius: 6

    // Font sizes
    readonly property int namePixelSize: 14
    readonly property int labelPixelSize: 11
    readonly property int clockPixelSize: 12
    readonly property int statusPixelSize: 10
    readonly property int statusSmallPixelSize: 8

    // Connection status dimensions
    readonly property int connectionStatusWidth: 220
    readonly property int connectionStatusHeight: 36

    // Icon dimensions
    readonly property int iconSize: 18
    readonly property int bellIconSize: 20

    // Notification bell dimensions
    readonly property int bellButtonSize: 40
    readonly property int bellRadius: 20

    // Divider dimensions
    readonly property int dividerWidth: 1
}
