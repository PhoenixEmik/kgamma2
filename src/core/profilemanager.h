// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once

#include <QString>

struct GammaValues {
    double gamma = 1.0;
    double red = 1.0;
    double green = 1.0;
    double blue = 1.0;
};

class ProfileManager
{
public:
    static QString profileDirectory();
    static bool writeDerived(const QString &basePath, const QString &destination, const GammaValues &values, QString *error);
};
