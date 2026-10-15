// SPDX-FileCopyrightText: David Edmundson <davidedmundson@kde.org>
// SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later
#include "correctionsettings.h"
#include "main.h"

#include <QGuiApplication>
#include <QFile>
#include <KLocalizedContext>
#include <KLocalizedQmlContext>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QCursor>
#include <QScreen>
#include <QTimer>
#include <QWindow>

int main(int argc, char *argv[])
{
    if (argc > 1) return runCli(argc, argv);

    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kgamma2"));
    app.setApplicationVersion(QStringLiteral(KGAMMA2_VERSION));
    app.setDesktopFileName(QStringLiteral("kgamma2"));
    app.setOrganizationName(QStringLiteral("kgamma2"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("kgamma2"), QIcon(QStringLiteral(":/icons/kgamma2.svg"))));
    QQmlApplicationEngine engine;
    KLocalization::setupLocalizedContext(&engine);
    engine.rootContext()->setContextProperty(QStringLiteral("qtVersion"), QString::fromLatin1(qVersion()));
    CorrectionSettings settings;
    engine.rootContext()->setContextProperty(QStringLiteral("correctionSettings"), &settings);
    QFile license(QStringLiteral(":/licenses/LGPL-2.1-or-later.txt"));
    if (!license.open(QIODevice::ReadOnly)) return 1;
    engine.rootContext()->setContextProperty(QStringLiteral("applicationLicense"), QString::fromUtf8(license.readAll()));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule(QStringLiteral("KGammaWayland"), QStringLiteral("Main"));
    if (engine.rootObjects().isEmpty()) return 1;
    auto *window = qobject_cast<QWindow *>(engine.rootObjects().first());
    if (!window) return 1;
    QTimer::singleShot(0, window, [window] {
        // Wayland placement belongs to the compositor; use the supplied KWin
        // rule there. Other platforms can position the window before mapping.
        if (!QGuiApplication::platformName().startsWith(QStringLiteral("wayland"))) {
            auto *screen = QGuiApplication::screenAt(QCursor::pos());
            if (!screen) screen = window->screen();
            if (screen) {
                window->setScreen(screen);
                const auto area = screen->availableGeometry();
                window->setPosition(area.x() + qMax(0, (area.width() - window->width()) / 2),
                                    area.y() + qMax(0, (area.height() - window->height()) / 2));
            }
        }
        window->show();
    });
    return app.exec();
}
