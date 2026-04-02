# SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.30)

project(konlineaccounts LANGUAGES CXX VERSION 0.1.0)

find_package(ECM 6.19.0 REQUIRED)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMQmlModule)
include(ECMGenerateExportHeader)
include(ECMQtDeclareLoggingCategory)

find_package(Qt6 6.10 REQUIRED COMPONENTS Core DBus NetworkAuth WebEngineCore WebEngineQuick)

find_package(KF6 REQUIRED COMPONENTS CoreAddons Config I18n KCMUtils KIO DBusAddons Crash)

find_package(KPim6GAPI REQUIRED)

find_package(QCoro6 REQUIRED COMPONENTS Core Network)

find_package(Qt6Keychain REQUIRED)

qcoro_enable_coroutines()

add_subdirectory(lib)
add_subdirectory(daemon)
add_subdirectory(kcm)
add_subdirectory(plugins)

ecm_qt_install_logging_categories(
    EXPORT KOnlineAccounts
    FILE konlineaccounts.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
