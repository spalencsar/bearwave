// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0
import components 1.0

ApplicationWindow {
    id: root
    title: qsTr("BearWave")
    width: 1400
    height: 720
    minimumWidth: 900
    minimumHeight: 460
    visible: true

    onClosing: function(close) {
        close.accepted = false
        root.hide()
    }

    property var currentPage: "top"
    property var backend: (typeof radioBackend !== "undefined" ? radioBackend : null)
    readonly property string appVersion: (typeof bearwaveVersion !== "undefined" ? ("" + bearwaveVersion) : Qt.application.version)
    readonly property string appBuildId: (typeof bearwaveBuildId !== "undefined" ? ("" + bearwaveBuildId) : "?")
    readonly property string appLicenseText: (typeof bearwaveLicenseText !== "undefined" ? ("" + bearwaveLicenseText) : "")
    property bool compactMode: width < 900
    property var selectedStation: null
    property real contentOpacity: 1.0
    property string activeQuickFilter: ""
    property string selectedWorldCategory: ""
    property string selectedWorldType: ""
    property string countrySearchText: ""
    property alias searchField: searchToolbar.searchField
    property alias stationList: stationPanel.stationList
    property alias addDialog: addDialogPane
    property alias editDialog: editDialogPane

    function toast(message) {
        toastPopup.show(message)
    }

    function navigateToTop() {
        if (!backend) return
        currentPage = "top"
        activeQuickFilter = ""
        backend.loadTopStations()
    }

    function navigateToGerman() {
        if (!backend) return
        currentPage = "german"
        activeQuickFilter = ""
        backend.loadGermanStations()
    }

    function navigateToDutch() {
        if (!backend) return
        currentPage = "dutch"
        activeQuickFilter = ""
        backend.loadDutchStations()
    }

    function navigateToWorld() {
        currentPage = "world"
        activeQuickFilter = ""
        selectedWorldCategory = ""
        selectedWorldType = ""
    }

    function navigateToFavorites() {
        currentPage = "favorites"
        activeQuickFilter = ""
    }

    function navigateToHistory() {
        currentPage = "history"
        activeQuickFilter = ""
    }

    function navigateToAbout() {
        currentPage = "about"
        activeQuickFilter = ""
    }

    function resetSearchFilter() {
        searchTimer.stop()
        if (searchField.text !== "") {
            searchField.text = ""
        } else if (backend && backend.filterQuery !== "") {
            backend.filterQuery = ""
        }
    }

    function getFilteredCountries() {
        if (!backend || !backend.countries) return [];
        var query = countrySearchText.toLowerCase().trim();
        var list = [];
        if (query === "") {
            list.push({ name: qsTr("Top Global"), code: "GLOBAL" });
        }
        for (var i = 0; i < backend.countries.length; i++) {
            var c = backend.countries[i];
            if (query === "" || c.name.toLowerCase().indexOf(query) !== -1 || c.code.toLowerCase().indexOf(query) !== -1) {
                list.push(c);
            }
        }
        return list;
    }

    function activeModel() {
        if (!backend) {
            return []
        }
        if (currentPage === "favorites") {
            return backend.favoriteStations
        } else if (currentPage === "history") {
            return backend.recentStations
        } else if (currentPage === "world" && selectedWorldCategory === "") {
            return []
        } else {
            return backend.stations
        }
    }

    onCurrentPageChanged: {
        contentOpacity = 0.85
        contentFadeRestart.restart()
        if (currentPage !== "about") {
            selectedStation = null
        }
        if (currentPage !== "search") {
            resetSearchFilter()
        }
        if (currentPage !== "world") {
            selectedWorldCategory = ""
            selectedWorldType = ""
            countrySearchText = ""
        }
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
        color: BearTheme.bgA
    }

    SidebarNavigation {
        id: sidebarNav
        visible: !root.compactMode
        width: root.compactMode ? 0 : 200
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 0
        app: root
        compactMode: root.compactMode
    }

    Item {
        id: workspace
        anchors.left: root.compactMode ? parent.left : sidebarNav.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 0
        anchors.leftMargin: root.compactMode ? 0 : 0

        Rectangle {
            id: headerPanel
            height: currentPage === "about" && !root.compactMode ? 1 : headerContent.implicitHeight + 18
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            radius: 0
            color: BearTheme.panel
            border.color: "transparent"
            clip: true

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: BearTheme.cardBorder
            }

            ColumnLayout {
                id: headerContent
                anchors.fill: parent
                anchors.margins: 9
                spacing: 8
                visible: !(currentPage === "about" && !root.compactMode)

                HeaderNavigation {
                    Layout.fillWidth: true
                    app: root
                    compactMode: root.compactMode
                    visible: root.compactMode
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: BearTheme.cardBorder
                    opacity: 0.6
                    visible: root.compactMode
                }

                SearchToolbar {
                    id: searchToolbar
                    Layout.fillWidth: true
                    app: root
                    compactMode: root.compactMode
                    searchTimer: searchTimer
                    visible: currentPage !== "about"
                }

                QuickFilters {
                    Layout.fillWidth: true
                    app: root
                    compactMode: root.compactMode
                    visible: currentPage !== "about"
                }

                Label {
                    Layout.fillWidth: true
                    visible: backend && backend.lastError.length > 0
                    text: backend ? (qsTr("Error: ") + backend.lastError) : ""
                    color: BearTheme.warn
                    elide: Text.ElideRight
                    font.pixelSize: 12
                }
            }
        }

        PlayerBar {
            id: playerPanel
            height: implicitHeight
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            app: root
        }

        Item {
            id: contentArea
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: headerPanel.bottom
            anchors.bottom: playerPanel.top
            anchors.topMargin: 0
            anchors.bottomMargin: 0
            clip: true
            opacity: contentOpacity

            Behavior on opacity {
                NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
            }

            Rectangle {
                id: stationPanelFrame
                visible: currentPage !== "about"
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: root.compactMode ? parent.right : stationDetails.left
                anchors.rightMargin: root.compactMode ? 0 : 0
                radius: 0
                color: BearTheme.panel
                border.color: "transparent"
                clip: true

                Rectangle {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 1
                    color: BearTheme.cardBorder
                    visible: !root.compactMode
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 0

                    WorldCategoryHeader {
                        Layout.fillWidth: true
                        visible: currentPage === "world" && selectedWorldCategory !== ""
                        app: root
                    }

                    StationListPanel {
                        id: stationPanel
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        app: root
                    }

                    WorldCategories {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        visible: currentPage === "world" && selectedWorldCategory === ""
                        app: root
                        compactMode: root.compactMode
                    }
                }
            }

            StationDetailsPanel {
                id: stationDetails
                visible: !root.compactMode && currentPage !== "about"
                width: 350
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                app: root
            }

            AboutPage {
                visible: currentPage === "about"
                anchors.fill: parent
                compactMode: root.compactMode
                appVersion: root.appVersion
                buildId: root.appBuildId
                licenseText: root.appLicenseText
            }
        }
    }

    LoadingOverlay {
        app: root
    }

    Component.onCompleted: {
        if (backend) {
            backend.loadTopStations()
        }
    }

    AddStationDialog {
        id: addDialogPane
        app: root
        compactMode: root.compactMode
    }

    EditStationDialog {
        id: editDialogPane
        app: root
        compactMode: root.compactMode
    }

    ToastPopup {
        id: toastPopup
        window: root
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
