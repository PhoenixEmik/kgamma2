# SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

Name:           kgamma2
Version:        0.1.0
Release:        0
Summary:        Gamma adjustment for KDE Plasma Wayland
License:        LGPL-2.1-or-later AND BSD-3-Clause AND CC-BY-SA-4.0 AND CC0-1.0
URL:            https://github.com/PhoenixEmik/kgamma2
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  desktop-file-utils
BuildRequires:  gcc-c++
BuildRequires:  kf6-extra-cmake-modules
BuildRequires:  kf6-kconfig-devel
BuildRequires:  kf6-ki18n-devel
BuildRequires:  kf6-kirigami-devel
BuildRequires:  libkscreen6-devel
BuildRequires:  liblcms2-devel
BuildRequires:  pkgconfig
BuildRequires:  qt6-core-devel
BuildRequires:  qt6-gui-devel
BuildRequires:  qt6-qml-devel
BuildRequires:  qt6-quickcontrols2-devel

%description
Per-monitor gamma and color channel adjustment through ICC VCGT profiles
and KScreen on KDE Plasma Wayland.

%prep
%autosetup -p1

%build
%cmake
%cmake_build

%check
%ctest

%install
%cmake_install

%files
%license LICENSES/*
%doc README.md
%{_bindir}/kgamma2
%{_datadir}/applications/kgamma2.desktop
%{_datadir}/icons/hicolor/scalable/apps/kgamma2.svg

%changelog
