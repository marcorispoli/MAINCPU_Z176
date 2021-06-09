#ifndef SERVERDEBUG_H
#define SERVERDEBUG_H

#include "application.h"

#define IRS_MAJ  2
#define IRS_MIN  0
#define IRS_BETA 0

/*___________________________________________________________________________________
                DESCRIZIONE REVISIONI INTERFACCIA IRS

REV 1.1.0
    La funzione di inserimento collimazione è in grado di aggiungere un nuovo PAD

REV 1.1.1
    Aggiunto comando per update software

REV 1.3.0
    Nuovo IRS per la versione di rilascio ID10

REV 1.4.0
    Nuovo IRS per la versione di rilascio ID11

REV 1.5.0
    Aggiunto comando per lettura tubo da generatore:

REV 1.6.0
    Aggiunta gestione delle soglie di riconoscimento PAD


_________________________________________________________________________________
10/10/2018

REV 1.6.1

 - Aggiunto comanto compressore: getTrolley()
 - Aggiunto comando compressore: setCompressorRelease()
___________________________________________________________________________________*/

class serverDebug : public QObject
{
    Q_OBJECT
public:
    explicit serverDebug(void);
    void activateConnections(void);

signals:

public slots:
    QHostAddress setIpAddress( int val);
    bool isIp(int val);

    void configurationSlot(void);
    void serviceRxHandler(QByteArray data); // Handler ricezione frame IRS

    void handleBiopsySimulator(QByteArray data);


    void handleMasterShell(QByteArray data);
    void handleSlaveShell(QByteArray data);
    void slotRemoteShell(QByteArray data);

    void handleShell(QByteArray data);
    void resetGonioNotify(unsigned char id, unsigned char mcccode, QByteArray buffer);
    void getGonioNotify(unsigned char id, unsigned char mcccode, QByteArray buffer);
    void handleGetTrolleyNotify(unsigned char id,unsigned char cmd, QByteArray data);
    void handleSetCompressorRelease(QByteArray data);


    void handleAnalog(QByteArray data);
        void PCB244A_Notify(unsigned char id, unsigned char mcccode, QByteArray buffer);

    void handleGeneratore(QByteArray data);    

    void handleSystem(QByteArray data);
    void handleConfig(QByteArray data);

        void handleSetLanguage(QByteArray data); // Imposta la lingua
        void handleGetRevisions(void);              // Comando di richiesta revisioni correnti
        void handleGetInputs(void); // Stampa lo stato degli inputs di sistema
        void handleGetOutputs(void);// Stampa lo stato degli Outputs di sistema
        void handleSetOutputs(QByteArray data);
        void handleSetDatabaseS(QByteArray data);
        void handleSetDatabaseI(QByteArray data);
        void handleSetDatabaseU(QByteArray data);
        void handleGetAlarmInfo(QByteArray data);

    void handleDebug(QByteArray data);
        void handleSetCompressorNotify(QByteArray data);
        void handleSetActuatorEnableNotify(QByteArray data);

    // Drivers Group
    void handleDrivers(QByteArray data);
        void handleDriverFreeze(bool stat);         // Attivazione Disattivazione drivers
        void handleDriverRead8(QByteArray data);
        void handleDriverRead16(QByteArray data);
        void handleDriverWrite8(QByteArray data);
        void handleDriverWrite16(QByteArray data);
        void handleDriverCommand(QByteArray data);
        void handleDriverSpecial(QByteArray data);
        void handleDriverFreezeNotify(unsigned char,unsigned char,QByteArray);
        void handleDriverSendNotify(unsigned char id,unsigned char cmd, QByteArray data);

    void serviceRxConsoleHandler(QByteArray data);    // Handler ricezione dati da AWS
    void serviceTxConsoleHandler(QByteArray data);    // Handler invio dati ad AWS
    void serviceTxAsyncHandler(QByteArray data);      // Handler invio dati verso canale async di AWS
    void serviceErrorTxHandler(int codice, QString msg);// Handler dei messaggi inviati sul canale errori di AWS

    void notificheConnectionHandler(bool stat);    // Handler cambio stato connessione

    // Ricezione dati richiesti

    void serviceLogHandler(QByteArray data);
    void handleReadConfigNotify(_picConfigStr data);
    void serviceNotifyFineRaggi(QByteArray data);

    QByteArray getNextFieldAfterTag(QByteArray buf, QString tag);
    QList<QByteArray> getNextFieldsAfterTag(QByteArray data, QString tag);

    void handleCollimatore(QByteArray data);
        void handleGetCalib(QByteArray data);
        void handleSetCalibFiltro(QByteArray data);
        void handleSetCalibMirror(QByteArray data);
        void handleSetCalib2D(QByteArray data);
        void handleSetCalibTomo(QByteArray data);
        void handleSetCalibTomoFiltro(QByteArray data); // Hotfix 11C

    void setManualLameVal(QString lama, int val);
    void handleRotazioni(QByteArray data);
    void handleMoveTrx(QString tag, QByteArray data);
    void handleMoveArm(QString tag, QByteArray data);

    void handleBiopsy(QByteArray data);        


    void handleSetAlarm(QByteArray data, bool selfreset);

    // Funzioni per la gestione del compressore
    void handleCompressore(QByteArray data);
        void handleGetPadList(void);
        void handleSetThick(QByteArray data);
        void handleSetKF(QByteArray data);
        void handleSetPeso(QByteArray data);
        void handleGetCalibPad(QByteArray data);
        void handleSetCalibPad(QByteArray data);
        void handleGetCalibNacchera(void);
        void handleSetCalibNacchera(QByteArray data);
        void handleCalibThresholds(QByteArray data);



    // Funzioni per la gestione del potter
    void handlePotter(QByteArray data);
        void handleClearPCB244Errors(void);
        void handleResetPCB244(void);


        void handleSetGrid2D(QByteArray data);

    // Funzioni per device su canopen
    void handleCanOpen(QByteArray data);
        void handleCanOpen_test(QByteArray data);

        void debugPrint(QString data);
private:
    bool mccService(int id, _MccServiceNotify_Code cmd, QByteArray data);
    bool mccService(_MccServiceNotify_Code cmd);

    void handleList(void);                      // Visualizza la lista di comandi disponibili

    void handleConsole(QByteArray frame);       // Inietta una stringa di protocollo Console nel canale di ricezione della console


    void handleRodaggioTubo(QByteArray data);              // Comando di attivazione rodaggio del tubo
    void handleSetPad(QByteArray data);
    void handleSetPage(QByteArray data);

    void handleLoader(QByteArray data);         // Gestione comandi Loader
        void handleGetCRC(QByteArray data);
        void handleSetCRC(QByteArray data);
        void handleSetRemoteCRC(QByteArray data);
        void handleReadConfig(QByteArray data);     // Lettura configurazione
        void handleLoaderUpload(QByteArray data);   // Attivazione procedura caricamentoi manuale firmware



    int getVal(QString val);

    private:
    TcpIpServer*     serviceTcp; // Connessione da monitor esterno
    QByteArray      lastValidFrame; // Ultimo comando eseguito. Utilizzato per il comando repeat..
    QByteArray      cmdGroup;       // Ultimo gruppo di comandi selezionato


    // Invio e ricezione di dati a 16bit dai drivers
    QString frameTarget;
    bool frameCompleted;
    unsigned char  frameD0;
    unsigned char  frameD1;
    unsigned char  frameD2;
    unsigned short frameData;
    bool frameWrite;
    bool frameFormat16;
    bool frameDH;
    bool isCommand;
    bool isSpecial;


};

#endif // SERVERDEBUG_H
