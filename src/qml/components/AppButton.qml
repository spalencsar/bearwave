// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import theme 1.0

Button {
    id: control

    // Keep keyboard navigation without retaining focus after a mouse click.
    focusPolicy: Qt.TabFocus
    leftPadding: 12
    rightPadding: 12
    topPadding: 6
    bottomPadding: 6

    contentItem: Label {
        text: control.text
        color: control.enabled ? BearTheme.textMain : BearTheme.textMuted
        font: control.font
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        radius: 7
        color: control.down || control.highlighted
               ? BearTheme.selection
               : (control.hovered
                  ? BearTheme.cardHover
                  : (control.flat ? "transparent" : BearTheme.imageWell))
        border.color: control.visualFocus || control.hovered
                      ? BearTheme.accent
                      : (control.highlighted
                         ? BearTheme.selectionBorder
                         : BearTheme.cardBorder)
        border.width: control.flat
                      && !control.hovered
                      && !control.highlighted
                      && !control.visualFocus ? 0 : 1
        opacity: control.enabled ? 1.0 : 0.55
    }
}
