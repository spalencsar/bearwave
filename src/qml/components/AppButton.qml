// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import theme 1.0

// Soft chip button — flatter than stock Fusion/KDE toolbuttons.
Button {
    id: control

    focusPolicy: Qt.TabFocus
    leftPadding: 14
    rightPadding: 14
    topPadding: 7
    bottomPadding: 7
    font.pixelSize: 12

    contentItem: Label {
        text: control.text
        color: {
            if (!control.enabled)
                return BearTheme.textMuted
            if (control.highlighted || control.down)
                return BearTheme.isLight ? BearTheme.accent : BearTheme.textMain
            return BearTheme.textMain
        }
        font.pixelSize: control.font.pixelSize
        font.family: control.font.family
        font.bold: control.highlighted
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        radius: 16
        color: {
            if (control.flat && !control.hovered && !control.highlighted && !control.down)
                return "transparent"
            if (control.highlighted || control.down)
                return BearTheme.isLight ? "#ffe4ee" : "#2a2430"
            if (control.hovered)
                return BearTheme.cardHover
            return BearTheme.isLight ? "#ebebf0" : "#1a1a1e"
        }
        border.width: control.visualFocus ? 1.5 : (control.highlighted ? 1 : 0)
        border.color: control.visualFocus || control.highlighted ? BearTheme.accent : "transparent"
        opacity: control.enabled ? 1.0 : 0.5
    }
}
