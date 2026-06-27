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

    height: 52
    width: listWidth
    radius: 0

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
        ? "#4a4a4d"
        : (cardMouse.containsMouse ? "#2d2e34" : "transparent")

    border.width: 0

    readonly property string metadataText: {
        if (!modelData) return ""
        var parts = []
        if (modelData.country) parts.push(modelData.country)
        if (modelData.codec && modelData.codec !== "unknown" && modelData.codec !== "") parts.push(modelData.codec.toUpperCase())
        if (modelData.bitrate > 0) parts.push(modelData.bitrate + " kbps")
        if (modelData.votes > 0) {
            parts.push(modelData.votes.toLocaleString(Qt.locale(), "f", 0) + " " + qsTr("votes"))
        } else {
            parts.push(qsTr("Stream"))
        }
        return parts.join("  •  ")
    }

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
            app.selectedStation = root.modelData
            if (!app.backend) return
            if (root.isCurrent) {
                app.backend.player.togglePlayPause()
            } else {
                playStation()
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: BearTheme.cardBorder
        opacity: 0.65
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        spacing: 10

        // Favicon Rect
        Rectangle {
            Layout.preferredWidth: 34
            Layout.preferredHeight: 34
            radius: 5
            color: "#34353b"
            clip: true

            Image {
                id: stationFavicon
                anchors.fill: parent
                anchors.margins: 2
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

        // Info Column
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 1
            Layout.alignment: Qt.AlignVCenter

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Label {
                    Layout.fillWidth: true
                    text: root.modelData.name
                    color: root.isCurrent ? BearTheme.accent : BearTheme.textMain
                    font.bold: true
                    font.pixelSize: 13
                    elide: Text.ElideRight
                }

                // Tiny EQ Indicator
                Row {
                    id: eqAnimation
                    spacing: 2
                    Layout.alignment: Qt.AlignVCenter
                    visible: root.isPlaying

                    Rectangle {
                        id: bar1
                        width: 2
                        height: 10
                        color: BearTheme.playingAccent
                        radius: 1
                        Behavior on height { NumberAnimation { duration: 120 } }
                    }
                    Rectangle {
                        id: bar2
                        width: 2
                        height: 10
                        color: BearTheme.playingAccent
                        radius: 1
                        Behavior on height { NumberAnimation { duration: 120 } }
                    }
                    Rectangle {
                        id: bar3
                        width: 2
                        height: 10
                        color: BearTheme.playingAccent
                        radius: 1
                        Behavior on height { NumberAnimation { duration: 120 } }
                    }

                    Timer {
                        interval: 150
                        running: root.isPlaying
                        repeat: true
                        onTriggered: {
                            bar1.height = Math.floor(Math.random() * 9) + 2
                            bar2.height = Math.floor(Math.random() * 9) + 2
                            bar3.height = Math.floor(Math.random() * 9) + 2
                        }
                    }

                    onVisibleChanged: {
                        if (!visible) {
                            bar1.height = 10
                            bar2.height = 10
                            bar3.height = 10
                        }
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                text: root.metadataText
                color: BearTheme.textMuted
                font.pixelSize: 11
                elide: Text.ElideRight
            }
        }

        // Edit Button (Custom User Stations Only)
        Button {
            visible: !root.modelData.uuid
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32
            text: "✏"
            flat: true
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Edit Station")
            onClicked: {
                app.editDialog.stationObject = root.modelData
                app.editDialog.setupAndOpen()
            }
        }

        // Favorite Button
        Button {
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32
            text: root.modelData.isFavorite ? "★" : "☆"
            flat: true
            font.pixelSize: 14
            onClicked: {
                if (!app.backend) return
                app.backend.toggleFavoriteById(root.modelData.uuid, root.modelData.urlResolved)
                app.toast(root.modelData.isFavorite ? qsTr("Removed from favorites") : qsTr("Added to favorites"))
            }
        }

        // Play/Pause Button
        Button {
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32
            text: (root.isCurrent && app.backend && app.backend.player && app.backend.player.playing) ? "⏸" : "▶"
            flat: true
            font.pixelSize: 13
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
