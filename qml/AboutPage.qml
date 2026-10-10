/*
 * SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.AboutPage {
    id: page
    objectName: "aboutPage"
    title: i18n("About kgamma2")

    signal closeRequested()

    aboutData: ({
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
                emailAddress: "phoenix0919mik@gmail.com",
                webAddress: "https://github.com/PhoenixEmik",
                ocsUsername: ""
            },
            {
                name: "David Edmundson",
                task: i18n("Original author"),
                emailAddress: "davidedmundson@kde.org",
                webAddress: "https://invent.kde.org/davidedmundson/kgamma2",
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
            ToolButton {
                text: i18n("GitHub repository")
                icon.name: "vcs-branch"
                onClicked: Qt.openUrlExternally(page.aboutData.homepage)
            }
        }
    }
}
