#pragma once

#include <QString>
#include "ApiAuthInfo.h"
#include "ApiAppInfo.h"
#include "ApiBase.h"

namespace googleQt{
    class GoogleAppInfo;

    class GoogleWebAuth
    {
    public:
        /**
            getCodeAuthorizeUrl - format string that should be opened to enable
            Google access and request for access token, which will be used in all
            API interactions.
        */
        static QUrl getCodeAuthorizeUrl(std::shared_ptr<const ApiAppInfo> appInfo, QString scope, const QString &redirectUrl = QString());

        static QUrl getCodeAuthorizeUrl(std::shared_ptr<const ApiAppInfo> appInfo, const STRING_LIST& scopes, const QString &redirectUrl = QString());

        /**
           buildGetTokenRequest - builds request for new token http (POST)
         */
        static QNetworkRequest buildGetTokenRequest(QByteArray &outData, std::shared_ptr<const ApiAppInfo> appInfo, QString code, std::shared_ptr<ApiAuthInfo> auth, const QString &redirectUrl = QString());

        /**
           getTokenFromCode - makes http call to Google to retrive
           access token by providing authorize code
         */
        static int getTokenFromCode(std::shared_ptr<const ApiAppInfo> appInfo, QString code, std::shared_ptr<ApiAuthInfo> auth, const QString &redirectUrl = QString());

        /**
           buildRefreshTokenRequest - builds request for refresh token http (POST)
         */
        static QNetworkRequest buildRefreshTokenRequest(QByteArray &outData, std::shared_ptr<const ApiAppInfo> appInfo, std::shared_ptr<ApiAuthInfo> auth);

        /**
           refreshToken - makes http call to Google to retrive
           access token by providing refresh token
         */
        static int refreshToken(std::shared_ptr<const ApiAppInfo> appInfo, std::shared_ptr<ApiAuthInfo> auth);

        /**
           buildUserEmailRequest - builds a http request to Google to retrive
           user email
         */
        static QNetworkRequest buildUserEmailRequest(std::shared_ptr<ApiAuthInfo> auth);

        /**
           updateUserEmail - makes http call to Google to retrive
           user email
         */
        static void updateUserEmail(std::shared_ptr<ApiAuthInfo> auth);

        /**
           revoke - revokes access token
         */
        static QNetworkRequest buildRevokeRequest(std::shared_ptr<ApiAuthInfo> auth);

        /**
           revoke - revokes access token
         */
        static void revoke(std::shared_ptr<ApiAuthInfo> auth);

        /**
        * Get user email.
        */
        static QString authScope_userinfo_email();

        /**
        * Create, read, update, and delete labels only.
        */
        static QString authScope_gmail_labels();
        
        /**
         *  Read all resources and their metadata—no write operations.
         */
        static QString authScope_gmail_readonly();

        /**
         * Create, read, update, and delete drafts. Send messages and drafts.
         */
        static QString authScope_gmail_compose();

        /**
         * Send messages only. No read or modify privileges on mailbox.
         */
        static QString authScope_gmail_send();

        /**
         * All read/write operations except immediate, permanent deletion of threads and messages, bypassing Trash.
         */
        static QString authScope_gmail_modify();

        /**
         * Full access to the account, including permanent deletion of threads and messages.
         */
        static QString authScope_full_access();

        /**
        * read/write access to Tasks
        */
        static QString authScope_tasks();

        /**
        * read-only access to Tasks
        */
        static QString authScope_tasks_readonly();

        /**
        * GDrive access
        */
        static QString authScope_gdrive();

        /**
        * GDrive file access
        */
        static QString authScope_gdrive_file();

        /**
        * GDrive readonly access
        */
        static QString authScope_gdrive_readonly();

        /**
        * AppDataFolder access
        */
        static QString authScope_gdrive_appdata();
        
        /**
        * read/write access to Contacts and Contact Groups
        */
        static QString authScope_contacts_modify();

        /**
        * read-only access to Contacts and Contact Groups
        */
        static QString authScope_contacts_read_only();

        /**
        * read/write access to People Contacts and Contact Groups
        
        static QString authScope_contacts();
        */
    protected:
        static int updateToken(const QNetworkRequest& req, std::shared_ptr<ApiAuthInfo> auth, const QByteArray& reqData);
    };
};
