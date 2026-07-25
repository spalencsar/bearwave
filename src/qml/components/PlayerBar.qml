// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Rectangle {
    id: root

    required property var app
    readonly property var player: app.backend ? app.backend.player : null

    function connectionStatusText() {
        if (!player) return qsTr("Inactive")
        switch (player.connectionState) {
        case "connecting": return qsTr("Connecting…")
        case "buffering": return qsTr("Buffering…")
        case "retrying": return qsTr("Reconnecting…")
        case "paused": return qsTr("Paused")
        case "error": return qsTr("Stream unavailable")
        case "idle": return qsTr("Inactive")
        default:
            return player.currentNowPlaying.length > 0
                    ? (qsTr("Now playing: ") + player.currentNowPlaying)
                    : qsTr("Now playing: No track info")
        }
    }

    implicitHeight: 96
    radius: 0
    color: BearTheme.panel
    border.color: BearTheme.cardBorder

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 14

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
                    text: app.backend && app.backend.player
                          ? (app.backend.player.currentStationName || qsTr("No station selected"))
                          : qsTr("No station selected")
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
                       : (root.player
                          && ["connecting", "buffering", "retrying"]
                             .indexOf(root.player.connectionState) !== -1
                          ? BearTheme.accent : BearTheme.textMuted)
                font.pixelSize: 11
                elide: Text.ElideRight
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                AppButton {
                    text: "⏮"
                    Layout.preferredWidth: 38
                    Layout.preferredHeight: 30
                    enabled: app.backend ? app.backend.hasPreviousStation() : false
                    onClicked: {
                        if (app.backend) {
                            app.backend.playPreviousStation()
                        }
                    }
                }

                AppButton {
                    text: app.backend && app.backend.player && app.backend.player.playing ? "⏸" : "▶"
                    Layout.preferredWidth: 38
                    Layout.preferredHeight: 30
                    enabled: root.player
                             && ["connecting", "retrying"]
                                .indexOf(root.player.connectionState) === -1
                    onClicked: {
                        if (app.backend && app.backend.player) {
                            app.backend.player.togglePlayPause()
                        }
                    }
                }

                AppButton {
                    text: "⏹"
                    Layout.preferredWidth: 38
                    Layout.preferredHeight: 30
                    onClicked: {
                        if (app.backend && app.backend.player) {
                            app.backend.player.stop()
                        }
                    }
                }

                AppButton {
                    text: "⏭"
                    Layout.preferredWidth: 38
                    Layout.preferredHeight: 30
                    enabled: app.backend ? app.backend.hasNextStation() : false
                    onClicked: {
                        if (app.backend) {
                            app.backend.playNextStation()
                        }
                    }
                }

                AppButton {
                    id: muteButton
                    text: (app.backend && app.backend.player && app.backend.player.volume > 0) ? "🔊" : "🔇"
                    Layout.preferredWidth: 38
                    Layout.preferredHeight: 30
                    property real lastVolume: 0.5

                    ToolTip.visible: hovered
                    ToolTip.text: (app.backend && app.backend.player && app.backend.player.volume > 0)
                                  ? qsTr("Mute") : qsTr("Unmute")

                    onClicked: {
                        if (app.backend && app.backend.player) {
                            if (app.backend.player.volume > 0) {
                                lastVolume = app.backend.player.volume
                                app.backend.player.setVolume(0)
                            } else {
                                app.backend.player.setVolume(lastVolume > 0 ? lastVolume : 0.5)
                            }
                        }
                    }
                }

                Slider {
                    Layout.fillWidth: true
                    from: 0
                    to: 1
                    value: app.backend && app.backend.player ? app.backend.player.volume : 0.5
                    onMoved: {
                        if (app.backend && app.backend.player) {
                            app.backend.player.setVolume(value)
                        }
                    }
                }
            }
        }
    }
}
