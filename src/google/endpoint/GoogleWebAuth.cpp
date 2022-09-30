#include <QUrl>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <iostream>
#include "GoogleWebAuth.h"
#include "GoogleHost.h"
#include "ApiException.h"

using namespace googleQt;

QUrl GoogleWebAuth::getCodeAuthorizeUrl(std::shared_ptr<const ApiAppInfo> appInfo, QString scope, const QString &redirectUrl)
{
    QUrl url(u"https://%1/%2"_qs.arg(GoogleHost::DEFAULT().getAuth(), u"o/oauth2/auth"_qs));
    QUrlQuery q;
    q.addQueryItem(u"response_type"_qs, u"code"_qs);
    q.addQueryItem(u"client_id"_qs, appInfo->getKey());
    if (!redirectUrl.isEmpty())
        q.addQueryItem(u"redirect_uri"_qs, redirectUrl);
    else
        q.addQueryItem(u"redirect_uri"_qs, u"urn:ietf:wg:oauth:2.0:oob"_qs);
    q.addQueryItem(u"scope"_qs, scope);
    
    url.setQuery(q);
    return url;
};

QUrl GoogleWebAuth::getCodeAuthorizeUrl(std::shared_ptr<const ApiAppInfo> appInfo, const STRING_LIST& scopes, const QString &redirectUrl)
{
    QString scope_summary;

    for(STRING_LIST::const_iterator i = scopes.begin(); i != scopes.end();i++)
        {
            scope_summary += *i;
            scope_summary += "+";
        }
    scope_summary = scope_summary.left(scope_summary.length() - 1);

    return getCodeAuthorizeUrl(appInfo, scope_summary, redirectUrl);
};

int GoogleWebAuth::updateToken(const QNetworkRequest& req, std::shared_ptr<ApiAuthInfo> auth, const QByteArray& reqData)
{
#ifdef API_QT_AUTOTEST
    Q_UNUSED(url);
    Q_UNUSED(str);

    QJsonObject js;
    js["access_token"]  = "access_token_value_123";
    js["refresh_token"] = "refresh_token_value_456";
    js["token_type"]    = "my_token_type";
    js["expires_in"]    = 3599;
    js["expire_time"]   = QDateTime::currentDateTime().addSecs(3599).toString(Qt::ISODate);

    bool rv = auth->updateToken(js);
    return 200;
#else
    QNetworkAccessManager mgr;
    QNetworkReply *reply = mgr.post(req, reqData);
    {
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();
    }

    const QByteArray data = reply->readAll();
    int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    switch (status_code) {
    case 200:
        if (!data.isEmpty())
            auth->updateToken(QJsonDocument::fromJson(data).object());
        break;

    default:
        qDebug() << "Failed to update token. Unexpected status" << status_code;
        break;
    }
    reply->deleteLater();
    return status_code;
#endif
}

QNetworkRequest GoogleWebAuth::buildGetTokenRequest(QByteArray &outData, std::shared_ptr<const ApiAppInfo> appInfo, QString code, std::shared_ptr<ApiAuthInfo> auth, const QString &redirectUrl)
{
    QUrl url(u"https://%1/%2"_qs.arg(GoogleHost::DEFAULT().getAuth(), u"o/oauth2/token"_qs));
    QString str = u"code=%1&client_id=%2&client_secret=%3&grant_type=%4&redirect_uri=%5"_qs
        .arg(code, appInfo->getKey(), appInfo->getSecret(), u"authorization_code"_qs,
             !redirectUrl.isEmpty() ? redirectUrl : u"urn:ietf:wg:oauth:2.0:oob"_qs);
    outData = str.toUtf8();

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded"_qba);

    return req;
};

int GoogleWebAuth::getTokenFromCode(std::shared_ptr<const ApiAppInfo> appInfo, QString code, std::shared_ptr<ApiAuthInfo> auth, const QString &redirectUrl)
{
    QByteArray data;
    QNetworkRequest req = buildGetTokenRequest(data, appInfo, code, auth, redirectUrl);

    int status_code = updateToken(req, auth, data);
    if (status_code == 200) {
        updateUserEmail(auth);
    }
    return status_code;
};

QNetworkRequest GoogleWebAuth::buildRefreshTokenRequest(QByteArray &outData, std::shared_ptr<const ApiAppInfo> appInfo, std::shared_ptr<ApiAuthInfo> auth)
{
    QUrl url(u"https://%1/%2"_qs.arg(GoogleHost::DEFAULT().getAuth(), u"o/oauth2/token"_qs));
    QString str = u"refresh_token=%1&client_id=%2&grant_type=%3"_qs
        .arg(auth->getRefreshToken(), appInfo->getKey(), u"refresh_token"_qs);
    if (!appInfo->getSecret().isEmpty())
        str.append(u"&client_secret=%1"_qs.arg(appInfo->getSecret()));
    outData = str.toUtf8();

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded"_qba);

    return req;
};

int GoogleWebAuth::refreshToken(std::shared_ptr<const ApiAppInfo> appInfo, std::shared_ptr<ApiAuthInfo> auth)
{
    QByteArray data;
    QNetworkRequest req = buildRefreshTokenRequest(data, appInfo, auth);

    return updateToken(req, auth, data);
};

QNetworkRequest GoogleWebAuth::buildUserEmailRequest(std::shared_ptr<ApiAuthInfo> auth)
{
    QUrl url(u"https://%1/%2"_qs.arg(GoogleHost::DEFAULT().getApi(), u"oauth2/v2/userinfo"_qs));
    QNetworkRequest req(url);
    req.setRawHeader("Authorization"_qba, u"Bearer %1"_qs.arg(auth->getAccessToken()).toUtf8());
    return req;
};

void GoogleWebAuth::updateUserEmail(std::shared_ptr<ApiAuthInfo> auth)
{
    QNetworkRequest req = buildUserEmailRequest(auth);
    QNetworkAccessManager mgr;

    QNetworkReply *reply = mgr.get(req);
    {
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();
    }

    int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    switch (status_code) {
    case 200: {
        const QByteArray data = reply->readAll();
        if (!data.isEmpty()) {
            QJsonObject userinfo = QJsonDocument::fromJson(data).object();
            auth->setEmail(userinfo["email"].toString());
            auth->setUserId(userinfo["id"].toString());
            auth->save();
        }
        break;
    }

    default:
        qDebug() << "Failed to update user email. Unexpected status" << status_code;
        break;
    }
    reply->deleteLater();
};

QNetworkRequest GoogleWebAuth::buildRevokeRequest(std::shared_ptr<ApiAuthInfo> auth)
{
    QUrl url(u"https://%1/%2"_qs.arg(GoogleHost::DEFAULT().getAuth(), u"o/oauth2/revoke"_qs));
    QUrlQuery q;
    q.addQueryItem(u"token"_qs, auth->getAccessToken());
    url.setQuery(q);
    QNetworkRequest req(url);
    return req;
};

void GoogleWebAuth::revoke(std::shared_ptr<ApiAuthInfo> auth)
{
    if (auth->getAccessToken().isEmpty())
        return;

    QNetworkAccessManager mgr;
    QNetworkRequest req = buildRevokeRequest(auth);
    QNetworkReply *reply = mgr.get(req);
    {
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();
    }

    int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    switch (status_code) {
    case 200:
        qDebug() << "Access revoked successfully.";
        break;

    default:
        qDebug() << "Failed to revoke token. Unexpected status" << status_code;
        break;
    }
    reply->deleteLater();
};

#define DEFINE_SCOPE(N, L) QString GoogleWebAuth::N(){return L;};

DEFINE_SCOPE(authScope_userinfo_email,      u"https://www.googleapis.com/auth/userinfo.email"_qs);
DEFINE_SCOPE(authScope_gmail_labels,        u"https://www.googleapis.com/auth/gmail.labels"_qs);
DEFINE_SCOPE(authScope_gmail_readonly,      u"https://www.googleapis.com/auth/gmail.readonly"_qs);
DEFINE_SCOPE(authScope_gmail_compose,       u"https://www.googleapis.com/auth/gmail.compose"_qs);
DEFINE_SCOPE(authScope_gmail_send,          u"https://www.googleapis.com/auth/gmail.send"_qs);
DEFINE_SCOPE(authScope_gmail_modify,        u"https://www.googleapis.com/auth/gmail.modify"_qs);
DEFINE_SCOPE(authScope_full_access,         u"https://mail.google.com/"_qs);
DEFINE_SCOPE(authScope_tasks,               u"https://www.googleapis.com/auth/tasks"_qs);
DEFINE_SCOPE(authScope_tasks_readonly,      u"https://www.googleapis.com/auth/tasks.readonly"_qs);
DEFINE_SCOPE(authScope_gdrive,              u"https://www.googleapis.com/auth/drive"_qs);
DEFINE_SCOPE(authScope_gdrive_file,         u"https://www.googleapis.com/auth/drive.file"_qs);
DEFINE_SCOPE(authScope_gdrive_readonly,     u"https://www.googleapis.com/auth/drive.readonly"_qs);
DEFINE_SCOPE(authScope_gdrive_appdata,      u"https://www.googleapis.com/auth/drive.appdata"_qs);
DEFINE_SCOPE(authScope_contacts_modify,     u"https://www.google.com/m8/feeds"_qs);
DEFINE_SCOPE(authScope_contacts_read_only,  u"https://www.googleapis.com/auth/contacts.readonly"_qs);

#undef DEFINE_SCOPE
