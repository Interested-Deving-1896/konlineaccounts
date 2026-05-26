# SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: BSD-2-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"konlineaccounts_google\")

kcoreaddons_add_plugin(google INSTALL_NAMESPACE "konlineaccounts/providers")

target_sources(google PRIVATE google.cpp googleinterface.cpp)

target_link_libraries(google PRIVATE accounts KF6::CoreAddons Qt::DBus Qt6Keychain::Qt6Keychain)

ecm_add_qml_module(googlesetup URI "org.kde.konlineaccounts.google" QML_FILES Main.qml GENERATE_PLUGIN_SOURCE)
target_sources(googlesetup PRIVATE googlesetup.cpp)
target_link_libraries(googlesetup PRIVATE QCoro::Core KF6::I18n KPim6::GAPICore Qt6Keychain::Qt6Keychain accounts)
ecm_finalize_qml_module(googlesetup)

ecm_qt_declare_logging_category(google
    HEADER debug.h
    IDENTIFIER LOG_KONLINEACCOUNTS_GOOGLE
    CATEGORY_NAME konlineaccounts.google
    DESCRIPTION "KOnlineAccounts Google"
    EXPORT KOnlineAccounts
)

ecm_qt_declare_logging_category(googlesetup
    HEADER setup_debug.h
    IDENTIFIER LOG_KONLINEACCOUNTS_GOOGLE_SETUP
    CATEGORY_NAME konlineaccounts.google.setup
    DESCRIPTION "KOnlineAccounts Google Setup"
    EXPORT KOnlineAccounts
)
