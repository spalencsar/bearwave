// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Rectangle {
    id: btn

    property string text: ""
    property string iconText: ""
    property bool isActive: false
    signal clicked()

    Layout.fillWidth: true
    Layout.preferredHeight: 32
    radius: 6

    color: isActive
        ? "#4b4b4f"
        : (mouseArea.containsMouse ? "#36373d" : "transparent")

    border.color: "transparent"
    border.width: 0

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: btn.clicked()
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 9
        anchors.rightMargin: 9
        spacing: 10

        Label {
            text: btn.iconText
            font.pixelSize: 14
            Layout.alignment: Qt.AlignVCenter
        }

        Label {
            text: btn.text
            color: btn.isActive ? BearTheme.textMain : (mouseArea.containsMouse ? BearTheme.textMain : BearTheme.textMuted)
            font.bold: btn.isActive
            font.pixelSize: 12
            elide: Text.ElideRight
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
        }
    }
}
