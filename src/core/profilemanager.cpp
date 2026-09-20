// SPDX-FileCopyrightText: David Edmundson <davidedmundson@kde.org>
// SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later
#include "profilemanager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>
#include <QtEndian>
#include <algorithm>
#include <array>
#include <cmath>
#include <lcms2.h>
#include <limits>

namespace {
constexpr int entries = 256;
constexpr int vcgtSize = 18 + 3 * entries * 2;
quint16 u16(const QByteArray &data, qsizetype at) { return qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(data.constData() + at)); }
quint32 u32(const QByteArray &data, qsizetype at) { return qFromBigEndian<quint32>(reinterpret_cast<const uchar *>(data.constData() + at)); }
void set32(QByteArray &data, qsizetype at, quint32 value) { qToBigEndian(value, reinterpret_cast<uchar *>(data.data() + at)); }
void append16(QByteArray &data, quint16 value)
{
    char bytes[2];
    qToBigEndian(value, reinterpret_cast<uchar *>(bytes));
    data.append(bytes, 2);
}
void append32(QByteArray &data, quint32 value)
{
    char bytes[4];
    qToBigEndian(value, reinterpret_cast<uchar *>(bytes));
    data.append(bytes, 4);
}

struct ExistingVcgt {
    std::array<std::array<double, entries>, 3> curves{};
    ExistingVcgt()
    {
        for (auto &curve : curves) {
            for (int i = 0; i < entries; ++i) curve[i] = double(i) / (entries - 1);
        }
    }
    double sample(int channel, double x) const
    {
        const double position = std::clamp(x, 0.0, 1.0) * (entries - 1);
        const int lower = int(position);
        const int upper = std::min(lower + 1, entries - 1);
        return std::lerp(curves[channel][lower], curves[channel][upper], position - lower);
    }
};

bool readVcgt(const QByteArray &tag, ExistingVcgt *existing, QString *error)
{
    if (tag.size() < 12 || tag.left(4) != "vcgt") {
        *error = QStringLiteral("Original ICC has an unsupported VCGT tag");
        return false;
    }
    const auto type = u32(tag, 8);
    if (type == 0 && tag.size() >= 18) {
        const int channels = u16(tag, 12);
        const int count = u16(tag, 14);
        const int bytes = u16(tag, 16);
        if (channels != 3 || count < 2 || (bytes != 1 && bytes != 2) || tag.size() < 18 + channels * count * bytes) {
            *error = QStringLiteral("Original ICC has an unsupported VCGT table");
            return false;
        }
        for (int channel = 0; channel < 3; ++channel) {
            for (int i = 0; i < entries; ++i) {
                const double position = double(i) * (count - 1) / (entries - 1);
                const int low = int(position);
                const int high = std::min(low + 1, count - 1);
                auto read = [&](int index) {
                    const int at = 18 + (channel * count + index) * bytes;
                    return bytes == 1 ? quint8(tag[at]) / 255.0 : u16(tag, at) / 65535.0;
                };
                existing->curves[channel][i] = std::lerp(read(low), read(high), position - low);
            }
        }
        return true;
    }
    if (type == 1 && tag.size() >= 48) {
        // Apple VCGT formula: gamma, minimum, maximum for each channel (16.16).
        for (int channel = 0; channel < 3; ++channel) {
            const auto at = 12 + channel * 12;
            const double gamma = u32(tag, at) / 65536.0;
            const double minimum = u32(tag, at + 4) / 65536.0;
            const double maximum = u32(tag, at + 8) / 65536.0;
            if (!std::isfinite(gamma) || gamma <= 0 || minimum > maximum || maximum > 1.0) {
                *error = QStringLiteral("Original ICC has invalid VCGT formula values");
                return false;
            }
            for (int i = 0; i < entries; ++i) {
                existing->curves[channel][i] = minimum + (maximum - minimum) * std::pow(double(i) / (entries - 1), gamma);
            }
        }
        return true;
    }
    *error = QStringLiteral("Original ICC has an unsupported VCGT format");
    return false;
}

QByteArray makeVcgt(const ExistingVcgt &existing, const GammaValues &values)
{
    QByteArray tag;
    tag.reserve(vcgtSize);
    tag.append("vcgt", 4);
    append32(tag, 0);
    append32(tag, 0);
    append16(tag, 3);
    append16(tag, entries);
    append16(tag, 2);
    const std::array<double, 3> scales = {values.red, values.green, values.blue};
    for (int channel = 0; channel < 3; ++channel) {
        for (int i = 0; i < entries; ++i) {
            const double input = std::clamp(std::pow(double(i) / (entries - 1), 1.0 / values.gamma) * scales[channel], 0.0, 1.0);
            append16(tag, quint16(std::lround(std::clamp(existing.sample(channel, input), 0.0, 1.0) * 65535.0)));
        }
    }
    return tag;
}

bool validProfile(const QByteArray &bytes)
{
    auto profile = cmsOpenProfileFromMem(bytes.constData(), cmsUInt32Number(bytes.size()));
    if (!profile) return false;
    cmsCloseProfile(profile);
    return true;
}
}

QString ProfileManager::profileDirectory()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QStringLiteral("/profiles");
}

bool ProfileManager::writeDerived(const QString &basePath, const QString &destination, const GammaValues &values, QString *error)
{
    if (!std::isfinite(values.gamma) || values.gamma < GammaRange::minimum || values.gamma > GammaRange::maximum ||
        !std::isfinite(values.red) || values.red < 0 || values.red > 2 ||
        !std::isfinite(values.green) || values.green < 0 || values.green > 2 ||
        !std::isfinite(values.blue) || values.blue < 0 || values.blue > 2) {
        *error = QStringLiteral("Gamma must be 0.1–10.0 and channels must be 0–2.0");
        return false;
    }

    QByteArray profile;
    if (basePath.isEmpty()) {
        auto *base = cmsCreate_sRGBProfile();
        if (!base) { *error = QStringLiteral("Unable to create sRGB profile"); return false; }
        cmsUInt32Number size = 0;
        if (cmsSaveProfileToMem(base, nullptr, &size) && size > 0) {
            profile.resize(size);
            if (!cmsSaveProfileToMem(base, profile.data(), &size)) profile.clear();
        }
        cmsCloseProfile(base);
    } else {
        QFile file(basePath);
        if (!file.open(QIODevice::ReadOnly)) { *error = QStringLiteral("Unable to read original ICC: %1").arg(basePath); return false; }
        profile = file.readAll();
    }
    if (profile.size() < 132 || !validProfile(profile) || u32(profile, 128) > quint32((profile.size() - 132) / 12)) {
        *error = QStringLiteral("Original ICC is invalid");
        return false;
    }
    const quint32 count = u32(profile, 128);
    const qsizetype tableEnd = 132 + qsizetype(count) * 12;
    ExistingVcgt existing;
    int vcgtEntry = -1;
    for (quint32 i = 0; i < count; ++i) {
        const auto at = 132 + qsizetype(i) * 12;
        const auto offset = u32(profile, at + 4);
        const auto size = u32(profile, at + 8);
        if (offset < tableEnd || offset > quint32(profile.size()) || size > quint32(profile.size()) - offset) {
            *error = QStringLiteral("Original ICC has an invalid tag table");
            return false;
        }
        if (profile.mid(at, 4) == "vcgt") {
            vcgtEntry = int(i);
            if (!readVcgt(profile.mid(offset, size), &existing, error)) return false;
        }
    }
    if (vcgtEntry < 0) {
        if (count == std::numeric_limits<quint32>::max() || profile.size() > std::numeric_limits<int>::max() - 12 - vcgtSize) {
            *error = QStringLiteral("Original ICC is too large");
            return false;
        }
        profile.insert(tableEnd, QByteArray(12, '\0'));
        for (quint32 i = 0; i < count; ++i) {
            const auto at = 132 + qsizetype(i) * 12;
            set32(profile, at + 4, u32(profile, at + 4) + 12);
        }
        set32(profile, 128, count + 1);
        vcgtEntry = int(count);
    }
    while (profile.size() % 4) profile.append('\0');
    const auto vcgtOffset = quint32(profile.size());
    const auto tag = makeVcgt(existing, values);
    profile.append(tag);
    const auto at = 132 + qsizetype(vcgtEntry) * 12;
    profile.replace(at, 4, "vcgt");
    set32(profile, at + 4, vcgtOffset);
    set32(profile, at + 8, quint32(tag.size()));
    set32(profile, 0, quint32(profile.size()));
    // The previous profile ID no longer identifies these bytes.
    profile.replace(84, 16, QByteArray(16, '\0'));
    if (!validProfile(profile)) { *error = QStringLiteral("Derived ICC is invalid"); return false; }
    if (!QDir().mkpath(QFileInfo(destination).absolutePath())) { *error = QStringLiteral("Unable to create profile directory"); return false; }
    QSaveFile file(destination);
    if (!file.open(QIODevice::WriteOnly) || file.write(profile) != profile.size() || !file.commit()) {
        *error = QStringLiteral("Unable to save derived ICC: %1").arg(destination);
        return false;
    }
    return true;
}
