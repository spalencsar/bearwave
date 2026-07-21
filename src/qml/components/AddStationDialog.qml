// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Dialog {
    id: root

    required property var app
    required property bool compactMode

    title: qsTr("Add station manually")
    modal: true
    anchors.centerIn: parent
    width: compactMode ? 320 : 420
    standardButtons: Dialog.NoButton

    background: Rectangle {
        color: BearTheme.panel
        border.color: BearTheme.cardBorder
        border.width: 1
        radius: 8
    }

    header: Label {
        text: root.title
        color: BearTheme.textMain
        font.bold: true
        font.pixelSize: 14
        leftPadding: 16
        rightPadding: 16
        topPadding: 14
        bottomPadding: 6
    }

    contentItem: ColumnLayout {
        spacing: 8
        TextField {
            id: manualName
            Layout.fillWidth: true
            placeholderText: qsTr("Name")
        }
        TextField {
            id: manualUrl
            Layout.fillWidth: true
            placeholderText: qsTr("Stream URL (http/https)")
        }
        TextField {
            id: manualCountry
            Layout.fillWidth: true
            placeholderText: qsTr("Country (optional)")
        }
    }

    footer: DialogButtonBox {
        alignment: Qt.AlignRight
        background: Item {}
        padding: 12
        spacing: 8

        ThemedButton {
            text: qsTr("Cancel")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
        ThemedButton {
            text: qsTr("OK")
            primary: true
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
        }
    }

    onAccepted: {
        if (app.backend) {
            app.backend.addManualStation(manualName.text, manualUrl.text, manualCountry.text)
            app.toast(qsTr("Station added"))
        }
        manualName.text = ""
        manualUrl.text = ""
        manualCountry.text = ""
    }
}
