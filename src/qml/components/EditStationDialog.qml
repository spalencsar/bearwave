// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Dialog {
    id: root

    required property var app
    required property bool compactMode

    property var stationObject: null

    title: qsTr("Edit station")
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
        TextField { id: editName; Layout.fillWidth: true; placeholderText: qsTr("Name") }
        TextField { id: editUrl; Layout.fillWidth: true; placeholderText: qsTr("Stream URL (http/https)") }
        TextField { id: editCountry; Layout.fillWidth: true; placeholderText: qsTr("Country (optional)") }
    }

    function setupAndOpen() {
        if (stationObject) {
            editName.text = stationObject.name || ""
            editUrl.text = stationObject.url || ""
            editCountry.text = (stationObject.country === qsTr("Manual") ? "" : (stationObject.country || ""))
            open()
        }
    }

    function clearFields() {
        stationObject = null
        editName.text = ""
        editUrl.text = ""
        editCountry.text = ""
    }

    onAccepted: {
        if (app.backend && stationObject) {
            app.backend.editManualStation(stationObject, editName.text, editUrl.text, editCountry.text)
            app.toast(qsTr("Station updated"))
        }
        clearFields()
    }

    onRejected: clearFields()
}
