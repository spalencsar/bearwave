// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import theme 1.0

Item {
    id: root

    required property var app

    property alias stationList: stationList

    readonly property bool showEmptyState: stationList.count === 0
        && !(app.currentPage === "world" && app.selectedWorldCategory === "")

    ScrollView {
        id: stationScrollView
        anchors.fill: parent
        visible: !(app.currentPage === "world" && app.selectedWorldCategory === "")
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn

        ListView {
            id: stationList
            width: stationScrollView.availableWidth
            spacing: 8
            model: app.activeModel()
            visible: count > 0

            delegate: StationCard {
                app: root.app
                compactMode: root.app.compactMode
                listWidth: stationScrollView.availableWidth
            }
        }
    }

    Column {
        anchors.centerIn: parent
        spacing: 8
        visible: root.showEmptyState

        Label {
            text: app.currentPage === "history" ? qsTr("No playback history") : qsTr("No stations loaded yet")
            color: BearTheme.textMain
            font.bold: true
        }

        Label {
            text: app.currentPage === "history"
                  ? qsTr("Play some stations to build history")
                  : qsTr("Load DE/NL stations or use search")
            color: BearTheme.textMuted
        }
    }
}