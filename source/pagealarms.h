#ifndef PAGEALARMS_H
#define PAGEALARMS_H

#include "application.h"


#define _PG_ALARM_BACKGROUND        "://AlarmPage/AlarmPage/background.png"


class PageAlarms : public GWindow
{
    Q_OBJECT


    // Errori relativi alla condizione di Powerdown:  _DB_ALLARMI_POWERDOWN _______________________________________________
    #define ERROR_POWER_DOWN_EMERGENCY_BUTTON   1
    #define ERROR_POWER_DOWN_MAINS              2
    #define ERROR_POWER_DOWN_TF155_FUSE         3
    #define ERROR_POWER_DOWN_WARNING_BLITERS_ON 4

    // Elenco errori per categoria: _DB_ALLARMI_BIOPSIA ____________________________________________________________________
    #define ERROR_BIOP_MOVE_XYZ                  1    // Errore raggiungimento target X
    #define ERROR_BIOP_APPLY_COMPRESSION         2    // Applicare compressione per proseguire
    #define ERROR_BIOP_MISSING_COMPRESSION       3    // La compressione è venuta a mancare
    #define ERROR_BIOP_TIMEOUT                   4    // Timeout durante il movimento
    #define ERROR_BIOP_BUSY                      5    // Comando in corso
    #define ERROR_BIOP_INVALID_REFERENCES        6    // Valori reader console acquisiti non corretti
    #define ERROR_BIOP_LESION_TOO_LOWER          7    // Margine inferiore a 4mm rispetto alla fibra di carbonio
    #define ERROR_BIOP_LESION_TOO_HIGH           8    // Lesione prossima al bordo pelle


    // Allarmi relativi al pad _DB_ALLARMI_ALR_PAD ____________________________________________________________________
    #define _ALR_NONE                      0
    #define _ALR_UNLOCK_COMPR              1          // Compressore non agganciato
    #define _ALR_UNLOCK_PAD                2          // PAD Compressore non agganciato
    #define _ALR_WRONG_PAD                 3          // PAD non riconosciuto
    #define _ALR_MIRROR_ERROR              4          // Specchio non attivo
    #define _ALR_COMPR_CLOSED_STUDY        5          // Compressione a studio aperto
    #define _ALR_UNLOCK_POTTER             6          // Potter disconnesso
    #define _ALR_PAD_SAFETY_FAULT          7          // Circuit
    #define _ALR_LAST_FIELD                8

    // ___________________      _DB_ALLARMI_ALR_COMPRESSORE __________________________________________________________________

    #define  _COMPR_SAFETY_FAULT    1

    // ___________________      _DB_ALLARMI_ALR_POTTER _______________________________________________________________________

    #define ERROR_MAG_READ 1    // Codifica ingrandimento errato
    #define ERROR_MAG_CONF 2    // Codifica ingrandimento non configurato

    // ___________________      _DB_ALLARMI_ALR_ARM ___________________________________________________________________________
    // shared.h

    // ___________________      _DB_ALLARMI_ALR_TRX ___________________________________________________________________________
    // shared.h

    // ___________________      _DB_ALLARMI_ALR_LENZE _________________________________________________________________________
    // shared.h

    // ___________________      _DB_ALLARMI_ALR_GEN ___________________________________________________________________________
     #define GEN_SET_FUOCO       1
     #define GEN_R16_FAULT       3
     #define GEN_GND_FAULT       4
     #define GEN_CALIB_HV        5
     #define GEN_HV_FAULT        6
     #define GEN_MAS_FAULT       7
     #define GEN_IFIL_FAULT      8
     #define GEN_AMPTEMP_FAULT   9
     #define GEN_STARTER_NOT_CALIBRATED 10

     // Altri errori sono definiti in shared.h

     // ___________________      _DB_ALLARMI_ALR_COLLI ________________________________________________________________________
     #define COLLI_UPDATE_FALLITO 1
     #define COLLI_FILTRO_FALLITO 2
     #define COLLI_SPECCHIO_FALLITO 3
     #define COLLI_LAMP_FALLITO   4

    // __________________________ _DB_ALLARME_XRAY_PUSH,// Diagnostica sul pulsante raggi
    #define XRAY_PUSH_TIMEOUT 1 // Pulsante raggi rimasto attivo troppo tempo

    // __________________________ _DB_ALLARME_CMP_PUSH, // Diagnostica sulla pedaliera del compressore
    #define COMPRESSOR_INVALID_PUSH 1 // Pulsante raggi rimasto attivo troppo tempo

    // __________________________ _DB_ALLARME_LIFT_PUSH,// Diagnostica sulla pedaliera del motore alto/basso
    #define LIFT_INVALID_PUSH 1 // Pulsante raggi rimasto attivo troppo tempo

    // __________________________ _DB_ALLARME_ARM_PUSH, // Diagnostica sui pulsanti di rotazione
    #define ARM_PUSH_TIMEOUT 1 // Pulsante raggi rimasto attivo troppo tempo

    // __________________________ _DB_ALLARME_INFO_STAT,
    #define INFOMSG_NOT_READY_STARTUP                       1           // Il systema non ha completato lo startup
    #define INFOMSG_NOT_READY_HV_NOT_CALIBRATED             2           // La tensione di rete non è stata calibrata
    #define INFOMSG_NOT_READY_LSSTARTER_NOT_CALIBRATED      3           // Starter a bassa velocità non calibrat
    #define INFOMSG_NOT_READY_STUDY_OPEN                    4           // Lo studio risulta aperto
    #define INFOMSG_NOT_READY_INVALID_PAD                   5           // Pad non riconosciuto
    #define INFOMSG_NOT_READY_NOT_COMPRESSED                6           // Sistem non compresso
    #define INFOMSG_NOT_READY_INVALID_POTTER                7           // Accessorio non compatibile o non riconosciuto
    #define INFOMSG_NOT_READY_MISSING_PATIENT_PROTECTION    8           // Manca la protezione paziente
    #define INFOMSG_OPERATING_PAGE_DISABLED_WITH_AWS        9           // Non si apre la pagina operativa se c'è il pc
    #define INFOMSG_NOT_READY_MISSING_CASSETTE              10          // Errore mancanza cassetta
    #define INFOMSG_NOT_READY_EXPOSED_CASSETTE              11          // Errore cassetta già esposta


    // __________________________ _DB_ALLARME_SYSCONF,                   // Configurazione di sistema
    #define ERROR_STARTUP           1   // Errore nella fase di startup
    #define ERROR_DOSE_CALCULATOR_CONFIGURATION   2   // Il calcolatore di dose non è configurato o la configurazione è corrotta
    #define ERROR_DOSE_FILTER_CALCULATOR   3  // E' selezionato un filtro senza il rispettivo file kerma
    #define COLLI_WRONG_CONFIG      4   // Collimatore non configurato
    #define ERROR_CONF_GENERATOR    5   // Il generatore non è configurato
    #define ERROR_CONF_COMPRESSOR   6   // Il compressore non è configurato
    #define ERROR_SYS_CONFIG        7   // Il sistema non è configurato
    #define ERROR_USER_CONFIG       8   // Il sistem user non è configurato
    #define ERROR_PACKAGE_CONFIG    9   // Il package non è configurato
    #define ERROR_SN_CONFIG         10   // Il SN non è stato assegnato
    #define ERROR_ANALOG_CONFIG     11  // File di configurazione analogica mancante


    // ___________________      _DB_ALLARMI_ANALOGICA __________________________________________________________________________
    #define ERROR_SETTING_DET_FIELD     1
    #define ERROR_NO_AEC_PROFILE        2

    // ___________________      _DB_ALLARMI_ALR_SOFT __________________________________________________________________________
    #define ERROR_MCC               1   // Erroe su comando MCC
    #define ERROR_REVISIONS         2   // Incompatibilita tra revisioni
    #define WARNIN_SPEGNIMENTO      3   // Spegnimento sistema in corso

    signals:
        void newAlarmSgn(int codice, QString messaggio);

    public:
        PageAlarms(QString bg ,bool showLogo,int w,int h, qreal angolo,QPainterPath pn, int pgpn, QPainterPath pp, int pgpp, int pg);
        virtual ~PageAlarms();
        void childStatusPage(bool stat,int param); // Override funzione della classe base GWindow
                                                    // Al cambio pagina riporta lo stato di attivazione

        void timerEvent(QTimerEvent* ev); // Override della classe QObject
        void mousePressEvent(QGraphicsSceneMouseEvent* event); // Override funzione della classe base GWindow

        typedef struct{
          QString   codestr;  // Codice stringa assegnato all'errore
          int       codeval;  // Codice numerico relativo all'errore
          QString   errmsg;   // Messaggio in lingua
          QPixmap   errpix;   // Pixmap associato all'errore
          QString   errdescr; // (opzionale) descrizione dell'errore
        } _alarmStruct;

        typedef struct {
            QString className;
            QString classDescription;
            QList<_alarmStruct> errlist;
        } _alarmClass;


        unsigned char refreshAlarmStatus(void);
        void setWindow(void);

        void setAlarmLabel(int classe, unsigned char codice,bool new_alr);
        void resetOneShotAlarms(void);

        _alarmStruct* setErrorWindow(int classe, int code); // Compila la pagina con la grafica relativa e restituisce la stringa errore
        bool isAlarmOn(int classe); // True se un allarme per la classe Ã¨ attivo
        bool openAlarmPage(void); // Richiede l'apertura della pagina

        _alarmStruct* getErrorInfo(int code); // Restituisce il blocco descrittivo dell'errore di codice 00XXX
        _alarmStruct* getErrorInfo(int classe, int code); // Restituisce il blocco descrittivo dell'errore CLASSE/CODICE
        QString getErrorString(int classe, int code);

        void createMessageList(void);

    public slots:
        void valueChanged(int,int); // Link esterno alla fonte dei contenuti dei campi valore
        void buttonActivationNotify(int id,bool status,int opt);
        void languageChanged(); // Link esterno alla fonte dei contenuti dei campi valore


    public:
        bool alarm_enable;  // Abilitazione generazione allarmi
        int  curClass;      // Classe correntemente visualizzata
        int  numAlarm;      // Numero di allarmi presenti
        bool newAlarm;      // Indica se si tratta di un nuovo allarme

        int timerId; // Usato per la gestione del timer della data
        int timerPg; // Usato per la gestione del timer della data

        // Testo per la DATA DI SISTEMA
        QGraphicsTextItem* dateText;

        // Testo per campo spessore
        GLabel* alarmLabel;

        // Testo per dati testuali al posto della pixmap
        GLabel* infoLabel;

        // Testo per Intestazione
        GLabel* intestazioneValue;


        // Pulsante di scorrimento allarmi
        GPush* pulsanteScorriAllarmi;
        GPush* pulsanteUpdateRevision;

        QString stringaAllarme;
        QGraphicsPixmapItem* activePix;

        static bool activateNewAlarm(int classe, int codice);
        static bool activateNewAlarm(int classe, int codice, bool self_resetting);
        static bool addNewAlarm(int classe, int codice); // Imposta l'allarme senza però aprire la finestra

        static bool debugActivateNewAlarm(int classe, int codice, bool self_resetting);
        static void reopenExistingAlarm(int classe, int codice, bool self_resetting);



        QString Format(QString code, QString stringa);
        QString FormatExcel(QString stringa,char separatore);
        void exportMessageList(void);

    private:
        QList<_alarmClass> errors;
    };

    #endif // PAGEALARMS_H
