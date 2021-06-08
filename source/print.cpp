#include "appinclude.h"
#include "globvar.h"

// Comandi da inviare a Console
#define MASTER_PRINT_PORT 10020
#define SLAVE_PRINT_PORT  10021



void printDebug::activateConnections(void){
    QObject::connect(printTcp,SIGNAL(clientConnection(bool)),this,SLOT(printConnectionHandler(bool)),Qt::UniqueConnection);
    QObject::connect(this,SIGNAL(printTxHandler(QByteArray)), printTcp,SLOT(txData(QByteArray)),Qt::UniqueConnection);

#ifdef __PRINT
    if(isMaster)
        printTcp->Start(QHostAddress(__PRINT),MASTER_PRINT_PORT);
    else
        printTcp->Start(QHostAddress(__PRINT),SLAVE_PRINT_PORT);
#endif
}

printDebug::printDebug(QObject *parent) :
    QObject(0)
{
    printConnected = FALSE;
    coda.clear();
    // Socket per segnali asincroni
    printTcp = new TcpIpClient();
    activateConnections();

}



void printDebug::printConnectionHandler(bool stat)
{
    printConnected = stat;

    if(stat==true) print(QString("HELLO\n\r"));
    return;
}


/*
 *  Questa funzione viene lanciata per notificare la AWS di un comando di movimento braccio in corso
 */
void printDebug::print(QString stringa)
{
    static int idx=0;
    if(!printConnected){
        coda.append(stringa);
        return;
    }
    if(coda.size()){
        stringa.prepend(coda);
        coda.clear();
    }

    stringa.prepend(QString("[%1]:>").arg(idx++));
    stringa.append("\n\r");
    emit printTxHandler(stringa.toAscii());

}



