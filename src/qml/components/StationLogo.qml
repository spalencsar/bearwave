// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15

import theme 1.0

Rectangle {
    id: root

    required property var app
    required property string stationName
    required property string stationKey
    property string faviconUrl: ""
    property string homepageUrl: ""
    property int logoMargin: 4

    readonly property var gradients: [
        ["#1d4ed8", "#0891b2"],
        ["#7c3aed", "#db2777"],
        ["#047857", "#65a30d"],
        ["#b45309", "#dc2626"],
        ["#4338ca", "#7c3aed"],
        ["#0f766e", "#2563eb"]
    ]
    readonly property int paletteIndex: app.backend
        ? app.backend.stationLogoPaletteIndex(stationKey || stationName,
                                              gradients.length) : 0
    readonly property var logoColors: gradients[paletteIndex]
    readonly property string resolvedLogo: {
        if (!app.backend) return ""
        var revision = app.backend.stationImageRevision
        if (revision < 0) return ""
        return app.backend.stationLogoSource(stationKey || stationName,
                                             faviconUrl, homepageUrl)
    }

    radius: 6
    clip: true
    color: BearTheme.imageWell
    border.color: BearTheme.cardBorder

    Rectangle {
        anchors.fill: parent
        visible: !logoImage.visible
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: root.logoColors[0] }
            GradientStop { position: 1.0; color: root.logoColors[1] }
        }

        Text {
            anchors.centerIn: parent
            text: root.app.backend
                  ? root.app.backend.stationInitials(root.stationName) : "♫"
            color: "white"
            font.bold: true
            font.pixelSize: Math.max(12, Math.round(root.height * 0.34))
        }
    }

    Image {
        id: logoImage
        anchors.fill: parent
        anchors.margins: root.logoMargin
        source: root.resolvedLogo
        fillMode: Image.PreserveAspectFit
        asynchronous: true
        cache: true
        smooth: true
        visible: source !== "" && status === Image.Ready
    }
}
