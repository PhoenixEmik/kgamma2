/*
 * SPDX-FileCopyrightText: David Edmundson <davidedmundson@kde.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "screenprofilemanager.h"

#include <QFileInfo>

#include <kscreen/config.h>
#include <kscreen/configoperation.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/output.h>
#include <kscreen/setconfigoperation.h>

ScreenProfileManager::ScreenProfileManager(QObject *parent)
    : QObject(parent)
{
    auto *operation = new KScreen::GetConfigOperation(KScreen::ConfigOperation::NoOptions, this);
    connect(operation, &KScreen::ConfigOperation::finished, this, [this, operation] {
        m_config = operation->config();
        refreshScreenNames();
    });
}

ScreenProfileManager::~ScreenProfileManager()
{
}

QStringList ScreenProfileManager::screenNames() const
{
    return m_screenNames;
}

bool ScreenProfileManager::setIccProfile(int index, const QString &profilePath)
{
    if (m_config.isNull()) {
        return false;
    }

    if (index < 0 || index >= static_cast<int>(m_screenIds.size())) {
        return false;
    }
    int outputId = m_screenIds.at(index);

    // either kwin is caching it's icc profiles or kscreen is trying to be clever and work out it's not in the changeset
    // so lets yolo unset and reset
    // do not let Xaver see this!
    {
        KScreen::ConfigPtr updatedConfig = m_config->clone();
        KScreen::OutputPtr output = updatedConfig->output(outputId);
        output->setIccProfilePath(QString());
        output->setColorProfileSource(KScreen::Output::ColorProfileSource::ICC);

        auto *operation = new KScreen::SetConfigOperation(updatedConfig, this);
        connect(operation, &KScreen::ConfigOperation::finished, this, [this, operation] {
            if (operation->hasError()) {
                qWarning() << operation->errorString();
                return;
            }
        });
    }
    {
        KScreen::ConfigPtr updatedConfig = m_config->clone();
        KScreen::OutputPtr output = updatedConfig->output(outputId);
        output->setIccProfilePath(profilePath);
        output->setColorProfileSource(KScreen::Output::ColorProfileSource::ICC);

        auto *operation = new KScreen::SetConfigOperation(updatedConfig, this);
        connect(operation, &KScreen::ConfigOperation::finished, this, [this, operation] {
            if (operation->hasError()) {
                qWarning() << operation->errorString();
                return;
            }
        });
    }
    return true;
}

void ScreenProfileManager::refreshScreenNames()
{
    QStringList names;
    m_screenIds.clear();

    if (m_config) {
        for (auto output : m_config->outputs()) {
            names.append(output->name());
            m_screenIds.append(output->id());
        }
    }

    if (m_screenNames == names) {
        return;
    }

    m_screenNames = names;
    Q_EMIT screenNamesChanged();
}

#include "moc_screenprofilemanager.cpp"
