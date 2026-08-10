// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

// Shared transport with MediaIcon (no emoji).
// variant "inline" — single row (bottom PlayerBar)
// variant "stage"  — roomy stacked layout (Now Playing dock)
Item {
    id: root

    required property var app
    property string variant: "inline"
    property int controlHeight: 34
    property int controlWidth: 40

    readonly property var player: app && app.backend ? app.backend.player : null
    readonly property var backend: app ? app.backend : null
    readonly property bool canToggle: player
                                      && ["connecting", "retrying"]
                                         .indexOf(player.connectionState) === -1
    readonly property bool isMuted: player && player.volume <= 0.001

    property real lastVolume: 0.5

    function toggleMute() {
        if (!player)
            return
        if (player.volume > 0.001) {
            lastVolume = player.volume
            player.setVolume(0)
        } else {
            player.setVolume(lastVolume > 0.05 ? lastVolume : 0.5)
        }
    }

    implicitHeight: variant === "stage" ? stageCol.implicitHeight : controlHeight
    implicitWidth: 280

    // —— Inline ——
    RowLayout {
        anchors.fill: parent
        spacing: 6
        visible: root.variant === "inline"

        IconButton {
            iconName: "prev"
            iconSize: 16
            implicitWidth: root.controlWidth
            implicitHeight: root.controlHeight
            enabled: root.backend ? root.backend.hasPreviousStation() : false
            onClicked: if (root.backend) root.backend.playPreviousStation()
        }
        IconButton {
            iconName: root.player && root.player.playing ? "pause" : "play"
            iconSize: 16
            implicitWidth: root.controlWidth
            implicitHeight: root.controlHeight
            highlighted: true
            enabled: root.canToggle
            onClicked: if (root.player) root.player.togglePlayPause()
        }
        IconButton {
            iconName: "stop"
            iconSize: 14
            implicitWidth: root.controlWidth
            implicitHeight: root.controlHeight
            onClicked: if (root.player) root.player.stop()
        }
        IconButton {
            iconName: "next"
            iconSize: 16
            implicitWidth: root.controlWidth
            implicitHeight: root.controlHeight
            enabled: root.backend ? root.backend.hasNextStation() : false
            onClicked: if (root.backend) root.backend.playNextStation()
        }
        IconButton {
            iconName: root.isMuted ? "mute" : "volume"
            iconSize: 18
            implicitWidth: root.controlWidth
            implicitHeight: root.controlHeight
            ToolTip.visible: hovered
            ToolTip.text: root.isMuted ? qsTr("Unmute") : qsTr("Mute")
            onClicked: root.toggleMute()
        }
        Slider {
            Layout.fillWidth: true
            Layout.preferredHeight: root.controlHeight
            from: 0
            to: 1
            value: root.player ? root.player.volume : 0.5
            onMoved: if (root.player) root.player.setVolume(value)
        }
    }

    // —— Stage dock ——
    ColumnLayout {
        id: stageCol
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        spacing: 18
        visible: root.variant === "stage"

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            Item { Layout.fillWidth: true }

            IconButton {
                iconName: "prev"
                iconSize: 20
                implicitWidth: 48
                implicitHeight: 48
                enabled: root.backend ? root.backend.hasPreviousStation() : false
                onClicked: if (root.backend) root.backend.playPreviousStation()
            }

            IconButton {
                iconName: root.player && root.player.playing ? "pause" : "play"
                iconSize: 24
                implicitWidth: 60
                implicitHeight: 60
                round: true
                accentFill: true
                enabled: root.canToggle
                onClicked: if (root.player) root.player.togglePlayPause()
            }

            IconButton {
                iconName: "next"
                iconSize: 20
                implicitWidth: 48
                implicitHeight: 48
                enabled: root.backend ? root.backend.hasNextStation() : false
                onClicked: if (root.backend) root.backend.playNextStation()
            }

            Item { Layout.fillWidth: true }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            IconButton {
                iconName: "stop"
                iconSize: 16
                implicitWidth: 44
                implicitHeight: 40
                flat: true
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Stop")
                onClicked: if (root.player) root.player.stop()
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8

            RowLayout {
                Layout.fillWidth: true
                Label {
                    text: qsTr("Volume")
                    color: BearTheme.textMuted
                    font.pixelSize: 11
                    font.bold: true
                    font.letterSpacing: 0.6
                }
                Item { Layout.fillWidth: true }
                Label {
                    text: Math.round((root.player ? root.player.volume : 0.5) * 100) + "%"
                    color: BearTheme.textMuted
                    font.pixelSize: 12
                    font.bold: true
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                IconButton {
                    iconName: root.isMuted ? "mute" : "volume"
                    iconSize: 20
                    implicitWidth: 44
                    implicitHeight: 44
                    ToolTip.visible: hovered
                    ToolTip.text: root.isMuted ? qsTr("Unmute") : qsTr("Mute")
                    onClicked: root.toggleMute()
                }

                Slider {
                    id: volSlider
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    from: 0
                    to: 1
                    value: root.player ? root.player.volume : 0.5
                    onMoved: if (root.player) root.player.setVolume(value)

                    background: Rectangle {
                        x: volSlider.leftPadding
                        y: volSlider.topPadding + volSlider.availableHeight / 2 - height / 2
                        implicitWidth: 200
                        implicitHeight: 6
                        width: volSlider.availableWidth
                        height: 6
                        radius: 3
                        color: BearTheme.isLight ? "#e0e0e6" : "#2a2a30"
                        Rectangle {
                            width: volSlider.visualPosition * parent.width
                            height: parent.height
                            radius: 3
                            color: BearTheme.accent
                        }
                    }
                    handle: Rectangle {
                        x: volSlider.leftPadding + volSlider.visualPosition
                           * (volSlider.availableWidth - width)
                        y: volSlider.topPadding + volSlider.availableHeight / 2 - height / 2
                        width: 18
                        height: 18
                        radius: 9
                        color: volSlider.pressed ? BearTheme.accent : BearTheme.panel
                        border.color: BearTheme.accent
                        border.width: 2
                    }
                }
            }
        }
    }
}
