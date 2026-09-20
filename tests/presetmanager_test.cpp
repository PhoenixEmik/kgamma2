// SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later
#include "presetmanager.h"

#include <KConfigGroup>
#include <QCoreApplication>
#include <QDebug>
#include <QTemporaryDir>
#include <algorithm>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTemporaryDir temp;
    if (!temp.isValid()) { qCritical() << "Unable to create temporary preset directory:" << temp.errorString(); return 1; }
    const auto path = temp.path() + QStringLiteral("/kgamma2rc");

    QVector<OutputStatus> current(2);
    current[0].output.key = QStringLiteral("stable-dp");
    current[0].output.name = QStringLiteral("DP-1");
    current[0].output.edidHash = QStringLiteral("edid-dell");
    current[0].output.model = QStringLiteral("Dell");
    current[0].output.serial = QStringLiteral("1234");
    current[0].adjusted = true;
    current[0].values = {0.85, 1.0, 0.92, 0.82};
    current[0].originalPath = QStringLiteral("/tmp/original.icc");
    current[0].originalSource = KScreen::Output::ColorProfileSource::ICC;
    current[1].output.key = QStringLiteral("stable-hdmi");
    current[1].output.name = QStringLiteral("HDMI-A-1");
    current[1].output.edidHash = QStringLiteral("edid-lg");
    current[1].adjusted = false;

    PresetManager writer(path);
    QString error;
    if (!writer.savePreset(PresetManager::capture(QStringLiteral("night"), current), &error)) { qCritical() << error; return 1; }
    PresetManager reader(path);
    if (reader.presets() != QStringList{QStringLiteral("night")}) { qCritical() << "Preset list did not persist"; return 1; }
    const auto loaded = reader.loadPreset(QStringLiteral("night"), &error);
    if (!loaded || loaded->outputs.size() != 2) { qCritical() << "Multi-output preset did not round-trip" << error; return 1; }
    const auto dp = std::find_if(loaded->outputs.cbegin(), loaded->outputs.cend(), [](const OutputPreset &output) {
        return output.outputId == QStringLiteral("stable-dp");
    });
    const auto hdmi = std::find_if(loaded->outputs.cbegin(), loaded->outputs.cend(), [](const OutputPreset &output) {
        return output.outputId == QStringLiteral("stable-hdmi");
    });
    if (dp == loaded->outputs.cend() || hdmi == loaded->outputs.cend() || dp->values.gamma != 0.85 ||
        !dp->enabled || dp->originalIcc != QStringLiteral("/tmp/original.icc") || hdmi->enabled) {
        qCritical() << "Multi-output preset content changed";
        return 1;
    }
    if (PresetManager::matchOutput(*dp, current, &error) != 0) { qCritical() << "Stable ID match failed"; return 1; }
    current[0].output.key = QStringLiteral("changed-id");
    current[0].output.name = QStringLiteral("DP-2");
    if (PresetManager::matchOutput(*dp, current, &error) != 0) { qCritical() << "EDID fallback failed"; return 1; }
    current[0].output.edidHash.clear();
    if (PresetManager::matchOutput(*dp, current, &error) != 0) { qCritical() << "Serial fallback failed"; return 1; }
    current[0].output.serial.clear();
    current[0].output.name = QStringLiteral("DP-1");
    if (PresetManager::matchOutput(*dp, current, &error) != 0) { qCritical() << "Connector fallback failed"; return 1; }
    current[1].output.name = QStringLiteral("DP-1");
    if (PresetManager::matchOutput(*dp, current, &error) || error.isEmpty()) {
        qCritical() << "Ambiguous connector was not rejected";
        return 1;
    }
    const auto config = KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    KConfigGroup general(config, QStringLiteral("General"));
    general.writeEntry("ActivePreset", QStringLiteral("night"));
    general.writeEntry("Modified", false);
    if (!config->sync() || reader.currentLabel() != QStringLiteral("night")) {
        qCritical() << "Active preset did not persist";
        return 1;
    }
    if (!reader.savePreset(*loaded, &error) || reader.currentLabel() != QStringLiteral("night")) {
        qCritical() << "Saving unchanged preset marked it modified" << error;
        return 1;
    }
    auto changed = *loaded;
    auto changedDp = std::find_if(changed.outputs.begin(), changed.outputs.end(), [](const OutputPreset &output) {
        return output.outputId == QStringLiteral("stable-dp");
    });
    changedDp->values.gamma = 0.9;
    if (!reader.savePreset(changed, &error) || reader.currentLabel() != QStringLiteral("night (modified)")) {
        qCritical() << "Changed preset did not update current label" << error;
        return 1;
    }
    if (!reader.removePreset(QStringLiteral("night"), &error) || !reader.presets().isEmpty()) {
        qCritical() << "Preset delete failed" << error;
        return 1;
    }
    if (reader.currentLabel() != QStringLiteral("none")) { qCritical() << "Deleted preset remained active"; return 1; }
    return 0;
}
