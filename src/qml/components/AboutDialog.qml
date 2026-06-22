// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Dialog {
    id: root

    required property bool compactMode
    property string appVersion: Qt.application.version
    property string buildId: "?"

    title: qsTr("About BearWave")
    modal: true
    anchors.centerIn: parent
    width: compactMode ? 360 : 480
    standardButtons: Dialog.Ok

    readonly property string versionLine: {
        var version = root.appVersion
        if (version === undefined || version === null || version === "")
            version = Qt.application.version
        var build = root.buildId
        if (build === undefined || build === null || build === "")
            build = "?"
        return qsTr("Version") + " " + String(version) + " · " + qsTr("build") + " " + String(build)
    }

    contentItem: ColumnLayout {
        spacing: 12

        Image {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 72
            Layout.preferredHeight: 72
            source: "qrc:/assets/app/bearwave.png"
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("BearWave")
            color: BearTheme.textMain
            font.bold: true
            font.pixelSize: 22
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Internet Radio Player for KDE")
            color: BearTheme.textMuted
            font.pixelSize: 13
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: root.versionLine
            color: BearTheme.textMuted
            font.pixelSize: 12
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Author: Sebastian Palencsár")
            color: BearTheme.textMain
            font.pixelSize: 13
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 12

            Button {
                text: qsTr("Website")
                onClicked: Qt.openUrlExternally("https://palencsar.pro")
            }

            Button {
                text: qsTr("GitHub")
                onClicked: Qt.openUrlExternally("https://github.com/spalencsar")
            }

            Button {
                text: qsTr("LinkedIn")
                onClicked: Qt.openUrlExternally("https://www.linkedin.com/in/spalencsar/")
            }
        }

        Label {
            Layout.fillWidth: true
            Layout.preferredWidth: root.width - 48
            color: BearTheme.textMuted
            font.pixelSize: 10
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Licensed under GNU GPLv3-or-later. Copyright (c) 2026 Sebastian Palencsar.")
        }
    }
}