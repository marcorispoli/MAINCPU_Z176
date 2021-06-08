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
#define _ARM_C
#include "dbt_m4.h" 
#include "pd4.h"
#include "pd4_c6018l.h"

// Context to be used with the canopen SDO functions (compliance with CANOPEN_CONTEXT_DEC)
#define CANOPEN_ARM_CONTEXT  MB_TX_TO_ARM, &txmb_to_arm, CANOPEN_ARM_NODE, MB_RX_FROM_ARM, &driver_stat.tmo_sdo_tk
#define DEVICE  "ARM"

// RAPPORTO DI RIDUZIONE ALBERO LENTO - ALBERO MOTORE = 424.5
#undef  RIDUZIONE
#undef  dGRADsec_TO_ROTmin
#undef  dGRAD_TO_POS

//#define RIDUZIONE 1200 // 40 * 3, Riduzione * 10  Versione a fune
//#define RIDUZIONE 840 // 28 * 3, Riduzione * 10   Versione a fune
#define RIDUZIONE 1120 // 56 * 2, Riduzione * 10    Versione con cinghia(2) e corona(56)

#define dGRADsec_TO_ROTmin(x)  ((x * RIDUZIONE)/600)
#define dGRAD_TO_POS(x)        ((x * RIDUZIONE )/18)
#define POS_TO_dGRAD(x)        ((((long) x) * 18) / RIDUZIONE)

#define TIMEOUT(speed,delay) (3600*1000/(speed*delay))

static _PD4_Status_t driver_stat;
static _arm_positioning_data_t positioningData;
static int  blocco_input_ostacolo;
static bool armSafetyDuringMotion(int activation_mode);

#define PARAM_ID    1 // Identificativo del codice del blocco parametri da salvare
static const _canopen_ObjectDictionary_t generalMotorProfile[]={

    {OD_4013_01,1},    // 1 = EXTERNAL VCC LOGIC ON

    // NMT Behavior in case of fault
    {OD_1029_01,0},
    {OD_1029_02,1},
    {OD_2031_00,5000}, 	// Maximum Current
    {OD_2032_00,5000}, 	// Maximum Speed
    {OD_2033_00,0 },	// Plunger Block
    {OD_2034_00,60000 },// Upper Voltage Warning Level
    {OD_2035_00,24000 },// Lower Voltage Warning Level
    {OD_2036_00,2000}, 	// Open Loop Current Reduction Idle Time
    {OD_2037_00,-50 },	// Open Loop Current Reduction Value/factor

    // I2t Parameters
    {OD_203B_01,6000 },	// Nominal Current
    {OD_203B_02,1000 },	// Maximum Duration Of Peak Current
    {OD_203B_03,0 },	// Threshold
    {OD_203B_04,0 },	// CalcValue
    {OD_203B_05,6000}, 	// LimitedCurrent
    {OD_2056_00,500 },	// Limit Switch Tolerance Band

    // user unitS
    {OD_2061_00,1 },	// Velocity Numerator
    {OD_2062_00,60 },	// Velocity Denominator
    {OD_2063_00,1 },	// Acceleration Numerator
    {OD_2064_00,60}, 	// Acceleration Denominator
    {OD_2065_00,1 },	// Jerk Numerator
    {OD_2066_00,60 },	// Jerk Denominator
    {OD_3202_00,9}, 	// Motor Drive Submode Select: 6:BLDC 3:CurRed 2:Brake 1:VoS 0:CLOOP/OLOOP

    // Motor Drive Sensor Display Closed Loop
    {OD_320B_01,0 	},// Commutation
    {OD_320B_02,0 },	// Torque
    {OD_320B_03,1 },	// Velocity
    {OD_320B_04,1}, 	// Position

    // Motor Drive Parameter Set
    {OD_3210_01,50000 },	// Position Loop, Proportional Gain (closed Loop)
    {OD_3210_02,10 },	// ex 50 Position Loop, Integral Gain (closed Loop)

    // Analogue Inputs Control
    {OD_3221_00,0}, // 0 , Voltage, 1, Current

    // Digital Inputs Control
    {OD_3240_01,0}, // Special Function Enable (b2:ZS, b1:PL, b0:Nl)
    {OD_3240_02,0}, // Function Inverted (0,NO; 1,NC)
    {OD_3240_03,0 },// Force Enable
    {OD_3240_04,0 },// Force Value
    {OD_3240_06,0 },// Input Range Select (0,threshold,5V, 1: threshold,24V)

    // Digital Input Capture
    {OD_3241_01,0 },// Control (0:off, 1:RE, 2:FE, 3:RE+FE)
    // 3241:02,0 // Capture Count
    //3241:03,0 // Encoder user units
    //3241:04,0 // Encoder Raw Value

    //Digital Outputs Control
    {OD_3250_02,0}, // Function Inverted
    {OD_3250_03,3 },// Force Enable
    {OD_3250_04,0 },// Force Value
    {OD_3250_08,0 },// Routing Enable


    // Following Error Option Code
    {OD_3700_00,-1},
            // -1 No reaction
            // 0 Immediate stop
            // 1 Braking with "slow down ramp"
            // 2 Braking with "quick stop ramp"

    // Quick Stop Option Code
    {OD_605A_00,0 },
            // 0 Immediate stop
            // 1 Braking with "slow down ramp"
            // 2 Braking with "quick stop ramp"

    // Shutdown Option Code
    {OD_605B_00,0 },
            // 0 Immediate stop
            // 1 Braking with "slow down ramp"

    // Disable Option Code
    {OD_605C_00,1 },
            // 0 Immediate stop
            // 1 Braking with "slow down ramp"

    // Halt Option Code
    {OD_605D_00,0 },
            // 0 Immediate stop
            // 1 Braking with "slow down ramp"
            // 2 Braking with "quick stop ramp"

    // Fault Option Code
    {OD_605E_00,2 },
            // 0 Immediate stop
            // 1 Braking with "slow down ramp"
            // 2 Braking with "quick stop ramp"

    // Following Error Window and time
    {OD_6065_00,256}, 	// Window
    {OD_6066_00,100 },	// Time (ms)


    // Position Window + time
    {OD_6067_00,10 },	// Window
    {OD_6068_00,100 },	// Time


    // Position Range Limit
    {OD_607B_01,0}, 	// Min Position Range Limit
    {OD_607B_02,0 },	// Max Position Range Limit

    // Software Position Limit: attenzione che l'encoder non viene azzerato con l'inclinometro
    // pertanto se la macchina viene accesas con il braccio ruotato di 180, l'endoder deve poter fare almeno 360 gradi
    {OD_607D_01,dGRAD_TO_POS(-3650)},	// Min Position Limit
    {OD_607D_02,dGRAD_TO_POS(3650) },	// Max Position Limit

    // Polarity
    {OD_607E_00,0 },	// b7:1-> inverse rotaion

    {OD_6081_00,500 },  // Maximum travel speed
    {OD_6082_00,0 },	// End Velocity
    {OD_6083_00,250 },	// Maximum acceleration
    {OD_6084_00,250 },	// Maximum Deceleration
    {OD_6085_00,250 },	// Quick Stop Deceleration

    // Position Encoder Resolution: EncInc/MotRev
    {OD_608F_01,2000 },	// Encoder Increments
    {OD_608F_02,1}, 	// Motor Revolutions

    {OD_60F2_00,0x0002}, // Absolute positionning

    // Gear Ratio
    {OD_6091_01,1}, 	// Motor Revolutions
    {OD_6091_02,1 },	// Shaft Revolutions

    // Max Absolute Acceleration and Deceleration
    {OD_60C5_00,5000 },// Max Acceleration
    {OD_60C6_00,5000 },// Max Deceleration

    // Homing registers
    //{OD_6098_00,19},                                            // Homing method 19
    //{OD_607C_00,0},                                             // Offset value
    //{OD_6099_01,ARM_HOMING_SPEED_TO_SWITCH},                    // Approaching speed to switch
    //{OD_6099_02,ARM_HOMING_SWITCH_TO_REF},                      // Approaching speeed to reference
    //{OD_609A_00,ARM_HOMING_ACCEL},                              // Acceleration in homing

    {0,0,0,0} // Last element always present!!

};

static bool caricamentoArmDati(void);

// Machine status functions according with the CiA402 standard
static void funcCiA402_UndefinedStat(_PD4_Status_t* pStat);
static void funcCiA402_NotReadyToSwitch(_PD4_Status_t* pStat);
static void funcCiA402_SwitchOnDisabled(_PD4_Status_t* pStat);
static void funcCiA402_ReadyToSwitchOn(_PD4_Status_t* pStat);
static void funcCiA402_SwitchedOn(_PD4_Status_t* pStat);
static void funcCiA402_OperationEnabled(_PD4_Status_t* pStat);
static void funcCiA402_QuickStopActive(_PD4_Status_t* pStat);
static void funcCiA402_FaultReactionActive(_PD4_Status_t* pStat);
static void funcCiA402_Fault(_PD4_Status_t* pStat);

static void (*armCiA402funcPtr[])(_PD4_Status_t*) ={\
    funcCiA402_UndefinedStat,
    funcCiA402_NotReadyToSwitch,
    funcCiA402_SwitchOnDisabled,
    funcCiA402_ReadyToSwitchOn,
    funcCiA402_SwitchedOn,
    funcCiA402_OperationEnabled,
    funcCiA402_QuickStopActive,
    funcCiA402_FaultReactionActive,
    funcCiA402_Fault,
};


static bool InitPositionSetting(_PD4_Status_t* pStat);
static void PositionSettingLoop(_PD4_Status_t* pStat);
static bool InitManualPositionSetting(_PD4_Status_t* pStat);
static void ManualPositionSettingLoop(_PD4_Status_t* pStat);


// Inizializzazione modulo
void CiA402_Arm_Stat(void){
    _canopen_ObjectDictionary_t inputs_sdo={OD_3240_05};

    // Mutex init
    if (_mutex_init(&driver_stat.req_mutex, NULL) != MQX_OK)
    {
      printf("%s: Mutex Init failed!!\n",DEVICE);
      _mqx_exit(-1);
    }

    _EVCLR(_EV0_ARM_CONNECTED);

    // Tick definition for the timout  ARM
    TIME_STRUCT tmo_ms;             // time in second+ms
    tmo_ms.MILLISECONDS = 100;
    tmo_ms.SECONDS = 0;
    _time_to_ticks(&tmo_ms, &driver_stat.tmo_sdo_tk);

#ifdef _CANDEVICE_SIMULATION
    while(1){
        printf("%s: SIMULATORE MODULE RESTART...........\n\n",DEVICE);
        _EVCLR(_EV0_ARM_CONNECTED);
        driver_stat.connected = false;

        // Init Driver status
        driver_stat.memCia402Stat = -1; // Force the next change status
        driver_stat.zeroSettingOK = true;
        driver_stat.switch_on=false;
        driver_stat.resetModule = false;
        driver_stat.profileIndex = -1;
        driver_stat.configured = false;
        driver_stat.can_error = false;
        driver_stat.init_module = true;

        // Segnalazione di driver connesso
        _EVSET(_EV0_ARM_CONNECTED);
        driver_stat.connected = true;
        generalConfiguration.armConnected = true;

        // Attende infine l'ok generale prima di iniziare il ciclo di lavoro
        while(!generalConfiguration.deviceConnected) _time_delay(500);
        driver_stat.init_module = false;

        driver_stat.can_error = false;
        driver_stat.inputs = 0;

        while(1){
            _time_delay(1000);
        }
    }
#endif
    while(1){
    // Reset moule starts here
        printf("%s: MODULE RESTART...........\n\n",DEVICE);
        _EVCLR(_EV0_ARM_CONNECTED);
        driver_stat.connected = false;

        // Init Driver status
        driver_stat.memCia402Stat = -1; // Force the next change status
        driver_stat.zeroSettingOK = true;
        driver_stat.switch_on=false;
        driver_stat.resetModule = false;
        driver_stat.profileIndex = -1;
        driver_stat.configured = false;
        driver_stat.can_error = false;
        driver_stat.init_module = true;

        // Reset Node command to ARM device
        canopenResetNode(2000, CANOPEN_ARM_CONTEXT);

        // Disabilita il driver e riprova fino a che ci riesce
        while(1){
            if(driver_stat.resetModule) break; // Exit the main while loop and restart the module
            if(CiA402_To_SwitchOnDisabled(CANOPEN_ARM_CONTEXT, &driver_stat)==true) break;
            _time_delay(500);
        }

        // Continua a provare fino a che riesce a completare tutte le operazioni
        while(uploadOjectDictionary(PARAM_ID,DEVICE, generalMotorProfile, CANOPEN_ARM_CONTEXT)==false) _time_delay(100);

        // Segnalazione di driver connesso
        _EVSET(_EV0_ARM_CONNECTED);
        driver_stat.connected = true;
        generalConfiguration.armConnected = true;

        // Attende infine l'ok generale prima di iniziare il ciclo di lavoro
        while(!generalConfiguration.deviceConnected) _time_delay(500);
        driver_stat.init_module = false;

        while(1){
           if(driver_stat.resetModule) break; // Exit the main while loop and restart the module

           // Lettura Inputs
           if(!canopenReadSDO(&inputs_sdo, CANOPEN_ARM_CONTEXT)){
               int tentativi = 20;
               while(!canopenReadSDO(&inputs_sdo, CANOPEN_ARM_CONTEXT)){
                   if(tentativi){
                       tentativi--;
                       if(tentativi==0){
                           printf("%s: ERRORE FATALE: COMUNICATION ERROR\n",DEVICE);
                           driver_stat.errors = ARM_ERROR_COMMUNICATION_CODE;
                           driver_stat.event_type = ARM_FAULT;
                           driver_stat.event_code = ARM_CAN_ERROR;
                           driver_stat.can_error = true;
                           driver_stat.event_data = driver_stat.errors;
                           _EVSET(_EV0_ARM_EVENT);
                       }
                       _time_delay(10);
                   }else _time_delay(1000);
               }

               // Reset communication error
               if(driver_stat.errors==ARM_ERROR_COMMUNICATION_CODE){
                   printf("%s: RESET COMUNICATION ERROR\n",DEVICE);
                   driver_stat.errors = 0;
                   driver_stat.event_type = ARM_FAULT;
                   driver_stat.event_code = 0;
                   driver_stat.event_data = 0;
                   _EVSET(_EV0_ARM_EVENT);
               }
           }
           driver_stat.can_error = false;
           driver_stat.inputs = inputs_sdo.val;

           // Set the switch_on flag based on the MAINS_ON system flag           
           if(!SystemInputs.CPU_MAINS_ON) driver_stat.switch_on = false;
           else if(!SystemOutputs.CPU_MASTER_ENA) driver_stat.switch_on = false;
           else driver_stat.switch_on = true;

            _mutex_lock(&driver_stat.req_mutex);

            // Get the current internal status
            if(CiA402_GetCurrentStatus(CANOPEN_ARM_CONTEXT, &driver_stat)==false){
                printf("ERRORE IN GET CURENT STATUS\n");
                _mutex_unlock(&driver_stat.req_mutex);
                _time_delay(200);
                continue;
            }

            // Activates the proper status loop function
            armCiA402funcPtr[driver_stat.Cia402Stat](&driver_stat);
            _mutex_unlock(&driver_stat.req_mutex);

            _time_delay(10);
        }

    }

    return;
}

/*
 *  SWITCHED ON DISABLED STATUS
 *  In this status the system upload the profiles registers
 *  and only when all registers are corectly uploaded switches
 *  to the READY TO SWITCH ON status
 */
void funcCiA402_SwitchOnDisabled(_PD4_Status_t* pStat)
{
    if(pStat->statChanged){
        printf("%s: CiA402 SWITCH ON DISABLED\n",DEVICE);
        // The rotation requires the zero setting
        driver_stat.zeroSettingOK = false;
    }

    // Upload successfully completed: change status
    // Command Shutdown to enter the ReadyToSwitchedOn status
    printf("%s: SWITCH STATUS ATTEMPT TO -READY T SWITCH- STATUS...\n",DEVICE);
    if(CiA402_SwitchOnDisabled_To_ReadyToSwitchOn(CANOPEN_ARM_CONTEXT,pStat)==false) printf("ERRORE CAMBIO STATO\n");
    _time_delay(10);
    return;

}

void funcCiA402_QuickStopActive(_PD4_Status_t* pStat)
{
    if(pStat->statChanged){
        printf("%s: CiA402 QUICK STOP ACTIVE\n",DEVICE);
    }
}

void funcCiA402_FaultReactionActive(_PD4_Status_t* pStat)
{
    if(pStat->statChanged){
        printf("%s: CiA402 FAULT REACTION ACTIVE\n",DEVICE);
    }
}

// Rileva il primo evento di Fault
void funcCiA402_Fault(_PD4_Status_t* pStat)
{
    _canopen_ObjectDictionary_t fault_od={OD_1001_00};
    _canopen_ObjectDictionary_t errcode_od={OD_1003_01};

    /*
    if(!(generalConfiguration.trxDriver)){
        // E' stato rilevato un blocco degli inputs di rilevamento ostacolo
        // Attende che si kliberi per almeno un certo tempo
        if(driver_stat.errors == ARM_ERROR_BLOCKED_OBSTACLE_CODE){
            pStat->statChanged = false;
            if(driver_stat.inputs & ARM_OBSTACLE_DETECTION_INPUT){
                if(blocco_input_ostacolo){
                     blocco_input_ostacolo--;
                     if(!blocco_input_ostacolo){
                         pStat->errors = 0;

                         // Prova a ritornare operativo
                         printf("%s: CiA402 RESET FAULT\n",DEVICE);
                         Pd4CiA402FaultReset(CANOPEN_ARM_CONTEXT,pStat);

                         pStat->event_type = ARM_FAULT;
                         pStat->event_code = 0;
                         pStat->event_data = 0;
                         _EVSET(_EV0_ARM_EVENT);
                         return;
                     }else return;
                }

            }else{
                blocco_input_ostacolo = 400;
                return;
            }
        }

    }

   */

    // Legge se c'è un errore in corso
    canopenReadSDO(&fault_od, CANOPEN_ARM_CONTEXT);
    canopenReadSDO(&errcode_od, CANOPEN_ARM_CONTEXT);

    if(fault_od.val==0){// prova ad uscire
            pStat->errors=0;

            // Prova a ritornare operativo
            printf("%s: CiA402 RESET FAULT\n",DEVICE);
            Pd4CiA402FaultReset(CANOPEN_ARM_CONTEXT,pStat);

            pStat->event_type = ARM_FAULT;
            pStat->event_code = 0;
            pStat->event_data = 0;
            _EVSET(_EV0_ARM_EVENT);
            return;
     }


    // Entra nello stato di errore
    if(pStat->statChanged){               
        switch(pStat->operatingMode){
            case ARM_ZERO_SETTING:
        case ARM_MOVE_TO_POSITION:
        case ARM_MOVE_MANUAL:
            pStat->event_type = pStat->operatingMode;

            // Fault condition during a command
            if(!SystemOutputs.CPU_ROT_ENA){
                pStat->event_code = ARM_DISABLED_ERROR;
                pStat->errors = ARM_ERROR_DISABLE_CODE;
            }else if(!SystemInputs.CPU_EXT_ROT_ENA){
                pStat->event_code = ARM_SECURITY_SWITCHES;
                pStat->errors = ARM_ERROR_SWITCH_SAFETY_CODE;
            }else{
                pStat->event_code = ARM_DEVICE_ERROR;
                pStat->errors = errcode_od.val;
            }
            pStat->event_data = pStat->errors;
            _EVSET(_EV0_ARM_EVENT);

            break;
        default:
            pStat->event_type = ARM_FAULT;            
            if((SystemOutputs.CPU_ROT_ENA)&&(!SystemOutputs.CPU_XRAY_ENA)){
                // La segnalazione del fault all'interfaccia avviene solo se la rotazione è abilitata
                if(!SystemInputs.CPU_EXT_ROT_ENA){
                    pStat->event_code = ARM_SECURITY_SWITCHES;
                    pStat->errors = ARM_ERROR_SWITCH_SAFETY_CODE;
                }else{
                    pStat->event_code = ARM_DEVICE_ERROR;
                    pStat->errors = errcode_od.val;
                }
                pStat->event_data = pStat->errors;
                _EVSET(_EV0_ARM_EVENT);
            }
        }
        pStat->operatingMode = ARM_FAULT;        
        printf("%s: CiA402 FAULT %x\n",DEVICE,pStat->errors);
    }

    return;
}

void funcCiA402_UndefinedStat(_PD4_Status_t* pStat)
{
    if(pStat->statChanged){
        printf("%s: CiA402 UNDEFINED STAT\n",DEVICE);
    }
}

void funcCiA402_NotReadyToSwitch(_PD4_Status_t* pStat)
{
    if(pStat->statChanged){
        printf("%s: CiA402 NOT READY TO SWITCH ON\n",DEVICE);
    }
}



void funcCiA402_ReadyToSwitchOn(_PD4_Status_t* pStat)
{
    if(pStat->statChanged){
        printf("%s: CiA402 READY TO SWITCH ON\n",DEVICE);

        // Set the operating register to no operation
        // Set the switch_on flag based on the MAINS_ON system flag
        if(SystemInputs.CPU_POWER_DOWN) {
            printf("%s: POWER DOWN STATUS DETECTED\n",DEVICE);
        }

    }

    if(SystemInputs.CPU_POWER_DOWN) {
        // Operazioni da compiere in Power Down Mode

    }

    // Verifies if the Application enables to Switch On the motor
    if(pStat->switch_on){
        CiA402_ReadyToSwitchOn_To_SwitchedOn(CANOPEN_ARM_CONTEXT,pStat);
        _time_delay(100);
    }
    _time_delay(10);

}

/*___________________________________________________________________________________

                THIS IS THE DRIVER IDLE STATE
 ____________________________________________________________________________________
 */
#define POLLING_COUNT 100
#define POLLING_COUNT2 500
void funcCiA402_SwitchedOn(_PD4_Status_t* pStat)
{
    static int pollingCount=POLLING_COUNT;

    if(pStat->statChanged){
        printf("%s: CiA402 SWITCHED ON\n",DEVICE);
        pollingCount = POLLING_COUNT;
        // Reset the operating mode register
        CiA402_SetOperatingOD(PD4_NO_PROFILE,CANOPEN_ARM_CONTEXT,pStat);

    }

    // Verifies if the Application shutdown the operations
    if(!pStat->switch_on){
        CiA402_SwitchedOn_To_ReadyToSwitchOn(CANOPEN_ARM_CONTEXT,pStat);
        _time_delay(100);
        return;
    }


    // Set the IDLE arm operating mode
    if(pStat->operatingMode != ARM_IDLE){
        printf("ARM IDLE\n");
        pStat->operatingMode = ARM_IDLE;
        blocco_input_ostacolo = 400;
    }

    switch(pStat->reqCommand){

    case ARM_IDLE:
        pStat->reqCommand=ARM_NO_COMMAND;
        break;

    case ARM_MOVE_TO_POSITION:
        printf("%s: Execution of positioning \n",DEVICE);
        pStat->reqCommand=ARM_NO_COMMAND;
        if(InitPositionSetting(pStat)==false){
            printf("FALLITO INIZIO ROTAZIONE\n");
            pStat->event_type = ARM_MOVE_TO_POSITION;
            pStat->event_code = ARM_ERROR_POSITION_SETTING_INIT;
            pStat->event_data = 0;
            _EVSET(_EV0_ARM_EVENT);

        }else   pStat->operatingMode = ARM_MOVE_TO_POSITION;

        break;

     case ARM_MOVE_MANUAL:
        printf("%s: Execution of positioning \n",DEVICE);
        pStat->reqCommand=ARM_NO_COMMAND;
        if(InitManualPositionSetting(pStat)==false){
            pStat->event_type = ARM_MOVE_MANUAL;
            pStat->event_code = ARM_ERROR_POSITION_SETTING_INIT;
            pStat->event_data = 0;
            _EVSET(_EV0_ARM_EVENT);

        }else   pStat->operatingMode = ARM_MOVE_MANUAL;

        break;

    }

    // In Idle ogni secondo circa invia un comando di status al Master
    // per correggere eventuali flag rimasti appesi per perdita di comunicazione
    if(pStat->operatingMode== ARM_IDLE){
        if(--pollingCount==0){
            pollingCount=POLLING_COUNT2;
            pStat->event_type = ARM_POLLING_STATUS;
            pStat->event_code = ACUATORS_ARM_POLLING_IDLE ;
            pStat->event_data = 0;
            _EVSET(_EV0_ARM_EVENT);
        }

        // Se la pendolazione non è presente è la rotazione che si occupa di rilevare
        // l'ostacolo e pertanto è la rotazione che ne deve garantire l'integrità
        /*
        if(!(generalConfiguration.trxDriver)){
            if(!(driver_stat.inputs & ARM_OBSTACLE_DETECTION_INPUT)){
                  if(blocco_input_ostacolo){
                      blocco_input_ostacolo--;
                      if(!blocco_input_ostacolo){
                          driver_stat.errors = ARM_ERROR_BLOCKED_OBSTACLE_CODE;
                          blocco_input_ostacolo = 400;
                          printf("ARM: ERRORE INPUT RILEVAZIONE OSTACOLO BLOCCATO\n");
                          driver_stat.event_type = ARM_FAULT;
                          driver_stat.event_code = ARM_OBSTACLE_BLOCKED_ERROR;
                          driver_stat.event_data = driver_stat.errors;
                          _EVSET(_EV0_ARM_EVENT);
                      }
                  }
            }else blocco_input_ostacolo = 10;
        }*/
    }

    return;
}

void funcCiA402_OperationEnabled(_PD4_Status_t* pStat)
{
    printf("PASSATO!\n");

    // Verifies if the Application shutdown the operations
    if(!pStat->switch_on){
        printf("OPERATION MODE FAILED PER SWITCHED ON\n");
        CiA402_OperationEnabled_To_ReadyToSwitchOn(CANOPEN_ARM_CONTEXT,pStat);
        _time_delay(100);
        return;
    }

    // During operation, a request to IDLE stop the motor power
    if(pStat->reqCommand==ARM_IDLE){
        pStat->reqCommand=ARM_NO_COMMAND;
        pStat->operatingMode==ARM_NO_COMMAND;
        CiA402_OperationEnabled_To_SwitchedOn(0,0,CANOPEN_ARM_CONTEXT,pStat);
        _time_delay(10);
        return;
    }

    if(pStat->operatingMode==ARM_NO_COMMAND){
        // Sta provando a cambiare stato
        CiA402_OperationEnabled_To_SwitchedOn(0,0,CANOPEN_ARM_CONTEXT,pStat);
        _time_delay(10);
    }else if(pStat->operatingMode==ARM_MOVE_TO_POSITION){
        PositionSettingLoop(pStat);
    }else if(pStat->operatingMode==ARM_MOVE_MANUAL){
        ManualPositionSettingLoop(pStat);
    }


}



//______________________________________________________________________________________________

bool InitPositionSetting(_PD4_Status_t* pStat){

    pStat->positionOk = false;



    // Caricamento registri del target
    _canopen_ObjectDictionary_t odspeed={OD_6081_00,dGRADsec_TO_ROTmin(armConfig.speed)};
    if(canopenWriteSDO(&odspeed, CANOPEN_ARM_CONTEXT)==false) return false;

    _canopen_ObjectDictionary_t odacc={OD_6083_00,dGRADsec_TO_ROTmin(armConfig.accell)};
    if(canopenWriteSDO(&odacc, CANOPEN_ARM_CONTEXT)==false) return false;

    _canopen_ObjectDictionary_t oddec={OD_6084_00,dGRADsec_TO_ROTmin(armConfig.decell)};
    if(canopenWriteSDO(&oddec, CANOPEN_ARM_CONTEXT)==false) return false;


    // Upload registers for target setting..
    _canopen_ObjectDictionary_t od={OD_6064_00,0};
    if(canopenReadSDO(&od, CANOPEN_ARM_CONTEXT)==false) return false;
    positioningData.initEncoder = od.val; // Device Units

    // Attiva contestualmente il riposizionamento del LENZE
    lenzeActivatePositionCompensation(positioningData.initPosition,positioningData.targetPosition);

    // Parcheggio
    if(positioningData.targetPosition==2000) positioningData.targetPosition = 1810;
    else if(positioningData.targetPosition==-2000) positioningData.targetPosition = -1810;

    // Set the Position target
    long deltaP = dGRAD_TO_POS((long) (positioningData.targetPosition - positioningData.initPosition)); // Trasformato in device units
    driver_stat.position_target = positioningData.initEncoder  + deltaP;
    printf("ENCODER:%d, TARGET:%d\n", positioningData.initEncoder,driver_stat.position_target);

    _canopen_ObjectDictionary_t od1={OD_607A_00,driver_stat.position_target};
    if(canopenWriteSDO(&od1, CANOPEN_ARM_CONTEXT)==false) return false;

    // Caricamento modo operativo
    _canopen_ObjectDictionary_t odpos={OD_6060_00,PD4_PROFILE_POSITIONING};
    if(canopenWriteSDO(&odpos, CANOPEN_ARM_CONTEXT)==false) return false;
    _time_delay(20);

    // Switch CiA status in Operation Enable to start the zero setting
    if(CiA402_SwitchedOn_To_OperationEnabled(POSITION_SETTING_CTRL_INIT,CANOPEN_ARM_CONTEXT,pStat)==false) return false;
    _time_delay(20);

    driver_stat.activation_timeout = TIMEOUT(armConfig.speed,75);

    driver_stat.event_type = ARM_RUN;
    driver_stat.event_code = 1; // Movimento Automatico
    driver_stat.event_data = 0;
    _EVSET(_EV0_ARM_EVENT);

    return true;
}

void PositionSettingLoop(_PD4_Status_t* pStat){
    static uint16_t memCtrl=0xFFFF;
    _canopen_ObjectDictionary_t od={OD_6041_00};


    if(pStat->statChanged){
        printf("%s: POSITION SETTING MODE STARTED \n",DEVICE);
        // Set the BIT4 of Control Word to start the sequence
        Pd4CiA402SetControlOD(POSITION_SETTING_START,CANOPEN_ARM_CONTEXT,pStat);
    }

    // Lettura posizione corrente
    _canopen_ObjectDictionary_t odencoder={OD_6064_00};
    if(canopenReadSDO(&odencoder, CANOPEN_ARM_CONTEXT)==true){
        pStat->dAngolo = POS_TO_dGRAD(odencoder.val);
    }

    // Controlli di sicurezza. Se fallisce la funzione effettua già il switch di modo
    if(armSafetyDuringMotion(ARM_MOVE_TO_POSITION)==false) return;


    if((pStat->statusword!=memCtrl)||(odencoder.val == pStat->position_target)){
        memCtrl = pStat->statusword;

        if((pStat->statusword&0x1400)==0x1400){

            printf("%s: TARGET OK\n",DEVICE);
            pStat->positionOk = true;

            pStat->event_type = ARM_MOVE_TO_POSITION;
            pStat->event_code = ARM_NO_ERRORS;
            pStat->event_data = pStat->dAngolo;
            _EVSET(_EV0_ARM_EVENT);


            // Reset OMS bit of the control word
            Pd4CiA402SetControlOD(PD4_RESET_OMS,CANOPEN_ARM_CONTEXT,pStat);

            // Change status to Switched On status
            CiA402_OperationEnabled_To_SwitchedOn(0,0,CANOPEN_ARM_CONTEXT,pStat);
            pStat->operatingMode = ARM_NO_COMMAND;
            _time_delay(10);
            return ;

        }

    }

    _time_delay(50);
}

//______________________________________________________________________________________________

bool InitManualPositionSetting(_PD4_Status_t* pStat){

    pStat->positionOk = false;

    // Caricamento registri del target
    if(positioningData.mode == _MANUAL_ACTIVATION_ARM_STANDARD){
        _canopen_ObjectDictionary_t odspeed={OD_6081_00,dGRADsec_TO_ROTmin(armConfig.manual_speed)};
        if(canopenWriteSDO(&odspeed, CANOPEN_ARM_CONTEXT)==false) return false;

        _canopen_ObjectDictionary_t odacc={OD_6083_00,dGRADsec_TO_ROTmin(armConfig.manual_accell)};
        if(canopenWriteSDO(&odacc, CANOPEN_ARM_CONTEXT)==false) return false;

        _canopen_ObjectDictionary_t oddec={OD_6084_00,dGRADsec_TO_ROTmin(armConfig.manual_decell)};
        if(canopenWriteSDO(&oddec, CANOPEN_ARM_CONTEXT)==false) return false;
    }else{
        _canopen_ObjectDictionary_t odspeed={OD_6081_00,dGRADsec_TO_ROTmin(5)};
        if(canopenWriteSDO(&odspeed, CANOPEN_ARM_CONTEXT)==false) return false;

        _canopen_ObjectDictionary_t odacc={OD_6083_00,dGRADsec_TO_ROTmin(10)};
        if(canopenWriteSDO(&odacc, CANOPEN_ARM_CONTEXT)==false) return false;

        _canopen_ObjectDictionary_t oddec={OD_6084_00,dGRADsec_TO_ROTmin(10)};
        if(canopenWriteSDO(&oddec, CANOPEN_ARM_CONTEXT)==false) return false;
    }

    // Upload registers for target setting..
    _canopen_ObjectDictionary_t od={OD_6064_00,0};
    if(canopenReadSDO(&od, CANOPEN_ARM_CONTEXT)==false) return false;
    positioningData.initEncoder = od.val; // Device Units

    // Set the Position target
    long deltaP = dGRAD_TO_POS((long) (positioningData.targetPosition )); // Trasformato in device units
    driver_stat.position_target = positioningData.initEncoder  + deltaP;
    printf("ENCODER:%d, TARGET:%d\n", positioningData.initEncoder,driver_stat.position_target);

    _canopen_ObjectDictionary_t od1={OD_607A_00, driver_stat.position_target};
    if(canopenWriteSDO(&od1, CANOPEN_ARM_CONTEXT)==false) return false;

    // Caricamento modo operativo
    _canopen_ObjectDictionary_t odpos={OD_6060_00,PD4_PROFILE_POSITIONING};
    if(canopenWriteSDO(&odpos, CANOPEN_ARM_CONTEXT)==false) return false;
    _time_delay(20);

    // Switch CiA status in Operation Enable to start the zero setting
    if(CiA402_SwitchedOn_To_OperationEnabled(POSITION_SETTING_CTRL_INIT,CANOPEN_ARM_CONTEXT,pStat)==false) return false;
    _time_delay(20);

    driver_stat.event_type = ARM_RUN;
    driver_stat.event_code = 1;
    driver_stat.event_data = 0;
    _EVSET(_EV0_ARM_EVENT);

    return true;
}

void ManualPositionSettingLoop(_PD4_Status_t* pStat){
    static uint16_t memCtrl=0xFFFF;
    _canopen_ObjectDictionary_t od={OD_6041_00};
    _canopen_ObjectDictionary_t odencoder={OD_6064_00};

    if(pStat->statChanged){
        printf("%s: MANUAL POSITION SETTING MODE STARTED \n",DEVICE);
        // Set the BIT4 of Control Word to start the sequence
        Pd4CiA402SetControlOD(POSITION_SETTING_START,CANOPEN_ARM_CONTEXT,pStat);
    }

    // Lettura posizione corrente
    if(canopenReadSDO(&odencoder, CANOPEN_ARM_CONTEXT)==true){
        pStat->dAngolo = POS_TO_dGRAD(odencoder.val);
    }

    // Read the Input manual IO in order to stop the activation as soon as the Inputs
    // are released
    if((SystemInputs.CPU_ROT_CW==0) && (SystemInputs.CPU_ROT_CCW==0)){
       // Pd4CiA402QuickStop(CANOPEN_ARM_CONTEXT, pStat);

        // Target OK
        printf("%s: PULSANTI MOVIMENTO MANUALE RILASCIATI\n",DEVICE);
        pStat->positionOk = true;

        pStat->event_type = ARM_MOVE_MANUAL;
        pStat->event_code = ARM_NO_ERRORS;
        pStat->event_data = pStat->dAngolo;
        _EVSET(_EV0_ARM_EVENT);


        // Reset OMS bit of the control word
        Pd4CiA402SetControlOD(PD4_RESET_OMS,CANOPEN_ARM_CONTEXT,pStat);

        // Change status to Switched On status
        CiA402_OperationEnabled_To_SwitchedOn(0,0,CANOPEN_ARM_CONTEXT,pStat);
        pStat->operatingMode = ARM_NO_COMMAND;
        _time_delay(10);
        return ;

    }



    // Periodically read of the status word to detect the status of the operation
    if(canopenReadSDO(&od, CANOPEN_ARM_CONTEXT)==true){


        if((od.val!=memCtrl)||(odencoder.val == pStat->position_target)){
            memCtrl = od.val;
            if((od.val&0x1400)==0x1400){
                // Target OK
                printf("%s: RAGGIUNTO LIMITE DI MOVIMENTO MANUALE\n",DEVICE);
                pStat->positionOk = true;

                pStat->event_type = ARM_MOVE_MANUAL;
                pStat->event_code = ARM_NO_ERRORS;
                pStat->event_data = pStat->dAngolo;
                _EVSET(_EV0_ARM_EVENT);


                // Reset OMS bit of the control word
                Pd4CiA402SetControlOD(PD4_RESET_OMS,CANOPEN_ARM_CONTEXT,pStat);

                // Change status to Switched On status
                CiA402_OperationEnabled_To_SwitchedOn(0,0,CANOPEN_ARM_CONTEXT,pStat);
                pStat->operatingMode = ARM_NO_COMMAND;
                _time_delay(10);
                return ;

            }

        }

    }


    _time_delay(50);
}



//______________________________________________________________________________________________
//______________________________________________________________________________________________
//______________________________________________________________________________________________
//                                      API APPLICATION

 // Enable the motor operations (switch on=true)
bool armSetSwitchOn(bool stat){
    driver_stat.switch_on = stat;
    return true;
}

bool armResetModule(void){
    driver_stat.resetModule = true;
    return true;
}



bool armSetCommand(_arm_command_t command, void* data){
#ifdef _CANDEVICE_SIMULATION
    _time_delay(500);
    driver_stat.event_type = command;
    driver_stat.event_code = ARM_NO_ERRORS;
    driver_stat.event_data = 0;
    _EVSET(_EV0_ARM_EVENT);
    return true;

#endif


    switch(command){


    case ARM_IDLE:
        // This command force to enter the IDLE state
        _mutex_lock(&driver_stat.req_mutex);
        driver_stat.reqCommand=command;
        driver_stat.cmdData = 0; // Not used
        _mutex_unlock(&driver_stat.req_mutex);
        break;


    case ARM_MOVE_MANUAL:
        if((driver_stat.configured == false)||(driver_stat.operatingMode!=ARM_IDLE)||(data==null)||(SystemInputs.CPU_ARM_PED_FAULT)||(driver_stat.errors)||(SystemInputs.CPU_LIFT_DROP)){
            driver_stat.event_type = ARM_MOVE_MANUAL;
            driver_stat.event_code = ARM_ERROR_INVALID_STATUS;
            driver_stat.event_data = 0;
            printf("COMMAND SET ARM ERROR\n");
            _EVSET(_EV0_ARM_EVENT);
            return false;
        }

        _mutex_lock(&driver_stat.req_mutex);
        memcpy(&positioningData,data,sizeof(_arm_positioning_data_t));
        printf("RICHIESTO MOVIMENTO MANUALE ARM: encoder:%d target:%d\n",positioningData.initPosition, positioningData.targetPosition);
        driver_stat.reqCommand=command;
        driver_stat.cmdData = 0; // Not used
        _mutex_unlock(&driver_stat.req_mutex);
        return true;

    case ARM_MOVE_TO_POSITION:
        if((driver_stat.configured == false)||(driver_stat.operatingMode!=ARM_IDLE)||(data==null)||(driver_stat.errors)||(SystemInputs.CPU_LIFT_DROP)){
            driver_stat.event_type = ARM_MOVE_TO_POSITION;
            driver_stat.event_code = ARM_ERROR_INVALID_STATUS;
            driver_stat.event_data = 0;
            printf("COMMAND SET MANUAL ARM ERROR\n");
            _EVSET(_EV0_ARM_EVENT);
            return false;
        }

        _mutex_lock(&driver_stat.req_mutex);
        memcpy(&positioningData,data,sizeof(_arm_positioning_data_t));
        printf("RICHIESTO MOVIMENTO ARM: encoder:%d target:%d\n",positioningData.initPosition, positioningData.targetPosition);
        driver_stat.reqCommand=command;
        driver_stat.cmdData = 0; // Not used
        _mutex_unlock(&driver_stat.req_mutex);
        return true;

    }

    return false;
}

_PD4_Status_t* armGetStatus(void){
    return &driver_stat;
}

void printArmConfig(void){
#ifndef PRINTCFG
return;
#endif
    printf("---------- ARM CONFIG:-----------------------\n");

    printf("SPEED:%d\n", armConfig.speed);
    printf("ACCELL:%d\n", armConfig.accell);
    printf("DECELL:%d\n", armConfig.decell);
    printf("MANUAL SPEED:%d\n", armConfig.manual_speed);
    printf("MANUAL ACCELL:%d\n", armConfig.manual_accell);
    printf("MANUAL DECELL:%d\n", armConfig.manual_decell);

}

void armUpdateConfiguration(void){

    driver_stat.configured = true;
    printArmConfig();
}


uint32_t getArmInputs(void){
    return driver_stat.inputs;
}

uint32_t getArmVbus(void){
#ifdef _CANDEVICE_SIMULATION
    return 40000;
#endif
    // Lettura Inputs
    _canopen_ObjectDictionary_t vbus={OD_4014_01};
    if(canopenReadSDO(&vbus, CANOPEN_ARM_CONTEXT)==true) return vbus.val;
    else return 0;
}

uint32_t getArmVlogic(void){
#ifdef _CANDEVICE_SIMULATION
    return 24000;
#endif
    // Lettura configurazione Hardware
    _canopen_ObjectDictionary_t vlog={OD_4013_01};
    if(canopenReadSDO(&vlog, CANOPEN_ARM_CONTEXT)==true) return vlog.val;
    else return 0;
}

uint32_t getArmTemp(void){
#ifdef _CANDEVICE_SIMULATION
    return 25;
#endif
    // Lettura Inputs
    _canopen_ObjectDictionary_t temp={OD_4014_03};
    if(canopenReadSDO(&temp, CANOPEN_ARM_CONTEXT)==true) return temp.val;
    else return 0;
}

uint32_t getArmFault(void){

    return driver_stat.errors;
}

bool armGetObstacleStat(void){
#ifdef _CANDEVICE_SIMULATION
    return false;
#endif
    // Se c'è la pendolazione l'ostacolo viene rilevato dalla pendolazione
    if(generalConfiguration.trxDriver){
        // Input Attivo basso
        if(trxGetStatus()->inputs & TRX_OBSTACLE_DETECTION_INPUT) return false;
        return true;
    }else if(generalConfiguration.armDriver){
        // Input Attivo basso
        if(armGetStatus()->inputs & ARM_OBSTACLE_DETECTION_INPUT) return false;
        return true;
    }else return false;
}

// Funzione locale per l'acquisizione dello status dal driver
// Se la funzione fallisce invia un evento di fault e attiva il comando di quick stop
// Da utilizzare durante i movimenti
bool armGetPositionActivationStatus(int attempt){
    _canopen_ObjectDictionary_t status={OD_6041_00};

    // Tentativo di lettura dello status
    while(canopenReadSDO(&status, CANOPEN_ARM_CONTEXT)==false){
        attempt--;
        if(!attempt) return false;
        _time_delay(10);
    }
    driver_stat.statusword = status.val;
    return true;
}

bool armSafetyDuringMotion(int activation_mode){

    // Richiesa Quick stop
    if(driver_stat.quickstop){
        Pd4CiA402QuickStop(CANOPEN_ARM_CONTEXT, &driver_stat);

        printf("%s: RICHIESTA DI QUICK STOP\n",DEVICE);
        driver_stat.positionOk = true;
        driver_stat.event_type = activation_mode;
        driver_stat.event_code = ARM_NO_ERRORS;
        driver_stat.event_data = driver_stat.dAngolo;
        _EVSET(_EV0_ARM_EVENT);

        // Reset OMS bit of the control word
        Pd4CiA402SetControlOD(PD4_RESET_OMS,CANOPEN_ARM_CONTEXT,&driver_stat);

        // Change status to Switched On status
        CiA402_OperationEnabled_To_SwitchedOn(0,0,CANOPEN_ARM_CONTEXT,&driver_stat);

        driver_stat.operatingMode = ARM_NO_COMMAND;
        return false;
    }

    // Acquisizione dello status
    if(armGetPositionActivationStatus(10)==false){

        // Quick Stop
        Pd4CiA402QuickStop(CANOPEN_ARM_CONTEXT, &driver_stat);

        printf("%s: TARGET NOK PER RICHIESTA STATUS:%d\n",DEVICE,driver_stat.dAngolo);
        driver_stat.positionOk = false;
        driver_stat.event_type = activation_mode;
        driver_stat.event_code = ARM_CAN_ERROR;
        driver_stat.event_data = driver_stat.dAngolo;
        _EVSET(_EV0_ARM_EVENT);

        // Reset OMS bit of the control word
        Pd4CiA402SetControlOD(PD4_RESET_OMS,CANOPEN_ARM_CONTEXT,&driver_stat);

        // Change status to Switched On status
        CiA402_OperationEnabled_To_SwitchedOn(0,0,CANOPEN_ARM_CONTEXT,&driver_stat);
        driver_stat.operatingMode = ARM_NO_COMMAND;
        return false;
    }

    // Fine movimento per Ostacolo
    if(driver_stat.activation_timeout) driver_stat.activation_timeout--; // Timeout movimento
    if((armGetObstacleStat()) || (driver_stat.statusword & 0x2000) || (!driver_stat.activation_timeout)){

            // Quick Stop
            Pd4CiA402QuickStop(CANOPEN_ARM_CONTEXT, &driver_stat);

            if(driver_stat.statusword & 0x2000){
                printf("%s: TARGET NOK PER ERRORE DI INSEGUIMENTO:%d\n",DEVICE,driver_stat.dAngolo);
                driver_stat.event_code = ARM_OBSTACLE_OBSTRUCTION_ERROR;
            }else if(!driver_stat.activation_timeout){
                printf("%s: TARGET NOK PER TIMEOUT:%d\n",DEVICE,driver_stat.dAngolo);
                driver_stat.event_code = ARM_ERROR_POSITION_TMO;
            }else{
                printf("%s: TARGET NOK PER DISPOSITIVO RILEVAMENTO OSTACOLO:%d\n",DEVICE,driver_stat.dAngolo);
                driver_stat.event_code = ARM_OBSTACLE_ERROR;
            }

            driver_stat.positionOk = false;
            driver_stat.event_type = activation_mode;

            driver_stat.event_data = driver_stat.dAngolo;
            _EVSET(_EV0_ARM_EVENT);

            // Reset OMS bit of the control word
            Pd4CiA402SetControlOD(PD4_RESET_OMS,CANOPEN_ARM_CONTEXT,&driver_stat);

            // Change status to Switched On status
            CiA402_OperationEnabled_To_SwitchedOn(0,0,CANOPEN_ARM_CONTEXT,&driver_stat);
            driver_stat.operatingMode = ARM_NO_COMMAND;
            return false;
    }

   // printf("ARM TIMEOUT COUNT:%d\n", driver_stat.activation_timeout);
    return true;
}


/* EOF */
