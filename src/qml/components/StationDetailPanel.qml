// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Rectangle {
    id: root

    required property var app

    color: BearTheme.panelAlt
    border.color: BearTheme.cardBorder
    radius: 0
    readonly property int contentPadding: 16

    readonly property var station: {
        if (app.backend && app.backend.selectedStation && app.backend.selectedStation.name) {
            return app.backend.selectedStation
        }
        var model = app.activeModel ? app.activeModel() : []
        return model && model.length > 0 && model[0] ? model[0] : ({})
    }
    readonly property var player: app && app.backend ? app.backend.player : null
    readonly property string coverArtUrl: player && player.currentCoverArtUrl
                                               ? player.currentCoverArtUrl : ""
    readonly property bool hasStation: Boolean(station && station.name
                                               && station.name.length > 0)
    readonly property bool isCurrent: {
        if (!app.backend || !hasStation) return false
        var currentUuid = app.backend.currentStationUuid
        var currentUrl = app.backend.currentStationUrl
        var selectedUuid = station.uuid || ""
        var selectedUrl = station.urlResolved || station.url || ""
        if (currentUuid !== "" && selectedUuid !== "") {
            return currentUuid === selectedUuid
        }
        return currentUrl !== "" && currentUrl === selectedUrl
    }

    function streamUrl() {
        if (!hasStation) return ""
        return station.urlResolved && station.urlResolved !== "" ? station.urlResolved : (station.url || "")
    }

    function connectionStatusText() {
        if (!player) return qsTr("Inactive")
        switch (player.connectionState) {
        case "connecting": return qsTr("Connecting…")
        case "buffering": return qsTr("Buffering…")
        case "retrying": return qsTr("Reconnecting…")
        case "playing": return qsTr("Playing")
        case "paused": return qsTr("Paused")
        case "error": return qsTr("Stream unavailable")
        default: return qsTr("Inactive")
        }
    }

    ScrollView {
        id: detailScroll
        anchors.fill: parent
        clip: true
        visible: root.hasStation

        ColumnLayout {
            width: Math.max(1, detailScroll.availableWidth - (root.contentPadding * 2))
            x: root.contentPadding
            y: root.contentPadding
            spacing: 14

            Item {
                Layout.preferredWidth: 70
                Layout.preferredHeight: 70

                StationLogo {
                    anchors.fill: parent
                    app: root.app
                    stationName: root.station.name || ""
                    stationKey: root.station.uuid
                                || root.station.urlResolved
                                || root.station.url
                                || root.station.name
                                || ""
                    faviconUrl: root.station.favicon || ""
                    homepageUrl: root.station.homepage || ""
                    logoMargin: 7
                }

                Image {
                    id: detailArtwork
                    anchors.fill: parent
                    anchors.margins: 0
                    source: {
                        if (!app.backend || !root.isCurrent
                                || root.coverArtUrl === "") return ""
                        var revision = app.backend.stationImageRevision
                        if (revision < 0) return ""
                        return app.backend.stationImageSource(root.coverArtUrl)
                    }
                    fillMode: Image.PreserveAspectCrop
                    smooth: true
                    asynchronous: true
                    visible: source !== "" && status === Image.Ready
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Label {
                    Layout.fillWidth: true
                    text: root.station.name || ""
                    color: BearTheme.textMain
                    font.pixelSize: 17
                    font.bold: true
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                }
                Label {
                    Layout.fillWidth: true
                    text: root.station.country || ""
                    visible: text.length > 0
                    color: BearTheme.textMuted
                    font.pixelSize: 12
                    elide: Text.ElideRight
                }
            }

            Flow {
                Layout.fillWidth: true
                spacing: 8

                AppButton {
                    text: qsTr("▶ Play")
                    height: 30
                    onClicked: {
                        if (app.backend) {
                            app.backend.playSelectedStation()
                        }
                    }
                }
                AppButton {
                    text: Boolean(root.station.isFavorite) ? qsTr("★ Favorite") : qsTr("☆ Favorite")
                    height: 30
                    onClicked: {
                        if (!app.backend) return
                        app.backend.toggleFavoriteById(root.station.uuid || "", root.station.urlResolved || "")
                        app.toast(Boolean(root.station.isFavorite)
                                  ? qsTr("Removed from favorites")
                                  : qsTr("Added to favorites"))
                    }
                }
            }

            DetailSection {
                title: qsTr("Now Playing")
                visible: root.isCurrent
                LabeledValue {
                    label: qsTr("Status")
                    value: root.connectionStatusText()
                }
                LabeledValue {
                    visible: Boolean(root.player
                                     && root.player.currentTrackArtist
                                     && root.player.currentTrackArtist.length > 0)
                    label: qsTr("Artist")
                    value: root.player ? (root.player.currentTrackArtist || "") : ""
                }
                LabeledValue {
                    visible: Boolean(root.player
                                     && root.player.currentTrackTitle
                                     && root.player.currentTrackTitle.length > 0)
                    label: qsTr("Title")
                    value: root.player ? (root.player.currentTrackTitle || "") : ""
                }
                Label {
                    Layout.fillWidth: true
                    visible: Boolean(root.player
                                     && (root.player.currentTrackArtist || "").length === 0
                                     && (root.player.currentTrackTitle || "").length === 0)
                    text: qsTr("No stream metadata yet")
                    color: BearTheme.textMuted
                    font.pixelSize: 12
                }
            }

            DetailSection {
                title: qsTr("Details")
                LabeledValue { visible: Boolean(root.station.codec); label: qsTr("Codec"); value: (root.station.codec || "").toUpperCase() }
                LabeledValue { visible: Number(root.station.bitrate || 0) > 0; label: qsTr("Bitrate"); value: Number(root.station.bitrate || 0) > 0 ? root.station.bitrate + " kbps" : "" }
                LabeledValue { visible: Number(root.station.votes || 0) > 0; label: qsTr("Votes"); value: Number(root.station.votes || 0) > 0 ? String(root.station.votes) : "" }
                LabeledValue { visible: Boolean(root.station.tags); label: qsTr("Tags"); value: root.station.tags || "" }
            }

            DetailSection {
                title: qsTr("Links")
                AppButton {
                    visible: Boolean(root.station.homepage)
                    text: qsTr("Open Homepage")
                    onClicked: Qt.openUrlExternally(root.station.homepage)
                }
                AppButton {
                    enabled: root.streamUrl() !== ""
                    text: qsTr("Copy Stream URL")
                    onClicked: {
                        if (!app.backend) return
                        app.backend.copyToClipboard(root.streamUrl())
                        app.toast(qsTr("Stream URL copied"))
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: root.contentPadding
            }
        }
    }

    Column {
        anchors.centerIn: parent
        visible: !root.hasStation
        spacing: 8
        Label {
            text: qsTr("No station selected")
            color: BearTheme.textMain
            font.bold: true
        }
        Label {
            text: qsTr("Select a station from the list")
            color: BearTheme.textMuted
        }
    }

    component DetailSection: ColumnLayout {
        property alias title: heading.text
        Layout.fillWidth: true
        spacing: 8
        Label {
            id: heading
            Layout.fillWidth: true
            color: BearTheme.textMain
            font.pixelSize: 13
            font.bold: true
        }
    }

    component LabeledValue: RowLayout {
        property string label: ""
        property string value: ""
        Layout.fillWidth: true
        spacing: 8
        Label {
            text: parent.label
            color: BearTheme.textMain
            font.pixelSize: 12
            font.bold: true
            Layout.preferredWidth: 72
        }
        Label {
            text: parent.value
            color: BearTheme.textMuted
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }
}
