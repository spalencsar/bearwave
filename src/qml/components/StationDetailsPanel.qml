// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Rectangle {
    id: root

    required property var app

    color: BearTheme.panel
    border.color: "transparent"
    border.width: 0
    radius: 0

    readonly property var activeStation: app.selectedStation || (app.backend ? app.backend.currentStation : null)
    readonly property bool isCurrentPlaying: {
        if (!activeStation || !app.backend) return false
        var currentUuid = app.backend.currentStationUuid
        var currentUrl = app.backend.currentStationUrl
        var activeUuid = activeStation.uuid || ""
        var activeUrl = activeStation.urlResolved || activeStation.url || ""
        if (currentUuid !== "" && activeUuid !== "") {
            return currentUuid === activeUuid
        }
        return currentUrl !== "" && currentUrl === activeUrl
    }
    readonly property bool isPlaying: isCurrentPlaying && app.backend && app.backend.player && app.backend.player.playing

    // Helper text edit to facilitate clipboard copying
    TextEdit {
        id: clipboardHelper
        visible: false
        selectByMouse: true
    }

    function copyToClipboard(text) {
        clipboardHelper.text = text
        clipboardHelper.selectAll()
        clipboardHelper.copy()
        app.toast(qsTr("Stream URL copied!"))
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10
        visible: root.activeStation !== null

        ScrollView {
            id: detailsScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            ColumnLayout {
                width: detailsScroll.availableWidth
                spacing: 10

                // Cover Art / Favicon
                Rectangle {
                    Layout.alignment: Qt.AlignLeft
                    Layout.preferredWidth: 72
                    Layout.preferredHeight: 72
                    radius: 8
                    color: "#34353b"
                    border.color: "transparent"
                    clip: true

                    Image {
                        id: detailFavicon
                        anchors.fill: parent
                        anchors.margins: {
                            if (app.backend && app.backend.player && app.backend.player.currentCoverArtUrl && root.isCurrentPlaying) {
                                return 0
                            }
                            return activeStation && activeStation.favicon && activeStation.favicon.startsWith("https://") ? 8 : 16
                        }
                        source: {
                            if (!root.activeStation) return ""
                            if (root.isCurrentPlaying && app.backend && app.backend.player && app.backend.player.currentCoverArtUrl && app.backend.player.currentCoverArtUrl !== "") {
                                return app.backend.player.currentCoverArtUrl
                            }
                            return (root.activeStation.favicon && root.activeStation.favicon.startsWith("https://")) ? root.activeStation.favicon : ""
                        }
                        fillMode: (root.isCurrentPlaying && app.backend && app.backend.player && app.backend.player.currentCoverArtUrl && app.backend.player.currentCoverArtUrl !== "") ? Image.PreserveAspectCrop : Image.PreserveAspectFit
                        asynchronous: true
                        cache: true
                        visible: source !== "" && status === Image.Ready
                    }

                    Image {
                        anchors.fill: parent
                        anchors.margins: 16
                        source: "qrc:/assets/app/bearwave.png"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                        visible: !detailFavicon.visible
                    }
                }

                // Station Name & Country
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    Layout.alignment: Qt.AlignLeft

                    Label {
                        text: root.activeStation ? root.activeStation.name : ""
                        color: BearTheme.textMain
                        font.bold: true
                        font.pixelSize: 16
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    Label {
                        text: root.activeStation ? root.activeStation.country : ""
                        color: BearTheme.textMuted
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignLeft
                        Layout.fillWidth: true
                    }
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: 6

                    Button {
                        text: root.isPlaying ? "⏸  " + qsTr("Pause") : "▶  " + qsTr("Play")
                        highlighted: true
                        width: 86
                        height: 28
                        font.pixelSize: 11
                        onClicked: {
                            if (!app.backend || !root.activeStation) return
                            if (root.isCurrentPlaying) {
                                app.backend.player.togglePlayPause()
                            } else {
                                // Find index or play by url
                                app.backend.playRecentByUuid(root.activeStation.uuid, root.activeStation.urlResolved)
                            }
                        }
                    }

                    Button {
                        text: root.activeStation && root.activeStation.isFavorite ? "★  " + qsTr("Favorite") : "☆  " + qsTr("Add")
                        width: 104
                        height: 28
                        font.pixelSize: 11
                        onClicked: {
                            if (!app.backend || !root.activeStation) return
                            app.backend.toggleFavoriteById(root.activeStation.uuid, root.activeStation.urlResolved)
                            app.toast(root.activeStation.isFavorite ? qsTr("Removed from favorites") : qsTr("Added to favorites"))
                        }
                    }

                    Button {
                        text: "📁  " + qsTr("Groups")
                        width: 92
                        height: 28
                        font.pixelSize: 11
                        onClicked: app.toast(qsTr("Groups coming soon"))
                    }
                }

                // Separator
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: BearTheme.cardBorder
                    opacity: 0.6
                }

                // Technical Specifications
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        text: qsTr("Details")
                        color: BearTheme.textMuted
                        font.bold: true
                        font.pixelSize: 10
                    }

                    GridLayout {
                        columns: 2
                        columnSpacing: 16
                        rowSpacing: 8

                        Label {
                            text: qsTr("Codec:")
                            color: BearTheme.textMuted
                            font.pixelSize: 12
                        }
                        Label {
                            text: (root.activeStation && root.activeStation.codec && root.activeStation.codec !== "unknown") ? root.activeStation.codec.toUpperCase() : qsTr("Unknown")
                            color: BearTheme.textMain
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Label {
                            text: qsTr("Bitrate:")
                            color: BearTheme.textMuted
                            font.pixelSize: 12
                        }
                        Label {
                            text: (root.activeStation && root.activeStation.bitrate > 0) ? root.activeStation.bitrate + " kbps" : qsTr("Unknown")
                            color: BearTheme.textMain
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Label {
                            text: qsTr("Votes:")
                            color: BearTheme.textMuted
                            font.pixelSize: 12
                        }
                        Label {
                            text: root.activeStation ? "" + root.activeStation.votes : "0"
                            color: BearTheme.textMain
                            font.pixelSize: 12
                            font.bold: true
                        }
                    }
                }

                // Separator
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: BearTheme.cardBorder
                    opacity: 0.6
                }

                // Tags / Genres
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        text: qsTr("Tags")
                        color: BearTheme.textMuted
                        font.bold: true
                        font.pixelSize: 10
                    }

                    Flow {
                        Layout.fillWidth: true
                        spacing: 6

                        Repeater {
                            model: (root.activeStation && root.activeStation.tags) ? root.activeStation.tags.split(",") : []
                            delegate: Rectangle {
                                visible: modelData.trim().length > 0
                                height: tagText.implicitHeight + 8
                                width: tagText.implicitWidth + 12
                                radius: 4
                                color: BearTheme.card
                                border.color: BearTheme.cardBorder
                                border.width: 1

                                Label {
                                    id: tagText
                                    anchors.centerIn: parent
                                    text: modelData.trim()
                                    color: BearTheme.textMuted
                                    font.pixelSize: 10
                                }
                            }
                        }
                    }
                }

                // Separator
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: BearTheme.cardBorder
                    opacity: 0.6
                }

                // Links / URLs
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        text: qsTr("Links")
                        color: BearTheme.textMuted
                        font.bold: true
                        font.pixelSize: 10
                    }

                    Button {
                        text: "🔗  " + qsTr("Open homepage")
                        Layout.fillWidth: true
                        enabled: root.activeStation && root.activeStation.homepage && root.activeStation.homepage.startsWith("http")
                        onClicked: {
                            if (root.activeStation && root.activeStation.homepage) {
                                Qt.openUrlExternally(root.activeStation.homepage)
                            }
                        }
                    }

                    Button {
                        text: "📋  " + qsTr("Copy stream URL")
                        Layout.fillWidth: true
                        enabled: root.activeStation && (root.activeStation.urlResolved || root.activeStation.url)
                        onClicked: {
                            if (root.activeStation) {
                                var url = root.activeStation.urlResolved || root.activeStation.url
                                root.copyToClipboard(url)
                            }
                        }
                    }
                }
            }
        }
    }

    // Empty state if no station is active
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 12
        visible: root.activeStation === null

        Label {
            text: "📻"
            font.pixelSize: 48
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: qsTr("No station selected")
            color: BearTheme.textMain
            font.bold: true
            font.pixelSize: 14
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: qsTr("Select a station from the list\nto view details.")
            color: BearTheme.textMuted
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
