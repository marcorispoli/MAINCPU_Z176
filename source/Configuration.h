#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "application.h"


#define _CFGPATH    "/resource/config"          // Path di allocazione files di configurazione
#define _TUBEPATH   "/resource/config/Tubes"    // Path di allocazione files di configurazione
#define _SYSCFG     "sysCfg.cnf"                // Nome del file di configurazione
#define _SWCFG      "firmwares.cnf"             // file di configurazione compatibilit√  firmware
#define _BPCFG      "biopsy.cnf"                // file di configurazione biopsia

#define Mo_STR  "Mo"  // Identificatore Molibdeno
#define W_STR   "W"   // Identificatore Tungsteno
#define PAD_OPEN_STR "OPEN" // Tag speciale per la collimazione OPEN


typedef struct
{
    // Dettagli sequenza Tomo
    unsigned char numero_samples_wide; // Numero di campioni
    unsigned char n_ignored_wide;      // Numero di campioni saltati
    unsigned char numero_samples_narrow; // Numero di campioni
    unsigned char n_ignored_narrow;      // Numero di campioni saltati
}tomoCfg_Str;

typedef struct
{
    unsigned char L; // Left Blade
    unsigned char R; // Right Blade
    unsigned char F; // Front Blade
    unsigned char B; // Back blade
    unsigned char T; // Trap blade
    int           PadCode; // Codice PAD associato
}_colliPadStr;

typedef struct
{
    unsigned char           filterTomo[4];     // Angoli con incremento dinamico posizione filtro in Tomo Hotfix 11C
    unsigned char           filterPos[4];      // Posizioni filtri Rh, Al, Ag, User
    unsigned char           filterType[4];     // Associazione Materiale - posizione
    int                     mirrorSteps_ASSY_01;       // Steps posizionamento specchio in campo per ASSY 01
    int                     mirrorSteps_ASSY_02;       // Steps posizionamento specchio in campo per ASSY 02

    QList<_colliPadStr>     colli2D;           // Collimazioni 2D
    _colliPadStr            colliOpen;         // Collimazione speciale detta OPEN
    colliTomoConf_Str       colliTomoW;        // Questo blocco viene passato al driver
    colliTomoConf_Str       colliTomoMo;       // Questo blocco viene passato al driver
}colliConf_Str;


/*
 *  CODICI STRINGA PER I PARAMETRI DI CONFIGURAZIONE
 */

// Generali
#define PAR_TEST_ACCESSORIO     "[P001]"
#define PAR_SBLOCCO_ENA         "[P002]"

// Diagnosi
#define PAR_TEST_HV             "[P011]"
#define PAR_TEST_HEART          "[P012]"
#define PAR_IANODE_TEST         "[P013]"
#define PAR_TEST_MASMETRO       "[P014]"
#define PAR_TEST_FIL            "[P015]"
#define PAR_WARM_I_FIL          "[P016]"

// Speciali
#define PAR_DEMO                "[P021]"
#define PAR_TOMO_2F             "[P022]"
#define PAR_DEAD_MAN            "[P023]"
#define PAR_ROT_LIMIT           "[P024]"

// Starter
#define PAR_HS_STARTER          "[P031]"
#define PAR_STARTER_XOFF        "[P032]"
#define PAR_STARTER_BRK         "[P033]"
#define PAR_STARTER_TMO         "[P034]"



typedef struct
{

    // Gruppo parametri Starter
    unsigned char enableSblocco;    // <SBLOCCO_COMPRESSORE,0>   Abilitazione sblocco compressore
    bool starter_off_after_exposure;// <STARTER_OFF,0>   Abilitazione spegnimento starter dopo esposizione
    bool starter_brake;             // <STARTER_BRK,0>   Abilitazione all'uso del freno per fermare lo starter (solo Interno)
    int  starterTimeout;            // <STARTER_TIMEOUT,300>     Timeout in secondi per lo stop dello starter

    // Gruppo parametri generali
    bool enableCheckAccessorio;     // <ACCESSORIO_PAZIENTE,0>   Abilita / Disabilita l'uso dell'accessorio
    bool enableComboCheckAccessorio;// <ACCESSORIO_PAZIENTE_COMBO,1>   Abilita / Disabilita controllo su combo
    bool enable2DCheckAccessorio;   // <ACCESSORIO_PAZIENTE_2D,1>   Abilita / Disabilita controllo su 2D
    bool enable3DCheckAccessorio;   // <ACCESSORIO_PAZIENTE_3D,1>   Abilita / Disabilita controllo su 2D
    bool enablePbCheckAccessorio;   // <ACCESSORIO_PAZIENTE_PB,1>   Abilita / Disabilita controllo su Piombo

    // Gruppo parametri diagnostica
    bool enableTestGnd;          // <PCB190_GNDTEST_ENABLE,1> Abilitazione test presenza GND su PCB190
    int  correnteAnodicaTest;    // <PCB190_ITEST,60>         Valore atteso di corrente anodica di test
    bool enableTestMasmetro;     // <PCB190_MAS_ENA,1>        Abilitazione monitoraggio mAsmetro
    bool enableIFil;             // <PCB190_IFIL_ENA,1>       Abilitazione monitoraggio corrente di filamento
    int  correnteFilamento;      // <PCB190_IFIL,2020>        Corrente di filamento di riscaldamento
    bool enableHVMonitor;        // <PCB190_HV,1>             Abilitazione monitoraggio livelli HV
    bool demoMode;               // <DEMO,0>                  Abilitazione modalit√  demo
    bool deadman;                // <DEAD_MAN,0>              Abilitazione modalit√  dead men
    int  tempCuffiaAlr ;         // <TEMPCUFFIA_ON,51>        Attivazione allarme cuffia
    int  tempCuffiaAlrOff ;      // <TEMPCUFFIA_OFF,45>       Disattivazione allarme cuffia

    QString tubeFileName;        // Nome del file con la configurazione del Tubo utilizzato
    QString languageInterface;   // linguaggio dell'interfaccia grafica    
    QString SN;                  // Seriale macchina
    QString ServicePassword;     // Password per entrare nel menu

    bool    audioEnable;         // Abilitazione messaggi audio
    unsigned char volumeAudio;   // 0 = MAX; 10 = MIN

}userCnf_Str;

#define ANALOG_FILTRO_AUTO   1          // Filtro automatico in funzione di PLOG
#define ANALOG_FILTRO_FISSO  0          // Filtro in modalit‡ fissa

#define MAX_PLOG 160                   // max plog
#define MAX_PROFILE_CALIB_POINTS 10     // Massimo numero di punti di calibrazione


#define MAX_ANALOG_PROFILES     16      // Numero massimo di profili
#define ANALOG_TECH_MODE_MANUAL 0       // Tecnica manuale
#define ANALOG_TECH_MODE_SEMI   1       // Tecnica a un punto
#define ANALOG_TECH_MODE_AUTO   2       // Tecnica a zero punti

#define ANALOG_AECFIELD_FRONT   0       // Campi esposimetro frontale
#define ANALOG_AECFIELD_CENTER  1       // Campi esposimetro medio
#define ANALOG_AECFIELD_BACK    2       // Campi esposimetro posteriore

#define ANALOG_TECH_PROFILE_STD   0     // Profilo di annerimento
#define ANALOG_TECH_PROFILE_HC    1     // Profilo di annerimento
#define ANALOG_TECH_PROFILE_LD    2     // Profilo di annerimento
#define ANALOG_TECH_PROFILE_UNDEF 3

#define ANALOG_PLATE_FILM           0     // Tipo di supporto Film
#define ANALOG_PLATE_CR             1     // Tipo di supporto CR
#define ANALOG_PLATE_UNDEF          2     // Non definito

#define _ANALOG_AEC_PULSE_KV         29
#define _ANALOG_AEC_MAS              1



typedef struct{
    // Ultime impostazioni effettuate
    char          doseFormat;       // 'u', 'm', 's': u=uG, m= mG, s=Sievert
    unsigned char selected_filtro;  // In caso di filtro fisso, definisce quale filtro Ë attualmente selezionato
    bool          auto_filtro_mode; // [TRUE, FALSE]                      Modalit‡ automatica attiva / filtro fisso
    unsigned char aec_field;        // [0:2]                              Campo esposimetro selezionato
    unsigned char tech_mode;        // [0:2]                              Manuale/Un punto/0 punti
    unsigned char current_profile;  // [0:MAX_ANALOG_PROFILES-1]      Selezione profilo film
    int           selectedDkV;
    int           selectedDmas;

    // Configurazione filtro
    unsigned char primo_filtro;     // [FILTRO_Mo, FILTRO_Rh]             Primo filtro o filtro fisso
    unsigned char secondo_filtro;   // [FILTRO_Mo, FILTRO_Rh, FILTRO_ND]  Secondo filtro opzionale

    int           DKV_HC;           // Delta kV per profilo ad alto contrasto
    int           DKV_LD;           // Delta kV per profilo a bassa dose

    int           minKvFilm;        // Minimo valore di kV selezionabile in semi Auto per Film
    int           maxKvFilm;        // Max valore di kV selezionabile in semi Auto per Film
    int           minKvCR;          // Minimo valore di kV selezionabile in semi Auto per CR
    int           maxKvCR;          // Max valore di kV selezionabile in semi Auto per CR

    unsigned short calib_f1;        // Valore di riferimento per verifica sul campo (campo 1)
    unsigned short calib_f2;        // Valore di riferimento per verifica sul campo (campo 2)
    unsigned short calib_f3;        // Valore di riferimento per verifica sul campo (campo 3)
    unsigned char  calib_margine;   // Margine di accettabilita per la verifica dei campi

    // Gestione posizioni rotazione braccio
    int LCC;
    int LMLO;
    int LML;
    int LISO;
    int LFB;
    int LSIO;
    int LLM;
    int LLMO;

    int RCC;
    int RMLO;
    int RML;
    int RISO;
    int RFB;
    int RSIO;
    int RLM;
    int RLMO;


}analogCnf_Str;

typedef struct
{
    float IFILwarm;
    float IFILlimit;
    float IFILmax;
    unsigned short IFILdac;
}filamentCnf_Str;

typedef struct
{
    float iAnodeK; // Fattore di conversione lettura corrente
    float kvK;     // Fattore di conversione lettura tensione
    filamentCnf_Str filData;
    pcb190Conf_Str pcb190; // Dati di configurazione PCB190
}genCnf_Str;


typedef struct
{
    // Revisioni correnti
    QString rvPackage;              // Codice di revisione del Package istallato
    QString installedPackageName;   // Filename + Directory name ultima istallazione
    QString rvLanguage;             // Revisione pacchetto lingue
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

}firmwareCfg_Str;



//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

// PORTE DI PROTOCOLLO MCC
#define __MCCCORE(x,y,z) (x)
#define __MCCNODE(x,y,z) (y)
#define __MCCPORT(x,y,z) (z)

#define _MCCPORT(x) __MCCPORT(x)
#define _MCCNODE(x) __MCCNODE(x)
#define _MCCCORE(x) __MCCCORE(x)



typedef struct
{
    unsigned short configWord;
    unsigned short ID[4];
}_picConfigStr;


// Struttura dati per comandi di attivazione raggi
typedef  struct
{
    unsigned short vdac;      // V
    unsigned short idac;      // I
    unsigned short masdac;    // mas
    unsigned short rxtmo;     // tmo raggi

    unsigned char  maxV;      // MAssima tensione rilevabile
    unsigned char  minV;      // Minima tensione rilevabile
    unsigned char  maxI;      // MAssima corrente rilevabile
    unsigned char  minI;      // Minima corrente rilevabile

    bool           swa;       // Stato SWA
    bool           swb;       // Stato SWB
    bool           starterHS; // Presenza Starter alta velocit√ 
    bool           sbloccoCompressore; // Abilitazione allo sblocco compressore

    unsigned char  griglia;   // Selezione griglia
    unsigned char  samples;   // Numero di samples in caso di Tomo
    unsigned char  pre_samples;   // Numero di preimpulsi (da scartare) in caso di Tomo
    unsigned char  tomoMode; // Modalit√  calibrazione


}_SeqRaggiParam_Str;


// Codici da protocollo seriale PCB 215
#define PCB215_BATTERY_LOW      0x1
#define PCB215_BATTERY_FAULT    0x2
#define PCB215_SAFETY_FAULT     0x10



#endif // CONFIGURATION_H
