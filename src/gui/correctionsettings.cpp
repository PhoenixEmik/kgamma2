// SPDX-FileCopyrightText: David Edmundson <davidedmundson@kde.org>
// SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later
#include "correctionsettings.h"

#include <algorithm>
#include <cmath>

CorrectionSettings::CorrectionSettings(QObject *parent) : QObject(parent)
{
    reload();
    reloadPresets();
}

void CorrectionSettings::updateDraftModified()
{
    bool modified = false;
    if (m_selectedScreenIndex >= 0 && m_selectedScreenIndex < m_statuses.size()) {
        const auto &saved = m_statuses[m_selectedScreenIndex].values;
        modified = m_values.gamma != saved.gamma || m_values.red != saved.red ||
            m_values.green != saved.green || m_values.blue != saved.blue;
    }
    if (m_draftModified != modified) {
        m_draftModified = modified;
        emit presetStateChanged();
    }
}

void CorrectionSettings::setGamma(double value)
{
    if (std::isfinite(value) && value >= GammaRange::minimum && value <= GammaRange::maximum && m_values.gamma != value) { m_values.gamma = value; updateDraftModified(); emit valuesChanged(); }
}
void CorrectionSettings::setRed(double value)
{
    if (std::isfinite(value) && value >= 0 && value <= 2.0 && m_values.red != value) { m_values.red = value; updateDraftModified(); emit valuesChanged(); }
}
void CorrectionSettings::setGreen(double value)
{
    if (std::isfinite(value) && value >= 0 && value <= 2.0 && m_values.green != value) { m_values.green = value; updateDraftModified(); emit valuesChanged(); }
}
void CorrectionSettings::setBlue(double value)
{
    if (std::isfinite(value) && value >= 0 && value <= 2.0 && m_values.blue != value) { m_values.blue = value; updateDraftModified(); emit valuesChanged(); }
}

QStringList CorrectionSettings::screenNames() const
{
    QStringList names;
    for (const auto &status : m_statuses) names.append(status.output.name);
    return names;
}

void CorrectionSettings::setSelectedScreenIndex(int index)
{
    if (index < 0 || index >= m_statuses.size() || index == m_selectedScreenIndex) return;
    m_selectedScreenIndex = index;
    m_values = m_statuses[index].values;
    m_draftModified = false;
    emit selectedScreenIndexChanged();
    emit valuesChanged();
    emit presetStateChanged();
}

void CorrectionSettings::setSelectedPresetIndex(int index)
{
    if (index < 0 || index >= m_presetNames.size() || index == m_selectedPresetIndex) return;
    m_selectedPresetIndex = index;
    emit selectedPresetIndexChanged();
}

QString CorrectionSettings::activePresetLabel() const
{
    const auto name = m_presets.activePreset();
    if (name.isEmpty()) return QStringLiteral("None");
    return name + ((m_presets.isModified() || m_draftModified) ? QStringLiteral(" (modified)") : QString());
}

void CorrectionSettings::reloadPresets()
{
    const QString selected = m_selectedPresetIndex >= 0 && m_selectedPresetIndex < m_presetNames.size()
        ? m_presetNames[m_selectedPresetIndex] : m_presets.activePreset();
    m_presetNames = m_presets.presets();
    m_selectedPresetIndex = m_presetNames.indexOf(selected);
    if (m_selectedPresetIndex < 0 && !m_presetNames.isEmpty()) m_selectedPresetIndex = 0;
    emit presetNamesChanged();
    emit selectedPresetIndexChanged();
    emit presetStateChanged();
}

QString CorrectionSettings::lastSavedProfile() const
{
    if (m_selectedScreenIndex < 0 || m_selectedScreenIndex >= m_statuses.size()) return {};
    const auto &status = m_statuses[m_selectedScreenIndex];
    return status.adjusted ? status.output.iccPath : QString();
}

void CorrectionSettings::setError(const QString &error)
{
    if (error == m_lastError) return;
    m_lastError = error;
    emit lastErrorChanged();
}

void CorrectionSettings::reload()
{
    QString error;
    if (!m_controller.refresh(&error)) { setError(error); return; }
    m_statuses = m_controller.statuses();
    if (m_selectedScreenIndex >= m_statuses.size()) m_selectedScreenIndex = 0;
    m_values = m_statuses.isEmpty() ? GammaValues{} : m_statuses[m_selectedScreenIndex].values;
    m_draftModified = false;
    emit screenNamesChanged();
    emit valuesChanged();
    emit presetStateChanged();
    setError({});
}

bool CorrectionSettings::apply()
{
    if (m_selectedScreenIndex >= m_statuses.size()) { setError(QStringLiteral("No output selected")); return false; }
    QString error;
    if (!m_controller.apply(m_statuses[m_selectedScreenIndex].output.key, m_values, &error)) { setError(error); return false; }
    if (!m_presets.markModified(&error)) { reload(); setError(error); return false; }
    reload();
    return true;
}

bool CorrectionSettings::reset()
{
    if (m_selectedScreenIndex >= m_statuses.size()) { setError(QStringLiteral("No output selected")); return false; }
    const bool wasAdjusted = m_statuses[m_selectedScreenIndex].adjusted;
    QString error;
    if (!m_controller.reset(m_statuses[m_selectedScreenIndex].output.key, &error)) { setError(error); return false; }
    if (wasAdjusted && !m_presets.markModified(&error)) { reload(); setError(error); return false; }
    reload();
    return true;
}

bool CorrectionSettings::savePreset()
{
    if (m_selectedPresetIndex < 0 || m_selectedPresetIndex >= m_presetNames.size()) {
        setError(QStringLiteral("No preset selected"));
        return false;
    }
    return saveNamedPreset(m_presetNames[m_selectedPresetIndex]);
}

bool CorrectionSettings::savePresetAs(const QString &name)
{
    if (m_presets.presets().contains(name)) { setError(QStringLiteral("Preset already exists; use Save to overwrite it")); return false; }
    return saveNamedPreset(name);
}

bool CorrectionSettings::saveNamedPreset(const QString &name)
{
    if (m_statuses.isEmpty()) { setError(QStringLiteral("No connected outputs to save")); return false; }
    auto preset = PresetManager::capture(name, m_statuses);
    if (m_selectedScreenIndex >= 0 && m_selectedScreenIndex < preset.outputs.size()) {
        auto &output = preset.outputs[m_selectedScreenIndex];
        output.values = m_values;
        output.enabled = output.enabled || m_draftModified;
    }
    QString error;
    if (!m_presets.savePreset(preset, &error)) { setError(error); return false; }
    reloadPresets();
    m_selectedPresetIndex = m_presetNames.indexOf(name);
    emit selectedPresetIndexChanged();
    setError({});
    return true;
}

bool CorrectionSettings::applyPreset()
{
    if (m_selectedPresetIndex < 0 || m_selectedPresetIndex >= m_presetNames.size()) { setError(QStringLiteral("No preset selected")); return false; }
    QString error;
    if (!m_presets.applyPreset(m_presetNames[m_selectedPresetIndex], m_controller, &error)) { reload(); setError(error); return false; }
    reload();
    emit presetStateChanged();
    return true;
}

bool CorrectionSettings::deletePreset()
{
    if (m_selectedPresetIndex < 0 || m_selectedPresetIndex >= m_presetNames.size()) { setError(QStringLiteral("No preset selected")); return false; }
    QString error;
    if (!m_presets.removePreset(m_presetNames[m_selectedPresetIndex], &error)) { setError(error); return false; }
    reloadPresets();
    setError({});
    return true;
}
