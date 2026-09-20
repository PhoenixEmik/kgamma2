// SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once

#include "gammacontroller.h"

#include <KSharedConfig>
#include <QList>
#include <QStringList>
#include <optional>

struct OutputPreset {
    QString outputId;
    QString connector;
    QString edidHash;
    QString model;
    QString serial;
    bool enabled = false;
    GammaValues values;
    QString originalIcc;
    KScreen::Output::ColorProfileSource originalSource = KScreen::Output::ColorProfileSource::sRGB;
};

struct GammaPreset {
    QString name;
    QList<OutputPreset> outputs;
};

class PresetManager
{
public:
    explicit PresetManager(const QString &configFile = QStringLiteral("kgamma2rc"));

    QStringList presets() const;
    std::optional<GammaPreset> loadPreset(const QString &name, QString *error) const;
    bool savePreset(const GammaPreset &preset, QString *error);
    bool removePreset(const QString &name, QString *error);
    bool applyPreset(const QString &name, GammaController &controller, QString *error);
    QString activePreset() const;
    bool isModified() const;
    QString currentLabel() const;
    bool markModified(QString *error);

    static GammaPreset capture(const QString &name, const QVector<OutputStatus> &statuses);
    static std::optional<int> matchOutput(const OutputPreset &preset, const QVector<OutputStatus> &statuses, QString *error);

private:
    static bool validName(const QString &name);
    bool setActive(const QString &name, bool modified, QString *error);

    KSharedConfigPtr m_config;
};
