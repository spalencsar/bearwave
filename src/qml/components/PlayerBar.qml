// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

// Bottom player bar:
// - full: logo + station meta + transport (stage hidden / narrow window)
// - transportOnly: controls only (stage still visible, too short for stage dock)
Rectangle {
    id: root

    required property var app
    // When true, stage already shows Now Playing — only transport here.
    property bool transportOnly: false
    readonly property var player: app.backend ? app.backend.player : null

    function connectionStatusText() {
        if (!player) return qsTr("Inactive")
        if (player.playing) {
            return player.currentNowPlaying.length > 0
                    ? (qsTr("Now playing: ") + player.currentNowPlaying)
                    : qsTr("Now playing: No track info")
        }
        switch (player.connectionState) {
        case "connecting": return qsTr("Connecting…")
        case "buffering": return qsTr("Buffering…")
        case "retrying": return qsTr("Reconnecting…")
        case "paused": return qsTr("Paused")
        case "error": return qsTr("Stream unavailable")
        case "idle": return qsTr("Inactive")
        case "playing":
            return player.currentNowPlaying.length > 0
                    ? (qsTr("Now playing: ") + player.currentNowPlaying)
                    : qsTr("Now playing: No track info")
        default:
            return player.currentNowPlaying.length > 0
                    ? (qsTr("Now playing: ") + player.currentNowPlaying)
                    : qsTr("Now playing: No track info")
        }
    }

    implicitHeight: transportOnly ? 64 : 96
    radius: 0
    color: BearTheme.panel
    border.color: BearTheme.cardBorder

    // —— Transport-only strip (stage open, short height) ——
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        anchors.topMargin: 10
        anchors.bottomMargin: 10
        spacing: 12
        visible: root.transportOnly

        Label {
            text: qsTr("Playback")
            color: BearTheme.textMuted
            font.pixelSize: 11
            font.bold: true
            font.letterSpacing: 0.6
        }

        PlayerTransport {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            app: root.app
            variant: "inline"
            controlHeight: 36
            controlWidth: 40
        }
    }

    // —— Full bar (no stage) ——
    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 14
        visible: !root.transportOnly

        Item {
            Layout.preferredWidth: 64
            Layout.preferredHeight: 64

            StationLogo {
                anchors.fill: parent
                app: root.app
                stationName: app.backend && app.backend.currentStation
                             ? (app.backend.currentStation.name || "") : ""
                stationKey: app.backend && app.backend.currentStation
                            ? (app.backend.currentStation.uuid
                               || app.backend.currentStation.urlResolved
                               || app.backend.currentStation.url
                               || app.backend.currentStation.name
                               || "") : ""
                faviconUrl: app.backend && app.backend.currentStation
                            ? (app.backend.currentStation.favicon || "") : ""
                homepageUrl: app.backend && app.backend.currentStation
                             ? (app.backend.currentStation.homepage || "") : ""
                logoMargin: 8
            }

            Image {
                id: coverImage
                anchors.fill: parent
                source: {
                    if (!app.backend || !app.backend.player
                            || !app.backend.player.currentCoverArtUrl
                            || app.backend.player.currentCoverArtUrl === "") return ""
                    var revision = app.backend.stationImageRevision
                    if (revision < 0) return ""
                    return app.backend.stationImageSource(
                                app.backend.player.currentCoverArtUrl)
                }
                fillMode: Image.PreserveAspectCrop
                smooth: true
                asynchronous: true
                visible: source !== "" && status === Image.Ready
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 4

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Label {
                    Layout.fillWidth: true
                    text: {
                        if (!app.backend || !app.backend.player)
                            return qsTr("No station selected")
                        var name = app.backend.player.currentStationName || ""
                        if (name.length === 0)
                            return qsTr("No station selected")
                        return name
                    }
                    color: BearTheme.textMain
                    font.pixelSize: 14
                    font.bold: true
                    elide: Text.ElideRight
                }

                Rectangle {
                    visible: app.backend && app.backend.currentStation && app.backend.currentStation.codec
                             && app.backend.currentStation.codec !== "unknown"
                             && app.backend.currentStation.codec !== ""
                    height: 18
                    width: codecLabel.implicitWidth + 12
                    radius: 4
                    color: "transparent"
                    border.color: BearTheme.accent
                    border.width: 1

                    Label {
                        id: codecLabel
                        anchors.centerIn: parent
                        text: (app.backend && app.backend.currentStation && app.backend.currentStation.codec)
                              ? app.backend.currentStation.codec.toUpperCase() : ""
                        color: BearTheme.accent
                        font.pixelSize: 9
                        font.bold: true
                    }
                }

                Rectangle {
                    visible: app.backend && app.backend.currentStation && app.backend.currentStation.bitrate > 0
                    height: 18
                    width: bitrateLabel.implicitWidth + 12
                    radius: 4
                    color: BearTheme.accent
                    border.color: BearTheme.accent
                    border.width: 1

                    Label {
                        id: bitrateLabel
                        anchors.centerIn: parent
                        text: (app.backend && app.backend.currentStation)
                              ? app.backend.currentStation.bitrate + " kbps" : ""
                        color: "#ffffff"
                        font.pixelSize: 9
                        font.bold: true
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                text: root.connectionStatusText()
                color: root.player && root.player.connectionState === "error"
                       ? BearTheme.warn
                       : (root.player && root.player.playing
                          ? BearTheme.textMuted
                          : (root.player
                             && ["connecting", "buffering", "retrying"]
                                .indexOf(root.player.connectionState) !== -1
                             ? BearTheme.accent : BearTheme.textMuted))
                font.pixelSize: 11
                elide: Text.ElideRight
            }

            PlayerTransport {
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                app: root.app
                variant: "inline"
                controlHeight: 30
                controlWidth: 38
            }
        }
    }
}
