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

    height: compactMode ? 58 : 52
    width: listWidth
    radius: 6

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
    readonly property bool isSelected: {
        if (!app.backend || !app.backend.selectedStation || !modelData) return false
        var selectedUuid = app.backend.selectedStation.uuid || ""
        var selectedUrl = app.backend.selectedStation.urlResolved || app.backend.selectedStation.url || ""
        var cardUuid = modelData.uuid || ""
        var cardUrl = modelData.urlResolved || modelData.url || ""
        if (selectedUuid !== "" && cardUuid !== "") {
            return selectedUuid === cardUuid
        }
        return selectedUrl !== "" && selectedUrl === cardUrl
    }

    color: isSelected
        ? (cardMouse.containsMouse ? BearTheme.selectionHover : BearTheme.selection)
        : (cardMouse.containsMouse ? BearTheme.cardHover : "transparent")
    border.color: isCurrent ? BearTheme.accent : (isSelected ? BearTheme.selectionBorder : BearTheme.cardBorder)
    border.width: isCurrent || isSelected ? 1 : 0

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

    function selectStation() {
        if (!app.backend) return
        if (app.currentPage === "favorites") {
            app.backend.selectFavoriteStation(index)
        } else if (app.currentPage === "history") {
            app.backend.selectRecentByUuid(modelData.uuid, modelData.urlResolved)
        } else {
            app.backend.selectStation(index)
        }
    }

    MouseArea {
        id: cardMouse
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        onClicked: {
            selectStation()
        }
        onDoubleClicked: {
            playStation()
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 8
        anchors.topMargin: 5
        anchors.bottomMargin: 5
        spacing: 10

        StationLogo {
            Layout.preferredWidth: compactMode ? 38 : 34
            Layout.preferredHeight: compactMode ? 38 : 34
            app: root.app
            stationName: root.modelData.name || ""
            stationKey: root.modelData.uuid
                        || root.modelData.urlResolved
                        || root.modelData.url
                        || root.modelData.name
                        || ""
            faviconUrl: root.modelData.favicon || ""
            homepageUrl: root.modelData.homepage || ""
            logoMargin: 3
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
                    font.pixelSize: compactMode ? 13 : 13
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

        AppButton {
            visible: !root.modelData.uuid
            Layout.preferredWidth: compactMode ? 32 : 34
            Layout.preferredHeight: 28
            flat: true
            text: "✎"
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Edit Station")
            onClicked: {
                app.editDialog.stationObject = root.modelData
                app.editDialog.setupAndOpen()
            }
        }

        AppButton {
            Layout.preferredWidth: compactMode ? 32 : 34
            Layout.preferredHeight: 28
            flat: true
            text: root.modelData.isFavorite ? "★" : "☆"
            onClicked: {
                if (!app.backend) return
                app.backend.toggleFavoriteById(root.modelData.uuid, root.modelData.urlResolved)
                app.toast(root.modelData.isFavorite ? qsTr("Removed from favorites") : qsTr("Added to favorites"))
            }
        }

        AppButton {
            Layout.preferredWidth: compactMode ? 32 : 34
            Layout.preferredHeight: 28
            flat: true
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
