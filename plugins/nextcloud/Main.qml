/*
 *   SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import QtWebEngine

import org.kde.kirigami as Kirigami
import org.kde.ki18n
import org.kde.konlineaccounts.nextcloud
import org.kde.konlineaccounts

SetupItem {
    id: root

    KI18nContext {
        id: _tr
        translationDomain: "konlineaccounts_nextcloud"
    }

    Loader {
        id: loader

        anchors.fill: parent

        sourceComponent: server

        NextcloudSetup {
            id: nextcloud

            builder: root.builder

            onStateChanged: {
                if (state === NextcloudSetup.WebLogin) {
                    loader.sourceComponent = webview;
                }
            }
        }

        Component {
            id: server

            ColumnLayout {
                Kirigami.FormLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Controls.TextField {
                        id: instance
                        Kirigami.FormData.label: _tr.i18n("Server:")
                    }
                }

                Controls.Button {
                    text: _tr.i18n("Add")

                    Layout.alignment: Qt.AlignRight

                    onClicked: {
                        nextcloud.checkServer(instance.text);
                    }
                }
            }
        }

        Component {
            id: webview

            WebEngineView {
                url: nextcloud.loginUrl
                profile: nextcloud.webengineProfile
            }
        }
    }
}
