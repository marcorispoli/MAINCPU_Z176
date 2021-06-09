#ifndef GENERATORE_H
#define GENERATORE_H

#include "application.h"
#include "AEC.h"
#include "DOSE.h"
#define _TUBE_CONFIG_NAME "tube.cnf"
#define _TUBE_KV_OFFSET_NAME "KV_OFFS.cnf"
#define _TUBE_CALIB_NAME  "calibrated.cnf"
#define _KVPREFIX         "KV"
#define _KVSUFFIX         ".cnf"
#define _TUBE_REV_FILENAME "revision.cnf"
#define _CUR_TEMPLATE_REV 1             // Definisce la revisione corrente del file di configurazione


class Generatore : public QObject
{
    Q_OBJECT

public:
    explicit Generatore(QObject *parent = 0);
    void activateConnections(void);

signals:
    void sigStopStarter(void);

public slots:
    void updateFuocoNotify(unsigned char id, unsigned char mcccode, QByteArray buffer);
    void pcb190Notify(unsigned char id, unsigned char cmd, QByteArray data);
    void stopStarterSlot(void);

public:
    #define _MIN_KV     20  // Minima tensione selezionabile
    #define _MAX_KV     49  // Massima tensione selezionabile
    #define _MAX_MAS    640 // Massimo numero di mAs selezionabili
    #define _MAX_In     200 // Massima corrente nominale regolabile

    typedef enum
    {
        FUOCO_LARGE,
        FUOCO_SMALL,
        FUOCO_SZ_ND
    }_FuocoSize_Enum;


    // Apertura generale dati del tubo
    bool openTube(QString tubeDir); // Richiesta di caricamento dati tubo
        int getTubeRevision(QString filename);
        void setTubeRevision(QString tube);



    bool saveTube(QString filename); // Salva il file di configurazione
    bool openCurrentTube();         // Riapre il tubo gi√  selezionato

        bool readTubeCalibFile(QString tubeDir); // Legge se esiste il file con le ultime calibrazioni effettuate

        void readTubeStatisticsFile(QString tubename); // Legge le statistiche accumulate per il tubo in oggetto
        void saveTubeStatisticsFile(QString tubename); // Scrive le statistiche accumulate per il tubo in oggetto

        void readTubeFilamentFile(QString tubename);    // Legge i dati di gestione del filamento del tubo
        void saveTubeFilamentFile(QString tubename);    // Salva i dati di gestione del filamento del tubo

        void saveTubeKvOffsetFile(QString tubeDir); // Salva il contenuto degli offset per un tubo generico
        bool readTubeKvOffsetFile(QString tubeDir); // Legge il contenuto degli offset per un tubo generico

        void readTubeKvReadCalibrationFile(); // Coefficienti di calibrazione della lettura dei kV
        void saveTubeKvReadCalibrationFile();

        void saveStarterFile(void); // Lettura scrittura dati di calibrazione diagnostica STarter
        void readStarterFile(void);

        bool isStarterCalibrated(void); // Verifica se lo starter Ë stato calibrato
        int getIminIndex(int kV, QString anodo, int fsize);
        int getImaxIndex(int kV, QString anodo, int fsize);


    unsigned char validateData(void) ;       // Validazione dei dati del generatore per modelli digitali
    unsigned char validateAnalogData(unsigned char modo, bool calibMode, bool isPre);// Validazione dei dati del generatore per modelli analogici

    bool setmAs(double mAs);           // Imposta i mAs
    bool setMasTomo(unsigned short index, double mAs);           // Imposta i mAs

    bool setkV(float kV);   // Imposta i kV

    bool getIdacForKvCalibration(int kV,  QString anodo, int* Idac, int* Inom);

    void manualShot(QString filename);
    bool manualShot(void);

    void fuocoOff(void); // Spegne il filamento

    // Restituisce TRUE se i KV indicati sono attivi
    bool isKvEnabled(int val) {
        if((val<_MIN_KV)||(val>_MAX_KV)) return FALSE;
        return tube[val-_MIN_KV].vRef.enable;
    }

    // Definizione delle correnti definite per KV
    typedef struct
    {
        char sym;            // Simbolo asociato
        int In;              // Corrente nominale
        int INcalib;         // Corrente effettivamente calibrata
        int Idac;            // Corrente calibrata
        float derivata;      // Derivata della curva nel punto di calibrazione (KV/DAC)
        QString anode;       // Lista materiali associati all'Anodo
        unsigned char fsize; // Dimensione del fuoco
    }_ITabStr;

    typedef struct
    {
      int da;               // Intervallo > di
      int a;                // Intervallo <= di
      char sym;             // Simbolo asociato
      QString anode;        // Materiale associato
      unsigned char fsize;  // Fuoco associato
    }_mAsStr;

    typedef struct
    {
      int da;            // Intervallo > di
      int a;             // Intervallo <= di
      QString anode;              // Materiale associato
      unsigned char fsize;        // Fuoco associato
    }_starterStr;

    typedef struct
    {
        unsigned short Vread;   // Valore analogico riletto dall'ingresso kV della PCB190
        unsigned short Vdac;    // Valore dac
        int  Voffset;           // Valore % di variazione sul DAC calibrato, per gestire variazioni locali di +-2KV (circa 80/Kv punti)
        bool SWA;               // Relay A scambio primari
        bool SWB;               // Relay B scambio primari
        bool enable;            // Abilitazione uso di questa tensione
    }_KvStr;

    // <"T",105,2275>
    typedef struct
    {
        int  In;        // Corrente nominale
        int  INcalib;   // Corrente effettivamente calibrata
        int  Idac;      // Corrente calibrata
        float derivata; // Derivata della curva nel punto di calibrazione (KV/DAC)
        bool enabled;   // Blocco abilitato
        QString anode;  // Anodo su cui √® configurato        
    }_TomoStr;

    typedef struct
    {
      QList<_ITabStr> iTab;         // Tabella correnti associate ai fuochi
      QList<_mAsStr>  mAs;          // Lista carichi 2D per tutti i fuochi
      QList<_starterStr> starter;   // Velocit√  starter
      _KvStr      vRef;             // Tensioni di riferimento
      QList<_TomoStr>  tomo;        // Correnti per sessioni Tomo
    }_tubeStr;


    // CONFIGURAZIONE DEL GENERATORE
    _tubeStr   tube[_MAX_KV-_MIN_KV+1];
    int max_selectable_kv;   // Massimo valore di kV selezionabili
    int getMaxKv(void){return max_selectable_kv;}

    QString confF1;          // Associazione F1 a materiale
    QString confF2;          // Associazione F2 a materiale
    _FuocoSize_Enum startupSelectedFSize;  // Dimensione Fuoco selezionato alla partenza
    QString         startupSelectedAnodo;  // Materiale Anodo selezionato alla pertenza

    QString getKvCalibAnode(void); // Seleziona automaticamente l'anodo disponibile per la calibrazione dei Kv

    QString getSelectedFuocoMat(void) { return selectedAnodo;} // Restituisce il materiale dell'Anodo selezionato
    _FuocoSize_Enum  getSelectedFuocoSize(void){ return selectedFSize;} // Restituisce la dimensione del fuoco selezionato

    // Imposta il fuoco con Anodo F1 - Grande
    void setFuocoGrande(void){
        selectedAnodo = confF1;
        selectedFSize = FUOCO_LARGE;
    }

    // Imposta il fuoco con Anodo F1 - Piccolo
    void setFuocoPiccolo(void){
        selectedAnodo = confF1;
        selectedFSize = FUOCO_SMALL;
    }

    // Imposta il fuoco con anodo richiesto (per bimetallici)
    bool setFuoco(QString mat) {                                        // Imposta il Materiale dell'Anodo selezionato
        if((mat!=confF1)&&(mat!=confF2)) return FALSE;
        selectedAnodo = mat;
        return TRUE;
    }
    void setFuoco(_FuocoSize_Enum fuocoSize) { selectedFSize = fuocoSize;} // Imposta la dimensione del fuoco
    bool isValidFuoco(void) {
        if((selectedFSize!=FUOCO_SMALL)&&(selectedFSize!=FUOCO_LARGE)) return FALSE;
        if((selectedAnodo!=confF1)&&(selectedAnodo!=confF2)) return FALSE;
        return TRUE;
    }
    bool isValidAnode(QString mat) {
        if((mat!=confF1)&&(mat!=confF2)) return FALSE;
        return TRUE;
    }
    bool isValidKv(int val) {
        if(val>_MAX_KV) return FALSE;
        if(val<_MIN_KV) return FALSE;
        return tube[val-_MIN_KV].vRef.enable;
    }

    unsigned short getVdac(int val)
    {
        if(!isValidKv(val)) return 0;
        return tube[val-_MIN_KV].vRef.Vdac;
    }

    bool getSWA(int val)
    {
        if(!isValidKv(val)) return FALSE;
        return tube[val-_MIN_KV].vRef.SWA;
    }

    bool getSWB(int val)
    {
        if(!isValidKv(val)) return FALSE;
        return tube[val-_MIN_KV].vRef.SWB;
    }


    // Conversione lettura KV da PCB190
    unsigned char convertPcb190Kv(unsigned char val);       // Converte il valore RAW letto in kv interi
    float convertPcb190Kv(unsigned char val, float k_corr);       // Converte il valore VreadRAW letto in kv float

    unsigned char convertPcb190KvToRaw(unsigned short kV);  // Converte il valore VDAC in Vread(raw)
    unsigned char convertPcb190Ianode(unsigned char val);   // Converte il valore IRAW letto in mA interi
    unsigned char convertPcb190IanodeToRaw(unsigned short Ianode);// Converte il valore IDAC in Iread(raw)


    bool updateFuoco(void); // Attiva il fuoco correntemente selezionato

    // Diagnostica
    void testFilamento(void);
    void testHV(void);

public:
    genCnf_Str      genCnf;     // parametri generici di gestione Generatore
    AEC*            pAECprofiles; // In caso di macchine analogiche, vengono gestiti i profili AEC
    DOSE*           pDose;

    bool            validated;      // La selezione corrente √® valida    
    float           selectedKv;     // KV selezionati reali

    bool            filOn;          // Stato del filamento (ON/OFF)
    _FuocoSize_Enum selectedFSize;  // Dimensione Fuoco selezionato
    QString         selectedAnodo;  // Materiale Anodo selezionato

    unsigned int    selectedIn;     // Corrente nominale selezionata
    unsigned int    selectedDmAs;   // Decimo di mAs selezionati
    unsigned short  selectedIdac;   // Corrente Dac selezionata;
    unsigned short  selectedVdac;   // Tensione Dac selezionata;
    unsigned short  selectedmAsDac; // mAs convertiti in unit√  per il driver PCB190
    bool            SWA;            // Scambio primari A
    bool            SWB;            // Scambio primari B
    bool            starterHS;      // Velocit√  starter

    double           selectedmAsTomo; // mAs selezionati per la Tomo
    //unsigned short  selectedmAsTomo; // mAs selezionati per la Tomo
    unsigned short  selectedmAsDacTomo; // mAs Tomo convertiti in unit√  per il driver PCB190


    unsigned char   timeoutExp;     // Timeout esposizione sulla base della selezione
    unsigned short  maxI;           // Massima corrente rilevabile
    unsigned short  minI;           // Minima corrente rilevabile
    unsigned short  maxV;           // Massima tensione rilevabile
    unsigned short  minV;           // Minima tensione rilevabile
    unsigned char   minVPcc;        // Minima tensione di alimentazione rilevabile;

    unsigned char error;            // Errore rilevato in validazione

    bool tomoMode;                  // Modalit√  Tomo attiva
    bool aecMode;                   // Modalit√  AEC attivata

    // Gestione Starter
    int timerStarter;
    void stopStarter(void);         // Ferma immediatamente lo starter
    void refreshStarter(void);      // Rinfresca il timeout o fa partire il timer;
    void setStarter(unsigned char stat); // Attivazione starter

    void timerEvent(QTimerEvent* ev);

    // ---------------------------------------------------------------------------------------
    // Gestione della tensione HV
    void openHVcalibFile(void);
    void storeHVcalibFile(void);
    int  convertHvexp(unsigned char data);
    void verifyHValarm(void);

    #define _HV_POWER_H     660 // Oltre il 20%
    #define _HV_WRN_POWER_H 644 // Oltre il 15%
    #define _HV_STD_POWER_H 616 // Oltre il 10%
    #define _HV_STD_POWER_L 504
    #define _HV_WRN_POWER_L 476
    #define _HV_POWER_L     440

    bool fault_starter;

    // Gestione lettura tensione Potenza HV
    bool hv_calibrated; // Indica se la HV √® stata calibrata (esiste il file di calibrazione)
    bool hv_updated;    // Indica che ha ricevuto un aggiornamento

    int  hvraw;         // Valore non calibrato

    // Stato di funzionamento della tensione di potenza
    int  hvval;         // Valore calibrato HV
    bool warningHV;     // Valore oltre il 10%
    bool faultHV;       // Valore oltre 15%(parametro) o 20%
    int  levelHV;       // Livello identificato [0:6]

    // Lettura corrente di filamento
    int iFilamento;         // Corrente di filamento riletta
    bool warningIFilamento; // Warning calibrazione filamento
    bool faultIFilamento;   // Errore sul filamento

    // Lettura temperatura dell'amplificatore di filamento
    int tempAmplFil;        // Temperatura Amplificatore di filamento
    bool faultFilAmpTemp;   // Fault dell'amplificatore

    unsigned char flags0;
    int anodicaTest;        // Rilettura corrente anodica durante IDLE

    float mAsTest;          // Rilettura mAs durante i test
    bool warningMas;        // Valore anomalo masmetro
    bool faultMas;          // Fault mAsmetro

    bool faultGnd;          // Fault sulla connessione GND
    bool faultR16;          // Guasto resistenza R16

    bool warningITest;      // Valore anomalo corrente di test
    bool faultITest;        // Calibrazione lettura anodica sballata

    // Tensioni di alimentazione scheda
    float v32;
    bool  v32_warning;
    float vm32;
    bool  vm32_warning;
    float v12;
    bool  v12_warning;
    float vm12;
    bool  vm12_warning;
    float v15;
    bool  v15_warning;
    float v15ext;
    bool  v15ext_warning;


    bool dgn_fault; // Flag di fault attivo
    int  dgn_fault_code; // Codice di errore salvato

    QString faultString; // Stringa con le indicazioni di fault


    // Funzioni interne per la lettura della configurazione del tubo
    int  templateRevision; // Indica la revisione corrente del template

    bool readKvFile(unsigned char kV, QString path);                // Lettura di un file dati KV
    bool readTubeConfigFile(QFile* fp);                      // Lettura di un file dati KV
    bool getVLine(QByteArray* dato, int* index, unsigned char kV);
    bool getMLine(QByteArray* dato, int* index, unsigned char kV);
    bool getILine(QByteArray* dato, int* index, unsigned char kV);
    bool getSLine(QByteArray* dato, int* index, unsigned char kV);    
    bool getTLine(QByteArray* dato, int* index, unsigned char kV);

    int  getITabIndex(unsigned char kV);
    bool isStarterHS(unsigned char kV, int mAs);
    bool getValidKv(float val, unsigned char* kv, unsigned short* vdac);


    // Contatori statistici
    void notifyStatisticData(int datakv, unsigned int datamAs, bool isStandard);
    bool statChanged;
    long numShots;
    long cumulatedJ;
    int  nTomo;
    int  nStandard;
    QDateTime anodeHuTime; // Data ultimo aggiornamento calore anodico
    float anodeHUSaved;    // VAlore HU al momento della registrazione

    // Esposizioni in manuale
    bool manualMode;     // Modalit√  manuale attivata
    bool manAutoIdacInc; // Modalit√  auto incremento Idac
    unsigned short manDIdac; // Quantit√  di incremento

    unsigned short manVnom;
    unsigned short manVdac;
    unsigned short manInom;
    unsigned short manIdac;
    QString        manFuoco;
    unsigned short manMas;
    unsigned short manHs;
    unsigned short manSWA;
    unsigned short manSWB;
    unsigned short manGrid;
    unsigned short manTMO;

    /*
    *                          (550000 * 0.9) * (Tcuffia-Tamb)
    *          HU cuffia =    ----------------------------------
    *                                  (40)
    */
    // Variabili per il calcolo degli HU accumulati sull'anodo
    // Ad ogni RX viene aggiunto istantaneamente la quota energetica a quella residua
    // e viene ricalcolato il T0 della curva teorica.
    // HU anodica deve essere inizializzata sulla base della temperatua della cuffia
    // e comunque non puÚ essere considerata pi˘ bassa della temperatura cuffia stessa
    // La formula del raffreddamento Ë stata ricavata dal DS IAE XM1016T e interpolata
    // kHU = 1.33 * (-39.6 * Ln(t)+324) = -52.77 Ln(t) + 430.9
    QElapsedTimer hutimer;
    int timerUpdateAnodeHU;
    bool anodeHuInitOk; // Indica che Ë stato inizializzato
    bool alarmAnodeHU;  // Diagnostica
    long T0; // Tempo in millisecondi

    float anodeHU;
    long hutimer_elapsed;

    // Funzione da chiamare quando si voglia aggiungere HU da qualsiasi fonte
    float updateAnodeHU(float hu);

    // Funzione che restituisce il valore corrente degli HU
    float getCurrentHU(void);

    // Inizializzazione all'avvio: occorre attendere un aggiornamento
    // della temperatura della cuffia per poter procedere.
    void initAnodeHU(void);
    void initAnodeHU(int Y, int M, int D, int h, int m, int s);


    int     getMaxDMas(unsigned char kV, QString anodo, unsigned char fuoco); // GEnerico
    int     getMaxDMas(void);    // Lavora sulla selezione corrente
    float   getCurrentAnalogKv(void);   // Restituisce i kV correnti
    int     getCurrentAnalogDmAs(void);    // Restituisce i mAs correnti

};

#endif // GENERATORE_H
