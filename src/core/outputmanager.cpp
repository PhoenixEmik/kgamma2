// SPDX-License-Identifier: LGPL-2.1-or-later
#include "outputmanager.h"

#include <QCryptographicHash>
#include <algorithm>
#include <kscreen/configoperation.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/setconfigoperation.h>

bool OutputManager::refresh(QString *error)
{
    KScreen::GetConfigOperation operation;
    if (!operation.exec() || operation.hasError() || !operation.config()) {
        *error = operation.errorString().isEmpty() ? QStringLiteral("Unable to read screen configuration") : operation.errorString();
        return false;
    }
    m_config = operation.config();
    m_outputs.clear();
    for (const auto &output : m_config->outputs()) {
        if (!output->isConnected()) {
            continue;
        }
        // UUID is KScreen's persistent output identifier. EDID plus connector
        // distinguishes identical monitors when the backend does not supply one.
        QString identity = output->uuid();
        if (identity.isEmpty()) {
            identity = output->hashMd5() + QLatin1Char(':') + output->name();
        }
        const auto key = QString::fromLatin1(QCryptographicHash::hash(identity.toUtf8(), QCryptographicHash::Sha256).toHex());
        m_outputs.append({output->id(), output->name(), key, output->iccProfilePath(), output->colorProfileSource()});
    }
    return true;
}

bool OutputManager::setProfile(const QString &key, const QString &path, KScreen::Output::ColorProfileSource source, QString *error)
{
    auto found = std::find_if(m_outputs.cbegin(), m_outputs.cend(), [&](const ManagedOutput &output) { return output.key == key; });
    if (found == m_outputs.cend() || !m_config) {
        *error = QStringLiteral("Output is no longer available");
        return false;
    }
    auto updated = m_config->clone();
    auto output = updated->output(found->id);
    if (!output) {
        *error = QStringLiteral("Output is no longer available");
        return false;
    }
    output->setIccProfilePath(path);
    output->setColorProfileSource(source);
    KScreen::SetConfigOperation operation(updated);
    if (!operation.exec() || operation.hasError()) {
        *error = operation.errorString().isEmpty() ? QStringLiteral("Unable to apply screen configuration") : operation.errorString();
        return false;
    }
    m_config = updated;
    for (auto &managedOutput : m_outputs) {
        if (managedOutput.key == key) {
            managedOutput.iccPath = path;
            managedOutput.source = source;
            break;
        }
    }
    return true;
}
