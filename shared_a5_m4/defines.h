#ifndef _DEFINES_H
#define _DEFINES_H
  

#define __PRODUCTION_COMPILATION


//_________________________________________________________________________________
// DIRETTIVE DI COMPILAZIONE GENERALI
#ifndef __PRODUCTION_COMPILATION

//#define _PIGNONE_35_PRODU
#define _PIGNONE_40_RD

//#define __BIOPSY_SIMULATOR
//#define __ROT_WITH_COMPRESSION
//#define __NO_SLAVE_STUB
//#define  __NO_CANBUS
#define __PRINT   "192.168.30.10" // Attiva le Print di debug sulle GUI

//#define __FORCE_DIGITAL // Effettua lo stub per la PCB244-A
//#define _CANDEVICE_SIMULATION // Stub per mancanza CAN BUS su m4_slave
//#define __APPLICATION_DESKTOP_COMPILE // Compilazione per moduli TS da tavolo: rotazione 180 display, indirizzi CONSOLE a 192.168.30.10, abilitazione PRINT
//#define __CONSOLE_IP_ADDRESS 10 // MODIFICA l'IP AWS
//#define __ONLY_ONE_DET_FIELD  2
//#define __NO_AUDIO_MSG
//#define __MCCTEST_ENA
//#define __ANALOG_CALIB_SMALL_FOCUS
#else
    #define _PIGNONE_35_PRODU
#endif
//_________________________________________________________________________________


/*
 *  Questo file contiene tutte le dichiarazioni di
 *  strutture e costanti comuni tra M4 e A5
 */


//________________________________________________________________________________
//  Defines relativi all'architettura di sistema, salvata nella variabile di database
// _DB_SYSTEM_CONFIGURATION
#define _ARCH_HIGH_SPEED_STARTER    0x01
#define _ARCH_TRX_MOTOR             0x02
#define _ARCH_ARM_MOTOR             0x04

#define _COLLI_TYPE_NOT_ASSIGNED   0 // Vecchio collimatore
#define _COLLI_TYPE_ASSY_01   1 // Vecchio collimatore
#define _COLLI_TYPE_ASSY_02   2 // Nuovo collimatore

//________________________________________________________________________________


#define _INVALID_PAD_LEVEL      0
#define _UNLOCK_PAD_LEVEL       9
#define _UNLOCK_COMPR_LEVEL     10

// Livelli per Pads della famiglia Potter
#define _POTTER_24x30_LEVEL     1   // PAD 24x30
#define _POTTER_18x24_LEVEL     2   // PAD 18x24
#define _POTTER_10x24_LEVEL     3   // PAD 10x24
#define _POTTER_BP2D_LEVEL      4   // PAD BIOPSIA 2D
#define _POTTER_TOMO_LEVEL      5   // PAD TOMOGRAFIA
#define _POTTER_PROSTHESIS      6   // 10x24 per protesi
#define _POTTER_18x24L_LEVEL    7   // 18x24 shiftato Left
#define _POTTER_18x24R_LEVEL    8   // 18x24 shiftato Right

// Livelli per Pads della famiglia Ingranditore
#define _MAG_9x21_LEVEL         1 // PAD 9x21
#define _MAG_LEVEL_2            2
#define _MAG_LEVEL_3            3
#define _MAG_LEVEL_4            4
#define _MAG_9x9_LEVEL          5   // 9x9
#define _MAG_D75_LEVEL          6   // D75 A SBALZO
#define _MAG_LEVEL_7            7
#define _MAG_LEVEL_8            8

// Livelli per Pads della famiglia Biopsia *** (da definirsi)
#define _BIOP_LEVEL_1            1
#define _BIOP_LEVEL_2            2
#define _BIOP_LEVEL_3            3
#define _BIOP_3D_LEVEL           4
#define _BIOP_LEVEL_5            5
#define _BIOP_LEVEL_6            6
#define _BIOP_LEVEL_7            7
#define _BIOP_LEVEL_8            8

// CODICI IDENTIFICATIVI POTTERS
#define POTTER_2D               0 // Macchine digitali
#define POTTER_MAGNIFIER        1
#define POTTER_TOMO             2
#define POTTER_BIOP             3
#define BIOPSY_DEVICE           4
#define POTTER_UNDEFINED        5

// CODICI AGGIUNTIVI PER POTTER
#define POTTER_DESCR_24x30      0
#define POTTER_DESCR_18x24      1


// Struttura dati di configurazione Tomo
#define COLLI_ACCESSORIO_ND 0
#define COLLI_ACCESSORIO_PROTEZIONE_PAZIENTE_2D 1
#define COLLI_ACCESSORIO_PROTEZIONE_PAZIENTE_3D 2
#define COLLI_ACCESSORIO_CALIB_PLEXYGLASS 3
#define COLLI_ACCESSORIO_PIOMBO 4
#define COLLI_ACCESSORIO_FAULT 5


// Lista codici standard associati ai  PAD riconosciuti
typedef enum
{
    PAD_24x30=0,        // 24x30
    PAD_18x24,          // 18x24 Centered
    PAD_18x24_LEFT,     // 18x24 Shifted Left
    PAD_18x24_RIGHT,    // 18x24 Shifted Right
    PAD_9x21,           // 9x21  Ingranditore
    PAD_10x24,          // Pad formato 10x24
    PAD_PROSTHESIS,     // Pad per le protesi
    PAD_D75_MAG,        // Pad Sbalzato per ingranditore, diametro 75
    PAD_BIOP_2D,        // Pad per biopsia 2D
    PAD_BIOP_3D,        // Pad per biopsia con torretta
    PAD_TOMO_24x30,     // Pad per scansione Tomo 24x30
    PAD_9x9_MAG,        // Pad Sbalzato per ingrandimento

    //____________//
    PAD_ENUM_SIZE,      // Codice utilizzato per identificare il primo dei non validi
    PAD_UNLOCKED,       // Paddle non bloccato
    PAD_UNMOUNTED,      // Nacchera non bloccata
    POTTER_DISCONNECTED, // Accessorio non connesso
    PAD_ND              // Non spostare
}Pad_Enum;

// Notifiche ID speciali per il solo collimatore
#define _COLLI_ID 10


//////////////////////////////////////////////////////////////////////////////
// MODALITA' TOMO
//////////////////////////////////////////////////////////////////////////////

#define _TOMO_MODE_1F   1
#define _TOMO_MODE_2F   2
#define _TOMO_MODE_3F   3
#define _TOMO_MODE_4F   4


//////////////////////////////////////////////////////////////////////////////
// Elenco codici detectors
//////////////////////////////////////////////////////////////////////////////

#define DETECTOR_SCREENPLUS_STR     "SCREENPLUS"
#define DETECTOR_LMAM2_STR          "LMAM2"
#define DETECTOR_SOLO_STR           "SOLO"
#define DETECTOR_FDI_STR            "FDI"
#define DETECTOR_LMAMV2_STR         "LMAM2V2"
#define DETECTOR_SMAM_STR           "SMAM"
#define DETECTOR_SMAMV2_STR         "SMAMV2"
#define DETECTOR_FDIV2_STR          "FDIV2"
#define DETECTOR_SCREENPLUSV2_STR   "SCREENPLUSV2"

#define DETECTOR_SCREENPLUS     0
#define DETECTOR_LMAM2          1
#define DETECTOR_SOLO           2
#define DETECTOR_FDI            3
#define DETECTOR_LMAMV2         4
#define DETECTOR_SMAM           5
#define DETECTOR_SMAMV2         6
#define DETECTOR_FDIV2          7
#define DETECTOR_SCREENPLUSV2   8
#define DETECTOR_NUMBERS        9

// PWRMANAGEMENT_STAT
#define PWRMANAGEMENT_STAT_OK           0
#define PWRMANAGEMENT_STAT_POWERDOWN    1
#define PWRMANAGEMENT_STAT_EMERGENCY    2
#define PWRMANAGEMENT_STAT_FUSE_TF155   3
#define PWRMANAGEMENT_STAT_BLITERS_ON   4

// PWRMANAGEMENT_ACTUATOR_STATUS
#define ACTUATOR_STATUS_ENABLE_ROT_FLAG  0x1
#define ACTUATOR_STATUS_ENABLE_PEND_FLAG 0x2
#define ACTUATOR_STATUS_ENABLE_LIFT_FLAG 0x4

#define PWRMANAGEMENT_STAT              0
#define PWRMANAGEMENT_VBAT1             1
#define PWRMANAGEMENT_VBAT2             2
#define PWRMANAGEMENT_VLENZE_L          3
#define PWRMANAGEMENT_VLENZE_H          4
#define PWRMANAGEMENT_ACTUATOR_STATUS   5

#define PWRMANAGEMENT_ACTUATOR_IO       6
#define PWRMANAGEMENT_SIZE (PWRMANAGEMENT_ACTUATOR_IO + sizeof(_SystemInputs_Str) + sizeof(_SystemOutputs_Str))

#define ACT_STAT_ROTENA 0
#define ACT_STAT_PENDENA 1
#define ACT_STAT_LIFTENA 2
#define ACT_STAT_DIM 3


#define _SHOT_WITH_DETECTOR 1
#define _SHOT_WITHOUT_DETECTOR 2

#define _PAD_THRESHOLD_0  12;
#define _PAD_THRESHOLD_1  37;
#define _PAD_THRESHOLD_2  61;
#define _PAD_THRESHOLD_3  85;
#define _PAD_THRESHOLD_4  108;
#define _PAD_THRESHOLD_5  133;
#define _PAD_THRESHOLD_6  158;
#define _PAD_THRESHOLD_7  183;
#define _PAD_THRESHOLD_8  211;
#define _PAD_THRESHOLD_9  240;

//////////////////////////////////////////////////////////////////////////////
// COSTANTI RELATIVE AL COMANDO DI INVIO DATI DA AEC
//////////////////////////////////////////////////////////////////////////////
#define _AEC_2D             0 // Esposizioni 2D
#define _AEC_AE             1 // Esposizioni Alta Energia
#define _AEC_TOMO           2 // Esposizioni Tomo

//////////////////////////////////////////////////////////////////////////////
// MODALITA SEQUENZWE TOMO
//////////////////////////////////////////////////////////////////////////////
#define _TOMO_MODE_NARROW       0
#define _TOMO_MODE_INTERMEDIATE 3
#define _TOMO_MODE_WIDE         1
#define _TOMO_MODE_STATIC       2

//////////////////////////////////////////////////////////////////////////////
//                COSTANTI RELATIVE ALLE OPZIONI DI MOVIMENTO
//////////////////////////////////////////////////////////////////////////////
#define MOVE_WAIT_START       0x1  // Attende termine comando precedente
#define MOVE_WAIT_END         0x2  // Attende termine comando attivato
#define MOVE_ANG_CRITICO      0x4  // Attivazione angolo critico

#define TRX_MOVE_CC             0
#define TRX_MOVE_M15            1
#define TRX_MOVE_P15            2
#define TRX_MOVE_HOME_W         3
#define TRX_MOVE_HOME_N         4
#define TRX_MOVE_END_W          5
#define TRX_MOVE_END_N          6
#define TRX_MOVE_HOME_I         7
#define TRX_MOVE_END_I          8
#define TRX_MOVE_ANGLE          9
#define TRX_MOVE_STOP           10

// Definizione fasi di startup della macchina
#define INIT_STARTUP            0
#define PARTENZA_DRIVERS        1
#define MODULI_PARTITI          2
#define ATTESA_CONFIGURAZIONE_DISPOSITIVI       3
#define DRIVERS_OPERATIVI                       4

//__________________________________________________________________________
// Modalità di attivazione rotazioni da pulsanti manuali
#define _MANUAL_ACTIVATION_ARM_STANDARD 0
#define _MANUAL_ACTIVATION_TRX_STANDARD 1
#define _MANUAL_ACTIVATION_TRX_CALIB 2
#define _MANUAL_ACTIVATION_ARM_CALIB 3

//________________________________________________________________________
// DEFINES RTC
#define _RTC_SUN 1
#define _RTC_MON 2
#define _RTC_TUE 3
#define _RTC_WED 4
#define _RTC_THU 5
#define _RTC_FRI 6
#define _RTC_SAT 7

#define _RTC_DOM 1
#define _RTC_LUN 2
#define _RTC_MAR 3
#define _RTC_MER 4
#define _RTC_GIO 5
#define _RTC_VEN 6
#define _RTC_SAB 7


#endif
