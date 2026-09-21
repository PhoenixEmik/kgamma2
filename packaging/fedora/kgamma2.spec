# SPDX-FileCopyrightText: 2026 PhoenixEmik <phoenix0919mik@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

Name:           kgamma2
Version:        0.1.0
Release:        1%{?dist}
Summary:        Per-monitor gamma adjustment for KDE Plasma Wayland

License:        LGPL-2.1-or-later AND BSD-3-Clause AND CC-BY-SA-4.0 AND CC0-1.0
URL:            https://github.com/PhoenixEmik/kgamma2
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  desktop-file-utils
BuildRequires:  extra-cmake-modules
BuildRequires:  gcc-c++
BuildRequires:  kf6-kconfig-devel
BuildRequires:  kf6-ki18n-devel
BuildRequires:  kf6-kirigami-devel
BuildRequires:  lcms2-devel
BuildRequires:  libkscreen-devel
BuildRequires:  pkgconfig
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtdeclarative-devel

%description
kgamma2 adjusts gamma and RGB channels independently for each monitor on KDE
Plasma Wayland. It creates ICC VCGT profiles through KScreen and restores the
previous color profile when an adjustment is reset.

%prep
%autosetup -n %{name}-%{version}

%build
%cmake -DKGAMMA2_VERSION=%{version}
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
