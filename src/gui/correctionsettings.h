// SPDX-FileCopyrightText: David Edmundson <davidedmundson@kde.org>
// SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once

#include "gammacontroller.h"
#include "presetmanager.h"
#include <QObject>
#include <QStringList>

class CorrectionSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double gamma READ gamma WRITE setGamma NOTIFY valuesChanged)
    Q_PROPERTY(double red READ red WRITE setRed NOTIFY valuesChanged)
    Q_PROPERTY(double green READ green WRITE setGreen NOTIFY valuesChanged)
    Q_PROPERTY(double blue READ blue WRITE setBlue NOTIFY valuesChanged)
    Q_PROPERTY(QStringList screenNames READ screenNames NOTIFY screenNamesChanged)
    Q_PROPERTY(int selectedScreenIndex READ selectedScreenIndex WRITE setSelectedScreenIndex NOTIFY selectedScreenIndexChanged)
    Q_PROPERTY(QString lastSavedProfile READ lastSavedProfile NOTIFY valuesChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QStringList presetNames READ presetNames NOTIFY presetNamesChanged)
    Q_PROPERTY(int selectedPresetIndex READ selectedPresetIndex WRITE setSelectedPresetIndex NOTIFY selectedPresetIndexChanged)
    Q_PROPERTY(QString activePresetLabel READ activePresetLabel NOTIFY presetStateChanged)
public:
    explicit CorrectionSettings(QObject *parent = nullptr);
    double gamma() const { return m_values.gamma; }
    double red() const { return m_values.red; }
    double green() const { return m_values.green; }
    double blue() const { return m_values.blue; }
    void setGamma(double value);
    void setRed(double value);
    void setGreen(double value);
    void setBlue(double value);
    QStringList screenNames() const;
    int selectedScreenIndex() const { return m_selectedScreenIndex; }
    void setSelectedScreenIndex(int index);
    QString lastSavedProfile() const;
    QString lastError() const { return m_lastError; }
    QStringList presetNames() const { return m_presetNames; }
    int selectedPresetIndex() const { return m_selectedPresetIndex; }
    void setSelectedPresetIndex(int index);
    QString activePresetLabel() const;
    Q_INVOKABLE bool apply();
    Q_INVOKABLE bool reset();
    Q_INVOKABLE bool savePreset();
    Q_INVOKABLE bool savePresetAs(const QString &name);
    Q_INVOKABLE bool applyPreset();
    Q_INVOKABLE bool deletePreset();
signals:
    void valuesChanged();
    void screenNamesChanged();
    void selectedScreenIndexChanged();
    void lastErrorChanged();
    void presetNamesChanged();
    void selectedPresetIndexChanged();
    void presetStateChanged();
private:
    void reload();
    void reloadPresets();
    void updateDraftModified();
    bool saveNamedPreset(const QString &name);
    void setError(const QString &error);
    GammaController m_controller;
    PresetManager m_presets;
    QVector<OutputStatus> m_statuses;
    QStringList m_presetNames;
    GammaValues m_values;
    int m_selectedScreenIndex = 0;
    int m_selectedPresetIndex = -1;
    bool m_draftModified = false;
    QString m_lastError;
};
