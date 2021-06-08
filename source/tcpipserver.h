#ifndef TCPIPSERVER_H
#define TCPIPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QAbstractSocket>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>

 // ************************************* TCP SOCKET CLASS **********************
class TcpIpServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit TcpIpServer();
    ~TcpIpServer();

    int Start(int porta); // Attivazione delle connessioni
    int localPort(){return localport;}
    QHostAddress localAddress(){return localip;}
    static QHostAddress hostAddress();


protected:
    void incomingConnection(int socketDescriptor);

signals:
    void serverConnection(bool status);// Segnali disponibili per moduli esterni
    void rxData(QByteArray data); // accoda i messaggi ricevuti dal canale Tcp



public slots:
    void txData(QByteArray data); // accoda i messaggi da trasmettere sul canale Tcp
    void txData1000(QByteArray data); // accoda i messaggi da trasmettere sul canale Tcp
    void txData(QByteArray data, long timeout);
    void socketError(QAbstractSocket::SocketError error);


private slots:
    void socketRxData(); // Funzione agganciata internamente al segnale del sockect
    void disconnected();          // Socket segnala chiusura

public:
    bool connection_status; // Stato della connessione
    QTcpSocket*  socket;        // Socket di comunicazione con il client


private:

    QHostAddress localip;       // Address of the local server
    quint16      localport;     // Port of the local server
};



#endif // TCPIPSERVER_H
