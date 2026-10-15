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
    height: minimumHeight + Kirigami.Units.largeSpacing * 2
    // Show after the initial layout and startup placement have been resolved.
    visible: false
    title: i18n("Wayland Gamma")
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.None

    minimumWidth: 500
    minimumHeight: Math.ceil(mainLayout.implicitHeight + mainPage.topPadding + mainPage.bottomPadding)

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

    AboutWindow {
        id: aboutWindow
        transientParent: window
    }

    pageStack.initialPage: Kirigami.Page {
        id: mainPage
        padding: Kirigami.Units.largeSpacing * 2
        ColumnLayout {
            id: mainLayout
            anchors.fill: parent
            spacing: Kirigami.Units.largeSpacing

            RowLayout {
                Layout.fillWidth: true
                Kirigami.Heading {
                    Layout.fillWidth: true
                    text: i18n("Monitor correction")
                    level: 1
                    font.weight: Font.Bold
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.6
                    wrapMode: Text.Wrap
                }
                ToolButton {
                    objectName: "aboutButton"
                    text: i18n("About")
                    icon.name: "help-about"
                    onClicked: {
                        aboutWindow.show()
                        aboutWindow.raise()
                        aboutWindow.requestActivate()
                    }
                }
            }

            GroupBox {
                id: presetsGroup
                title: i18n("Presets")
                label: Kirigami.Heading {
                    text: presetsGroup.title
                    level: 3
                    font.weight: Font.Bold
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.15
                    wrapMode: Text.Wrap
                    width: parent.availableWidth
                }
                Layout.fillWidth: true
                padding: Kirigami.Units.largeSpacing
                Layout.topMargin: Kirigami.Units.smallSpacing

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
                            color: Kirigami.Theme.textColor
                            opacity: 0.75
                            ToolTip.text: text
                            ToolTip.visible: presetStatusHover.hovered
                            HoverHandler { id: presetStatusHover }
                        }

                        ToolButton {
                            text: i18n("Save")
                            enabled: correctionSettings.selectedPresetIndex >= 0
                            onClicked: correctionSettings.savePreset()
                        }

                        ToolButton {
                            text: i18n("Save As…")
                            enabled: correctionSettings.screenNames.length > 0
                            onClicked: {
                                presetNameField.text = ""
                                saveAsDialog.open()
                            }
                        }

                        ToolButton {
                            text: i18n("Delete")
                            enabled: correctionSettings.selectedPresetIndex >= 0
                            onClicked: correctionSettings.deletePreset()
                        }
                    }
                }
            }

            GroupBox {
                id: monitorGroup
                title: i18n("Selected monitor")
                label: Kirigami.Heading {
                    text: monitorGroup.title
                    level: 3
                    font.weight: Font.Bold
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.15
                    wrapMode: Text.Wrap
                    width: parent.availableWidth
                }
                Layout.fillWidth: true
                padding: Kirigami.Units.largeSpacing
                Layout.topMargin: Kirigami.Units.smallSpacing

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
                        rowSpacing: Kirigami.Units.smallSpacing

                        Label { text: i18n("Gamma"); Layout.preferredWidth: Kirigami.Units.gridUnit * 4 }
                        Slider {
                            Layout.fillWidth: true
                            from: -1.0
                            to: 1.0
                            stepSize: 0.001
                            value: Math.log(correctionSettings.gamma) / Math.LN10
                            enabled: correctionSettings.screenNames.length > 0
                            Accessible.name: i18n("Gamma")
                            onMoved: correctionSettings.gamma = Math.round(Math.pow(10, value) * 100) / 100
                        }
                        Label { text: correctionSettings.gamma.toFixed(2); Layout.preferredWidth: Kirigami.Units.gridUnit * 3; horizontalAlignment: Text.AlignRight; font.family: "monospace" }

                        RowLayout {
                            Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                            spacing: Kirigami.Units.smallSpacing
                            Rectangle {
                                width: Kirigami.Units.smallSpacing * 2
                                height: width
                                radius: width / 2
                                color: "#da4453"
                            }
                            Label { text: i18n("Red") }
                            Item { Layout.fillWidth: true }
                        }
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
                        Label { text: correctionSettings.red.toFixed(2); Layout.preferredWidth: Kirigami.Units.gridUnit * 3; horizontalAlignment: Text.AlignRight; font.family: "monospace" }

                        RowLayout {
                            Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                            spacing: Kirigami.Units.smallSpacing
                            Rectangle {
                                width: Kirigami.Units.smallSpacing * 2
                                height: width
                                radius: width / 2
                                color: "#27ae60"
                            }
                            Label { text: i18n("Green") }
                            Item { Layout.fillWidth: true }
                        }
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
                        Label { text: correctionSettings.green.toFixed(2); Layout.preferredWidth: Kirigami.Units.gridUnit * 3; horizontalAlignment: Text.AlignRight; font.family: "monospace" }

                        RowLayout {
                            Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                            spacing: Kirigami.Units.smallSpacing
                            Rectangle {
                                width: Kirigami.Units.smallSpacing * 2
                                height: width
                                radius: width / 2
                                color: "#3daee9"
                            }
                            Label { text: i18n("Blue") }
                            Item { Layout.fillWidth: true }
                        }
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
                        Label { text: correctionSettings.blue.toFixed(2); Layout.preferredWidth: Kirigami.Units.gridUnit * 3; horizontalAlignment: Text.AlignRight; font.family: "monospace" }
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

            Kirigami.Separator { Layout.fillWidth: true }

            RowLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                Item { Layout.fillWidth: true }
                Button {
                    text: i18n("Reset")
                    enabled: correctionSettings.screenNames.length > 0
                    onClicked: correctionSettings.reset()
                }

                Button {
                    objectName: "applyChangesButton"
                    text: i18n("Apply Changes")
                    highlighted: true
                    enabled: correctionSettings.screenNames.length > 0
                    onClicked: correctionSettings.apply()
                }
            }
        }
    }
}
