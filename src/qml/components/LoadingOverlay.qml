// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import theme 1.0

Rectangle {
    id: root

    required property var app

    visible: app.backend && app.backend.loading
    anchors.fill: parent
    color: "#7f0b121a"
    z: 20

    Rectangle {
        anchors.centerIn: parent
        width: 210
        height: 110
        radius: 12
        color: BearTheme.panel
        border.color: BearTheme.cardBorder

        Column {
            anchors.centerIn: parent
            spacing: 10

            BusyIndicator {
                running: true
                width: 36
                height: 36
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Label {
                text: qsTr("Loading stations...")
                color: BearTheme.textMain
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}