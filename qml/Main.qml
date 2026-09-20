/*
 * SPDX-FileCopyrightText: David Edmundson <davidedmundson@kde.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: window

    width: 520
    height: 360
    visible: true
    title: i18n("Wayland Gamma")
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.None

    minimumHeight: mainLayout.implicitHeight + Kirigami.Units.largeSpacing * 2
    minimumWidth: mainLayout.implicitWidth+ Kirigami.Units.largeSpacing * 2


    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Heading {
            Layout.fillWidth: true
            text: i18n("Monitor correction")
            level: 1
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true

            ComboBox {
                Layout.fillWidth: true
                Kirigami.FormData.label: i18n("Screen:")
                model: correctionSettings.screenNames
                currentIndex: correctionSettings.selectedScreenIndex
                enabled: count > 0
                onActivated: correctionSettings.selectedScreenIndex = currentIndex
            }

            Slider {
                Layout.fillWidth: true
                Kirigami.FormData.label: i18n("Gamma:")
                from: 0.1
                to: 3.0
                stepSize: 0.01
                value: correctionSettings.gamma
                onMoved: correctionSettings.gamma = value
            }

            Label {
                text: correctionSettings.gamma.toFixed(2)
            }

            Slider {
                Layout.fillWidth: true
                Kirigami.FormData.label: i18n("Red:")
                from: 0.0
                to: 2.0
                stepSize: 0.01
                value: correctionSettings.red
                onMoved: correctionSettings.red = value
            }

            Label {
                text: correctionSettings.red.toFixed(2)
            }

            Slider {
                Layout.fillWidth: true
                Kirigami.FormData.label: i18n("Green:")
                from: 0.0
                to: 2.0
                stepSize: 0.01
                value: correctionSettings.green
                onMoved: correctionSettings.green = value
            }

            Label {
                text: correctionSettings.green.toFixed(2)
            }

            Slider {
                Layout.fillWidth: true
                Kirigami.FormData.label: i18n("Blue:")
                from: 0.0
                to: 2.0
                stepSize: 0.01
                value: correctionSettings.blue
                onMoved: correctionSettings.blue = value
            }

            Label {
                text: correctionSettings.blue.toFixed(2)
            }
        }

        Item {
            Layout.fillHeight: true
        }

        Label {
            Layout.fillWidth: true
            visible: correctionSettings.lastSavedProfile.length > 0
            text: i18n("Last saved profile: %1", correctionSettings.lastSavedProfile)
            wrapMode: Text.WrapAnywhere
        }

        Label {
            Layout.fillWidth: true
            visible: correctionSettings.lastError.length > 0
            text: correctionSettings.lastError
            color: Kirigami.Theme.negativeTextColor
            wrapMode: Text.Wrap
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            Button {
                text: i18n("Reset")
                enabled: correctionSettings.screenNames.length > 0
                onClicked: correctionSettings.reset()
            }
            Button {
                text: i18n("Apply")
                enabled: correctionSettings.screenNames.length > 0
                onClicked: correctionSettings.apply()
            }
        }
    }
}
