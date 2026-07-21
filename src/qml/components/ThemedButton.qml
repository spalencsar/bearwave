// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import theme 1.0

// Flat dark-theme button without the light default Control border.
Button {
    id: root

    property bool primary: false

    flat: true
    focusPolicy: Qt.StrongFocus

    contentItem: Text {
        text: root.text
        font: root.font
        opacity: root.enabled ? 1.0 : 0.45
        color: root.primary
               ? "#ffffff"
               : (root.down || root.hovered || root.highlighted ? BearTheme.textMain : BearTheme.textMuted)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: 72
        implicitHeight: 32
        radius: 6
        border.width: 1
        border.color: root.primary
                      ? "transparent"
                      : (root.down || root.hovered || root.highlighted ? BearTheme.cardBorder : "transparent")
        color: {
            if (root.primary) {
                if (!root.enabled)
                    return "#3a5f8a"
                if (root.down)
                    return Qt.darker(BearTheme.playingAccent, 1.2)
                if (root.hovered)
                    return Qt.lighter(BearTheme.playingAccent, 1.08)
                return BearTheme.playingAccent
            }
            if (root.down)
                return BearTheme.cardHover
            if (root.hovered || root.highlighted)
                return BearTheme.card
            return "transparent"
        }
    }
}
