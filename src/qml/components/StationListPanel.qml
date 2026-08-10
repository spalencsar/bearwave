// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Item {
    id: root

    required property var app

    property alias stationList: stationList

    readonly property bool showEmptyState: stationList.count === 0
        && !(app.currentPage === "world" && app.selectedWorldCategory === "")

    readonly property string pageTitle: {
        switch (app.currentPage) {
        case "top": return qsTr("Top stations")
        case "german": return qsTr("Germany")
        case "dutch": return qsTr("Netherlands")
        case "world": return qsTr("World")
        case "search": return qsTr("Search results")
        case "favorites": return qsTr("Favorites")
        case "mystations": return qsTr("My stations")
        case "history": return qsTr("Recent")
        default:
            if (String(app.currentPage).indexOf("genre-") === 0
                    || String(app.currentPage).indexOf("country-") === 0)
                return qsTr("Stations")
            return qsTr("Stations")
        }
    }

    readonly property string pageSubtitle: {
        if (showEmptyState)
            return ""
        var n = stationList.count
        if (n === 1)
            return qsTr("1 station")
        return qsTr("%1 stations").arg(n)
    }

    ColumnLayout {
        anchors.fill: parent
        // Keep cards clear of left nav / right stage so borders don't collide.
        anchors.leftMargin: 8
        anchors.rightMargin: 18
        anchors.topMargin: 4
        anchors.bottomMargin: 8
        spacing: 10
        visible: !(app.currentPage === "world" && app.selectedWorldCategory === "")

        // List header
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 6
            Layout.rightMargin: 6
            spacing: 10

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    Layout.fillWidth: true
                    text: root.pageTitle
                    color: BearTheme.textMain
                    font.pixelSize: 16
                    font.bold: true
                    elide: Text.ElideRight
                }
                Label {
                    Layout.fillWidth: true
                    visible: root.pageSubtitle.length > 0
                    text: root.pageSubtitle
                    color: BearTheme.textMuted
                    font.pixelSize: 12
                }
            }

            // Loading hint
            Label {
                visible: app.backend && app.backend.loading
                text: qsTr("Loading…")
                color: BearTheme.accent
                font.pixelSize: 12
                font.bold: true
            }
        }

        // Station list
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: stationList
                anchors.fill: parent
                clip: true
                // Gap between cards — no shared edge with the stage panel.
                spacing: 10
                model: app.activeModel()
                visible: count > 0 && !root.showEmptyState
                boundsBehavior: Flickable.StopAtBounds
                // Inset content so card borders never kiss the right stage edge.
                leftMargin: 2
                rightMargin: 14
                topMargin: 2
                bottomMargin: 8
                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                    width: 8
                    padding: 2
                    contentItem: Rectangle {
                        implicitWidth: 5
                        radius: 3
                        color: BearTheme.cardBorder
                        opacity: 0.85
                    }
                }

                delegate: StationCard {
                    app: root.app
                    compactMode: root.app.compactMode
                    // Width accounts for ListView horizontal margins.
                    listWidth: Math.max(120, stationList.width - stationList.leftMargin - stationList.rightMargin)
                }

                add: Transition {
                    NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 120 }
                }
            }

            // Empty state
            ColumnLayout {
                anchors.centerIn: parent
                width: Math.min(320, parent.width - 40)
                spacing: 12
                visible: root.showEmptyState

                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 72
                    height: 72
                    radius: 36
                    color: BearTheme.imageWell
                    border.color: BearTheme.cardBorder
                    border.width: 1

                    MediaIcon {
                        anchors.centerIn: parent
                        width: 28
                        height: 28
                        name: {
                            if (app.currentPage === "favorites") return "heart"
                            if (app.currentPage === "mystations") return "radio"
                            if (app.currentPage === "history") return "clock"
                            if (app.currentPage === "search") return "search"
                            return "top"
                        }
                        color: BearTheme.textMuted
                    }
                }

                Label {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    text: {
                        if (app.currentPage === "history")
                            return qsTr("No playback history")
                        if (app.currentPage === "mystations")
                            return qsTr("No stations yet")
                        if (app.currentPage === "favorites")
                            return qsTr("No favorites yet")
                        if (app.currentPage === "search")
                            return qsTr("No results")
                        return qsTr("No stations loaded yet")
                    }
                    color: BearTheme.textMain
                    font.pixelSize: 15
                    font.bold: true
                }

                Label {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    text: {
                        if (app.currentPage === "history")
                            return qsTr("Play some stations to build history")
                        if (app.currentPage === "mystations")
                            return qsTr("Add a stream URL via + or “Add station”")
                        if (app.currentPage === "favorites")
                            return qsTr("Star stations while browsing to save them here")
                        if (app.currentPage === "search")
                            return qsTr("Try another search term")
                        return qsTr("Load DE/NL stations or use search")
                    }
                    color: BearTheme.textMuted
                    font.pixelSize: 12
                }

                AppButton {
                    Layout.alignment: Qt.AlignHCenter
                    visible: app.currentPage === "mystations"
                    text: qsTr("Add station")
                    highlighted: true
                    onClicked: app.addDialog.open()
                }
            }
        }
    }
}
