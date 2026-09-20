// SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later
#include "presetmanager.h"

#include <KConfigGroup>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>
#include <cmath>
#include <functional>

namespace {
bool validValues(const GammaValues &values)
{
    return std::isfinite(values.gamma) && values.gamma >= GammaRange::minimum && values.gamma <= GammaRange::maximum &&
        std::isfinite(values.red) && values.red >= 0.0 && values.red <= 2.0 &&
        std::isfinite(values.green) && values.green >= 0.0 && values.green <= 2.0 &&
        std::isfinite(values.blue) && values.blue >= 0.0 && values.blue <= 2.0;
}

bool sameAdjustments(const GammaPreset &a, const GammaPreset &b)
{
    if (a.outputs.size() != b.outputs.size()) return false;
    for (const auto &output : a.outputs) {
        const auto found = std::find_if(b.outputs.cbegin(), b.outputs.cend(), [&](const OutputPreset &other) {
            return other.outputId == output.outputId;
        });
        if (found == b.outputs.cend() || found->enabled != output.enabled ||
            found->values.gamma != output.values.gamma || found->values.red != output.values.red ||
            found->values.green != output.values.green || found->values.blue != output.values.blue) return false;
    }
    return true;
}

std::optional<int> uniqueMatch(const QVector<OutputStatus> &statuses, const std::function<bool(const ManagedOutput &)> &predicate, QString *error)
{
    std::optional<int> result;
    for (int i = 0; i < statuses.size(); ++i) {
        if (!predicate(statuses[i].output)) continue;
        if (result) {
            *error = QStringLiteral("Preset output matches multiple connected screens");
            return {};
        }
        result = i;
    }
    return result;
}
}

PresetManager::PresetManager(const QString &configFile)
    : m_config(KSharedConfig::openConfig(configFile, KConfig::SimpleConfig))
{
}

bool PresetManager::validName(const QString &name)
{
    static const QRegularExpression pattern(QStringLiteral("^[A-Za-z0-9][A-Za-z0-9._ -]{0,63}$"));
    return pattern.match(name).hasMatch();
}

QStringList PresetManager::presets() const
{
    m_config->reparseConfiguration();
    const KConfigGroup root(m_config, QStringLiteral("Presets"));
    auto names = root.groupList();
    names.sort(Qt::CaseInsensitive);
    return names;
}

GammaPreset PresetManager::capture(const QString &name, const QVector<OutputStatus> &statuses)
{
    GammaPreset preset;
    preset.name = name;
    for (const auto &status : statuses) {
        OutputPreset output;
        output.outputId = status.output.key;
        output.connector = status.output.name;
        output.edidHash = status.output.edidHash;
        output.model = status.output.model;
        output.serial = status.output.serial;
        output.enabled = status.adjusted;
        output.values = status.values;
        output.originalIcc = status.adjusted ? status.originalPath : status.output.iccPath;
        output.originalSource = status.adjusted ? status.originalSource : status.output.source;
        preset.outputs.append(output);
    }
    return preset;
}

std::optional<GammaPreset> PresetManager::loadPreset(const QString &name, QString *error) const
{
    if (!validName(name)) { *error = QStringLiteral("Invalid preset name"); return {}; }
    m_config->reparseConfiguration();
    const KConfigGroup root(m_config, QStringLiteral("Presets"));
    const KConfigGroup group(&root, name);
    if (!group.exists()) { *error = QStringLiteral("Preset does not exist: %1").arg(name); return {}; }
    const KConfigGroup outputs(&group, QStringLiteral("Outputs"));
    GammaPreset preset;
    preset.name = name;
    for (const auto &id : outputs.groupList()) {
        const KConfigGroup entry(&outputs, id);
        OutputPreset output;
        output.outputId = id;
        output.connector = entry.readEntry("Connector", QString());
        output.edidHash = entry.readEntry("EdidHash", QString());
        output.model = entry.readEntry("Model", QString());
        output.serial = entry.readEntry("Serial", QString());
        output.enabled = entry.readEntry("Enabled", false);
        output.values.gamma = entry.readEntry("Gamma", 1.0);
        output.values.red = entry.readEntry("Red", 1.0);
        output.values.green = entry.readEntry("Green", 1.0);
        output.values.blue = entry.readEntry("Blue", 1.0);
        output.originalIcc = entry.readEntry("OriginalICC", QString());
        const int source = entry.readEntry("OriginalSource", int(KScreen::Output::ColorProfileSource::sRGB));
        if (source < 0 || source > 2 || !validValues(output.values)) {
            *error = QStringLiteral("Preset has invalid settings: %1").arg(name);
            return {};
        }
        output.originalSource = static_cast<KScreen::Output::ColorProfileSource>(source);
        preset.outputs.append(output);
    }
    if (preset.outputs.isEmpty()) { *error = QStringLiteral("Preset has no outputs: %1").arg(name); return {}; }
    std::sort(preset.outputs.begin(), preset.outputs.end(), [](const OutputPreset &a, const OutputPreset &b) {
        return a.connector == b.connector ? a.outputId < b.outputId : a.connector < b.connector;
    });
    return preset;
}

bool PresetManager::savePreset(const GammaPreset &preset, QString *error)
{
    if (!validName(preset.name)) { *error = QStringLiteral("Preset name must be 1–64 letters, digits, spaces, dots, underscores or hyphens"); return false; }
    if (preset.outputs.isEmpty()) { *error = QStringLiteral("No connected outputs to save"); return false; }
    QSet<QString> ids;
    for (const auto &output : preset.outputs) {
        if (output.outputId.isEmpty() || ids.contains(output.outputId) || !validValues(output.values)) {
            *error = QStringLiteral("Preset has duplicate or invalid output settings");
            return false;
        }
        ids.insert(output.outputId);
    }
    bool changesActive = false;
    if (activePreset() == preset.name) {
        QString ignored;
        const auto previous = loadPreset(preset.name, &ignored);
        changesActive = !previous || !sameAdjustments(*previous, preset);
    }
    m_config->reparseConfiguration();
    KConfigGroup root(m_config, QStringLiteral("Presets"));
    KConfigGroup group(&root, preset.name);
    group.deleteGroup();
    KConfigGroup newGroup(&root, preset.name);
    KConfigGroup outputs(&newGroup, QStringLiteral("Outputs"));
    for (const auto &output : preset.outputs) {
        KConfigGroup entry(&outputs, output.outputId);
        entry.writeEntry("Connector", output.connector);
        entry.writeEntry("EdidHash", output.edidHash);
        entry.writeEntry("Model", output.model);
        entry.writeEntry("Serial", output.serial);
        entry.writeEntry("Enabled", output.enabled);
        entry.writeEntry("Gamma", output.values.gamma);
        entry.writeEntry("Red", output.values.red);
        entry.writeEntry("Green", output.values.green);
        entry.writeEntry("Blue", output.values.blue);
        entry.writeEntry("OriginalICC", output.originalIcc);
        entry.writeEntry("OriginalSource", int(output.originalSource));
    }
    if (!m_config->sync()) { *error = QStringLiteral("Unable to save kgamma2rc"); return false; }
    if (changesActive && !markModified(error)) return false;
    return true;
}

bool PresetManager::removePreset(const QString &name, QString *error)
{
    if (!validName(name)) { *error = QStringLiteral("Invalid preset name"); return false; }
    m_config->reparseConfiguration();
    KConfigGroup root(m_config, QStringLiteral("Presets"));
    KConfigGroup group(&root, name);
    if (!group.exists()) { *error = QStringLiteral("Preset does not exist: %1").arg(name); return false; }
    group.deleteGroup();
    KConfigGroup general(m_config, QStringLiteral("General"));
    if (general.readEntry("ActivePreset", QString()) == name) {
        general.deleteEntry("ActivePreset");
        general.deleteEntry("Modified");
    }
    if (!m_config->sync()) { *error = QStringLiteral("Unable to update kgamma2rc"); return false; }
    return true;
}

std::optional<int> PresetManager::matchOutput(const OutputPreset &preset, const QVector<OutputStatus> &statuses, QString *error)
{
    error->clear();
    if (!preset.outputId.isEmpty()) {
        const auto match = uniqueMatch(statuses, [&](const ManagedOutput &output) { return output.key == preset.outputId; }, error);
        if (match || !error->isEmpty()) return match;
    }
    if (!preset.edidHash.isEmpty()) {
        const auto match = uniqueMatch(statuses, [&](const ManagedOutput &output) {
            return output.edidHash == preset.edidHash && (preset.serial.isEmpty() || output.serial == preset.serial);
        }, error);
        if (match || !error->isEmpty()) return match;
    }
    if (!preset.serial.isEmpty()) {
        const auto match = uniqueMatch(statuses, [&](const ManagedOutput &output) {
            return output.serial == preset.serial && (preset.model.isEmpty() || output.model == preset.model);
        }, error);
        if (match || !error->isEmpty()) return match;
    }
    if (!preset.connector.isEmpty()) {
        return uniqueMatch(statuses, [&](const ManagedOutput &output) { return output.name == preset.connector; }, error);
    }
    return {};
}

bool PresetManager::applyPreset(const QString &name, GammaController &controller, QString *error)
{
    const auto preset = loadPreset(name, error);
    if (!preset || !controller.refresh(error)) return false;
    const auto statuses = controller.statuses();
    struct Action { QString key; OutputPreset output; OutputStatus previous; };
    QList<Action> actions;
    QSet<QString> used;
    bool skipped = false;
    for (const auto &output : preset->outputs) {
        const auto index = matchOutput(output, statuses, error);
        if (!error->isEmpty()) return false;
        if (!index) { skipped = true; continue; } // A saved monitor may be disconnected.
        const auto key = statuses[*index].output.key;
        if (used.contains(key)) { *error = QStringLiteral("Two preset outputs match the same connected screen"); return false; }
        used.insert(key);
        actions.append({key, output, statuses[*index]});
    }
    if (actions.isEmpty()) { *error = QStringLiteral("No preset outputs match connected screens"); return false; }
    QList<Action> applied;
    for (const auto &action : actions) {
        const bool ok = action.output.enabled ? controller.apply(action.key, action.output.values, error) : controller.reset(action.key, error);
        if (!ok) {
            const QString applyError = *error;
            QString rollbackError;
            applied.append(action); // The failed operation may have changed KScreen before reporting an error.
            for (auto it = applied.crbegin(); it != applied.crend(); ++it) {
                QString currentError;
                if (controller.refresh(&currentError)) {
                    const auto current = controller.statuses();
                    const auto unchanged = std::find_if(current.cbegin(), current.cend(), [&](const OutputStatus &status) {
                        return status.output.key == it->key && status.output.iccPath == it->previous.output.iccPath &&
                            status.output.source == it->previous.output.source;
                    });
                    if (unchanged != current.cend()) continue;
                }
                currentError.clear();
                const bool restored = it->previous.adjusted
                    ? controller.apply(it->key, it->previous.values, &currentError)
                    : controller.reset(it->key, &currentError);
                if (!restored) rollbackError += QStringLiteral("; %1: %2").arg(it->previous.output.name, currentError);
            }
            if (!rollbackError.isEmpty()) {
                QString ignored;
                markModified(&ignored);
                *error = applyError + QStringLiteral("; rollback incomplete") + rollbackError;
            } else {
                *error = applyError;
            }
            return false;
        }
        applied.append(action);
    }
    if (!setActive(name, skipped, error)) {
        QString ignored;
        markModified(&ignored);
        return false;
    }
    return true;
}

QString PresetManager::activePreset() const
{
    m_config->reparseConfiguration();
    const KConfigGroup general(m_config, QStringLiteral("General"));
    return general.readEntry("ActivePreset", QString());
}

bool PresetManager::isModified() const
{
    m_config->reparseConfiguration();
    const KConfigGroup general(m_config, QStringLiteral("General"));
    return general.readEntry("Modified", false);
}

QString PresetManager::currentLabel() const
{
    const auto name = activePreset();
    if (name.isEmpty()) return QStringLiteral("none");
    return isModified() ? name + QStringLiteral(" (modified)") : name;
}

bool PresetManager::setActive(const QString &name, bool modified, QString *error)
{
    m_config->reparseConfiguration();
    KConfigGroup general(m_config, QStringLiteral("General"));
    general.writeEntry("ActivePreset", name);
    general.writeEntry("Modified", modified);
    if (!m_config->sync()) { *error = QStringLiteral("Unable to update active preset"); return false; }
    return true;
}

bool PresetManager::markModified(QString *error)
{
    const auto name = activePreset();
    if (name.isEmpty() || isModified()) return true;
    return setActive(name, true, error);
}
