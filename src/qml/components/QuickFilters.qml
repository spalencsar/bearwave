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

    spacing: 8

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
        spacing: 6

        Label {
            text: qsTr("Genre:")
            color: BearTheme.textMuted
            font.pixelSize: 11
            rightPadding: 10
        }

        Button {
            text: qsTr("Rock")
            highlighted: app.activeQuickFilter === "tag:rock"
            onClicked: loadTag("rock", "genre-rock")
        }

        Button {
            text: qsTr("News")
            highlighted: app.activeQuickFilter === "tag:news"
            onClicked: loadTag("news", "genre-news")
        }

        Button {
            text: qsTr("Jazz")
            highlighted: app.activeQuickFilter === "tag:jazz"
            onClicked: loadTag("jazz", "genre-jazz")
        }

        Item { Layout.preferredWidth: 20 }

        Label {
            text: qsTr("Country:")
            color: BearTheme.textMuted
            font.pixelSize: 11
            rightPadding: 10
        }

        Button {
            text: qsTr("US")
            highlighted: app.activeQuickFilter === "cc:US"
            onClicked: loadCountry("US", "country-us")
        }

        Button {
            text: qsTr("UK")
            highlighted: app.activeQuickFilter === "cc:GB"
            onClicked: loadCountry("GB", "country-gb")
        }

        Button {
            text: qsTr("FR")
            highlighted: app.activeQuickFilter === "cc:FR"
            onClicked: loadCountry("FR", "country-fr")
        }

        Button {
            text: qsTr("WORLD")
            highlighted: app.activeQuickFilter === "world"
            onClicked: openWorld()
        }

        Item { Layout.fillWidth: true }
    }

    ColumnLayout {
        visible: root.compactMode
        Layout.fillWidth: true
        spacing: 8

        Flow {
            Layout.fillWidth: true
            width: parent.width
            spacing: 8

            Label {
                text: qsTr("Genre:")
                color: BearTheme.textMuted
                font.pixelSize: 11
                verticalAlignment: Text.AlignVCenter
                leftPadding: 8
                rightPadding: 16
                topPadding: 8
                bottomPadding: 8
            }

            Button {
                text: qsTr("Rock")
                highlighted: app.activeQuickFilter === "tag:rock"
                onClicked: loadTag("rock", "genre-rock")
            }

            Button {
                text: qsTr("News")
                highlighted: app.activeQuickFilter === "tag:news"
                onClicked: loadTag("news", "genre-news")
            }

            Button {
                text: qsTr("Jazz")
                highlighted: app.activeQuickFilter === "tag:jazz"
                onClicked: loadTag("jazz", "genre-jazz")
            }
        }

        Flow {
            Layout.fillWidth: true
            width: parent.width
            spacing: 8

            Label {
                text: qsTr("Country:")
                color: BearTheme.textMuted
                font.pixelSize: 11
                verticalAlignment: Text.AlignVCenter
                leftPadding: 8
                rightPadding: 16
                topPadding: 8
                bottomPadding: 8
            }

            Button {
                text: qsTr("US")
                highlighted: app.activeQuickFilter === "cc:US"
                onClicked: loadCountry("US", "country-us")
            }

            Button {
                text: qsTr("UK")
                highlighted: app.activeQuickFilter === "cc:GB"
                onClicked: loadCountry("GB", "country-gb")
            }

            Button {
                text: qsTr("FR")
                highlighted: app.activeQuickFilter === "cc:FR"
                onClicked: loadCountry("FR", "country-fr")
            }

            Button {
                text: qsTr("WORLD")
                highlighted: app.activeQuickFilter === "world"
                onClicked: openWorld()
            }
        }
    }
}