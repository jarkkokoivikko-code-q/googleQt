#include <QUrlQuery>
#include "Endpoint.h"

using namespace googleQt;

Endpoint::Endpoint(ApiClient* c):ApiEndpoint(c)
{
    
}


GoogleClient* Endpoint::client()
{
    GoogleClient* rv = dynamic_cast<GoogleClient*>(m_client);
    return rv;
};

const GoogleClient* Endpoint::client()const
{
    const GoogleClient* rv = dynamic_cast<const GoogleClient*>(m_client);
    return rv;
};

void Endpoint::onErrorUnauthorized(const errors::ErrorInfo*)
{
    if(!client()->refreshToken())
        {
            qWarning() << "Failed to refresh token";
        };
};

QString Endpoint::prepareErrorSummary(int status_code)
{
    QString s;
    switch(status_code)
        {
        case 400:s = QObject::tr("Bad Request"); break;
        case 403:s = QObject::tr("Access denied. Please try to login again."); break;
        case 404:s = QObject::tr("File not found."); break;
        case 500:s = QObject::tr("Internal Server Error. Please try again later."); break;
        }
    QString rv = QString("%1").arg(status_code);
    if (!s.isEmpty())
        rv += QString(" - %1").arg(s);
    return rv;
};

QString Endpoint::prepareErrorInfo(int status_code, const QUrl& url, const QByteArray& data) 
{
    QString rv;
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject js = doc.object();
    if (js.contains("error")) {
        rv = js["error"].toObject()["message"].toString();
    } else {
        rv = QObject::tr("Unexpected error %1").arg(prepareErrorSummary(status_code));
    }
#ifdef API_QT_DIAGNOSTICS
    rv += "\n";
    rv += lastRequestInfo().request;
#endif//API_QT_DIAGNOSTICS
    return rv;
};

void Endpoint::addAppKeyParameter(QUrl& url)const
{
    QUrlQuery q(url);
    q.addQueryItem("key", client()->getAppKey());
    url.setQuery(q);
};
