// SPDX-FileCopyrightText: David Edmundson <davidedmundson@kde.org>
// SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later
#include "correctionsettings.h"

#include <algorithm>
#include <cmath>

CorrectionSettings::CorrectionSettings(QObject *parent) : QObject(parent)
{
    reload();
}

void CorrectionSettings::setGamma(double value)
{
    if (std::isfinite(value) && value >= 0.1 && value <= 3.0 && m_values.gamma != value) { m_values.gamma = value; emit valuesChanged(); }
}
void CorrectionSettings::setRed(double value)
{
    if (std::isfinite(value) && value >= 0 && value <= 2.0 && m_values.red != value) { m_values.red = value; emit valuesChanged(); }
}
void CorrectionSettings::setGreen(double value)
{
    if (std::isfinite(value) && value >= 0 && value <= 2.0 && m_values.green != value) { m_values.green = value; emit valuesChanged(); }
}
void CorrectionSettings::setBlue(double value)
{
    if (std::isfinite(value) && value >= 0 && value <= 2.0 && m_values.blue != value) { m_values.blue = value; emit valuesChanged(); }
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
    emit selectedScreenIndexChanged();
    emit valuesChanged();
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
    emit screenNamesChanged();
    emit valuesChanged();
    setError({});
}

bool CorrectionSettings::apply()
{
    if (m_selectedScreenIndex >= m_statuses.size()) { setError(QStringLiteral("No output selected")); return false; }
    QString error;
    if (!m_controller.apply(m_statuses[m_selectedScreenIndex].output.key, m_values, &error)) { setError(error); return false; }
    reload();
    return true;
}

bool CorrectionSettings::reset()
{
    if (m_selectedScreenIndex >= m_statuses.size()) { setError(QStringLiteral("No output selected")); return false; }
    QString error;
    if (!m_controller.reset(m_statuses[m_selectedScreenIndex].output.key, &error)) { setError(error); return false; }
    reload();
    return true;
}
