/*
 *   SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "googlesetup.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDebug>

#include <KConfigGroup>
#include <KSharedConfig>

#include <qt6keychain/keychain.h>

#include "setup_debug.h"

using namespace Qt::Literals;

QList<QUrl> googleScopes()
{
    // TODO: determine it based on what user wants?
    return {
        KGAPI2::Account::accountInfoScopeUrl(),
        KGAPI2::Account::calendarScopeUrl(),
        KGAPI2::Account::calendarEventsScopeUrl(),
        KGAPI2::Account::peopleScopeUrl(),
        KGAPI2::Account::tasksScopeUrl(),
    };
};

QString clientId()
{
    return QStringLiteral("554041944266.apps.googleusercontent.com");
}

QString clientSecret()
{
    return QStringLiteral("mdT1DjzohxN3npUUzkENT0gO");
}

GoogleSetup::GoogleSetup(QObject *parent)
    : QObject(parent)
{
    doWork();
}

QCoro::Task<void> GoogleSetup::doWork()
{
    auto account = KGAPI2::AccountPtr(new KGAPI2::Account());
    const QList<QUrl> resourceScopes = googleScopes();
    for (const QUrl &scope : resourceScopes) {
        if (!account->scopes().contains(scope)) {
            account->addScope(scope);
        }
    }
    auto authJob = new KGAPI2::AuthJob(account, clientId(), clientSecret());
    authJob->setUsername(QString());

    co_await qCoro(authJob, &KGAPI2::AuthJob::finished);

    account = authJob->account();

    if (authJob->error() != KGAPI2::NoError) {
        qCWarning(LOG_KONLINEACCOUNTS_GOOGLE_SETUP) << "error!" << authJob->error() << authJob->errorString();
        m_builder->fail(authJob->errorString());
        co_return;
    }

    auto googleGroup = m_builder->config().group(u"Google"_s);
    googleGroup.writeEntry("accountName", account->accountName());
    googleGroup.writeEntry("clientId", clientId());
    googleGroup.writeEntry("clientSecret", clientSecret());
    googleGroup.writeEntry("scopes", account->scopes());

    auto writeRefreshTokenJob = new QKeychain::WritePasswordJob(u"konlineaccounts"_s);
    writeRefreshTokenJob->setKey(u"account/" + m_builder->accountId() + u"/google/refresh_token");
    writeRefreshTokenJob->setTextData(account->refreshToken());
    writeRefreshTokenJob->start();

    co_await qCoro(writeRefreshTokenJob, &QKeychain::WritePasswordJob::finished);

    if (writeRefreshTokenJob->error()) {
        qCWarning(LOG_KONLINEACCOUNTS_GOOGLE_SETUP) << "Failed to write refresh token for Google account" << m_builder->accountId()
                                                    << writeRefreshTokenJob->errorString();
        m_builder->fail(writeRefreshTokenJob->errorString());
        co_return;
    }

    auto writeAccessTokenJob = new QKeychain::WritePasswordJob(u"konlineaccounts"_s);
    writeAccessTokenJob->setKey(u"account/" + m_builder->accountId() + u"/google/access_token");
    writeAccessTokenJob->setTextData(account->accessToken());
    writeAccessTokenJob->start();

    co_await qCoro(writeAccessTokenJob, &QKeychain::WritePasswordJob::finished);

    if (writeAccessTokenJob->error()) {
        qCWarning(LOG_KONLINEACCOUNTS_GOOGLE_SETUP) << "Failed to write access token for Google account" << m_builder->accountId()
                                                    << writeAccessTokenJob->errorString();
        m_builder->fail(writeAccessTokenJob->errorString());
        co_return;
    }

    m_builder->finish();
}

AccountBuilder *GoogleSetup::builder() const
{
    return m_builder;
}

void GoogleSetup::setBuilder(AccountBuilder *builder)
{
    m_builder = builder;
    Q_EMIT builderChanged();
}
