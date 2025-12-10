/**
 * @file Colors.qml
 * @brief Color constants for Z Monitor UI - Tailwind Zinc palette
 *
 * Centralized color definitions matching React/Tailwind design system
 *
 * @author Z Monitor Team
 * @date 2025-12-10
 */

pragma Singleton
import QtQuick

QtObject {
    // Zinc palette
    readonly property color zinc950: "#09090b"
    readonly property color zinc900: "#18181b"
    readonly property color zinc800: "#27272a"
    readonly property color zinc700: "#3f3f46"
    readonly property color zinc500: "#71717a"
    readonly property color zinc400: "#a1a1aa"

    // Emerald (success/active)
    readonly property color emerald500: "#10b981"

    // Red palette (danger/warning)
    readonly property color red900_20: "#7f1d1d"  // 20% opacity equivalent
    readonly property color red900_40: "#9f1239"  // 40% opacity equivalent
    readonly property color red900_50: "#7c2d12"  // 50% opacity equivalent
    readonly property color red500: "#ef4444"

    // Neutral/Light
    readonly property color lightGray: "#e5e7eb"
    readonly property color transparent: "transparent"
    readonly property color black: "#000000"
}

