#ifndef ANALOG_H
#define ANALOG_H

#define _W_FLT_MODE 248,236,102   // Giallo
#define _W_TECH_MODE 248,236,102   // Giallo

#define _DB_KV_STR                       _DB_SERVICE1_STR
#define _DB_MAS_STR                      _DB_SERVICE2_STR
#define _DB_PROFILE_NAME                 _DB_SERVICE3_STR
#define _DB_ANODE_NAME                   _DB_SERVICE4_STR   // Anodo associato al tubo
#define _DB_X_UDOSE                      _DB_SERVICE5_STR


#define _DB_ANALOG_FLAGS                 _DB_SERVICE1_INT
    #define _DB_ANFLG_EXP_READY          0x1                // Ready for exposure
    #define _DB_WRONG_PROJECTION         0x2                // Wrong Projection
    #define _DB_ARM_WITH_MOTOR           0x4                // Opzione rotazione motorizzata
    #define _DB_TRX_WITH_MOTOR           0x8                // Opzione pendolazione


#define _DB_CLOSE_STUDY_INT              _DB_SERVICE2_INT

#define _DB_PROFILE_INDEX                _DB_SERVICE3_INT   // Indice profilo corrente
#define _DB_PLATE_TYPE                   _DB_SERVICE4_INT   // Tipo di profilo corrente
#define _DB_TECHNIC                      _DB_SERVICE5_INT
#define _DB_OD                           _DB_SERVICE6_INT
#define _DB_TECH_MODE                    _DB_SERVICE7_INT
#define _DB_CHANGE_PANNELLO              _DB_SERVICE8_INT
#define _DB_CALLBACKS                    _DB_SERVICE9_INT
#define _DB_CURRENT_LAT                  _DB_SERVICE10_INT
#define _DB_CAMPI                        _DB_SERVICE11_INT

#define _DB_CURRENT_FUOCO                _DB_SERVICE12_INT // Fuoco correntemente impostato
#define _DB_MIN_MAX_DKV                  _DB_SERVICE13_INT // MAX H, MIN L
#define _DB_MIN_MAX_DMAS                 _DB_SERVICE14_INT // MAX H, MIN L
#define _DB_DKV                          _DB_SERVICE15_INT // KV SELECTED
#define _DB_DMAS                         _DB_SERVICE16_INT // dMAS SELECTED
#define _DB_SELECTED_FILTER              _DB_SERVICE17_INT // Current filter
#define _DB_FILTER_MODE                  _DB_SERVICE18_INT // Auto(1) Fixed(0)
#define _DB_SELECTED_PROJECTION          _DB_SERVICE19_INT
#define _DB_CURRENT_MAS_TAB              _DB_SERVICE20_INT // Imposta il pannello R20 corrente
#define _DB_NUMERO_PROFILI               _DB_SERVICE21_INT // Numero profili configurati
#define _DB_PRIMO_FILTRO                 _DB_SERVICE22_INT // Codice primo filtro
#define _DB_SECONDO_FILTRO               _DB_SERVICE23_INT // Codice secondo filtro
#define _DB_SET_PROFILE                  _DB_SERVICE24_INT // Trigger di selezione profilo
#define _DB_INFO_ALARM_MODE              _DB_SERVICE25_INT // 0= NO ALARM, 1=INFO, 2= ALARM
#define _DB_XDMAS                        _DB_SERVICE26_INT
#define _DB_XDKV                         _DB_SERVICE27_INT
#define _DB_ANOPER_STOP_ATTESA_DATI      _DB_SERVICE28_INT
#define _DB_ANALOG_MANUAL_COLLI_STAT     _DB_SERVICE29_INT
#define _DB_ANALOG_CUSTOM_COLLI_LEFT     _DB_SERVICE30_INT
#define _DB_ANALOG_CUSTOM_COLLI_RIGHT    _DB_SERVICE31_INT
#define _DB_ANALOG_CUSTOM_COLLI_FRONT    _DB_SERVICE32_INT
#define _DB_ANALOG_CUSTOM_COLLI_BACK     _DB_SERVICE33_INT
#define _DB_ANALOG_BIOPSY_WORKFLOW       _DB_SERVICE34_INT

#define _DB_ANALOG_BIOPSY_MESSAGES       _DB_SERVICE35_INT
    // CODIFICA MESSAGGI DI TIPO _DB_ANALOG_BIOPSY_MESSAGES
    #define _DB_ANALOG_BIOPSY_MESSAGES_ERROR_MISSING_COMPRESSION        1
    #define _DB_ANALOG_BIOPSY_MESSAGES_ERROR_INVALID_NEEDLE_LENGHT      2

#define _DB_ANALOG_BIOPSY_BUTTONS        _DB_SERVICE36_INT
    #define _DB_ANALOG_BIOPSY_HOME                  1
    #define _DB_ANALOG_BIOPSY_BUTTON_Xp             2
    #define _DB_ANALOG_BIOPSY_BUTTON_Xm             3
    #define _DB_ANALOG_BIOPSY_BUTTON_Yp             4
    #define _DB_ANALOG_BIOPSY_BUTTON_Ym             5
    #define _DB_ANALOG_BIOPSY_BUTTON_Zp             6
    #define _DB_ANALOG_BIOPSY_BUTTON_Zm             7
    #define _DB_ANALOG_BIOPSY_BUTTON_TRX_P15        8
    #define _DB_ANALOG_BIOPSY_BUTTON_TRX_M15        9
    #define _DB_ANALOG_BIOPSY_BUTTON_TRX_CC         10
    #define _DB_ANALOG_BIOPSY_BUTTON_TRX_EP15       11
    #define _DB_ANALOG_BIOPSY_BUTTON_TRX_EM15       12

#define _DB_ANALOG_BIOPSY_ERR_CALC        _DB_SERVICE37_INT

#define PANNELLO_PROIEZIONI             1
#define PANNELLO_COMANDI                2
#define PANNELLO_KV                     3
#define PANNELLO_MAS                    4
#define PANNELLO_OPZIONI                5
#define PANNELLO_COLLI                  6

#define PROJ_LCC    0
#define PROJ_LMLO   1
#define PROJ_LML    2
#define PROJ_LISO   3
#define PROJ_LFB    4
#define PROJ_LSIO   5
#define PROJ_LLM    6
#define PROJ_LLMO   7
#define PROJ_RCC    8
#define PROJ_RMLO   9
#define PROJ_RML    10
#define PROJ_RISO   11
#define PROJ_RFB    12
#define PROJ_RSIO   13
#define PROJ_RLM    14
#define PROJ_RLMO   15
#define PROJ_RLMO   15
#define PROJ_UNDEF  16


#define BREAST_R 0
#define BREAST_L 1



// Pannello Proiezioni

#define CALLBACK_PROJEXIT_SELECTION         2
#define CALLBACK_KVEXIT_SELECTION           3
#define CALLBACK_MASEXIT_SELECTION          4
#define CALLBACK_OPTIONEXIT_SELECTION       5
#define CALLBACK_COLLIEXIT_SELECTION        6
#define CALLBACK_BIOPSIAEXIT_SELECTION      7

#define CALLBACK_COMANDI_KV_SELECTION        15
#define CALLBACK_COMANDI_MAS_SELECTION       16
#define CALLBACK_COMANDI_PROJ_SELECTION      17
#define CALLBACK_COMANDI_CAMPI_SELECTION      18
#define CALLBACK_COMANDI_OPTION_SELECTION     19
#define CALLBACK_COMANDI_COLLI_SELECTION      20


// PARAMETRI GENERALI OPERATIVI
#define KV_PRE_FG_NO_GRID   26  // NON USATO
#define KV_PRE_FG_GRID      29
#define KV_PRE_FP_NO_GRID   30
#define KV_PRE_FP_GRID      32  // NON USATO
#define mAs_PRE             10
#define PREPULSE_TIME       10

#define KV_COLLIMATION_LARGE      35
#define mAs_COLLIMATION_LARGE     100
#define KV_COLLIMATION_SMALL      35
#define mAs_COLLIMATION_SMALL     50

#define _AN_COLLI_AUTO              0
#define _AN_COLLI_MAN_24x30         1
#define _AN_COLLI_MAN_18x24         2
#define _AN_COLLI_MAN_BIOPSY        3
#define _AN_COLLI_MAN_MAGNIFIER     4
#define _AN_COLLI_MAN_CUSTOM        5

#endif
