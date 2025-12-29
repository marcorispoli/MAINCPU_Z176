#ifndef PRINT_H
#define PRINT_H

#include "application.h"

class mccPrintCom: public mccCom
{
public:
    mccPrintCom(int x, int y, int z) : mccCom(x,y,z) {}
    void mccRxHandler(_MccFrame_Str mccframe);

};

class infoClass : public QObject
{
    Q_OBJECT
public:
    explicit infoClass(QObject *parent = 0);

signals:

    void debugTxHandler(QString);   // Info di tipo Debug
    void logTxHandler(QString);     // Info di tipo Log
    void qtTxHandler(QString);      // Info dalla libreria Qt
    void m4TxHandler(QString);      // Info dal driver M4

public:
    bool connected;
    void activateConnections(void);
    void resetTimestamp(void);


#define PRINT(x)    pInfo->emit_debugTxHandler(x)
#define DEBUG(x)    pInfo->emit_debugTxHandler(x) // Messaggi visualizzati esclusivamente su socket
#define LOG(x)      pInfo->emit_logTxHandler(x)   // Messaggi salvati anche su file

#define QTMSG(x)    pInfo->emit_qtTxHandler(x)   // Messaggi inviati a console
#define M4MSG(x)    pInfo->emit_m4TxHandler(x)   // Messaggi inviati a console

public slots:

    void emit_debugTxHandler(QString msg) {emit debugTxHandler(msg); }
    void emit_logTxHandler(QString msg)   {emit logTxHandler(msg); }
    void emit_qtTxHandler(QString msg)    {emit qtTxHandler(msg); }
    void emit_m4TxHandler(QString msg)    {emit m4TxHandler(msg); }

    // Ricezione connessione ethernet
    void serviceRxHandler(QByteArray data); // Handler ricezione da ethernet
    void notificheMasterConnectionHandler(bool stat);    // Handler cambio stato connessione
    void notificheSlaveConnectionHandler(bool stat);    // Handler cambio stato connessione


    // Handler di debug
    void serviceRxConsoleHandler(QByteArray data);    // Handler ricezione dati da AWS
    void serviceTxConsoleHandler(QByteArray data);    // Handler invio dati ad AWS
    void serviceTxAsyncHandler(QByteArray data);      // Handler invio dati verso canale async di AWS
    void serviceErrorTxHandler(int codice, QString msg);// Handler dei messaggi inviati sul canale errori di AWS
    void serviceLogHandler(QString data);
    void serviceDebugHandler(QString data);

    void serviceQtHandler(QString data);
    void serviceM4Handler(QString data);


private:
    TcpIpServer*     serviceTcp; // Connessione da monitor esterno

};


#endif // PRINT_H
