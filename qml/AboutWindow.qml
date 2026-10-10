/*
 * SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

import QtQuick
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: window
    objectName: "aboutWindow"
    title: i18n("About kgamma2")
    visible: false
    flags: Qt.Dialog
    width: Math.min(620, Screen.desktopAvailableWidth)
    height: Math.min(640, Screen.desktopAvailableHeight)
    minimumWidth: Math.min(400, Screen.desktopAvailableWidth)
    minimumHeight: Math.min(350, Screen.desktopAvailableHeight)
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.None

    pageStack.initialPage: AboutPage {
        onCloseRequested: window.close()
    }
}
