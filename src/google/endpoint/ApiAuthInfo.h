#pragma once
#include <QDateTime>
#include <QString>

namespace googleQt{
    class ApiAuthInfo
    {
        friend class GoogleWebAuth;
    public:
        ApiAuthInfo();
        ApiAuthInfo(QString token_file, int scope = 0);
        virtual ~ApiAuthInfo(){};
        
        void purge();
        bool reload();
        bool save();
        bool updateToken(const QJsonObject& js_in);
        void setEmail(QString email){ m_email = email; }
        void setUserId(QString id){ m_userId = id; }

        QString getAccessToken()const {return m_accessToken;}
        QString getRefreshToken()const{return m_refreshToken;}
        QString getEmail()const{return m_email; }
        QString getUserId()const{return m_userId; }
        int     getExpirationInSeconds() const;
        QString getScope()const { return m_scope; }

    protected:
        virtual bool readFromFile(QString path);
        virtual bool storeToFile(QString path)const;

    protected:
        QString m_token_file;
        QString m_accessToken;
        QString m_refreshToken;
        QString m_type;
        QDateTime m_expire_time;
        QString m_scope;
        QString m_email;
        QString m_userId;
    };
}//dropboxQt
