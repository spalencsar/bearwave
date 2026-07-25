// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Dialog {
    id: root

    required property var app
    required property bool compactMode

    title: qsTr("Add station manually")
    modal: true
    anchors.centerIn: parent
    width: compactMode ? 320 : 420
    footer: DialogButtonBox {
        standardButtons: DialogButtonBox.Ok | DialogButtonBox.Cancel
        delegate: AppButton {}
        onAccepted: root.accept()
        onRejected: root.reject()
    }

    contentItem: ColumnLayout {
        spacing: 8
        TextField { id: manualName; Layout.fillWidth: true; placeholderText: qsTr("Name") }
        TextField { id: manualUrl; Layout.fillWidth: true; placeholderText: qsTr("Stream URL (http/https)") }
        TextField { id: manualCountry; Layout.fillWidth: true; placeholderText: qsTr("Country (optional)") }
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
