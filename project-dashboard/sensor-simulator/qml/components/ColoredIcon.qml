import QtQuick 2.15
import QtQuick.Effects 1.0

// Colored SVG icon component
Item {
    id: root
    
    property string source: ""
    property color iconColor: "white"
    property int iconSize: 24
    
    width: iconSize
    height: iconSize
    
    // Simple approach: Use Image with a colored rectangle overlay
    // Since SVG uses stroke="currentColor", we need to apply color differently
    Image {
        id: svgImage
        anchors.fill: parent
        source: root.source
        sourceSize.width: root.iconSize
        sourceSize.height: root.iconSize
        fillMode: Image.PreserveAspectFit
        
        // Apply color using layer and shader effect
        layer.enabled: true
        layer.effect: ShaderEffect {
            property color color: root.iconColor
            property variant source: svgImage
            
            fragmentShader: "
                varying vec2 qt_TexCoord0;
                uniform sampler2D source;
                uniform vec4 color;
                void main() {
                    vec4 pixel = texture2D(source, qt_TexCoord0);
                    // Apply color while preserving alpha
                    gl_FragColor = vec4(color.rgb * pixel.rgb, pixel.a);
                }
            "
        }
    }
}

