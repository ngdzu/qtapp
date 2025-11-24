pragma Singleton
import QtQuick 2.15

QtObject {
    // Color Palette - Backgrounds (matching Tailwind slate colors exactly)
    readonly property color background: "#020617"           // slate-950 (exact from React)
    readonly property color backgroundSecondary: "#0f172a"  // slate-900
    readonly property color cardBackground: "#0f172a"       // slate-900
    readonly property color cardBackgroundSecondary: "#1e293b" // slate-800
    readonly property color border: "#1e293b"                // slate-800 border
    
    // Color Palette - Text (matching Tailwind exactly)
    readonly property color textPrimary: "#e2e8f0"          // slate-200
    readonly property color textSecondary: "#94a3b8"       // slate-400
    readonly property color textMuted: "#64748b"           // slate-500
    readonly property color textMutedDark: "#475569"       // slate-600
    
    // Color Palette - Accents (matching Tailwind exactly)
    readonly property color accentEmerald: "#10b981"        // emerald-500
    readonly property color accentEmeraldLight: "#34d399"    // emerald-400
    readonly property color accentSky: "#38bdf8"            // sky-400
    readonly property color accentAmber: "#fbbf24"          // amber-400
    readonly property color accentIndigo: "#4f46e5"         // indigo-600
    readonly property color accentRed: "#ef4444"            // red-500
    readonly property color accentRedLight: "#f87171"        // red-400
    readonly property color accentOrange: "#fb923c"          // orange-400
    
    // Color Palette - Semantic
    readonly property color success: accentEmerald
    readonly property color warning: accentOrange
    readonly property color error: accentRed
    readonly property color info: accentSky
    readonly property color primary: accentIndigo
    
    // Typography - Matching React reference (Inter + JetBrains Mono)
    readonly property string fontFamily: "Inter, -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif"
    readonly property string fontFamilyMono: "'JetBrains Mono', 'JetBrainsMono', 'Monaco', 'Courier New', monospace"
    
    // Font sizes matching Tailwind (React reference)
    readonly property int fontSizeXs: 12        // text-xs (0.75rem)
    readonly property int fontSizeSm: 14         // text-sm (0.875rem)
    readonly property int fontSizeBase: 12       // text-xs base
    readonly property int fontSizeMd: 14         // text-sm
    readonly property int fontSizeLg: 16         // text-base
    readonly property int fontSizeXl: 20        // text-xl (1.25rem)
    readonly property int fontSize2xl: 20       // text-xl
    readonly property int fontSize3xl: 24       // text-2xl
    readonly property int fontSize4xl: 48        // text-5xl (3rem) for vital values
    readonly property int fontSize5xl: 48       // text-5xl
    readonly property int fontSize10px: 10       // text-[10px] custom
    
    // Spacing (matching Tailwind spacing scale exactly)
    readonly property int spacingXs: 4      // gap-1, space-y-1
    readonly property int spacingSm: 8      // gap-2, p-2
    readonly property int spacingMd: 12     // gap-3, p-3
    readonly property int spacingLg: 16     // gap-4, p-4
    readonly property int spacingXl: 24     // gap-6, mb-6
    readonly property int spacing2xl: 32
    
    // Border Radius
    readonly property int radiusSm: 6
    readonly property int radiusMd: 8
    readonly property int radiusLg: 10
    readonly property int radiusXl: 12
    
    // Sizes
    readonly property int headerHeight: 56
    readonly property int buttonHeight: 44
    readonly property int buttonHeightLg: 52
    readonly property int buttonHeightXl: 56
    readonly property int buttonHeightSm: 36
    readonly property int buttonHeightXs: 30
    readonly property int cardPadding: 16
    readonly property int iconSize: 24
    readonly property int iconSizeSm: 20
    readonly property int iconSizeXs: 12
    
    // Component-specific sizes
    readonly property int logHeaderHeight: 48
    readonly property int logEntryHeight: 32
    readonly property int logBadgeWidth: 70
    readonly property int logBadgeHeight: 20
    readonly property int logTimeWidth: 80
    readonly property int logIndicatorWidth: 3
    readonly property int comboBoxWidth: 120
    readonly property int comboBoxHeight: 30
    readonly property int comboBoxIndicatorWidth: 12
    readonly property int comboBoxIndicatorHeight: 8
    readonly property int eventBadgeWidth: 60
    readonly property int eventBadgeHeight: 20
    readonly property int eventBadgeRadius: 10
    readonly property int filterChipHeight: 32
    readonly property int statusDotSize: 6
    readonly property int statusDotRadius: 3
    readonly property int eventIconSize: 32
    readonly property int vitalCardMinWidth: 140
    readonly property int vitalCardHeight: 120
    readonly property int systemInfoHeight: 64
    readonly property int dividerHeight: 1
    readonly property int scrollBarSize: 8
    readonly property int dialogWidth: 420
    readonly property int dialogHeight: 200
    readonly property int dialogHeaderHeight: 56
    readonly property int dialogButtonWidth: 140
    readonly property int topBarIconSize: 40
    readonly property int topBarLastEventWidth: 300
    readonly property int topBarLastEventHeight: 36
    readonly property int waveformScanlineWidth: 64
    readonly property int controlPanelWidth: 320
    readonly property int controlPanelMinWidth: 280
    readonly property int waveformHeight: 180
    
    // Opacity
    readonly property real opacityDisabled: 0.5
    readonly property real opacityMuted: 0.6
    readonly property real opacitySubtle: 0.1
    
    // Shadows (using rgba for glow effects)
    function rgba(color, opacity) {
        return Qt.rgba(
            parseInt(color.substring(1, 3), 16) / 255.0,
            parseInt(color.substring(3, 5), 16) / 255.0,
            parseInt(color.substring(5, 7), 16) / 255.0,
            opacity
        )
    }
    
    // Helper function for color with opacity
    function colorWithOpacity(baseColor, opacity) {
        return Qt.rgba(
            baseColor.r,
            baseColor.g,
            baseColor.b,
            opacity
        )
    }
}

