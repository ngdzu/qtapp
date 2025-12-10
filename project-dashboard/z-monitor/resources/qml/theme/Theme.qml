pragma Singleton
import QtQuick 2.15

QtObject {
    // Colors matching sample_app constants.ts
    readonly property var colors: {
        return {
            background: "#09090b" // Zinc-950
            ,
            panel: "#18181b"      // Zinc-900
            ,
            border: "#27272a"     // Zinc-800
            ,
            text: "#fafafa"       // Zinc-50
            ,
            textMuted: "#71717a"  // Zinc-500
            ,
            ECG: "#10b981"        // Emerald-500
            ,
            SPO2: "#3b82f6"       // Blue-500
            ,
            RESP: "#eab308"       // Yellow-500
            ,
            NIBP: "#a1a1aa"       // Zinc-400
            ,
            TEMP: "#d946ef"       // Fuchsia-500
            ,
            critical: "#ef4444"   // Red-500
            ,
            warning: "#f59e0b"    // Amber-500
            ,
            success: "#10b981"    // Emerald-500
            ,
            banner: "#27272a"     // Zinc-800
            ,
            bannerBorder: "#3f3f46" // Zinc-700
            ,
            clinicianSubtle: "#3f3f46" // Zinc-700
        };
    }

    // Fonts
    readonly property var fonts: {
        return {
            mono: "monospace"
        };
    }

    // Spacing
    readonly property var spacing: {
        return {
            xs: 4,
            sm: 8,
            md: 12,
            lg: 16,
            xl: 20
        };
    }
}
