// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Item {
    id: root

    required property var app
    required property bool compactMode

    implicitWidth: compactMode ? compactNav.implicitWidth : desktopNav.implicitWidth
    implicitHeight: compactMode ? compactNav.implicitHeight : desktopNav.implicitHeight

    function loadTop() {
        if (!app.backend) return
        app.currentPage = "top"
        app.activeQuickFilter = ""
        app.backend.loadTopStations()
    }

    function loadGerman() {
        if (!app.backend) return
        app.currentPage = "german"
        app.activeQuickFilter = ""
        app.backend.loadGermanStations()
    }

    function loadDutch() {
        if (!app.backend) return
        app.currentPage = "dutch"
        app.activeQuickFilter = ""
        app.backend.loadDutchStations()
    }

    function showFavorites() {
        app.currentPage = "favorites"
        app.activeQuickFilter = ""
    }

    function showHistory() {
        app.currentPage = "history"
        app.activeQuickFilter = ""
    }

    RowLayout {
        id: desktopNav
        anchors.left: parent.left
        anchors.right: parent.right
        visible: !root.compactMode
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
            color: BearTheme.cardBorder
            opacity: 0.6
        }

        Button {
            text: qsTr("Top")
            highlighted: app.currentPage === "top"
            onClicked: loadTop()
        }

        Button {
            text: qsTr("DE")
            highlighted: app.currentPage === "german"
            onClicked: loadGerman()
        }

        Button {
            text: qsTr("NL")
            highlighted: app.currentPage === "dutch"
            onClicked: loadDutch()
        }

        Button {
            text: qsTr("Favorites")
            highlighted: app.currentPage === "favorites"
            onClicked: showFavorites()
        }

        Button {
            text: qsTr("History")
            highlighted: app.currentPage === "history"
            onClicked: showHistory()
        }

        Item { Layout.fillWidth: true }

        Button {
            text: qsTr("Manual +")
            onClicked: app.addDialog.open()
        }

        Button {
            text: qsTr("About")
            onClicked: app.aboutDialog.open()
        }

        Button {
            visible: app.backend && app.backend.canResumeLastStation
            text: qsTr("Resume")
            onClicked: {
                if (app.backend) {
                    app.backend.resumeLastStation()
                }
            }
        }
    }

    ColumnLayout {
        id: compactNav
        anchors.left: parent.left
        anchors.right: parent.right
        visible: root.compactMode
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
                onClicked: app.addDialog.open()
            }

            Button {
                text: qsTr("About")
                onClicked: app.aboutDialog.open()
            }

            Button {
                visible: app.backend && app.backend.canResumeLastStation
                text: "↺"
                onClicked: {
                    if (app.backend) {
                        app.backend.resumeLastStation()
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
                highlighted: app.currentPage === "top"
                onClicked: loadTop()
            }

            Button {
                text: qsTr("DE")
                highlighted: app.currentPage === "german"
                onClicked: loadGerman()
            }

            Button {
                text: qsTr("NL")
                highlighted: app.currentPage === "dutch"
                onClicked: loadDutch()
            }

            Button {
                text: qsTr("Favorites")
                highlighted: app.currentPage === "favorites"
                onClicked: showFavorites()
            }

            Button {
                text: qsTr("History")
                highlighted: app.currentPage === "history"
                onClicked: showHistory()
            }
        }
    }
}