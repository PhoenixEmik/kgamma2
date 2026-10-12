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

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.largeSpacing
            Kirigami.Icon {
                source: page.aboutData.programLogo
                Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                Layout.preferredHeight: Kirigami.Units.iconSizes.huge
            }
            ColumnLayout {
                Layout.fillWidth: true
                Kirigami.Heading {
                    Layout.fillWidth: true
                    text: page.aboutData.displayName + " " + page.aboutData.version
                    wrapMode: Text.Wrap
                }
                Label {
                    Layout.fillWidth: true
                    text: page.aboutData.shortDescription
                    wrapMode: Text.Wrap
                }
            }
        }

        Label {
            Layout.fillWidth: true
            text: page.aboutData.copyrightStatement
            wrapMode: Text.Wrap
        }

        Kirigami.Heading { text: i18n("License"); level: 2 }
        Button {
            objectName: "licenseButton"
            Layout.fillWidth: true
            text: page.aboutData.licenses[0].name
            onClicked: licenseDialog.open()
        }

        Kirigami.Heading { text: i18n("Project links"); level: 2 }
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

        Kirigami.Heading { text: i18n("Authors"); level: 2 }
        Repeater {
            model: page.aboutData.authors
            delegate: ColumnLayout {
                required property var modelData
                Layout.fillWidth: true
                Label { text: modelData.name }
                Label {
                    Layout.fillWidth: true
                    text: modelData.task
                    opacity: 0.7
                    wrapMode: Text.Wrap
                }
            }
        }

        Kirigami.Heading { text: i18n("Libraries in use"); level: 2 }
        Label {
            Layout.fillWidth: true
            text: i18n("Qt %1", qtVersion)
            wrapMode: Text.Wrap
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
        RowLayout {
            anchors.fill: parent
            ToolButton {
                objectName: "aboutCloseButton"
                text: i18n("Close")
                icon.name: "window-close"
                onClicked: page.closeRequested()
            }
            Item { Layout.fillWidth: true }
        }
    }
}
