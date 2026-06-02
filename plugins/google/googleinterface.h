/*
 *   SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusUnixFileDescriptor>
#include <QObject>

#include <KConfigGroup>

#include "account.h"

class GoogleInterface : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KOnlineAccounts.Google")

    Q_PROPERTY(QString accountName READ accountName)
    Q_PROPERTY(QString clientId READ clientId)
    Q_PROPERTY(QString clientSecret READ clientSecret)
    Q_PROPERTY(QStringList scopes READ scopes)

public:
    GoogleInterface(Account *account, KConfigGroup config);

    QString accountName() const;
    QString clientId() const;
    QString clientSecret() const;
    QStringList scopes() const;

    Q_SCRIPTABLE QDBusUnixFileDescriptor accessToken() const;
    Q_SCRIPTABLE QDBusUnixFileDescriptor refreshToken() const;

private:
    KConfigGroup m_config;
    Account *m_account;
};
