// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Rectangle {
    id: root

    required property var app

    color: BearTheme.sidebar
    border.color: "transparent"
    radius: 0

    function showTop() {
        if (!app.backend) return
        app.currentPage = "top"
        app.activeQuickFilter = ""
        app.backend.loadTopStations()
    }

    function showGerman() {
        if (!app.backend) return
        app.currentPage = "german"
        app.activeQuickFilter = ""
        app.backend.loadGermanStations()
    }

    function showDutch() {
        if (!app.backend) return
        app.currentPage = "dutch"
        app.activeQuickFilter = ""
        app.backend.loadDutchStations()
    }

    function showWorld() {
        if (!app.backend) return
        app.currentPage = "world"
        app.activeQuickFilter = "world"
        app.selectedWorldCategory = ""
        app.selectedWorldType = ""
        if (app.backend.countries.length === 0) {
            app.backend.loadCountries()
        }
    }

    function showFavorites() {
        app.currentPage = "favorites"
        app.activeQuickFilter = ""
        if (app.backend && app.backend.favoriteStations.length > 0) {
            app.backend.selectFavoriteStation(0)
        }
    }

    function showHistory() {
        app.currentPage = "history"
        app.activeQuickFilter = ""
        var recent = app.backend ? app.backend.recentStations : []
        if (app.backend && recent.length > 0) {
            app.backend.selectRecentByUuid(recent[0].uuid, recent[0].urlResolved)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 10
        anchors.topMargin: 14
        anchors.bottomMargin: 10
        spacing: 9

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Image {
                Layout.preferredWidth: 120
                Layout.preferredHeight: 34
                source: "qrc:/assets/app/bearwave_line.png"
                fillMode: Image.PreserveAspectFit
                smooth: true
                mipmap: true
            }
        }

        SidebarSectionLabel { text: qsTr("Stations") }
        SidebarRow { text: qsTr("Top"); iconText: "▮"; active: app.currentPage === "top"; onClicked: root.showTop() }
        SidebarRow { text: qsTr("Germany"); iconText: "⚑"; active: app.currentPage === "german"; onClicked: root.showGerman() }
        SidebarRow { text: qsTr("Netherlands"); iconText: "⚐"; active: app.currentPage === "dutch"; onClicked: root.showDutch() }
        SidebarRow { text: qsTr("World"); iconText: "◎"; active: app.currentPage === "world"; onClicked: root.showWorld() }
        SidebarRow { text: qsTr("Search Results"); iconText: "⌕"; active: app.currentPage === "search"; onClicked: app.currentPage = "search" }

        SidebarSectionLabel { text: qsTr("Library"); Layout.topMargin: 10 }
        SidebarRow {
            text: qsTr("Favorites")
            iconText: "★"
            active: app.currentPage === "favorites"
            badge: app.backend ? app.backend.favoriteStations.length : 0
            onClicked: root.showFavorites()
        }
        SidebarRow {
            text: qsTr("Recent")
            iconText: "◴"
            active: app.currentPage === "history"
            badge: app.backend ? app.backend.recentStations.length : 0
            onClicked: root.showHistory()
        }
        SidebarRow {
            text: qsTr("Manual")
            iconText: "+"
            active: false
            onClicked: app.addDialog.open()
        }

        SidebarSectionLabel { text: qsTr("Folders"); Layout.topMargin: 10 }
        Label {
            Layout.fillWidth: true
            text: qsTr("No folders yet")
            color: BearTheme.textMuted
            font.pixelSize: 11
            wrapMode: Text.WordWrap
        }

        Item { Layout.fillHeight: true }

        RowLayout {
            Layout.fillWidth: true
            AppButton {
                text: "+"
                Layout.preferredWidth: 36
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Add Station")
                onClicked: app.addDialog.open()
            }
            AppButton {
                text: "?"
                Layout.preferredWidth: 36
                ToolTip.visible: hovered
                ToolTip.text: qsTr("About")
                onClicked: app.aboutDialog.open()
            }
            AppButton {
                visible: app.backend && app.backend.canResumeLastStation
                text: "↻"
                Layout.preferredWidth: 36
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Resume")
                onClicked: app.backend.resumeLastStation()
            }
        }
    }

    component SidebarSectionLabel: Label {
        Layout.fillWidth: true
        color: BearTheme.textMuted
        font.pixelSize: 11
        font.bold: true
        leftPadding: 4
    }

    component SidebarRow: AppButton {
        id: row
        property string iconText: ""
        property bool active: false
        property int badge: 0

        Layout.fillWidth: true
        height: 30
        highlighted: false
        flat: true
        background: Rectangle {
            radius: 6
            color: row.active ? BearTheme.selection : (row.hovered ? BearTheme.cardHover : "transparent")
            border.color: row.active ? BearTheme.selectionBorder : "transparent"
            border.width: row.active ? 1 : 0
        }
        contentItem: RowLayout {
            spacing: 8
            Label {
                text: row.iconText
                color: row.active ? BearTheme.textMain : BearTheme.textMuted
                font.pixelSize: 14
                Layout.preferredWidth: 18
                horizontalAlignment: Text.AlignHCenter
            }
            Label {
                text: row.text
                color: row.active ? BearTheme.textMain : BearTheme.textMuted
                font.pixelSize: 13
                font.bold: row.active
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
            Label {
                visible: row.badge > 0
                text: row.badge
                color: BearTheme.textMuted
                font.pixelSize: 11
            }
        }
    }
}
