/*
 *   SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "mastodonsetup.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QOAuthHttpServerReplyHandler>
#include <QUrlQuery>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>

#include <QCoro/QCoroCore>

#include <qt6keychain/keychain.h>

#include "setup_debug.h"

using namespace Qt::Literals;

MastodonSetup::MastodonSetup(QObject *parent)
    : QObject(parent)
{
}

void MastodonSetup::registerMastodon(const QString &instanceUrl)
{
    doRegisterMastodon(instanceUrl);
}

QCoro::Task<void> MastodonSetup::doRegisterMastodon(const QString &_instanceUrl)
{
    const QString instanceUrl = _instanceUrl;

    auto handler = new QOAuthHttpServerReplyHandler(1234, this);

    handler->setCallbackText(i18n("Authentication completed, return to System Settings"));

    connect(handler, &QOAuthHttpServerReplyHandler::tokenRequestErrorOccurred, this, [this](QAbstractOAuth::Error /*error*/, const QString &errorString) {
        qCWarning(LOG_KONLINEACCOUNTS_MASTODON_SETUP) << "Mastodon OAuth callback error" << errorString;
        m_builder->fail(errorString);
    });

    // register
    const QUrl regUrl = QUrl(instanceUrl + u"/api/v1/apps");
    const QJsonObject obj{
        {QStringLiteral("client_name"), i18n("KDE Online Accounts")},
        {QStringLiteral("redirect_uris"), u"http://localhost:1234"_s},
        {QStringLiteral("scopes"), QStringLiteral("read write follow")},
    };
    const QJsonDocument doc(obj);

    QNetworkRequest req(regUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QNetworkAccessManager *nam = new QNetworkAccessManager;

    QNetworkReply *registerReply = nam->post(req, doc.toJson());
    const QByteArray registerReplyData = co_await registerReply;

    if (registerReply->error()) {
        qCWarning(LOG_KONLINEACCOUNTS_MASTODON_SETUP) << "Error registering OAuth app" << registerReply->errorString();
        m_builder->fail(registerReply->errorString());
        co_return;
    }

    QJsonDocument replyDoc = QJsonDocument::fromJson(registerReplyData);

    const QString clientId = replyDoc[u"client_id"].toString();
    const QString clientSecret = replyDoc[u"client_secret"].toString();

    QDesktopServices::openUrl(
        QUrl(instanceUrl + u"/oauth/authorize?client_id=%1&scope=read+write&redirect_uri=http://localhost:1234&response_type=code"_s.arg(clientId)));

    const QVariantMap values = co_await qCoro(handler, &QOAuthHttpServerReplyHandler::callbackReceived);

    qCDebug(LOG_KONLINEACCOUNTS_MASTODON_SETUP) << "Got Mastodon OAuth callback";

    const QString authCode = values[u"code"_s].toString();

    const QUrl tokenUrl = QUrl(instanceUrl + u"/oauth/token");
    QUrlQuery q;

    q.addQueryItem(QStringLiteral("client_id"), clientId);
    q.addQueryItem(QStringLiteral("client_secret"), clientSecret);
    q.addQueryItem(QStringLiteral("redirect_uri"), u"http://localhost:1234"_s);
    q.addQueryItem(QStringLiteral("grant_type"), u"authorization_code"_s);
    q.addQueryItem(QStringLiteral("code"), authCode);

    QNetworkRequest tokenRequest(tokenUrl);
    tokenRequest.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    QNetworkReply *tokenReply = nam->post(tokenRequest, q.toString().toUtf8());
    auto tokenData = co_await tokenReply;

    if (tokenReply->error()) {
        qCWarning(LOG_KONLINEACCOUNTS_MASTODON_SETUP) << "Failed to obtain Mastodon token" << tokenReply->errorString();
        m_builder->fail(tokenReply->errorString());
        co_return;
    }

    QJsonDocument authReplyDoc = QJsonDocument::fromJson(tokenData);

    const QString accessToken = authReplyDoc[u"access_token"].toString();

    auto mastodonGroup = m_builder->config().group(u"Mastodon"_s);
    mastodonGroup.writeEntry("instanceUrl", instanceUrl);
    mastodonGroup.writeEntry("username", "nico");
    mastodonGroup.writeEntry("clientId", clientId);

    auto writeClientSecretJob = new QKeychain::WritePasswordJob(u"konlineaccounts"_s);
    writeClientSecretJob->setKey(u"account/" + m_builder->accountId() + u"/mastodon/client_secret");
    writeClientSecretJob->setTextData(clientSecret);
    writeClientSecretJob->start();

    co_await qCoro(writeClientSecretJob, &QKeychain::WritePasswordJob::finished);

    if (writeClientSecretJob->error()) {
        qCWarning(LOG_KONLINEACCOUNTS_MASTODON_SETUP) << "Failed to write client secret for Mastodon account" << m_builder->accountId()
                                                      << writeClientSecretJob->errorString();
        m_builder->fail(writeClientSecretJob->errorString());
        co_return;
    }

    auto writeAccessTokenJob = new QKeychain::WritePasswordJob(u"konlineaccounts"_s);
    writeAccessTokenJob->setKey(u"account/" + m_builder->accountId() + u"/mastodon/access_token");
    writeAccessTokenJob->setTextData(accessToken);
    writeAccessTokenJob->start();

    co_await qCoro(writeAccessTokenJob, &QKeychain::WritePasswordJob::finished);

    if (writeAccessTokenJob->error()) {
        qCWarning(LOG_KONLINEACCOUNTS_MASTODON_SETUP) << "Failed to write access token for Mastodon account" << m_builder->accountId()
                                                      << writeAccessTokenJob->errorString();
        m_builder->fail(writeAccessTokenJob->errorString());
        co_return;
    }

    m_builder->finish();

    co_return;
}

AccountBuilder *MastodonSetup::builder() const
{
    return m_builder;
}

void MastodonSetup::setBuilder(AccountBuilder *builder)
{
    m_builder = builder;
    Q_EMIT builderChanged();
}
