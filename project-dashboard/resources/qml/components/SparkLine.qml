import QtQuick

Item {
    id: root
    property var dataPoints: []
    property color lineColor: "#10b981"
    property real lineWidth: 2

    Canvas {
        id: canvas
        anchors.fill: parent
        antialiasing: true
        
        onPaint: {
            if (!root.dataPoints || root.dataPoints.length < 2) return;
            
            var ctx = canvas.getContext('2d');
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            
            let points = root.dataPoints;
            let maxVal = 0;
            let minVal = 1000;
            
            // Find range
            for (let i = 0; i < points.length; i++) {
                if (points[i] > maxVal) maxVal = points[i];
                if (points[i] < minVal) minVal = points[i];
            }
            
            // Add some padding to the range
            let range = maxVal - minVal;
            if (range === 0) range = 1;
            
            let stepX = canvas.width / (points.length - 1);
            
            // Draw the line
            ctx.strokeStyle = root.lineColor;
            ctx.lineWidth = root.lineWidth;
            ctx.lineCap = 'round';
            ctx.lineJoin = 'round';
            
            ctx.beginPath();
            let firstY = canvas.height - ((points[0] - minVal) / range) * canvas.height;
            ctx.moveTo(0, firstY);
            
            for (let i = 1; i < points.length; i++) {
                let x = i * stepX;
                let y = canvas.height - ((points[i] - minVal) / range) * canvas.height;
                ctx.lineTo(x, y);
            }
            
            ctx.stroke();
        }
    }
    
    onDataPointsChanged: {
        canvas.requestPaint();
    }
}
