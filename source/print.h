#ifndef PRINTDEBUG_H
#define PRINTDEBUG_H

#include "application.h"





class printDebug : public QObject
{
    Q_OBJECT
public:
    explicit printDebug(QObject *parent = 0);

signals:

    void printTxHandler(QByteArray);       // Invio su socket log

public:

    bool printConnected;   // Stato della connessione
    void activateConnections(void);
public slots:
    void print(QString frame);  // Slot per invio da fuori messaggi di notifica

    // Socket per connessione con console
    void printConnectionHandler(bool status);

private:
    TcpIpClient*        printTcp;      // Socket Client per invio notifiche asincrone
    QString coda;
};

#ifdef __PRINT
    #ifdef MAIN_C
        printDebug* pPrint;
    #else
        extern printDebug* pPrint;
    #endif

    #define PRINT(x) pPrint->print(x)
#else
    #define PRINT(x) ;
#endif


#endif // PRINTDEBUG_H
