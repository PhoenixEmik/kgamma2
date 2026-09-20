/*
 * SPDX-FileCopyrightText: David Edmundson <davidedmundson@kde.org>
 * SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: window

    width: 620
    height: 350
    visible: true
    title: i18n("Wayland Gamma")
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.None

    minimumWidth: 500
    minimumHeight: mainLayout.implicitHeight + Kirigami.Units.largeSpacing * 2

    Dialog {
        id: saveAsDialog
        title: i18n("Save Preset As")
        modal: true
        anchors.centerIn: parent
        width: Math.min(380, window.width - Kirigami.Units.largeSpacing * 2)
        standardButtons: Dialog.Save | Dialog.Cancel
        onOpened: presetNameField.forceActiveFocus()
        onAccepted: correctionSettings.savePresetAs(presetNameField.text.trim())

        TextField {
            id: presetNameField
            width: parent.width
            placeholderText: i18n("Preset name")
            onAccepted: saveAsDialog.accept()
        }
    }

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

        GroupBox {
            title: i18n("Presets")
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing

                    ComboBox {
                        Layout.fillWidth: true
                        model: correctionSettings.presetNames
                        currentIndex: correctionSettings.selectedPresetIndex
                        displayText: count > 0 ? currentText : i18n("No presets saved")
                        enabled: count > 0
                        Accessible.name: i18n("Preset")
                        onActivated: correctionSettings.selectedPresetIndex = currentIndex
                    }

                    Button {
                        text: i18n("Apply Preset")
                        enabled: correctionSettings.selectedPresetIndex >= 0
                        onClicked: correctionSettings.applyPreset()
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing

                    Label {
                        Layout.fillWidth: true
                        text: i18n("Current: %1", correctionSettings.activePresetLabel)
                        elide: Text.ElideRight
                    }

                    Button {
                        text: i18n("Save")
                        enabled: correctionSettings.selectedPresetIndex >= 0
                        onClicked: correctionSettings.savePreset()
                    }

                    Button {
                        text: i18n("Save As…")
                        enabled: correctionSettings.screenNames.length > 0
                        onClicked: {
                            presetNameField.text = ""
                            saveAsDialog.open()
                        }
                    }

                    Button {
                        text: i18n("Delete")
                        enabled: correctionSettings.selectedPresetIndex >= 0
                        onClicked: correctionSettings.deletePreset()
                    }
                }
            }
        }

        GroupBox {
            title: i18n("Selected monitor")
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing

                    Label {
                        text: i18n("Screen")
                        Layout.preferredWidth: 70
                    }

                    ComboBox {
                        Layout.fillWidth: true
                        model: correctionSettings.screenNames
                        currentIndex: correctionSettings.selectedScreenIndex
                        enabled: count > 0
                        Accessible.name: i18n("Screen")
                        onActivated: correctionSettings.selectedScreenIndex = currentIndex
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 3
                    columnSpacing: Kirigami.Units.smallSpacing
                    rowSpacing: 0

                    Label { text: i18n("Gamma"); Layout.preferredWidth: 70 }
                    Slider {
                        Layout.fillWidth: true
                        from: 0.1
                        to: 3.0
                        stepSize: 0.01
                        value: correctionSettings.gamma
                        enabled: correctionSettings.screenNames.length > 0
                        Accessible.name: i18n("Gamma")
                        onMoved: correctionSettings.gamma = value
                    }
                    Label { text: correctionSettings.gamma.toFixed(2); Layout.preferredWidth: 42; horizontalAlignment: Text.AlignRight }

                    Label { text: i18n("Red"); Layout.preferredWidth: 70 }
                    Slider {
                        Layout.fillWidth: true
                        from: 0.0
                        to: 2.0
                        stepSize: 0.01
                        value: correctionSettings.red
                        enabled: correctionSettings.screenNames.length > 0
                        Accessible.name: i18n("Red")
                        onMoved: correctionSettings.red = value
                    }
                    Label { text: correctionSettings.red.toFixed(2); Layout.preferredWidth: 42; horizontalAlignment: Text.AlignRight }

                    Label { text: i18n("Green"); Layout.preferredWidth: 70 }
                    Slider {
                        Layout.fillWidth: true
                        from: 0.0
                        to: 2.0
                        stepSize: 0.01
                        value: correctionSettings.green
                        enabled: correctionSettings.screenNames.length > 0
                        Accessible.name: i18n("Green")
                        onMoved: correctionSettings.green = value
                    }
                    Label { text: correctionSettings.green.toFixed(2); Layout.preferredWidth: 42; horizontalAlignment: Text.AlignRight }

                    Label { text: i18n("Blue"); Layout.preferredWidth: 70 }
                    Slider {
                        Layout.fillWidth: true
                        from: 0.0
                        to: 2.0
                        stepSize: 0.01
                        value: correctionSettings.blue
                        enabled: correctionSettings.screenNames.length > 0
                        Accessible.name: i18n("Blue")
                        onMoved: correctionSettings.blue = value
                    }
                    Label { text: correctionSettings.blue.toFixed(2); Layout.preferredWidth: 42; horizontalAlignment: Text.AlignRight }
                }

                Label {
                    Layout.fillWidth: true
                    visible: correctionSettings.lastSavedProfile.length > 0
                    text: i18n("Correction active on this monitor")
                    opacity: 0.7
                    ToolTip.text: correctionSettings.lastSavedProfile
                    ToolTip.visible: hoverHandler.hovered
                    HoverHandler { id: hoverHandler }
                }
            }
        }

        Item { Layout.fillHeight: true }

        Label {
            Layout.fillWidth: true
            visible: correctionSettings.lastError.length > 0
            text: correctionSettings.lastError
            color: Kirigami.Theme.negativeTextColor
            wrapMode: Text.Wrap
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: Kirigami.Units.smallSpacing

            Button {
                text: i18n("Reset")
                enabled: correctionSettings.screenNames.length > 0
                onClicked: correctionSettings.reset()
            }

            Button {
                text: i18n("Apply Changes")
                highlighted: true
                enabled: correctionSettings.screenNames.length > 0
                onClicked: correctionSettings.apply()
            }
        }
    }
}
