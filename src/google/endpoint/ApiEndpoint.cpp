#include <iostream>
#include <QBuffer>
#include <QTimer>
#include <QLoggingCategory>
#include "ApiEndpoint.h"

#ifdef API_QT_AUTOTEST
#include "ApiAutotest.h"
#define LOG_REQUEST     ApiAutotest::INSTANCE().logRequest(lastRequestInfo().request);
#endif

#if defined(API_QT_DIAGNOSTICS) || defined(QT_DEBUG)
#define TRACE_REQUEST(C, R, D)     QString rinfo = QString(C) + " " + R.url().toString() + "\n";\
                                    QList<QByteArray> lst = R.rawHeaderList();\
                                    for (QList<QByteArray>::iterator i = lst.begin(); i != lst.end(); i++)\
                                    rinfo += QString("--header %1 : %2 \n").arg(i->constData()).arg(R.rawHeader(*i).constData());\
                                    if(!QString(D).isEmpty())rinfo += QString("--data %1").arg(D);\
                                    updateLastRequestInfo(rinfo);\


#else
    #define TRACE_REQUEST(C, R, D)
#endif

Q_DECLARE_LOGGING_CATEGORY(lcGoogleQt)

using namespace googleQt;

int googleQt::TIMES_TO_REFRESH_TOKEN_BEFORE_GIVEUP = 2;

ApiEndpoint::ApiEndpoint(ApiClient* c):m_client(c)
{
}

ApiEndpoint::~ApiEndpoint() 
{

};

void ApiEndpoint::runEventsLoop()const
{
    m_loop.exec();
};

void ApiEndpoint::exitEventsLoop()const
{
    m_loop.exit();
};


void ApiEndpoint::setProxy(const QNetworkProxy& proxy)
{
    networkAccessManager()->setProxy(proxy);
};

void ApiEndpoint::setNetworkAccessManager(QNetworkAccessManager *networkAccessManager)
{
    m_con_mgr = networkAccessManager;
};

QNetworkAccessManager *ApiEndpoint::networkAccessManager()
{
    if (!m_con_mgr) {
        if (!m_own_con_mgr)
            m_own_con_mgr.reset(new QNetworkAccessManager());
        return m_own_con_mgr.get();
    }
    return m_con_mgr;
};

bool ApiEndpoint::isQueryInProgress()const 
{
    return !m_replies_in_progress.empty();
};

void ApiEndpoint::abortRequests()
{
    if (!m_replies_in_progress.empty()) {
        NET_REPLIES_IN_PROGRESS copy_of_replies = m_replies_in_progress;
        std::for_each(copy_of_replies.begin(), copy_of_replies.end(), [](std::pair<QNetworkReply*, std::shared_ptr<FINISHED_REQ>> p)
        {
            p.first->abort();
        });
    }
};

void ApiEndpoint::cancelAll()
{
#ifdef API_QT_AUTOTEST
    ApiAutotest::INSTANCE().cancellAll();
#endif
    if (!m_replies_in_progress.empty()) {
        abortRequests();
        for (int i = 0; !m_replies_in_progress.empty() && i < 5; i++) {
            {
                QEventLoop loop;
                QTimer::singleShot(200, &loop, &QEventLoop::quit);
                loop.exec();
            }
            if (m_replies_in_progress.empty())
                break;

            qWarning() << "googleQt/trying to abort network request" 
                << "size=" << m_replies_in_progress.size() 
                << "attempt=" << i;
            abortRequests();
        }
    }

    if (!m_replies_in_progress.empty()) {
        qWarning() << "Warning googleQt/failed to abort network request"
            << "size=" << m_replies_in_progress.size();
    }
};

void ApiEndpoint::registerReply(std::shared_ptr<requester>& rb, QNetworkReply* r, std::shared_ptr<FINISHED_REQ> finishedLambda)
{
    Q_UNUSED(rb);

    QObject::connect(r, &QNetworkReply::downloadProgress, [&](qint64 bytesProcessed, qint64 total) {
        emit m_client->downloadProgress(bytesProcessed, total);
    });

    QObject::connect(r, &QNetworkReply::uploadProgress, [&](qint64 bytesProcessed, qint64 total) {
        emit m_client->uploadProgress(bytesProcessed, total);
    });

    NET_REPLIES_IN_PROGRESS::iterator i = m_replies_in_progress.find(r);
    if (i != m_replies_in_progress.end())
    {
        qWarning() << "Duplicate reply objects registered, map size:" << m_replies_in_progress.size() << r;
    }

    m_replies_in_progress[r] = finishedLambda;
};

void ApiEndpoint::unregisterReply(QNetworkReply* r)
{
    NET_REPLIES_IN_PROGRESS::iterator i = m_replies_in_progress.find(r);
    if (i == m_replies_in_progress.end())
    {
        qWarning() << "Failed to locate reply objects in registered map, map size:" << m_replies_in_progress.size() << r;
    }
    else
    {
        m_replies_in_progress.erase(i);
    }
    r->deleteLater();
};

DiagnosticRequestInfo ApiEndpoint::lastRequestInfo()const
{
#ifdef API_QT_DIAGNOSTICS
    if (!m_requests.empty()) {
        auto i = m_requests.rbegin();
        return *i;
    }
    DiagnosticRequestInfo r;
    return r;
#else
    DiagnosticRequestInfo r;
    r.tag = "";
    r.request = "last_req_info is not available because googleQt lib was compiled without API_QT_DIAGNOSTICS tracing option.";
    return r;
#endif//API_QT_DIAGNOSTICS
};

QString ApiEndpoint::diagnosticTimeStamp() 
{
    QString rv = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    return rv;
};

void ApiEndpoint::setLastResponse(const QByteArray &lastResponse)
{
    m_last_response = lastResponse;
#ifdef QT_DEBUG
    qCDebug(lcGoogleQt).noquote() << lastResponse;
#endif
}

void ApiEndpoint::diagnosticSetRequestTag(QString s)
{    
#ifdef API_QT_DIAGNOSTICS
    m_diagnosticsRequestTag = s;
    auto o = qWarning();
    o.noquote();
    o << diagnosticTimeStamp() << "[gapi]" << s;
#endif
}

const DGN_LIST& ApiEndpoint::diagnosticRequests()const 
{
#ifdef API_QT_DIAGNOSTICS
    return m_requests;
#else
    static DGN_LIST rv;
    return rv;
#endif
};

void ApiEndpoint::diagnosticClearRequestsList() 
{
#ifdef API_QT_DIAGNOSTICS
    m_requests.clear();
#endif
};

void ApiEndpoint::diagnosticSetRequestContext(QString s) 
{ 
#ifdef API_QT_DIAGNOSTICS
    m_diagnosticsRequestContext = s; 
#endif
}

void ApiEndpoint::diagnosticLogAsyncTask(EndpointRunnable* task, TaskState s)
{
#ifdef API_QT_DIAGNOSTICS
    QString sname = "";
    switch (s) 
    {
    case TaskState::started:    sname = "s"; break;
    case TaskState::completed:  sname = "c"; break;
    case TaskState::failed:     sname = "f"; break;
    case TaskState::canceled:   sname = "e"; break;
    }
    qCWarning(lcGoogleQt).noquote() << diagnosticTimeStamp() << QString("[g-diagn-task][%1][%2][%3][%4]")
                        .arg(sname)
                        .arg(m_diagnosticsRequestContext)
                        .arg(m_diagnosticsRequestTag)
                        .arg(task->name());
#else
    Q_UNUSED(task);
    Q_UNUSED(s);
#endif
};

void ApiEndpoint::diagnosticLogSQL(QString sql, QString prefix) 
{
    Q_UNUSED(sql);
    Q_UNUSED(prefix);
    /*
#ifdef API_QT_DIAGNOSTICS
    QDebug o = qWarning();
    o.noquote();
    o << diagnosticTimeStamp() << QString("[g-diagn-sql-%1][%2][%3][%4]")
        .arg(prefix)
        .arg(m_diagnosticsRequestContext)
        .arg(m_diagnosticsRequestTag)
        .arg(sql);
#endif
    */
};

void ApiEndpoint::updateLastRequestInfo(QString s)
{
#ifdef API_QT_DIAGNOSTICS
    DiagnosticRequestInfo r;
    r.tag = m_diagnosticsRequestTag;
    r.context = m_diagnosticsRequestContext;
    r.request = s;
    m_requests.push_back(r);
    if (m_requests.size() > 512) {
        m_requests.erase(m_requests.begin(), m_requests.begin() + 256);
    }
#endif //API_QT_DIAGNOSTICS
#ifdef QT_DEBUG
    qCDebug(lcGoogleQt).noquote() << QStringView(s.utf16(), 1024) << (s.length() > 1024 ? " ..." : "");
#endif
};

QNetworkReply* ApiEndpoint::getData(const QNetworkRequest &req)
{
    TRACE_REQUEST("GET", req, "");
    
#ifdef API_QT_AUTOTEST
    LOG_REQUEST;
    return nullptr;
#else
    QNetworkReply *r = networkAccessManager()->get(req);
    return r;
#endif
};

QNetworkReply* ApiEndpoint::postData(const QNetworkRequest &req, const QByteArray& data)
{
    TRACE_REQUEST("POST", req, data.constData());

#ifdef API_QT_AUTOTEST
    LOG_REQUEST;
    return nullptr;
#else
    QNetworkReply *r = networkAccessManager()->post(req, data);
    return r;
#endif
};

QNetworkReply* ApiEndpoint::postData(const QNetworkRequest &req, QHttpMultiPart* mpart)
{
    TRACE_REQUEST("POST", req, "multipart");

#ifdef API_QT_AUTOTEST
    Q_UNUSED(mpart);
    LOG_REQUEST;
    return nullptr;
#else
    QNetworkReply *r = networkAccessManager()->post(req, mpart);
    mpart->setParent(r);
    return r;
#endif
};

QNetworkReply* ApiEndpoint::putData(const QNetworkRequest &req, const QByteArray& data)
{    
    TRACE_REQUEST("PUT", req, data.constData());

#ifdef API_QT_AUTOTEST
    LOG_REQUEST;
    return nullptr;
#else
    QNetworkReply *r = networkAccessManager()->put(req, data);
    return r;
#endif
};

QNetworkReply* ApiEndpoint::deleteData(const QNetworkRequest &req)
{    
    TRACE_REQUEST("DELETE", req, "");

#ifdef API_QT_AUTOTEST
    LOG_REQUEST;
    return nullptr;
#else
    QNetworkReply *r = networkAccessManager()->deleteResource(req);
    return r;
#endif
};

QNetworkReply* ApiEndpoint::patchData(const QNetworkRequest &req, const QByteArray& data)
{
    TRACE_REQUEST("PATCH", req, data.constData());

#ifdef API_QT_AUTOTEST
    LOG_REQUEST;
    return nullptr;
#else
    QBuffer *buf = new QBuffer();
    buf->setData(data);
    buf->open(QBuffer::ReadOnly);
    QNetworkReply *r = networkAccessManager()->sendCustomRequest(req, "PATCH", buf);
    buf->setParent(r);
    return r;
#endif
};

QNetworkReply *ApiEndpoint::patchData(const QNetworkRequest &req, QHttpMultiPart *mpart)
{
    TRACE_REQUEST("PATCH", req, "multipart");

#ifdef API_QT_AUTOTEST
    Q_UNUSED(mpart);
    LOG_REQUEST;
    return nullptr;
#else
    QNetworkReply *r = networkAccessManager()->sendCustomRequest(req, "PATCH", mpart);
    mpart->setParent(r);
    return r;
#endif
};

///MPartUpload_requester
QNetworkReply* ApiEndpoint::MPartUpload_requester::request(QNetworkRequest& r)
{
    QByteArray meta_bytes;
    if (m_js_out.isEmpty()) {
        meta_bytes.append("null");
    }
    else {
        QJsonDocument doc(m_js_out);
        meta_bytes = doc.toJson(QJsonDocument::Compact);
    }

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart jsonPart;
    jsonPart.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=UTF-8"_qba);
    jsonPart.setBody(meta_bytes);
    multiPart->append(jsonPart);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream"_qba);
    filePart.setBodyDevice(m_readFrom);
    multiPart->append(filePart);

    if (!r.url().path().endsWith("files"))
        return m_ep.patchData(r, multiPart);
    return m_ep.postData(r, multiPart);
}

/**
    requester
*/
ApiEndpoint::requester::requester(ApiEndpoint& e) :m_ep(e) 
{

};

QNetworkReply * ApiEndpoint::requester::makeRequest(QNetworkRequest& r) 
{
    addAuthHeader(r);
    return request(r);
};

void ApiEndpoint::requester::addAuthHeader(QNetworkRequest& request)
{
#define DEF_USER_AGENT "googleQtC++11-Client"

    QString bearer = QString("Bearer %1").arg(m_ep.apiClient()->getToken());
    request.setRawHeader("Authorization", bearer.toUtf8());
    const char* h = getHostHeader();
    if (h && h[0]) {
        request.setRawHeader("Host", h);
    }
    QString user_agent = !m_ep.apiClient()->userAgent().isEmpty() ? m_ep.apiClient()->userAgent() : DEF_USER_AGENT;
    request.setRawHeader("User-Agent", user_agent.toUtf8());

#undef DEF_USER_AGENT
};

const char* ApiEndpoint::requester::getHostHeader()const 
{
    return "www.googleapis.com";
};


/**
    contact_requester
*/
ApiEndpoint::contact_requester::contact_requester(ApiEndpoint& e) :requester(e)
{

};

const char* ApiEndpoint::contact_requester::getHostHeader()const
{
    return nullptr;
};

