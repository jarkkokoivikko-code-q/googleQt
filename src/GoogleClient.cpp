#include <QFile>
#include "GoogleClient.h"
#include "Endpoint.h"
#include "google/endpoint/GoogleWebAuth.h"
#include "gmail/GmailRoutes.h"
#include "gtask/GtaskRoutes.h"
#include "gdrive/GdriveRoutes.h"
#include "gcontact/GcontactRoutes.h"
#include "gcontact/GcontactCache.h"

using namespace googleQt;

#ifdef API_QT_AUTOTEST
int g__gclient_alloc_counter = 0;
#endif

gclient_ptr googleQt::createClient(std::shared_ptr<ApiAppInfo> appInfo, std::shared_ptr<ApiAuthInfo> authInfo, gcontact::GContactCacheBase* custom_contacts_cache)
{
    std::shared_ptr<GoogleClient>  rv(new GoogleClient(appInfo, authInfo, custom_contacts_cache));
    return rv;
};

void googleQt::releaseClient(gclient_ptr p)
{
    if (p) {
        p.reset();
    }
};


GoogleClient::GoogleClient(std::shared_ptr<ApiAppInfo> appInfo,
                         std::shared_ptr<ApiAuthInfo> authInfo,
                         gcontact::GContactCacheBase* custom_contacts_cache)
    :ApiClient(appInfo, authInfo)
{
    m_endpoint.reset(new Endpoint(this));
    if (custom_contacts_cache) {
        m_contacts_cache = custom_contacts_cache;
        m_own_contacts_cache = false;
    }
    else {
        m_contacts_cache = new gcontact::GContactCache(*this);
        m_own_contacts_cache = true;
    }
#ifdef API_QT_AUTOTEST
    g__gclient_alloc_counter++;
#endif
};

GoogleClient::~GoogleClient()
{
#ifdef API_QT_AUTOTEST
    g__gclient_alloc_counter--;
#endif
    if (m_own_contacts_cache) {
        delete m_contacts_cache;
    }
};

void GoogleClient::cancelAllRequests() 
{
    m_endpoint->cancelAll();
};

bool GoogleClient::isQueryInProgress()const
{
  return m_endpoint->isQueryInProgress();
};

QString GoogleClient::lastApiCall()
{
#ifdef API_QT_DIAGNOSTICS
    return m_endpoint->lastRequestInfo().request;
#else
    return "";
#endif
}

void GoogleClient::runEventsLoop()const
{
    m_endpoint->runEventsLoop();
};

void GoogleClient::exitEventsLoop()const
{
    m_endpoint->exitEventsLoop();
};

void GoogleClient::setUserId(QString email)
{
    if(email != userId()){
        ApiClient::setUserId(email);
        if(m_gmail_routes){
            m_gmail_routes->onUserReset();
        }
    }
};

QByteArray GoogleClient::lastResponse()
{
    return m_endpoint->lastResponse();
};

void GoogleClient::printLastApiCall()
{
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "API call" << std::endl;
    std::cout << lastApiCall().toStdString() << std::endl;
};

void GoogleClient::printLastResponse() 
{
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "API call" << std::endl;
    std::cout << lastApiCall().toStdString() << std::endl;    
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "response" << std::endl;
    std::cout << lastResponse().toStdString() << std::endl;
};

void GoogleClient::exportLastResponse(QString fileName) 
{
    QFile file_in(fileName);
    if (!file_in.open(QFile::WriteOnly)) {
        qWarning() << "Error opening file: " << fileName;
        return;
    }
    file_in.write(lastApiCall().toUtf8());
    file_in.write("\n-----------------------------------------\n");
    file_in.write(lastResponse());
    file_in.close();
};

GmailRoutes* GoogleClient::gmail()
{
    if(!m_gmail_routes){
        m_gmail_routes.reset(new GmailRoutes(m_endpoint.get()));
    }
    return m_gmail_routes.get();
};

GtaskRoutes* GoogleClient::gtask()
{
    if(!m_gtask_routes){
      m_gtask_routes.reset(new GtaskRoutes(m_endpoint.get()));
    }
    return m_gtask_routes.get();
};

GdriveRoutes* GoogleClient::gdrive() 
{
    if (!m_gdrive_routes) {
        m_gdrive_routes.reset(new GdriveRoutes(m_endpoint.get()));
    }
    return m_gdrive_routes.get();
};

GcontactRoutes* GoogleClient::gcontact()
{
    if (!m_contact_routes) {
        m_contact_routes.reset(new GcontactRoutes(*(m_endpoint.get())));
    }
    return m_contact_routes.get();
};

googleQt::mail_cache::GmailCacheRoutes* GoogleClient::gmail_cache_routes() 
{
    auto m = gmail();
    if (m) {
        return m->cacheRoutes();
    }
    return nullptr;
};

googleQt::mail_cache::GMailSQLiteStorage* GoogleClient::gmail_storage() 
{
    auto s = gmail_cache_routes();
    if (s) {
        return s->storage();
    }
    return nullptr;
};

bool GoogleClient::refreshToken()
{
    int status_code = GoogleWebAuth::refreshToken(m_app, m_auth);
    return status_code == 200;
};

void GoogleClient::setNetworkProxy(const QNetworkProxy& proxy)
{
    m_endpoint->setProxy(proxy);
};

ApiEndpoint* GoogleClient::endpoint()
{
    return m_endpoint.get();
};
