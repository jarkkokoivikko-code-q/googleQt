#include <QJsonObject>
#include <QFile>
#include <iostream>
#include "ApiAuthInfo.h"
#include "ApiUtil.h"

using namespace googleQt;

ApiAuthInfo::ApiAuthInfo() 
{
#ifdef API_QT_AUTOTEST
    m_accessToken = "TEST-TOKEN";
#endif //API_QT_AUTOTEST
};

ApiAuthInfo::ApiAuthInfo(QString token_file, int scope) :
    m_token_file(token_file)
{
};

void ApiAuthInfo::purge()
{
    if (!m_token_file.isEmpty())
        QFile::remove(m_token_file);
    m_accessToken.clear();
    m_refreshToken.clear();
    m_type.clear();
    m_expire_time = QDateTime();
    m_scope.clear();
    m_email.clear();
    m_userId.clear();
};

bool ApiAuthInfo::readFromFile(QString path)
{
    QJsonObject js;
    if(!loadJsonFromFile(path, js))
        return false;
    m_accessToken = js["access_token"].toString();
    m_refreshToken = js["refresh_token"].toString();
    m_type = js["token_type"].toString();
    m_expire_time = QDateTime::fromString(js["expire_time"].toString(), Qt::ISODate);
    m_scope = js["scope"].toString();
    m_email = js["email"].toString();
    m_userId = js["user_id"].toString();
    return true;
};

bool ApiAuthInfo::storeToFile(QString path)const
{
    QJsonObject js;
    js["access_token"] = m_accessToken;
    js["refresh_token"] =  m_refreshToken;
    js["token_type"] = m_type;
    js["expire_time"] = m_expire_time.toString(Qt::ISODate);
    js["scope"] = m_scope;
    js["email"] = m_email;
    js["user_id"] = m_userId;

    if(!storeJsonToFile(path, js))
        return false;
    return true;

};

bool ApiAuthInfo::reload()
{
    return !m_token_file.isEmpty() ? readFromFile(m_token_file) : false;
};

bool ApiAuthInfo::save()
{
    return !m_token_file.isEmpty() ? storeToFile(m_token_file) : false;
};

int ApiAuthInfo::getExpirationInSeconds() const
{
    if (!m_expire_time.isValid())
        return -1;

    const QDateTime currentDateTime = QDateTime::currentDateTime();
    return qMax(currentDateTime.secsTo(m_expire_time), 0);
};

bool ApiAuthInfo::updateToken(const QJsonObject& js_in)
{
    m_accessToken = js_in["access_token"].toString();
    QString refreshToken = js_in["refresh_token"].toString();
    if(!refreshToken.isEmpty())
        {
            m_refreshToken = refreshToken;
        }
    m_type = js_in["token_type"].toString();
    m_expire_time = QDateTime::currentDateTime().addSecs(js_in["expires_in"].toInt());
    m_scope = js_in["scope"].toString();
    //do don't update email - it will be empty on refresh
    //email/userid is something that is setup on client side

    save();
    
    return true;
};
