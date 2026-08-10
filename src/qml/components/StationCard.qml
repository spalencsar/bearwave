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

    width: listWidth
    height: compactMode ? 72 : 70
    radius: 12

    readonly property bool isCurrent: {
        if (!app.backend || !modelData) return false
        var currentUuid = app.backend.currentStationUuid
        var currentUrl = app.backend.currentStationUrl
        var cardUuid = modelData.uuid || ""
        var cardUrl = modelData.urlResolved || modelData.url || ""
        if (currentUuid !== "" && cardUuid !== "")
            return currentUuid === cardUuid
        return currentUrl !== "" && currentUrl === cardUrl
    }
    readonly property bool isPlaying: isCurrent && app.backend && app.backend.player && app.backend.player.playing
    readonly property bool isSelected: {
        if (!app.backend || !app.backend.selectedStation || !modelData) return false
        var selectedUuid = app.backend.selectedStation.uuid || ""
        var selectedUrl = app.backend.selectedStation.urlResolved || app.backend.selectedStation.url || ""
        var cardUuid = modelData.uuid || ""
        var cardUrl = modelData.urlResolved || modelData.url || ""
        if (selectedUuid !== "" && cardUuid !== "")
            return selectedUuid === cardUuid
        return selectedUrl !== "" && selectedUrl === cardUrl
    }
    readonly property bool isManualPage: app.currentPage === "mystations"
    readonly property string countryText: modelData && modelData.country ? String(modelData.country) : ""
    readonly property string codecText: {
        if (!modelData || !modelData.codec) return ""
        var c = String(modelData.codec)
        if (c === "" || c === "unknown") return ""
        return c.toUpperCase()
    }
    readonly property int bitrateValue: modelData ? Number(modelData.bitrate || 0) : 0
    readonly property string primaryTag: {
        if (!modelData || !modelData.tags) return ""
        var parts = String(modelData.tags).split(/[,;]/)
        for (var i = 0; i < parts.length; ++i) {
            var t = parts[i].trim()
            if (t.length > 0)
                return t
        }
        return ""
    }

    // Quiet base; only selection/hover/current draw a strong frame.
    color: {
        if (isSelected)
            return cardMouse.containsMouse ? BearTheme.selectionHover : BearTheme.selection
        if (cardMouse.containsMouse)
            return BearTheme.cardHover
        return BearTheme.isLight ? "#fafafb" : "#121214"
    }
    // Avoid a full-width “box outline” that collides with the right stage edge:
    // idle cards = no border; selected = soft; current/playing = accent.
    border.color: {
        if (isCurrent)
            return BearTheme.accent
        if (isSelected)
            return BearTheme.selectionBorder
        return "transparent"
    }
    border.width: (isCurrent || isSelected) ? 1 : 0
    opacity: 1.0

    Behavior on color { ColorAnimation { duration: 100 } }

    // Soft bottom separator for idle cards (no full box border).
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 56
        anchors.rightMargin: 12
        height: 1
        color: BearTheme.cardBorder
        opacity: 0.35
        visible: !root.isSelected && !root.isCurrent && !cardMouse.containsMouse
    }

    // Current / selected accent rail (left only — never a right edge against the stage).
    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 7
        width: 3
        radius: 2
        color: BearTheme.accent
        visible: root.isCurrent || root.isSelected
        opacity: root.isCurrent ? 1.0 : 0.5
    }

    function playStation() {
        if (!app.backend) return
        if (app.currentPage === "favorites") {
            app.backend.playFavoriteStation(index)
        } else if (app.currentPage === "mystations") {
            app.backend.playManualStation(index)
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
        } else if (app.currentPage === "mystations") {
            app.backend.selectManualStation(index)
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
        // Leave room for action buttons
        anchors.rightMargin: actionsRow.width + 12
        onClicked: {
            selectStation()
            // Single click also starts playback — faster radio UX
            if (!root.isPlaying)
                playStation()
        }
        onDoubleClicked: playStation()
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 14
        anchors.rightMargin: 10
        anchors.topMargin: 10
        anchors.bottomMargin: 10
        spacing: 12

        StationLogo {
            Layout.preferredWidth: 48
            Layout.preferredHeight: 48
            app: root.app
            stationName: root.modelData.name || ""
            stationKey: root.modelData.uuid
                        || root.modelData.urlResolved
                        || root.modelData.url
                        || root.modelData.name
                        || ""
            faviconUrl: root.modelData.favicon || ""
            homepageUrl: root.modelData.homepage || ""
            logoMargin: 2
            radius: 10
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 5

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Label {
                    Layout.fillWidth: true
                    text: root.modelData.name || ""
                    color: root.isCurrent ? BearTheme.accent : BearTheme.textMain
                    font.bold: true
                    font.pixelSize: 14
                    elide: Text.ElideRight
                }

                // Playing equalizer
                Row {
                    id: eqAnimation
                    spacing: 2
                    Layout.alignment: Qt.AlignVCenter
                    visible: root.isPlaying
                    height: 14

                    Repeater {
                        model: 3
                        Rectangle {
                            width: 2.5
                            height: 8
                            radius: 1
                            color: BearTheme.accent
                            property int barIndex: index

                            SequentialAnimation on height {
                                running: root.isPlaying
                                loops: Animation.Infinite
                                NumberAnimation {
                                    to: 4 + (index * 3)
                                    duration: 180 + index * 40
                                    easing.type: Easing.InOutSine
                                }
                                NumberAnimation {
                                    to: 12 - index
                                    duration: 200 + index * 30
                                    easing.type: Easing.InOutSine
                                }
                            }
                        }
                    }
                }

                Label {
                    visible: root.isPlaying
                    text: qsTr("Live")
                    color: BearTheme.accent
                    font.pixelSize: 10
                    font.bold: true
                }
            }

            // Meta chips
            Flow {
                Layout.fillWidth: true
                spacing: 6

                MetaChip {
                    visible: root.countryText.length > 0
                    text: root.countryText
                }
                MetaChip {
                    visible: root.primaryTag.length > 0
                    text: root.primaryTag
                    accent: false
                }
                MetaChip {
                    visible: root.codecText.length > 0
                    text: root.codecText
                }
                MetaChip {
                    visible: root.bitrateValue > 0
                    text: root.bitrateValue + " kbps"
                    filled: true
                }
                MetaChip {
                    visible: root.countryText.length === 0
                             && root.codecText.length === 0
                             && root.bitrateValue <= 0
                             && root.primaryTag.length === 0
                    text: qsTr("Stream")
                }
            }
        }

        RowLayout {
            id: actionsRow
            spacing: 4
            Layout.alignment: Qt.AlignVCenter
            opacity: cardMouse.containsMouse || root.isSelected || root.isCurrent || root.isManualPage ? 1.0 : 0.55
            Behavior on opacity { NumberAnimation { duration: 100 } }

            AppButton {
                visible: root.isManualPage
                         || !(root.modelData.uuid && String(root.modelData.uuid).length > 0)
                Layout.preferredWidth: 34
                Layout.preferredHeight: 34
                flat: true
                text: "✎"
                font.pixelSize: 13
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Edit Station")
                onClicked: {
                    app.editDialog.stationObject = root.modelData
                    app.editDialog.setupAndOpen()
                }
            }

            IconButton {
                visible: root.isManualPage
                iconName: "trash"
                iconSize: 15
                implicitWidth: 34
                implicitHeight: 34
                flat: true
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Remove from My stations")
                onClicked: {
                    if (!app.backend) return
                    app.backend.removeManualStation(root.modelData.uuid || "",
                                                   root.modelData.urlResolved || root.modelData.url || "")
                    app.toast(qsTr("Removed from My stations"))
                }
            }

            IconButton {
                visible: !root.isManualPage
                iconName: root.modelData.isFavorite ? "heart" : "heartOutline"
                iconSize: 16
                implicitWidth: 34
                implicitHeight: 34
                flat: true
                highlighted: !!root.modelData.isFavorite
                ToolTip.visible: hovered
                ToolTip.text: root.modelData.isFavorite
                              ? qsTr("Remove from favorites")
                              : qsTr("Add to favorites")
                onClicked: {
                    if (!app.backend) return
                    app.backend.toggleFavoriteById(root.modelData.uuid, root.modelData.urlResolved)
                    app.toast(root.modelData.isFavorite
                              ? qsTr("Removed from favorites")
                              : qsTr("Added to favorites"))
                }
            }

            IconButton {
                iconName: (root.isCurrent && app.backend && app.backend.player && app.backend.player.playing)
                           ? "pause" : "play"
                iconSize: 16
                implicitWidth: 36
                implicitHeight: 36
                round: true
                accentFill: root.isCurrent || root.isPlaying
                ToolTip.visible: hovered
                ToolTip.text: root.isPlaying ? qsTr("Pause") : qsTr("Play")
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

    component MetaChip: Rectangle {
        property string text: ""
        property bool filled: false
        property bool accent: false
        height: 20
        width: chipLabel.implicitWidth + 12
        radius: 6
        color: filled ? BearTheme.accent
               : (BearTheme.isLight ? "#ececf0" : "#1a1a1e")
        border.width: filled ? 0 : 1
        border.color: BearTheme.cardBorder

        Label {
            id: chipLabel
            anchors.centerIn: parent
            text: parent.text
            color: parent.filled ? "#ffffff" : BearTheme.textMuted
            font.pixelSize: 10
            font.bold: parent.filled
        }
    }
}
