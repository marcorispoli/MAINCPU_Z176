#ifndef _MCC_SHARED_H
#define _MCC_SHARED_H
  
//////////////////////////////////////////////////////////////////////////////
//                   INTERFACCIA MCC SHARED AREA
//////////////////////////////////////////////////////////////////////////////
#define _DEF_MCC_INPUTS_TO_APP_SLAVE           0,0,1 // Invio Inputs a App Slave
#define _DEF_MCC_IO_TO_M4_SLAVE                1,0,2 // Invio Outputs a M4 Slave
#define _DEF_MCC_GUI_TO_M4_MASTER              1,0,3 // Comandi da Master App a Master M4
#define _DEF_MCC_MASTER_TO_APP_MASTER          0,0,4 // Risposte comandi da MASTER M4  a MASTER applicazione
#define _DEF_MCC_INPUTS_TO_M4_MASTER           1,0,5 // Invio System Inputs da Master App a MASTER M4
#define _DEF_MCC_OUTPUTS_TO_APP_MASTER         0,0,6 // Invio setting Outputs da Master M4 a Master Applicazione
#define _DEF_APP_SLAVE_TO_M4_SLAVE             1,0,7 // Porta M4 SLAVE
#define _DEF_M4_SLAVE_TO_APP_SLAVE             0,0,8 // Porta GUI SLAVE

//__________________________________________________________________________________
// ESPOSIMETRO
#define _ANALOG_DETECTOR_FRONT_FIELD    0
#define _ANALOG_DETECTOR_MIDDLE_FIELD   1
#define _ANALOG_DETECTOR_BACK_FIELD     2


// _______________________________________________
// SOTTOGRUPPO COMANDI MCC_CALIB_ZERO
#define CALIB_ZERO_MANUAL_ACTIVATION_TRX_CALIB     1 // Attivazione manuale TRX lenta
#define CALIB_ZERO_MANUAL_ACTIVATION_ARM_CALIB     2 // Attivazione manuale ARM lenta
#define CALIB_ZERO_MANUAL_ACTIVATION_TRX_STANDARD  3 // Attivazione manuale TRX 2D
#define CALIB_ZERO_MANUAL_ACTIVATION_ARM_STANDARD  4 // Attivazione manuale ARM standard

#define CALIB_ZERO_ACTIVATE_TRX_ZERO_SETTING       5 // Attivazione azzeramento Tubo
#define CALIB_ZERO_ACTIVATE_ARM_ZERO_SETTING       6 // Attivazione azzeramento Arm
#define CALIB_ZERO_ACTIVATE_GONIO_ZERO_SETTING     7 // Attivazione azzeramento Gonio
// _______________________________________________

// Sottocomandi gruppo MCC_CONFIG
#define CONFIG_STATUS         1
#define CONFIG_GANTRY         2
#define CONFIG_GENERAL        3

#define CONFIG_PCB190         20
#define CONFIG_PCB269         30
#define CONFIG_PCB249U2       40
#define CONFIG_PCB244         50
#define CONFIG_BIOPSY         60
#define CONFIG_TRX            70
#define CONFIG_ARM            80
#define CONFIG_LENZE          90


#define CONFIG_PCB249U1_1     110
#define CONFIG_PCB249U1_2     111
#define CONFIG_PCB249U1_3     112
#define CONFIG_PCB249U1_4     113
#define CONFIG_PCB249U1_5     114
#define CONFIG_PCB249U1_6     115
#define CONFIG_PCB249U1_7     116

#define CONFIG_COMPLETED      255

//_____________________________________________________________________________
// Sottocomandi gruppo MCC_ACTUATOR_NOTIFY
typedef enum
{
  ACTUATOR_STATUS = 1

}_MccActuatorNotify_Code;

//_____________________________________________________________________________
// Sottocomandi gruppo LOADER
typedef enum
{
  LOADER_ACTIVATION = 1,
  LOADER_CHIP_ERASE,
  LOADER_WRITE_BLK,
  LOADER_WRITE_CONFIG,
  LOADER_WRITE_COMPLETED,
  LOADER_READ_CONFIG
}_MccLoaderNotify_Code;
//_____________________________________________________________________________

//_____________________________________________________________________________
// Comando MCC_SET_COLLI
#define COLLI_F 0   // Lama frontale
#define COLLI_B 1   // Lama posteriore
#define COLLI_L 2   // Lama Left
#define COLLI_R 3   // Lama Right
#define COLLI_T 4   // Lama Trapezio
#define COLLI_LEN 5 // Dimensione dati
//_____________________________________________________________________________

//_____________________________________________________________________________
// Comandi sottogruppo PCB244 (MCC_PCB244_COMMANDS)
#define MCC_PCB244_CMD_START_VC
#define MCC_PCB244_CMD_STOP_VC
#define MCC_PCB244_CMD_SET_FREQ
#define MCC_PCB244_CMD_SET_AMPL
#define MCC_PCB244_CMD_GET_PARAM
//_____________________________________________________________________________

//____________________________________________________
// BYTE PER COMANDO DI NOTIFICA COMPRESSORE
#define COMPRESSORE_FLAG0   0
#define COMPRESSORE_FLAG1   1
#define COMPRESSORE_FORZA   2
#define COMPRESSORE_THICKL  3
#define COMPRESSORE_THICKH  4
#define COMPRESSORE_PAD     5
#define COMPRESSORE_TARGET  6
#define COMPRESSORE_POSL     7
#define COMPRESSORE_POSH     8
#define PCB215_NOTIFY_COMPR_DATA_LEN 9
//_____________________________________________________________________________
// Codici per notifiche PCB215
typedef enum
{
  PCB215_NOTIFY_COMPR_DATA=0,

  PCB215_NOTIFY_CALIB_DATA,     // buffer[0]: SYS_FLAGS,
                                // buffer[1]: SYS_FLAGS1,
                                // buffer[2:3]: RAW PADDLE,
                                // buffer[4]: RAW STRENGHT,

  PCB215_NOTIFY_ERRORS          // buffer[0]: codice errore

}_MccPCB215Notify_Code;
//_____________________________________________________________________________


// PCB215_NOTIFY_ERRORS
    #define PCB215_NO_ERRORS            0
    #define PCB215_ERROR_PEDALS_STARTUP 1
//_____________________________________________________________________________
// Codici posizionali per messaggio dati di fine esposizione
#define RX_END_CODE     0
#define MAS_PRE_L       1
#define MAS_PRE_H       2
#define MAS_PLS_L       3
#define MAS_PLS_H       4
#define I_MEAN          5
#define V_MEAN          6
#define V_SIGMA         7
#define T_MEAN_PRE_L    8
#define T_MEAN_PRE_H    9
#define T_MEAN_PLS_L    10
#define T_MEAN_PLS_H    11
#define HV_POST_RAGGI   12
#define IFIL_POST_RAGGI 13
#define NSAMPLES_AEC    14
#define NSAMPLES_PLS    15
#define SAMPLES_BUFFER  16
#define RX_DATA_LEN     17 // Buffer dati ....



// Codici per notifiche PCB190
#define PCB190_FLAGS0           0
#define PCB190_DGN_HV_IDLE      1
#define PCB190_DGN_VTEMP        2
#define PCB190_I_FIL            3
#define PCB190_I_ANODICA        4
#define PCB190_MAS_TEST_L       5
#define PCB190_MAS_TEST_H       6
#define PCB190_V32              7
#define PCB190_VM32             8
#define PCB190_V12              9
#define PCB190_VM12             10
#define PCB190_V15              11
#define PCB190_V15EXT           12
#define PCB190_FAULTS           13
#define PCB190_ARLS             14
#define PCB190_ARHS             15
#define PCB190_DGN_LEN          16

typedef enum
{
  PCB190_NOTIFY_DGN=0

}_MccPCB190Notify_Code;


// Comandi tra SLAVE GUI e SLAVE M4 tramite MCC
typedef enum
{
    MCC_SLAVE_NOTIFY=0, // Notifica da M4 SLAVE
    MCC_SLAVE_GET_REV   // Richiesta revisione

}slaveToSlaveMccEnum;

// Codici per notifiche PCB249U1
typedef enum
{
  PCB249U1_NOTIFY_DATA=0        // buffer[0]: SYSTEM FLAGS0,
                                // buffer[1]: TEMPERATURA CUFFIA,

}_MccPCB249U1Notify_Code;

// _____________________________________________________________________
// Codici per notifiche Biopsia
// Contenuto notifica m4-A5 biopsia
#define _BP_CONNESSIONE     0 // Stato della connessione
#define _BP_MOTION          1 // Stato dell'attivazione in corso
#define _BP_MOTION_END      2 // Risultato del movimento appena terminato
#define _BP_PUSH_SBLOCCO    3 // Stato del pulsante di sblocco
#define _BP_ADAPTER_ID      4 // Codice holder riconosciuto

// Posizione torretta in dmm
#define _BP_XL              5
#define _BP_XH              6
#define _BP_YL              7
#define _BP_YH              8
#define _BP_ZL              9
#define _BP_ZH              10

#define _BP_MAX_Z           11 // Massima escursione di Z prima dell'impatto con il compressore
#define _BP_ZLIMIT          12 // Valore massimo possibile di Z calcolato sullla base di MAX_Z e lunghezza ago

// Posizione lesione in dmm
#define _BP_JXL             13
#define _BP_JXH             14
#define _BP_JYL             15
#define _BP_JYH             16
#define _BP_17              17
#define _BP_18              18

// Stato pulsanti console biopsia
#define _BP_CONSOLE_PUSH    19

// Dati per la revisione e checksum
#define _BP_CHKH            20
#define _BP_CHKL            21
#define _BP_REVIS           22
//______________________________
#define _BP_DATA_LEN        23
//_____________________________

// Codici relativi al tipo di adapter ID
#define _BP_ADAPTER_OPEN            0
#define _BP_ADAPTER_NEEDLE          1
#define _BP_ADAPTER_A               2
#define _BP_ADAPTER_B               3
#define _BP_ADAPTER_SHORT           4


// Codici relativi allo stato della connessione
#define _BP_CONNESSIONE_DISCONNECTED            0
#define _BP_CONNESSIONE_CONNECTED               1

// Codici relativi allo stato del movimento
#define _BP_NO_MOTION                     0
#define _BP_MOTION_ON                     1
#define _BP_MOTION_TERMINATED             2

// Codici risultato movimento
#define _BP_TIMEOUT_COMANDO     1
#define _BP_ERROR_POSITIONINIG  2
#define _BP_POSITIONINIG_OK     3

// Codici pulsante di sblocco
#define _BP_PUSH_SBLOCCO_DISATTIVO           0
#define _BP_PUSH_SBLOCCO_ATTIVO              1

// Codici pulsanti della console di biopsia
#define _BP_BIOP_PUSH_NO_EVENT  0
#define _BP_BIOP_PUSH_RESET     0x1
#define _BP_BIOP_PUSH_AGO_10    0x2
#define _BP_BIOP_PUSH_AGO_1     0x4
#define _BP_BIOP_PUSH_SEQ       0x8
#define _BP_BIOP_PUSH_BACK      0x10


// __________________________________________________________
// CODICI COMANDO BIOPSIA (DA GUI A M4): MCC_BIOPSY_CMD
#define _MCC_BIOPSY_CMD_MOVE_HOME   1
#define _MCC_BIOPSY_CMD_MOVE_XYZ    2
#define _MCC_BIOPSY_CMD_MOVE_INCX   3
#define _MCC_BIOPSY_CMD_MOVE_DECX   4
#define _MCC_BIOPSY_CMD_MOVE_INCY   5
#define _MCC_BIOPSY_CMD_MOVE_DECY   6
#define _MCC_BIOPSY_CMD_MOVE_INCZ   7
#define _MCC_BIOPSY_CMD_MOVE_DECZ   8
#define _MCC_BIOPSY_CMD_SET_STEPVAL 9
#define _MCC_BIOPSY_CMD_SET_LAGO    10
#define _MCC_BIOPSY_CMD_RESET_BYM   11




//________________________________________________________________________
typedef enum
{
  BIOP_NOTIFY_STAT=0,           // buffer[0]: 0=NULLA, 1 = ->CONNESSO, 2->NON CONNESSO
                                // buffer[1]: 0=NULLA, 1 = ->SBLOCCO ON, 2-> SBLOCCO OFF
                                // buffer[2:3], Posizione corrente Z (se buffer[4]==4)
                                // buffer[4]: 1= muove X, 2=muoveY, 3=muoveZ 4=fine movimenti
  BIOP_NOTIFY_MOVE_CMD          // <TBD>
}_MccBiopNotify_Code;

typedef struct
{
  unsigned char cmd;
  unsigned char buffer[100];     // Buffer con i dati dalla GUI
}_MccGuiToDevice_Str;

typedef enum
{
    SRV_FREEZE_DEVICE=0,          // Freeze dei devices
    SRV_SERIAL_SEND,              // Invio comandi seriali
    SRV_RODAGGIO_TUBO,            // Attivazione rodaggio del tubo
    SRV_ARM_STOP,                 // Fermo di qualsiasi movimento in corso
    SRV_START_POTTER_2D_GRID,     // Attivazione Griglia 2D
    SRV_STOP_POTTER_2D_GRID,      // Stop Griglia 2D
    SRV_RESET_PCB244,             // Reset scheda PCB244
    SRV_RESET_FAULT_PCB244,        // Reset Fault PCB244
    SRV_TEST_LS_STARTER            // RIchiesta attivazione test starter bassa velocità

}_MccServiceNotify_Code;

typedef enum
{
    MCC_PCB244_A_GET_REV = 0,   // Restituisce la revisione corrente
    MCC_PCB244_A_GET_RADx1,     // Restituisce il campionamento x1
    MCC_PCB244_A_GET_RADx5,     // Restituisce il campionamento x5
    MCC_PCB244_A_GET_RADx25,    // Restituisce il campionamento x25
    MCC_PCB244_A_SET_OFFSET,    // Imposta un dato valore di offset
    MCC_PCB244_A_SET_AUTO_OFFSET,// Imposta modalità auto offset
    MCC_PCB244_A_ACTIVATE_GRID, // Attiva la griglia per un certo numero di passate
    MCC_PCB244_A_GET_CASSETTE,  // Legge la presenza/stato della cassetta
    MCC_PCB244_A_GET_ID,        // Legge la codifica dell'accessorio
    MCC_PCB244_A_SET_FIELD,     // Imposta il campo corrente
    MCC_PCB244_A_RESET_BOARD,   // Effettua il reset della scheda
    MCC_PCB244_A_RX_ABORT,      // Forza l'uscita da una sequenza
    MCC_PCB244_A_SET_CASSETTE   // Forza lo stato d'uso della cassetta


}_MccPCB244A_Code;


// Comandi tra MASTER GUI e MASTER M4 tramite MCC
typedef enum
{
    MCC_CONFIG = 1,       // Gruppo Configurazione dispositivi
    MCC_SERVICE,          // Comandi specifici per porta di servizio
    MCC_LOADER,           // Comandi riservati al Loader

    // MESSAGGI SERIALI
    MCC_SERIAL_READ,      // Richiesta di lettura
    MCC_SERIAL_WRITE,     // Richiesta di scrittura
    MCC_SERIAL_CMD,       // Richiesta di invio comando
    MCC_SERIAL_CMDSPC,    // Richiesta di invio comando speciale

    // PCB240
    MCC_SET240_CMD,       // Richiesta di invio comando a PCB240
    MCC_SET_OUTPUTS,      // Richiesta di scrittura Outputs
    MCC_GET_IO,           // Richiesta stato Outputs

    // PCB-269
    MCC_CMD_SBLOCCO,      // Richiede una sequenza di sblocco compressore
    MCC_CMD_PAD_UP,       // Richiede una sequenza di attivazione carrello compressore UP
    MCC_CMD_PCB215_CALIB, // Richiede attivazione modo calibrazione compressore
    MCC_CMD_CMP_STOP,     // Ferma motore del compressore
    MCC_CMD_CMP_AUTO,     // Manda in com pressione il compressore
    MCC_GET_TROLLEY,      // Richiede la posizione del compressore anche se non in compressione
    MCC_NOTIFY_COMPR,     // NOTIFICHE dal compressore

    // Rotazioni motorizzate
    MCC_CMD_ARM,          // Comando completo di movimentazione Braccio
    MCC_CMD_TRX,          // Comando completo di movimentazione Tubo
    MCC_ARM_ERRORS,       // Comunicazione errori da ARM
    MCC_TRX_ERRORS,       // Comunicazione errori da TRX
    MCC_LENZE_ERRORS,     // Comunicazione errori da LENZE

    // PCB190
    //MCC_CMD_RAGGI_STD,    // Richiesta attivazione sequenza raggi (MASTER M4)
    //MCC_CMD_RAGGI_AEC,    // Richiesta attivazione sequenza raggi AEC
    //MCC_CMD_EXP_AEC,      // Aggiornamento con dati di esposizione
    //MCC_TEST_RX_SHOT,     // Attiva una sequenza manuale raggi standard senza detector
    MCC_STARTER,          // GEstione attivazione Starter
    MCC_SET_FUOCO,        // Impostazione Fuoco

    // PCB244 - POTTER
    MCC_POTTER_ID,
    MCC_244_A_DETECTOR_FIELD, // Impostazione campo esposimetro


    // PCB249U1 - COLLIMAZIONE LAME LATERIALI e BACK TRAP
    MCC_RESET_GONIO,      // Effettua il reset degli inclinometri
    MCC_SET_COLLI,        // Effettua il setting della collimazione
    MCC_SET_MIRROR,       // Imposta lo stato corrente dello specchi
    MCC_UPDATE_MIRROR_LAMP,    // Imposta il valore degli Step dello specchio e mette in campo con luce centratore
    MCC_SET_LAMP,         // Imposta lo stato corrente della luce di collimazione
    MCC_SET_FILTRO,       // Imposta il filtro
    MCC_CALIB_FILTRO,     // Calibrazione filtro

    // GENERALI
    MCC_GET_GONIO,       // Richiede i registri relativi agli angoli(*)
    MCC_RAGGI_DATA,      // Messaggio di notifica dati di esposizione

    // Comandi di impostazione configurazioni
    MCC_SET_ROT_TOOL_CONFIG,    // Configurazione del tool di gesitone rotazioni

    // BIOPSY
    MCC_BIOPSY_DEMO_CMD,     // Comando di attivazione/Disattivazione demo
    MCC_BIOPSY_CMD,          // Comando verso Biopsia
    MCC_BIOPSY_SIMULATOR,    // Cpomandi per il simulatore se compilato

    MCC_GUI_NOTIFY,          // Notifica per l'applicazione da GUI M4
    MCC_CONFIG_NOTIFY,       // Notifica dal configuratore
    MCC_PCB215_NOTIFY,       // Notifica ad A5 da parte di PCB215

    MCC_PCB249U1_NOTIFY,     // Notifica ad A5 da parte di PCB249U1

    MCC_SERVICE_NOTIFY,      // Notifica ad A5 da parte di Service
    MCC_LOADER_NOTIFY,       // Notifica ad A5 da parte di Loader
    MCC_BIOP_NOTIFY,         // Notifica da Biopsia
    MCC_PCB190_NOTIFY,       // Notifiche da driver della scheda PCB190

    // Sezione dedicata ai test
    MCC_TEST,

    MCC_CANOPEN,
    MCC_POWER_OFF,
    MCC_POWER_MANAGEMENT,
    MCC_SLAVE_ERRORS,
    MCC_CALIB_ZERO,          // Comandi e notifiche (MASTER) per calibrazione azzeramenti
    MCC_CALIB_LENZE,          // Slave: gestione calibrazione potenziometro lenze
    MCC_GET_TRX_INPUTS,
    MCC_GET_ARM_INPUTS,
    MCC_GET_LENZE_INPUTS,

    // Sezione dedicata alla versione analogica
    MCC_XRAY_ANALOG_MANUAL,
    MCC_XRAY_ANALOG_AUTO,
    MCC_XRAY_ANALOG_CALIB_PRE,          // Calibrazione pre-impulso
    MCC_XRAY_ANALOG_CALIB_PROFILE,      // Calibrazione pre-impulso
    MCC_XRAY_ANALOG_CALIB_TUBE,         // Calibrazione kV e corrente anodica
    MCC_XRAY_ANALOG_REQ_AEC_PULSE,      // il driver richiede i dati per l'impulso

    // Messaggi Audio
    MCC_AUDIO,
    MCC_244_A_FUNCTIONS, // Sezione di comandi deicati all'esposimetro

    // RTC
    MCC_RTC_COMMANDS    // Sezione dedicata ai comandi RTC

}_MccGuiToDevice_Cmd;


#endif
