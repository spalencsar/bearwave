// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import theme 1.0
import components 1.0

ApplicationWindow {
    id: root
    title: qsTr("BearWave")
    // Outside KDE: own chrome. KDE sessions keep system window decorations.
    readonly property bool clientChrome: (typeof bearwaveClientChrome !== "undefined")
                                         ? !!bearwaveClientChrome
                                         : false
    flags: clientChrome ? (Qt.Window | Qt.FramelessWindowHint) : Qt.Window
    color: BearTheme.bgA
    readonly property int preferredWindowWidth: 1440
    // Sidebar hides only when very narrow.
    readonly property int sidebarLayoutThreshold: 760
    // Detail stage needs ~420px width; hide below this so list stays usable.
    readonly property int detailLayoutThreshold: 1240
    // Transport dock in the stage needs vertical room (cover + track + dock).
    // At ~900px height (e.g. MacBook Air 2015) keep a bottom player bar instead.
    readonly property int stageDockHeightThreshold: 960
    width: Math.max(minimumWidth,
                    Math.min(preferredWindowWidth,
                             Screen.desktopAvailableWidth > 0
                             ? Screen.desktopAvailableWidth - 80
                             : preferredWindowWidth))
    height: Math.max(minimumHeight,
                     Math.min(820,
                              Screen.desktopAvailableHeight > 0
                              ? Screen.desktopAvailableHeight - 80
                              : 720))
    minimumWidth: 640
    minimumHeight: 600
    visible: true

    onClosing: function(close) {
        close.accepted = false
        root.hide()
    }

    property var currentPage: "top"
    property var backend: (typeof radioBackend !== "undefined" ? radioBackend : null)
    readonly property string appVersion: (typeof bearwaveVersion !== "undefined" ? ("" + bearwaveVersion) : Qt.application.version)
    readonly property string appBuildId: (typeof bearwaveBuildId !== "undefined" ? ("" + bearwaveBuildId) : "?")
    readonly property string appChangelog: (typeof bearwaveChangelog !== "undefined" ? ("" + bearwaveChangelog) : "")
    // No sidebar (very narrow).
    property bool compactMode: width < sidebarLayoutThreshold
    // Full three-column layout (sidebar + list + detail).
    readonly property bool showDetailPanel: width >= detailLayoutThreshold
    // Dock transport in the right stage only when wide enough *and* tall enough.
    readonly property bool stageTransportDock: showDetailPanel && height >= stageDockHeightThreshold
    // Wrapped/flow filters when the one-line filter row would clip.
    readonly property bool useFlowFilters: !showDetailPanel
    property real contentOpacity: 1.0
    property string activeQuickFilter: ""
    property string selectedWorldCategory: ""
    property string selectedWorldType: ""
    property string countrySearchText: ""
    property alias searchField: searchToolbar.searchField
    property alias stationList: stationPanel.stationList
    property alias addDialog: addDialogPane
    property alias editDialog: editDialogPane
    property alias aboutDialog: aboutDialogPane

    function toast(message) {
        toastPopup.show(message)
    }

    function resetSearchFilter() {
        searchTimer.stop()
        if (searchField.text !== "") {
            searchField.text = ""
        }
        if (compactSearchToolbar.searchField.text !== "") {
            compactSearchToolbar.searchField.text = ""
        } else if (backend && backend.filterQuery !== "") {
            backend.filterQuery = ""
        }
    }

    function focusSearch() {
        if (compactMode) {
            compactSearchToolbar.searchField.forceActiveFocus()
        } else {
            searchField.forceActiveFocus()
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
            if (backend.countryMatches(c, query)) {
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
        } else if (currentPage === "mystations") {
            return backend.manualStations
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
        onActivated: focusSearch()
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: BearTheme.bgA }
            GradientStop { position: 1.0; color: BearTheme.bgB }
        }
    }

    SidebarNavigation {
        id: sidebar
        width: root.compactMode ? 0 : 228
        visible: !root.compactMode
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        app: root
    }

    Item {
        id: workspace
        anchors.left: root.compactMode ? parent.left : sidebar.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        Rectangle {
            id: topToolbar
            // Size to header content so Flow filters are never clipped.
            height: headerContent.implicitHeight + 22
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            // Seamless with workspace — no heavy KDE toolwindow frame.
            color: BearTheme.bgA
            border.width: 0
            clip: true

            // Soft separator under header
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: BearTheme.cardBorder
                opacity: 0.55
            }

            // Drag the frameless window from the toolbar (Wayland: startSystemMove).
            DragHandler {
                enabled: root.clientChrome
                target: null
                acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
                onActiveChanged: if (active) root.startSystemMove()
            }

            ColumnLayout {
                id: headerContent
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                anchors.topMargin: 10
                spacing: 8

                HeaderNavigation {
                    Layout.fillWidth: true
                    visible: root.compactMode
                    app: root
                    compactMode: root.compactMode
                }

                // Branding only lives in the sidebar wordmark — not repeated here.
                RowLayout {
                    Layout.fillWidth: true
                    visible: !root.compactMode
                    spacing: 12

                    SearchToolbar {
                        id: searchToolbar
                        Layout.fillWidth: true
                        app: root
                        compactMode: false
                        searchTimer: searchTimer
                    }
                }

                SearchToolbar {
                    id: compactSearchToolbar
                    Layout.fillWidth: true
                    visible: root.compactMode
                    app: root
                    compactMode: true
                    searchTimer: searchTimer
                }

                QuickFilters {
                    Layout.fillWidth: true
                    app: root
                    // Flow layout whenever the one-line filter row would clip.
                    compactMode: root.useFlowFilters
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

        // Bottom bar when stage dock is off:
        // - no stage (narrow): full bar with station meta
        // - stage open but short: transport-only strip
        // Tall+wide: transport docks inside StationDetailPanel (no bottom bar).
        PlayerBar {
            id: playerBar
            height: visible ? implicitHeight : 0
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            app: root
            visible: !root.stageTransportDock
            transportOnly: root.showDetailPanel && !root.stageTransportDock
        }

        Item {
            id: contentArea
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: topToolbar.bottom
            anchors.bottom: root.stageTransportDock ? parent.bottom : playerBar.top
            clip: true
            opacity: contentOpacity

            Behavior on opacity {
                NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
            }

            Rectangle {
                id: stationArea
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                // Gap before the stage so list chrome never abuts the right panel.
                anchors.right: root.showDetailPanel ? detailPanel.left : parent.right
                anchors.rightMargin: root.showDetailPanel ? 0 : 0
                color: BearTheme.bgA
                border.width: 0
                clip: true

                // Soft divider owned by the list column (not by card borders).
                Rectangle {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    width: 1
                    visible: root.showDetailPanel
                    color: BearTheme.cardBorder
                    opacity: 0.45
                    z: 2
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: root.showDetailPanel ? 20 : 10
                    anchors.topMargin: 10
                    anchors.bottomMargin: 10
                    spacing: 10

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
                        compactMode: root.useFlowFilters
                    }
                }
            }

            StationDetailPanel {
                id: detailPanel
                width: 420
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                visible: root.showDetailPanel
                app: root
                showTransportDock: root.stageTransportDock
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

    AboutDialog {
        id: aboutDialogPane
        compactMode: root.compactMode
        appVersion: root.appVersion
        buildId: root.appBuildId
        changelogDocument: root.appChangelog
        languageSettings: (typeof appLanguageSettings !== "undefined" ? appLanguageSettings : null)
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
            var text = (compactMode ? compactSearchToolbar.searchField.text : searchField.text).trim()
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
