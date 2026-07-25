// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

RowLayout {
    id: root

    required property var app

    spacing: 12

    AppButton {
        text: qsTr("← Back to Categories")
        onClicked: {
            app.selectedWorldCategory = ""
            app.selectedWorldType = ""
        }
    }

    Label {
        text: app.selectedWorldType === "country"
              ? (qsTr("World > Country: ") + app.selectedWorldCategory)
              : (qsTr("World > Genre: ") + app.selectedWorldCategory)
        color: BearTheme.textMain
        font.bold: true
        font.pixelSize: 13
        Layout.fillWidth: true
        elide: Text.ElideRight
    }
}