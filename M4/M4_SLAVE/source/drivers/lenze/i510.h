/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 1989-2008 ARC International
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale MQX RTOS License
* distributed with this Material.
* See the MQX_RTOS_LICENSE file distributed for more details.
*
* Brief License Summary:
* This software is provided in source form for you to use free of charge,
* but it is not open source software. You are allowed to use this software
* but you cannot redistribute it or derivative works of it in source form.
* The software may be used only in connection with a product containing
* a Freescale microprocessor, microcontroller, or digital signal processor.
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains the source for the rs485 example program.
*
*
*END************************************************************************/
#ifndef _i510_H_
#define _i510_H_
#include "canopen.h"

typedef struct {
    bool resetModule;       // Set this flag to reset to whole software module
    bool switch_on;         // Enable to enter the CiA402Mask_SwitchedOn status

    unsigned char operatingMode; // Hold the module operating mode (depends by the module)
    unsigned char reqCommand;    // A command has been requested
    MUTEX_STRUCT  req_mutex;     // Mutex to sync the driver thread with the req command thread

    // CiA status management
    _canopen_network_stat_t  networkStat, memNetworkStat; // Current canopen network stat
    bool            statChanged; // Set in case of status changed

    MQX_TICK_STRUCT tmo_sdo_tk; // Timeoutto be used for the SDO messages


    uint16_t analog1;           // Valore analogica 1
    uint16_t analog2;           // Valore analogica 2
    bool     manual_mode;       // Current activation mode
    bool     gestione_soglie;   // A motore fermo si risistemano le soglie e i modi di funzionamento
    uint16_t err_soglie;        // Errore relativo alle soglie

    bool     run_configuration; // Il driver è stato configurato
    bool     connected;         // CAN connection status
    bool     configured;        // lenze already configured
    bool     manual_mode_limit; // Soglia minima o massima raggiunta
    bool     upmode_enable;     // Up direction enabled
    bool     dwnmode_enable;    // Dwn direction enabled

    // Dati per invio eventi
    unsigned char event_type;    // Tipo di evento
    uint32_t      event_code;    // Codice evento
    uint32_t      event_data;    // dato associato all'evento

    uint16_t internal_errors;     // error detected by the device
    int      diagnostic_errors;  // Errori aggiuntivi

}_i510_Status_t;


bool getI510NetworkStatus(CANOPEN_CONTEXT_DEC, _i510_Status_t* pStat); // return the current internal CiA402 status
bool setI510PresetSpeed(CANOPEN_CONTEXT_DEC, unsigned char preset);
bool setI510WriteSDO(CANOPEN_CONTEXT_DEC,_OD_TEMPLATE, uint32_t data);
uint16_t getI510ErrorRegister(CANOPEN_CONTEXT_DEC);
char* i510ErrorString(unsigned short value);
int getI510BusVoltage(CANOPEN_CONTEXT_DEC);



// __________ ERROR WORD ________________________________________________________________
#define i510_603F_00_OD   0x603F, 0x00, CANOPEN_SDO_TX_2BYTE, 0  // CiA 402 status word
#define i510_2631_04_OD   0x2631, 0x04, CANOPEN_SDO_TX_1BYTE, 0  // ResetError function


// __________ STATUS WORD ________________________________________________________________
#define i510_STATUSWORD_OD   0x2308, 0x00, CANOPEN_SDO_TX_1BYTE, 0  // CiA 402 status word
#define CANOPEN_NETWORK_INIZIALIZATION_VAL      0
#define CANOPEN_NETWORK_RESETNODE_VAL           1
#define CANOPEN_NETWORK_RESETCOMM_VAL           2
#define CANOPEN_NETWORK_STOPPED_VAL             4
#define CANOPEN_NETWORK_OPERATIONAL_VAL         5
#define CANOPEN_NETWORK_PRE_OPERATIONAL_VAL     127


// __________ FREQUENCY PRESET REGISTERS AND VALUES ______________________________________
#define i510_2860_01_OD 0x2860,0x01,CANOPEN_SDO_TX_1BYTE, 0  // Freq. control input selection
    #define i510_2860_01_NETWORK    5
    #define i510_2860_01_PRESET1    11
    #define i510_2860_01_PRESET2    12
    #define i510_2860_01_PRESET3    13
    #define i510_2860_01_PRESET4    14
    #define i510_2860_01_PRESET5    15
    #define i510_2860_01_PRESET6    16
    #define i510_2860_01_PRESET7    17
    #define i510_2860_01_PRESET8    18
    #define i510_2860_01_PRESET9    19
    #define i510_2860_01_PRESET10   20
    #define i510_2860_01_PRESET11   21
    #define i510_2860_01_PRESET12   22
    #define i510_2860_01_PRESET13   23
    #define i510_2860_01_PRESET14   24
    #define i510_2860_01_PRESET15   25

// SETPOINT VALUE REGISTER
#define i510_2911_01_OD 0x2911,0x01,CANOPEN_SDO_TX_2BYTE, 0  // SETPOIN 1
#define i510_2911_02_OD 0x2911,0x02,CANOPEN_SDO_TX_2BYTE, 0  // SETPOIN 2
#define i510_2911_03_OD 0x2911,0x03,CANOPEN_SDO_TX_2BYTE, 0  // SETPOIN 3
#define i510_2911_04_OD 0x2911,0x04,CANOPEN_SDO_TX_2BYTE, 0  // SETPOIN 4
#define i510_2911_05_OD 0x2911,0x05,CANOPEN_SDO_TX_2BYTE, 0  // SETPOIN 5
#define i510_2911_06_OD 0x2911,0x06,CANOPEN_SDO_TX_2BYTE, 0  //
#define i510_2911_07_OD 0x2911,0x07,CANOPEN_SDO_TX_2BYTE, 0  //
#define i510_2911_08_OD 0x2911,0x08,CANOPEN_SDO_TX_2BYTE, 0  //
#define i510_2911_09_OD 0x2911,0x09,CANOPEN_SDO_TX_2BYTE, 0  //
#define i510_2911_10_OD 0x2911,0xA,CANOPEN_SDO_TX_2BYTE, 0  //
#define i510_2911_11_OD 0x2911,0xB,CANOPEN_SDO_TX_2BYTE, 0  //
#define i510_2911_12_OD 0x2911,0xC,CANOPEN_SDO_TX_2BYTE, 0  //
#define i510_2911_13_OD 0x2911,0xD,CANOPEN_SDO_TX_2BYTE, 0  //
#define i510_2911_14_OD 0x2911,0xE,CANOPEN_SDO_TX_2BYTE, 0  //
#define i510_2911_15_OD 0x2911,0xF,CANOPEN_SDO_TX_2BYTE, 0  // SETPOIN 15

// __________ ANALOG INPUT REGISTERS ______________________________________

#define i510_ERROR_NO_REACTION 0
#define i510_ERROR_WARNING 1
#define i510_ERROR_TROUBLE 2
#define i510_ERROR_FAULT 3

#define i510_GREATER_CODE 1
#define i510_LOWER_CODE 0


// Analog 1
#define i510_2DA4_01_OD 0x2DA4,1,CANOPEN_SDO_TX_2BYTE, 0  // Lettura valore
#define i510_2636_08_OD 0x2636,8,CANOPEN_SDO_TX_2BYTE, 0  // % soglia
#define i510_2636_09_OD 0x2636,9,CANOPEN_SDO_TX_1BYTE, 0  // 0= <,  1= >
#define i510_2636_10_OD 0x2636,10,CANOPEN_SDO_TX_1BYTE, 0  // Reazione alla soglia (0,1,2,3) ANALOG 1

// Analog 2
#define i510_2DA5_01_OD 0x2DA5,1,CANOPEN_SDO_TX_2BYTE, 0  // Lettura valore
#define i510_2637_08_OD 0x2637,8,CANOPEN_SDO_TX_2BYTE, 0  // % soglia
#define i510_2637_09_OD 0x2637,9,CANOPEN_SDO_TX_1BYTE, 0  // 0= <,  1= >
#define i510_2637_10_OD 0x2637,10,CANOPEN_SDO_TX_1BYTE, 0  // Reazione alla soglia (0,1,2,3) ANALOG 2

#define i510_60FD_00_OD 0x60FD,0,CANOPEN_SDO_TX_4BYTE, 0  // Lettura Inputs
#define i510_4016_05_OD 0x4016,5,CANOPEN_SDO_TX_1BYTE, 0  // Lettura stato DO1
#define i510_2D84_01_OD 0x2D84,1,CANOPEN_SDO_TX_2BYTE, 0  // Lettura temperatura heatsink
#define i510_282A_01_OD 0x282A,1,CANOPEN_SDO_TX_4BYTE, 0  // Lettura STO (bit 14)


//____________________________________________________________________________________________________
//                          FUNZIONI DI ATTIVAZIONE
#define i510_TRIGGER_NC   0
#define i510_TRIGGER_TRUE 1
#define i510_TRIGGER_IN1  11
#define i510_TRIGGER_IN2  12
#define i510_TRIGGER_IN3  13
#define i510_TRIGGER_IN4  14
#define i510_TRIGGER_IN5  15


#define i510_2634_02_OD 0x2634,2,CANOPEN_SDO_TX_1BYTE, 0   // Assegnazione trigger output D01

#define i510_2631_01_OD 0x2631,1,CANOPEN_SDO_TX_1BYTE, 0    // Assegnazione trigger ENABLE
#define i510_2631_02_OD 0x2631,2,CANOPEN_SDO_TX_1BYTE, 0   // Assegnazione trigger RUN
#define i510_2631_08_OD 0x2631,8,CANOPEN_SDO_TX_1BYTE, 0   // Assegnazione trigger RUN-CW
#define i510_2631_09_OD 0x2631,9,CANOPEN_SDO_TX_1BYTE, 0   // Assegnazione trigger RUN-CCW
#define i510_2631_13_OD 0x2631,13,CANOPEN_SDO_TX_1BYTE, 0  // Assegnazione trigger INVERSIONE


#define i510_2DDD_00_OD 0x2DDD,0,CANOPEN_SDO_TX_2BYTE, 0   // LETTURA FREQUENZA IN USCITA AL MOTORE

#define i510_2D87_00_OD 0x2D87,0,CANOPEN_SDO_TX_2BYTE, 0   // LETTURA DC BUS



// MANCANO ALL'APPELLO
#define i510_2631_18_OD 0x2631,18,CANOPEN_SDO_TX_1BYTE // Assegnazione trigger "INVERSIONE"ACTIVATE PRESET 0"
#define i510_2631_19_OD 0x2631,19,CANOPEN_SDO_TX_1BYTE  // Assegnazione trigger "INVERSIONE"ACTIVATE PRESET 1"

#define i510_2633_1_OD 0x2633,1,CANOPEN_SDO_TX_1BYTE  // Debounce Input 1 (ms)
#define i510_2633_2_OD 0x2633,2,CANOPEN_SDO_TX_1BYTE  // Debounce Input 2 (ms)
#define i510_2633_3_OD 0x2633,3,CANOPEN_SDO_TX_1BYTE  // Debounce Input 3 (ms)
#define i510_2633_4_OD 0x2633,4,CANOPEN_SDO_TX_1BYTE  // Debounce Input 4 (ms)
#define i510_2633_5_OD 0x2633,5,CANOPEN_SDO_TX_1BYTE  // Debounce Input 5 (ms)

#define i510_2634_01_OD 0x2634,1,CANOPEN_SDO_TX_1BYTE   // Assegnazione trigger output RELAY

#define i510_2839_04_OD 0x2839,4,CANOPEN_SDO_TX_2BYTE   // Fault: trouble counter reset time

#define i510_2860_02_OD 0x2860,0x02,CANOPEN_SDO_TX_1BYTE  // Freq. control input selection
#define i510_2910_02_OD 0x2910,0x02,CANOPEN_SDO_TX_4BYTE  // Load Inerzia

#define i510_2917_00_OD 0x2917,0x00,CANOPEN_SDO_TX_2BYTE  // Accell Time
#define i510_2918_00_OD 0x2918,0x00,CANOPEN_SDO_TX_2BYTE // Decell Time

#define i510_2C11_06_OD 0x2C11,0x06,CANOPEN_SDO_TX_2BYTE  // Stall monitoring %
#define i510_2C12_01_OD 0x2C12,0x01,CANOPEN_SDO_TX_2BYTE  // SM-low speed range Accell current %
#define i510_2C12_02_OD 0x2C12,0x02,CANOPEN_SDO_TX_2BYTE  // SM-low speed range Standstill current %


#endif
/* EOF */
