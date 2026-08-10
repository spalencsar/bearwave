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
    border.width: 0
    radius: 0

    Rectangle {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: 1
        color: BearTheme.cardBorder
        opacity: 0.55
    }

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

    function showMyStations() {
        app.currentPage = "mystations"
        app.activeQuickFilter = ""
        if (app.backend && app.backend.manualStations.length > 0) {
            app.backend.selectManualStation(0)
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
        anchors.leftMargin: 14
        anchors.rightMargin: 12
        anchors.topMargin: 16
        anchors.bottomMargin: 14
        spacing: 4

        // Brand
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            Layout.bottomMargin: 10

            Image {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                width: 128
                height: 36
                source: "qrc:/assets/app/bearwave_line.png"
                fillMode: Image.PreserveAspectFit
                smooth: true
                mipmap: true
            }
        }

        SidebarSectionLabel { text: qsTr("Browse") }

        SidebarRow {
            text: qsTr("Top")
            iconName: "top"
            active: app.currentPage === "top"
            onClicked: root.showTop()
        }
        SidebarRow {
            text: qsTr("Germany")
            code: "DE"
            active: app.currentPage === "german"
            onClicked: root.showGerman()
        }
        SidebarRow {
            text: qsTr("Netherlands")
            code: "NL"
            active: app.currentPage === "dutch"
            onClicked: root.showDutch()
        }
        SidebarRow {
            text: qsTr("World")
            iconName: "globe"
            active: app.currentPage === "world"
            onClicked: root.showWorld()
        }
        SidebarRow {
            text: qsTr("Search Results")
            iconName: "search"
            active: app.currentPage === "search"
            onClicked: app.currentPage = "search"
        }

        SidebarSectionLabel {
            text: qsTr("Library")
            Layout.topMargin: 14
        }

        SidebarRow {
            text: qsTr("My stations")
            iconName: "radio"
            active: app.currentPage === "mystations"
            badge: app.backend ? app.backend.manualStations.length : 0
            onClicked: root.showMyStations()
        }
        SidebarRow {
            text: qsTr("Favorites")
            iconName: "heart"
            active: app.currentPage === "favorites"
            badge: app.backend ? app.backend.favoriteStations.length : 0
            onClicked: root.showFavorites()
        }
        SidebarRow {
            text: qsTr("Recent")
            iconName: "clock"
            active: app.currentPage === "history"
            badge: app.backend ? app.backend.recentStations.length : 0
            onClicked: root.showHistory()
        }
        SidebarRow {
            text: qsTr("Add station")
            iconName: "plus"
            active: false
            onClicked: app.addDialog.open()
        }

        Item { Layout.fillHeight: true }

        // Footer tools
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: BearTheme.cardBorder
            opacity: 0.6
            Layout.bottomMargin: 8
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            IconButton {
                iconName: "plus"
                iconSize: 16
                implicitWidth: 40
                implicitHeight: 40
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Add Station")
                onClicked: app.addDialog.open()
            }
            IconButton {
                iconName: "info"
                iconSize: 18
                implicitWidth: 40
                implicitHeight: 40
                flat: true
                ToolTip.visible: hovered
                ToolTip.text: qsTr("About")
                onClicked: app.aboutDialog.open()
            }
            Item { Layout.fillWidth: true }
            IconButton {
                visible: app.backend && app.backend.canResumeLastStation
                iconName: "refresh"
                iconSize: 16
                implicitWidth: 40
                implicitHeight: 40
                highlighted: true
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Resume")
                onClicked: app.backend.resumeLastStation()
            }
        }
    }

    component SidebarSectionLabel: Label {
        id: sectionLabel
        Layout.fillWidth: true
        Layout.topMargin: 2
        Layout.bottomMargin: 4
        color: BearTheme.textMuted
        font.pixelSize: 10
        font.bold: true
        font.letterSpacing: 1.1
        leftPadding: 10
        // Visual style: section captions in small caps feel.
        font.capitalization: Font.AllUppercase
    }

    component SidebarRow: Item {
        id: row
        property string text: ""
        property string iconName: ""
        property string code: "" // e.g. DE / NL chip instead of icon
        property bool active: false
        property int badge: 0
        signal clicked()

        Layout.fillWidth: true
        Layout.preferredHeight: 40
        Accessible.role: Accessible.Button
        Accessible.name: text
        Accessible.onPressAction: row.clicked()

        Rectangle {
            id: bg
            anchors.fill: parent
            radius: 10
            color: row.active
                   ? BearTheme.selection
                   : (rowMouse.containsMouse ? BearTheme.cardHover : "transparent")
            border.color: row.active ? BearTheme.selectionBorder : "transparent"
            border.width: row.active ? 1 : 0
            Behavior on color { ColorAnimation { duration: 100 } }
        }

        // Active accent bar
        Rectangle {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: 3
            height: parent.height - 12
            radius: 2
            color: BearTheme.accent
            visible: row.active
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 10

            // Icon or country code chip
            Item {
                Layout.preferredWidth: 28
                Layout.preferredHeight: 28

                MediaIcon {
                    anchors.centerIn: parent
                    width: 18
                    height: 18
                    visible: row.iconName.length > 0
                    name: row.iconName
                    color: row.active ? BearTheme.accent : BearTheme.textMuted
                }

                Rectangle {
                    anchors.centerIn: parent
                    width: 26
                    height: 20
                    radius: 5
                    visible: row.code.length > 0
                    color: row.active
                           ? (BearTheme.isLight ? "#ffe4ee" : "#2a2430")
                           : (BearTheme.isLight ? "#e4e4ea" : "#1a1a1e")
                    border.color: row.active ? BearTheme.accent : "transparent"
                    border.width: row.active ? 1 : 0

                    Label {
                        anchors.centerIn: parent
                        text: row.code
                        color: row.active ? BearTheme.accent : BearTheme.textMuted
                        font.pixelSize: 9
                        font.bold: true
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                text: row.text
                color: row.active ? BearTheme.textMain : BearTheme.textMuted
                font.pixelSize: 13
                font.bold: row.active
                elide: Text.ElideRight
            }

            // Badge pill
            Rectangle {
                visible: row.badge > 0
                height: 20
                width: Math.max(20, badgeLabel.implicitWidth + 10)
                radius: 10
                color: row.active
                       ? (BearTheme.isLight ? "#ffd6e6" : "#32323a")
                       : (BearTheme.isLight ? "#e4e4ea" : "#1c1c20")

                Label {
                    id: badgeLabel
                    anchors.centerIn: parent
                    text: row.badge > 99 ? "99+" : String(row.badge)
                    color: row.active ? BearTheme.accent : BearTheme.textMuted
                    font.pixelSize: 10
                    font.bold: true
                }
            }
        }

        MouseArea {
            id: rowMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: row.clicked()
        }
    }
}
