/*
 * SPDX-FileCopyrightText: David Edmundson <davidedmundson@kde.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "correctionsettings.h"
#include "screenprofilemanager.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QScopeGuard>

#include <algorithm>
#include <array>
#include <cmath>
#include <lcms2.h>

namespace
{
constexpr int VcgtEntryCount = 256;

QByteArray buildVcgtTag(double gamma, double red, double green, double blue)
{
    QByteArray tag;
    tag.reserve(8 + 4 + 2 + 2 + 2 + (3 * VcgtEntryCount * 2));

    auto appendU16 = [&tag](quint16 value) {
        tag.append(char((value >> 8) & 0xff));
        tag.append(char(value & 0xff));
    };

    auto appendU32 = [&tag](quint32 value) {
        tag.append(char((value >> 24) & 0xff));
        tag.append(char((value >> 16) & 0xff));
        tag.append(char((value >> 8) & 0xff));
        tag.append(char(value & 0xff));
    };

    tag.append("vcgt", 4);
    appendU32(0);
    appendU32(0);
    appendU16(3);
    appendU16(VcgtEntryCount);
    appendU16(2);

    const std::array<double, 3> channelScales = {
        std::max(0.01, red),
        std::max(0.01, green),
        std::max(0.01, blue),
    };
    const double safeGamma = std::max(0.1, gamma);

    for (double scale : channelScales) {
        for (int i = 0; i < VcgtEntryCount; ++i) {
            const double normalized = static_cast<double>(i) / (VcgtEntryCount - 1);
            const double corrected = std::pow(normalized, 1.0 / safeGamma) * scale;
            const double clamped = std::clamp(corrected, 0.0, 1.0);
            appendU16(static_cast<quint16>(std::lround(clamped * 65535.0)));
        }
    }

    return tag;
}

cmsToneCurve *buildLinearCurve()
{
    return cmsBuildGamma(nullptr, 1.0);
}

QString profilePath()
{
    return QDir::homePath() + QStringLiteral("/.config/kgamma-forced.icc");
}

quint16 readU16(const QByteArray &data, int offset)
{
    return (quint8(data[offset]) << 8) | quint8(data[offset + 1]);
}

quint32 readU32(const QByteArray &data, int offset)
{
    return (quint32(quint8(data[offset])) << 24) |
           (quint32(quint8(data[offset + 1])) << 16) |
           (quint32(quint8(data[offset + 2])) << 8) |
           quint32(quint8(data[offset + 3]));
}

double estimateScale(const std::vector<double> &channel)
{
    return std::clamp(channel.back(), 0.0, 1.0);
}

double estimateGamma(const std::vector<double> &channel, double scale)
{
    if (scale <= 0.0) {
        return 1.0;
    }

    constexpr std::array<int, 4> sampleIndexes = {32, 64, 128, 192};
    double sum = 0.0;
    int count = 0;

    for (const int index : sampleIndexes) {
        const double x = static_cast<double>(index) / (VcgtEntryCount - 1);
        const double y = channel[index] / scale;
        if (x <= 0.0 || y <= 0.0 || y >= 1.0) {
            continue;
        }

        sum += std::log(x) / std::log(y);
        ++count;
    }

    if (count == 0) {
        return 1.0;
    }

    return std::clamp(sum / count, 0.1, 3.0);
}

bool parseVcgtTag(const QByteArray &tag, double *gamma, double *red, double *green, double *blue)
{
    if (tag.size() < 18 || tag.mid(0, 4) != "vcgt" || readU32(tag, 8) != 0) {
        return false;
    }

    const quint16 channels = readU16(tag, 12);
    const quint16 entries = readU16(tag, 14);
    const quint16 entrySize = readU16(tag, 16);
    if (channels != 3 || entries != VcgtEntryCount || entrySize != 2) {
        return false;
    }

    if (tag.size() < 18 + (channels * entries * entrySize)) {
        return false;
    }

    std::array<std::vector<double>, 3> channelValues;
    int offset = 18;
    for (auto &channel : channelValues) {
        channel.reserve(entries);
        for (int i = 0; i < entries; ++i) {
            channel.push_back(readU16(tag, offset) / 65535.0);
            offset += 2;
        }
    }

    *red = estimateScale(channelValues[0]);
    *green = estimateScale(channelValues[1]);
    *blue = estimateScale(channelValues[2]);
    *gamma = std::clamp((estimateGamma(channelValues[0], *red) +
                         estimateGamma(channelValues[1], *green) +
                         estimateGamma(channelValues[2], *blue)) / 3.0,
        0.1,
        3.0);
    return true;
}

bool readValuesFromProfile(const QString &path, double *gamma, double *red, double *green, double *blue)
{
    const QByteArray pathBytes = QFile::encodeName(path);
    cmsHPROFILE profile = cmsOpenProfileFromFile(pathBytes.constData(), "r");
    if (!profile) {
        return false;
    }

    const auto closeProfile = qScopeGuard([profile] {
        cmsCloseProfile(profile);
    });

    const cmsUInt32Number rawTagSize = cmsReadRawTag(profile, cmsSigVcgtTag, nullptr, 0);
    if (rawTagSize == 0) {
        return false;
    }

    QByteArray rawTag(rawTagSize, Qt::Uninitialized);
    if (cmsReadRawTag(profile, cmsSigVcgtTag, rawTag.data(), rawTagSize) != rawTagSize) {
        return false;
    }

    return parseVcgtTag(rawTag, gamma, red, green, blue);
}
}

CorrectionSettings::CorrectionSettings(QObject *parent)
    : QObject(parent)
{
    m_screenProfileManager = new ScreenProfileManager(this);
    connect(m_screenProfileManager, &ScreenProfileManager::screenNamesChanged, this, [this] {
        if (m_selectedScreenIndex >= m_screenProfileManager->screenNames().size()) {
            setSelectedScreenIndex(0);
        }
        Q_EMIT screenNamesChanged();
    });

    load();
}

double CorrectionSettings::gamma() const
{
    return m_gamma;
}

void CorrectionSettings::setGamma(double gamma)
{
    gamma = std::clamp(gamma, 0.1, 3.0);
    if (qFuzzyCompare(m_gamma, gamma)) {
        return;
    }

    m_gamma = gamma;
    Q_EMIT gammaChanged();
}

double CorrectionSettings::red() const
{
    return m_red;
}

void CorrectionSettings::setRed(double red)
{
    red = std::clamp(red, 0.0, 2.0);
    if (qFuzzyCompare(m_red, red)) {
        return;
    }

    m_red = red;
    Q_EMIT redChanged();
}

double CorrectionSettings::green() const
{
    return m_green;
}

void CorrectionSettings::setGreen(double green)
{
    green = std::clamp(green, 0.0, 2.0);
    if (qFuzzyCompare(m_green, green)) {
        return;
    }

    m_green = green;
    Q_EMIT greenChanged();
}

double CorrectionSettings::blue() const
{
    return m_blue;
}

void CorrectionSettings::setBlue(double blue)
{
    blue = std::clamp(blue, 0.0, 2.0);
    if (qFuzzyCompare(m_blue, blue)) {
        return;
    }

    m_blue = blue;
    Q_EMIT blueChanged();
}

QStringList CorrectionSettings::screenNames() const
{
    return m_screenProfileManager ? m_screenProfileManager->screenNames() : QStringList();
}

int CorrectionSettings::selectedScreenIndex() const
{
    return m_selectedScreenIndex;
}

void CorrectionSettings::setSelectedScreenIndex(int index)
{
    const int maxIndex = screenNames().size() - 1;
    if (maxIndex < 0) {
        index = 0;
    } else {
        index = std::clamp(index, 0, maxIndex);
    }

    if (m_selectedScreenIndex == index) {
        return;
    }

    m_selectedScreenIndex = index;
    Q_EMIT selectedScreenIndexChanged();
}

QString CorrectionSettings::lastSavedProfile() const
{
    return m_lastSavedProfile;
}

bool CorrectionSettings::apply()
{
    const QString targetProfilePath = profilePath();
    QDir dir = QFileInfo(targetProfilePath).dir();
    if (!dir.mkpath(QStringLiteral("."))) {

        qWarning() << "Failed to create config directory:" << dir.path();
        return false;
    }

    QString errorMessage;
    if (!writeProfile(targetProfilePath, &errorMessage)) {
        qWarning() << errorMessage;
        return false;
    }

    if (!m_screenProfileManager || !m_screenProfileManager->setIccProfile(m_selectedScreenIndex, targetProfilePath)) {
        return false;
    }

    setLastSavedProfile(targetProfilePath);
    qInfo().noquote() << targetProfilePath;
    return true;
}

void CorrectionSettings::load()
{
    const QString path = profilePath();
    setLastSavedProfile(path);

    double gamma = 1.0;
    double red = 1.0;
    double green = 1.0;
    double blue = 1.0;
    if (QFileInfo::exists(path) && readValuesFromProfile(path, &gamma, &red, &green, &blue)) {
        setGamma(gamma);
        setRed(red);
        setGreen(green);
        setBlue(blue);
    }
}

bool CorrectionSettings::writeProfile(const QString &profilePath, QString *errorMessage)
{
    cmsToneCurve *curve = buildLinearCurve();
    if (!curve) {
        *errorMessage = QStringLiteral("Failed to create tone curve");
        return false;
    }

    cmsToneCurve *curves[3] = {curve, curve, curve};

    // taken off the internet, I don't know what this means
    // https://registry.color.org/rgb-registry/srgb
    cmsCIExyYTRIPLE primaries = {
        {0.64, 0.33, 0.03},
        {0.30, 0.60, 0.10},
        {0.15, 0.06, 0.79},
    };
    cmsCIExyY whitePoint = {0.3127, 0.3290, 1.0};

    cmsHPROFILE profile = cmsCreateRGBProfile(&whitePoint, &primaries, curves);
    cmsFreeToneCurve(curve);

    if (!profile) {
        *errorMessage = QStringLiteral("Failed to create ICC profile");
        return false;
    }

    const auto closeProfile = qScopeGuard([profile] {
        cmsCloseProfile(profile);
    });

    cmsSetDeviceClass(profile, cmsSigDisplayClass);
    cmsSetColorSpace(profile, cmsSigRgbData);
    cmsSetPCS(profile, cmsSigXYZData);
    cmsSetHeaderRenderingIntent(profile, INTENT_PERCEPTUAL);
    cmsSetHeaderManufacturer(profile, 0);
    cmsSetHeaderModel(profile, 0);

    cmsMLU *description = cmsMLUalloc(nullptr, 1);
    if (!description) {
        *errorMessage = QStringLiteral("Failed to allocate profile description");
        return false;
    }

    const QByteArray vcgt = buildVcgtTag(m_gamma, m_red, m_green, m_blue);
    if (!cmsWriteRawTag(profile, cmsSigVcgtTag, vcgt.constData(), vcgt.size())) {
        *errorMessage = QStringLiteral("Failed to write vcgt tag");
        return false;
    }

    QByteArray profilePathBytes = QFile::encodeName(profilePath);
    if (!cmsSaveProfileToFile(profile, profilePathBytes.constData())) {
        *errorMessage = QStringLiteral("Failed to save profile to %1").arg(profilePath);
        return false;
    }

    return true;
}

void CorrectionSettings::setLastSavedProfile(const QString &profilePath)
{
    if (m_lastSavedProfile == profilePath) {
        return;
    }

    m_lastSavedProfile = profilePath;
    Q_EMIT lastSavedProfileChanged();
}
