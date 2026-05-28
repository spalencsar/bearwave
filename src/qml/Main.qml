// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1 as Platform

ApplicationWindow {
    id: root
    title: qsTr("BearWave")
    width: 980
    height: 700
    minimumWidth: 520
    minimumHeight: 460
    visible: true

    onClosing: function(close) {
        close.accepted = false
        root.hide()
    }

    Connections {
        target: backend
        function onRaiseRequested() {
            root.show()
            root.raise()
            root.requestActivate()
        }
    }

    Platform.SystemTrayIcon {
        id: systray
        visible: true
        icon.name: "multimedia-player"
        tooltip: qsTr("BearWave")

        menu: Platform.Menu {
            Platform.MenuItem {
                text: backend && backend.player && backend.player.playing ? qsTr("Pause") : qsTr("Play")
                onTriggered: {
                    if (backend && backend.player) {
                        backend.player.togglePlayPause()
                    }
                }
            }
            Platform.MenuItem {
                text: root.visible ? qsTr("Hide") : qsTr("Show")
                onTriggered: {
                    if (root.visible) {
                        root.hide()
                    } else {
                        root.show()
                        root.raise()
                        root.requestActivate()
                    }
                }
            }
            Platform.MenuSeparator {}
            Platform.MenuItem {
                text: qsTr("Quit")
                onTriggered: Qt.quit()
            }
        }

        onActivated: function(reason) {
            if (reason === Platform.SystemTrayIcon.Trigger) {
                if (root.visible) {
                    root.hide()
                } else {
                    root.show()
                    root.raise()
                    root.requestActivate()
                }
            }
        }
    }

    property var currentPage: "top"
    property var backend: (typeof radioBackend !== "undefined" ? radioBackend : null)
    property bool compactMode: width < 780
    property real contentOpacity: 1.0
    property string activeQuickFilter: ""

    readonly property color bgA: "#0f141b"
    readonly property color bgB: "#131b25"
    readonly property color panel: "#182433"
    readonly property color card: "#1b2a3d"
    readonly property color cardHover: "#223654"
    readonly property color cardBorder: "#2d4566"
    readonly property color accent: "#2bb0ff"
    readonly property color textMain: "#eaf1fb"
    readonly property color textMuted: "#9eb1c9"
    readonly property color warn: "#ff8b8b"

    function toast(message) {
        toastLabel.text = message
        toastPopup.open()
    }

    function activeModel() {
        if (!backend) {
            return []
        }
        if (currentPage === "favorites") {
            return backend.favoriteStations
        } else if (currentPage === "history") {
            return backend.recentStations
        } else {
            return backend.stations
        }
    }

    onCurrentPageChanged: {
        contentOpacity = 0.85
        contentFadeRestart.restart()
    }

    Shortcut {
        sequence: "Space"
        onActivated: {
            if (backend && backend.player) {
                backend.player.togglePlayPause()
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+F"
        onActivated: searchField.forceActiveFocus()
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: bgA }
            GradientStop { position: 1.0; color: bgB }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: headerContent.implicitHeight + 20
            radius: 12
            color: panel
            border.color: cardBorder

            ColumnLayout {
                id: headerContent
                anchors.fill: parent
                anchors.margins: 10
                spacing: 8

                RowLayout {
                    visible: !compactMode
                    Layout.fillWidth: true
                    spacing: 8

                    Image {
                        Layout.preferredWidth: 112
                        Layout.preferredHeight: 40
                        source: "qrc:/assets/app/bearwave_line.png"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                        mipmap: true
                    }

                    Rectangle {
                        Layout.preferredWidth: 1
                        Layout.fillHeight: true
                        color: cardBorder
                        opacity: 0.6
                    }

                    Button {
                        text: qsTr("Top")
                        highlighted: currentPage === "top"
                        onClicked: {
                            if (!backend) return
                            currentPage = "top"
                            activeQuickFilter = ""
                            backend.loadTopStations()
                        }
                    }

                    Button {
                        text: qsTr("DE")
                        highlighted: currentPage === "german"
                        onClicked: {
                            if (!backend) return
                            currentPage = "german"
                            activeQuickFilter = ""
                            backend.loadGermanStations()
                        }
                    }

                    Button {
                        text: qsTr("NL")
                        highlighted: currentPage === "dutch"
                        onClicked: {
                            if (!backend) return
                            currentPage = "dutch"
                            activeQuickFilter = ""
                            backend.loadDutchStations()
                        }
                    }

                    Button {
                        text: qsTr("Favorites")
                        highlighted: currentPage === "favorites"
                        onClicked: {
                            currentPage = "favorites"
                            activeQuickFilter = ""
                        }
                    }

                    Button {
                        text: qsTr("History")
                        highlighted: currentPage === "history"
                        onClicked: {
                            currentPage = "history"
                            activeQuickFilter = ""
                        }
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: compactMode ? "+" : qsTr("Manual +")
                        onClicked: addDialog.open()
                    }

                    Button {
                        text: qsTr("About")
                        onClicked: aboutDialog.open()
                    }

                    Button {
                        visible: backend && backend.canResumeLastStation
                        text: compactMode ? "↺" : qsTr("Resume")
                        onClicked: {
                            if (backend) {
                                backend.resumeLastStation()
                            }
                        }
                    }
                }

                ColumnLayout {
                    visible: compactMode
                    Layout.fillWidth: true
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Image {
                            Layout.preferredWidth: 96
                            Layout.preferredHeight: 34
                            source: "qrc:/assets/app/bearwave_line.png"
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                            mipmap: true
                        }

                        Item { Layout.fillWidth: true }

                        Button {
                            text: "+"
                            onClicked: addDialog.open()
                        }

                        Button {
                            text: qsTr("About")
                            onClicked: aboutDialog.open()
                        }

                        Button {
                            visible: backend && backend.canResumeLastStation
                            text: "↺"
                            onClicked: {
                                if (backend) {
                                    backend.resumeLastStation()
                                }
                            }
                        }
                    }

                    Flow {
                        Layout.fillWidth: true
                        width: parent.width
                        spacing: 8

                        Button {
                            text: qsTr("Top")
                            highlighted: currentPage === "top"
                            onClicked: {
                                if (!backend) return
                                currentPage = "top"
                                activeQuickFilter = ""
                                backend.loadTopStations()
                            }
                        }

                        Button {
                            text: qsTr("DE")
                            highlighted: currentPage === "german"
                            onClicked: {
                                if (!backend) return
                                currentPage = "german"
                                activeQuickFilter = ""
                                backend.loadGermanStations()
                            }
                        }

                        Button {
                            text: qsTr("NL")
                            highlighted: currentPage === "dutch"
                            onClicked: {
                                if (!backend) return
                                currentPage = "dutch"
                                activeQuickFilter = ""
                                backend.loadDutchStations()
                            }
                        }

                        Button {
                            text: qsTr("Favorites")
                            highlighted: currentPage === "favorites"
                            onClicked: {
                                currentPage = "favorites"
                                activeQuickFilter = ""
                            }
                        }

                        Button {
                            text: qsTr("History")
                            highlighted: currentPage === "history"
                            onClicked: {
                                currentPage = "history"
                                activeQuickFilter = ""
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: cardBorder
                    opacity: 0.6
                }

                RowLayout {
                    visible: !compactMode
                    Layout.fillWidth: true
                    spacing: 8

                    TextField {
                        id: searchField
                        Layout.fillWidth: true
                        placeholderText: qsTr("Search stations (name, genre, country)")
                        onTextChanged: {
                            if (backend) {
                                backend.filterQuery = text
                            }
                            searchTimer.restart()
                        }
                        onAccepted: {
                            if (text.length < 2 || !backend) return
                            currentPage = "search"
                            backend.searchStations(text)
                        }
                    }

                    Button {
                        text: qsTr("Search")
                        highlighted: true
                        onClicked: {
                            if (searchField.text.length < 2 || !backend) return
                            currentPage = "search"
                            backend.searchStations(searchField.text)
                        }
                    }

                    Button {
                        text: compactMode ? "A-Z" : qsTr("Sort A-Z")
                        onClicked: {
                            if (backend && currentPage !== "favorites") {
                                backend.sortStations("name")
                            }
                        }
                    }

                    Button {
                        text: compactMode ? "kb" : qsTr("Sort Bitrate")
                        onClicked: {
                            if (backend && currentPage !== "favorites") {
                                backend.sortStations("bitrate")
                            }
                        }
                    }

                    Button {
                        text: compactMode ? "❤" : qsTr("Sort Votes")
                        onClicked: {
                            if (backend && currentPage !== "favorites") {
                                backend.sortStations("votes")
                            }
                        }
                    }
                }

                ColumnLayout {
                    visible: compactMode
                    Layout.fillWidth: true
                    spacing: 8

                    TextField {
                        id: compactSearchField
                        Layout.fillWidth: true
                        placeholderText: qsTr("Search stations (name, genre, country)")
                        text: searchField.text
                        onTextChanged: {
                            if (searchField.text !== text) {
                                searchField.text = text
                            }
                            if (backend) {
                                backend.filterQuery = text
                            }
                        }
                        onAccepted: {
                            if (text.length < 2 || !backend) return
                            currentPage = "search"
                            backend.searchStations(text)
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Item { Layout.fillWidth: true }

                        Button {
                            text: qsTr("Search")
                            highlighted: true
                            onClicked: {
                                if (compactSearchField.text.length < 2 || !backend) return
                                currentPage = "search"
                                backend.searchStations(compactSearchField.text)
                            }
                        }
                    }

                    Flow {
                        Layout.fillWidth: true
                        width: parent.width
                        spacing: 8

                        Button {
                            text: "A-Z"
                            onClicked: {
                                if (backend && currentPage !== "favorites") {
                                    backend.sortStations("name")
                                }
                            }
                        }

                        Button {
                            text: "kb"
                            onClicked: {
                                if (backend && currentPage !== "favorites") {
                                    backend.sortStations("bitrate")
                                }
                            }
                        }

                        Button {
                            text: "❤"
                            onClicked: {
                                if (backend && currentPage !== "favorites") {
                                    backend.sortStations("votes")
                                }
                            }
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Tip: you are not limited to DE/NL. Search worldwide by country, genre, or station name.")
                    color: textMuted
                    font.pixelSize: 11
                    wrapMode: compactMode ? Text.WordWrap : Text.NoWrap
                    elide: compactMode ? Text.ElideNone : Text.ElideRight
                }

                RowLayout {
                    visible: !compactMode
                    Layout.fillWidth: true
                    spacing: 6

                    Label {
                        text: qsTr("Genre:")
                        color: textMuted
                        font.pixelSize: 11
                        rightPadding: 10
                    }

                    Button {
                        text: qsTr("Rock")
                        highlighted: activeQuickFilter === "tag:rock"
                        onClicked: {
                            if (!backend) return
                            currentPage = "genre-rock"
                            activeQuickFilter = "tag:rock"
                            backend.loadByTag("rock")
                        }
                    }

                    Button {
                        text: qsTr("News")
                        highlighted: activeQuickFilter === "tag:news"
                        onClicked: {
                            if (!backend) return
                            currentPage = "genre-news"
                            activeQuickFilter = "tag:news"
                            backend.loadByTag("news")
                        }
                    }

                    Button {
                        text: qsTr("Jazz")
                        highlighted: activeQuickFilter === "tag:jazz"
                        onClicked: {
                            if (!backend) return
                            currentPage = "genre-jazz"
                            activeQuickFilter = "tag:jazz"
                            backend.loadByTag("jazz")
                        }
                    }

                    Item { Layout.preferredWidth: 20 }

                    Label {
                        text: qsTr("Country:")
                        color: textMuted
                        font.pixelSize: 11
                        rightPadding: 10
                    }

                    Button {
                        text: qsTr("US")
                        highlighted: activeQuickFilter === "cc:US"
                        onClicked: {
                            if (!backend) return
                            currentPage = "country-us"
                            activeQuickFilter = "cc:US"
                            backend.loadByCountryCode("US")
                        }
                    }

                    Button {
                        text: qsTr("UK")
                        highlighted: activeQuickFilter === "cc:GB"
                        onClicked: {
                            if (!backend) return
                            currentPage = "country-gb"
                            activeQuickFilter = "cc:GB"
                            backend.loadByCountryCode("GB")
                        }
                    }

                    Button {
                        text: qsTr("FR")
                        highlighted: activeQuickFilter === "cc:FR"
                        onClicked: {
                            if (!backend) return
                            currentPage = "country-fr"
                            activeQuickFilter = "cc:FR"
                            backend.loadByCountryCode("FR")
                        }
                    }

                    Button {
                        text: qsTr("WORLD")
                        highlighted: activeQuickFilter === "world"
                        onClicked: {
                            if (!backend) return
                            currentPage = "world"
                            activeQuickFilter = "world"
                            backend.loadWorldStations()
                        }
                    }

                    Item { Layout.fillWidth: true }
                }

                ColumnLayout {
                    visible: compactMode
                    Layout.fillWidth: true
                    spacing: 8

                    Flow {
                        Layout.fillWidth: true
                        width: parent.width
                        spacing: 8

                        Label {
                            text: qsTr("Genre:")
                            color: textMuted
                            font.pixelSize: 11
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 8
                            rightPadding: 16
                            topPadding: 8
                            bottomPadding: 8
                        }

                        Button {
                            text: qsTr("Rock")
                            highlighted: activeQuickFilter === "tag:rock"
                            onClicked: {
                                if (!backend) return
                                currentPage = "genre-rock"
                                activeQuickFilter = "tag:rock"
                                backend.loadByTag("rock")
                            }
                        }

                        Button {
                            text: qsTr("News")
                            highlighted: activeQuickFilter === "tag:news"
                            onClicked: {
                                if (!backend) return
                                currentPage = "genre-news"
                                activeQuickFilter = "tag:news"
                                backend.loadByTag("news")
                            }
                        }

                        Button {
                            text: qsTr("Jazz")
                            highlighted: activeQuickFilter === "tag:jazz"
                            onClicked: {
                                if (!backend) return
                                currentPage = "genre-jazz"
                                activeQuickFilter = "tag:jazz"
                                backend.loadByTag("jazz")
                            }
                        }
                    }

                    Flow {
                        Layout.fillWidth: true
                        width: parent.width
                        spacing: 8

                        Label {
                            text: qsTr("Country:")
                            color: textMuted
                            font.pixelSize: 11
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 8
                            rightPadding: 16
                            topPadding: 8
                            bottomPadding: 8
                        }

                        Button {
                            text: qsTr("US")
                            highlighted: activeQuickFilter === "cc:US"
                            onClicked: {
                                if (!backend) return
                                currentPage = "country-us"
                                activeQuickFilter = "cc:US"
                                backend.loadByCountryCode("US")
                            }
                        }

                        Button {
                            text: qsTr("UK")
                            highlighted: activeQuickFilter === "cc:GB"
                            onClicked: {
                                if (!backend) return
                                currentPage = "country-gb"
                                activeQuickFilter = "cc:GB"
                                backend.loadByCountryCode("GB")
                            }
                        }

                        Button {
                            text: qsTr("FR")
                            highlighted: activeQuickFilter === "cc:FR"
                            onClicked: {
                                if (!backend) return
                                currentPage = "country-fr"
                                activeQuickFilter = "cc:FR"
                                backend.loadByCountryCode("FR")
                            }
                        }

                        Button {
                            text: qsTr("WORLD")
                            highlighted: activeQuickFilter === "world"
                            onClicked: {
                                if (!backend) return
                                currentPage = "world"
                                activeQuickFilter = "world"
                                backend.loadWorldStations()
                            }
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    visible: backend && backend.lastError.length > 0
                    text: backend ? (qsTr("Error: ") + backend.lastError) : ""
                    color: warn
                    elide: Text.ElideRight
                    font.pixelSize: 12
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 12
            color: panel
            border.color: cardBorder
            clip: true
            opacity: contentOpacity

            Behavior on opacity {
                NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
            }

            ScrollView {
                id: stationScrollView
                anchors.fill: parent
                anchors.margins: 10
                clip: true
                ScrollBar.vertical.policy: ScrollBar.AlwaysOn

                ListView {
                    id: stationList
                    width: stationScrollView.availableWidth
                    spacing: 8
                    model: activeModel()
                    visible: count > 0

                    delegate: Rectangle {
                        id: stationCard
                        required property int index
                        required property var modelData
                        width: stationScrollView.availableWidth
                        height: compactMode ? 72 : 78
                        radius: 10

                        readonly property bool isCurrent: {
                            if (!backend || !modelData) return false;
                            var currentUuid = backend.currentStationUuid;
                            var currentUrl = backend.currentStationUrl;
                            var cardUuid = modelData.uuid || "";
                            var cardUrl = modelData.urlResolved || modelData.url || "";
                            if (currentUuid !== "" && cardUuid !== "") {
                                return currentUuid === cardUuid;
                            }
                            return currentUrl !== "" && currentUrl === cardUrl;
                        }
                        readonly property bool isPlaying: isCurrent && backend && backend.player && backend.player.playing

                        color: stationCard.isCurrent
                            ? (cardMouse.containsMouse ? "#1d3350" : "#16283e")
                            : (cardMouse.containsMouse ? cardHover : card)
                        border.color: stationCard.isCurrent ? accent : cardBorder
                        border.width: stationCard.isCurrent ? 2 : 1

                        MouseArea {
                            id: cardMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            acceptedButtons: Qt.LeftButton
                            onClicked: {
                                if (!backend) return
                                if (stationCard.isCurrent) {
                                    backend.player.togglePlayPause()
                                } else {
                                    if (currentPage === "favorites") {
                                        backend.playFavoriteStation(index)
                                    } else if (currentPage === "history") {
                                        backend.playRecentByUuid(modelData.uuid, modelData.urlResolved)
                                    } else {
                                        backend.playStation(index)
                                    }
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
                                border.color: accent

                                Image {
                                    id: stationFavicon
                                    anchors.fill: parent
                                    anchors.margins: 3
                                    source: (stationCard.modelData.favicon && stationCard.modelData.favicon.startsWith("https://"))
                                            ? stationCard.modelData.favicon
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
                                        text: stationCard.modelData.name
                                        color: stationCard.isCurrent ? accent : textMain
                                        font.bold: true
                                        font.pixelSize: compactMode ? 13 : 14
                                        elide: Text.ElideRight
                                    }

                                    Row {
                                        id: eqAnimation
                                        spacing: 2
                                        Layout.alignment: Qt.AlignVCenter
                                        visible: stationCard.isPlaying

                                        Rectangle {
                                            id: bar1
                                            width: 2
                                            height: 12
                                            color: accent
                                            radius: 1
                                            Behavior on height {
                                                NumberAnimation { duration: 120 }
                                            }
                                        }
                                        Rectangle {
                                            id: bar2
                                            width: 2
                                            height: 12
                                            color: accent
                                            radius: 1
                                            Behavior on height {
                                                NumberAnimation { duration: 120 }
                                            }
                                        }
                                        Rectangle {
                                            id: bar3
                                            width: 2
                                            height: 12
                                            color: accent
                                            radius: 1
                                            Behavior on height {
                                                NumberAnimation { duration: 120 }
                                            }
                                        }

                                        Timer {
                                            interval: 150
                                            running: stationCard.isPlaying
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
                                    text: stationCard.modelData.country + "  •  "
                                          + (stationCard.modelData.codec && stationCard.modelData.codec !== "unknown" && stationCard.modelData.codec !== "" ? stationCard.modelData.codec.toUpperCase() + "  •  " : "")
                                          + (stationCard.modelData.bitrate > 0 ? stationCard.modelData.bitrate + " kbps" : qsTr("Stream"))
                                    color: textMuted
                                    font.pixelSize: 11
                                    elide: Text.ElideRight
                                }
                            }

                            Button {
                                visible: !stationCard.modelData.uuid
                                Layout.preferredWidth: compactMode ? 34 : 40
                                Layout.preferredHeight: 40
                                text: "✏"
                                ToolTip.visible: hovered
                                ToolTip.text: qsTr("Edit Station")
                                onClicked: {
                                    editDialog.stationObject = stationCard.modelData
                                    editDialog.setupAndOpen()
                                }
                            }

                            Button {
                                Layout.preferredWidth: compactMode ? 34 : 40
                                Layout.preferredHeight: 40
                                text: stationCard.modelData.isFavorite ? "★" : "☆"
                                onClicked: {
                                    if (!backend) return
                                    backend.toggleFavoriteById(stationCard.modelData.uuid, stationCard.modelData.urlResolved)
                                    toast(stationCard.modelData.isFavorite ? qsTr("Removed from favorites") : qsTr("Added to favorites"))
                                }
                            }

                            Button {
                                Layout.preferredWidth: compactMode ? 34 : 40
                                Layout.preferredHeight: 40
                                text: (stationCard.isCurrent && backend && backend.player && backend.player.playing) ? "⏸" : "▶"
                                onClicked: {
                                    if (!backend) return
                                    if (stationCard.isCurrent) {
                                        backend.player.togglePlayPause()
                                    } else {
                                        if (currentPage === "favorites") {
                                            backend.playFavoriteStation(index)
                                        } else if (currentPage === "history") {
                                            backend.playRecentByUuid(modelData.uuid, modelData.urlResolved)
                                        } else {
                                            backend.playStation(index)
                                        }
                                    }
                                }
                            }
                        }
                    }
            }
            }

            Column {
                anchors.centerIn: parent
                spacing: 8
                visible: stationList.count === 0

                Label {
                    text: currentPage === "history" ? qsTr("No playback history") : qsTr("No stations loaded yet")
                    color: textMain
                    font.bold: true
                }

                Label {
                    text: currentPage === "history" ? qsTr("Play some stations to build history") : qsTr("Load DE/NL stations or use search")
                    color: textMuted
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 108
            radius: 12
            color: panel
            border.color: cardBorder

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
                    border.color: cardBorder

                    Image {
                        id: coverImage
                        anchors.fill: parent
                        source: {
                            if (backend && backend.player && backend.player.currentCoverArtUrl && backend.player.currentCoverArtUrl !== "") {
                                return backend.player.currentCoverArtUrl;
                            }
                            if (backend && backend.currentStation && backend.currentStation.favicon && backend.currentStation.favicon.startsWith("https://")) {
                                return backend.currentStation.favicon;
                            }
                            return "qrc:/assets/app/bearwave.png";
                        }
                        fillMode: (backend && backend.player && backend.player.currentCoverArtUrl && backend.player.currentCoverArtUrl !== "") ? Image.PreserveAspectCrop : Image.PreserveAspectFit
                        smooth: true
                        asynchronous: true
                        anchors.margins: {
                            if (backend && backend.player && backend.player.currentCoverArtUrl && backend.player.currentCoverArtUrl !== "") {
                                return 0;
                            }
                            if (backend && backend.currentStation && backend.currentStation.favicon && backend.currentStation.favicon.startsWith("https://")) {
                                return 8;
                            }
                            return 16;
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
                            text: backend && backend.player ? (backend.player.currentStationName || qsTr("No station selected")) : qsTr("No station selected")
                            color: textMain
                            font.pixelSize: 16
                            font.bold: true
                            elide: Text.ElideRight
                        }

                        Rectangle {
                            id: codecBadge
                            visible: backend && backend.currentStation && backend.currentStation.codec && backend.currentStation.codec !== "unknown" && backend.currentStation.codec !== ""
                            height: 18
                            width: codecLabel.implicitWidth + 12
                            radius: 4
                            color: "transparent"
                            border.color: accent
                            border.width: 1

                            Label {
                                id: codecLabel
                                anchors.centerIn: parent
                                text: (backend && backend.currentStation && backend.currentStation.codec) ? backend.currentStation.codec.toUpperCase() : ""
                                color: accent
                                font.pixelSize: 9
                                font.bold: true
                            }
                        }

                        Rectangle {
                            id: bitrateBadge
                            visible: backend && backend.currentStation && backend.currentStation.bitrate > 0
                            height: 18
                            width: bitrateLabel.implicitWidth + 12
                            radius: 4
                            color: accent
                            border.color: accent
                            border.width: 1

                            Label {
                                id: bitrateLabel
                                anchors.centerIn: parent
                                text: (backend && backend.currentStation) ? backend.currentStation.bitrate + " kbps" : ""
                                color: "#ffffff"
                                font.pixelSize: 9
                                font.bold: true
                            }
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: backend && backend.player && backend.player.currentNowPlaying.length > 0
                              ? (qsTr("Now playing: ") + backend.player.currentNowPlaying)
                              : qsTr("Now playing: No track info")
                        color: textMuted
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Button {
                            text: "⏮"
                            Layout.preferredWidth: 44
                            enabled: backend ? backend.hasPreviousStation() : false
                            onClicked: {
                                if (backend) {
                                    backend.playPreviousStation()
                                }
                            }
                        }

                        Button {
                            text: backend && backend.player && backend.player.playing ? "⏸" : "▶"
                            Layout.preferredWidth: 44
                            onClicked: {
                                if (backend && backend.player) {
                                    backend.player.togglePlayPause()
                                }
                            }
                        }

                        Button {
                            text: "⏹"
                            Layout.preferredWidth: 44
                            onClicked: {
                                if (backend && backend.player) {
                                    backend.player.stop()
                                }
                            }
                        }

                        Button {
                            text: "⏭"
                            Layout.preferredWidth: 44
                            enabled: backend ? backend.hasNextStation() : false
                            onClicked: {
                                if (backend) {
                                    backend.playNextStation()
                                }
                            }
                        }

                        Button {
                            id: muteButton
                            text: (backend && backend.player && backend.player.volume > 0) ? "🔊" : "🔇"
                            Layout.preferredWidth: 44
                            property real lastVolume: 0.5

                            ToolTip.visible: hovered
                            ToolTip.text: (backend && backend.player && backend.player.volume > 0) ? qsTr("Mute") : qsTr("Unmute")

                            onClicked: {
                                if (backend && backend.player) {
                                    if (backend.player.volume > 0) {
                                        lastVolume = backend.player.volume
                                        backend.player.setVolume(0)
                                    } else {
                                        backend.player.setVolume(lastVolume > 0 ? lastVolume : 0.5)
                                    }
                                }
                            }
                        }

                        Slider {
                            Layout.fillWidth: true
                            from: 0
                            to: 1
                            value: backend && backend.player ? backend.player.volume : 0.5
                            onMoved: {
                                if (backend && backend.player) {
                                    backend.player.setVolume(value)
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        visible: backend && backend.loading
        anchors.fill: parent
        color: "#7f0b121a"
        z: 20

        Rectangle {
            anchors.centerIn: parent
            width: 210
            height: 110
            radius: 12
            color: panel
            border.color: cardBorder

            Column {
                anchors.centerIn: parent
                spacing: 10

                BusyIndicator {
                    running: true
                    width: 36
                    height: 36
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Label {
                    text: qsTr("Loading stations...")
                    color: textMain
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }

    Component.onCompleted: {
        if (backend) {
            backend.loadTopStations()
        }
    }

    Dialog {
        id: addDialog
        title: qsTr("Add station manually")
        modal: true
        anchors.centerIn: parent
        width: compactMode ? 320 : 420
        standardButtons: Dialog.Ok | Dialog.Cancel

        contentItem: ColumnLayout {
            spacing: 8
            TextField { id: manualName; Layout.fillWidth: true; placeholderText: qsTr("Name") }
            TextField { id: manualUrl; Layout.fillWidth: true; placeholderText: qsTr("Stream URL (http/https)") }
            TextField { id: manualCountry; Layout.fillWidth: true; placeholderText: qsTr("Country (optional)") }
        }

        onAccepted: {
            if (backend) {
                backend.addManualStation(manualName.text, manualUrl.text, manualCountry.text)
                toast(qsTr("Station added"))
            }
            manualName.text = ""
            manualUrl.text = ""
            manualCountry.text = ""
        }
    }

    Dialog {
        id: editDialog
        title: qsTr("Edit station")
        modal: true
        anchors.centerIn: parent
        width: compactMode ? 320 : 420
        standardButtons: Dialog.Ok | Dialog.Cancel

        property var stationObject: null

        contentItem: ColumnLayout {
            spacing: 8
            TextField { id: editName; Layout.fillWidth: true; placeholderText: qsTr("Name") }
            TextField { id: editUrl; Layout.fillWidth: true; placeholderText: qsTr("Stream URL (http/https)") }
            TextField { id: editCountry; Layout.fillWidth: true; placeholderText: qsTr("Country (optional)") }
        }

        function setupAndOpen() {
            if (stationObject) {
                editName.text = stationObject.name || ""
                editUrl.text = stationObject.url || ""
                editCountry.text = (stationObject.country === qsTr("Manual") ? "" : (stationObject.country || ""))
                open()
            }
        }

        onAccepted: {
            if (backend && stationObject) {
                backend.editManualStation(stationObject, editName.text, editUrl.text, editCountry.text)
                toast(qsTr("Station updated"))
            }
            stationObject = null
            editName.text = ""
            editUrl.text = ""
            editCountry.text = ""
        }

        onRejected: {
            stationObject = null
            editName.text = ""
            editUrl.text = ""
            editCountry.text = ""
        }
    }

    Dialog {
        id: aboutDialog
        modal: true
        anchors.centerIn: Overlay.overlay
        width: compactMode ? 360 : 520
        height: compactMode ? 560 : 680
        standardButtons: Dialog.Ok

        contentItem: ColumnLayout {
            spacing: 16

            // --- HEADER ---
            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 6

                Image {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 72
                    Layout.preferredHeight: 72
                    sourceSize.width: 72
                    sourceSize.height: 72
                    source: "qrc:/assets/app/bearwave.png"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("BearWave")
                    color: textMain
                    font.bold: true
                    font.pixelSize: 26
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Internet Radio Player for KDE")
                    color: textMuted
                    font.pixelSize: 14
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Version: %1").arg(Qt.application.version)
                    color: textMuted
                    font.pixelSize: 12
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Public beta")
                    color: accent
                    font.pixelSize: 12
                    font.bold: true
                }
            }

            // --- CREDITS & LINKS ---
            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 8
                Layout.topMargin: 8
                Layout.bottomMargin: 8

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Author: Sebastian Palencsár")
                    color: textMain
                    font.bold: true
                    font.pixelSize: 14
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 16

                    ToolButton {
                        Layout.preferredWidth: 40
                        Layout.preferredHeight: 40
                        onClicked: Qt.openUrlExternally("https://palencsar.pro")
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Open website")
                        contentItem: Item {
                            Image {
                                anchors.centerIn: parent
                                width: 19
                                height: 19
                                source: "qrc:/assets/ui/globe.svg"
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                            }
                        }
                        background: Rectangle {
                            radius: 20
                            color: parent.hovered ? "#274261" : "#1f3147"
                            border.color: cardBorder
                            border.width: 1
                        }
                    }

                    ToolButton {
                        Layout.preferredWidth: 40
                        Layout.preferredHeight: 40
                        onClicked: Qt.openUrlExternally("https://github.com/spalencsar")
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Open GitHub")
                        contentItem: Item {
                            Image {
                                anchors.centerIn: parent
                                width: 19
                                height: 19
                                source: "qrc:/assets/ui/github.svg"
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                            }
                        }
                        background: Rectangle {
                            radius: 20
                            color: parent.hovered ? "#274261" : "#1f3147"
                            border.color: cardBorder
                            border.width: 1
                        }
                    }

                    ToolButton {
                        Layout.preferredWidth: 40
                        Layout.preferredHeight: 40
                        onClicked: Qt.openUrlExternally("https://www.linkedin.com/in/spalencsar/")
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Open LinkedIn")
                        contentItem: Item {
                            Image {
                                anchors.centerIn: parent
                                width: 19
                                height: 19
                                source: "qrc:/assets/ui/linkedin.svg"
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                            }
                        }
                        background: Rectangle {
                            radius: 20
                            color: parent.hovered ? "#274261" : "#1f3147"
                            border.color: cardBorder
                            border.width: 1
                        }
                    }
                }
            }

            // --- LICENSE ---
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("GNU GPLv3 License")
                        color: textMain
                        font.bold: true
                    }
                    Label {
                        text: qsTr("Copyright (c) 2026")
                        color: textMuted
                        font.pixelSize: 11
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 140
                    radius: 8
                    color: "#101a26"
                    border.color: cardBorder

                    ScrollView {
                        id: licenseScroll
                        anchors.fill: parent
                        anchors.margins: 10
                        clip: true
                        ScrollBar.vertical.policy: ScrollBar.AlwaysOn

                        Label {
                            width: licenseScroll.availableWidth
                            color: textMain
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            text: "This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.\n\nThis program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>."
                        }
                    }
                }
            }
        }
    }

    Popup {
        id: toastPopup
        x: (root.width - width) / 2
        y: root.height - height - 20
        padding: 10
        closePolicy: Popup.NoAutoClose
        background: Rectangle {
            radius: 10
            color: "#24364e"
            border.color: accent
        }

        contentItem: Label {
            id: toastLabel
            color: textMain
            font.pixelSize: 12
        }

        Timer {
            interval: 1400
            running: toastPopup.visible
            repeat: false
            onTriggered: toastPopup.close()
        }
    }

    Timer {
        id: searchTimer
        interval: 600
        repeat: false
        onTriggered: {
            var text = searchField.text.trim()
            if (text.length < 2 || !backend) return

            if (currentPage === "search" || stationList.count === 0) {
                currentPage = "search"
                backend.searchStations(text)
            }
        }
    }

    Timer {
        id: contentFadeRestart
        interval: 120
        repeat: false
        onTriggered: contentOpacity = 1.0
    }
}
