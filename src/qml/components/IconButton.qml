// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import theme 1.0

// Circular/rounded button with MediaIcon (no emoji).
Button {
    id: control

    property string iconName: "play"
    property int iconSize: 20
    property bool round: false
    property bool accentFill: false
    property color iconColor: {
        if (!control.enabled)
            return BearTheme.textMuted
        if (control.accentFill)
            return "#ffffff"
        if (control.highlighted || control.down)
            return BearTheme.accent
        return BearTheme.textMain
    }

    focusPolicy: Qt.TabFocus
    implicitWidth: 44
    implicitHeight: 44
    padding: 0

    contentItem: Item {
        MediaIcon {
            anchors.centerIn: parent
            width: control.iconSize
            height: control.iconSize
            name: control.iconName
            color: control.iconColor
        }
    }

    background: Rectangle {
        radius: control.round ? width / 2 : 14
        color: {
            if (control.accentFill)
                return control.down ? "#c93a68"
                       : (control.hovered ? "#ff6a9a" : BearTheme.accent)
            if (control.flat && !control.hovered && !control.highlighted && !control.down)
                return "transparent"
            if (control.highlighted || control.down)
                return BearTheme.isLight ? "#ffe4ee" : "#2a2430"
            if (control.hovered)
                return BearTheme.cardHover
            return BearTheme.isLight ? "#ebebf0" : "#1a1a1e"
        }
        border.width: control.visualFocus ? 1.5 : (control.highlighted && !control.accentFill ? 1 : 0)
        border.color: control.visualFocus || control.highlighted ? BearTheme.accent : "transparent"
        opacity: control.enabled ? 1.0 : 0.5
        Behavior on color { ColorAnimation { duration: 100 } }
    }
}
