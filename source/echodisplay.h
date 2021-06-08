#ifndef ECHODISPLAY_H
#define ECHODISPLAY_H

#include "application.h"

class EchoDisplay: public QObject
{
    Q_OBJECT

public:
    EchoDisplay();

signals:
    void txData(QByteArray data);

public slots:
    void rxData(QByteArray data);                       // Stream ricevuto dal canale Tcp
    //void echoAll();                                     // Slot per operazioni di mirroring bottoni
    void setNewPageEcho(int index,int pagePrev,int opt);                // Slot per comunicazione cambio pagina
    void dbChanged(int index,int opt);                  // Slot per comunicazione cambio singolo valore
    void buttonChanged(int id, bool status, int opt);   // Slot per pulsante premuto
    void connection(bool status);                       // Refresh dopo nuova connessione
    bool isConnected(void) {return connectionStatus;}

    void masterDatabaseInitialization(void);
    void slaveDatabaseInitialization(void);

public:
    void  echoDate(QString dd, QString mm, QString yy,QString hh, QString min, QString ss,int opt); // Effettua l'eco della data

private:
    void  setDate(QString dd, QString mm, QString yy,QString hh, QString min, QString ss);

private:
    bool connectionStatus;
};

#endif // ECHODISPLAY_H
