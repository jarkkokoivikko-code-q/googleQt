#include <iostream>
#include <functional>

#include <QtCore/QCoreApplication>
#include <QFile>
#include <QStringList>

#include "google/endpoint/ApiAppInfo.h"
#include "google/endpoint/ApiAuthInfo.h"
#include "google/demo/ApiTerminal.h"
#include "google/demo/ApiListener.h"
#include "GcontactCommands.h"

using namespace googleQt;

int main(int argc, char *argv[]) 
{
    QCoreApplication app(argc, argv);

    if (argc != 4) {
        std::string  s = QString("\nUsage: %1 <app-file> <auth-file>\n"
                                 "Example: %1 ../app.info ../token.info me@gmail.com\n"
                                 "\n"
                                 "<app-info-file>: a JSON file with information about your API app.\n"
                                 "<auth-file>: An \"auth file\" that contains the information necessary to make\n"
                                 "an authorized Google API request.  Generate this file using the \"authorize\"\n"
                                 "example program.\n"
                                 "\n"
                                 " Press ENTER to proceed.").arg(argv[0]).toStdString();

        std::cout << s << std::endl;
        std::cout << std::endl;
        std::cin.ignore();
        return 0;
    }

    QString argAppFile = argv[1];
    QString argAuthFile = argv[2];
    QString argEmail = argv[3];

    std::unique_ptr<ApiAppInfo> appInfo(new ApiAppInfo);
    if(!appInfo->readFromFile(argAppFile)){
        std::cerr << "Error reading <app-info-file>" << std::endl;
        return 0;
    };    
    
    std::unique_ptr<ApiAuthInfo> authInfo(new ApiAuthInfo(argAuthFile));
    if(!authInfo->reload()){
        std::cout << "Error reading <auth-file>" << std::endl;
        std::cout << "Use 'authorize' sample to generate token file." << std::endl;
        std::cin.ignore();
        return 0;        
    }

    authInfo->setEmail(argEmail);
    
    std::cout << std::endl << std::endl;
    std::cout << "Contacts list" << std::endl;
    
    demo::ApiListener lsn;
    GoogleClient c(appInfo.release(), authInfo.release());
    QObject::connect(&c, &GoogleClient::downloadProgress, &lsn, &demo::ApiListener::transferProgress);

    GcontactCommands cmd(c);
    demo::Terminal t("gcontact");
    t.addAction("ls_contacts",     "List Contacts", [&](QString ) {cmd.ls_contacts(); });
    t.start();
    return 0;
};
