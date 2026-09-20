# SPDX-FileCopyrightText: David Edmundson <davidedmundson@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.21)

project(kgamma_wayland LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

set(QT_DEFAULT_MAJOR_VERSION 6)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)
find_package(KF6I18n REQUIRED)
find_package(KF6Kirigami REQUIRED)
find_package(KF6Screen REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LCMS2 REQUIRED IMPORTED_TARGET lcms2)

qt_standard_project_setup(REQUIRES 6.5)

add_definitions(-DTRANSLATION_DOMAIN=\"kgamma_wayland\")

add_library(kgamma2_core STATIC
    src/core/gammacontroller.cpp
    src/core/gammacontroller.h
    src/core/outputmanager.cpp
    src/core/outputmanager.h
    src/core/profilemanager.cpp
    src/core/profilemanager.h
)
target_include_directories(kgamma2_core PUBLIC src/core)
target_link_libraries(kgamma2_core PUBLIC Qt6::Core KF6::Screen PkgConfig::LCMS2)

add_executable(kgamma2
    src/gui/main.cpp
    src/gui/correctionsettings.cpp
    src/gui/correctionsettings.h
    src/cli/main.cpp
    src/cli/main.h
)
target_include_directories(kgamma2 PRIVATE src/cli)
target_link_libraries(kgamma2 PRIVATE kgamma2_core)

qt_add_qml_module(kgamma2
    URI KGammaWayland
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

target_link_libraries(kgamma2
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        KF6::I18n
        KF6::Kirigami
        KF6::Screen
        PkgConfig::LCMS2
	KF6::I18nQml
)

install(TARGETS kgamma2 ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

include(CTest)
if(BUILD_TESTING)
    add_executable(profilemanager_test tests/profilemanager_test.cpp)
    target_link_libraries(profilemanager_test PRIVATE kgamma2_core)
    add_test(NAME profilemanager_test COMMAND profilemanager_test)
endif()
