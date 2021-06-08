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
#ifndef _PD4_H_
#define _PD4_H_
#include "canopen.h"

typedef struct {
    bool resetModule;       // Set this flag to reset to whole software module
    bool switch_on;         // Enable to enter the CiA402Mask_SwitchedOn status
    bool zeroSettingOK;     // The zero setting has been successfully completed
    bool positionOk;        // Last target position ok
    bool init_module;       // Indica che il modulo è in fase di inizializzazione
    bool can_error;         // Se questo errore è attivo, in operativo, deve essere tolto l'enable all'alimentazione
                            // per evitare che eventualidanni al cavo can durante i movimenti possa portare a perdita di controllo
                            // dell'azionamento
    bool fatal_error;       // Il driver non deve essere ripristinato

    unsigned char operatingMode; // Hold the module operating mode (depends by the module)
    unsigned short activation_timeout; // TImeout durante il movimento. Da impostare all'inizio del movimento
    uint32_t       statusword;          // Status word durante il movimento



    uint32_t errors;                // last error detected;
    unsigned char reqCommand;       // A command has been requested
    uint32_t      cmdData;          // Data of the current command
    MUTEX_STRUCT  req_mutex;        // Mutex to sync the driver thread with the req command thread

    // CiA status management
    _CiA402_stat_t  Cia402Stat, memCia402Stat; // Current CiA402 status
    bool            statChanged; // Set in case of status changed
    bool            connected;   // CAN connection status

    MQX_TICK_STRUCT tmo_sdo_tk; // Timeoutto be used for the SDO messages

    uint8_t profileIndex;       // Index of the current motor profile used

    // Configurazione dispositivo
    bool    configured;

    bool    quickstop;          // Richiesta quickstop

    // Dati per invio eventi
    unsigned char event_type;    // Tipo di evento
    uint32_t      event_code;    // Codice evento
    uint32_t      event_data;    // dato associato all'evento


    short         dAngolo;       // Posizione corrente dell'encoder
    uint32_t      position_target; // Target richiesto per il posizionamento in device units

    uint32_t      inputs;
    unsigned char diagnostic_errors;

}_PD4_Status_t;



bool getPd4CiA402Status(CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat); // return the current internal CiA402 status
bool Pd4CiA402ShutdownCommand(CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat);
bool Pd4CiA402SwitchOnCommand(CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat);
bool Pd4CiA402DisableVoltageCommand(CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat);
bool Pd4CiA402EnableOperationCommand(uint32_t ctrlmask, uint32_t ctrlval, CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat);
bool Pd4CiA402DisableOperationCommand(uint32_t ctrlmask, uint32_t ctrlval, CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat);

bool Pd4CiA402SetControlOD(uint32_t mask, uint32_t val, CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat);
bool Pd4CiA402SetOperatingOD(uint8_t val, CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat);
bool Pd4CiA402FaultReset(CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat);
bool Pd4CiA402QuickStop(CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat);

bool uploadOjectDictionary(unsigned short ID, const char* DEVICE, const _canopen_ObjectDictionary_t* pDictionary,CANOPEN_CONTEXT_DEC); // Upload e salvataggio se necessario in flash
bool caricamentoNanojProgram(bool forceFlash, const char* DEVICE, const unsigned char* vmmFile, uint32_t vmmSize, CANOPEN_CONTEXT_DEC);

//_______________________ ZERO SETTING OBJECTS ________________________________________


#define ZERO_SETTING_CTRL_INIT  0x0270,0x0000                   // OMS BITS RESET
#define ZERO_SETTING_START      0x0270,0x0010                   // OMS BIT4 = ON
#define ZERO_SETTING_STOP       0x0270,0x0010                   // OMS BIT4 = OFF

#define ZERO_STAT_PERFORMED     0x3400,0x0000
#define ZERO_STAT_INTERRUPTED   0x3400,0x0400
#define ZERO_STAT_CONFIRMED     0x3400,0x1000
#define ZERO_STAT_COMPLETED     0x3400,0x1400
#define ZERO_STAT_ERROR_RUN     0x3400,0x2000
#define ZERO_STAT_ERROR_IDL     0x3400,0x2400


//_______________________ POSITIONING SETTING OBJECTS ________________________________________
#define ZERO_SETTING_CTRL_INIT  0x0270,0x0000                   // OMS BITS RESET
#define ZERO_SETTING_START      0x0270,0x0010                   // OMS BIT4 = ON
#define ZERO_SETTING_STOP       0x0270,0x0010                   // OMS BIT4 = OFF

#define ZERO_STAT_PERFORMED     0x3400,0x0000
#define ZERO_STAT_INTERRUPTED   0x3400,0x0400
#define ZERO_STAT_CONFIRMED     0x3400,0x1000
#define ZERO_STAT_COMPLETED     0x3400,0x1400
#define ZERO_STAT_ERROR_RUN     0x3400,0x2000
#define ZERO_STAT_ERROR_IDL     0x3400,0x2400

#define OD_1001_00 0x1001, 0x00, CANOPEN_SDO_TX_1BYTE  // ERROR REGISTER
#define OD_1003_01 0x1003, 0x01, CANOPEN_SDO_TX_4BYTE  // SPECIFIC ERROR REGISTER



//_______________ STORE PARAMETER REGISTER ________________________________________________
#define OD_1010_01 0x1010, 0x01, CANOPEN_SDO_TX_4BYTE  // Save All Parameters To Non-volatile Memory
#define OD_1010_02 0x1010, 0x02, CANOPEN_SDO_TX_4BYTE  // Save Communication Parameters To Non-volatile Memory
#define OD_1010_03 0x1010, 0x03, CANOPEN_SDO_TX_4BYTE  // Save Application Parameters To Non-volatile Memory
#define OD_1010_04 0x1010, 0x04, CANOPEN_SDO_TX_4BYTE  // Save Customer Parameters To Non-volatile Memory
#define OD_1010_05 0x1010, 0x05, CANOPEN_SDO_TX_4BYTE  // Save Drive Parameters To Non-volatile Memory
#define OD_1010_06 0x1010, 0x06, CANOPEN_SDO_TX_4BYTE  // Save Tuning Parameters To Non-volatile Memory
#define OD_SAVE_CODE 0x65766173
#define OD_SAVE_ENABLED 0x1
#define OD_SAVE_DISABLED 0

// User Internal data
#define OD_2700_01 0x2700, 0x01 ,CANOPEN_SDO_TX_1BYTE   // User data control word: SET to 1 to save User content. reset autom. to 0 when finished
#define OD_2700_02 0x2700, 0x02 ,CANOPEN_SDO_TX_2BYTE   // USER #0 - RESERVED FOR THE ID CONFIGURATION CODE
#define OD_2700_03 0x2700, 0x03 ,CANOPEN_SDO_TX_2BYTE   // USER #1
#define OD_2700_04 0x2700, 0x04 ,CANOPEN_SDO_TX_2BYTE   // USER #2
#define OD_2700_05 0x2700, 0x05 ,CANOPEN_SDO_TX_2BYTE   // USER #3
#define OD_2700_06 0x2700, 0x06 ,CANOPEN_SDO_TX_2BYTE   // USER #4
#define OD_2700_07 0x2700, 0x07 ,CANOPEN_SDO_TX_2BYTE   // USER #5
#define OD_2700_08 0x2700, 0x08 ,CANOPEN_SDO_TX_2BYTE   // USER #6
#define OD_2700_09 0x2700, 0x09 ,CANOPEN_SDO_TX_2BYTE   // USER #7

//_______________ NANOJ ________________________________________________
#define OD_1F51_02 0x1F51, 0x02 ,CANOPEN_SDO_TX_1BYTE   // VMM FLASH CONTROL WORD:
#define VMM_DELETE 0x10
#define VMM_INIT 0x11
#define VMM_WRITE 0x12

#define OD_1F50_02 0x1F50, 0x02 ,CANOPEN_SDO_TX_4BYTE   // VMM FLASH DATA CONTENT:
#define OD_1F57_02 0x1F57, 0x02 ,CANOPEN_SDO_TX_4BYTE   // VMM FLASH STATUS:


//___________ OPERATING MODE PROFILE ______________________________________________________
#define OD_6060_00 0x6060, 0x00, CANOPEN_SDO_TX_1BYTE  // OPERATING MODE CONROL WORD
    #define PD4_NO_PROFILE                  0
    #define PD4_PROFILE_HOMING              6
    #define PD4_PROFILE_POSITIONING         1

// __________ CONTROL WORD ________________________________________________________________
#define OD_6040_00 0x6040, 0x00, CANOPEN_SDO_TX_2BYTE  // CiA402 Control Word
    #define PD4_SHUTDOWN                    0x0087, 0x0006
    #define PD4_QUICKSTOP                   0x0087, 0x0002
    #define PD4_SWITCHON                    0x008F, 0x0007
    #define PD4_DISVOLTAGE                  0x0082, 0x0000
    #define PD4_ENABLEOP                    0x008F, 0x000F
    #define PD4_DISABLEOP                   0x008F, 0x0007
    #define PD4_RESET_OMS                   0x0270, 0x0000


// __________ STATUS WORD ________________________________________________________________
#define OD_6041_00 0x6041, 0x00, CANOPEN_SDO_TX_2BYTE  // CiA 402 status word
    #define CiA402Mask_NotReadyToSwitchOn   0x004F,0x0000
    #define CiA402Mask_SwitchOnDisabled     0x004F,0x0040
    #define CiA402Mask_ReadyToSwitchOn      0x006F,0x0021
    #define CiA402Mask_SwitchedOn           0x006F,0x0023
    #define CiA402Mask_OperationEnabled     0x006F,0x0027
    #define CiA402Mask_QuickStopActive      0x006F,0x0007
    #define CiA402Mask_FaultReactionActive  0x004F,0x000F
    #define CiA402Mask_Fault                0x004F,0x0008


// NMT Behavior in case of fault
#define OD_1029_01  0x1029, 0x01 ,CANOPEN_SDO_TX_1BYTE
#define OD_1029_02  0x1029, 0x02 ,CANOPEN_SDO_TX_1BYTE


#define OD_2031_00  0x2031, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Maximum Current
#define OD_2032_00  0x2032, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Maximum Speed
#define OD_2033_00  0x2033, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Plunger Block

#define OD_2034_00  0x2034, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Upper Voltage Warning Level
#define OD_2035_00  0x2035, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Lower Voltage Warning Level

#define OD_2036_00  0x2036, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Open Loop Current Reduction Idle Time
#define OD_2037_00  0x2037, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Open Loop Current Reduction Value/factor


// I2t Parameters
#define OD_203B_01  0x203B, 0x01 ,CANOPEN_SDO_TX_4BYTE  // Nominal Current
#define OD_203B_02  0x203B, 0x02 ,CANOPEN_SDO_TX_4BYTE  // Maximum Duration Of Peak Current
#define OD_203B_03  0x203B, 0x03 ,CANOPEN_SDO_TX_4BYTE  // Threshold
#define OD_203B_04  0x203B, 0x04 ,CANOPEN_SDO_TX_4BYTE  // CalcValue
#define OD_203B_05  0x203B, 0x05 ,CANOPEN_SDO_TX_4BYTE  // LimitedCurrent
#define OD_2056_00  0x2056, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Limit Switch Tolerance Band


// user unitS
#define OD_2061_00  0x2061, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Velocity Numerator
#define OD_2062_00  0x2062, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Velocity Denominator
#define OD_2063_00  0x2063, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Acceleration Numerator
#define OD_2064_00  0x2064, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Acceleration Denominator
#define OD_2065_00  0x2065, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Jerk Numerator
#define OD_2066_00  0x2066, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Jerk Denominator


#define OD_3202_00  0x3202, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Motor Drive Submode Select


// Motor Drive Sensor Display Closed Loop
#define OD_320B_01  0x320B, 0x01 ,CANOPEN_SDO_TX_4BYTE  // Commutation
#define OD_320B_02  0x320B, 0x02 ,CANOPEN_SDO_TX_4BYTE  // Torque
#define OD_320B_03  0x320B, 0x03 ,CANOPEN_SDO_TX_4BYTE  // Velocity
#define OD_320B_04  0x320B, 0x04 ,CANOPEN_SDO_TX_4BYTE  // Position

// Motor Drive Parameter Set
#define OD_3210_01  0x3210, 0x01 ,CANOPEN_SDO_TX_4BYTE  // Position Loop, Proportional Gain (closed Loop)
#define OD_3210_02  0x3210, 0x02 ,CANOPEN_SDO_TX_4BYTE  // Position Loop, Integral Gain (closed Loop)

// Analogue Inputs Control
#define OD_3221_00  0x3221, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Analogue Inputs Control: 0 , Voltage, 1, Current

// Digital Inputs Control
#define OD_3240_01  0x3240, 0x01 ,CANOPEN_SDO_TX_4BYTE  // Special Function Enable (b2:ZS, b1:PL, b0:Nl)
#define OD_3240_02  0x3240, 0x02 ,CANOPEN_SDO_TX_4BYTE  // Function Inverted (0,NO; 1,NC)
#define OD_3240_03  0x3240, 0x03 ,CANOPEN_SDO_TX_4BYTE  // Force Enable
#define OD_3240_04  0x3240, 0x04 ,CANOPEN_SDO_TX_4BYTE  // Force Value
#define OD_3240_05  0x3240, 0x05 ,CANOPEN_SDO_TX_4BYTE  // Input Raw Value
#define OD_3240_06  0x3240, 0x06 ,CANOPEN_SDO_TX_4BYTE  // Input Range Select (0,threshold,5V, 1: threshold,24V)

// Digital Input Capture
#define OD_3241_01  0x3241, 0x01 ,CANOPEN_SDO_TX_4BYTE  // Control (0:off, 1:RE, 2:FE, 3:RE+FE)
#define OD_3241_02  0x3241, 0x02 ,CANOPEN_SDO_TX_4BYTE  // Capture Count
#define OD_3241_03  0x3241, 0x03 ,CANOPEN_SDO_TX_4BYTE  // Encoder user units
#define OD_3241_04  0x3241, 0x04 ,CANOPEN_SDO_TX_4BYTE  // Encoder Raw Value


// Digital Outputs Control
#define OD_3250_02  0x3250, 0x02 ,CANOPEN_SDO_TX_4BYTE  // Function Inverted
#define OD_3250_03  0x3250, 0x03 ,CANOPEN_SDO_TX_4BYTE  // Force Enable
#define OD_3250_04  0x3250, 0x04 ,CANOPEN_SDO_TX_4BYTE  // Force Value
#define OD_3250_08  0x3250, 0x08 ,CANOPEN_SDO_TX_4BYTE  // Routing Enable


// Change status reaction:-1 No reaction,0 Immediate stop,1 Braking with "slow down ramp",2 Braking with "quick stop ramp"
#define OD_3700_00  0x3700, 0x00 ,CANOPEN_SDO_TX_2BYTE  // Following Error Option Code
#define OD_605A_00  0x605A, 0x00 ,CANOPEN_SDO_TX_2BYTE  // Quick Stop Option Code
#define OD_605B_00  0x605B, 0x00 ,CANOPEN_SDO_TX_2BYTE  // Shutdown Option Code
#define OD_605C_00  0x605C, 0x00 ,CANOPEN_SDO_TX_2BYTE  // Disable Option Code
#define OD_605D_00  0x605D, 0x00 ,CANOPEN_SDO_TX_2BYTE  // Halt Option Code
#define OD_605E_00  0x605E, 0x00 ,CANOPEN_SDO_TX_2BYTE  // Fault Option Code

// Position control characteristics
#define OD_6064_00  0x6064, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Actual position encoder value (user units)
#define OD_607A_00  0x607A, 0x00, CANOPEN_SDO_TX_4BYTE  // Target Position user unit (relative or absolute)

#define OD_6065_00  0x6065, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Following Error Window
#define OD_6066_00  0x6066, 0x00 ,CANOPEN_SDO_TX_2BYTE  // Following Error Time
#define OD_6067_00  0x6067, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Position Target Window + time
#define OD_6068_00  0x6068, 0x00 ,CANOPEN_SDO_TX_2BYTE  // Position Target Time
#define OD_607B_01  0x607B, 0x01 ,CANOPEN_SDO_TX_4BYTE  // Min Position Range Limit
#define OD_607B_02  0x607B, 0x02 ,CANOPEN_SDO_TX_4BYTE  // Max Position Range Limit
#define OD_607D_01  0x607D, 0x01 ,CANOPEN_SDO_TX_4BYTE  // Min Position Limit
#define OD_607D_02  0x607D, 0x02 ,CANOPEN_SDO_TX_4BYTE  // Max Position Limit
#define OD_607E_00  0x607E, 0x00 ,CANOPEN_SDO_TX_1BYTE  // Rotation Polarity
#define OD_6081_00  0x6081, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Maximum travel speed
#define OD_6082_00  0x6082, 0x00 ,CANOPEN_SDO_TX_4BYTE  // End Velocity
#define OD_6083_00  0x6083, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Maximum acceleration
#define OD_6084_00  0x6084, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Maximum Deceleration
#define OD_6085_00  0x6085, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Quick Stop Deceleration

#define OD_60F2_00  0x60F2, 0x00 ,CANOPEN_SDO_TX_2BYTE  // Positioning Option Code

// Position Encoder Resolution: EncInc/MotRev
#define OD_608F_01  0x608F, 0x01 ,CANOPEN_SDO_TX_4BYTE  // Encoder Increments
#define OD_608F_02  0x608F, 0x02 ,CANOPEN_SDO_TX_4BYTE  // Motor Revolutions

// Gear Ratio
#define OD_6091_01  0x6091, 0x01 ,CANOPEN_SDO_TX_4BYTE  // Motor Revolutions
#define OD_6091_02  0x6091, 0x02 ,CANOPEN_SDO_TX_4BYTE  // Shaft Revolutions

// Max Absolute Acceleration and Deceleration
#define OD_60C5_00  0x60c5, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Max Acceleration
#define OD_60C6_00  0x60c6, 0x00 ,CANOPEN_SDO_TX_4BYTE  // Max Deceleration

// Homing profile registers
#define OD_6098_00 0x6098, 0x00 ,CANOPEN_SDO_TX_1BYTE   // Homing method
#define OD_607C_00 0x607C, 0x00 ,CANOPEN_SDO_TX_4BYTE   // Offset to be applied to the zero setting
#define OD_6099_01 0x6099, 0x01 ,CANOPEN_SDO_TX_4BYTE   // Homing Speed to switch
#define OD_6099_02 0x6099, 0x02 ,CANOPEN_SDO_TX_4BYTE   // Homing Speed to reference
#define OD_609A_00 0x609A, 0x00 ,CANOPEN_SDO_TX_4BYTE   // Homing Acceleration

#define OD_4013_01 0x4013, 0x01 ,CANOPEN_SDO_TX_4BYTE   // Hardware Setup: 1= VCC LOGIC ON


#define OD_2300_00 0x2300, 0x00 ,CANOPEN_SDO_TX_4BYTE   // NanoJ Program execution: 1= RUN, 0=STOP
#define OD_2301_00 0x2301, 0x00 ,CANOPEN_SDO_TX_4BYTE   // NanoJ Status --- ERR,RES,RUN
#define OD_2302_00 0x2302, 0x00 ,CANOPEN_SDO_TX_4BYTE   // NanoJ Error code

#define OD_4014_01 0x4014, 0x01 ,CANOPEN_SDO_TX_4BYTE   // Voltage Power (mV)
#define OD_4014_02 0x4014, 0x02 ,CANOPEN_SDO_TX_4BYTE   // Voltage Logic (mV)
#define OD_4014_03 0x4014, 0x03 ,CANOPEN_SDO_TX_4BYTE   // Temperature (dC°)


#define OD_2500_01 0x2500, 0x01 ,CANOPEN_SDO_TX_4BYTE   // Base USER RAM NANO-J (1:32)

//______________________________________________________________________________________________________
//                                  ERRORS


#define PD4_GENERAL_ERROR   0x1
#define PD4_I_ERROR         0x2
#define PD4_VOLTAGE_ERROR   0x4
#define PD4_TEMP_ERROR      0x8
#define PD4_COM_ERROR       0x10
#define PD4_PROFILE_ERROR   0x20
#define PD4_RESERVED_ERROR  0x40
#define PD4_MANUFACT_ERROR  0x80


#endif
/* EOF */
