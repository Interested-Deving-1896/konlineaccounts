# SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: BSD-2-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"konlineaccounts_nextcloud\")

kcoreaddons_add_plugin(nextcloud INSTALL_NAMESPACE "konlineaccounts/providers")

target_sources(nextcloud PRIVATE nextcloud.cpp nextcloudinterface.cpp caldav.cpp carddav.cpp)

target_link_libraries(nextcloud PRIVATE accounts KF6::CoreAddons Qt::DBus Qt6Keychain::Qt6Keychain)

ecm_add_qml_module(nextcloudsetup URI "org.kde.konlineaccounts.nextcloud" QML_FILES Main.qml DEPENDENCIES QtWebEngine org.kde.konlineaccounts GENERATE_PLUGIN_SOURCE)
target_sources(nextcloudsetup PRIVATE nextcloudsetup.cpp)
target_link_libraries(nextcloudsetup PRIVATE Qt::WebEngineQuick QCoro::Core QCoro::Network KF6::I18n KF6::KIOCore Qt6Keychain::Qt6Keychain accounts)
ecm_finalize_qml_module(nextcloudsetup)

ecm_qt_declare_logging_category(nextcloud
    HEADER debug.h
    IDENTIFIER LOG_KONLINEACCOUNTS_NEXTCLOUD
    CATEGORY_NAME konlineaccounts.nextcloud
    DESCRIPTION "KOnlineAccounts Nextcloud"
    EXPORT KOnlineAccounts
)

ecm_qt_declare_logging_category(nextcloudsetup
    HEADER setup_debug.h
    IDENTIFIER LOG_KONLINEACCOUNTS_NEXTCLOUD_SETUP
    CATEGORY_NAME konlineaccounts.nextcloud.setup
    DESCRIPTION "KOnlineAccounts Nextcloud Setup"
    EXPORT KOnlineAccounts
)
