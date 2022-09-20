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
    QUrl url(QString("https://%1/%2").arg(GoogleHost::DEFAULT().getAuth()).arg("o/oauth2/auth"));
    QUrlQuery q;
    q.addQueryItem("response_type", "code");
    q.addQueryItem("client_id", appInfo->getKey());
    if (!redirectUrl.isEmpty())
        q.addQueryItem("redirect_uri", redirectUrl);
    else
        q.addQueryItem("redirect_uri", "urn:ietf:wg:oauth:2.0:oob");
    q.addQueryItem("scope", scope);
    
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

int GoogleWebAuth::updateToken(const QUrl& url, std::shared_ptr<ApiAuthInfo> auth, const QString& str)
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
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    
    QNetworkReply *reply = mgr.post(req, str.toUtf8());
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

int GoogleWebAuth::getTokenFromCode(std::shared_ptr<const ApiAppInfo> appInfo, QString code, std::shared_ptr<ApiAuthInfo> auth, const QString &redirectUrl)
{
    QUrl url(QString("https://%1/%2").arg(GoogleHost::DEFAULT().getAuth()).arg("o/oauth2/token"));
    QString str = QString("code=%1&client_id=%2&client_secret=%3&grant_type=%4&redirect_uri=%5")
        .arg(code)
        .arg(appInfo->getKey())
        .arg(appInfo->getSecret())
        .arg("authorization_code")
        .arg(!redirectUrl.isEmpty() ? redirectUrl : QString("urn:ietf:wg:oauth:2.0:oob"));

    int status_code = updateToken(url, auth, str);
    if (status_code == 200) {
        updateUserEmail(auth);
    }
    return status_code;
};

int GoogleWebAuth::refreshToken(std::shared_ptr<const ApiAppInfo> appInfo, std::shared_ptr<ApiAuthInfo> auth)
{
    QUrl url(QString("https://%1/%2").arg(GoogleHost::DEFAULT().getAuth()).arg("o/oauth2/token"));
    QString str = QString("refresh_token=%1&client_id=%2&grant_type=%3")
        .arg(auth->getRefreshToken())
        .arg(appInfo->getKey())
        .arg("refresh_token");
    if (!appInfo->getSecret().isEmpty())
        str.append(QString("&client_secret=%1").arg(appInfo->getSecret()));

    return updateToken(url, auth, str);
};

void GoogleWebAuth::updateUserEmail(std::shared_ptr<ApiAuthInfo> auth)
{
    QUrl url(u"https://www.googleapis.com/oauth2/v2/userinfo"_qs);

    QNetworkAccessManager mgr;
    QNetworkRequest req(url);
    QString bearer = QString("Bearer %1").arg(auth->getAccessToken());
    req.setRawHeader("Authorization", bearer.toUtf8());

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

void GoogleWebAuth::revoke(std::shared_ptr<ApiAuthInfo> auth)
{
    if (auth->getAccessToken().isEmpty())
        return;

    QUrl url(u"https://accounts.google.com/o/oauth2/revoke?token=%1"_qs.arg(auth->getAccessToken()));

    QNetworkAccessManager mgr;
    QNetworkRequest req(url);
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

DEFINE_SCOPE(authScope_userinfo_email,  "https://www.googleapis.com/auth/userinfo.email");
DEFINE_SCOPE(authScope_gmail_labels,    "https://www.googleapis.com/auth/gmail.labels");
DEFINE_SCOPE(authScope_gmail_readonly,  "https://www.googleapis.com/auth/gmail.readonly");
DEFINE_SCOPE(authScope_gmail_compose,   "https://www.googleapis.com/auth/gmail.compose");
DEFINE_SCOPE(authScope_gmail_send,      "https://www.googleapis.com/auth/gmail.send");
DEFINE_SCOPE(authScope_gmail_modify,    "https://www.googleapis.com/auth/gmail.modify");
DEFINE_SCOPE(authScope_full_access,     "https://mail.google.com/");
DEFINE_SCOPE(authScope_tasks,           "https://www.googleapis.com/auth/tasks");
DEFINE_SCOPE(authScope_tasks_readonly,  "https://www.googleapis.com/auth/tasks.readonly");
DEFINE_SCOPE(authScope_gdrive,          "https://www.googleapis.com/auth/drive");
DEFINE_SCOPE(authScope_gdrive_readonly, "https://www.googleapis.com/auth/drive.readonly");
DEFINE_SCOPE(authScope_gdrive_appdata,  "https://www.googleapis.com/auth/drive.appdata");
DEFINE_SCOPE(authScope_contacts_modify,  "https://www.google.com/m8/feeds");
DEFINE_SCOPE(authScope_contacts_read_only,  "https://www.googleapis.com/auth/contacts.readonly");

#undef DEFINE_SCOPE
