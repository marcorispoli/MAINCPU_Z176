#ifndef CONSOLE_H
#define CONSOLE_H

////////////////////////////////////////////////////////////////////////////////////
// INTERFACCIA DI COMANDO CON LA CONSOLE
////////////////////////////////////////////////////////////////////////////////////
#include "application.h"

///////////////////////////////////////////////////////////////////
// FUNZIONE DI GESTIONE DELLA RICEZIONE DATI DA CONSOLE
//
///////////////////////////////////////////////////////////////////
// Nuovi comandi per Analogica
#define SET_ANALOG_FOCUS    "SetAnalogFocus"            // Forza l'impostazione del fuoco durante la calibrazione
#define GET_TUBE_TEMP    "GetTubeTemp"               // Forza la ricezione della temperatura anodo tubo

#define SET_ANALOG_PARAM    "SetAnalogParam"            // Scrive un parametro di configurazione
#define GET_ANALOG_PARAM    "GetAnalogParam"            // Legge un pËarametro di configurazione
#define STORE_ANALOG_PARAM  "StoreAnalogParam"          // Salva i parametri di configurazione

#define SET_ANALOG_START_LOG "SetAnalogStartLog"
#define SET_ANALOG_STOP_LOG  "SetAnalogStopLog"

#define SET_SERIAL_NUMBER    "SetSerialNumber"          // Imposta il SN: <OK 0> = OK, <NOK 1> = Wrong param, <NOK 2>= Gi‡ assegnato
#define GET_SERIAL_NUMBER    "GetSerialNumber"          // Chiede il SN:  <OK sernum> = OK, <NOK 1> = SN non assegnato
#define SET_SERVICE_PSW      "SetServicePsw"            // Imposta la password per i pannelli di service
#define GET_ANALOG_AF_SETUP  "GetAnalogAfSetup"         // Restituisce la combinazione Anodo Filtro correntemente attiva

#define GET_PROFILE_LIST     "GetProfileList"           // Richiede la lista di files profilo
#define GET_VALID_LIST       "GetValidList"             // Richiede la lista dei profili validati e attivi
#define SET_VALID_LIST       "SetValidList"             // Imposta la lista dei profili validati
#define GET_PROFILE          "GetProfile"               // Upload  contenuto di un profilo
#define SET_PROFILE          "SetProfile"               // Download contenuto di un profilo
#define ERASE_PROFILE        "EraseProfile"             // Elimina un profilo (no template)
#define ERASE_ALL_PROFILES   "EraseAllProfiles"         // Elimina tutti i profili (no Templates)

#define GET_PROFILE_NOTE      "GetProfileNote"          // Restituisce il campo testo "note" del profilo richiesto"
#define SET_PROFILE_NOTE      "SetProfileNote"          // Imposta il campo testo del profilo richiesto


#define SET_AEC_FIELD            "SetAnalogDetectorField"   // Imposta il campo del Detector (solo calibrazione detector)
#define SET_CALIB_FIELD          "SetCalibField"            // Imposta i tre valori di calibrazione RMI-155 e il coefficiente di accettazione
#define GET_CALIB_FIELD          "GetCalibField"            // Richiama i valori di riferimento RAD
#define SET_CALIB_PROFILE_DATA   "SetCalibProfileData"  // Imposta i parametri per l'esposizione ion calibrazione profilo

#define SET_ANALOG_KERMA_CALIB_DATA    "SetAnalogKermaCalibData"        // Imposta i dati per l'effettuazione della calibrazione del AirKerma
#define SET_ANALOG_KV_CALIB_TUBE_DATA  "SetAnalogKvCalibTubeData"       // Imposta i dati per l'effettuazione della calibrazione del tubo
#define SET_ANALOG_IA_CALIB_TUBE_DATA  "SetAnalogIaCalibTubeData"       // Imposta i dati per l'effettuazione della calibrazione del tubo
#define SET_ANALOG_CALIB_TUBE_OFFSET   "SetAnalogTubeOffset"            // Aggiunge l'offset

#define SET_ANALOG_STORE_KERMA          "SetAnalogStoreKerma"    // filtro, S, Dstrumento, DDetector, KVH, val, KVC, val, KVL, val
#define GET_ANALOG_KERMA_DATA           "GetAnalogKermaData"     // filtro  >>> S, Dstrumento, DDetector, KVH, val, KVC, val, KVL, val
#define GET_ANALOG_GET_CGS              "GetAnalogCgs"             //  filtro, kV


// Da realizzare
#define SET_STORE_ANALOG_CONFIG     "SetStoreAnalogConfig"     // Determina il salvataggio dei parametri nel file di configurazione analogica
#define SET_STORE_ANALOG_PROFILE    "SetStoreAnalogProfile"    // Salva i profili in memoria nella directory del tubo selezionato



// Nuovi comandi
#define SET_IMAGE            "SetImage"
#define SET_POWER_OFF        "SetPowerOff"
#define SET_REBOOT              "SetReboot"
#define SET_PROIEZIONI       "SetProjections"   // Invia elenco delle proiezioni selezionabili (max 8)
#define SEL_PROIEZIONE       "SelProjection"    // Invia codice della proiezione effettivamente selezionata
#define ABORT_PROJECTION     "AbortProjection"  // Comando di cancellazione proiezione corrente
#define GET_TOMO_CONFIG      "GetTomoConfig"    // Richiede dati di configurazione della tomo


// Non pi˘ implementate
#define SET_LOGO             "SetLogo"       // Imposta il logo che deve essere visualizzato
#define GET_LOGO             "GetLogo"       // Richiede quale logo √® correntemente caricato
#define SET_LAT              "SetLat"

//-----------------------------------------------------------------------------------------------------------------
#define SET_COMPRESSOR_RELEASE     "SetCompressorRelease"  // Imposta il modo Rilascio dopo compressione
#define SET_EXPOSE_NO_COMPRESSION  "SetExposeWithoutCompression"  // Disabilita il controllo sulla compressione

#define SET_CUR_DATE    "SetDate"       // Set current Date of time
#define SET_KV          "SetkV"         // Set Kv for the nest exposition
#define SET_MAS         "SetmAs"        // Set Mas for the next exposition
#define SET_TOMO_MAS    "SetmAsTomo"    // Set Mas for the next exposition
#define SET_AEC_TOMO    "SetTomoAEC"    // Set Mas for the next exposition
#define SET_TOMO_SPEED  "SetTomoSpeed"  // Imposta la modalita 1F/2F per la Tomo
#define SET_DETECTOR_TYPE  "SetDetectorType"  // Imposta il detector utilizzato
#define SET_AF          "SetAF"         // Set Anode Filter for the next exposition
#define SET_MODE        "SetMode"       // Set Pulse mode for the next exposition
#define SET_CALIB_MODE   "SetCalibMode"  // Modalit√  di test generica
#define SET_OPER_MODE   "SetOperMode"   // Modalit√  Operativa
#define SET_FOCUS       "SetFocus"      // W/Mo Small / Large
#define SET_LINGUA      "SetLanguage"     // Imposta la linguia corrente della UI
#define SET_PUSH_ENA    "SetEnablePush" // Enable The XRAY push button usage
#define SET_AUTO_COMPRESSION    "SetAutoCompression"
#define SET_UNLOCK_COMPRESSION  "SetUnlockCompression"
#define SET_OUTPUT_PULSE        "SetOutputPulse"
#define SET_READY       "SetReady"
#define SET_AEC_DATA    "SetAEC"        // Set dati di esposizione sequenza AEC
#define SET_AE_DATA     "SetAEData"      // Set dati di esposizione sequenza Alta energia
#define SET_STARTER     "SetStarter"    // Attivazione Starter % STOP/H/L %
#define SET_XRAY_SYM        "SetXray"
#define SET_XRAY_LAMP       "SetXrayLamp"
#define SET_TOMO_HOME_CALIB_MODE    "SetTomoHomeCalibMode" // ->modalit√  Tomo calib
#define SET_TOMO_HOME   "SetTomoHome"
#define SET_BIOPSY_DATA     "SetBiopsyData"
#define SET_BIOPSY_HOME     "SetBiopsyHome"
#define SET_TUBE        "SetTube"       // Muove TRX ad angolo
#define SET_ARM         "SetArm"        // Muove ARM ad angolo
#define SET_STOP_MOVE   "SetStopMove"   // Blocca qualsiasi movimento in corso
#define SET_DEAD_MAN    "SetDeadMan"    // Impostazione della funzione DEAD-MEN
#define SET_KV_VDAC_DATA    "SetKvVdacData"     // Imposta i dati di calibrazione kV in memoria
#define SET_KV_MONITOR_DATA "SetKvMonitorData"  // Imposta i dati di calibrazione lettura kV
#define SET_CALIB_KV_READ   "SetCalibKvRead"    // Imposta i coefficienti di calibrazione della lettura dei KV
#define SET_KV_RX_DATA      "SetKvRxData"       // Imposta i dati per l'effettuazione dei raggi in calibrazione
#define SET_IA_CALIB_MODE   "SetIACalibMode"    // Attivazione modalit√  di calibrazione corrente Anodica
#define SET_IDAC_DATA       "SetIdacData"       // Impostazione correnti IDAC 2D
#define SET_IDAC_TOMO_DATA  "SetIdacTomoData"   // Impostazione correnti IDAC 3D
#define SET_IA_RX_DATA      "SetIaRxData"       // <TBD>
#define SET_COLLI_TOMO_CALIB_MODE     "SetColliTomoCalibMode"   // Attivazione modalit√  di calibrazione colimazione tomo
#define SET_COLLI_MODE          "SetColliMode"         // Impostazione modalit√  Automatica o Manuale
#define SET_MANUAL_COLLI        "SetManualColli"       // Imposta il contenuto delle lame in modalit√  Manuale: effettua update
#define SET_COLLI_2D            "SetColli2D"           // Imposta il valore delle lame di un Pas 2D
#define SET_COLLI_TOMO          "SetColliTomo"         // Assegna un array di valori ad una lama del collimatore
#define SET_COLLI_FILTRO        "SetColliFilter"       // Associa i materiali alle posizioni del collimatore e le relative posizioni
#define SET_COLLI_MIRROR        "SetColliMirror"       // Posizione in campo dello specchio
#define SET_COLLI_STORE         "SetColliStore"        // Salva il file di configurazione del collimatore
#define SET_FIRMWARE_UPDATE     "SetFirmwareUpdate"     // Richiesta di aggiornamento del sistema
#define SET_SYS_BACKUP          "SetSystemBackup"       // Effettua il Backup del sistema
#define SET_SYS_RESTORE         "SetSystemRestore"      // Effettua il Restore del sistema
#define SET_USER_DIR_CLEAN      "SetUserDirClean"       // Cancella tutti i files contenuti nell directory /home/user
#define SET_DEMO_MODE     "SetDemoMode"     // Impostazione Modo demo (1/0)
#define SET_TEST_CMD     "SetTestCmd"     // Comando di test


#define GET_TROLLEY                "GetTrolley" // Restituisce la posizione corrente del piano compressore

#define GET_TUBE_STATISTICS "GetTubeStatistics" // Richiesta dati statistica Tubo
#define GET_PUSH        "GetPush"       // Richiede lo stato del pulsante (per la combo)
#define GET_THICK       "GetThick"      // Legge lo spessore
#define GET_FORCE       "GetForce"      // Legge la forza di compressione
#define GET_POTTER      "GetPotter"     // 0,1,2,3
                                        // 0 = screeneng; 1= 1.5  2= 2x 3 =biop
#define GET_COMPRESSOR  "GetComp"       // PArametro PADS // 8=unlock Compr, 9 = unlock pad, 10 Wrong
#define GET_ACR             "GetAcr"    // Richiede i codici ACR
#define GET_DEMO_MODE     "GetDemoMode"     // Richiede se il sistema √® in demo
#define GET_PACKAGE             "GetRevisions"          // Richiede l'insieme di tutti i firmwares della macchina
#define GET_HU                  "GetHu"                 // Richiede il valore degli HU
#define GET_COLLI_2D            "GetColli2D"           // Restituisce il valore delle lame di un PAD 2D
#define GET_COLLI_TOMO          "GetColliTomo"         // Chiede il contenuto di un array di valori ad una lama del collimatore
#define GET_COLLI_FILTRO        "GetColliFilter"       // Restituisce i materiali e le posizioni associati alle posizioni del filtro (0:3)
#define GET_COLLI_MIRROR        "GetColliMirror"       // Restituisce gli steps dello specchio in campo
#define GET_IA_RX           "GetIaRx"   // Richiesta ultima corrente in mA utilizzata per raggi
#define GET_GEN_CONF        "GetGenConf"  // Richiesta valore dei massimi kV e presenza starter
#define GET_TOMO_HOME   "GetTomoHome"
#define GET_BIOPSY_Z        "GetBiopsyZ"
#define GET_TRX         "GetTrx"        // Richiesta angolo Tubo
#define GET_ARM         "GetArm"        // Richiesta angolo Braccio
#define GET_TUBES               "GetTubes"              // Restituisce il nome di tutti i file di configurazione Tubi
#define GET_KV_INFO         "GetKvInfo"         // Richiede i dati di calibrazione Kv del Tubo corrente
#define GET_IA_INFO         "GetIaInfo"         // Acquisizione dati sulle correnti da calibrare 2D
#define GET_IA_TOMO_INFO    "GetIaTomoInfo"     // Acquisizione dati sulle correnti da calibrare Tomo



// DEBUG
#define START_POT_TOMO      "StartPotterTomo"
#define STOP_POT_TOMO       "StopPotter"
#define START_POT_2D        "StartPotter"



#define OPEN_STUDYL     "OpenStudyL"    // Open the Study for a local session
#define OPEN_STUDYW     "OpenStudyW"    // Open the Study for a remote session
#define CLOSE_STUDY     "CloseStudy"

#define START_SEQ       "StartSeq"      // Set the Start XRAY sequence (Vedi Set Mode)
#define RESET_ALARMS     "ResetAlarms"   // Reset degli allarmi in corso
#define RESET           "Reset"         // <TBD>
#define SELECT_TUBE             "SelectTube"            // Seleziona un Tubo e modifica syscfg.
#define STORE_TUBE_CONFIG_DATA  "StoreTubeConfigData"   // Salva i dati di configurazione Tubo in memoria in un file Tubo






class console : public QObject
{
    Q_OBJECT
public:
    explicit console(QObject *parent = 0);
    void activateConnections(void);

    // Codici risposte comandi inviati tramite signal consoleCmdAnsSgn
    typedef enum
    {
        COMANDO_RAGGI=1,     // Notifica fine raggi
        COMANDO_ARM,         // Notifica fine movimento braccio
        COMANDO_TRX          // Notifica fine movimento tubo

    }_CmdAnsw_Enum;

signals:
    void consoleTxHandler(QByteArray data);     // Funzione di gestione della trasmissione dati a console
    void consoleRxSgn(QByteArray data);            // Segnale emesso alla ricezione di un frame (debug use)

    void mccGuiNotify(unsigned char id, unsigned char code, QByteArray buffer); // segnale da connettere per ricevere notifiche da GUI/M4
    void mccConfigNotify(unsigned char id, unsigned char code, QByteArray buffer); // segnale da connettere per ricevere notifiche da GUI/M4
    void mccActuatorNotify(unsigned char id, unsigned char code, QByteArray buffer); // segnale da connettere per ricevere notifiche da ACTUATOR/M4
    void mccPcb190Notify(unsigned char id, unsigned char code, QByteArray buffer); // segnale da connettere per ricevere notifiche dal driver PCB190/M$
    void mccPcb215Notify(unsigned char id, unsigned char code, QByteArray buffer); // segnale da connettere per ricevere notifiche dal driver PCB215/M4
    void mccPcb249U1Notify(unsigned char id, unsigned char code, QByteArray buffer); // segnale da connettere per ricevere notifiche dal driver PCB249U1/M4
    void mccBiopsyNotify(unsigned char id, unsigned char code, QByteArray buffer); // segnale da connettere per ricevere notifiche dal driver PCB249U1/M4
    void mccServiceNotify(unsigned char id, unsigned char code, QByteArray buffer); // segnale da connettere per ricevere notifiche dal driver PCB249U1/M4
    void mccLoaderNotify(unsigned char id, unsigned char code, QByteArray buffer); // segnale da connettere per ricevere notifiche dal driver PCB249U1/M4
    void raggiDataSgn(QByteArray data); // Emissione info dati dopo raggi
    void mccPcb244ANotifySgn(unsigned char id, unsigned char code, QByteArray buffer); // segnale da connettere per ricevere notifiche dal driver PCB249U1/M4


    //    void raggiStdNotify(QByteArray);
    void consoleCmdAnsSgn(unsigned char code, QByteArray data); // Emette segnale di fine comando console eseguito

public slots:    

    void consoleConnectionHandler(bool stat);  // slot di gestione dello stato della connessione tcp/ip
    void consoleRxHandler(QByteArray data);    // Funzione di gestione della ricezione dati da console


    void emitMccActuatorNotify(unsigned char id, unsigned char code, QByteArray buffer) { emit mccActuatorNotify(id,code,buffer);} // Emette il segnale di notifica MCC
    void emitMccGuiNotify(unsigned char id, unsigned char code, QByteArray buffer) { emit mccGuiNotify(id,code,buffer);} // Emette il segnale di notifica MCC
    void emitConfigNotify(unsigned char id, unsigned char code, QByteArray buffer) { emit mccConfigNotify(id,code,buffer);} // Emette il segnale di notifica MCC
    void emitPcb190Notify(unsigned char id, unsigned char code, QByteArray buffer) { emit mccPcb190Notify(id,code,buffer);} // Emette il segnale di notifica MCC
    void emitPcb215Notify(unsigned char id, unsigned char code, QByteArray buffer) { emit mccPcb215Notify(id,code,buffer);} // Emette il segnale di notifica MCC
    void emitPcb249U1Notify(unsigned char id, unsigned char code, QByteArray buffer) { emit mccPcb249U1Notify(id,code,buffer);} // Emette il segnale di notifica MCC
    void emitBiopsyNotify(unsigned char id, unsigned char code, QByteArray buffer) { emit mccBiopsyNotify(id,code,buffer);} // Emette il segnale di notifica MCC
    void emitServiceNotify(unsigned char id, unsigned char code, QByteArray buffer) { emit mccServiceNotify(id,code,buffer);} // Emette il segnale di notifica MCC
    void emitLoaderNotify(unsigned char id, unsigned char code, QByteArray buffer) { emit mccLoaderNotify(id,code,buffer);} // Emette il segnale di notifica MCC
    void emitRaggiData(QByteArray data) {emit raggiDataSgn(data);}
    void emitConsoleTxHandler(QByteArray data) {emit consoleTxHandler(data);}

    void emitPcb244ANotify(unsigned char id, unsigned char code, QByteArray buffer) { emit mccPcb244ANotifySgn(id,code,buffer);} // Emette il segnale di notifica MCC

    void emitConsoleCmdAnswer(unsigned char code, QByteArray data) {emit consoleCmdAnsSgn(code, data);}
    void rxDataLog(QByteArray); // Notifica dati di fine esposizione

    void guiNotify(unsigned char id, unsigned char mcccode, QByteArray data);
    void consoleSoftwareRevisionsNotify(void); // Slot di ricezione delle revisioni dei software correnti
    void pcb249U1ConfigCompletedSlot(void);


private:


public:
    void setOpenStudy(bool local, QString stringa); // Apre lo studio con il nome paziente in stringa
    void handleSetXraySym(bool stat);               // Gestione attivazione segnale di raggi in corso


    // Impostazione modalit√  di esecuzione raggi
    void handleSetOperatingMode(void);                  // Attivazione modalit√  OPERATIVA

    void handleSetKvCalibMode(void);                    // Attivazione modalit√  calibrazione KV generatore

    bool isOperatingMode(void)
    {
        if(xSequence.workingMode == _EXPOSURE_MODE_OPERATING_MODE) return TRUE;
        else return FALSE;
    }

    bool isCalibKvMode(void)
    {
        if(xSequence.workingMode == _EXPOSURE_MODE_CALIB_MODE_KV) return TRUE;
        else return FALSE;
    }

    // Esecuzione power Off
    void handlePowerOff(void);

    // Comandi di impostazione dati per sequenze raggi
    int handleSetMode(protoConsole* pFrame); // Impostazione modalit√  Operativa
    void handleSetKv(QString param);
    void handleSetMas(QString qmAs);
    bool handleSetAf(QString param);
    bool handleSetFocus(QString materiale, QString fuoco);

    //________________ Funzioni per sequenze raggi _________________________________________________

    bool handleSetPushEna(QString param);               // Abilitazione all'uso del pulsante raggi

    // _______________________________________________________________________________________________

    // Handlers di gestione comandi ricevuti da console
    void handleSetDate(QString data, QString tempo);
    bool handleSetTube(QString param, unsigned char id);
    void handleSetArm(int target,int minimo, int massimo, int id);
    void handleSetStopMove(void);
    void handleSetXrayLamp(QString par);
    void handleSetDeadMan(bool val);

    void handleOpenStudy(bool local, protoConsole* pFrame);
    void handleCloseStudy(void);
    void handleGetThick(QString param);    
    void handleSetSblocco(void);


    void handleRotStore(void); // Salva il file di configurazione


    //___________________  Funzioni per calibrazione COLLIMATORE ______________________________________________________

    bool handleSetColliMode(protoConsole* frame);    // Imposta la modalit√  di collimazione tra Manuale e Automatica
    bool handleSetManualColli(protoConsole* frame);  // Imposta il valore delle lame della modalit√  Manuale

    bool handleSetColli2D(protoConsole* frame);                     // Restituisce le lame di un PAD 2D configurato
    void handleGetColli2D(protoConsole* frame, protoConsole* answer); // Restituisce le lame di un PAD 2D configurato


    bool handleSetColliFilter(protoConsole* frame); // Imposta i dati relativi al filtro
    void handleGetColliFilter(protoConsole* frame, protoConsole* answer); // Richiesta parametri Filtro

    bool handleSetColliMirror(protoConsole* frame); // Imposta gli steps di specchio in campo
    void handleGetColliMirror(protoConsole* frame, protoConsole* answer); // Richiesta parametri Specchio

    int  idColliStore; // Memorizza l'ID della richiesta di Store qualora occorra aggiornare la periferica
    bool handleSetColliStore(void);   // Salva il contenuto in RAM delle collimazioni sul file di configurazione

    //________________________________________________________________________________________________________________________

    bool handleSetStarter(protoConsole* frame);     // ATTIVA/DISATTIVA LO STARTER
    void handleGetSoftwareRevisions(protoConsole* frame); // Richiesta revisioni software

    // CALIBRAZIONE GENERATORE
    void handleGetTubes(protoConsole* answer);      // Richiesta info su Tubi configurati (filesystem)
    void handleSelectTube(QString nome, protoConsole* answer); // Chiede di selezionare un dato tubo
    void handleStoreTube(QString nome, protoConsole* answer); // Salva i dati di configurazione di un tubo in un nuovo tubo
    void handleStoreTube(protoConsole* answer); // Salva i dati di configurazione nello stesso tubo attivo
    void handleGetKvInfo(protoConsole* answer);       // Richiesta configurazione Kv
    void handleSetKvVdacData(protoConsole* frame, protoConsole* answer); // Imposta i valori dac dei kV
    void handleSetKvMonitorData(protoConsole* frame, protoConsole* answer); // Imposta i valori di calibrazione della lettura kV
    void handleSetCalibKvRead(protoConsole* frame, protoConsole* answer);

    // CALIBRAZIONE CORRENTE ANODICA
    bool handleSetIaCalibMode(void);
    void handleGetIaInfo(protoConsole* frame, protoConsole* answer);
    void handleGetIaTomoInfo( protoConsole* answer);
    bool handleSetIdacData(protoConsole* frame,  bool modifica);
    bool handleSetIdacTomoData(protoConsole* frame, protoConsole* answer);
    bool handleSetIaRxData(protoConsole* frame, protoConsole* answer);

    // GESTIONE BIOPSIA


    int  handleSetBiopsyHome(protoConsole* frame);      // Aggiorna la configurazione della calibrazione dell' offsetZ biopsia

    int  handleSetLingua(protoConsole* frame);          // Impostazione della lingua
    void handleSetFirmwareUpdate(protoConsole* frame, protoConsole* answer);    // Attivazione aggiornamento del sistema
    void handleSetSystemBackup(protoConsole* frame, protoConsole* answer);      // Effettua il backup del systema in /HOME/USER
    void handleSetSystemRestore(protoConsole* frame, protoConsole* answer);     // Effettua il restore del systema in /HOME/USER
    void handleSetUserDirClean(protoConsole* frame, protoConsole* answer); // Effettua la cancel√ lazione dei dati mnella home

    // Movimento del compressore
    void handleCompressorActivation(unsigned char mode);
    void handleOutputPulse(QString nout, QString time);

    void handleSetProiezioni(protoConsole* protocollo);

    // Funzione di servizio pr le altre classi
    void protocolAnswer(protoConsole* answer, QString cmd);

    // Funzioni per attivazioni MCC interne
    void demoBiopsy(bool status);               // Attivazione Demo Biopsy

    void setITest(bool status);                 // Attivazione corrente di test Anodica
    void getFilData(void);                      // Legge i dati dal test masmetro

    //_________________________________________________________
    // MACCHINA ANALOGICA

    bool handleSetAnalogKermaCalibData(protoConsole* frame, protoConsole* answer);
    bool handleSetAnalogKvCalibTubeData(protoConsole* frame, protoConsole* answer);
    bool handleSetAnalogIaCalibTubeData(protoConsole* frame, protoConsole* answer);
    bool handleSetAnalogCalibTubeOffset(protoConsole* frame, protoConsole* answer);

    void handleGetProfileList(protoConsole* frame, protoConsole* answer);
    void handleGetValidList(protoConsole* frame, protoConsole* answer);
    void handleSetValidList(protoConsole* frame, protoConsole* answer);
    void handleGetProfile(protoConsole* frame, protoConsole* answer);
    void handleSetProfile(protoConsole* frame, protoConsole* answer);
    void handleEraseProfile(protoConsole* frame, protoConsole* answer);
    void handleEraseAllProfiles(protoConsole* frame, protoConsole* answer);
    void handleSetProfileNote(protoConsole* frame, protoConsole* answer);
    void handleGetProfileNote(protoConsole* frame, protoConsole* answer);

    void handleGetAnalogAfSetup(protoConsole* frame, protoConsole* answer);
    void handleSetAnalogStoreKerma(protoConsole* frame, protoConsole* answer);
    void handleGetAnalogKermaData(protoConsole* frame, protoConsole* answer);
    void handleGetAnalogCgs(protoConsole* frame, protoConsole* answer);

    void handleSetAnalogParam(protoConsole* frame, protoConsole* answer);
    void handleGetAnalogParam(protoConsole* frame, protoConsole* answer);
    void handleStoreAnalogParam(protoConsole* frame, protoConsole* answer);

    TcpIpServer*     consoleSocketTcp;

private:
    QByteArray SetFrame(char comando, char* buffer, int buf_len); // Costruisce un frame valido con comando e buffer
    QByteArray frameFormat(QString data, QString id);



    typedef struct
    {
        // Modalit√  raggi in OPERATIVO
        bool isTomoN;
        bool isTomoW;
        bool isTomoI;
        bool isTomoMoving;
        bool is2D;
        bool isAEC;        
        bool isCombo;
        bool isAE;      // Alta energia

        bool isValid;
        unsigned char samples;  // Numero campioni in calibrazione Tomo
        int arm_angolo;         // Angolo richiesto da console
        int arm_min;            // Angolo minimo
        int arm_max;            // Angolo massimo

        bool disable_check_compression; // Disabilita il controllo sulla compressione

        unsigned char workingMode;   // Definisce la modalit√  operativa selezionata

        // Dati da ultime impostazioni
        float kVExposePRE;
        float kVExposeLE;
        float kVExposeAE;

    }_Raggi_Str;

    // Dati per la sequenza raggi di calibrazione kV
    typedef struct
    {
      bool  validated; // I dati sono stati rinfrescati dall'ultima sequenza
      QString anodo;   // Anodo selezionato per la sequenza (W/Mo)
      QString filtro;  // Filtro selezionato per la sequenza
      int   Vdac;      // Valore analogico da verificare
      int   Vnom;      // Tensione nominale (intera) aspettata
      int   Idac;      // Corrente analogica da utilizzare
      int   Inom;      // Corrente nominale aspettata
      int   mAs;       // mAs da utilizzare durante la sequenza
      unsigned char SWA;
      unsigned char SWB;

    }_kVCalibData_Str;

    // Dati per la sequenza raggi di calibrazione Ia
    typedef struct
    {
      bool  validated; // I dati sono stati rinfrescati dall'ultima sequenza
      QString anodo;   // Anodo selezionato per la sequenza (W/Mo)
      QString fuoco;   // IMposta la dimensione del fuoco
      int   Vnom;     // Tensione nominale (intera) da utilizzare
      int   Idac;      // Corrente analogica da utilizzare
      int   Inom;      // Corrente nominale aspettata
      int   mAs;       // mAs da utilizzare durante la sequenza
    }_iACalibData_Str;

    _Raggi_Str xSequence;
    _kVCalibData_Str kvCalibData; // Dati per esecuzione sequenza raggi in calibrazione KV
    _iACalibData_Str iACalibData; // Dati per la calibrazione mA anodici

public:

public:
    mccCom*  pGuiMcc;           // Comunicazione con Master M4 Core


    bool consoleConnected;      // TRUE se console connessa
    bool openStudy;             // TRUE se Studio Aperto
    bool localStudy;            // TRUE se lo studio √® locale
    QString studyName;          // Nome Studio aperto

    // Parametri di esposizione
    int currentMas;

    unsigned char pendingId;  // ID comando che √® in attesa di completamento


};


#endif // CONSOLE_H
