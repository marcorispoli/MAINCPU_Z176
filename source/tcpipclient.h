#ifndef TCPIPCLIENT_H
#define TCPIPCLIENT_H


#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QAbstractSocket>
#include <QMutex>
#include <QWaitCondition>

class TcpIpClient: public QTcpServer
{
    Q_OBJECT

public:
    explicit TcpIpClient();
    ~TcpIpClient();


    int Start(QHostAddress remip, int remport); // Attivazione delle connessioni

    QHostAddress peerAddress() {return serverip;}
    int peerPort(){return serverport;}
    QHostAddress localAddress() {return localip;}
    static QHostAddress hostAddress();

signals:
    void clientConnection(bool status); // Segnali disponibili per moduli esterni
    void rxData(QByteArray data);

public slots:
    void txData(QByteArray data);
    void txData1000(QByteArray data);
    void txData(QByteArray data,long timeout);


private slots:
    void socketRxData(); // Ricezione dati da socket
    void socketError(QAbstractSocket::SocketError error); // Errore dal socket
    void socketConnected(); // Segnale di connessione avvenuta con il server
    void socketDisconnected(); // IL server ha chiiuso la connessione

public:
    bool connectionStatus;
    bool connectionAttempt; // E' in corso un tentativo di connessione

private:
    QHostAddress serverip;      // Addrees of the remote server
    quint16      serverport;    // Port of the remote server
    QHostAddress localip;      // Addrees of the remote server
    QTcpSocket*  socket;

    void clientConnect();       // Try to connect the remote server

};

Q_DECLARE_METATYPE(QAbstractSocket::SocketState)
Q_DECLARE_METATYPE(QAbstractSocket::SocketError)


#endif // TCPIPCLIENT_H
