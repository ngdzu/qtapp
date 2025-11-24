import QtQuick 2.15
import QtQuick.Layouts 1.12
import qml 1.0

Rectangle {
    id: root
    
    property var dataPoints: [] // Array of numbers
    property int maxPoints: 1000 // Keep last 1000 points
    property color waveformColor: Theme.accentEmerald
    property color gridColor: Theme.accentEmerald
    
    color: Theme.cardBackground
    border.color: Theme.border
    border.width: 1
    radius: Theme.radiusXl
    
    // Internal buffer to accumulate waveform data
    property var waveformBuffer: []
    
    function addSamples(samples) {
        for (var i = 0; i < samples.length; i++) {
            waveformBuffer.push(samples[i])
        }
        // Keep only last maxPoints
        if (waveformBuffer.length > maxPoints) {
            waveformBuffer = waveformBuffer.slice(waveformBuffer.length - maxPoints)
        }
        canvas.requestPaint()
    }
    
    // Grid background pattern
    Canvas {
        id: gridCanvas
        anchors.fill: parent
        z: 0
        
        Component.onCompleted: {
            requestPaint()
        }
        
        onPaint: {
            var ctx = getContext("2d")
            ctx.strokeStyle = Theme.colorWithOpacity(Theme.accentEmerald, 0.1)
            ctx.lineWidth = 0.5
            
            // Vertical lines (every 20px)
            for (var x = 0; x < width; x += 20) {
                ctx.beginPath()
                ctx.moveTo(x, 0)
                ctx.lineTo(x, height)
                ctx.stroke()
            }
            
            // Horizontal lines (every 20px)
            for (var y = 0; y < height; y += 20) {
                ctx.beginPath()
                ctx.moveTo(0, y)
                ctx.lineTo(width, y)
                ctx.stroke()
            }
        }
    }
    
    // Header - matching React reference
    RowLayout {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: Theme.spacingLg
        spacing: Theme.spacingSm
        
        Text {
            text: "Lead II"
            color: Theme.accentEmerald
            font.pixelSize: Theme.fontSizeXs
            font.bold: true
            font.letterSpacing: 2
        }
        
        Item { Layout.fillWidth: true }
        
        Text {
            text: "250 Hz / 25 mm/s"
            color: Theme.textMuted
            font.pixelSize: Theme.fontSizeXs
            font.family: Theme.fontFamilyMono
        }
    }
    
    // Waveform canvas
    Canvas {
        id: canvas
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: Theme.spacingMd
        z: 10
        
        Component.onCompleted: {
            // Initial paint to show grid even before data arrives
            requestPaint()
        }
        
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            
            if (waveformBuffer.length < 2) return
            
            // Show last 300 samples (window size)
            var windowSize = 300
            var slice = waveformBuffer.slice(-windowSize)
            
            // Normalize data to fit height
            var min = -50 // Expected min value
            var max = 150 // Expected max value (R peak is ~100)
            var range = max - min
            
            ctx.strokeStyle = waveformColor
            ctx.lineWidth = 2
            ctx.lineJoin = "round"
            ctx.shadowColor = Theme.colorWithOpacity(Theme.accentEmerald, 0.5)
            ctx.shadowBlur = 4
            
            ctx.beginPath()
            for (var i = 0; i < slice.length; i++) {
                var x = (i / (slice.length - 1)) * width
                var normalizedY = (slice[i] - min) / range
                var y = height - (normalizedY * height)
                
                if (i === 0) {
                    ctx.moveTo(x, y)
                } else {
                    ctx.lineTo(x, y)
                }
            }
            ctx.stroke()
        }
    }
    
    // Scan line effect (gradient overlay on right)
    Rectangle {
        anchors.top: canvas.top
        anchors.right: parent.right
        anchors.bottom: canvas.bottom
        width: Theme.waveformScanlineWidth
        z: 20
        gradient: Gradient {
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 1.0; color: root.color }
        }
    }
}

