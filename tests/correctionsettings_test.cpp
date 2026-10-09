// SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later
#include "correctionsettings.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTemporaryDir>
#include <kscreen/backendmanager_p.h>

int main(int argc, char **argv)
{
    QTemporaryDir temp;
    if (!temp.isValid()) return 1;
    // Keep both KConfig and controller state away from the user's settings.
    qputenv("XDG_CONFIG_HOME", temp.path().toUtf8());
    qputenv("XDG_DATA_HOME", temp.path().toUtf8());
    qputenv("KSCREEN_BACKEND", "Fake");
    qputenv("KSCREEN_BACKEND_INPROCESS", "1");
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("correctionsettings_test"));

    const auto fixturePath = temp.path() + QStringLiteral("/outputs.json");
    QFile fixture(fixturePath);
    const QByteArray config = R"({
        "screen": {
            "id": 1, "minSize": {"width": 1, "height": 1},
            "maxSize": {"width": 8192, "height": 8192},
            "currentSize": {"width": 1920, "height": 1080},
            "maxActiveOutputsCount": 1
        },
        "outputs": [{
            "id": 1, "name": "DP-1", "type": "DisplayPort",
            "connected": true, "enabled": true,
            "currentModeId": "1", "preferredModes": ["1"],
            "modes": [{"id": "1", "name": "1920x1080",
                       "size": {"width": 1920, "height": 1080}, "refreshRate": 60}]
        }]
    })";
    if (!fixture.open(QIODevice::WriteOnly) || fixture.write(config) != config.size()) return 1;
    fixture.close();
    KScreen::BackendManager::instance()->setBackendArgs({{QStringLiteral("TEST_DATA"), fixturePath}});

    CorrectionSettings settings;
    if (settings.screenNames() != QStringList{QStringLiteral("DP-1")}) {
        qCritical() << "Fake output did not load" << settings.lastError();
        return 1;
    }
    if (settings.savePreset() || settings.lastError().isEmpty()) {
        qCritical() << "Saving without a selected preset should fail";
        return 1;
    }
    if (!settings.savePresetAs(QStringLiteral("Zulu")) || !settings.savePresetAs(QStringLiteral("Alpha"))) {
        qCritical() << "Unable to create presets" << settings.lastError();
        return 1;
    }
    // Do not retain presetNames() here: a shared list can keep the old storage
    // alive and hide the dangling reference when Save reloads the list.
    settings.setSelectedPresetIndex(1);
    PresetManager reader;
    for (int i = 0; i < 20; ++i) {
        const double gamma = 1.1 + i * 0.1;
        settings.setGamma(gamma);
        if (!settings.savePreset() || settings.selectedPresetIndex() != 1 || !settings.lastError().isEmpty()) {
            qCritical() << "Save lost the selected preset" << settings.lastError();
            return 1;
        }
        QString error;
        const auto saved = reader.loadPreset(QStringLiteral("Zulu"), &error);
        if (!saved || saved->outputs.size() != 1 || saved->outputs[0].values.gamma != gamma || !saved->outputs[0].enabled) {
            qCritical() << "Save did not persist the current draft" << error;
            return 1;
        }
    }
    return 0;
}
