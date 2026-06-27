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

    implicitWidth: root.compactMode ? compactNav.implicitWidth : desktopNav.implicitWidth
    implicitHeight: root.compactMode ? compactNav.implicitHeight : desktopNav.implicitHeight

    RowLayout {
        id: desktopNav
        anchors.left: parent.left
        anchors.right: parent.right
        visible: !root.compactMode
        spacing: 8

        Button {
            text: qsTr("Top")
            highlighted: app.currentPage === "top"
            onClicked: app.navigateToTop()
        }

        Button {
            text: qsTr("DE")
            highlighted: app.currentPage === "german"
            onClicked: app.navigateToGerman()
        }

        Button {
            text: qsTr("NL")
            highlighted: app.currentPage === "dutch"
            onClicked: app.navigateToDutch()
        }

        Button {
            text: qsTr("World")
            highlighted: app.currentPage === "world"
            onClicked: app.navigateToWorld()
        }

        Button {
            text: qsTr("Favorites")
            highlighted: app.currentPage === "favorites"
            onClicked: app.navigateToFavorites()
        }

        Button {
            text: qsTr("History")
            highlighted: app.currentPage === "history"
            onClicked: app.navigateToHistory()
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
                highlighted: app.currentPage === "about"
                onClicked: app.navigateToAbout()
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
                onClicked: app.navigateToTop()
            }

            Button {
                text: qsTr("DE")
                highlighted: app.currentPage === "german"
                onClicked: app.navigateToGerman()
            }

            Button {
                text: qsTr("NL")
                highlighted: app.currentPage === "dutch"
                onClicked: app.navigateToDutch()
            }

            Button {
                text: qsTr("Favorites")
                highlighted: app.currentPage === "favorites"
                onClicked: app.navigateToFavorites()
            }

            Button {
                text: qsTr("History")
                highlighted: app.currentPage === "history"
                onClicked: app.navigateToHistory()
            }

            Button {
                text: qsTr("World")
                highlighted: app.currentPage === "world"
                onClicked: app.navigateToWorld()
            }
        }
    }
}