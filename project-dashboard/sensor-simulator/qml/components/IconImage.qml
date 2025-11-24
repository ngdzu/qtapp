import QtQuick 2.15

// Reusable icon component that handles SVG coloring
// Uses a workaround: Rectangle with color + Image with blend mode
Item {
    id: root
    
    property string source: ""
    property color iconColor: "white"
    property int iconSize: 24
    
    width: iconSize
    height: iconSize
    
    // Workaround for SVG coloring: use a colored rectangle with the icon as a mask
    Rectangle {
        id: colorRect
        anchors.fill: parent
        color: root.iconColor
        visible: false
    }
    
    Image {
        id: iconImage
        anchors.fill: parent
        source: root.source
        sourceSize.width: root.iconSize
        sourceSize.height: root.iconSize
        fillMode: Image.PreserveAspectFit
        
        // Use layer with color blending to apply color to SVG
        layer.enabled: true
        layer.effect: ShaderEffect {
            property color color: root.iconColor
            property variant source: iconImage
            
            fragmentShader: "
                varying vec2 qt_TexCoord0;
                uniform sampler2D source;
                uniform vec4 color;
                void main() {
                    vec4 pixel = texture2D(source, qt_TexCoord0);
                    gl_FragColor = vec4(color.rgb, pixel.a * color.a);
                }
            "
        }
    }
}

