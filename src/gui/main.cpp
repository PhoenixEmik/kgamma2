// SPDX-FileCopyrightText: David Edmundson <davidedmundson@kde.org>
// SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later
#include "correctionsettings.h"
#include "main.h"

#include <QGuiApplication>
#include <KLocalizedContext>
#include <KLocalizedQmlContext>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    if (argc > 1) return runCli(argc, argv);

    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kgamma2"));
    app.setDesktopFileName(QStringLiteral("kgamma2"));
    app.setOrganizationName(QStringLiteral("kgamma2"));
    QQmlApplicationEngine engine;
    KLocalization::setupLocalizedContext(&engine);
    CorrectionSettings settings;
    engine.rootContext()->setContextProperty(QStringLiteral("correctionSettings"), &settings);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule(QStringLiteral("KGammaWayland"), QStringLiteral("Main"));
    return app.exec();
}
