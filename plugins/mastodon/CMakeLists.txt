# SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: BSD-2-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"konlineaccounts_mastodon\")

kcoreaddons_add_plugin(mastodon INSTALL_NAMESPACE "konlineaccounts/providers")

target_sources(mastodon PRIVATE mastodon.cpp mastodoninterface.cpp)

target_link_libraries(mastodon PRIVATE accounts KF6::CoreAddons Qt::DBus Qt6Keychain::Qt6Keychain)

ecm_add_qml_module(mastodonsetup URI "org.kde.konlineaccounts.mastodon" QML_FILES Main.qml GENERATE_PLUGIN_SOURCE DEPENDENCIES QtCore org.kde.konlineaccounts)
target_sources(mastodonsetup PRIVATE mastodonsetup.cpp)
target_link_libraries(mastodonsetup PRIVATE Qt::NetworkAuth QCoro::Core QCoro::Network KF6::I18n KF6::KIOCore Qt6Keychain::Qt6Keychain accounts)
ecm_finalize_qml_module(mastodonsetup)

ecm_qt_declare_logging_category(mastodon
    HEADER debug.h
    IDENTIFIER LOG_KONLINEACCOUNTS_MASTODON
    CATEGORY_NAME konlineaccounts.mastodon
    DESCRIPTION "KOnlineAccounts Mastodon"
    EXPORT KOnlineAccounts
)

ecm_qt_declare_logging_category(mastodonsetup
    HEADER setup_debug.h
    IDENTIFIER LOG_KONLINEACCOUNTS_MASTODON_SETUP
    CATEGORY_NAME konlineaccounts.mastodon.setup
    DESCRIPTION "KOnlineAccounts Mastodon Setup"
    EXPORT KOnlineAccounts
)
