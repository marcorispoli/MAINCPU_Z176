#ifndef PROTOTOCONSOLE_H
#define PROTOTOCONSOLE_H

#include "application.h"


class protoToConsole : public QObject
{
    Q_OBJECT
public:
    explicit protoToConsole(QObject *parent = 0);
    void activateConnections(void);
    typedef enum
    {
        _RAGGI_OK=0,
        _RAGGI_PARZIALI,
        _NO_RAGGI
    }_fineRaggiCode;

    // codici dedicati alla macchina Analogica
    #define NOTIFY_SET_READY                "SetReady"
    #define NOTIFY_FINERAGGI_CALIB_DETECTOR "SetFineRaggiCalibDetector"
    #define NOTIFY_ANALOG_FINERAGGI_CALIB_TUBE     "SetAnalogFineRaggiCalibTube"
    #define NOTIFY_ANALOG_FINERAGGI_CALIB_PROFILE     "SetAnalogFineRaggiCalibProfile"

    // Codici stringa per i messaggi di notifica asincroni verso Console
    #define NOTIFY_SEND_PUSH_CMD               "SetPush"
    #define NOTIFY_SEND_FINE_RAGGI             "SetFineRaggi"
    #define NOTIFY_SEND_FINE_RAGGI_CALIB_KV    "SetFineRaggiCalibKv"
    #define NOTIFY_SEND_FINE_RAGGI_CALIB_IA    "SetFineRaggiCalibIa"
    #define NOTIFY_SEND_FINE_SYS_UPDATE        "SetUpdateStatus"
    #define NOTIFY_SEND_TUBE_MOVING            "SetTubeMoving"          // Invio notifica  movimento braccio
    #define NOTIFY_SEND_SET_PUSH_CMD           "SetSamples"
    #define NOTIFY_SET_CONFIG_CHANGED          "SetConfigChanged"
    #define NOTIFY_SEND_SELECTED_PROJECTION    "SelProjection"          // Notifica l'AWS con l'avvenuta selezione
    #define NOTIFY_SEND_ABORT_PROJECTION       "AbortProjection"        // Notifica l'AWS con l'avvenuta cancellazione della selezione

    #define NOTIFY_REQUEST_POWER_OFF            "RequestPowerOff"
    #define NOTIFY_SET_BIOPSY_POSITION          "SetBiopsyPosition"
    #define NOTIFY_TUBE_TEMPERATURE             "SetTubeTemp"           // Invia il valore della temperatura della cuffia e dell'Anodo

typedef enum
{
    ACCESSORIO=1,
    NO_CONFIG
}_configCode;

signals:
    // Socket per connessione con Console
//    void socketTx(QByteArray data);
    //void logMsgSgn(QString msg);
    void notificheTxHandler(QByteArray); // Segnale per invio su socket
    void errorTxHandler(QByteArray);     // Invio su socket errori
    void logTxHandler(QByteArray);       // Invio su socket log

public:

    void notifyReadyForExposure(bool);          // Notific dello stato di ready
    void enableXrayPush(bool enable);           // Richiesta di abilitazione pulsante raggi
    void notifyMovingArm(void);                 // Notifica di movimento braccio
    void notifyProjectionSelection(QString projection); // Notifica la AWS dell'avvenuta selezione
    void notifyAbortProjection(void); // Notifica la AWS dell'avvenuta selezione
    void notifyRequestPowerOff(void);
    void notifyTubeTemp(int anodeT, int tubeT);

    void setBiopsyPosition(int curX, int curY, int curZ);

    void endCommandAck(unsigned char id, unsigned char code); // Acnowledge per fine comando

    bool localDebugEnable;
    bool logConnected;   // Stato della connessione
    bool notificheConnected;                     // Stato della connessione
    bool errorConnected;

public slots:
    void sendNotificheTcp(QByteArray frame);  // Slot per invio da fuori messaggi di notifica
    void setSamples(QByteArray data);

    // Socket per connessione con console
    void notificheConnectionHandler(bool status);       // Gestore connessioni
    void logConnectionHandler(bool status);             // Gestore connessioni per messaggi di Log
    void errConnectionHandler(bool status);             // Gestore connessioni per messaggi di Log

    void activationXrayPush(void);

    void alarmNotify(int codice, QString msg); // Invio notifica Errore a COnsole
    void logMessages(QString msg); // Invio messaggio di Log

    void fineRaggiCalibKv(QByteArray data); // Slot di ricezione dati da sequenza raggi di calibrazione KV
    void fineRaggiCalibIa(QByteArray data); // Slot di ricezione dati da sequenza raggi di calibrazione KV


    void fineRaggiAnalogCalibDetector(QByteArray data); // Fine raggi calibrazione esposimetro macchine analogiche
    void fineRaggiAnalogCalibTube(unsigned char result, unsigned char rawkV, int dkV, int dIa);
    void fineRaggiAnalogCalibProfile(QByteArray data);

    // Slot di ricezione fine aggiornamento
    void fineSystemUpdate(bool ris, QString errstr);
    void systemUpdateStatus(int perc, QString filename);


private:
    TcpIpClient*        notificheTcp;      // Socket Client per invio notifiche asincrone
    unsigned int id;                    // ID dei messaggi


    TcpIpClient*        msgErrorTcp;    // Socket Client per invio notifiche messaggi di errore
    TcpIpClient*        msgLogTcp;      // Socket Client per invio messaggi di Log
    bool xrayEnable;


};

#endif // PROTOTOCONSOLE_H
