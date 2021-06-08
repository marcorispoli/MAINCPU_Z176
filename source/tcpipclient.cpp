#include "tcpipclient.h"

TcpIpClient::TcpIpClient():QTcpServer()
{
    connectionStatus=false;
    socket=0;
}
TcpIpClient::~TcpIpClient()
{
    if(socket)
    {
        socket->close();
        socket->deleteLater();
        socket=0;
        connectionStatus=FALSE;
    }
}

// FUNZIONE DI ATTIVAZIONE DELLA COMUNICAZIONE
int TcpIpClient::Start(QHostAddress remip, int remport)
{
    // Impostazione server remoto
    serverip = remip;
    serverport = remport;

    // Salva l'indirizzo locale
    localip = hostAddress();

    // Crea il socket
    socket = new QTcpSocket(this);
    connect(socket,SIGNAL(connected()),this,SLOT(socketConnected()),Qt::UniqueConnection);
    connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(socketError(QAbstractSocket::SocketError)),Qt::UniqueConnection);
    connect(socket,SIGNAL(readyRead()), this,SLOT(socketRxData()),Qt::UniqueConnection);
    connect(socket,SIGNAL(disconnected()),this,SLOT(socketDisconnected()),Qt::UniqueConnection);

    socket->connectToHost(serverip, serverport);
    return 0;
}


void TcpIpClient::socketConnected()
{
    // Connessione avvenuta
    connectionStatus=true;
    emit clientConnection(true);
    socket->setSocketOption(QAbstractSocket::LowDelayOption,1);


}

/////////////////////////////////////////////////////////////////
/*
    Questo segnale viene generato solamente quando il server
    remoto chiude la connessione. Il Socket a questo punto ha
    gi�  chiuso la connessione in corso e sar�  possibile effettuare
    un nuovo tentativo di riconnessione.

 Autore: M. Rispoli
 Data: 14/11/2014
*/
void TcpIpClient::socketDisconnected()
{
    socket->connectToHost(serverip, serverport);
}

/////////////////////////////////////////////////////////////////
/*
 Se l'errore è QAbstractSocket::RemoteHostClosedError
 allora bisogna aspettare il segnale di disconnessione prima
 di provare la riconnessione perchè il socket non ha ancora
 chiuso la connessione in corso ed in tal caso la riconnessione
 non funzionerebbe e nessun segnale di errore sarebbe generato.
 In tutti gli altri casi invece occorre chiudere il socket e riprovare
 la connessione

 Autore: M. Rispoli
 Data: 14/11/2014
*/
void TcpIpClient::socketError(QAbstractSocket::SocketError error)
{
    // Invia la comunicazione tempestiva che la comunicazione è interrotta
    if(connectionStatus==TRUE)
    {
        connectionStatus=FALSE;
        emit clientConnection(false);
    }

    if(error == QAbstractSocket::RemoteHostClosedError)
    {
        // Quest'errore viene generato PRIMA della chiusura
        // del socket (slot socketDisconnected())
        return ;
    }

    // Tentativo di riconnessione
    socket->abort();
    socket->close();
    socket->connectToHost(serverip, serverport);
    return;
}

// RICEZIONE DATI
void TcpIpClient::socketRxData()
{
    if(connectionStatus ==FALSE) return;
    if(socket->bytesAvailable()==0)
    {
        return;
    }
    QByteArray data = socket->readAll();
    if(data.size()==0)
    {
        return;
    }

    // Legge tutto il buffer ricevuto e lo passa a chi deve gestirlo
    emit rxData(data);//(socket->bytesAvailable()));

}

void TcpIpClient::txData(QByteArray data)
{
    // Invia i dati ed attende di ricevere la risposta
    if(!socket) return;
    if(!connectionStatus) return;

    socket->write(data);
    socket->waitForBytesWritten(5000);

}
void TcpIpClient::txData1000(QByteArray data)
{
    // Invia i dati ed attende di ricevere la risposta
    if(!socket) return;
    if(!connectionStatus) return;

    socket->write(data);
    socket->waitForBytesWritten(5000);

}
// timeout in msec di attesa trasmissione
void TcpIpClient::txData(QByteArray data,long timeout)
{
    // Invia i dati ed attende di ricevere la risposta
    if(!socket) return;
    if(!connectionStatus) return;

    socket->write(data);
    socket->waitForBytesWritten(timeout);
}

// Determina l'indirizzo della macchina
QHostAddress TcpIpClient::hostAddress()
{
    QNetworkInterface network;

    bool valid_interface;

    valid_interface = false;
    QList<QNetworkInterface> interfacesList = QNetworkInterface::allInterfaces();
    for(int i =0; i<interfacesList.size(); i++)
    {
        if (!interfacesList.at(i).name().contains("eth")) continue;
        if (!interfacesList.at(i).IsUp) continue;
        network = interfacesList.at(i);
        valid_interface = true;
    }

    if (valid_interface == false )
    {
        return QHostAddress("127.0.0.1");
    }

    QList<QHostAddress> ipAddressList = network.allAddresses();
    for(int i =0; i<ipAddressList.size(); i++)
    {
        // Scandisce la lista per trovare gli indirizzi ip validi
        if((ipAddressList.at(i) != QHostAddress::LocalHost) && // non è un local host
                ipAddressList.at(i).toIPv4Address())            // E' un indirizzo IPV4
        {
            return ipAddressList.at(i);
        }
    }

    return QHostAddress("127.0.0.1");

}

