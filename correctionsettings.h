/*
 * SPDX-FileCopyrightText: David Edmundson <davidedmundson@kde.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class ScreenProfileManager;

class CorrectionSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double gamma READ gamma WRITE setGamma NOTIFY gammaChanged)
    Q_PROPERTY(double red READ red WRITE setRed NOTIFY redChanged)
    Q_PROPERTY(double green READ green WRITE setGreen NOTIFY greenChanged)
    Q_PROPERTY(double blue READ blue WRITE setBlue NOTIFY blueChanged)
    Q_PROPERTY(QStringList screenNames READ screenNames NOTIFY screenNamesChanged)
    Q_PROPERTY(int selectedScreenIndex READ selectedScreenIndex WRITE setSelectedScreenIndex NOTIFY selectedScreenIndexChanged)
    Q_PROPERTY(QString lastSavedProfile READ lastSavedProfile NOTIFY lastSavedProfileChanged)

public:
    explicit CorrectionSettings(QObject *parent = nullptr);

    double gamma() const;
    void setGamma(double gamma);

    double red() const;
    void setRed(double red);

    double green() const;
    void setGreen(double green);

    double blue() const;
    void setBlue(double blue);

    QStringList screenNames() const;
    int selectedScreenIndex() const;
    void setSelectedScreenIndex(int index);

    QString lastSavedProfile() const;

    Q_INVOKABLE bool apply();

signals:
    void gammaChanged();
    void redChanged();
    void greenChanged();
    void blueChanged();
    void screenNamesChanged();
    void selectedScreenIndexChanged();
    void lastSavedProfileChanged();

private:
    void load();
    bool writeProfile(const QString &profilePath, QString *errorMessage);
    void setLastSavedProfile(const QString &profilePath);

    double m_gamma = 1.0;
    double m_red = 1.0;
    double m_green = 1.0;
    double m_blue = 1.0;
    int m_selectedScreenIndex = 0;
    QString m_lastSavedProfile;
    QString m_lastError;
    ScreenProfileManager *m_screenProfileManager = nullptr;
};
