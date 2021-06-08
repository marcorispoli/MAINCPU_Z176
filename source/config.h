#ifndef CONFIG_H
#define CONFIG_H

#include "application.h"

class mccMasterCom: public mccCom
{
public:
    mccMasterCom(int x, int y, int z, bool mode) : mccCom(x,y,z,mode) {}
    // gestione dei comandi GUI provenienti da M4 MASTER
    void mccRxHandler(_MccFrame_Str mccframe);

};

class mccSlaveCom: public mccCom
{
public:
    mccSlaveCom(int x, int y, int z, bool mode) : mccCom(x,y,z,mode) {}
    // gestione dei comandi GUI provenienti da M4 SLAVE
    void mccRxHandler(_MccFrame_Str mccframe);
};

class Config : public QObject
{
    Q_OBJECT
public:

    void masterUpdateDatabase(void); // Effettua un aggiornamento del database sullo slave laddove necessario

    explicit Config(bool master, QObject *parent = 0);
    void configMaster(void);
    void configSlave(void);

    void activateMasterConnections(void);
    void activateSlaveConnections(void);

    // Caricamento files di configurazione per dispositivi su CAN
    bool readTrxConfig(void);
    bool saveTomoConfig(QString filename);
    bool readTomoConfig(QString filename);
    QString getTomoFilename(void);

    bool readArmConfig(void);
    bool readLenzeConfig(void);

    bool saveTrxConfig(void);
    bool saveArmConfig(void);
    bool saveLenzeConfig(void);

    bool setTomoSpeedMode(QString tomoMode, int console_id); // Impostazione file di configurazione rotazioni

    void updateAllDrivers(void);
    void updateGeneral(void);
    void updateSystemCfg(void);
    void updatePCB249U1(void);
    void updateBiopsy(void);
    void updatePCB244(void);
    void updatePCB249U2(void);
    void updatePCB269(void);
    void updatePCB190(void);
    void updateLenzeDriver(void);
    void updateArmDriver(void);
    void updateTrxDriver(void);

    void emitMccSlaveNotify(unsigned char id, unsigned char code, QByteArray buffer) { emit mccSlaveNotifySgn(id,code,buffer);}
    void txToSlave(QByteArray data) { emit configMasterTx(data);} // Invio frame verso Slave. Il frame deve essere formattato PROTOCONSOLE
    int convertDangolo(int data);


    QString getNanotecErrorClass(unsigned char classe);
    QString getNanotecErrorSubClass(unsigned char subclass);
    QString getNanotecErrorCode(unsigned short code);
    QString getI550ErrorString(unsigned short value);
    QString getI550DiagnosticErrorStr(unsigned char code);



    // Comandi su protocollo MASTER/SLAVE
    #define SET_PCB215_CALIB_DATA   "SetpPcb215CalibData"
    #define MASTER_EXIT_PROGRAM     "SetMasterExitProgram"
    #define MAIN_ACTIVATE_WARNING_BIOP_ADAPTER "SetWarningBiopAdapter"
    #define SLAVE_EXECUTE_REBOOT     "SetReboot"
    #define SLAVE_EXECUTE_POWEROFF   "SetPoweroff"
    #define SLAVE_EXECUTE_PC_POWEROFF   "SetPcPoweroff"

    #define SLAVE_EXECUTE_UPDATE_IDE "SetUpdateIde"
    #define SLAVE_EXECUTE_UPDATE_GUI "SetUpdateGui"
    #define SLAVE_EXECUTE_SHELL      "SetShellCommand"
    #define SLAVE_TOOLS_DATA         "SlaveToolsData"
    #define SYNC_TO_SLAVE            "SyncToSlave"


    // Comandi aggiornamento firmware
    #define SLAVE_EXTRACT_ARCHIVE           "SlaveExtract"   // Master-> Slave
    #define SLAVE_EXECUTE_BACKUP            "SlaveExecuteBackup" // Master ->Slave -> Master
    #define SLAVE_EXECUTE_RESTORE           "SlaveExecuteRestore" // Master ->Slave -> Master
    #define SLAVE_EXECUTE_CLEAN             "SlaveExecuteClean" // Master ->Slave -> Master

    // Comandi relativi a FTP
    #define FTP_START                       "FtpStart" // Master->Slave
    #define FTP_END                         "FtpEnd"   // Master-> Slave

    #define FTP_BLOCK                       "FtpBlock" // Slave->Master
    #define FTP_ERR                         "FtpErr"   // Comando da Slave -> Master per comunicare un errore durante il trasferimento

    // Codici di errore relativi agli aggiorrnamenti del sistema
    #define PACKAGE_INESISTENTE     1
    #define FIRMWARE_INESISTENTE    2
    #define FORMATO_HEX             3
    #define FILE_INDICE_INESISTENTE 4
    #define FORMATO_INDICE_ERRATO   5
    #define LOAD_HEX                6
    #define MCC_COMMAND             7
    #define ERR_ACTIVATION          8
    #define ERR_ERASE               9
    #define ERR_BLOCK_WRITE         10
    #define ERR_CONFIG              11
    #define POTTER_NON_CONNESSO     12
    #define HARDWARE_INCOMPATIBILE  13
    #define FILE_TRANSFER_ERR       14

    // Formato files per Aggiornamento sistema
    #define FIRMWARE_FILE       QString("/resource/config/firmwares.cnf")
    #define INSTALL_LOG         QString("/resource/config/install.log")
    #define FILE_HOME           QString("/home/user/")
    #define INSTALL_DIR         QString("/INSTALL")

    #define MASTER_TAR_PACKAGE      FILE_HOME + QString("SWPackage.tar")    // Pacchetto di aggiornamento per il Master
    #define SLAVE_TAR_PACKAGE       FILE_HOME + QString("SWPackage.tar")  // Pacchetto di aggiornamento per lo slave (contenente anche comandi)
    #define PACKAGE_FIRMWARE        FILE_HOME + QString("FirmwarePackage.tar")  // Pacchetto di aggiornamento Firmware


    #define FILE_INDICE         FILE_HOME + QString("firmwares.cnf")
    #define FILE_DBT_GUI        FILE_HOME + QString("DBTController")
    #define FILE_DBT_M4_MASTER  FILE_HOME + QString("m4_master.bin")
    #define FILE_269            FILE_HOME + QString("FW269.hex")
    #define FILE_240            FILE_HOME + QString("FW240.hex")
    #define FILE_244            FILE_HOME + QString("FW244.hex")
    #define FILE_244A           FILE_HOME + QString("FW244A.hex")
    #define FILE_249U1          FILE_HOME + QString("FW249U1.hex")
    #define FILE_249U2          FILE_HOME + QString("FW249U2.hex")
    #define FILE_249U1A          FILE_HOME + QString("FW249U1A.hex")
    #define FILE_249U2A          FILE_HOME + QString("FW249U2A.hex")
    #define FILE_190            FILE_HOME + QString("FW190A.hex")



    // Nomi files di configurazione rotazioni
    #define ROTAZIONI_1F    "rotazioni_1F.cnf"
    #define ROTAZIONI_2F    "rotazioni_2F.cnf"

signals:
    void sysUpdateCompletedSgn(bool esito, QString errstr);
    void configRevisionSgn(void); // Segnale di avvenuto aggiornamento revisioni software di sistema
    void mccSlaveNotifySgn(unsigned char id, unsigned char code, QByteArray buffer); // segnale da connettere per ricevere notifiche da Ricezione M4
    void configUpdatedSgn(void); // Segnale di configurazione completata


    void configSlaveTx(QByteArray data); // Segnale per invio dati verso Master tramite socket
    void configMasterTx(QByteArray data); // Segnale per invio dati verso Slave tramite socket

    void ftpSgn(int data);  // Segnale di termine ftp

    void sgnRemoteShell(QByteArray); // Rispoosta ricevuta da slave su comando shell
    void awsTxHandler(QByteArray); // Segnale per invio su socket

public slots:
    void configSlaveRxHandler(QByteArray data); // Ricezione da Master frame di controllo su protocollo console
    void configMasterRxHandler(QByteArray data); // Ricezione da Slave frame di controllo su protocollo console

    void configNotifySlot(unsigned char id, unsigned char mcccode, QByteArray buffer); // Notifiche MCC gruppo configurazione
    void slaveNotifySlot(unsigned char id,unsigned char mcc_code,QByteArray data);
    void setRotazioniCfgSlot(void);


    // Slot di ricezione Risposte M4 Gruppo CONFIG


    void timerEvent(QTimerEvent* ev); // Override della classe QObject
    void onInstallPackage(void);
    void onOkCompletedInstallPackage(void);


    void ftpSlavePackageCompleted(int esito); // Slot chiamato al termine del trasferimento del package allo slave
    void slaveExtractArchive(void); // Master comanda allo slave l'estrazione dell'archivio di aggiornamento
    void loaderCompletedSlot(bool esito, QString errstr);// Slot associato alla signal di fine operazioni da parte del loader
    void ftpPrintMsg(int val);

    void localInstallMonitorCompleted(bool esito, QString errstr);


    void awsConnectionHandler(bool status);

    void powerOffSlot(void);
    void rebootSlot(void);

public:
    void selectOperatingPage(void);
    void selectMainPage(void);


    bool updateSoftwareRevision(void); // Richiesta di aggiornamento

    QString getM4StatusString(unsigned char codice);
    bool openSysCfg(void);          // Apre il file di configurazione di systema alla creazione della classe
    bool saveSysCfg(void);          // Salva il file di configurazione sulla base di quanto caricato in memoria
    bool openUserCfg(void);         // Apre il file di configurazione delle funzioni utente memorizzate
    bool saveUserCfg(void);         // Apre il file di configurazione delle funzioni utente memorizzate

    bool openPackageCfg(QString filename, firmwareCfg_Str* swConf);           // Apre il file di compatibilit�  versioni firmware
    bool savePackageCfg(QString filename, firmwareCfg_Str sw);

    // Apertura configurazione per macchine analogiche
    bool openAnalogConfig(void);
    bool saveAnalogConfig(void);

    bool fileTransfer(QString origine, QString destinazione);

    // Aggiornamento software del mammografo
    void sysUpdate(QString archivio);   // Entry point per l'aggiornamento generale    
    void slaveUpdated(void);
    bool isSystemUpdated(){
        if((master_updated==true)&&(slave_updated==true)&&(firmware_updated==true)) return true;
        return false;
    }
    void sysBackup(bool isMaster,QString filename, int id);
    void sysRestore(bool isMaster,QString filename, int id);
    void userDirClean(bool isMaster,int id); // Cancellazione dati in home
    void executeCmdFile(QString file); // Esegue comandi shell contenuti in un file di testo
    bool packageExist(void);

    // Varie
    bool setDemoMode(bool demo);
    void setToolsData(int tipo, QList<int> data);
    void setToolsData(int tipo);

    void awsPowerOff(void); // Eseguito dallo slave
    void masterRequestAwsPowerOff(void); // Comando Da Master per richiedere allo slave lo spegnimento del PC

    void activatePowerOff(void);
private:

    int revCount;       // NUmero di tic di visualizzazione
    int timerInstall;
    int mainsOnCnt;
    int timerXrayPush; // Timer per monitorare la pressione del pulsante raggi

    void readLogo(void);

public:
    bool isMaster; // Master o Slave

    bool pcb249U1UpdateRequired;

    bool checkPackage(void);
    bool revisionOK;
    QString revisionError;
    firmwareCfg_Str swConf; // Contenuto del package

    // Revisioni ricevute dai dispositivi
    QString rvGuiMaster;
    QString rvGuiSlave;
    QString rvM4Master;
    QString rvM4Slave;
    QString rv269;
    QString rv240;
    QString rv249U1;
    QString rv249U2;
    QString rv190;
    QString rv244;
    QString rv244A;
    QString rvPackage;


    unsigned char devIndex; // Indice blocco richiesto

    // Configurazione sistema
    trxConfig_Str   trxConfig;
    armConfig_Str   armConfig;
    lenzeConfig_Str lenzeConfig;

    systemCfg_Str   sys;            // System config
    userCnf_Str     userCnf;        // Dati uso interfaccia
    analogCnf_Str   analogCnf;      // Configurazione analogice

    // Funzioni per gestire i file di configurazione
    static QList<QString> getNextArrayFields(QFile* fp); // Restituisce una lista di Items tra <> e separati da virgole
    static QString removeSpaces(QString frame);          // Restituisce una stringa privata di spazi prima e dopo

    static QByteArray getNextValidLine(QFile* fp); // Restituisce il contenuto di una riga valida o Empty se nessuna riga è disponibile
    static QByteArray getNextParam(QByteArray* dato, int* index, bool last); // Restituisce il prossimo contenuto tra le virgole oppure l'ultimo,
    static QByteArray getNextTag(QByteArray* dato, int* index, bool last);   // Restituisce il prossimo "TAG"
    static QByteArray getLine(QFile* fp);
    static bool getNextLine(QFile* fp, unsigned char* data);
    static bool getNextLine(QFile* fp, signed char* data);
    static bool getNextLine(QFile* fp, unsigned short* data);
    static bool getNextLine(QFile* fp, signed short* data);
    static bool getNextLine(QFile* fp, signed int* data);
    static bool getNextLine(QFile* fp, unsigned int* data);
    static bool getNextLine(QFile* fp, QByteArray* data);
    static bool getNextLine(QFile* fp, QString* data);
    static bool getNextArrayLine(QFile* fp, unsigned char* data, int len);
    static bool getNextArrayLine(QFile* fp, int* data, int len);
    static QByteArray getNextArrayLine(QFile* fp);                 // Legge Array di caratteri di lunghezza variabile


    static void writeNextArrayLine(QFile* filedest, QFile* filesrc,int* array, int len); // Solo campi sequenziali
    static void writeNextStringLine(QFile* filedest, QFile* filesrc,QString stringa);   // Solo campi sequenziali
    static void writeNextStringLine(QFile* filedest,QString stringa);                   // Campi non sequenziali
    static bool alignFileWithTag(QFile* file,QString tag);                              // Allineamento del file ad un tag
    static QByteArray alignFileWithValidField(QFile* filedest, QFile* filesrc);

    // Socket di interconnessione tra i Config MASTER/SLAVE
    TcpIpServer*  configSlaveSocketTcp; // Server di ricezione su SLAVE
    TcpIpClient*  configMasterTcp;      // Client di trasmissione su MASTER
    mccCom*  pSlaveMcc;                // Comunicazione con SLAVE M4 Core


    // Operazione di aggiornamento del sistema
    bool master_updated;
    bool slave_updated;
    bool firmware_updated;

    // Esito dell'aggiornamento
    QString updErrStr;      // Stringa di errore in caso di fallimento
    bool    updError;       // Flag di errore attivato

    // File transfer Master side
    QFile ftpFile;
    unsigned short sentBlock;

    // File transfer Slave side
    QString pendingFileTransfer;
    bool ftpEnabled;


    // Configuration
    bool sendMccConfigCommand(unsigned char cmd);
    bool singleConfigUpdate; // Se TRUE identifica un singolo aggiornamento e non la configurazione generale

    // Flag di startup compèletato correttamente...

    bool rebootRequest; // Richiesta di reboot
    void executeReboot(); // Funzione per attivare la sequenza di reboot di sistema
    int timerReboot;

    // Comando di poweroff generale dei terminali
    void executePoweroff(unsigned char secondi);
    int timerPoweroff;

    // Effettua l'update generale quando ci sia il computer attaccato alla rete
    bool executeUpdateIde();
    bool executeUpdateGui();


    QByteArray executeShell(QByteArray comando);
    void executeSlaveShell(QByteArray data);

    int console_id; // VAriabile per salvare

    void syncToSlave(void); // Master only!!!

    bool awsConnected;                     // Stato della connessione
    TcpIpClient*  awsTcp;      // Socket Client per invio notifiche asincrone

    // Data structures for the inizialization process __________________________________________________________________
    bool systemInitialized;         // Vale per Master e Slave, attivato al termine della creazione di tutte le classi
    bool slaveDataInitialized;      // Solo per lo slave, dopo aver inizializzato i dati condivisi con il Master, dopo il SYNC
    void slaveInitialization(void); // Funzione chiamata dallo Slave per completare il data initialization

    bool sysConfigured;             // Architettura di sistema correttamente configurata
    bool userConfigured;            // File di configurazione user correttamente aperto
    bool packageConfigured;         // File package correttamente caricato
    bool SN_Configured;             // Serial Number configurato

    bool startupCompleted ;         // Fase di startup completata
    bool generator_configured;      // Il Generatore correttamente configurato
    bool compressor_configured;     // Compressore configurato
    bool collimator_configured;     // Configurazione collimatore acquisita

    bool aec_configured;            // AEC configurato
    bool kerma_mo_configured;       // Kerma data per Mo configurato;
    bool kerma_rh_configured;       // Kerma data per Rh configurato;
    bool kerma_cg_configured;       // Parametri CG configurati
    bool analog_configured;             // File di configurazione analogica

    // Bitfield relativi al test della configurazione di sistema
    // TRUE = ERROR
    bool testConfigError(bool msgon, bool forceshow);

    // RTC SECTION
    bool  rtc_present;
    unsigned char weekday;
    unsigned int  year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char min;
    unsigned char sec;
    void updateDate(void);
    void updateRTC(void);
};

#endif // CONFIG_H
