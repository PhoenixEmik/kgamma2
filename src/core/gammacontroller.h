// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once

#include "outputmanager.h"
#include "profilemanager.h"
#include <QJsonObject>

struct OutputStatus {
    ManagedOutput output;
    bool adjusted = false;
    GammaValues values;
    QString originalPath;
    KScreen::Output::ColorProfileSource originalSource = KScreen::Output::ColorProfileSource::sRGB;
};

class GammaController
{
public:
    GammaController();
    bool refresh(QString *error);
    QVector<OutputStatus> statuses() const;
    bool apply(const QString &outputKey, const GammaValues &values, QString *error);
    bool reset(const QString &outputKey, QString *error);

private:
    bool saveState(QString *error) const;
    QString statePath() const;
    OutputStatus statusFor(const ManagedOutput &output) const;

    OutputManager m_outputs;
    QJsonObject m_state;
    QString m_stateError;
};
