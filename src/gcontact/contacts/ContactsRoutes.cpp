/**********************************************************
 DO NOT EDIT
 This file was generated from stone specification "contacts"
 www.prokarpaty.net
***********************************************************/

#include "gcontact/contacts/ContactsRoutes.h"
#include "Endpoint.h"
#include "gcontact/GcontactRoutes.h"
using namespace googleQt;
using namespace contacts;

std::unique_ptr<gcontact::ContactsListResult> ContactsRoutes::batch(const gcontact::BatchContactArg& arg){
    return batch_Async(arg)->waitForResultAndRelease();
}

GoogleTask<gcontact::ContactsListResult>* ContactsRoutes::batch_Async(const gcontact::BatchContactArg& arg)
{
    GoogleTask<gcontact::ContactsListResult>* t = m_end_point->produceTask<gcontact::ContactsListResult>();
    m_end_point->postContactStyleB<
        gcontact::ContactsListResult,
        gcontact::ContactsListResult::factory
        ,gcontact::BatchContactArg>
        (m_end_point->buildContactBatchUrl(arg),
        arg,
        t);
    return t;
}

void ContactsRoutes::batch_AsyncCB(
    const gcontact::BatchContactArg& arg,
    std::function<void(std::unique_ptr<gcontact::ContactsListResult>)> completed_callback ,
    std::function<void(std::unique_ptr<GoogleException>)> failed_callback)
{
    m_end_point->postContactStyleB
        <
        gcontact::ContactsListResult,
        gcontact::ContactsListResult::factory
        , gcontact::BatchContactArg
        >
        (m_end_point->buildContactBatchUrl(arg),
        arg,
        completed_callback,
        failed_callback);
}

std::unique_ptr<gcontact::ContactsListResult> ContactsRoutes::create(const gcontact::CreateContactArg& arg){
    return create_Async(arg)->waitForResultAndRelease();
}

GoogleTask<gcontact::ContactsListResult>* ContactsRoutes::create_Async(const gcontact::CreateContactArg& arg)
{
    GoogleTask<gcontact::ContactsListResult>* t = m_end_point->produceTask<gcontact::ContactsListResult>();
    m_end_point->postContactStyleB<
        gcontact::ContactsListResult,
        gcontact::ContactsListResult::factory
        ,gcontact::CreateContactArg>
        (m_end_point->buildContactUrl(arg),
        arg,
        t);
    return t;
}

void ContactsRoutes::create_AsyncCB(
    const gcontact::CreateContactArg& arg,
    std::function<void(std::unique_ptr<gcontact::ContactsListResult>)> completed_callback ,
    std::function<void(std::unique_ptr<GoogleException>)> failed_callback)
{
    m_end_point->postContactStyleB
        <
        gcontact::ContactsListResult,
        gcontact::ContactsListResult::factory
        , gcontact::CreateContactArg
        >
        (m_end_point->buildContactUrl(arg),
        arg,
        completed_callback,
        failed_callback);
}

void ContactsRoutes::deleteContact(const gcontact::DeleteContactArg& arg ){
    deleteContact_Async(arg)->waitForResultAndRelease();
}

GoogleVoidTask* ContactsRoutes::deleteContact_Async(const gcontact::DeleteContactArg& arg)
{
    GoogleVoidTask* t = m_end_point->produceVoidTask();
    m_end_point->deleteContactStyleB
        (m_end_point->buildContactUrl(arg),
        arg,
        t);
    return t;
}

void ContactsRoutes::deleteContact_AsyncCB(
    const gcontact::DeleteContactArg& arg,
    std::function<void()> completed_callback ,
    std::function<void(std::unique_ptr<GoogleException>)> failed_callback)
{
    m_end_point->deleteContactStyleB
        <
        gcontact::DeleteContactArg
        >
        (m_end_point->buildContactUrl(arg),
        arg,
        completed_callback,
        failed_callback);
}

void ContactsRoutes::deleteContactPhoto(const gcontact::DeletePhotoArg& arg ){
    deleteContactPhoto_Async(arg)->waitForResultAndRelease();
}

GoogleVoidTask* ContactsRoutes::deleteContactPhoto_Async(const gcontact::DeletePhotoArg& arg)
{
    GoogleVoidTask* t = m_end_point->produceVoidTask();
    m_end_point->deleteContactPhotoStyleB
        (m_end_point->buildContactPhotoUrl(arg),
        arg,
        t);
    return t;
}

void ContactsRoutes::deleteContactPhoto_AsyncCB(
    const gcontact::DeletePhotoArg& arg,
    std::function<void()> completed_callback ,
    std::function<void(std::unique_ptr<GoogleException>)> failed_callback)
{
    m_end_point->deleteContactPhotoStyleB
        <
        gcontact::DeletePhotoArg
        >
        (m_end_point->buildContactPhotoUrl(arg),
        arg,
        completed_callback,
        failed_callback);
}

void ContactsRoutes::getContactPhoto(const gcontact::DownloadPhotoArg& arg , QIODevice* data){
    getContactPhoto_Async(arg, data)->waitForResultAndRelease();
}

GoogleVoidTask* ContactsRoutes::getContactPhoto_Async(const gcontact::DownloadPhotoArg& arg, QIODevice* data)
{
    GoogleVoidTask* t = m_end_point->produceVoidTask();
    m_end_point->downloadContactPhotoStyle
        (m_end_point->buildContactPhotoUrl(arg),
        data,
        t);
    return t;
}

void ContactsRoutes::getContactPhoto_AsyncCB(
    const gcontact::DownloadPhotoArg& arg,
    QIODevice* data,
    std::function<void()> completed_callback ,
    std::function<void(std::unique_ptr<GoogleException>)> failed_callback)
{
    m_end_point->downloadContactPhotoStyle
        (m_end_point->buildContactPhotoUrl(arg),
        data,
        completed_callback,
        failed_callback);
}

std::unique_ptr<gcontact::ContactsListResult> ContactsRoutes::list(const gcontact::ContactsListArg& arg){
    return list_Async(arg)->waitForResultAndRelease();
}

GoogleTask<gcontact::ContactsListResult>* ContactsRoutes::list_Async(const gcontact::ContactsListArg& arg)
{
    GoogleTask<gcontact::ContactsListResult>* t = m_end_point->produceTask<gcontact::ContactsListResult>();
    m_end_point->getContactStyle<
        gcontact::ContactsListResult,
        gcontact::ContactsListResult::factory
        >
        (m_end_point->buildContactUrl(arg),
        t);
    return t;
}

void ContactsRoutes::list_AsyncCB(
    const gcontact::ContactsListArg& arg,
    std::function<void(std::unique_ptr<gcontact::ContactsListResult>)> completed_callback ,
    std::function<void(std::unique_ptr<GoogleException>)> failed_callback)
{
    m_end_point->getContactStyle
        <
        gcontact::ContactsListResult,
        gcontact::ContactsListResult::factory
        >
        (m_end_point->buildContactUrl(arg),
        completed_callback,
        failed_callback);
}

std::unique_ptr<gcontact::ContactsListResult> ContactsRoutes::update(const gcontact::UpdateContactArg& arg){
    return update_Async(arg)->waitForResultAndRelease();
}

GoogleTask<gcontact::ContactsListResult>* ContactsRoutes::update_Async(const gcontact::UpdateContactArg& arg)
{
    GoogleTask<gcontact::ContactsListResult>* t = m_end_point->produceTask<gcontact::ContactsListResult>();
    m_end_point->putContactStyleB<
        gcontact::ContactsListResult,
        gcontact::ContactsListResult::factory
        ,gcontact::UpdateContactArg>
        (m_end_point->buildContactUrl(arg),
        arg,
        t);
    return t;
}

void ContactsRoutes::update_AsyncCB(
    const gcontact::UpdateContactArg& arg,
    std::function<void(std::unique_ptr<gcontact::ContactsListResult>)> completed_callback ,
    std::function<void(std::unique_ptr<GoogleException>)> failed_callback)
{
    m_end_point->putContactStyleB
        <
        gcontact::ContactsListResult,
        gcontact::ContactsListResult::factory
        , gcontact::UpdateContactArg
        >
        (m_end_point->buildContactUrl(arg),
        arg,
        completed_callback,
        failed_callback);
}

void ContactsRoutes::uploadContactPhoto(const gcontact::UploadPhotoArg& arg , QIODevice* data){
    uploadContactPhoto_Async(arg, data)->waitForResultAndRelease();
}

GoogleVoidTask* ContactsRoutes::uploadContactPhoto_Async(const gcontact::UploadPhotoArg& arg, QIODevice* data)
{
    GoogleVoidTask* t = m_end_point->produceVoidTask();
    m_end_point->uploadContactPhotoStyle
        (m_end_point->buildContactPhotoUrl(arg),
        data,
        t);
    return t;
}

void ContactsRoutes::uploadContactPhoto_AsyncCB(
    const gcontact::UploadPhotoArg& arg,
    QIODevice* data,
    std::function<void()> completed_callback ,
    std::function<void(std::unique_ptr<GoogleException>)> failed_callback)
{
    m_end_point->uploadContactPhotoStyle
        (m_end_point->buildContactPhotoUrl(arg),
        data,
        completed_callback,
        failed_callback);
}

