// SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later
#include "gammacontroller.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUuid>

namespace {
QString profilePath(const QString &key)
{
    return ProfileManager::profileDirectory() + QLatin1Char('/') + key + QLatin1Char('/') +
        QUuid::createUuid().toString(QUuid::WithoutBraces) + QStringLiteral(".icc");
}
bool managed(const ManagedOutput &output, const QJsonObject &record)
{
    if (output.source != KScreen::Output::ColorProfileSource::ICC) return false;
    const QString derived = record.value(QStringLiteral("derivedPath")).toString();
    const QString previous = record.value(QStringLiteral("previousPath")).toString();
    return (!derived.isEmpty() && output.iccPath == derived) || (!previous.isEmpty() && output.iccPath == previous);
}
}

GammaController::GammaController()
{
    QFile file(statePath());
    if (!file.exists()) return;
    if (!file.open(QIODevice::ReadOnly)) {
        m_stateError = QStringLiteral("Unable to read saved output state");
        return;
    }
    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        m_stateError = QStringLiteral("Saved output state is invalid");
        return;
    }
    m_state = document.object();
}

QString GammaController::statePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QStringLiteral("/state.json");
}

bool GammaController::saveState(QString *error) const
{
    if (!QDir().mkpath(QFileInfo(statePath()).absolutePath())) {
        *error = QStringLiteral("Unable to create state directory");
        return false;
    }
    QSaveFile file(statePath());
    const auto contents = QJsonDocument(m_state).toJson();
    if (!file.open(QIODevice::WriteOnly) || file.write(contents) != contents.size() || !file.commit()) {
        *error = QStringLiteral("Unable to save output state");
        return false;
    }
    return true;
}

bool GammaController::refresh(QString *error)
{
    if (!m_stateError.isEmpty()) { *error = m_stateError; return false; }
    return m_outputs.refresh(error);
}

OutputStatus GammaController::statusFor(const ManagedOutput &output) const
{
    OutputStatus result;
    result.output = output;
    const auto record = m_state.value(output.key).toObject();
    result.adjusted = !record.isEmpty() && managed(output, record);
    if (result.adjusted) {
        result.originalPath = record.value(QStringLiteral("originalPath")).toString();
        result.originalSource = static_cast<KScreen::Output::ColorProfileSource>(record.value(QStringLiteral("originalSource")).toInt());
        result.values.gamma = record.value(QStringLiteral("gamma")).toDouble(1.0);
        result.values.red = record.value(QStringLiteral("red")).toDouble(1.0);
        result.values.green = record.value(QStringLiteral("green")).toDouble(1.0);
        result.values.blue = record.value(QStringLiteral("blue")).toDouble(1.0);
    }
    return result;
}

QVector<OutputStatus> GammaController::statuses() const
{
    QVector<OutputStatus> result;
    for (const auto &output : m_outputs.outputs()) result.append(statusFor(output));
    return result;
}

bool GammaController::apply(const QString &outputKey, const GammaValues &values, QString *error)
{
    if (!refresh(error)) return false;
    for (const auto &output : m_outputs.outputs()) {
        if (output.key != outputKey) continue;
        const auto oldRecord = m_state.value(output.key).toObject();
        const bool wasManaged = !oldRecord.isEmpty() && managed(output, oldRecord);
        const QString originalPath = wasManaged ? oldRecord.value(QStringLiteral("originalPath")).toString() : output.iccPath;
        const auto originalSource = wasManaged ? static_cast<KScreen::Output::ColorProfileSource>(oldRecord.value(QStringLiteral("originalSource")).toInt()) : output.source;
        const QString basePath = originalSource == KScreen::Output::ColorProfileSource::ICC ? originalPath : QString();
        const QString newPath = profilePath(output.key);
        if (!ProfileManager::writeDerived(basePath, newPath, values, error)) return false;

        QJsonObject record;
        record.insert(QStringLiteral("originalPath"), originalPath);
        record.insert(QStringLiteral("originalSource"), int(originalSource));
        record.insert(QStringLiteral("derivedPath"), newPath);
        record.insert(QStringLiteral("previousPath"), wasManaged ? output.iccPath : QString());
        record.insert(QStringLiteral("gamma"), values.gamma);
        record.insert(QStringLiteral("red"), values.red);
        record.insert(QStringLiteral("green"), values.green);
        record.insert(QStringLiteral("blue"), values.blue);
        m_state.insert(output.key, record);
        if (!saveState(error)) { m_state.insert(output.key, oldRecord); QFile::remove(newPath); return false; }
        if (!m_outputs.setProfile(output.key, newPath, KScreen::Output::ColorProfileSource::ICC, error)) {
            if (oldRecord.isEmpty()) m_state.remove(output.key); else m_state.insert(output.key, oldRecord);
            QString ignored;
            saveState(&ignored);
            QFile::remove(newPath);
            return false;
        }
        record.remove(QStringLiteral("previousPath"));
        m_state.insert(output.key, record);
        if (!saveState(error)) return false;
        if (wasManaged && output.iccPath != newPath) QFile::remove(output.iccPath);
        return true;
    }
    *error = QStringLiteral("Output is no longer available");
    return false;
}

bool GammaController::reset(const QString &outputKey, QString *error)
{
    if (!refresh(error)) return false;
    for (const auto &output : m_outputs.outputs()) {
        if (output.key != outputKey) continue;
        const auto record = m_state.value(output.key).toObject();
        if (record.isEmpty()) return true;
        if (!managed(output, record)) {
            *error = QStringLiteral("Output profile was changed outside kgamma2; refusing to overwrite it");
            return false;
        }
        const QString originalPath = record.value(QStringLiteral("originalPath")).toString();
        const auto source = static_cast<KScreen::Output::ColorProfileSource>(record.value(QStringLiteral("originalSource")).toInt());
        if (!originalPath.isEmpty() && !QFileInfo::exists(originalPath)) {
            *error = QStringLiteral("Original ICC profile is missing: %1").arg(originalPath);
            return false;
        }
        if (!m_outputs.setProfile(output.key, originalPath, source, error)) return false;
        m_state.remove(output.key);
        if (!saveState(error)) return false;
        QFile::remove(record.value(QStringLiteral("derivedPath")).toString());
        const QString previous = record.value(QStringLiteral("previousPath")).toString();
        if (!previous.isEmpty()) QFile::remove(previous);
        return true;
    }
    *error = QStringLiteral("Output is no longer available");
    return false;
}
