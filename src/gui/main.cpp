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
    return app.exec();
}
