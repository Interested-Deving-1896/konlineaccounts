/*
 *   SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "googleinterface.h"

#include "debug.h"
#include "fdwriter.h"

#include <qt6keychain/keychain.h>

#include <QDBusConnection>
#include <QDBusMessage>

using namespace Qt::Literals;

GoogleInterface::GoogleInterface(Account *account, KConfigGroup config)
    : QDBusAbstractAdaptor(account)
    , m_config(config)
    , m_account(account)
{
}

#define CHECK_ACCESS                                                                                                                                           \
    if (!m_account->currentCallerHasAccess()) {                                                                                                                \
        m_account->sendErrorReply(QDBusError::AccessDenied, u"Caller is not authorized to read this property"_s);                                              \
        return {};                                                                                                                                             \
    }

QString GoogleInterface::accountName() const
{
    CHECK_ACCESS

    return m_config.readEntry("accountName", QString());
}

QString GoogleInterface::clientId() const
{
    CHECK_ACCESS

    return m_config.readEntry("clientId", QString());
}

QString GoogleInterface::clientSecret() const
{
    CHECK_ACCESS

    return m_config.readEntry("clientSecret", QString());
}

QDBusUnixFileDescriptor GoogleInterface::accessToken() const
{
    CHECK_ACCESS

    m_account->setDelayedReply(true);

    QKeychain::ReadPasswordJob *job = new QKeychain::ReadPasswordJob(u"konlineaccounts"_s);
    job->setKey(u"account/" + m_account->id() + u"/google/access_token");

    connect(job, &QKeychain::Job::finished, this, [job, message = m_account->message()] {
        if (job->error()) {
            qCWarning(LOG_KONLINEACCOUNTS_GOOGLE) << "Failed to access token from keychain" << job->errorString();
            auto reply = message.createErrorReply(QDBusError::InternalError, job->errorString());
            QDBusConnection::sessionBus().send(reply);
            return;
        }

        const auto result = FdWriter::write(job->textData().toUtf8());

        if (!result) {
            auto reply = message.createErrorReply(QDBusError::InternalError, u"Internal error"_s);
            QDBusConnection::sessionBus().send(reply);
            return;
        }

        auto reply = message.createReply(QVariant::fromValue(result.value()));
        QDBusConnection::sessionBus().send(reply);
    });

    job->start();

    return {};
}

QDBusUnixFileDescriptor GoogleInterface::refreshToken() const
{
    CHECK_ACCESS

    m_account->setDelayedReply(true);

    QKeychain::ReadPasswordJob *job = new QKeychain::ReadPasswordJob(u"konlineaccounts"_s);
    job->setKey(u"account/" + m_account->id() + u"/google/refresh_token");

    connect(job, &QKeychain::Job::finished, this, [job, message = m_account->message()] {
        if (job->error()) {
            qCWarning(LOG_KONLINEACCOUNTS_GOOGLE) << "Failed to refresh token from keychain" << job->errorString();
            auto reply = message.createErrorReply(QDBusError::InternalError, job->errorString());
            QDBusConnection::sessionBus().send(reply);
            return;
        }

        const auto result = FdWriter::write(job->textData().toUtf8());

        if (!result) {
            auto reply = message.createErrorReply(QDBusError::InternalError, u"Internal error"_s);
            QDBusConnection::sessionBus().send(reply);
            return;
        }

        auto reply = message.createReply(QVariant::fromValue(result.value()));
        QDBusConnection::sessionBus().send(reply);
    });

    job->start();

    return {};
}

QStringList GoogleInterface::scopes() const
{
    CHECK_ACCESS

    return m_config.readEntry("scopes", QStringList());
}
