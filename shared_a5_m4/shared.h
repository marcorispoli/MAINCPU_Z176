#ifndef _SHARED_H
#define _SHARED_H
  
#include "mcc.h"
#include "errors.h"
#include "defines.h"

// Configurazione generale della macchina
typedef struct
{
   unsigned short   spare1;            // Definisce la classe della macchina
   unsigned short   spare2;           // Codice detector type

   bool             armMotor;               // Presenza della rotazione motorizzata o del freno
   bool             trxMotor;               // Presenza Pendolazione

   bool             highSpeedStarter;       // Abilitazione presenza Starter IAE
   unsigned char    spare3;

}systemCfg_Str;

//______________________________________________________________________________________
//                  STRUTTURE DATI CONDIVISE
typedef struct
{
    unsigned short address;
    long           value;
}_objectDictionary_Str;



typedef struct
{
    unsigned short speed;           // 0.1° unit al secondo
    unsigned short accell;          // 0.1° unit al secondo al secondo
    unsigned short decell;          // 0.1° unit al secondo al secondo

}_activationParam;

typedef struct
{
    short          home_position;             // Posizione Home in decimi di grado
    short          end_position;              // Posizione di termine in valore assoluto in decimi di grado
    unsigned short speed;           // 0.1° unit al secondo
    unsigned short accell;          // 0.1° unit al secondo al secondo
    unsigned short decell;          // 0.1° unit al secondo al secondo
    unsigned short samples;         // Numero di campioni
    unsigned short pre_samples;     // Numero di campioni iniziali da scartare
    unsigned short skip_samples;    // Indica il numero di frame da saltare all'inetrno della sequenza (0 oppure 1)

}_tomoParam;

typedef struct{
    _tomoParam w;
    _tomoParam i;
    _tomoParam n;
}_tomo_profile;

typedef struct
{
    short          offset;     // Offset azzeramento (centesimi di grado)
    unsigned short speed_manual_approach; // 0.01° unit al secondo
    unsigned short speed_approach;  // 0.01° unit al secondo
    unsigned short speed_reverse;   // 0.01° unit al secondo
    unsigned short accell;          // 0.01° unit/sec al secondo

}_zerosettingParam;

typedef struct
{
     short              angolo_biopsia;// Posizione assoluta positiva per biop (decimi di grado)
     _activationParam   context2D;   // CONTEXT_2D
     _tomo_profile      tomo;
     _zerosettingParam  zero_setting;
     unsigned short     tomo_mode;  // Modalita 1F,2F,4F..
} trxConfig_Str;



#define MEM_ARM_DIR_CW 1
#define MEM_ARM_DIR_CCW 2
#define MEM_ARM_DIR_UNDEF 0
typedef struct
{
    // Rotazione Braccio
     short    angolo_rlat;    // Valore angolare corrispondente alla posizione R-LAT
     short    angolo_robl;    // Valore angolare corrispondente alla posizione R-OBL
     short    angolo_llat;    // Valore angolare corrispondente alla posizione L-LAT
     short    angolo_lobl;    // Valore angolare corrispondente alla posizione L-OBL

     unsigned short speed;    // 0.1° unit al secondo
     unsigned short accell;   // 0.1° unit al secondo al secondo
     unsigned short decell;   // 0.1° unit al secondo al secondo

     unsigned short manual_speed;    // 0.1° unit al secondo
     unsigned short manual_accell;   // 0.1° unit al secondo al secondo
     unsigned short manual_decell;   // 0.1° unit al secondo al secondo

     unsigned short direction_memory; // Memoria direzione se presente
} armConfig_Str;

typedef struct
{
    // ______  Alto Basso Braccio ________________________________________________________________
    unsigned short min_lenze_position;     // Minima posizione Lenze (% 0:1000)
    unsigned short max_lenze_position;     // Massima posizione Lenze (%0:1000)

    unsigned short manual_speed;           // Hz movimento manuale      // Setpoint 1
    unsigned short automatic_speed;        // Hz movimento automatico   // Setpoint 2

    unsigned char calibrated;              // 1 = CALIBRATED
    unsigned char spare;

} lenzeConfig_Str;




// CONFIGURAZIONI DISPOSITIVI
typedef struct
{
   int offset;          // Sbalzo rispetto alla nacchera
   unsigned char  kF;   // Compensazione forza
   unsigned char  peso; // Peso in newton
}pad_Str;

typedef struct
{
    unsigned short calibPosK;       // K linare calibrazione nacchera
    unsigned short calibPosOfs;     // Offset Calibrazione Nacchera
    pad_Str pads[PAD_ENUM_SIZE];    // Dati relativi ai compressori

    // Coefficienti per la forza
    unsigned short F0;               // Valore row forza zero calibrazione forza
    unsigned short KF0;             // Valore K forza zero calibrazione forza
    unsigned short F1;               // Valore soglia curva
    unsigned short KF1;              // Coeff. K curva operativa

    // Massima forza di compressione
    unsigned short max_compression_force;

    // Posizione nacchera
    unsigned short maxMechPosition; // MAssima posizione meccanica    
    unsigned short maxPosition;    // Massima posizione assoluta nacchera
    unsigned short maxProtection;  // Massima posizione con protezione paziente

    // Parametri ingranditore
    unsigned short sbalzoIngranditore[8];  // mm di sbalzo rispetto al piano
    unsigned short fattoreIngranditore[8]; //  X.Y, X=HB, Y=LB
    
    // Configurazione riconoscimento nacchera
    unsigned char thresholds[10];

}compressoreCnf_Str;



// Struttura dati di configurazione Tomo
#define COLLI_DYNAMIC_SAMPLES   25
typedef struct
{
    // Collimazioni dedicate alla Tomografia
    unsigned char    tomoLeftBladeP[COLLI_DYNAMIC_SAMPLES]; // POsizione left Blade 3d
    unsigned char    tomoLeftBladeN[COLLI_DYNAMIC_SAMPLES]; // POsizione left Blade 3d
    unsigned char    tomoRightBladeP[COLLI_DYNAMIC_SAMPLES]; // POsizione left Blade 3d
    unsigned char    tomoRightBladeN[COLLI_DYNAMIC_SAMPLES]; // POsizione left Blade 3d
    unsigned char    tomoBackTrapP[COLLI_DYNAMIC_SAMPLES];  // POsizione back Trap 3d
    unsigned char    tomoBackTrapN[COLLI_DYNAMIC_SAMPLES];  // POsizione back Trap 3d
    unsigned char    tomoBack;
    unsigned char    tomoFront;
    bool             enabled;           // Abiiltazione della configurazione
}colliTomoConf_Str;

typedef struct
{
    // Correnti misurate durante la fase di calibrazione
    int cal_main_run;    // Corrente MAIN RUN misurata in calibrazione
    int cal_shift_run;   // Corrente SHIFT RUN misurata in calibrazione
    int cal_main_keep;   // Corrente MAIN KEEP misurata in calibrazione
    int cal_shift_keep;  // Corrente SHIFT KEEP misurata in calibrazione
    int cal_main_off;    // Corrente MAIN OFF misurata in calibrazione
    int cal_shift_off;   // Corrente SHIFT OFF misurata in calibrazione
    int cal_main_brk;    // Corrente MAIN BRAKE misurata in calibrazione
    int cal_shift_brk;   // Corrente SHIFT BRAKE misurata in calibrazione

    // Valori massimi per MAIN e SHIFT
    int cal_max_main_off;
    int cal_max_shift_off;
    int cal_max_main_run;
    int cal_max_shift_run;
    int cal_max_main_keep;
    int cal_max_shift_keep;
    int cal_max_main_brk;
    int cal_max_shift_brk;


    // Valori minimi per MAIN e SHIFT
    int cal_min_main_run;
    int cal_min_shift_run;
    int cal_min_main_keep;
    int cal_min_shift_keep;
    int cal_min_main_brk;
    int cal_min_shift_brk;


  unsigned short IFIL_DAC_WARM;   // Impostazione per il riscaldamento
  unsigned short IFIL_MAX_SET;    // Massimo dac impostabile
  unsigned short IFIL_LIMIT;      // Massima corrente assoluta
  unsigned short HV_CONVERTION;   // Coefficiente di conversione tensione di BUS (* 1000)
  unsigned short HV_VAC;          // Tensione di rete letta in calibrazione
  unsigned short HV_VPRIMARIO;    // Tensione primario individuato durante la calibrazione


    // Calibrazione rilettura dei kV kv = RAW * (kV_CALIB/1000) + kvOFS / 1000
  unsigned short kV_CALIB;
  short kV_OFS;

  bool starter_off_after_exposure; // Spegne lo starter al termine dell'esposizione
  bool starter_off_with_brake; // Spegne lo starter (se lo spegne) con la frenatura

}pcb190Conf_Str;


// Configurazione biopsia
typedef struct
{
    // Configurazione console
    unsigned short dmm_DXReader;// DX fantoccio
    unsigned short dmm_DYReader;// DY fantoccio
    float          readerKX;    // Fattore di conversione unit/dmm
    float          readerKY;    // Fattore di conversione unit/dmm

    // Dati di calibrazione
    int  offsetX;                 // Offset di calibrazione puntamento X (dmm)
    int  offsetY;                 // Offset di calibrazione puntamento Y (dmm)
    int  offsetZ;                 // Offset di calibrazione puntamento Z (dmm)
    unsigned char  offsetFibra;   // Distanza Home torretta-Fibra di carbonio (mm)


    // Guardie anti impatto con il compressore
    unsigned char  offsetPad;               // Offset meccanico Staffe Pad (mm)
    unsigned char  margineRisalita;         // Margine di Salita compressore rispetto alla posizione del posizionatore (mm)
    unsigned char  marginePosizionamento;   // Margine di sicurezza rispetto al compressore sul posizionamento Ago (mm)

}biopsyConf_Str;


// STRUTTURA INPUT DA PCB240 
typedef struct 
{
  unsigned char bat1V; // Battery 1 voltage
  unsigned char bat2V; // Battery 2 voltage
  
  // Byte 0
  unsigned char CPU_XRAY_COMPLETED:1;
  unsigned char CPU_DETECTOR_ON:1;
  unsigned char CPU_COMPR_ENABLED:1;
  unsigned char CPU_ARM_PED_UP:1;
  unsigned char CPU_ARM_PED_DWN:1;
  unsigned char CPU_ROT_CW:1;
  unsigned char CPU_ROT_CCW:1;
  unsigned char CPU_CLOSED_DOOR:1;

  // Byte 1
  unsigned char CPU_POWER_DOWN:1;       // Power down event
  unsigned char CPU_UPS_FAULT:1;
  unsigned char CPU_MAINS_ON:1;         // Flag Bliters accesi/spenti
  unsigned char CPU_XRAY_REQ:1;
  unsigned char CPU_REQ_POWER_OFF:1;
  unsigned char CPU_BATT_DISABLED:1;
  unsigned char CPU_LIFT_DROP:1;
  unsigned char CPU_LIFT_ENABLED:1;

  // Byte 2
  unsigned char CPU_EXT_ROT_ENA:1;
  unsigned char CPU_XRAY_ENA_ACK:1;
  unsigned char CPU_LENZ_PED_FAULT:1;
  unsigned char CPU_ARM_PED_FAULT:1;
  unsigned char CPU_XRAYPUSH_FAULT:1;   // Fault segnalato dalla PCb240 allo Startup (solo durante accensione)
  unsigned char CPU_BUZZER_FEEDBACK:1;
  unsigned char B2_6:1;
  unsigned char B2_7:1;

  // Byte 3
  unsigned char B3_0:1;
  unsigned char B3_1:1;
  unsigned char B3_2:1;
  unsigned char B3_3:1;
  unsigned char B3_4:1;
  unsigned char B3_5:1;
  unsigned char B3_6:1;
  unsigned char B3_7:1;

  // Byte 4: di interesse diretto della CPU grafica
  unsigned char B4_0:1;
  unsigned char B4_1:1;
  unsigned char B4_2:1;
  unsigned char B4_3:1;
  unsigned char B4_4:1;
  unsigned char B4_5:1;
  unsigned char B4_6:1;
  unsigned char B4_7:1;
  
  // Totale = 4+2 
}_SystemInputs_Str;


// STRUTTURA OUTPUTS VERSO PCB240-3
typedef struct 
{

  // Byte 0
  unsigned char CPU_MASTER_ENA:1;
  unsigned char CPU_LIFT_ENA:1;
  unsigned char CPU_ROT_ENA:1;
  unsigned char CPU_PEND_ENA:1;
  unsigned char CPU_COMPRESSOR_ENA:1;
  unsigned char CPU_XRAY_ENA:1;
  unsigned char CPU_BURNING :1;
  unsigned char CPU_LOADER_PWR_ON:1;

  
  // Byte 1
  unsigned char CPU_LMP_SW1:1;
  unsigned char CPU_LMP_SW2:1;
  unsigned char CPU_XRAY_LED:1;
  unsigned char CPU_DEMO_ACTIVATION:1; // Ipulso di Buzzer in demo mode
  unsigned char CPU_EMERGENCY_BUTTON:1; // La CPU ha indivduato lo stato di Emergency attivo
  unsigned char CPU_DEMO_MODEL:1;
  unsigned char CPU_DEMO_MODEH:1;
  unsigned char CPU_DEMO_TOMO:1;


  // Byte 2
  unsigned char CPU_FRENO_MODE:1;
  unsigned char B2_1:1;
  unsigned char B2_2:1;
  unsigned char B2_3:1;
  unsigned char B2_4:1;
  unsigned char B2_5:1;
  unsigned char SLAVE_TERMINAL_PRESENT:1;  // Flags utiloizzati per recovery nel caso in cui un terminale non partisse
  unsigned char MASTER_TERMINAL_PRESENT:1;


}_SystemOutputs_Str;


// Struttura dati per fine sequenza raggi
typedef struct 
{
  unsigned char iAnodica;
  unsigned char spare;
  unsigned short mAs;
}_mAsData_Str;





#endif
