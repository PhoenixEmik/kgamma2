// SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later
#include "main.h"
#include "gammacontroller.h"
#include "presetmanager.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>
#include <algorithm>
#include <cmath>

namespace {
QString sourceName(KScreen::Output::ColorProfileSource source)
{
    switch (source) {
    case KScreen::Output::ColorProfileSource::ICC: return QStringLiteral("ICC");
    case KScreen::Output::ColorProfileSource::EDID: return QStringLiteral("EDID");
    case KScreen::Output::ColorProfileSource::sRGB: return QStringLiteral("sRGB");
    }
    return QStringLiteral("unknown");
}
bool readNumber(const QCommandLineParser &parser, const QString &name, double minimum, double maximum, double *target)
{
    if (!parser.isSet(name)) return true;
    bool ok = false;
    const double value = parser.value(name).toDouble(&ok);
    if (!ok || !std::isfinite(value) || value < minimum || value > maximum) return false;
    *target = value;
    return true;
}

int runPresetCommand(const QStringList &arguments)
{
    QTextStream out(stdout), err(stderr);
    const QString usage = QStringLiteral("Usage: kgamma2 preset {save|apply|list|show|delete|current} [NAME]\n");
    if (arguments.isEmpty() || arguments.first() == QStringLiteral("--help")) { out << usage; return arguments.isEmpty() ? 2 : 0; }
    const auto command = arguments.first();
    const bool needsName = command == QStringLiteral("save") || command == QStringLiteral("apply") ||
        command == QStringLiteral("show") || command == QStringLiteral("delete");
    if ((!needsName && arguments.size() != 1) || (needsName && arguments.size() != 2)) { err << usage; return 2; }
    PresetManager presets;
    QString error;
    if (command == QStringLiteral("list")) {
        for (const auto &name : presets.presets()) out << name << '\n';
        return 0;
    }
    if (command == QStringLiteral("current")) { out << presets.currentLabel() << '\n'; return 0; }
    if (command == QStringLiteral("show")) {
        const auto preset = presets.loadPreset(arguments[1], &error);
        if (!preset) { err << error << '\n'; return 1; }
        out << preset->name << '\n';
        for (const auto &output : preset->outputs) {
            out << "  " << output.connector << " (" << output.outputId << ") " << (output.enabled ? "enabled" : "disabled") << '\n';
            out << "    gamma=" << output.values.gamma << " red=" << output.values.red
                << " green=" << output.values.green << " blue=" << output.values.blue << '\n';
            out << "    model=" << output.model << " serial=" << output.serial << " edid=" << output.edidHash << '\n';
            out << "    original source=" << sourceName(output.originalSource) << '\n';
            out << "    original ICC=" << (output.originalIcc.isEmpty() ? QStringLiteral("none") : output.originalIcc) << '\n';
        }
        return 0;
    }
    if (command == QStringLiteral("delete")) {
        if (!presets.removePreset(arguments[1], &error)) { err << error << '\n'; return 1; }
        out << arguments[1] << ": deleted\n";
        return 0;
    }
    GammaController controller;
    if (command == QStringLiteral("save")) {
        if (!controller.refresh(&error)) { err << error << '\n'; return 1; }
        if (!presets.savePreset(PresetManager::capture(arguments[1], controller.statuses()), &error)) { err << error << '\n'; return 1; }
        out << arguments[1] << ": saved\n";
        return 0;
    }
    if (command == QStringLiteral("apply")) {
        if (!presets.applyPreset(arguments[1], controller, &error)) { err << error << '\n'; return 1; }
        out << arguments[1] << ": applied\n";
        return 0;
    }
    err << usage;
    return 2;
}
}

int runCli(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kgamma2"));
    app.setOrganizationName(QStringLiteral("kgamma2"));
    if (app.arguments().size() > 1 && app.arguments().at(1) == QStringLiteral("preset")) {
        return runPresetCommand(app.arguments().mid(2));
    }
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Adjust per-output gamma using ICC VCGT profiles\nPresets: kgamma2 preset --help"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({QStringLiteral("gamma"), QStringLiteral("Gamma value (0.1–10.0)"), QStringLiteral("value")});
    parser.addOption({QStringLiteral("red"), QStringLiteral("Red scale (0–2.0)"), QStringLiteral("value")});
    parser.addOption({QStringLiteral("green"), QStringLiteral("Green scale (0–2.0)"), QStringLiteral("value")});
    parser.addOption({QStringLiteral("blue"), QStringLiteral("Blue scale (0–2.0)"), QStringLiteral("value")});
    parser.addOption({QStringLiteral("output"), QStringLiteral("Output name or ID"), QStringLiteral("name")});
    parser.addOption({QStringLiteral("status"), QStringLiteral("Show current adjustments and original profiles")});
    parser.addOption({QStringLiteral("outputs"), QStringLiteral("List connected outputs")});
    parser.addOption({QStringLiteral("reset"), QStringLiteral("Restore original profile and source")});
    parser.process(app);

    QTextStream out(stdout), err(stderr);
    const bool change = parser.isSet(QStringLiteral("gamma")) || parser.isSet(QStringLiteral("red")) ||
        parser.isSet(QStringLiteral("green")) || parser.isSet(QStringLiteral("blue"));
    const int actions = int(change) + int(parser.isSet(QStringLiteral("reset"))) +
        int(parser.isSet(QStringLiteral("status"))) + int(parser.isSet(QStringLiteral("outputs")));
    if (actions != 1 || (parser.isSet(QStringLiteral("outputs")) && parser.isSet(QStringLiteral("output"))) || !parser.positionalArguments().isEmpty()) {
        err << "Specify exactly one action; --output works with adjustments, --reset, or --status.\n";
        return 2;
    }
    GammaController controller;
    QString error;
    if (!controller.refresh(&error)) { err << error << '\n'; return 1; }
    auto statuses = controller.statuses();
    if (statuses.isEmpty() && (change || parser.isSet(QStringLiteral("reset")))) {
        err << "No connected outputs found.\n";
        return 1;
    }
    if (parser.isSet(QStringLiteral("output"))) {
        const auto selector = parser.value(QStringLiteral("output"));
        statuses.erase(std::remove_if(statuses.begin(), statuses.end(), [&](const OutputStatus &status) {
            return status.output.name != selector && status.output.key != selector;
        }), statuses.end());
        if (statuses.size() != 1) { err << "Output not found or ambiguous: " << selector << '\n'; return 2; }
    }
    if (parser.isSet(QStringLiteral("outputs"))) {
        for (const auto &status : statuses) out << status.output.name << "\t" << status.output.key << '\n';
        return 0;
    }
    if (parser.isSet(QStringLiteral("status"))) {
        for (const auto &status : statuses) {
            out << status.output.name << " (" << status.output.key << "): " << (status.adjusted ? "adjusted" : "unchanged") << '\n';
            out << "  source: " << sourceName(status.output.source) << '\n';
            out << "  ICC: " << (status.output.iccPath.isEmpty() ? QStringLiteral("none") : status.output.iccPath) << '\n';
            if (status.adjusted) {
                out << "  original source: " << sourceName(status.originalSource) << '\n';
                out << "  original ICC: " << (status.originalPath.isEmpty() ? QStringLiteral("none") : status.originalPath) << '\n';
                out << "  gamma=" << status.values.gamma << " red=" << status.values.red
                    << " green=" << status.values.green << " blue=" << status.values.blue << '\n';
            }
        }
        return 0;
    }
    int failures = 0;
    int successes = 0;
    for (const auto &status : statuses) {
        error.clear();
        if (parser.isSet(QStringLiteral("reset"))) {
            if (!controller.reset(status.output.key, &error)) ++failures;
            else { if (status.adjusted) ++successes; out << status.output.name << ": restored\n"; }
        } else {
            auto values = status.values;
            if (!readNumber(parser, QStringLiteral("gamma"), GammaRange::minimum, GammaRange::maximum, &values.gamma) ||
                !readNumber(parser, QStringLiteral("red"), 0, 2.0, &values.red) ||
                !readNumber(parser, QStringLiteral("green"), 0, 2.0, &values.green) ||
                !readNumber(parser, QStringLiteral("blue"), 0, 2.0, &values.blue)) {
                err << "Gamma must be 0.1–10.0 and channels must be 0–2.0.\n";
                return 2;
            }
            if (!controller.apply(status.output.key, values, &error)) ++failures;
            else { ++successes; out << status.output.name << ": applied\n"; }
        }
        if (!error.isEmpty()) err << status.output.name << ": " << error << '\n';
    }
    if (successes > 0) {
        PresetManager presets;
        error.clear();
        if (!presets.markModified(&error)) { err << error << '\n'; return 1; }
    }
    return failures ? 1 : 0;
}
