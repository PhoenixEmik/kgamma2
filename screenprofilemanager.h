/*
 * SPDX-FileCopyrightText: David Edmundson <davidedmundson@kde.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <QObject>
#include <QStringList>
#include <kscreen/config.h>


class ScreenProfileManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList screenNames READ screenNames NOTIFY screenNamesChanged)

public:
    explicit ScreenProfileManager(QObject *parent = nullptr);
    ~ScreenProfileManager() override;

    QStringList screenNames() const;

    Q_INVOKABLE bool setIccProfile(int index, const QString &profilePath);

Q_SIGNALS:
    void screenNamesChanged();
    void lastErrorChanged();

private:
    void refreshScreenNames();

    KScreen::ConfigPtr m_config;
    QList<int> m_screenIds;
    QStringList m_screenNames;
};
