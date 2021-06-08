#include "tcpipserver.h"
#include "tcpipclient.h"
#include "echodisplay.h"
#include <QWSServer>
#include <QObject>
#include "resource.h"
#include "mainpage.h"
#include "pagelanguages.h"
#include "pageacr.h"
#include "pagealarms.h"
#include "lib/dbase.h"

ProtocolloMET	 seriale("/dev/ttySP0",0xA); // "/dev/ttyUSB0"
TcpIpServer*     externTcp;
TcpIpServer*     masterTcp;
TcpIpClient*     slaveTcp;
EchoDisplay      echoDisplay;  // Sviluppa il protocollo di mirroring tra i display
MainPage*        pagina0;
PageLanguages*   pagina_language;
PageACR*         paginaAcr;
PageAlarms*      paginaAllarmi;
DBase            ApplicationDatabase;
bool isMaster;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    #ifdef Q_WS_QWS
        QWSServer::setCursorVisible(false);
    #endif

    qRegisterMetaType<QAbstractSocket::SocketState>();
    qRegisterMetaType<QAbstractSocket::SocketError>();


    // Costruzione DATABASE CAMPI VARIABILI:ApplicationDatabase
    ApplicationDatabase.append("Collimazione");     // CAMPO COLLIMAZIONE: STRING
    ApplicationDatabase.append("Ingrandimento");    // CAMPO INGRANDIMENTO: STRING
    ApplicationDatabase.append((unsigned char) 0);  // CAMPO FORZA: UNSIGNED CHAR
    ApplicationDatabase.append((int)0);             // CAMPO SPESSORE: INTEGER
    ApplicationDatabase.append((int) 0);            // CAMPO ANGOLO: INTEGER
    ApplicationDatabase.append("Giovanna Vinci");   // CAMPO INTESTAZIONE: STRING
    ApplicationDatabase.append((unsigned char) 0);  // CAMPO VALORI ACR VIEW
    ApplicationDatabase.append((unsigned char) 0);  // CAMPO VALORI ACR SUFFIX
    ApplicationDatabase.append((unsigned char) 0);  // CAMPO CPU_FLAGS
    ApplicationDatabase.append((unsigned char) 0);  // CAMPO MOVIMENTI
    ApplicationDatabase.append((int) 0);            // CAMPO ALLARMI

    // Connessione database al modulo echoDisplay
    QObject::connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), &echoDisplay,SLOT(dbChanged(int,int)));

    // Connessione Windows page al modulo echoDisplay
    QObject::connect(&GWindowRoot,SIGNAL(changePage(int,int)), &echoDisplay,SLOT(pageChanged(int,int)));


    // Connessione al protocollo Ethernet
    // Attivazione protocollo seriale
    QHostAddress host = TcpIpServer::hostAddress();
    qDebug() << "HOST:" << host.toString();

    // Angolo di rotazione iniziale della vista relativa al MASTER
    int rotView = -90;

    // PROVVISORIO
    if (host==QHostAddress("192.168.1.12"))
    {
        // MASTER
        qDebug() << " MASTER OF THE COMMUNICATION STARTED ";
        isMaster=true;

        // IMPLEMENTARE IL SERVER PER SERVIZI ESTERNI
        masterTcp = new TcpIpServer();
        if(masterTcp->Start(10001)<0)
            return -1;
        QObject::connect(masterTcp,SIGNAL(rxData(QByteArray)),&echoDisplay,SLOT(rxData(QByteArray)));
        QObject::connect(masterTcp,SIGNAL(serverConnection(bool)),&echoDisplay,SLOT(connection(bool)));
        QObject::connect(&echoDisplay,SIGNAL(txData(QByteArray)), masterTcp,SLOT(txData(QByteArray)));

    }else
    {
        // SLAVE
        qDebug() << " SLAVE OF THE COMMUNICATION STARTED ";
        isMaster=false;
        slaveTcp = new TcpIpClient();
        QObject::connect(slaveTcp,SIGNAL(rxData(QByteArray)),&echoDisplay,SLOT(rxData(QByteArray)));
        QObject::connect(&echoDisplay,SIGNAL(txData(QByteArray)),slaveTcp,SLOT(txData(QByteArray)));
        QObject::connect(slaveTcp,SIGNAL(clientConnection(bool)),&echoDisplay,SLOT(connection(bool)));
        slaveTcp->Start(QHostAddress("192.168.1.12"),10001);
        rotView = +90;
    }

    ////////////////////////////////// PAGINA LANGUAGE //////////////////////////////////
    pagina_language = new PageLanguages( &a, PageLanguages::_LNG_ITA, true,_BACKGROUND_Y_PG_SELLNG,_BACKGROUND_C_PG_SELLNG,640,480,rotView,pagina0->setPointPath(8,390,0,480,0,480,90,390,90),(int)_PG_SELLNG,pagina0->setPointPath(8,0,0,90,0,90,90,0,90),(int)_PG_MAIN,(int)_PG_SELLNG);
    QObject::connect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)), pagina_language,SLOT(buttonActivationNotify(int,bool,int)));
    QObject::connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), pagina_language,SLOT(valueChanged(int,int)));


    ////////////////////////////////// PAGINA MAIN //////////////////////////////////
    pagina0 = new MainPage(true,QString(_BACKGROUND_Y_PG_MAIN),QString(_BACKGROUND_C_PG_MAIN),640,480,rotView,pagina0->setPointPath(8,390,0,480,0,480,90,390,90),(int)_PG_ACR,pagina0->setPointPath(8,0,0,90,0,90,90,0,90),(int)_PG_SELLNG,(int)_PG_MAIN);
    QObject::connect(pagina_language,SIGNAL(changeLanguageSgn()), pagina0,SLOT(languageChanged()));
    QObject::connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), pagina0,SLOT(valueChanged(int,int)));
    QObject::connect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)), pagina0,SLOT(buttonActivationNotify(int,bool,int)));
    QObject::connect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)), &echoDisplay,SLOT(buttonChanged(int,bool,int)));
    GPush::pushRefresh(DBase::_DB_NO_ECHO|DBase::_DB_NO_ACTION);

    ////////////////////////////////// PAGINA ACR RIGHT //////////////////////////////////
    paginaAcr= new PageACR(true,"","",640,480,rotView,pagina0->setPointPath(8,390,0,480,0,480,90,390,90),(int)_PG_MAIN,pagina0->setPointPath(8,0,0,90,0,90,90,0,90),(int) _PG_MAIN,(int)_PG_ACR);
    QObject::connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), paginaAcr,SLOT(valueChanged(int,int)));
    QObject::connect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)), paginaAcr,SLOT(buttonActivationNotify(int,bool,int)));


    ////////////////////////////////// PAGINA ALLARMI //////////////////////////////////
    paginaAllarmi = new PageAlarms(QString(_PG_ALARM_BACKGROUND),640,480,rotView,paginaAllarmi->setPointPath(8,390,0,480,0,480,90,390,90),(int)_PG_MAIN,pagina0->setPointPath(8,0,0,90,0,90,90,0,90),(int)_PG_MAIN,(int)_PG_ALARM);
    QObject::connect(pagina_language,SIGNAL(changeLanguageSgn()), paginaAllarmi,SLOT(languageChanged()));
    QObject::connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), paginaAllarmi,SLOT(valueChanged(int,int)));
//    QObject::connect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)), paginaAllarmi,SLOT(buttonActivationNotify(int,bool,int)));


    ///////////////////////////
    return a.exec();
    ////////////////////////////


}
