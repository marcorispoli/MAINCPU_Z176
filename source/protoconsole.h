#ifndef PROTOCONSOLE_H
#define PROTOCONSOLE_H

#include "application.h"

class protoConsole
{
public:
    protoConsole(QByteArray* dt);   // Pensata per 8bit ascii / frp
    protoConsole(QString* dt);      // Pensata per UNICODE
    protoConsole(int i,bool unicode);

#define _IMMEDIATO  0
#define _NO_TMO     -1




    QByteArray cmdToQByteArray(QString cmd);        // Crea un frame comando: <id size %cmd [param...]%>
    QByteArray answToQByteArray(QString cmd);       // Uguale a cmdToQByteArray
    QByteArray answToQByteArray(void);              // Crea una risposta tipo <id size %OK [param...]%>
    QByteArray answToQByteArray(int time);          // Crea una risposta tipo <id size %OK Time%>
    QByteArray ackToQByteArray(int code);           // Crea un ack ad un comando eseguito, tipo<A LEN %ID code%>
    QByteArray formatToQByteArray(QString cmd);     // Crea il contenuto del campo dati sostituendo i caratteri speciali <>%
    QByteArray unformatData(QString cmd);   // Trasforma il contenuto di una campo dati ricevuto ripristinando i caratteri originali


    bool isValid;
    int id;
    int len;
    int frame_len; // Lunghezza frame <..> non necessariamente uguale a len (Se frame errato)

    // Gestione protocollo  FTP (solo versione non unicode)
    QByteArray createFtpArray(QByteArray data);
    QByteArray ftp; // Aggiunto array di dati binari per trasferimento file <#CRC#>ddddddddddddd
    unsigned char ftpCrc;
    bool isFtpProtocol;
    bool errorCrc;

    // Gestione parameri
    void addParam(QString par);
    QList<QString> parametri;
    QString comando;

private:

    bool isUnicode;
};

#endif // PROTOCONSOLE_H
