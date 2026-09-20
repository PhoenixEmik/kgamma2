// SPDX-License-Identifier: LGPL-2.1-or-later
#include "profilemanager.h"

#include <QCoreApplication>
#include <QFile>
#include <QTemporaryDir>
#include <QtEndian>
#include <QDebug>
#include <lcms2.h>

namespace {
quint32 u32(const QByteArray &bytes, qsizetype at)
{
    return qFromBigEndian<quint32>(reinterpret_cast<const uchar *>(bytes.constData() + at));
}
QByteArray readFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return {};
    return file.readAll();
}
QByteArray tag(const QByteArray &profile, const QByteArray &signature)
{
    if (profile.size() < 132) return {};
    const auto count = u32(profile, 128);
    for (quint32 i = 0; i < count; ++i) {
        const auto at = 132 + qsizetype(i) * 12;
        if (at + 12 > profile.size()) return {};
        if (profile.mid(at, 4) == signature) {
            const auto offset = u32(profile, at + 4);
            const auto size = u32(profile, at + 8);
            if (offset > quint32(profile.size()) || size > quint32(profile.size()) - offset) return {};
            return profile.mid(offset, size);
        }
    }
    return {};
}
bool checkPreserved(const QByteArray &original, const QByteArray &derived)
{
    const auto count = u32(original, 128);
    for (quint32 i = 0; i < count; ++i) {
        const auto signature = original.mid(132 + qsizetype(i) * 12, 4);
        if (signature != "vcgt" && tag(original, signature) != tag(derived, signature)) return false;
    }
    return true;
}
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTemporaryDir dir;
    if (!dir.isValid()) return 1;
    const QString base = dir.path() + QStringLiteral("/base.icc");
    const QString first = dir.path() + QStringLiteral("/first.icc");
    const QString second = dir.path() + QStringLiteral("/second.icc");
    auto *profile = cmsCreate_sRGBProfile();
    if (!profile) return 1;
    const auto baseName = QFile::encodeName(base);
    const bool saved = cmsSaveProfileToFile(profile, baseName.constData());
    cmsCloseProfile(profile);
    if (!saved) return 1;
    QString error;
    if (!ProfileManager::writeDerived(base, first, {1.2, 1.0, 0.9, 0.8}, &error)) { qCritical() << error; return 1; }
    if (!ProfileManager::writeDerived(first, second, {0.8, 1.0, 1.0, 1.0}, &error)) { qCritical() << error; return 1; }
    const auto sourceBytes = readFile(base);
    const auto firstBytes = readFile(first);
    const auto secondBytes = readFile(second);
    if (!checkPreserved(sourceBytes, firstBytes) || !checkPreserved(firstBytes, secondBytes)) {
        qCritical() << "A non-VCGT ICC tag changed";
        return 1;
    }
    const auto firstVcgt = tag(firstBytes, "vcgt");
    const auto secondVcgt = tag(secondBytes, "vcgt");
    if (firstVcgt.size() != 18 + 3 * 256 * 2 || secondVcgt.size() != firstVcgt.size() || firstVcgt == secondVcgt) {
        qCritical() << "VCGT was not generated or composed";
        return 1;
    }
    return 0;
}
