/*
 * SPDX-FileCopyrightText: David Edmundson <davidedmundson@kde.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "correctionsettings.h"

#include <QGuiApplication>
#include <KLocalizedContext>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kgamma_wayland"));
    app.setOrganizationName(QStringLiteral("kgamma_wayland"));

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    CorrectionSettings correctionSettings;
    engine.rootContext()->setContextProperty(QStringLiteral("correctionSettings"), &correctionSettings);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        [] {
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.loadFromModule(QStringLiteral("KGammaWayland"), QStringLiteral("Main"));

    return app.exec();
}
