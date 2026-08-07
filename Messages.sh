#!/bin/sh
# SPDX-FileCopyrightText: 2026 Laurent Montel <montel@kde.org>
#
# # SPDX-License-Identifier: BSD-3-Clause

$XGETTEXT `find . -name "*.cpp" -o -name "*.qml"` -o $podir/kgamma_wayland.pot
