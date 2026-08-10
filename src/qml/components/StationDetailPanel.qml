// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

// Now Playing stage — active stream only (not list selection).
Rectangle {
    id: root

    required property var app
    // When false (short window), transport lives in the bottom PlayerBar instead.
    property bool showTransportDock: true
    // Compact stage content when height is limited (smaller cover, tighter spacing).
    readonly property bool compactStage: !showTransportDock

    color: BearTheme.panelAlt
    border.color: BearTheme.cardBorder
    radius: 0
    readonly property int contentPadding: compactStage ? 12 : 14

    readonly property var player: app && app.backend ? app.backend.player : null
    readonly property var station: app && app.backend ? app.backend.currentStation : null

    readonly property bool isActive: {
        if (!player)
            return false
        if ((player.currentStationName || "").length > 0)
            return true
        return player.connectionState
               && player.connectionState !== "idle"
               && player.connectionState !== ""
    }

    readonly property string stationName: {
        var fromPlayer = player ? (player.currentStationName || "") : ""
        if (fromPlayer.length > 0)
            return fromPlayer
        if (station && station.name)
            return station.name
        return ""
    }
    readonly property string country: station && station.country ? station.country : ""
    readonly property string homepage: station && station.homepage ? station.homepage : ""
    readonly property string favicon: station && station.favicon ? station.favicon : ""
    readonly property string codec: station && station.codec ? station.codec : ""
    readonly property int bitrate: station ? Number(station.bitrate || 0) : 0
    readonly property int votes: station ? Number(station.votes || 0) : 0
    readonly property bool isFavorite: station ? Boolean(station.isFavorite) : false
    readonly property string stationUuid: {
        if (station && station.uuid)
            return station.uuid
        return app && app.backend ? (app.backend.currentStationUuid || "") : ""
    }
    readonly property string stationUrl: {
        if (station) {
            if (station.urlResolved)
                return station.urlResolved
            if (station.url)
                return station.url
        }
        return app && app.backend ? (app.backend.currentStationUrl || "") : ""
    }
    readonly property string stationKey: stationUuid || stationUrl || stationName
    readonly property string coverArtUrl: player && player.currentCoverArtUrl
                                         ? player.currentCoverArtUrl : ""
    readonly property string trackArtist: player ? (player.currentTrackArtist || "") : ""
    readonly property string trackTitle: player ? (player.currentTrackTitle || "") : ""
    readonly property string nowPlayingLine: player ? (player.currentNowPlaying || "") : ""
    readonly property bool hasTrackInfo: trackArtist.length > 0
                                         || trackTitle.length > 0
                                         || nowPlayingLine.length > 0
    readonly property var historyList: player && player.trackHistory ? player.trackHistory : []
    readonly property int historyCount: historyList.length
    readonly property var previousTrack: historyCount > 0 ? historyList[0] : null

    readonly property var tagList: {
        if (!station || !station.tags)
            return []
        return String(station.tags).split(/[,;]/).map(function (t) {
            return t.trim()
        }).filter(function (t) {
            return t.length > 0
        }).slice(0, 8)
    }

    function connectionStatusText() {
        if (!player)
            return qsTr("Inactive")
        // Prefer Live whenever audio is actually playing (backend mirrors this).
        if (player.playing)
            return qsTr("Live")
        switch (player.connectionState) {
        case "connecting": return qsTr("Connecting…")
        case "buffering": return qsTr("Buffering…")
        case "retrying": return qsTr("Reconnecting…")
        case "playing": return qsTr("Live")
        case "paused": return qsTr("Paused")
        case "error": return qsTr("Stream unavailable")
        default: return qsTr("Inactive")
        }
    }

    function statusColor() {
        if (!player)
            return BearTheme.textMuted
        if (player.connectionState === "error")
            return BearTheme.warn
        if (player.playing || player.connectionState === "playing")
            return BearTheme.accent
        if (["connecting", "buffering", "retrying"].indexOf(player.connectionState) !== -1)
            return BearTheme.accent
        return BearTheme.textMuted
    }

    function trackLine(entry) {
        if (!entry)
            return ""
        if (entry.line)
            return entry.line
        var a = entry.artist || ""
        var t = entry.title || ""
        if (a && t)
            return a + " — " + t
        return t || a || ""
    }

    function hostLabel(url) {
        if (!url || url.length === 0)
            return ""
        var s = String(url).replace(/^https?:\/\//i, "")
        s = s.replace(/\/.*$/, "")
        s = s.replace(/^www\./i, "")
        return s.length > 0 ? s : url
    }

    function openHomepage() {
        if (root.homepage.length > 0)
            Qt.openUrlExternally(root.homepage)
    }

    function copyStreamUrl() {
        if (!app.backend || root.stationUrl.length === 0)
            return
        app.backend.copyToClipboard(root.stationUrl)
        app.toast(qsTr("Stream URL copied"))
    }

    Flickable {
        id: detailScroll
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: root.showTransportDock ? transportDock.top : parent.bottom
        clip: true
        visible: root.isActive
        contentWidth: width
        contentHeight: contentCol.implicitHeight + root.contentPadding * 2
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick

        ColumnLayout {
            id: contentCol
            x: root.contentPadding
            y: root.contentPadding
            width: Math.max(1, detailScroll.width - root.contentPadding * 2)
            spacing: root.compactStage ? 10 : 14

            // Header row: label + status chip
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Now Playing")
                    color: BearTheme.textMuted
                    font.pixelSize: 11
                    font.bold: true
                    font.letterSpacing: 0.8
                }

                Rectangle {
                    height: 22
                    width: statusChipLabel.implicitWidth + 14
                    radius: 11
                    color: "transparent"
                    border.color: root.statusColor()
                    border.width: 1

                    Label {
                        id: statusChipLabel
                        anchors.centerIn: parent
                        text: root.connectionStatusText()
                        color: root.statusColor()
                        font.pixelSize: 10
                        font.bold: true
                    }
                }
            }

            // Artwork — large when dock is in-stage; compact when bottom transport is used.
            Rectangle {
                id: heroWell
                Layout.fillWidth: true
                Layout.preferredHeight: root.compactStage
                                        ? Math.min(width * 0.55, 160)
                                        : width
                Layout.maximumHeight: root.compactStage ? 160 : 400
                radius: root.compactStage ? 12 : 16
                color: BearTheme.imageWell
                border.color: BearTheme.cardBorder
                border.width: 1
                clip: true

                // Soft vignette background
                Rectangle {
                    anchors.fill: parent
                    gradient: Gradient {
                        GradientStop {
                            position: 0.0
                            color: BearTheme.isLight ? "#12ff4f86" : "#18000000"
                        }
                        GradientStop {
                            position: 1.0
                            color: "transparent"
                        }
                    }
                }

                StationLogo {
                    anchors.centerIn: parent
                    width: Math.min(parent.width, parent.height) * 0.72
                    height: width
                    app: root.app
                    stationName: root.stationName
                    stationKey: root.stationKey
                    faviconUrl: root.favicon
                    homepageUrl: root.homepage
                    logoMargin: 0
                    visible: !coverImage.visible
                    radius: 12
                }

                Image {
                    id: coverImage
                    anchors.fill: parent
                    anchors.margins: 0
                    source: {
                        if (!app.backend || root.coverArtUrl === "")
                            return ""
                        var revision = app.backend.stationImageRevision
                        if (revision < 0)
                            return ""
                        return app.backend.stationImageSource(root.coverArtUrl)
                    }
                    // Fit full art; letterbox instead of harsh crop when ratio differs
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    asynchronous: true
                    visible: source !== "" && status === Image.Ready
                }
            }

            // Station name under art
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6

                Label {
                    Layout.fillWidth: true
                    text: root.stationName.length > 0 ? root.stationName : qsTr("Unknown station")
                    color: BearTheme.textMain
                    font.pixelSize: 17
                    font.bold: true
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                }
                Label {
                    Layout.fillWidth: true
                    text: root.country
                    visible: text.length > 0
                    color: BearTheme.textMuted
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                }

                // Quick actions: website · copy stream (always visible when URLs exist)
                RowLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: 4
                    spacing: 10
                    visible: root.homepage.length > 0 || root.stationUrl.length > 0

                    Item { Layout.fillWidth: true }

                    IconButton {
                        visible: root.homepage.length > 0
                        iconName: "globe"
                        iconSize: 18
                        implicitWidth: 44
                        implicitHeight: 44
                        ToolTip.visible: hovered
                        ToolTip.text: root.homepage.length > 0
                                      ? (qsTr("Website") + " · " + root.hostLabel(root.homepage))
                                      : qsTr("Website")
                        Accessible.name: qsTr("Open station website")
                        onClicked: root.openHomepage()
                    }

                    IconButton {
                        visible: root.stationUrl.length > 0
                        iconName: "copy"
                        iconSize: 17
                        implicitWidth: 44
                        implicitHeight: 44
                        flat: true
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Copy stream URL")
                        onClicked: root.copyStreamUrl()
                    }

                    Item { Layout.fillWidth: true }
                }
            }

            // —— Current track card ——
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: currentTrackCol.implicitHeight + 24
                radius: 14
                color: BearTheme.card
                border.color: root.hasTrackInfo ? BearTheme.accent : BearTheme.cardBorder
                border.width: root.hasTrackInfo ? 1.5 : 1

                ColumnLayout {
                    id: currentTrackCol
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 12
                    spacing: 6

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Current track")
                        color: BearTheme.accent
                        font.pixelSize: 10
                        font.bold: true
                        font.letterSpacing: 1.0
                    }

                    Label {
                        Layout.fillWidth: true
                        visible: root.trackArtist.length > 0
                        text: root.trackArtist
                        color: BearTheme.textMain
                        font.pixelSize: 16
                        font.bold: true
                        wrapMode: Text.WordWrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                    }
                    Label {
                        Layout.fillWidth: true
                        visible: root.trackTitle.length > 0
                        text: root.trackTitle
                        color: BearTheme.textMain
                        font.pixelSize: 15
                        wrapMode: Text.WordWrap
                        maximumLineCount: 3
                        elide: Text.ElideRight
                    }
                    Label {
                        Layout.fillWidth: true
                        visible: root.hasTrackInfo
                                 && root.trackArtist.length === 0
                                 && root.trackTitle.length === 0
                        text: root.nowPlayingLine
                        color: BearTheme.textMain
                        font.pixelSize: 15
                        font.bold: true
                        wrapMode: Text.WordWrap
                        maximumLineCount: 3
                        elide: Text.ElideRight
                    }
                    Label {
                        Layout.fillWidth: true
                        visible: !root.hasTrackInfo
                        text: qsTr("Waiting for titles…\nNot every station sends track metadata.")
                        color: BearTheme.textMuted
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                        lineHeight: 1.25
                    }
                }
            }

            // —— Previous track card ——
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: previousTrackCol.implicitHeight + 22
                radius: 14
                color: BearTheme.isLight ? "#f3f3f6" : "#101012"
                border.color: BearTheme.cardBorder
                border.width: 1
                visible: root.previousTrack !== null
                         && root.trackLine(root.previousTrack).length > 0
                opacity: 0.95

                ColumnLayout {
                    id: previousTrackCol
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 12
                    spacing: 4

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Previous track")
                        color: BearTheme.textMuted
                        font.pixelSize: 10
                        font.bold: true
                        font.letterSpacing: 1.0
                    }

                    Label {
                        Layout.fillWidth: true
                        visible: root.previousTrack
                                 && (root.previousTrack.artist || "").length > 0
                        text: root.previousTrack ? (root.previousTrack.artist || "") : ""
                        color: BearTheme.textMuted
                        font.pixelSize: 13
                        font.bold: true
                        wrapMode: Text.WordWrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                    }
                    Label {
                        Layout.fillWidth: true
                        visible: root.previousTrack
                                 && (root.previousTrack.title || "").length > 0
                        text: root.previousTrack ? (root.previousTrack.title || "") : ""
                        color: BearTheme.textMuted
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                    }
                    Label {
                        Layout.fillWidth: true
                        visible: root.previousTrack
                                 && (root.previousTrack.artist || "").length === 0
                                 && (root.previousTrack.title || "").length === 0
                        text: root.trackLine(root.previousTrack)
                        color: BearTheme.textMuted
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                    }
                }
            }

            // Older history (beyond previous)
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6
                visible: root.historyCount > 1

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Earlier on this stream")
                    color: BearTheme.textMuted
                    font.pixelSize: 10
                    font.bold: true
                    font.letterSpacing: 0.6
                }

                Repeater {
                    model: {
                        if (!player || !player.trackHistory || player.trackHistory.length < 2)
                            return []
                        return player.trackHistory.slice(1, 8)
                    }
                    delegate: Label {
                        required property var modelData
                        Layout.fillWidth: true
                        text: "·  " + root.trackLine(modelData)
                        color: BearTheme.textMuted
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                        opacity: 0.85
                    }
                }
            }

            Flow {
                Layout.fillWidth: true
                spacing: 6
                visible: root.tagList.length > 0

                Repeater {
                    model: root.tagList
                    delegate: Rectangle {
                        required property string modelData
                        height: 22
                        width: chipLabel.implicitWidth + 12
                        radius: 11
                        color: BearTheme.isLight ? "#ebebf0" : "#1a1a1e"
                        border.color: BearTheme.cardBorder
                        border.width: 1
                        Label {
                            id: chipLabel
                            anchors.centerIn: parent
                            text: parent.modelData
                            color: BearTheme.textMuted
                            font.pixelSize: 10
                        }
                    }
                }
            }

            Flow {
                Layout.fillWidth: true
                spacing: 6
                visible: (root.codec.length > 0 && root.codec !== "unknown")
                         || root.bitrate > 0
                         || root.votes > 0

                Rectangle {
                    visible: root.codec.length > 0 && root.codec !== "unknown"
                    height: 20
                    width: codecPill.implicitWidth + 10
                    radius: 5
                    color: "transparent"
                    border.color: BearTheme.accent
                    border.width: 1
                    Label {
                        id: codecPill
                        anchors.centerIn: parent
                        text: root.codec.toUpperCase()
                        color: BearTheme.accent
                        font.pixelSize: 9
                        font.bold: true
                    }
                }
                Rectangle {
                    visible: root.bitrate > 0
                    height: 20
                    width: brPill.implicitWidth + 10
                    radius: 5
                    color: BearTheme.accent
                    Label {
                        id: brPill
                        anchors.centerIn: parent
                        text: root.bitrate + " kbps"
                        color: "#ffffff"
                        font.pixelSize: 9
                        font.bold: true
                    }
                }
                Rectangle {
                    visible: root.votes > 0
                    height: 20
                    width: votesPill.implicitWidth + 10
                    radius: 5
                    color: BearTheme.isLight ? "#ebebf0" : "#1a1a1e"
                    border.color: BearTheme.cardBorder
                    border.width: 1
                    Label {
                        id: votesPill
                        anchors.centerIn: parent
                        text: "▲ " + root.votes
                        color: BearTheme.textMuted
                        font.pixelSize: 9
                        font.bold: true
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 2
            }
        }
    }

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: root.showTransportDock ? transportDock.top : parent.bottom
        anchors.margins: root.contentPadding
        spacing: 10
        visible: !root.isActive

        Item { Layout.fillHeight: true }

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            width: 72
            height: 72
            radius: 36
            color: BearTheme.imageWell
            border.color: BearTheme.cardBorder
            border.width: 1
            Label {
                anchors.centerIn: parent
                text: "📻"
                font.pixelSize: 28
            }
        }
        Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Nothing playing")
            color: BearTheme.textMain
            font.pixelSize: 16
            font.bold: true
        }
        Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            text: qsTr("Pick a station from the list to start listening.")
            color: BearTheme.textMuted
            font.pixelSize: 12
        }

        Item { Layout.fillHeight: true }
    }

    // Tall+wide only: transport dock. Short windows use the bottom PlayerBar.
    Rectangle {
        id: transportDock
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        visible: root.showTransportDock
        height: visible ? Math.max(200, dockCol.implicitHeight + 40) : 0
        color: BearTheme.isLight ? "#f7f7f9" : "#0e0e10"

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 1
            color: BearTheme.cardBorder
        }
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: 1
            height: 10
            gradient: Gradient {
                GradientStop {
                    position: 0.0
                    color: BearTheme.isLight ? "#14000000" : "#28000000"
                }
                GradientStop { position: 1.0; color: "transparent" }
            }
        }

        ColumnLayout {
            id: dockCol
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.leftMargin: 18
            anchors.rightMargin: 18
            anchors.topMargin: 20
            anchors.bottomMargin: 18
            spacing: 16

            PlayerTransport {
                Layout.fillWidth: true
                app: root.app
                variant: "stage"
            }

            // Secondary actions under transport
            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                spacing: 12

                Item { Layout.fillWidth: true }

                IconButton {
                    visible: root.homepage.length > 0
                    iconName: "globe"
                    iconSize: 18
                    implicitWidth: 48
                    implicitHeight: 44
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Website")
                    onClicked: root.openHomepage()
                }

                IconButton {
                    iconName: root.isFavorite ? "heart" : "heartOutline"
                    iconSize: 18
                    implicitWidth: 48
                    implicitHeight: 44
                    highlighted: root.isFavorite
                    enabled: root.stationUuid.length > 0 || root.stationUrl.length > 0
                    ToolTip.visible: hovered
                    ToolTip.text: root.isFavorite
                                  ? qsTr("Remove from favorites")
                                  : qsTr("Add to favorites")
                    onClicked: {
                        if (!app.backend)
                            return
                        app.backend.toggleFavoriteById(root.stationUuid, root.stationUrl)
                        app.toast(root.isFavorite
                                  ? qsTr("Removed from favorites")
                                  : qsTr("Added to favorites"))
                    }
                }

                IconButton {
                    visible: root.stationUrl.length > 0
                    iconName: "copy"
                    iconSize: 17
                    implicitWidth: 48
                    implicitHeight: 44
                    flat: true
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Copy stream URL")
                    onClicked: root.copyStreamUrl()
                }

                Item { Layout.fillWidth: true }
            }
        }
    }
}
