#include "appinclude.h"
#include "globvar.h"


static qint64 orig = QDateTime::currentMSecsSinceEpoch();

// Messaggi provenienti dallelibrerie Qt
void  qtOutput(QtMsgType type, const char *msg);
void  qtOutput(QtMsgType type, const char *msg)
{
    QTMSG(QString(msg));
    return;
}

//


infoClass::infoClass(QObject *parent) :
    QObject(0)
{
    qInstallMsgHandler(qtOutput);
    serviceTcp = new TcpIpServer();
    connected = false;
}

void infoClass::activateConnections(void) {
    //QObject::connect(serviceTcp,SIGNAL(rxData(QByteArray)),this,SLOT(serviceRxHandler(QByteArray)),Qt::UniqueConnection);
    if(isMaster) connect(serviceTcp,SIGNAL(serverConnection(bool)),this,SLOT(notificheMasterConnectionHandler(bool)),Qt::UniqueConnection);
    else connect(serviceTcp,SIGNAL(serverConnection(bool)),this,SLOT(notificheSlaveConnectionHandler(bool)),Qt::UniqueConnection);
    serviceTcp->Start(_PRINT_SERVICE_PORT);

    if(isMaster) pPrintMcc = new mccPrintCom(_DEF_M4_MASTER_DEBUG_MESSAGES_MCC); // Ricezione Print da Master
    else pPrintMcc = new mccPrintCom(_DEF_M4_SLAVE_DEBUG_MESSAGES_MCC); // Ricezione Print da Master

}

void infoClass::serviceRxHandler(QByteArray data)
{
    data.append("\r");
    data.append("\n");
    serviceTcp->txData(data);
}

void infoClass::notificheMasterConnectionHandler(bool stat)
{
    unsigned char data[2];

    if(stat)
    {
        // Connessione automatica al servizio di log eventi
        serviceTcp->txData(QString("---------   MASTER DEBUG INTERFACE -------------\r\n").toAscii());
        connected = true;

        // Connette tutti i sockets
        connect(pConsole,SIGNAL(consoleTxHandler(QByteArray)),this,SLOT(serviceTxConsoleHandler(QByteArray)),Qt::UniqueConnection);
        connect(pConsole,SIGNAL(consoleRxSgn(QByteArray)),this,SLOT(serviceRxConsoleHandler(QByteArray)),Qt::UniqueConnection);
        connect(pToConsole,SIGNAL(notificheTxHandler(QByteArray)),this,SLOT(serviceTxAsyncHandler(QByteArray)),Qt::UniqueConnection);
        connect(paginaAllarmi,SIGNAL(newAlarmSgn(int,QString)),this,SLOT(serviceErrorTxHandler(int,QString)),Qt::UniqueConnection);
        connect(this,SIGNAL(printTxHandler(QString)),this,SLOT(servicePrintHandler(QString)),Qt::UniqueConnection);
        connect(this,SIGNAL(debugTxHandler(QString)),this,SLOT(serviceDebugHandler(QString)),Qt::UniqueConnection);
        connect(this,SIGNAL(logTxHandler(QString)),this,SLOT(serviceLogHandler(QString)),Qt::UniqueConnection);
        connect(this,SIGNAL(qtTxHandler(QString)),this,SLOT(serviceQtHandler(QString)),Qt::UniqueConnection);
        connect(this,SIGNAL(m4TxHandler(QString)),this,SLOT(serviceM4Handler(QString)),Qt::UniqueConnection);


        data[0] = MCC_DEBUG_PRINT_ENABLE_CMD;
        data[1] = 1;
        pConsole->pGuiMcc->sendFrame(MCC_PRINT,1,data,2);
    }else
    {
        disconnect(this);
        connected = false;

        disconnect(pConsole,SIGNAL(consoleTxHandler(QByteArray)),this,SLOT(serviceTxConsoleHandler(QByteArray)));
        disconnect(pConsole,SIGNAL(consoleRxSgn(QByteArray)),this,SLOT(serviceRxConsoleHandler(QByteArray)));
        disconnect(pToConsole,SIGNAL(notificheTxHandler(QByteArray)),this,SLOT(serviceTxAsyncHandler(QByteArray)));
        disconnect(paginaAllarmi,SIGNAL(newAlarmSgn(int,QString)),this,SLOT(serviceErrorTxHandler(int,QString)));
        disconnect(this,SIGNAL(printTxHandler(QString)),this,SLOT(servicePrintHandler(QString)));
        disconnect(this,SIGNAL(debugTxHandler(QString)),this,SLOT(serviceDebugHandler(QString)));
        disconnect(this,SIGNAL(logTxHandler(QString)),this,SLOT(serviceLogHandler(QString)));
        disconnect(this,SIGNAL(qtTxHandler(QString)),this,SLOT(serviceQtHandler(QString)));
        disconnect(this,SIGNAL(m4TxHandler(QString)),this,SLOT(serviceM4Handler(QString)));

        data[0] = MCC_DEBUG_PRINT_ENABLE_CMD;
        data[1] = 0;
        pConsole->pGuiMcc->sendFrame(MCC_PRINT,1,data,2);

    }

    return;
}

void infoClass::notificheSlaveConnectionHandler(bool stat)
{
    unsigned char data[2];

    if(stat)
    {
        // Connessione automatica al servizio di log eventi
        serviceTcp->txData(QString("---------   SLAVE DEBUG INTERFACE -------------\r\n").toAscii());
        connected = true;

        connect(this,SIGNAL(printTxHandler(QString)),this,SLOT(servicePrintHandler(QString)),Qt::UniqueConnection);
        connect(this,SIGNAL(debugTxHandler(QString)),this,SLOT(serviceDebugHandler(QString)),Qt::UniqueConnection);
        connect(this,SIGNAL(logTxHandler(QString)),this,SLOT(serviceLogHandler(QString)),Qt::UniqueConnection);
        connect(this,SIGNAL(qtTxHandler(QString)),this,SLOT(serviceQtHandler(QString)),Qt::UniqueConnection);
        connect(this,SIGNAL(m4TxHandler(QString)),this,SLOT(serviceM4Handler(QString)),Qt::UniqueConnection);


        data[0] = MCC_DEBUG_PRINT_ENABLE_CMD;
        data[1] = 1;
        pConfig->pSlaveMcc->sendFrame(MCC_PRINT,1,data,2);
    }else
    {
        disconnect(this);
        connected = false;

        disconnect(this,SIGNAL(printTxHandler(QString)),this,SLOT(servicePrintHandler(QString)));
        disconnect(this,SIGNAL(debugTxHandler(QString)),this,SLOT(serviceDebugHandler(QString)));
        disconnect(this,SIGNAL(logTxHandler(QString)),this,SLOT(serviceLogHandler(QString)));
        disconnect(this,SIGNAL(qtTxHandler(QString)),this,SLOT(serviceQtHandler(QString)));
        disconnect(this,SIGNAL(m4TxHandler(QString)),this,SLOT(serviceM4Handler(QString)));

        data[0] = MCC_DEBUG_PRINT_ENABLE_CMD;
        data[1] = 0;
        pConfig->pSlaveMcc->sendFrame(MCC_PRINT,1,data,2);

    }

    return;
}

// Handler messaggi spediti verso console: deve essere trasformato in UNICODE
void infoClass::serviceTxConsoleHandler(QByteArray data)
{
    QTextCodec *codec = QTextCodec::codecForName(UNICODE_TYPE);
    QString stringa = codec->toUnicode(data);
    stringa = "AWS<" + stringa + "\r\n";
    serviceTcp->txData(stringa.toAscii());
    return;
}

// Handler messaggi ricevuti da console: deve essere trasformato in UNICODE
void infoClass::serviceRxConsoleHandler(QByteArray data)
{
    QTextCodec *codec = QTextCodec::codecForName(UNICODE_TYPE);
    QString stringa = codec->toUnicode(data);
    float time = (float) (QDateTime::currentMSecsSinceEpoch()-orig)/1000;
    stringa = QString("%1:").arg(QString::number(time,'f',2)) +  "AWS>" + stringa + "\r\n";
    serviceTcp->txData(stringa.toAscii());
    return;
}


// Risposte asincrone verso Console: UNICODE
void infoClass::serviceTxAsyncHandler(QByteArray data)
{

    QTextCodec *codec = QTextCodec::codecForName(UNICODE_TYPE);
    QString stringa = codec->toUnicode(data);
    float time = (float) (QDateTime::currentMSecsSinceEpoch()-orig)/1000;
    stringa = QString("%1:").arg(QString::number(time,'f',2)) +   "ASYNC AWS>" + stringa + "\r\n";

    serviceTcp->txData(stringa.toAscii());
     return;
}


void infoClass::serviceErrorTxHandler(int codice, QString msg)
{
    QString stringa;
    float time = (float) (QDateTime::currentMSecsSinceEpoch()-orig)/1000;
    stringa = QString("%1:").arg(QString::number(time,'f',2)) +   "ERROR AWS>" + QString("%1:").arg(codice) + msg + "\r\n";
    serviceTcp->txData(stringa.toAscii());
    return;
}

void infoClass::serviceLogHandler(QString data)
{
    QString stringa;
    float time = (float) (QDateTime::currentMSecsSinceEpoch()-orig)/1000;
    stringa = QString("%1:").arg(QString::number(time,'f',2)) +  "LOG: "+ data + "\r\n";
    serviceTcp->txData(stringa.toAscii());
    return;
}

void infoClass::serviceDebugHandler(QString data)
{
    QString stringa;
    float time = (float) (QDateTime::currentMSecsSinceEpoch()-orig)/1000;
    stringa = QString("%1:").arg(QString::number(time,'f',2)) +   "DBG: "+ data + "\r\n";
    serviceTcp->txData(stringa.toAscii());
    return;
}

void infoClass::servicePrintHandler(QString data)
{
    QString stringa;
    float time = (float) (QDateTime::currentMSecsSinceEpoch()-orig)/1000;
    stringa = QString("%1:").arg(QString::number(time,'f',2)) +   "PRINT: "+ data + "\r\n";
    serviceTcp->txData(stringa.toAscii());
    return;
}


void infoClass::serviceQtHandler(QString data)
{
    QString stringa;
    float time = (float) (QDateTime::currentMSecsSinceEpoch()-orig)/1000;
    stringa = QString("%1:").arg(QString::number(time,'f',2)) +  "QT: "+ data + "\r\n";
    serviceTcp->txData(stringa.toAscii());
    return;
}

void infoClass::serviceM4Handler(QString data)
{
    QString stringa;
    float time = (float) (QDateTime::currentMSecsSinceEpoch()-orig)/1000;
    stringa = QString("%1:").arg(QString::number(time,'f',2)) +  "M4: "+ data + "\r\n";
    serviceTcp->txData(stringa.toAscii());
    return;
}


void infoClass::resetTimestamp(void){
    orig = QDateTime::currentMSecsSinceEpoch();
}

void mccPrintCom::mccRxHandler(_MccFrame_Str mccframe)
{
    M4MSG(QString((const char*)mccframe.buffer));
}
