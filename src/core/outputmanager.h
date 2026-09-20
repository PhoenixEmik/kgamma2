// SPDX-FileCopyrightText: David Edmundson <davidedmundson@kde.org>
// SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once

#include <QString>
#include <QVector>
#include <kscreen/config.h>
#include <kscreen/output.h>

struct ManagedOutput {
    int id = -1;
    QString name;
    QString key;
    QString edidHash;
    QString model;
    QString serial;
    QString iccPath;
    KScreen::Output::ColorProfileSource source = KScreen::Output::ColorProfileSource::sRGB;
};

class OutputManager
{
public:
    bool refresh(QString *error);
    const QVector<ManagedOutput> &outputs() const { return m_outputs; }
    bool setProfile(const QString &key, const QString &path, KScreen::Output::ColorProfileSource source, QString *error);

private:
    KScreen::ConfigPtr m_config;
    QVector<ManagedOutput> m_outputs;
};
