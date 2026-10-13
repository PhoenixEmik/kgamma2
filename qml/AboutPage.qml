/*
 * SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: page
    objectName: "aboutPage"
    title: i18n("About kgamma2")

    signal closeRequested()

    readonly property var aboutData: ({
        displayName: "kgamma2",
        componentName: "kgamma2",
        desktopFileName: "kgamma2",
        programLogo: "qrc:/icons/kgamma2.svg",
        productName: "kgamma2",
        version: Qt.application.version,
        shortDescription: i18n("Per-monitor gamma and RGB adjustment for KDE Plasma Wayland."),
        homepage: "https://github.com/PhoenixEmik/kgamma2",
        bugAddress: "https://github.com/PhoenixEmik/kgamma2/issues",
        copyrightStatement: "© 2026 PhoenixEmik\n© David Edmundson",
        authors: [
            {
                name: "PhoenixEmik",
                task: i18n("Maintainer"),
                emailAddress: "",
                webAddress: "https://github.com/PhoenixEmik",
                ocsUsername: ""
            },
            {
                name: "David Edmundson",
                task: i18n("Original author"),
                emailAddress: "",
                webAddress: "",
                ocsUsername: ""
            }
        ],
        credits: [],
        translators: [],
        licenses: [{
            name: i18n("GNU Lesser General Public License v2.1 or later"),
            spdx: "LGPL-2.1-or-later",
            text: applicationLicense
        }]
    })

    readonly property real preferredHeight: aboutLayout.implicitHeight + topPadding + bottomPadding
        + footer.implicitHeight + Kirigami.Units.largeSpacing * 2

    ColumnLayout {
        id: aboutLayout
        spacing: Kirigami.Units.largeSpacing * 2

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.largeSpacing * 2
            Kirigami.Icon {
                source: page.aboutData.programLogo
                Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                Layout.preferredHeight: Kirigami.Units.iconSizes.huge
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing
                RowLayout {
                    Layout.fillWidth: true
                    Kirigami.Heading {
                        text: page.aboutData.displayName
                        level: 1
                        font.weight: Font.Bold
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.6
                        wrapMode: Text.Wrap
                    }
                    Label {
                        text: page.aboutData.version
                        color: Kirigami.Theme.textColor
                        opacity: 0.75
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize
                    }
                    Item { Layout.fillWidth: true }
                }
                Label {
                    Layout.fillWidth: true
                    text: page.aboutData.shortDescription
                    wrapMode: Text.Wrap
                }
            }
        }

        GroupBox {
            id: licenseGroup
            title: i18n("License")
            label: Kirigami.Heading {
                text: licenseGroup.title
                level: 3
                font.weight: Font.Bold
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.15
                wrapMode: Text.Wrap
                width: parent.availableWidth
            }
            Layout.fillWidth: true
            padding: Kirigami.Units.largeSpacing
            contentItem: RowLayout {
                spacing: Kirigami.Units.largeSpacing
                ColumnLayout {
                    Layout.fillWidth: true
                    Label {
                        Layout.fillWidth: true
                        text: "LGPL-2.1-or-later"
                        font.bold: true
                        wrapMode: Text.Wrap
                    }
                    Label {
                        Layout.fillWidth: true
                        text: i18n("GNU Lesser General Public License")
                        color: Kirigami.Theme.textColor
                        opacity: 0.75
                        wrapMode: Text.Wrap
                    }
                }
                Button {
                    objectName: "licenseButton"
                    text: i18n("View license")
                    onClicked: licenseDialog.open()
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.largeSpacing
            Button {
                objectName: "homepageButton"
                Layout.fillWidth: true
                text: i18n("Homepage")
                icon.name: "globe"
                onClicked: Qt.openUrlExternally(page.aboutData.homepage)
            }
            Button {
                objectName: "reportBugButton"
                Layout.fillWidth: true
                text: i18n("Report a bug")
                icon.name: "tools-report-bug"
                onClicked: Qt.openUrlExternally(page.aboutData.bugAddress)
            }
        }

        GroupBox {
            id: authorsGroup
            title: i18n("Authors")
            label: Kirigami.Heading {
                text: authorsGroup.title
                level: 3
                font.weight: Font.Bold
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.15
                wrapMode: Text.Wrap
                width: parent.availableWidth
            }
            Layout.fillWidth: true
            padding: Kirigami.Units.largeSpacing
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.largeSpacing
                Repeater {
                    model: page.aboutData.authors
                    delegate: RowLayout {
                        required property var modelData
                        Layout.fillWidth: true
                        Label {
                            Layout.fillWidth: true
                            text: modelData.name
                            wrapMode: Text.Wrap
                        }
                        Label {
                            text: modelData.task
                            color: Kirigami.Theme.textColor
                            opacity: 0.75
                        }
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            Label {
                Layout.fillWidth: true
                text: page.aboutData.copyrightStatement
                color: Kirigami.Theme.textColor
                opacity: 0.75
                wrapMode: Text.Wrap
            }
            Label {
                Layout.fillWidth: true
                text: i18n("Built with Qt %1", qtVersion)
                color: Kirigami.Theme.textColor
                opacity: 0.75
                wrapMode: Text.Wrap
            }
        }
    }

    Dialog {
        id: licenseDialog
        objectName: "licenseDialog"
        title: page.aboutData.licenses[0].name
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(600, parent.width - Kirigami.Units.largeSpacing * 2)
        height: Math.min(500, parent.height - Kirigami.Units.largeSpacing * 2)
        modal: true
        standardButtons: Dialog.Close
        contentItem: ScrollView {
            clip: true
            TextArea {
                text: page.aboutData.licenses[0].text
                readOnly: true
                selectByMouse: true
                wrapMode: Text.Wrap
            }
        }
    }

    footer: ToolBar {
        leftPadding: Kirigami.Units.largeSpacing * 2
        rightPadding: Kirigami.Units.largeSpacing * 2
        topPadding: Kirigami.Units.largeSpacing
        bottomPadding: Kirigami.Units.largeSpacing
        implicitHeight: closeButton.implicitHeight + topPadding + bottomPadding

        contentItem: RowLayout {
            Item { Layout.fillWidth: true }
            Button {
                id: closeButton
                objectName: "aboutCloseButton"
                text: i18n("Close")
                icon.name: "window-close"
                onClicked: page.closeRequested()
            }
        }
    }
}
