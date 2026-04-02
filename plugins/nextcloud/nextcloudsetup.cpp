/*
 *   SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "nextcloudsetup.h"
#include "setup_debug.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>

#include <KConfigGroup>
#include <KIO/Global>
#include <KLocalizedString>
#include <KSharedConfig>

#include <qt6keychain/keychain.h>

using namespace Qt::Literals;

NextcloudSetup::NextcloudSetup(QObject *parent)
    : QObject(parent)
    , m_webengineProfile(new QQuickWebEngineProfile(this))
{
    m_webengineProfile->setUrlRequestInterceptor(&m_urlIntercepter);
    m_webengineProfile->setHttpUserAgent(QStringLiteral("Not KAccounts Nextcloud Login"));

    QDesktopServices::setUrlHandler(QStringLiteral("nc"), this, "finalUrlHandler");
}

QUrl createStatusUrl(const QString &input)
{
    QString fixedUrl;
    if (!input.startsWith(QLatin1String("http://")) && !input.startsWith(QLatin1String("https://"))) {
        fixedUrl.append(QLatin1String("https://"));
        fixedUrl.append(input);
    } else {
        fixedUrl = input;
    }

    QUrl url(fixedUrl);

    if (!url.path().endsWith(QLatin1Char('/'))) {
        url.setPath(url.path() + QLatin1Char('/'));
    }

    url.setPath(url.path() + QLatin1String("status.php"));

    return url;
}

void NextcloudSetup::finalUrlHandler(const QUrl &url)
{
    // url is of the form: nc://login/server:<server>&user:<loginname>&password:<password>

    QUrlQuery urlQuery;
    urlQuery.setQueryDelimiters(QLatin1Char(':'), QLatin1Char('&'));
    urlQuery.setQuery(url.path(QUrl::FullyEncoded).mid(1));

    const QString username = urlQuery.queryItemValue(QStringLiteral("user"), QUrl::FullyDecoded);
    const QString password = urlQuery.queryItemValue(QStringLiteral("password"), QUrl::FullyDecoded);

    const QString davUrl = m_server.toString() + u"remote.php/dav"_s;

    auto calDav = m_builder->config().group(u"CalDAV"_s);
    calDav.writeEntry("url", davUrl);
    calDav.writeEntry("username", username);

    auto cardDav = m_builder->config().group(u"CardDAV"_s);
    cardDav.writeEntry("url", davUrl);
    cardDav.writeEntry("username", username);

    auto nextcloudGroup = m_builder->config().group(u"Nextcloud"_s);
    nextcloudGroup.writeEntry("url", m_server.toString());
    nextcloudGroup.writeEntry("username", username);
    nextcloudGroup.writeEntry("storagePath", u"/remote.php/dav/files/%1"_s.arg(username));

    auto job = new QKeychain::WritePasswordJob(u"konlineaccounts"_s);
    job->setKey(u"account/" + m_builder->accountId() + u"/nextcloud/password");
    job->setTextData(password);
    connect(job, &QKeychain::WritePasswordJob::finished, this, [this, job] {
        if (job->error()) {
            qCWarning(LOG_KONLINEACCOUNTS_NEXTCLOUD_SETUP) << "Failed to write password for Nextcloud account" << m_builder->accountId() << job->errorString();
            m_builder->fail(job->errorString());
            return;
        }
        m_builder->finish();
    });
    job->start();
}

void NextcloudSetup::checkServer(const QString &path)
{
    m_errorMessage.clear();
    Q_EMIT errorMessageChanged();

    checkServer(createStatusUrl(path));
}

QCoro::Task<void> NextcloudSetup::checkServer(const QUrl &url)
{
    setWorking(true);

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::AutoDeleteReplyOnFinishAttribute, true);

    QNetworkReply *reply = m_nam.get(request);

    co_await qCoro(reply, &QNetworkReply::finished);

    if (reply->error() || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
        wrongUrlDetected();
        co_return;
    }

    QJsonDocument parser = QJsonDocument::fromJson(reply->readAll());
    QJsonObject map = parser.object();
    if (!map.contains(QStringLiteral("version"))) {
        wrongUrlDetected();
        co_return;
    }

    m_server = KIO::upUrl(request.url());

    m_loginUrl = QUrl(m_server.toString() + u"/index.php/login/flow");
    Q_EMIT loginUrlChanged();

    m_state = WebLogin;
    Q_EMIT stateChanged();

    co_return;
}

void NextcloudUrlIntercepter::interceptRequest(QWebEngineUrlRequestInfo &info)
{
    info.setHttpHeader("OCS-APIREQUEST", "true");
}

bool NextcloudSetup::isWorking()
{
    return m_isWorking;
}

QString NextcloudSetup::errorMessage() const
{
    return m_errorMessage;
}

// When url entered by user is wrong
void NextcloudSetup::wrongUrlDetected()
{
    m_errorMessage = i18n("Unable to connect to Nextcloud at the given server URL. Please check the server URL.");
    setWorking(false);
    Q_EMIT errorMessageChanged();
}

void NextcloudSetup::setWorking(bool start)
{
    if (start == m_isWorking) {
        return;
    }

    m_isWorking = start;
    Q_EMIT isWorkingChanged();
}

AccountBuilder *NextcloudSetup::builder() const
{
    return m_builder;
}

void NextcloudSetup::setBuilder(AccountBuilder *builder)
{
    m_builder = builder;
    Q_EMIT builderChanged();
}
