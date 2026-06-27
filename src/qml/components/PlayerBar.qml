// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Rectangle {
    id: root

    required property var app

    implicitHeight: 76
    radius: 0
    color: BearTheme.panel
    border.color: "transparent"

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 1
        color: BearTheme.cardBorder
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        anchors.topMargin: 9
        anchors.bottomMargin: 9
        spacing: 12

        Rectangle {
            Layout.preferredWidth: 56
            Layout.preferredHeight: 56
            radius: 6
            color: "#34353b"
            clip: true
            border.color: "transparent"

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
                    return 10
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: 3

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

            Label {
                Layout.fillWidth: true
                text: app.backend && app.backend.player && app.backend.player.currentNowPlaying.length > 0
                      ? (qsTr("Now playing: ") + app.backend.player.currentNowPlaying)
                      : qsTr("Live radio")
                color: BearTheme.textMuted
                font.pixelSize: 12
                elide: Text.ElideRight
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignVCenter
            spacing: 8

            Button {
                text: "⏮"
                Layout.preferredWidth: 42
                Layout.preferredHeight: 30
                enabled: app.backend ? app.backend.hasPreviousStation() : false
                onClicked: {
                    if (app.backend) {
                        app.backend.playPreviousStation()
                    }
                }
            }

            Button {
                text: app.backend && app.backend.player && app.backend.player.playing ? "⏸" : "▶"
                Layout.preferredWidth: 42
                Layout.preferredHeight: 30
                onClicked: {
                    if (app.backend && app.backend.player) {
                        app.backend.player.togglePlayPause()
                    }
                }
            }

            Button {
                text: "⏹"
                Layout.preferredWidth: 42
                Layout.preferredHeight: 30
                onClicked: {
                    if (app.backend && app.backend.player) {
                        app.backend.player.stop()
                    }
                }
            }

            Button {
                text: "⏭"
                Layout.preferredWidth: 42
                Layout.preferredHeight: 30
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
                Layout.preferredWidth: 36
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
                Layout.preferredWidth: app.width < 1180 ? 115 : 160
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
