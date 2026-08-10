// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15

// Lightweight monochrome line icons (no emoji).
Item {
    id: root
    property string name: "play"
    // media: play pause stop prev next volume mute
    // ui: heart heartOutline globe copy search plus help refresh clock top radio
    property color color: "#f2f2f5"
    property real stroke: Math.max(1.5, width * 0.09)
    width: 22
    height: 22

    onNameChanged: canvas.requestPaint()
    onColorChanged: canvas.requestPaint()
    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()
    onStrokeChanged: canvas.requestPaint()

    Canvas {
        id: canvas
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            ctx.clearRect(0, 0, width, height)
            ctx.strokeStyle = root.color
            ctx.fillStyle = root.color
            ctx.lineWidth = root.stroke
            ctx.lineCap = "round"
            ctx.lineJoin = "round"

            var p = width * 0.18
            var s = width - p * 2
            var cx = width / 2
            var cy = height / 2

            function line(x1, y1, x2, y2) {
                ctx.beginPath()
                ctx.moveTo(x1, y1)
                ctx.lineTo(x2, y2)
                ctx.stroke()
            }

            switch (root.name) {
            case "play":
                ctx.beginPath()
                ctx.moveTo(p + s * 0.22, p + s * 0.12)
                ctx.lineTo(p + s * 0.22, p + s * 0.88)
                ctx.lineTo(p + s * 0.88, cy)
                ctx.closePath()
                ctx.fill()
                break
            case "pause":
                ctx.fillRect(p + s * 0.18, p + s * 0.15, s * 0.22, s * 0.70)
                ctx.fillRect(p + s * 0.60, p + s * 0.15, s * 0.22, s * 0.70)
                break
            case "stop":
                ctx.fillRect(p + s * 0.22, p + s * 0.22, s * 0.56, s * 0.56)
                break
            case "prev":
                line(p + s * 0.78, p + s * 0.18, p + s * 0.38, cy)
                line(p + s * 0.38, cy, p + s * 0.78, p + s * 0.82)
                line(p + s * 0.28, p + s * 0.18, p + s * 0.28, p + s * 0.82)
                break
            case "next":
                line(p + s * 0.22, p + s * 0.18, p + s * 0.62, cy)
                line(p + s * 0.62, cy, p + s * 0.22, p + s * 0.82)
                line(p + s * 0.72, p + s * 0.18, p + s * 0.72, p + s * 0.82)
                break
            case "volume":
                // speaker body
                ctx.beginPath()
                ctx.moveTo(p + s * 0.12, p + s * 0.38)
                ctx.lineTo(p + s * 0.32, p + s * 0.38)
                ctx.lineTo(p + s * 0.52, p + s * 0.18)
                ctx.lineTo(p + s * 0.52, p + s * 0.82)
                ctx.lineTo(p + s * 0.32, p + s * 0.62)
                ctx.lineTo(p + s * 0.12, p + s * 0.62)
                ctx.closePath()
                ctx.fill()
                // waves
                ctx.beginPath()
                ctx.arc(p + s * 0.52, cy, s * 0.22, -0.7, 0.7, false)
                ctx.stroke()
                ctx.beginPath()
                ctx.arc(p + s * 0.52, cy, s * 0.38, -0.7, 0.7, false)
                ctx.stroke()
                break
            case "mute":
                ctx.beginPath()
                ctx.moveTo(p + s * 0.12, p + s * 0.38)
                ctx.lineTo(p + s * 0.32, p + s * 0.38)
                ctx.lineTo(p + s * 0.52, p + s * 0.18)
                ctx.lineTo(p + s * 0.52, p + s * 0.82)
                ctx.lineTo(p + s * 0.32, p + s * 0.62)
                ctx.lineTo(p + s * 0.12, p + s * 0.62)
                ctx.closePath()
                ctx.fill()
                line(p + s * 0.62, p + s * 0.32, p + s * 0.88, p + s * 0.68)
                line(p + s * 0.88, p + s * 0.32, p + s * 0.62, p + s * 0.68)
                break
            case "heart":
            case "heartOutline":
                ctx.beginPath()
                ctx.moveTo(cx, p + s * 0.78)
                ctx.bezierCurveTo(p + s * 0.08, p + s * 0.52,
                                  p + s * 0.08, p + s * 0.22,
                                  p + s * 0.32, p + s * 0.22)
                ctx.bezierCurveTo(p + s * 0.46, p + s * 0.22,
                                  cx, p + s * 0.34,
                                  cx, p + s * 0.42)
                ctx.bezierCurveTo(cx, p + s * 0.34,
                                  p + s * 0.54, p + s * 0.22,
                                  p + s * 0.68, p + s * 0.22)
                ctx.bezierCurveTo(p + s * 0.92, p + s * 0.22,
                                  p + s * 0.92, p + s * 0.52,
                                  cx, p + s * 0.78)
                if (root.name === "heart")
                    ctx.fill()
                else
                    ctx.stroke()
                break
            case "globe":
                ctx.beginPath()
                ctx.arc(cx, cy, s * 0.38, 0, Math.PI * 2)
                ctx.stroke()
                // meridians / parallels
                ctx.beginPath()
                ctx.ellipse(cx - s * 0.18, cy - s * 0.38, s * 0.36, s * 0.76)
                ctx.stroke()
                line(cx - s * 0.38, cy, cx + s * 0.38, cy)
                break
            case "copy":
                ctx.strokeRect(p + s * 0.28, p + s * 0.12, s * 0.52, s * 0.58)
                ctx.strokeRect(p + s * 0.12, p + s * 0.30, s * 0.52, s * 0.58)
                break
            case "search":
                ctx.beginPath()
                ctx.arc(p + s * 0.40, p + s * 0.40, s * 0.28, 0, Math.PI * 2)
                ctx.stroke()
                line(p + s * 0.60, p + s * 0.60, p + s * 0.86, p + s * 0.86)
                break
            case "plus":
                line(cx, p + s * 0.18, cx, p + s * 0.82)
                line(p + s * 0.18, cy, p + s * 0.82, cy)
                break
            case "help":
            case "info":
                // "i" in circle — clear About/info affordance
                ctx.beginPath()
                ctx.arc(cx, cy, s * 0.40, 0, Math.PI * 2)
                ctx.stroke()
                // dot of i
                ctx.beginPath()
                ctx.arc(cx, p + s * 0.30, Math.max(1.4, s * 0.055), 0, Math.PI * 2)
                ctx.fill()
                // stem of i
                ctx.lineWidth = Math.max(root.stroke, s * 0.12)
                line(cx, p + s * 0.44, cx, p + s * 0.72)
                ctx.lineWidth = root.stroke
                break
            case "refresh":
                ctx.beginPath()
                ctx.arc(cx, cy, s * 0.32, Math.PI * 0.15, Math.PI * 1.55, false)
                ctx.stroke()
                ctx.beginPath()
                ctx.moveTo(p + s * 0.72, p + s * 0.22)
                ctx.lineTo(p + s * 0.88, p + s * 0.38)
                ctx.lineTo(p + s * 0.62, p + s * 0.42)
                ctx.closePath()
                ctx.fill()
                break
            case "clock":
                ctx.beginPath()
                ctx.arc(cx, cy, s * 0.38, 0, Math.PI * 2)
                ctx.stroke()
                line(cx, cy, cx, p + s * 0.28)
                line(cx, cy, p + s * 0.68, cy)
                break
            case "top":
                // simple rising bars
                ctx.fillRect(p + s * 0.14, p + s * 0.55, s * 0.16, s * 0.30)
                ctx.fillRect(p + s * 0.42, p + s * 0.35, s * 0.16, s * 0.50)
                ctx.fillRect(p + s * 0.70, p + s * 0.18, s * 0.16, s * 0.67)
                break
            case "radio":
                ctx.beginPath()
                ctx.arc(cx, p + s * 0.62, s * 0.12, 0, Math.PI * 2)
                ctx.fill()
                ctx.beginPath()
                ctx.arc(cx, p + s * 0.62, s * 0.26, Math.PI * 1.15, Math.PI * 1.85, false)
                ctx.stroke()
                ctx.beginPath()
                ctx.arc(cx, p + s * 0.62, s * 0.40, Math.PI * 1.15, Math.PI * 1.85, false)
                ctx.stroke()
                break
            case "trash":
                // lid
                line(p + s * 0.22, p + s * 0.28, p + s * 0.78, p + s * 0.28)
                line(p + s * 0.38, p + s * 0.28, p + s * 0.42, p + s * 0.16)
                line(p + s * 0.58, p + s * 0.16, p + s * 0.62, p + s * 0.28)
                // body
                ctx.beginPath()
                ctx.moveTo(p + s * 0.28, p + s * 0.28)
                ctx.lineTo(p + s * 0.32, p + s * 0.84)
                ctx.lineTo(p + s * 0.68, p + s * 0.84)
                ctx.lineTo(p + s * 0.72, p + s * 0.28)
                ctx.stroke()
                line(p + s * 0.42, p + s * 0.40, p + s * 0.42, p + s * 0.72)
                line(p + s * 0.58, p + s * 0.40, p + s * 0.58, p + s * 0.72)
                break
            default:
                break
            }
        }
    }
}
