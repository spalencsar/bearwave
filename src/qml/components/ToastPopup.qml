// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import theme 1.0

Popup {
    id: root

    required property var window

    padding: 10
    closePolicy: Popup.NoAutoClose
    x: (window.width - width) / 2
    y: window.height - height - 20

    function show(message) {
        toastLabel.text = message
        open()
    }

    background: Rectangle {
        radius: 10
        color: "#24364e"
        border.color: BearTheme.accent
    }

    contentItem: Label {
        id: toastLabel
        color: BearTheme.textMain
        font.pixelSize: 12
    }

    Timer {
        interval: 1400
        running: root.visible
        repeat: false
        onTriggered: root.close()
    }
}