#ifndef RESOURCE_H
#define RESOURCE_H


// MACRO PER LA GESTIONE DELLA TRADUZIONE
//#define TRTEST(y) QApplication::translate("TEST",y)
//#define TRFILE(y) QApplication::translate("FILE",y)
//#define TRSTR(y) QApplication::translate("STRING",y)


/////////////////////////////////////////////////////////////////////
//          GENERALI

// COLORI IN RGBF FORMAT
#define _W_TEXT         255,255,255    // Bianco testo
#define _BK_TEXT         0,0,0   // Black testo
#define _Y_COL          255,255,153    // Giallo
#define _C_COL          176,234,239    // Azzurro
#define _DBR_COL        102,102,102    // Marrone scuro
#define _BR_COL         79,79,79       // Marrone chiaro
#define _R_COL          255,0,0        // Rosso Attivazione Comando in corso
#define _GRAY_COL       201,201,201    // Grigio sfondo
#define _GRAY_DARK_COL  79,79,79       // Grigio scuro
#define _GREEN_CUFFIA    0,128,0       // Verde temperatura cuffia
#define _ORANGE_CUFFIA   255,128,0     // Verde temperatura cuffia
#define _RED_CUFFIA      255,0,0       // Verde temperatura cuffia
#define _GREEN_COMPRESSIONE 0,255,0    // Verde indicazione compressione in corso

#define BUTTON_COL      _GRAY_DARK_COL // Colore Bottone Non attivato
#define BUTTON_SEL_COL  255,0,0        // Colore Bottone durante azionamento in corso

#define __R(r,g,b) r
#define __G(r,g,b) g
#define __B(r,g,b) b

#define _R(C) __R(C)
#define _G(C) __G(C)
#define _B(C) __B(C)

//////////// CODICI PAGINE ///////////////////////////////////////
enum
{
    _PG_NONE=0,
    _PG_STARTUP=1,
    _PG_MAIN_DIGITAL=2,
    _PG_MAIN_ANALOG=2,
    _PG_OPEN_STUDY_DIGITAL,
    _PG_OPEN_STUDY_ANALOG,
    _PG_PROJECTIONS,
    _PG_BIOPSY_DIGITAL,
    _PG_BIOPSY_ANALOG,
    _PG_SELLNG,
    _PG_ACR,
    _PG_ALARM,
    _PG_XRAY_IMG,
    _PG_TOOL_MENU,        // Pagina di selezione Tools
    _PG_ROTAZIONI_TOOL,      // Pagina tool: gestione rotazioni
    _PG_PCB190,
    _PG_COMPR_TOOL,
    _PG_SERVICE_MENU,
    _PG_SERVICE_SETUP_MENU,
    _PG_SERVICE_POWER_PANEL,
    _PG_SERVICE_TOOLS_MENU,
    _PG_SERVICE_PACKAGE_PANEL,
    _PG_SERVICE_CALIB_MENU,
    _PG_SERVICE_CALIB_HV,
    _PG_SERVICE_CALIB_POSITION,
    _PG_SERVICE_CALIB_FORCE,
    _PG_SERVICE_CALIB_ZEROSETTING,
    _PG_SERVICE_CALIB_STARTER,
    _PG_SERVICE_CALIB_FILTER,
    _PG_SERVICE_CALIB_POWER,
    _PG_SERVICE_CALIB_LENZE_POT,
    _PG_SERVICE_CALIB_CONSOLE,
    _PG_SERVICE_TOOLS_TILT,
    _PG_SERVICE_TOOLS_ARM,
    _PG_SERVICE_TOOLS_LENZE,
    _PG_SERVICE_TOOLS_STARTER,
    _PG_SERVICE_TOOLS_INVERTER,
    _PG_SERVICE_TOOLS_AUDIO,
    _PG_SERVICE_TOOLS_POTTER,
    _PG_CALIB_ANALOG,
    _PG_SYSTEM_SETUP

};



#define DATE_LABEL_POSITION 600,436

/////////////////////////////////////////////////////////////////////


#define CALIB_TOOL_ICON  "://calib_tool_icon2.png"
#define XRAY_PIX_DEMO ":/xray_demo.png"
#define XRAY_PIX_RAGGI_DEMO ":/xray_demo_raggi.png"
#define ERR_PIX  "://AlarmPage/AlarmPage/alarmPix.png"
#define INFO_PIX  "://AlarmPage/AlarmPage/infoPix.png"
#define WARN_PIX  "://AlarmPage/AlarmPage/warningPix.png"

//////////// RISORSE PAGINA 0 ///////////////////////////////////////


//////////// RISORSE PAGINA SELLNG ///////////////////////////////////////

#define _BACKGROUND_Y_PG_SELLNG  "://LanguagePage/LanguagePage/BandiereY.png"
#define _BACKGROUND_C_PG_SELLNG  "://LanguagePage/LanguagePage/BandiereC.png"

#define _PG_LNG_X_OFS 10
#define _PG_LNG_Y_OFS 10
#define _PG_LNG_SELECT_PIX        "://LanguagePage/LanguagePage/SelectFlags.png"

#define _PG_LNG_PUSH_ITA_PATH       8,56,164,226,164,225,232,59,232
#define _PG_LNG_PUSH_ITA_POS        52-_PG_LNG_X_OFS, 159-_PG_LNG_Y_OFS

#define _PG_LNG_PUSH_SPA_PATH       8,252,161,424,162,426,234,253,236
#define _PG_LNG_PUSH_SPA_POS        249-_PG_LNG_X_OFS, 159-_PG_LNG_Y_OFS

#define _PG_LNG_PUSH_ENG_PATH       8,56,255,224,255,226,324,57,324
#define _PG_LNG_PUSH_ENG_POS        52-_PG_LNG_X_OFS, 251-_PG_LNG_Y_OFS

#define _PG_LNG_PUSH_GER_PATH       8,254,255,419,256,423,324,259,326
#define _PG_LNG_PUSH_GER_POS        249-_PG_LNG_X_OFS, 251-_PG_LNG_Y_OFS

#define _PG_LNG_PUSH_POR_PATH       8,58,348,223,350,224,416,60,418
#define _PG_LNG_PUSH_POR_POS        52-_PG_LNG_X_OFS, 343-_PG_LNG_Y_OFS

#define _PG_LNG_PUSH_TUR_PATH       8,258,350,424,348,422,416,256,415
#define _PG_LNG_PUSH_TUR_POS        249-_PG_LNG_X_OFS, 343-_PG_LNG_Y_OFS

#define _PG_LNG_PUSH_RUS_PATH       8,56,438,225,439,226,508,56,510
#define _PG_LNG_PUSH_RUS_POS        52-_PG_LNG_X_OFS, 435-_PG_LNG_Y_OFS

#define _PG_LNG_PUSH_FRA_PATH       8,256,440,416,444,420,507,258,509
#define _PG_LNG_PUSH_FRA_POS        249-_PG_LNG_X_OFS, 435-_PG_LNG_Y_OFS


#define _CLOSED_STUDY_STATUS    0
#define _OPEN_STUDY_LOCAL       1
#define _OPEN_STUDY_DICOM       2
#define _OPEN_STUDY_ANALOG      3


//////////////////////////////////////////////////////////////////////////////////////////////
// INDICI CAMPI DATABASE
enum
{
_DB_SLAVE_GUI_REVISION=0,
_DB_SYSTEM_CONFIGURATION,
_DB_SERVICE_PASSWORD,
_DB_PASSWORD,
_DB_LINGUA,
_DB_COLLIMAZIONE,
_DB_COMPRESSOR_PAD,
_DB_COMPRESSOR_PAD_CODE,
_DB_ACCESSORY_NAME,
_DB_MAG_FACTOR,
_DB_MAG_OFFSET,
_DB_ACCESSORIO,
_DB_COLLI_FORMAT,//-------
_DB_FORZA,
_DB_TARGET_FORCE,
_DB_SPESSORE,
_DB_COMPRESSOR_POSITION,
_DB_DANGOLO,
_DB_TRX,
_DB_GONIO,
_DB_ROT_MODE,

_DB_ACVOLT,
_DB_VPRIMARIO,
_DB_BATTCHARGE,
_DB_AWS_CONNECTION,
_DB_PCAWS_CONNECTION,
_DB_AUDIO_PRESENT,
_DB_AUDIO_MUTE,

_DB_INTESTAZIONE,
_DB_ACR_VIEW,
_DB_ACR_SUF,
_DB_XRAY_SYM,
_DB_COMPRESSOR_UNLOCK,
_DB_CALIB_SYM,
_DB_EXPOSURE_MODE,
_DB_ENABLE_MOVIMENTI,
_DB_CONNETCTED_DRIVER,
_DB_PACKAGE_ID,
_DB_STARTUP_FASE,
_DB_IMAGE_NAME,
_DB_STUDY_STAT,        // 0 = CLOSED STUDY, 1= OPEN STUDY LOCAL, 2=OPEN_STUDY_DICOM,
_DB_PROIEZIONI,    // Lista proiezioni disponibili
_DB_SEL_PROJ,      // Proiezione selezionata
_DB_READY_EXPOSURE,    // Stato di pronto esecuzione raggi
_DB_XRAY_PUSH_BUTTON, // Stato del pulsante raggi
_DB_CLOSED_DOOR,       // Porta dello studio chiusa

// Blocco Allarmi__________________________________________________________________
_DB_NALLARMI_ATTIVI,   // Indica il numero di allarmi che sono al momento attivi
_DB_ALLARMI_POWERDOWN,
_DB_ALLARMI_BIOPSIA,
_DB_ALLARMI_ALR_PAD,
_DB_ALLARMI_ALR_COMPRESSORE,
_DB_ALLARMI_ALR_POTTER,
_DB_ALLARMI_ALR_RAGGI,
_DB_ALLARMI_ALR_ARM,
_DB_ALLARMI_ALR_TRX,
_DB_ALLARMI_ALR_LENZE,
_DB_ALLARMI_ALR_GEN,
_DB_ALLARMI_ALR_COLLI,
_DB_ALLARME_XRAY_PUSH,// Diagnostica sul pulsante raggi
_DB_ALLARME_CMP_PUSH, // Diagnostica sulla pedaliera del compressore
_DB_ALLARME_LIFT_PUSH,// Diagnostica sulla pedaliera del motore alto/basso
_DB_ALLARME_ARM_PUSH, // Diagnostica sui pulsanti di rotazione
_DB_ALLARME_INFO_STAT,// Messaggi di informazione stato
_DB_ALLARMI_SYSCONF,  // Allarmi relativi alla configurazione generale del sistema
_DB_ALLARMI_ANALOGICA, // Allarmi specifici versioni analogiche
_DB_ALLARMI_ALR_SOFT, // Questo √® l'ultimo: non spostare
// Fine Blocco allarmi_________________________________________________

_DB_FAULT_CODE_GEN,
_DB_T_CUFFIA,
_DB_HU_ANODE,

_DB_BIOP_HOLDER,
_DB_BIOP_MAX_AGO,
_DB_BIOP_MIN_AGO,
_DB_BIOP_AGO,
_DB_BIOP_MARG,
_DB_BIOP_ZLIMIT,

_DB_BIOP_X,
_DB_BIOP_Y,
_DB_BIOP_Z,

_DB_BIOP_LES_X,
_DB_BIOP_LES_Y,
_DB_BIOP_LES_Z,
_DB_BIOP_LES_ZFIBRA,

_DB_BIOP_Z_FIBRA,
_DB_BIOP_UNLOCK_BUTTON,
_DB_BIOP_CONSOLE_BUTTON,


_DB_DEMO_MODE,
_DB_DEAD_MEN,
_DB_POWER_STAT,
_DB_REQ_POWEROFF,
_DB_REVISION_ERROR_STRING,



// Variabili stringa di servizio
_DB_SERVICE1_STR,
_DB_SERVICE2_STR,
_DB_SERVICE3_STR,
_DB_SERVICE4_STR,
_DB_SERVICE5_STR,
_DB_SERVICE6_STR,
_DB_SERVICE7_STR,
_DB_SERVICE8_STR,
_DB_SERVICE9_STR,
_DB_SERVICE10_STR,
_DB_SERVICE11_STR,
_DB_SERVICE12_STR,
_DB_SERVICE13_STR,
_DB_SERVICE14_STR,
_DB_SERVICE15_STR,

_DB_SERVICE1_INT,
_DB_SERVICE2_INT,
_DB_SERVICE3_INT,
_DB_SERVICE4_INT,
_DB_SERVICE5_INT,
_DB_SERVICE6_INT,
_DB_SERVICE7_INT,
_DB_SERVICE8_INT,
_DB_SERVICE9_INT,
_DB_SERVICE10_INT,
_DB_SERVICE11_INT,
_DB_SERVICE12_INT,
_DB_SERVICE13_INT,
_DB_SERVICE14_INT,
_DB_SERVICE15_INT,
_DB_SERVICE16_INT,
_DB_SERVICE17_INT,
_DB_SERVICE18_INT,
_DB_SERVICE19_INT,
_DB_SERVICE20_INT,
_DB_SERVICE21_INT,
_DB_SERVICE22_INT,
_DB_SERVICE23_INT,
_DB_SERVICE24_INT,
_DB_SERVICE25_INT,
_DB_SERVICE26_INT,
_DB_SERVICE27_INT,
_DB_SERVICE28_INT,
_DB_SERVICE29_INT,
_DB_SERVICE30_INT,
_DB_SERVICE31_INT,
_DB_SERVICE32_INT,
_DB_SERVICE33_INT,
_DB_SERVICE34_INT,
_DB_SERVICE35_INT,
_DB_SERVICE36_INT,
_DB_SERVICE37_INT,
_DB_SERVICE38_INT,
_DB_SERVICE39_INT,
_DB_SERVICE40_INT,
_DB_SERVICE41_INT,
_DB_SERVICE42_INT,
_DB_SERVICE43_INT,
_DB_SERVICE44_INT,
_DB_SERVICE45_INT,
_DB_SERVICE46_INT,
_DB_SERVICE47_INT,
_DB_SERVICE48_INT,
_DB_SERVICE49_INT,
_DB_SERVICE50_INT,
_DB_LAST_FIELD

} ;
#define FIRST_ALR_CLASS _DB_ALLARMI_POWERDOWN
#define LAST_ALR_CLASS _DB_ALLARMI_ALR_SOFT




// MODI OPERATIVI DI ESPOSIZIONE
#define _EXPOSURE_MODE_OPERATING_MODE         0
#define _EXPOSURE_MODE_RX_SHOT_MODE           1
#define _EXPOSURE_MODE_RX_SHOT_NODET_MODE     2
#define _EXPOSURE_MODE_CALIB_MODE_KV          3
#define _EXPOSURE_MODE_CALIB_MODE_IA          4
#define _EXPOSURE_MODE_CALIB_MODE_KERMA       5
#define _EXPOSURE_MODE_CALIB_MODE_TOMO        6
#define _EXPOSURE_MODE_CALIB_MODE_COLLI_TOMO  7
#define _EXPOSURE_MODE_CALIB_MODE_TOMO_HOME   8
#define _EXPOSURE_MODE_CALIB_MODE_EXPOSIMETER 9
#define _EXPOSURE_MODE_CALIB_MODE_PROFILE     10
#define _EXPOSURE_MODE_ANALOG_MANUAL_EXPOSURE 11
#define _EXPOSURE_MODE_ANALOG_COLLIMATION_EXPOSURE 12

//////////////////////////////////////////////////////////////////////////////////////////////
// CPU FLAGS
#define CPU_C_STUDY         0x1  // Studio Y == 0, Studio C ==1
#define CPU_OPEN_STUDY      0x2  // 1 == studio aperto
#define CPU_XRAY_ON         0x4  // 1 == Raggi attivati
#define CPU_CALIB_MODE      0x8  // 1 == Modalit√  Calibrazione attivata
#define CPU_SBLOCCO_COMPR   0x10 // 1 == Sblocco compressore attivato



//////////////////////////////////////////////////////////////////////////////////////////////
// CODICI ATTIVAZIONE BRACCIO
#define _ACTIVATION_LAT_L_CODE    1
#define _ACTIVATION_OBL_L_CODE    2
#define _ACTIVATION_CC_CODE       4
#define _ACTIVATION_LAT_R_CODE    8
#define _ACTIVATION_OBL_R_CODE    0x10

// INDICI PER BOTTONI IN COMBO
#define _COMBO_INDEX_ACTIVATION   1
#define _COMBO_INDEX_ACR_SUFFIX   2
#define _COMBO_INDEX_ACR_LAT      3
#define _COMBO_INDEX_LNG          4
#define _COMBO_ACR_CC             5


// Viste su 4 bit
#define _ACR_CC_CODE    1
#define _ACR_MLO_CODE   2
#define _ACR_ML_CODE    4
#define _ACR_ISO_CODE   8
#define _ACR_FB_CODE    0x10
#define _ACR_LMO_CODE   0x20
#define _ACR_LM_CODE    0x40
#define _ACR_SIO_CODE   0x80
#define _ACR_XCCL_CODE  0x100
#define _ACR_XCCM_CODE  0x200
#define _ACR_CV_CODE    0x400
#define _ACR_AT_CODE    0x800
#define _ACR_VIEW_MASK  0xFFF

// Modificatori e Lateralit√ 
#define _ACR_R_L_CODE   0x1000 // 0 = Left

// Suffissi
#define _ACR_ID_CODE    0x1
#define _ACR_TAN_CODE   0x2
#define _ACR_S_CODE     0x4
#define _ACR_RL_CODE    0x8
#define _ACR_RM_CODE    0x10


class Resource
{
public:




    // Costruttore
    Resource();



};

#endif // RESOURCE_H
