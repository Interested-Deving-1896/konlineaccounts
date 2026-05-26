/*
 *   SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.ki18n
import org.kde.konlineaccounts.mastodon
import org.kde.konlineaccounts

SetupItem {
    id: root

    KI18nContext {
        id: _tr
        translationDomain: "konlineaccounts_mastodon"
    }

    MastodonSetup {
        id: mastodon

        builder: root.builder
    }

    ColumnLayout {
        anchors.fill: parent

        Kirigami.FormLayout {
            id: fl

            Layout.fillWidth: true
            Layout.fillHeight: true

            Controls.TextField {
                id: instance
                placeholderText: "https://kde.social"
                Kirigami.FormData.label: _tr.i18n("Instance URL:")
            }

            Controls.BusyIndicator {
                id: busy

                running: false
            }
        }

        Controls.Button {
            text: _tr.i18n("Add")

            enabled: !busy.running

            Layout.alignment: Qt.AlignRight

            onClicked: {
                busy.running = true;
                mastodon.registerMastodon(instance.text);
            }
        }
    }
}
