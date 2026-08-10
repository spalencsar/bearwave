// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

ColumnLayout {
    id: root

    required property var app
    required property bool compactMode

    spacing: 0

    function loadTag(tag, page) {
        if (!app.backend) return
        app.currentPage = page
        app.activeQuickFilter = "tag:" + tag
        app.backend.loadByTag(tag)
    }

    function loadCountry(code, page) {
        if (!app.backend) return
        app.currentPage = page
        app.activeQuickFilter = "cc:" + code
        app.backend.loadByCountryCode(code)
    }

    function openWorld() {
        if (!app.backend) return
        app.currentPage = "world"
        app.activeQuickFilter = "world"
        app.selectedWorldCategory = ""
        app.selectedWorldType = ""
        if (app.backend.countries.length === 0) {
            app.backend.loadCountries()
        }
    }

    RowLayout {
        visible: !root.compactMode
        Layout.fillWidth: true
        spacing: 8

        Label {
            text: qsTr("Quick")
            color: BearTheme.textMuted
            font.pixelSize: 11
            font.bold: true
            rightPadding: 4
            opacity: 0.9
        }

        AppButton {
            text: qsTr("Rock")
            highlighted: app.activeQuickFilter === "tag:rock"
            onClicked: loadTag("rock", "genre-rock")
        }

        AppButton {
            text: qsTr("News")
            highlighted: app.activeQuickFilter === "tag:news"
            onClicked: loadTag("news", "genre-news")
        }

        AppButton {
            text: qsTr("Jazz")
            highlighted: app.activeQuickFilter === "tag:jazz"
            onClicked: loadTag("jazz", "genre-jazz")
        }

        Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 20; color: BearTheme.cardBorder }

        AppButton {
            text: qsTr("US")
            highlighted: app.activeQuickFilter === "cc:US"
            onClicked: loadCountry("US", "country-us")
        }

        AppButton {
            text: qsTr("GB")
            highlighted: app.activeQuickFilter === "cc:GB"
            onClicked: loadCountry("GB", "country-gb")
        }

        AppButton {
            text: qsTr("FR")
            highlighted: app.activeQuickFilter === "cc:FR"
            onClicked: loadCountry("FR", "country-fr")
        }

        AppButton {
            text: qsTr("World")
            highlighted: app.activeQuickFilter === "world"
            onClicked: openWorld()
        }

        Item { Layout.fillWidth: true }

        Label {
            text: qsTr("Sort")
            color: BearTheme.textMuted
            font.pixelSize: 11
            font.bold: true
            rightPadding: 4
        }

        AppButton {
            text: qsTr("Name")
            onClicked: {
                if (app.backend && app.currentPage !== "favorites") {
                    app.backend.sortStations("name")
                }
            }
        }

        AppButton {
            text: qsTr("Bitrate")
            onClicked: {
                if (app.backend && app.currentPage !== "favorites") {
                    app.backend.sortStations("bitrate")
                }
            }
        }

        AppButton {
            text: qsTr("Votes")
            onClicked: {
                if (app.backend && app.currentPage !== "favorites") {
                    app.backend.sortStations("votes")
                }
            }
        }
    }

    ColumnLayout {
        visible: root.compactMode
        Layout.fillWidth: true
        spacing: 8

        Flow {
            Layout.fillWidth: true
            width: parent ? parent.width : 400
            spacing: 8

            Label {
                text: qsTr("Quick")
                color: BearTheme.textMuted
                font.pixelSize: 11
                font.bold: true
                verticalAlignment: Text.AlignVCenter
                leftPadding: 4
                rightPadding: 8
                topPadding: 8
                bottomPadding: 8
            }

            AppButton {
                text: qsTr("Rock")
                highlighted: app.activeQuickFilter === "tag:rock"
                onClicked: loadTag("rock", "genre-rock")
            }

            AppButton {
                text: qsTr("News")
                highlighted: app.activeQuickFilter === "tag:news"
                onClicked: loadTag("news", "genre-news")
            }

            AppButton {
                text: qsTr("Jazz")
                highlighted: app.activeQuickFilter === "tag:jazz"
                onClicked: loadTag("jazz", "genre-jazz")
            }

            AppButton {
                text: qsTr("US")
                highlighted: app.activeQuickFilter === "cc:US"
                onClicked: loadCountry("US", "country-us")
            }

            AppButton {
                text: qsTr("GB")
                highlighted: app.activeQuickFilter === "cc:GB"
                onClicked: loadCountry("GB", "country-gb")
            }

            AppButton {
                text: qsTr("FR")
                highlighted: app.activeQuickFilter === "cc:FR"
                onClicked: loadCountry("FR", "country-fr")
            }

            AppButton {
                text: qsTr("World")
                highlighted: app.activeQuickFilter === "world"
                onClicked: openWorld()
            }
        }

        Flow {
            Layout.fillWidth: true
            width: parent ? parent.width : 400
            spacing: 8

            Label {
                text: qsTr("Sort")
                color: BearTheme.textMuted
                font.pixelSize: 11
                font.bold: true
                verticalAlignment: Text.AlignVCenter
                leftPadding: 4
                rightPadding: 8
                topPadding: 8
                bottomPadding: 8
            }

            AppButton {
                text: qsTr("Name")
                onClicked: {
                    if (app.backend && app.currentPage !== "favorites") {
                        app.backend.sortStations("name")
                    }
                }
            }

            AppButton {
                text: qsTr("Bitrate")
                onClicked: {
                    if (app.backend && app.currentPage !== "favorites") {
                        app.backend.sortStations("bitrate")
                    }
                }
            }

            AppButton {
                text: qsTr("Votes")
                onClicked: {
                    if (app.backend && app.currentPage !== "favorites") {
                        app.backend.sortStations("votes")
                    }
                }
            }
        }
    }
}
