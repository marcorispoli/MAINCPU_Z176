#ifndef _DBT_M4_H
#define _DBT_M4_H

#define REVISIONE "1.0.0"
#define REVMAJ 1
#define REVMIN 0
#define REV_BETA 0

#define M4_SLAVE
/*______________________________________________________________________________


    REV 1.0 .0
    FIRMWARE PROCESSORE M4 SLAVE, PROGETTO DMD

    - Aggiunto caricamento file NANO-J
    - Aggiunto salvataggio parametri sia su ARM che su TRX
    - Aggiunto caricamento principali parametri di funzionamento del LENZE
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
#include <fsl_flexcan_driver.h>
#include <mcc_config.h>
#include <mcc_common.h>
#include <mcc_api.h>
#include <mcc_mqx.h>

// Definizione degli ID per i tasks
/* Task IDs & PRIO*/
#define MAIN_TASK                    5,12
#define GUI_INTERFACE                6,10   
#define ETH_INTERFACE                7,9   
#define CPU_TO_PCB240_DRIVER         8,7
#define FAULT_TASK                   9,7  // Alta priorità
#define INPUTS_TASK                  10,8 // Alta priorità
#define OUTPUTS_TASK                 11,8 // Alta priorità

#define TRX_TX_TASK                  12,8
#define ARM_TX_TASK                  14,8
#define LENZE_TX_TASK                16,8
#define CANRXERR_TASK                18,8
#define DEVICE_STARTUP_TASK          19,12  // Thread di lancio dei drivers

#define ACTUATORS_RX_FROM_MASTER     20,9  // Handler di ricezione comandi da Master
#define ACTUATORS_RX_FROM_DEVICES    21,9  // Handler di ricezione eventi dai devices

#include "eventi.h"

//////////////////////////////////////////////////////////////////////////////
//  INCLUDE FILES APPLICAZIONE
//////////////////////////////////////////////////////////////////////////////
#include <shared.h>     // Strutture e costanti condivise con progetto A5
#include <common.h>
#include <canopen.h>
#include <mccClass.h>
#include "ser240.h"     // Seriale 485 per protocollo di comunicazione con PCB240
#include "trx.h"        // Gestione movimento pendolazione
#include "arm.h"        // Gestione movimento braccio
#include "lenze.h"      // Gestione movimento alto/basso

#include "fault.h"      // Gestione dei faults
#include "slave_can.h"  // Gestione comnicazione CAN
#include "arm.h"
#include "lenze.h"
#include "actuators.h"  // Gestione comnicazione CAN


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

ext bool isPowerdown;
ext void show_mqx_error(int code);

typedef struct
{
    // Architettura Hardware
    bool lenzeDriver;   // Presenza driver lenze
    bool trxDriver;     // Presenza driver trx
    bool armDriver;     // Presenza driver arm
    bool deviceStarted; // Flag di driver attivati

    // Stato di connessione dei dispositivi (se presenti)
    bool lenzeConnected;
    bool trxConnected;
    bool armConnected;
    bool pcb240connected;
    bool deviceConnected;

    bool lenzeCalibPot; // Attiva il rilevamento frequente del potenziometro

}deviceCfg_Str;

ext deviceCfg_Str generalConfiguration;
ext void mainStartProcesses(void);
ext void device_startup_Task(uint32_t parameter);

#endif
