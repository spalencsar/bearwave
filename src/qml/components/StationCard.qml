// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Rectangle {
    id: root

    required property int index
    required property var modelData
    required property var app
    required property bool compactMode
    required property real listWidth

    height: compactMode ? 72 : 78
    width: listWidth
    radius: 10

    readonly property bool isCurrent: {
        if (!app.backend || !modelData) return false
        var currentUuid = app.backend.currentStationUuid
        var currentUrl = app.backend.currentStationUrl
        var cardUuid = modelData.uuid || ""
        var cardUrl = modelData.urlResolved || modelData.url || ""
        if (currentUuid !== "" && cardUuid !== "") {
            return currentUuid === cardUuid
        }
        return currentUrl !== "" && currentUrl === cardUrl
    }
    readonly property bool isPlaying: isCurrent && app.backend && app.backend.player && app.backend.player.playing

    color: isCurrent
        ? (cardMouse.containsMouse ? "#1d3350" : "#16283e")
        : (cardMouse.containsMouse ? BearTheme.cardHover : BearTheme.card)
    border.color: isCurrent ? BearTheme.accent : BearTheme.cardBorder
    border.width: isCurrent ? 2 : 1

    function playStation() {
        if (!app.backend) return
        if (app.currentPage === "favorites") {
            app.backend.playFavoriteStation(index)
        } else if (app.currentPage === "history") {
            app.backend.playRecentByUuid(modelData.uuid, modelData.urlResolved)
        } else {
            app.backend.playStation(index)
        }
    }

    MouseArea {
        id: cardMouse
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        onClicked: {
            if (!app.backend) return
            if (root.isCurrent) {
                app.backend.player.togglePlayPause()
            } else {
                playStation()
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Rectangle {
            Layout.preferredWidth: 44
            Layout.preferredHeight: 44
            radius: 8
            color: "#123154"
            border.color: BearTheme.accent

            Image {
                id: stationFavicon
                anchors.fill: parent
                anchors.margins: 3
                source: (root.modelData.favicon && root.modelData.favicon.startsWith("https://"))
                        ? root.modelData.favicon
                        : ""
                fillMode: Image.PreserveAspectFit
                asynchronous: true
                cache: true
                visible: source !== "" && status === Image.Ready
            }

            Image {
                anchors.fill: parent
                anchors.margins: 6
                source: "qrc:/assets/app/bearwave.png"
                fillMode: Image.PreserveAspectFit
                smooth: true
                visible: !stationFavicon.visible
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Label {
                    Layout.fillWidth: true
                    text: root.modelData.name
                    color: root.isCurrent ? BearTheme.accent : BearTheme.textMain
                    font.bold: true
                    font.pixelSize: compactMode ? 13 : 14
                    elide: Text.ElideRight
                }

                Row {
                    id: eqAnimation
                    spacing: 2
                    Layout.alignment: Qt.AlignVCenter
                    visible: root.isPlaying

                    Rectangle {
                        id: bar1
                        width: 2
                        height: 12
                        color: BearTheme.accent
                        radius: 1
                        Behavior on height {
                            NumberAnimation { duration: 120 }
                        }
                    }
                    Rectangle {
                        id: bar2
                        width: 2
                        height: 12
                        color: BearTheme.accent
                        radius: 1
                        Behavior on height {
                            NumberAnimation { duration: 120 }
                        }
                    }
                    Rectangle {
                        id: bar3
                        width: 2
                        height: 12
                        color: BearTheme.accent
                        radius: 1
                        Behavior on height {
                            NumberAnimation { duration: 120 }
                        }
                    }

                    Timer {
                        interval: 150
                        running: root.isPlaying
                        repeat: true
                        onTriggered: {
                            bar1.height = Math.floor(Math.random() * 11) + 3
                            bar2.height = Math.floor(Math.random() * 11) + 3
                            bar3.height = Math.floor(Math.random() * 11) + 3
                        }
                    }

                    onVisibleChanged: {
                        if (!visible) {
                            bar1.height = 12
                            bar2.height = 12
                            bar3.height = 12
                        }
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                text: root.modelData.country + "  •  "
                      + (root.modelData.codec && root.modelData.codec !== "unknown" && root.modelData.codec !== ""
                         ? root.modelData.codec.toUpperCase() + "  •  " : "")
                      + (root.modelData.bitrate > 0 ? root.modelData.bitrate + " kbps" : qsTr("Stream"))
                color: BearTheme.textMuted
                font.pixelSize: 11
                elide: Text.ElideRight
            }
        }

        Button {
            visible: !root.modelData.uuid
            Layout.preferredWidth: compactMode ? 34 : 40
            Layout.preferredHeight: 40
            text: "✏"
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Edit Station")
            onClicked: {
                app.editDialog.stationObject = root.modelData
                app.editDialog.setupAndOpen()
            }
        }

        Button {
            Layout.preferredWidth: compactMode ? 34 : 40
            Layout.preferredHeight: 40
            text: root.modelData.isFavorite ? "★" : "☆"
            onClicked: {
                if (!app.backend) return
                app.backend.toggleFavoriteById(root.modelData.uuid, root.modelData.urlResolved)
                app.toast(root.modelData.isFavorite ? qsTr("Removed from favorites") : qsTr("Added to favorites"))
            }
        }

        Button {
            Layout.preferredWidth: compactMode ? 34 : 40
            Layout.preferredHeight: 40
            text: (root.isCurrent && app.backend && app.backend.player && app.backend.player.playing) ? "⏸" : "▶"
            onClicked: {
                if (!app.backend) return
                if (root.isCurrent) {
                    app.backend.player.togglePlayPause()
                } else {
                    playStation()
                }
            }
        }
    }
}