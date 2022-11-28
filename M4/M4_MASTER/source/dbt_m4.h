#ifndef _DBT_M4_H
#define _DBT_M4_H

#define REVISIONE "1.3.0"
#define REVMAJ 1
#define REVMIN 3
#define REV_BETA 0

#define M4_MASTER

/*______________________________________________________________________________


    1.0 RILASCIO ID01

    -----

    1.1: modifiche gestione Biopsia analogica
_____________________________________________________________________________*/

//////////////////////////////////////////////////////////////////////////////
//  INCLUDE FILES DI LIBRERIA
//////////////////////////////////////////////////////////////////////////////
#include <mqx.h>
#include <bsp.h> 
#include <fio.h>
#include <string.h>
#include <mutex.h>
#include <math.h>
#include <timer.h>
#include <lwgpio_vgpio.h>
#include <mcc_config.h>
#include <mcc_common.h>
#include <mcc_api.h>
#include <mcc_mqx.h>
#include <fsl_flexcan_driver.h>

// Definizione degli ID per i tasks
/* Task IDs & PRIO*/
#define MAIN_TASK                    5,12


#define GUI_INTERFACE                7,10   
#define PCB215                       8,9
#define PCB190                       10,9
#define PCB244_A                     12,9
#define BIOPSY                       13,9
#define PCB249U1                     14,9
#define PCB249U2                     15,9
#define CANTX_TASK                   16,7 // Alta priorità
#define CANRX_TASK                   17,7 // Alta priorità
#define CANACTUATORS_TASK            18,7 // Alta priorità
#define CANRXERR_TASK                19,8 // Alta priorità


#define SER422_DRIVER                20,8
#define FAULT_TASK                   21,7       // Alta priorità
#define IO_TASK                      23,7       // Alta priorità
#define BIOPSYM                      24,9      // Simulatore di biosia


#define ANALOG_RX_TASK               55,11
#define VMUSIC3_TASK                 56,11

#define SEQ_TASK                     60,11

#include "eventi.h"


//////////////////////////////////////////////////////////////////////////////
//  INCLUDE FILES APPLICAZIONE
//////////////////////////////////////////////////////////////////////////////
#include <debug_print.h>
#include <shared.h>     // Strutture e costanti condivise con progetto A5
#include <common.h>     // Strutture e costanti condivise Master/Slave
#include <canopen.h>    // CAN OPEN BUS
#include <mccClass.h>   // Comandi di libreria per gestione MCC
#include "sequences.h"  // Gestore di sequenze complesse
#include "ser422.h"     // Seriale 422 per protocollo di comunicazione Devices
#include "loader.h"     // Comandi di gestione del Loader
#include "pcb215.h"     // Gestore Carrello compressore
#include "pcb190.h"     // Gestore generatore
#include "pcb244-A.h"   // Gestore potter + esposimetro Analogico
#include "BIOPSY/biopsy.h"     // Gestore biopsia
#include "BIOPSY/biopsy_driver.h"     // Gestore biopsia
#include "BIOPSY/biopsy_simulator.h"     // Gestore biopsia
#include "pcb249U1.h"   // Gestore filtri, lame di collimazione
#include "pcb249U2.h"   // Gestore filtri, back/front, mirror,light
#include "actuators.h"  // Gestore movimentazioni
#include "gui.h"        // Gestore interfacciamento con GUI A5
#include "fault.h"      // Gestione dei faults
#include "master_can.h" // Gestione comnicazione CAN
#include "esposimetro.h"
#include "analog_rx_sequences.h"
#include "i2c.h"
#include "rtc.h"


///////////////////////////////////////////////////////////////////////////////
// INCLUDE SPECIALI APPLICAZIONE
///////////////////////////////////////////////////////////////////////////////
#ifdef ext
#undef ext
#undef extrd
#endif
#ifdef _MAIN_C_
  #define ext 
  #define extrd 
#else
  #define ext extern
  #define extrd extern const
#endif

 ext unsigned char systemMode;
#define SYSTEM_RUNNING_MODE     0
#define SYSTEM_CALIB_PCB215     1

// Struttura configurazioni di sistema

typedef struct
{
  revStr m4_master;
  revStr m4_slave;
  revStr pcb269;
  revStr pcb249U1;
  revStr pcb249U2;
  revStr pcb240;
  revStr pcb190;
  revStr pcb244;
  revStr biopsy;
}revisionStr;

typedef struct{

     // Gestione del movimento in maniera asincrona rispetto alla GUI
     bool           run;      // Comando di movimento in corso
     unsigned char  id;       // ID da restituire alla GUI
     bool           success;  // Ultimo comando eseguito con successo
     bool           completed;// Comando completato
     bool           idle;     // IDLE STATUS

     // Fault corrente
     unsigned char  faultcode;
     unsigned char  faultsubcode;

     // Dati operativi
     short cAngolo;         // Angolo corrente in centesimi di grado

     // Test di rodaggio tubo
     bool test;
     int  cicli;
     int  seq;

} _trxCommand_Str;


typedef struct{

    // Gestione del movimento in maniera asincrona rispetto alla GUI
    bool           run;      // Comando di movimento in corso
    unsigned char  id;       // ID da restituire alla GUI
    bool           success;  // Ultimo comando eseguito con successo
    bool           completed;// Comando completato
    bool           lenze_run; // Indica se il lenze è in aggiornamento automatico

    // Fault corrente
    unsigned char  faultcode;
    unsigned char  faultsubcode;

    // Dati operativi
    short dAngolo;              // Angolo corrente in decimi di grado
    short dAngolo_inclinometro; // Angolo inclinometro in decimi di grado


    // Test di rodaggio arm
    bool test;
    int  cicli;
    int  seq;

} _armCommand_Str;

typedef struct
{
    bool                gantryConfigurationReceived; // Flag di avvenuta configurazione della struttura della macchina
    systemCfg_Str       gantryCfg;       // Configurazione generale della macchina
    bool                deviceConfigured; // Tutti i dispositivi sono stati correttamente configurati

    bool                canConnected;
    bool                trxConnectedStatus;
    bool                armConnectedStatus;
    bool                lenzeConnectedStatus;
    bool                pcb240ConnectedStatus;
    bool                slaveDeviceConnected;
    bool                deviceConnected; // Flag di avvenuta configurazione e ricezione delle relative revisioni
    unsigned char       candevice_error_startup; // Errori durante la fase di startup

    trxConfig_Str       trxCfg;         // Configurazione passata da GUI
    armConfig_Str       armCfg;         // Configurazione passata da GUI
    lenzeConfig_Str     lenzeCfg;       // Configurazione passata da GUI

    _trxCommand_Str    trxExecution;    // Dati relativi a comandi in corso
    _armCommand_Str    armExecution;    // Dati relativi a comandi in corso
    unsigned char      manual_mode_activation; // Modalità di attivazione TRX / ARM da pulsanti per calibrazioni

    compressoreCfg_Str  comprCfg;       // Parametri compressore
    potterCfg_Str       potterCfg;      // Configurazione Potter
    colliConf_Str       colliCfg;       // Configurazione collimatore
    biopsyStat_Str      biopsyCfg;      // Stato + Configurazione biopsia
    pcb190Conf_Str      pcb190Cfg;      // Configurazione PCB190

    bool deviceStarted;  // Definisce lo stato di Startup dei drivers
    bool deviceConfigOk; // Dichiara che la configurazione è giunta da A5
    
    revisionStr revisioni; // Revisioni di sistema
    
    unsigned char colli_filter[5];  // Posizioni filtro come da file su Master: il quinto byte è uno status
    bool colli_filter_update; // Richiesta di calibrazione
    unsigned short mirror_position; // Posizione dello specchio


    unsigned char filterTomoEna;
    unsigned char filterTomo[3];  // Posizioni cambio filtro in Tomo. 
    
    bool        isInCompression;    // stato di compressione in corso
    
    // Loader
    bool loaderOn;      // Flag di loader attivo    
    bool demoMode;      // Se attivo impone la modalità Demo

    bool enableAudio;   // Configurazione dell'audio: l'audio è presente
    bool audioInitiated; // L'Audio è stato correttamente inizializzato e può essere utilizzato
    unsigned char volumeAudio;


    bool collimator_model_error;

    bool rtc_present;
    unsigned char weekday;
    unsigned int  year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char min;
    unsigned char sec;



}deviceCfg_Str;

ext deviceCfg_Str generalConfiguration;
ext int mcc_count;
ext void mainPrintHardwareConfig(void);

#endif
