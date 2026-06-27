// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Rectangle {
    id: root

    required property var app
    required property bool compactMode

    color: BearTheme.sidebar
    border.color: "transparent"
    border.width: 0
    radius: 0

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            Layout.bottomMargin: 8

            Image {
                Layout.preferredWidth: 34
                Layout.preferredHeight: 34
                source: "qrc:/assets/app/bearwave.png"
                fillMode: Image.PreserveAspectFit
                smooth: true
                mipmap: true
            }

            Image {
                Layout.fillWidth: true
                Layout.preferredHeight: 28
                source: "qrc:/assets/app/bearwave_line.png"
                fillMode: Image.PreserveAspectFit
                horizontalAlignment: Image.AlignLeft
                smooth: true
                mipmap: true
            }
        }

        Label {
            text: qsTr("Stations")
            color: BearTheme.textMuted
            font.bold: true
            font.pixelSize: 10
            Layout.fillWidth: true
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            SidebarButton {
                text: qsTr("Top Global")
                iconText: "🌍"
                isActive: app.currentPage === "top"
                onClicked: app.navigateToTop()
            }

            SidebarButton {
                text: qsTr("Germany")
                iconText: "🇩🇪"
                isActive: app.currentPage === "german"
                onClicked: app.navigateToGerman()
            }

            SidebarButton {
                text: qsTr("Netherlands")
                iconText: "🇳🇱"
                isActive: app.currentPage === "dutch"
                onClicked: app.navigateToDutch()
            }

            SidebarButton {
                text: qsTr("Worldwide")
                iconText: "🌐"
                isActive: app.currentPage === "world"
                onClicked: app.navigateToWorld()
            }
        }

        Label {
            text: qsTr("Library")
            color: BearTheme.textMuted
            font.bold: true
            font.pixelSize: 10
            Layout.fillWidth: true
            Layout.topMargin: 8
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            SidebarButton {
                text: qsTr("Favorites")
                iconText: "★"
                isActive: app.currentPage === "favorites"
                onClicked: app.navigateToFavorites()
            }

            SidebarButton {
                text: qsTr("History")
                iconText: "🕒"
                isActive: app.currentPage === "history"
                onClicked: app.navigateToHistory()
            }
        }

        Item { Layout.fillHeight: true }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            SidebarButton {
                text: qsTr("Add Station")
                iconText: "➕"
                isActive: false
                onClicked: app.addDialog.open()
            }

            SidebarButton {
                text: qsTr("About BearWave")
                iconText: "ℹ️"
                isActive: app.currentPage === "about"
                onClicked: app.navigateToAbout()
            }

            SidebarButton {
                visible: app.backend && app.backend.canResumeLastStation
                text: qsTr("Resume")
                iconText: "↺"
                isActive: false
                onClicked: {
                    if (app.backend) {
                        app.backend.resumeLastStation()
                    }
                }
            }
        }
    }
}