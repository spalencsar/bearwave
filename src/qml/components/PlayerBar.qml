// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Rectangle {
    id: root

    required property var app

    implicitHeight: 108
    radius: 12
    color: BearTheme.panel
    border.color: BearTheme.cardBorder

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 12

        Rectangle {
            Layout.preferredWidth: 88
            Layout.preferredHeight: 88
            radius: 8
            color: "#182637"
            clip: true
            border.color: BearTheme.cardBorder

            Image {
                id: coverImage
                anchors.fill: parent
                source: {
                    if (app.backend && app.backend.player && app.backend.player.currentCoverArtUrl
                            && app.backend.player.currentCoverArtUrl !== "") {
                        return app.backend.player.currentCoverArtUrl
                    }
                    if (app.backend && app.backend.currentStation && app.backend.currentStation.favicon
                            && app.backend.currentStation.favicon.startsWith("https://")) {
                        return app.backend.currentStation.favicon
                    }
                    return "qrc:/assets/app/bearwave.png"
                }
                fillMode: (app.backend && app.backend.player && app.backend.player.currentCoverArtUrl
                           && app.backend.player.currentCoverArtUrl !== "")
                          ? Image.PreserveAspectCrop : Image.PreserveAspectFit
                smooth: true
                asynchronous: true
                anchors.margins: {
                    if (app.backend && app.backend.player && app.backend.player.currentCoverArtUrl
                            && app.backend.player.currentCoverArtUrl !== "") {
                        return 0
                    }
                    if (app.backend && app.backend.currentStation && app.backend.currentStation.favicon
                            && app.backend.currentStation.favicon.startsWith("https://")) {
                        return 8
                    }
                    return 16
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 6

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Label {
                    Layout.fillWidth: true
                    text: app.backend && app.backend.player
                          ? (app.backend.player.currentStationName || qsTr("No station selected"))
                          : qsTr("No station selected")
                    color: BearTheme.textMain
                    font.pixelSize: 16
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
                text: app.backend && app.backend.player && app.backend.player.currentNowPlaying.length > 0
                      ? (qsTr("Now playing: ") + app.backend.player.currentNowPlaying)
                      : qsTr("Now playing: No track info")
                color: BearTheme.textMuted
                font.pixelSize: 12
                elide: Text.ElideRight
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Button {
                    text: "⏮"
                    Layout.preferredWidth: 44
                    enabled: app.backend ? app.backend.hasPreviousStation() : false
                    onClicked: {
                        if (app.backend) {
                            app.backend.playPreviousStation()
                        }
                    }
                }

                Button {
                    text: app.backend && app.backend.player && app.backend.player.playing ? "⏸" : "▶"
                    Layout.preferredWidth: 44
                    onClicked: {
                        if (app.backend && app.backend.player) {
                            app.backend.player.togglePlayPause()
                        }
                    }
                }

                Button {
                    text: "⏹"
                    Layout.preferredWidth: 44
                    onClicked: {
                        if (app.backend && app.backend.player) {
                            app.backend.player.stop()
                        }
                    }
                }

                Button {
                    text: "⏭"
                    Layout.preferredWidth: 44
                    enabled: app.backend ? app.backend.hasNextStation() : false
                    onClicked: {
                        if (app.backend) {
                            app.backend.playNextStation()
                        }
                    }
                }

                Button {
                    id: muteButton
                    text: (app.backend && app.backend.player && app.backend.player.volume > 0) ? "🔊" : "🔇"
                    Layout.preferredWidth: 44
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