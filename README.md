<!--
SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
SPDX-License-Identifier: CC-BY-SA-4.0
-->

# kgamma2

kgamma2 adjusts gamma and RGB channels on KDE Plasma Wayland through ICC VCGT
profiles and KScreen. The GUI and CLI share the same controller. Each output's
generated profile and its prior ICC path and color profile source are tracked
separately. Reset restores the previous path and source.

This fork follows [David Edmundson's KDE upstream](https://invent.kde.org/davidedmundson/kgamma2).
For a new clone, add it with:

```sh
git remote add upstream https://invent.kde.org/davidedmundson/kgamma2.git
```

Licensing follows each file's SPDX header: application code is
`LGPL-2.1-or-later`, CMake and packaging files are `BSD-3-Clause`, and this
README is `CC-BY-SA-4.0`. The complete texts are in `LICENSES/`.

## Build

Requires Qt 6, KDE Frameworks 6 I18n and Kirigami, libkscreen 6, LittleCMS 2,
and CMake with ECM.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## CLI

```sh
kgamma2 --outputs
kgamma2 --status
kgamma2 --gamma 1.2
kgamma2 --red 1.0 --green 0.9 --blue 0.8
kgamma2 --output DP-1 --gamma 1.15
kgamma2 --reset --output DP-1
kgamma2 --reset
```

Without arguments, `kgamma2` starts the GUI. Adjustments without `--output`
apply to all connected outputs. Unspecified channel values keep that output's
current kgamma2 values. `--outputs` also prints persistent output IDs, which
can be used with `--output` if a connector name is ambiguous.

The generated profiles and restore state live under the Qt application data
directory, normally `~/.local/share/kgamma2/`. When an existing ICC profile
contains a supported VCGT table or formula, the generated VCGT composes the
adjustment with that calibration curve. Other ICC tags are copied byte for
byte. An unsupported VCGT format is rejected to avoid discarding calibration.

If an output used KScreen's sRGB or EDID profile source, kgamma2 uses an sRGB
base while the adjustment is active and restores the previous source on Reset.
KScreen does not expose the effective EDID-derived ICC data for deriving a
profile from it.

## openSUSE RPM

`packaging/opensuse/kgamma2.spec` builds one package with the GUI and CLI.
Create a source tarball named `kgamma2-0.1.0.tar.gz` from this tree, then pass
it as `Source0` to `rpmbuild -ba` or an OBS package.
