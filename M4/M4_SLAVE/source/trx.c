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
#define _TRX_C
#include "dbt_m4.h" 
#include "pd4.h"
#include "pd4_c6018l.h"
#include "vmm.h"



// Context to be used with the canopen SDO functions (compliance with CANOPEN_CONTEXT_DEC)
#define CANOPEN_TRX_CONTEXT  MB_TX_TO_TRX, &txmb_to_trx, CANOPEN_TRX_NODE, MB_RX_FROM_TRX, &driver_stat.tmo_sdo_tk
#define DEVICE  "TRX"

#undef  RIDUZIONE
#undef  dGRADsec_TO_ROTmin
#undef  dGRAD_TO_POS

//#define ENABLE_SAFETY_FAULT false
#define ENABLE_SAFETY_FAULT true


#ifdef _PIGNONE_40_RD
    #define RIDUZIONE 396.2  // Rapporto di riduzuzione con pignone a 40 denti e riduttore 1:28 (R&D + IMQ)
    #define _PIGNONE_CONFIG "TRX: COMPILAZIONE CON PIGNONE 40 DENTI PER MACCHINA RD\n"
#else
    #define RIDUZIONE 452.8  // Rapporto di riduzuzione con pignone a 35 denti e riduttore 1:28 (MEDICA)
    #define _PIGNONE_CONFIG "TRX:COMPILAZIONE CON PIGNONE 35 DENTI PER MACCHINA PRODU\n"
#endif


#define SPEED_DENOMINATOR       120
#define cGRADsec_TO_ROT_min(x)  ((uint32_t) (((float)((long)x)  * (float) RIDUZIONE / 36000) * (float) SPEED_DENOMINATOR))


// decimi di grado
#define cGRAD_TO_POS(x)        ((long) (((float) ((long)x) * (RIDUZIONE * 10) )/180))
#define POS_TO_cGRAD(x)        ((long)((((float) ((long)x)) * 180) / (RIDUZIONE*10)))

static _PD4_Status_t driver_stat;
static _trx_positioning_data_t positioningData;
static int  blocco_input_ostacolo;


static bool trxUploadActivationContext(unsigned char index, short target);
static bool readTrxPosition(void);
static bool caricamentoTrxDati(bool forceFlash);
static bool trxGetPositionActivationStatus( int attempt);
static bool trxSafetyDuringMotion(int activation_mode);
static void _resetDriver(void); // Procedura di reset driver
static void _manageInputs(void);
static bool _readInputs(void);
static void _printTrxConfig(void);

static bool _initZeroSetting(_PD4_Status_t* pStat);
static void _zeroSettingLoop(_PD4_Status_t* pStat);

static bool _initPositionSetting(_PD4_Status_t* pStat);
static void _positionSettingLoop(_PD4_Status_t* pStat);

static bool _initManualPositionSetting(_PD4_Status_t* pStat);
static void _manualPositionSettingLoop(_PD4_Status_t* pStat);

static bool _initPositionTriggerSetting(_PD4_Status_t* pStat);
static void _positionTriggerSettingLoop(_PD4_Status_t* pStat);



#define PARAM_ID 1
static const _canopen_ObjectDictionary_t generalMotorProfile[]={

    // Hardware configuration
    {OD_4013_01,1},    // 1 = EXTERNAL VCC LOGIC ON


    // NMT Behavior in case of fault
    {OD_1029_01,0},
    {OD_1029_02,1},
    {OD_2031_00,5000}, 	// Peak current
    {OD_2032_00,5000}, 	// Maximum Speed
    {OD_2033_00,0 },	// Plunger Block
    {OD_2034_00,51500 },// Upper Voltage Warning Level
    {OD_2035_00,24000 },// Lower Voltage Warning Level
    {OD_2036_00,2000}, 	// Open Loop Current Reduction Idle Time
    {OD_2037_00,-50 },	// Open Loop Current Reduction Value/factor

    // I2t Parameters
    {OD_203B_01,5000 },	// Nominal Current
    {OD_203B_02,1000 },	// Maximum Duration Of Peak Current
    {OD_203B_03,0 },	// Threshold
    {OD_203B_04,0 },	// CalcValue
    {OD_203B_05,5000}, 	// LimitedCurrent
    {OD_2056_00,500 },	// Limit Switch Tolerance Band

    // user unitS
    {OD_2061_00,1 },	// Velocity Numerator
    {OD_2062_00,SPEED_DENOMINATOR },  // Velocity Denominator        ***********************
    {OD_2063_00,1 },	// Acceleration Numerator
    {OD_2064_00,SPEED_DENOMINATOR}, 	// Acceleration Denominator    ***********************
    {OD_2065_00,1 },	// Jerk Numerator
    {OD_2066_00,60 },	// Jerk Denominator
    {OD_3202_00,9}, 	// Motor Drive Submode Select: 6:BLDC 3:CurRed 2:Brake 1:VoS 0:CLOOP/OLOOP

    // Motor Drive Sensor Display Closed Loop
    {OD_320B_01,0 	},  // Commutation
    {OD_320B_02,0 },	// Torque
    {OD_320B_03,1 },	// Velocity
    {OD_320B_04,1}, 	// Position

    // Motor Drive Parameter Set
    {OD_3210_01,50000 }, // Position Loop, Proportional Gain (closed Loop)
    {OD_3210_02,10 },	 // Position Loop, Integral Gain (closed Loop)

    // Analogue Inputs Control
    {OD_3221_00,0},     // 0 , Voltage, 1, Current

    // Digital Inputs Control
    {OD_3240_01,4},     // Special Function Enable (b2:ZS, b1:PL, b0:Nl)
    {OD_3240_02,0},     // Function Inverted (0,NO; 1,NC)
    {OD_3240_03,0 },    // Force Enable
    {OD_3240_04,0 },    // Force Value
    {OD_3240_06,0 },    // Input Range Select (0,threshold,5V, 1: threshold,24V)

    // Digital Input Capture
    {OD_3241_01,0 },    // Control (0:off, 1:RE, 2:FE, 3:RE+FE)
    // 3241:02,0        // Capture Count
    //3241:03,0         // Encoder user units
    //3241:04,0         // Encoder Raw Value

    //Digital Outputs Control
    {OD_3250_02,0},     // Function Inverted
    {OD_3250_03,3 },    // Force Enable
    {OD_3250_04,0 },    // Force Value
    {OD_3250_08,0 },    // Routing Enable


    // Following Error Option Code
    {OD_3700_00,-1},
            // -1 No reaction
            // 0 Immediate stop
            // 1 Braking with "slow down ramp"
            // 2 Braking with "quick stop ramp"

    // Quick Stop Option Code
    {OD_605A_00,2 },
            // 0 Immediate stop
            // 1 Braking with "slow down ramp"
            // 2 Braking with "quick stop ramp"

    // Shutdown Option Code
    {OD_605B_00,0 },
            // 0 Immediate stop
            // 1 Braking with "slow down ramp"

    // Disable Option Code
    {OD_605C_00,0 },
            // 0 Immediate stop
            // 1 Braking with "slow down ramp"

    // Halt Option Code
    {OD_605D_00,2 },
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

    // Software Position Limit
    {OD_607D_01,cGRAD_TO_POS(-2700) },	// Min Position Limit
    {OD_607D_02,cGRAD_TO_POS(2700) },	// Max Position Limit

    // Polarity
    {OD_607E_00,0 },	// b7:1-> inverse rotaion

    {OD_6081_00,250 },  // Position Target Speed (at the end of the ramp)
    {OD_6082_00,0 },	// End Velocity
    {OD_6083_00,250 },	// Position acceleraton
    {OD_6084_00,250 },	// Position Deceleration
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
    {OD_6098_00,21},                                            // Homing method 21
    {OD_607C_00,0},                                             // Offset value

    {0,0,0,0} // Last element always present!!
};


// Machine status functions according with the CiA402 standard
void funcCiA402_UndefinedStat(_PD4_Status_t* pStat);
void funcCiA402_NotReadyToSwitch(_PD4_Status_t* pStat);
void funcCiA402_SwitchOnDisabled(_PD4_Status_t* pStat);
void funcCiA402_ReadyToSwitchOn(_PD4_Status_t* pStat);
void funcCiA402_SwitchedOn(_PD4_Status_t* pStat);
void funcCiA402_OperationEnabled(_PD4_Status_t* pStat);
void funcCiA402_QuickStopActive(_PD4_Status_t* pStat);
void funcCiA402_FaultReactionActive(_PD4_Status_t* pStat);
void funcCiA402_Fault(_PD4_Status_t* pStat);

#define _CIA402_FAULT_STAT 8
void (*trxCiA402funcPtr[])(_PD4_Status_t*) ={\
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



// Inizializzazione modulo


void CiA402_Trx_Stat(void){


    // Mutex init
    if (_mutex_init(&driver_stat.req_mutex, NULL) != MQX_OK)
    {
      printf("%s: Mutex Init failed!!\n",DEVICE);
      _mqx_exit(-1);
    }

    _EVCLR(_EV0_TRX_CONNECTED);

    // Tick definition for the timout  TRX
    TIME_STRUCT tmo_ms;             // time in second+ms
    tmo_ms.MILLISECONDS = 100;
    tmo_ms.SECONDS = 0;
    _time_to_ticks(&tmo_ms, &driver_stat.tmo_sdo_tk);

#ifdef _CANDEVICE_SIMULATION
    while(1){
        // Reset module starts here
        _EVCLR(_EV0_TRX_CONNECTED);
        driver_stat.connected = false;
        printf("%s:SIMULATORE MODULE RESTART...........\n\n",DEVICE);

        // Init Driver status
        driver_stat.memCia402Stat = -1; // Force the next change status
        driver_stat.zeroSettingOK = false;
        driver_stat.switch_on=false;
        driver_stat.resetModule = false;
        driver_stat.profileIndex = -1;
        driver_stat.configured = false;
        driver_stat.can_error = false;
        driver_stat.fatal_error = false;
        driver_stat.init_module = true;

        // Segnalazione di driver connesso
        _EVSET(_EV0_TRX_CONNECTED);
        driver_stat.connected = TRUE;
        generalConfiguration.trxConnected = true;

        // Attende infine l'ok generale prima di iniziare il ciclo di lavoro
        while(!generalConfiguration.deviceConnected) _time_delay(500);
        driver_stat.init_module = false;
        // Fine della fase di inizializzazione del Driver

        while(1){

           // Lettura Inputs
           driver_stat.inputs = 0;

           // Set the switch_on flag based on the MAINS_ON system flag
           if(!SystemOutputs.CPU_MASTER_ENA) driver_stat.switch_on = false;
           else driver_stat.switch_on = true;

            _time_delay(1000);
        }
    }
#endif

    while(1){
        // Reset module starts here
        _EVCLR(_EV0_TRX_CONNECTED);
        driver_stat.connected = false;
        printf("%s: MODULE RESTART...........\n\n",DEVICE);

        printf(_PIGNONE_CONFIG);

        if(!ENABLE_SAFETY_FAULT){
            printf("TRX: ATTENZIONE, COMPILAZIONE CON INGRESSO SAFETY DISABILITATO!!!!\n");
        }

        // Init Driver status
        driver_stat.memCia402Stat = -1; // Force the next change status
        driver_stat.zeroSettingOK = false;
        driver_stat.switch_on=false;
        driver_stat.resetModule = false;
        driver_stat.profileIndex = -1;
        driver_stat.configured = false;
        driver_stat.can_error = false;
        driver_stat.fatal_error = false;
        driver_stat.init_module = true;


        // Prima di procedere verifica se il dispositivo di sicurezza rotazione è attivo
        while(!_readInputs()) _time_delay(100);
        _time_delay(1000);
        while(!_readInputs()) _time_delay(100);

        // Verifica se è attivo: toglie l'alimentazione di potenza nel caso
        if((driver_stat.inputs&TRX_FAULT_DETECTION_INPUT)&&(ENABLE_SAFETY_FAULT)){            // TDB FAULT SICUREZZA
            printf("TRX FATAL ERROR: RILEVATO BLOCCO ATTIVO ALLO STARTUP\n");
            driver_stat.fatal_error = true;
            driver_stat.init_module = false;
        }else{

            // Reset Node command to TRX device
            canopenResetNode(2000, CANOPEN_TRX_CONTEXT);

            // Disabilita il driver e riprova fino a che ci riesce
            while(1){
                if(driver_stat.resetModule) break; // Exit the main while loop and restart the module
                if(CiA402_To_SwitchOnDisabled(CANOPEN_TRX_CONTEXT, &driver_stat)==true) break;
                _time_delay(500);
            }


            // Caricamento programma nanoJ
            while(caricamentoNanojProgram(false,DEVICE,nanojVmmfile,sizeof(nanojVmmfile), CANOPEN_TRX_CONTEXT)==false) _time_delay(100);

            // Continua a provare fino a che riesce a completare tutte le operazioni
            while(uploadOjectDictionary(PARAM_ID,DEVICE, generalMotorProfile, CANOPEN_TRX_CONTEXT)==false) _time_delay(100);
        }

        // Segnalazione di driver connesso
        _EVSET(_EV0_TRX_CONNECTED);
        driver_stat.connected = TRUE;
        generalConfiguration.trxConnected = true;

        // Attende infine l'ok generale prima di iniziare il ciclo di lavoro
        while(!generalConfiguration.deviceConnected) _time_delay(500);
        driver_stat.init_module = false;
        // Fine della fase di inizializzazione del Driver

        // ________________________ CICLO DI LAVORO DEL DRIVER _________________________________
        while(1){
            if(driver_stat.resetModule) _resetDriver(); // Esegue il reset completo del driver

           // Lettura Inputs
            _manageInputs();

           // In errore non fa altro
           if(driver_stat.errors){
                if(driver_stat.Cia402Stat!=_CIA402_FAULT_STAT){
                    driver_stat.statChanged=true;
                }else{
                    driver_stat.statChanged=false;
                }
                trxCiA402funcPtr[_CIA402_FAULT_STAT](&driver_stat);                
                _time_delay(10);
                continue;
           }

           // Set the switch_on flag based on the MAINS_ON system flag
           if(!SystemOutputs.CPU_MASTER_ENA) driver_stat.switch_on = false;
           else driver_stat.switch_on = true;


           _mutex_lock(&driver_stat.req_mutex);
            // Get the current internal status
            if(CiA402_GetCurrentStatus(CANOPEN_TRX_CONTEXT, &driver_stat)==false){
                _mutex_unlock(&driver_stat.req_mutex);
                _time_delay(200);
                continue;
            }


            // Activates the proper status loop function
            trxCiA402funcPtr[driver_stat.Cia402Stat](&driver_stat);
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

    }

    // Upload successfully completed: change status
    // Command Shutdown to enter the ReadyToSwitchedOn status
    printf("%s: SWITCH STATUS ATTEMPT TO -READY T SWITCH- STATUS...\n",DEVICE);
    CiA402_SwitchOnDisabled_To_ReadyToSwitchOn(CANOPEN_TRX_CONTEXT,pStat);
    return;

}

void funcCiA402_QuickStopActive(_PD4_Status_t* pStat)
{
    if(pStat->statChanged){
        printf("%s: CiA402 QUICK STOP ACTIVE\n",DEVICE);        
    }


    // Tenta di riattivare lo stato normale
    CiA402_To_SwitchOnDisabled(CANOPEN_TRX_CONTEXT,pStat);

}

void funcCiA402_FaultReactionActive(_PD4_Status_t* pStat)
{
    if(pStat->statChanged){
        printf("%s: CiA402 FAULT REACTION ACTIVE\n",DEVICE);
    }
}

void funcCiA402_Fault(_PD4_Status_t* pStat)
{
    _canopen_ObjectDictionary_t fault_od={OD_1001_00};
    _canopen_ObjectDictionary_t errcode_od={OD_1003_01};
    _canopen_ObjectDictionary_t odencoder={OD_6064_00};
    static uint32_t error;


    // Errore Fatale non ripristinabile
    if(driver_stat.fatal_error) return;


    // E' stato rilevato un blocco degli inputs di rilevamento ostacolo
    // Attende che si kliberi per almeno un certo tempo
    if(driver_stat.errors == TRX_ERROR_BLOCKED_OBSTACLE_CODE){
        pStat->statChanged = false;
        if(driver_stat.inputs & TRX_OBSTACLE_DETECTION_INPUT){
            if(blocco_input_ostacolo){
                 blocco_input_ostacolo--;
                 if(!blocco_input_ostacolo) pStat->errors = 0;
                 else return;
            }
        }else{
            blocco_input_ostacolo = 400;
            return;
        }
    }


    // Legge se c'è un errore in corso
    canopenReadSDO(&fault_od, CANOPEN_TRX_CONTEXT);
    canopenReadSDO(&errcode_od, CANOPEN_TRX_CONTEXT);

    if(fault_od.val==0) pStat->errors = 0;
    else pStat->errors = errcode_od.val;

    if(pStat->statChanged){

        // Lettura posizione corrente
        if(canopenReadSDO(&odencoder, CANOPEN_TRX_CONTEXT)==true){
            pStat->dAngolo = POS_TO_cGRAD(odencoder.val);
        }

        switch(pStat->operatingMode){
            case TRX_ZERO_SETTING:
        case TRX_MOVE_TO_POSITION:
        case TRX_MOVE_WITH_TRIGGER:
        case TRX_MANUAL_MOVE_TO_POSITION:
            // Fault condition during a command
            printf("TRX FAULT DURING COMMAND\n");
            pStat->event_type = pStat->operatingMode;            
            if(SystemOutputs.CPU_PEND_ENA) pStat->event_code = TRX_DISABLED_ERROR;
            else  pStat->event_code = TRX_DEVICE_ERROR;
            pStat->event_data = pStat->errors;
            _EVSET(_EV0_TRX_EVENT);
            break;
        default:
            pStat->event_type = TRX_FAULT;            
            if((SystemOutputs.CPU_PEND_ENA)&&(!SystemOutputs.CPU_XRAY_ENA)){
                // La segnalazione del fault all'interfaccia avviene solo se la rotazione è abilitata
                pStat->event_code = TRX_DEVICE_ERROR;
                pStat->event_data = pStat->errors;
                _EVSET(_EV0_TRX_EVENT);
            }

        }
        pStat->operatingMode = TRX_FAULT;
        error = pStat->errors;
        printf("%s: CiA402 FAULT %x\n",DEVICE,error);
        return;
    }

    // prova ad uscire
    if(pStat->errors==0){
        error = 0;

        // Lettura posizione corrente
        if(canopenReadSDO(&odencoder, CANOPEN_TRX_CONTEXT)==true){
            pStat->dAngolo = POS_TO_cGRAD(odencoder.val);
        }

        // Prova a ritornare operativo
        printf("%s: CiA402 RESET FAULT\n",DEVICE);
        Pd4CiA402FaultReset(CANOPEN_TRX_CONTEXT,pStat);

        pStat->event_type = TRX_FAULT;
        pStat->event_code = 0;
        pStat->event_data = 0;
        _EVSET(_EV0_TRX_EVENT);
        return;
    }

    // Verifica che errore c'è
    if(error!=pStat->errors){
        error = pStat->errors;

        if(SystemOutputs.CPU_PEND_ENA){
            pStat->event_type = TRX_FAULT;
            pStat->event_code = TRX_DEVICE_ERROR;
            pStat->event_data = pStat->errors;
            _EVSET(_EV0_TRX_EVENT);
        }

        error = pStat->errors;
        printf("%s: CiA402 FAULT %x\n",DEVICE,error);
     }
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
    }

    // Verifies if the Application enables to Switch On the motor
    if(pStat->switch_on){
        CiA402_ReadyToSwitchOn_To_SwitchedOn(CANOPEN_TRX_CONTEXT,pStat);
        _time_delay(10);
    }

}

/*___________________________________________________________________________________

                THIS IS THE DRIVER IDLE STATE
                Il ciclo di loop è di circa 10ms
 ____________________________________________________________________________________
 */
#define POLLING_COUNT  100
#define POLLING_COUNT2 500
void funcCiA402_SwitchedOn(_PD4_Status_t* pStat)
{
    static int pollingCount=POLLING_COUNT;

    if(pStat->statChanged){
        printf("%s: CiA402 SWITCHED ON\n",DEVICE);
        pollingCount = POLLING_COUNT;

        // Reset the operating mode register
        CiA402_SetOperatingOD(PD4_NO_PROFILE,CANOPEN_TRX_CONTEXT,pStat);        
    }

    // Verifies if the Application shutdown the operations
    if(!pStat->switch_on){
        CiA402_SwitchedOn_To_ReadyToSwitchOn(CANOPEN_TRX_CONTEXT,pStat);
        _time_delay(100);
        return;
    }


    // Set the IDLE trx operating mode
    if(pStat->operatingMode != TRX_IDLE){
        pStat->operatingMode = TRX_IDLE;

        blocco_input_ostacolo = 400; // Attende un pò di tempo prima di monitorare il fault di blocco ostacolo

        // Disattivazione del programma nanoJ se necessario
        _canopen_ObjectDictionary_t nanoJ={OD_2300_00,0};
        canopenWriteSDO(&nanoJ, CANOPEN_TRX_CONTEXT);
        //_EVSET(_EV0_TRX_IDLE);

        pStat->event_type = TRX_IDLE;
        pStat->event_code = 0;
        pStat->event_data = 0;
        _EVSET(_EV0_TRX_EVENT);
        return;
    }

    // Verifica se deve attivare l'azzeramento del braccio
    if((driver_stat.zeroSettingOK==false)&&(driver_stat.configured)){
        printf("%s: Execution of Automatic zero setting \n",DEVICE);
        pStat->reqCommand=TRX_NO_COMMAND;
        if(_initZeroSetting(pStat)==false){
            pStat->event_type = TRX_ZERO_SETTING;
            pStat->event_code = TRX_ERROR_ZERO_SETTING_INIT;
            pStat->event_data = 0;
            _EVSET(_EV0_TRX_EVENT);
        }else   pStat->operatingMode = TRX_ZERO_SETTING;

    }


    switch(pStat->reqCommand){

    case TRX_IDLE:
        break;
    case TRX_ZERO_SETTING:
        printf("%s: Execution of zero setting \n",DEVICE);
        pStat->reqCommand=TRX_NO_COMMAND;
        if(_initZeroSetting(pStat)==false){
            pStat->event_type = TRX_ZERO_SETTING;
            pStat->event_code = TRX_ERROR_ZERO_SETTING_INIT;
            pStat->event_data = 0;
            _EVSET(_EV0_TRX_EVENT);
        }else   pStat->operatingMode = TRX_ZERO_SETTING;
        break;


    case TRX_MOVE_WITH_TRIGGER:
        printf("%s: Execution of positioning strating with trigger signal\n",DEVICE);
        pStat->reqCommand=TRX_NO_COMMAND;
        if(_initPositionTriggerSetting(pStat)==false){
            pStat->event_type = TRX_MOVE_WITH_TRIGGER;
            pStat->event_code = TRX_ERROR_POSITION_SETTING_INIT;
            pStat->event_data = 0;
            _EVSET(_EV0_TRX_EVENT);
        }else   pStat->operatingMode = TRX_MOVE_WITH_TRIGGER;
        break;

    case TRX_MOVE_TO_POSITION:
        printf("%s: Execution of positioning \n",DEVICE);
        pStat->reqCommand=TRX_NO_COMMAND;
        if(_initPositionSetting(pStat)==false){
            printf("TRX Init Fallito\n");
            pStat->event_type = TRX_MOVE_TO_POSITION;
            pStat->event_code = TRX_ERROR_POSITION_SETTING_INIT;
            pStat->event_data = 0;
            _EVSET(_EV0_TRX_EVENT);
        }else   pStat->operatingMode = TRX_MOVE_TO_POSITION;

        break;

    case TRX_MANUAL_MOVE_TO_POSITION:
        printf("%s: Execution of manual positioning \n",DEVICE);
        pStat->reqCommand=TRX_NO_COMMAND;
        if(_initManualPositionSetting(pStat)==false){
            pStat->event_type = TRX_MANUAL_MOVE_TO_POSITION;
            pStat->event_code = TRX_ERROR_POSITION_SETTING_INIT;
            pStat->event_data = 0;
            _EVSET(_EV0_TRX_EVENT);
        }else   pStat->operatingMode = TRX_MANUAL_MOVE_TO_POSITION;

        break;
    }


    if(pStat->operatingMode== TRX_IDLE){
        if(--pollingCount==0){
            pollingCount=POLLING_COUNT2;
            pStat->event_type = TRX_POLLING_STATUS;
            pStat->event_code = ACUATORS_TRX_POLLING_IDLE ;
            pStat->event_data = pStat->dAngolo;
            _EVSET(_EV0_TRX_EVENT);
        }

        // In Idle controlla lo stato di blocco dispositivo ostacolo
        if((generalConfiguration.armDriver)&&(armGetStatus()->operatingMode!= ARM_IDLE)) blocco_input_ostacolo = 400;

        if(!(driver_stat.inputs & TRX_OBSTACLE_DETECTION_INPUT)){
              if(blocco_input_ostacolo){
                  blocco_input_ostacolo--;
                  if(!blocco_input_ostacolo){
                      driver_stat.errors = TRX_ERROR_BLOCKED_OBSTACLE_CODE;
                      blocco_input_ostacolo = 400;
                      printf("TRX: ERRORE INPUT RILEVAZIONE OSTACOLO BLOCCATO\n");
                      driver_stat.event_type = TRX_FAULT;
                      driver_stat.event_code = TRX_OBSTACLE_BLOCKED_ERROR;
                      driver_stat.event_data = driver_stat.errors;
                      _EVSET(_EV0_TRX_EVENT);
                  }
              }
        }else blocco_input_ostacolo = 10;

    }


    return;
}

void funcCiA402_OperationEnabled(_PD4_Status_t* pStat)
{
    // Verifies if the Application shutdown the operations
    if(!pStat->switch_on){
        CiA402_OperationEnabled_To_ReadyToSwitchOn(CANOPEN_TRX_CONTEXT,pStat);
        _time_delay(100);
        return;
    }

    if(pStat->operatingMode==TRX_ZERO_SETTING){
        _zeroSettingLoop(pStat);
    }else  if(pStat->operatingMode==TRX_MOVE_TO_POSITION){
        _positionSettingLoop(pStat);
    }else  if(pStat->operatingMode==TRX_MANUAL_MOVE_TO_POSITION){
        _manualPositionSettingLoop(pStat);
    }else  if(pStat->operatingMode==TRX_MOVE_WITH_TRIGGER){
        _positionTriggerSettingLoop(pStat);
    }


}



//______________________________________________________________________________________________

bool _initZeroSetting(_PD4_Status_t* pStat){

    // reset di un eventuale richiesta inevasa di stop
    driver_stat.quickstop = false;

    // Caricamento registri di movimento
    if(trxUploadActivationContext(CONTEXT_TRX_ZEROSETTING,0)==false) return false;

    // Caricamento modo operativo
    _canopen_ObjectDictionary_t od={OD_6060_00,PD4_PROFILE_HOMING};
    if(canopenWriteSDO(&od, CANOPEN_TRX_CONTEXT)==false) return false;

    // Reset BIT 4 of the control word
    if(Pd4CiA402SetControlOD(ZERO_SETTING_CTRL_INIT,CANOPEN_TRX_CONTEXT,pStat)==false) return false;

    // Switch CiA status in Operation Enable to start the zero setting
    if(CiA402_SwitchedOn_To_OperationEnabled(ZERO_SETTING_CTRL_INIT,CANOPEN_TRX_CONTEXT,pStat)==false) return false;

    driver_stat.event_type = TRX_RUN;
    driver_stat.event_code = 0;
    driver_stat.event_data = 0;
    _EVSET(_EV0_TRX_EVENT);

    return true;
}

bool _initPositionSetting(_PD4_Status_t* pStat){

    pStat->positionOk = false;

    // reset di un eventuale richiesta inevasa di stop
    driver_stat.quickstop = false;

    // Caricamento registri di movimento
    if(trxUploadActivationContext(positioningData.contextIndex, positioningData.targetPosition)==false) return false;

    // Caricamento modo operativo
    _canopen_ObjectDictionary_t od={OD_6060_00,PD4_PROFILE_POSITIONING};
    if(canopenWriteSDO(&od, CANOPEN_TRX_CONTEXT)==false) return false;


    // Switch CiA status in Operation Enable to start the zero setting
    if(CiA402_SwitchedOn_To_OperationEnabled(POSITION_SETTING_CTRL_INIT,CANOPEN_TRX_CONTEXT,pStat)==false) return false;

    driver_stat.event_type = TRX_RUN;
    driver_stat.event_code = 1;
    driver_stat.event_data = 0;
    _EVSET(_EV0_TRX_EVENT);

    return true;
}

bool _initManualPositionSetting(_PD4_Status_t* pStat){

    pStat->positionOk = false;
    // reset di un eventuale richiesta inevasa di stop
    driver_stat.quickstop = false;

    // Caricamento registri di movimento
    if(trxUploadActivationContext(positioningData.contextIndex, positioningData.targetPosition)==false) return false;

    // Caricamento modo operativo
    _canopen_ObjectDictionary_t od={OD_6060_00,PD4_PROFILE_POSITIONING};
    if(canopenWriteSDO(&od, CANOPEN_TRX_CONTEXT)==false) return false;


    // Switch CiA status in Operation Enable to start the zero setting
    if(CiA402_SwitchedOn_To_OperationEnabled(POSITION_SETTING_CTRL_INIT,CANOPEN_TRX_CONTEXT,pStat)==false) return false;

    driver_stat.event_type = TRX_RUN;
    driver_stat.event_code = 1;
    driver_stat.event_data = 0;
    _EVSET(_EV0_TRX_EVENT);

    return true;
}


bool _initPositionTriggerSetting(_PD4_Status_t* pStat){

    pStat->positionOk = false;
    // reset di un eventuale richiesta inevasa di stop
    driver_stat.quickstop = false;

    // Attivazione programma
    _canopen_ObjectDictionary_t nanoJ={OD_2300_00,1};
    if(canopenWriteSDO(&nanoJ, CANOPEN_TRX_CONTEXT)==false) return false;

    // Verifica attivazione
    _canopen_ObjectDictionary_t nanoJstat={OD_2301_00};
    _canopen_ObjectDictionary_t nanoJerr={OD_2302_00};

    int index = 50;
    while(index--){
        if(canopenReadSDO(&nanoJstat, CANOPEN_TRX_CONTEXT)==false){_time_delay(50); continue;}
        if(nanoJstat.val&0x4){            

            //Legge il codice di errore
            canopenReadSDO(&nanoJerr, CANOPEN_TRX_CONTEXT);
            printf("%s: ERROR OF THE RUNNING NANOJ PROGRAM: %x\n",DEVICE, nanoJerr.val);
            return false;
        }else if(nanoJstat.val&0x1) break;

        _time_delay(50);
    }

    if(index==0){
        printf("%s: ERROR TIMEOUT OF THE RUNNING NANOJ PROGRAM\n",DEVICE);
        return false;
    }

    // Caricamento registri di movimento
    if(trxUploadActivationContext(positioningData.contextIndex, positioningData.targetPosition)==false) return false;

    // Caricamento modo operativo
    _canopen_ObjectDictionary_t od={OD_6060_00,PD4_PROFILE_POSITIONING};
    if(canopenWriteSDO(&od, CANOPEN_TRX_CONTEXT)==false) return false;


    // Switch CiA status in Operation Enable
    if(CiA402_SwitchedOn_To_OperationEnabled(POSITION_TRIGGER_SETTING_CTRL_INIT,CANOPEN_TRX_CONTEXT,pStat)==false) return false;

    driver_stat.event_type = TRX_RUN;
    driver_stat.event_code = 1;
    driver_stat.event_data = 0;
    _EVSET(_EV0_TRX_EVENT);

    return true;
}

void _positionSettingLoop(_PD4_Status_t* pStat){
    static uint16_t memCtrl=0xFFFF;

    if(pStat->statChanged){
        printf("%s: POSITION SETTING MODE STARTED: Timeout value = %d \n",DEVICE, pStat->activation_timeout);
        // Set the BIT4 of Control Word to start the sequence
        Pd4CiA402SetControlOD(POSITION_SETTING_START,CANOPEN_TRX_CONTEXT,pStat);        
    }

    // Lettura posizione corrente
    _canopen_ObjectDictionary_t odencoder={OD_6064_00};
    if(canopenReadSDO(&odencoder, CANOPEN_TRX_CONTEXT)==true){
        pStat->dAngolo = POS_TO_cGRAD(odencoder.val);
    }


    // Controlli di sicurezza. Se fallisce la funzione effettua già il switch di modo
    if(trxSafetyDuringMotion(TRX_MOVE_TO_POSITION)==false) return;

    if((pStat->statusword!=memCtrl)||(odencoder.val == pStat->position_target)){
        memCtrl = pStat->statusword;
        if((pStat->statusword & 0x1400)==0x1400){
            printf("%s: TARGET OK\n",DEVICE);
            pStat->positionOk = true;
            pStat->event_type = TRX_MOVE_TO_POSITION;
            pStat->event_code = TRX_NO_ERRORS;
            pStat->event_data = pStat->dAngolo;
            _EVSET(_EV0_TRX_EVENT);

            // Reset OMS bit of the control word
            Pd4CiA402SetControlOD(PD4_RESET_OMS,CANOPEN_TRX_CONTEXT,pStat);

            // Change status to Switched On status
            CiA402_OperationEnabled_To_SwitchedOn(0,0,CANOPEN_TRX_CONTEXT,pStat);
            return ;

        }
    }

    _time_delay(50);
}


void _manualPositionSettingLoop(_PD4_Status_t* pStat){
    static uint16_t memCtrl=0xFFFF;
    _canopen_ObjectDictionary_t od={OD_6041_00};
    _canopen_ObjectDictionary_t odencoder={OD_6064_00};


    if(pStat->statChanged){
        printf("%s: MANUAL POSITION SETTING MODE STARTED \n",DEVICE);
        // Set the BIT4 of Control Word to start the sequence
        Pd4CiA402SetControlOD(POSITION_SETTING_START,CANOPEN_TRX_CONTEXT,pStat);
    }

    // Lettura posizione corrente
    if(canopenReadSDO(&odencoder, CANOPEN_TRX_CONTEXT)==true){
        pStat->dAngolo = POS_TO_cGRAD(odencoder.val);
    }

    // Fine movimento per Ostacolo
    if(trxGetObstacleStat()){

            // Quick Stop
            Pd4CiA402QuickStop(CANOPEN_TRX_CONTEXT, pStat);

            // Lettura posizione corrente
            if(canopenReadSDO(&odencoder, CANOPEN_TRX_CONTEXT)==true){
                pStat->dAngolo = POS_TO_cGRAD(odencoder.val);
            }

            // Target NOK
            printf("%s: TARGET NOK PER OSTACOLO:%d\n",DEVICE,pStat->dAngolo);
            pStat->positionOk = false;
            pStat->event_type = TRX_MANUAL_MOVE_TO_POSITION;
            pStat->event_code = TRX_OBSTACLE_ERROR;
            pStat->event_data = pStat->dAngolo;
            _EVSET(_EV0_TRX_EVENT);

            // Reset OMS bit of the control word
            Pd4CiA402SetControlOD(PD4_RESET_OMS,CANOPEN_TRX_CONTEXT,pStat);

            // Change status to Switched On status
            CiA402_OperationEnabled_To_SwitchedOn(0,0,CANOPEN_TRX_CONTEXT,pStat);
            return ;
    }

    // Fine movimento per rilascio pulsanti
    if(((SystemInputs.CPU_ROT_CW==0) && (SystemInputs.CPU_ROT_CCW==0)) || (pStat->quickstop)){
            Pd4CiA402QuickStop(CANOPEN_TRX_CONTEXT, pStat);

            // Lettura posizione corrente
            if(canopenReadSDO(&odencoder, CANOPEN_TRX_CONTEXT)==true){
                pStat->dAngolo = POS_TO_cGRAD(odencoder.val);
            }

            // Target OK
            printf("%s: TARGET OK PER RILASCIO PULSANTI:%d\n",DEVICE,pStat->dAngolo);
            pStat->positionOk = true;
            pStat->event_type = TRX_MANUAL_MOVE_TO_POSITION;
            pStat->event_code = TRX_NO_ERRORS;
            pStat->event_data = pStat->dAngolo;
            _EVSET(_EV0_TRX_EVENT);

            // Reset OMS bit of the control word
            Pd4CiA402SetControlOD(PD4_RESET_OMS,CANOPEN_TRX_CONTEXT,pStat);

            // Change status to Switched On status
            CiA402_OperationEnabled_To_SwitchedOn(0,0,CANOPEN_TRX_CONTEXT,pStat);
            return ;
    }

    // Periodically read of the status word to detect the status of the operation
    if(canopenReadSDO(&od, CANOPEN_TRX_CONTEXT)==true){
        if((od.val!=memCtrl)||(odencoder.val == pStat->position_target)||((SystemInputs.CPU_ROT_CW==0) && (SystemInputs.CPU_ROT_CCW==0))){
            memCtrl = od.val;
            if((od.val&0x1400)==0x1400){
                // Target OK
                printf("%s: TARGET OK\n",DEVICE);
                pStat->positionOk = true;
                pStat->event_type = TRX_MANUAL_MOVE_TO_POSITION;
                pStat->event_code = TRX_NO_ERRORS;
                pStat->event_data = pStat->dAngolo;
                _EVSET(_EV0_TRX_EVENT);

                // Reset OMS bit of the control word
                Pd4CiA402SetControlOD(PD4_RESET_OMS,CANOPEN_TRX_CONTEXT,pStat);

                // Change status to Switched On status
                CiA402_OperationEnabled_To_SwitchedOn(0,0,CANOPEN_TRX_CONTEXT,pStat);
                return ;

            }
        }

    }

    _time_delay(50);
}

void _positionTriggerSettingLoop(_PD4_Status_t* pStat){
    static uint16_t memCtrl=0xFFFF;

    if(pStat->statChanged){
        printf("%s: POSITION SETTING MODE STARTED: Timeout value = %d \n",DEVICE, driver_stat.activation_timeout);
    }

    // Lettura posizione corrente
    _canopen_ObjectDictionary_t odencoder={OD_6064_00};
    if(canopenReadSDO(&odencoder, CANOPEN_TRX_CONTEXT)==true){
        pStat->dAngolo = POS_TO_cGRAD(odencoder.val);
    }

    // Controlli di sicurezza. Se fallisce la funzione effettua già il switch di modo
    if(trxSafetyDuringMotion(TRX_MOVE_WITH_TRIGGER)==false) return;


    if((pStat->statusword!=memCtrl)||(odencoder.val == pStat->position_target)){
        memCtrl = pStat->statusword;
        if((pStat->statusword&0x1400)==0x1400){
            // Target OK
            printf("%s: TARGET OK\n",DEVICE);
            pStat->positionOk = true;
            pStat->event_type = TRX_MOVE_WITH_TRIGGER;
            pStat->event_code = TRX_NO_ERRORS;
            pStat->event_data = pStat->dAngolo;
            _EVSET(_EV0_TRX_EVENT);

            // Reset OMS bit of the control word
            Pd4CiA402SetControlOD(PD4_RESET_OMS,CANOPEN_TRX_CONTEXT,pStat);

            // Change status to Switched On status
            CiA402_OperationEnabled_To_SwitchedOn(0,0,CANOPEN_TRX_CONTEXT,pStat);

            trxGetNanojSamples();

            return ;

        }
    }

    _time_delay(50);
}



void _zeroSettingLoop(_PD4_Status_t* pStat){
    static uint16_t memCtrl=0xFFFF;
    bool zeroSettingError=false;

    if(pStat->statChanged){
        printf("%s: ZERO SETTING MODE STARTED \n",DEVICE);
        // Set the BIT4 of Control Word to start the sequence
        Pd4CiA402SetControlOD(ZERO_SETTING_START,CANOPEN_TRX_CONTEXT,pStat);
    }

    // Periodically read of the status word to detect the status of the operation
    _canopen_ObjectDictionary_t od={OD_6041_00};
    if(canopenReadSDO(&od, CANOPEN_TRX_CONTEXT)==true){

        od.val &= CiA402MASK(ZERO_STAT_COMPLETED);
        if(od.val!=memCtrl){
            memCtrl = od.val;
            switch(od.val){
            case CiA402VAL(ZERO_STAT_PERFORMED):
                //printf("%s: ZERO SETTING PERFORMED \n",DEVICE);
                //pStat->zeroSettingOK = true;
                break;
            case CiA402VAL(ZERO_STAT_INTERRUPTED):
                printf("%s: ZERO SETTING INTERRUPTED \n",DEVICE);
                zeroSettingError=true;
                break;
            case CiA402VAL(ZERO_STAT_CONFIRMED):
                printf("%s: ZERO SETTING INITIATED \n",DEVICE);
                break;
            case CiA402VAL(ZERO_STAT_COMPLETED):
                printf("%s: ZERO SETTING COMPLETED \n",DEVICE);
                pStat->zeroSettingOK = true;
                break;
            case CiA402VAL(ZERO_STAT_ERROR_RUN):
                printf("%s: ZERO SETTING ERROR DETECTED: MOTOR RUNNING \n",DEVICE);
                zeroSettingError=true;
                break;
            case CiA402VAL(ZERO_STAT_ERROR_IDL):
                printf("%s: ZERO SETTING ERROR DETECTED: MOTOR STOP!! \n",DEVICE);
                zeroSettingError=true;
                break;

            }

            if(zeroSettingError) {
                readTrxPosition();
                pStat->zeroSettingOK = false;
                pStat->event_type = TRX_ZERO_SETTING;
                pStat->event_code = TRX_ERROR_ZERO_SETTING_EXECUTION;
                pStat->event_data = od.val; // Status Word con l'errore individuato
                _EVSET(_EV0_TRX_EVENT);

                // Reset OMS bit of the control word
                Pd4CiA402SetControlOD(PD4_RESET_OMS,CANOPEN_TRX_CONTEXT,pStat);

                // Change status to Switched On status
                CiA402_OperationEnabled_To_SwitchedOn(0,0,CANOPEN_TRX_CONTEXT,pStat);

                return;
            }

            if(pStat->zeroSettingOK) {
                readTrxPosition();
                pStat->event_type = TRX_ZERO_SETTING;
                pStat->event_code = TRX_NO_ERRORS;
                pStat->event_data = 0;
                _EVSET(_EV0_TRX_EVENT);

                // Reset OMS bit of the control word
                Pd4CiA402SetControlOD(PD4_RESET_OMS,CANOPEN_TRX_CONTEXT,pStat);

                // Change status to Switched On status
                CiA402_OperationEnabled_To_SwitchedOn(0,0,CANOPEN_TRX_CONTEXT,pStat);
                return;
            }

        }

    }

    _time_delay(100);
}


//______________________________________________________________________________________________
//______________________________________________________________________________________________
//______________________________________________________________________________________________
//                                      API APPLICATION

 // Enable the motor operations (switch on=true)
bool trxSetSwitchOn(bool stat){
    driver_stat.switch_on = stat;
    return true;
}

_PD4_Status_t* trxGetStatus(void){
    return &driver_stat;
}

void trxUpdateConfiguration(void){
    _printTrxConfig();
    driver_stat.configured = true;

    return;
}


void trxGetNanojSamples(void){
#ifdef _CANDEVICE_SIMULATION
    return;
#endif

    _canopen_ObjectDictionary_t od_samples={OD_2500_01};

    for(int i=1; i<32;i++){
        od_samples.sbidx = i;
        if(canopenReadSDO(&od_samples, CANOPEN_TRX_CONTEXT)==true){
            printf("TRX CAMPIONAMENTO-%d = %d\n",i,POS_TO_cGRAD(od_samples.val));
        }
    }

    return;
}

// ______________________________________________________________________________________________
// ______________________________________________________________________________________________

bool trxSetCommand(_trx_command_t command, void* data){
#ifdef _CANDEVICE_SIMULATION
    _time_delay(500);
    driver_stat.event_type = command;
    driver_stat.event_code = TRX_NO_ERRORS;
    driver_stat.event_data = 0;
    _EVSET(_EV0_TRX_EVENT);
    return true;
#endif

    // Lock the the mutex to sync the threads with trx driver
    switch(command){

    case TRX_IDLE:
        // This command force to enter the IDLE state
        break;

    case TRX_ZERO_SETTING:
        if((driver_stat.configured==false)||(driver_stat.operatingMode!=TRX_IDLE)||(driver_stat.errors)||(SystemInputs.CPU_LIFT_DROP)){
            driver_stat.event_type = TRX_ZERO_SETTING;
            driver_stat.event_code = TRX_ERROR_INVALID_STATUS;
            driver_stat.event_data = 0;
            _EVSET(_EV0_TRX_EVENT);
        }

        _mutex_lock(&driver_stat.req_mutex);
        driver_stat.reqCommand=command;
        driver_stat.zeroSettingOK = false;
        _mutex_unlock(&driver_stat.req_mutex);
        break;


    case TRX_MOVE_WITH_TRIGGER:
        if((driver_stat.configured==false)||(driver_stat.zeroSettingOK== false)||(driver_stat.operatingMode!=TRX_IDLE)||(data==null)||(driver_stat.errors)||(SystemInputs.CPU_LIFT_DROP)){
            driver_stat.event_type = TRX_MOVE_WITH_TRIGGER;
            driver_stat.event_code = TRX_ERROR_INVALID_STATUS;
            driver_stat.event_data = 0;
            _EVSET(_EV0_TRX_EVENT);
        }

        _mutex_lock(&driver_stat.req_mutex);
        memcpy(&positioningData,data,sizeof(_trx_positioning_data_t));
        printf("TRX: RICHIESTO MOVIMENTO TRX WITH TRIGGER: target:%d context:%d\n",positioningData.targetPosition, positioningData.contextIndex);
        driver_stat.reqCommand=command;
        driver_stat.cmdData = 0; // Not used
        _mutex_unlock(&driver_stat.req_mutex);

        break;



    case TRX_MOVE_TO_POSITION:
        if((driver_stat.configured==false)||(driver_stat.zeroSettingOK== false)||(driver_stat.operatingMode!=TRX_IDLE)||(data==null)||(driver_stat.errors)||(SystemInputs.CPU_LIFT_DROP)){
            driver_stat.event_type = TRX_MOVE_TO_POSITION;
            driver_stat.event_code = TRX_ERROR_INVALID_STATUS;
            driver_stat.event_data = 0;
            if(driver_stat.configured==false) printf("TRX NON CONFIGURATO!\n");
            else if (driver_stat.zeroSettingOK== false)  printf("TRX NON INIZIALIZZATO!\n");
            else if (driver_stat.operatingMode!=TRX_IDLE)  printf("TRX NON IN IDLE !\n");
            else if (data==null)  printf("TRX DATA NULL !\n");
            else if (driver_stat.errors)  printf("TRX DRIVER IN ERROR !\n");
            else    printf("TRX BOH IN ERROR!\n");

            _EVSET(_EV0_TRX_EVENT);
        }

        _mutex_lock(&driver_stat.req_mutex);
        memcpy(&positioningData,data,sizeof(_trx_positioning_data_t));
        printf("TRX: RICHIESTO MOVIMENTO TRX: target:%d context:%d\n",positioningData.targetPosition, positioningData.contextIndex);
        driver_stat.reqCommand=command;
        driver_stat.cmdData = 0; // Not used
        _mutex_unlock(&driver_stat.req_mutex);

        break;

    case TRX_MANUAL_MOVE_TO_POSITION:
        if((driver_stat.configured==false)||(driver_stat.zeroSettingOK== false)||(driver_stat.operatingMode!=TRX_IDLE)||(data==null)||(driver_stat.errors)||(SystemInputs.CPU_LIFT_DROP)){
            driver_stat.event_type = TRX_MANUAL_MOVE_TO_POSITION;
            driver_stat.event_code = TRX_ERROR_INVALID_STATUS;
            driver_stat.event_data = 0;
            _EVSET(_EV0_TRX_EVENT);
        }

        _mutex_lock(&driver_stat.req_mutex);
        memcpy(&positioningData,data,sizeof(_trx_positioning_data_t));
        printf("TRX: RICHIESTO MOVIMENTO MANUALE TRX: target:%d context:%d\n",positioningData.targetPosition, positioningData.contextIndex);
        driver_stat.reqCommand=command;
        driver_stat.cmdData = 0; // Not used
        _mutex_unlock(&driver_stat.req_mutex);

        break;

    case TRX_QUICK_STOP:
        driver_stat.quickstop = true; // Richiede lo stop immediato a qualsiasi movimento in corso
        break;
    default: return false;
    }

    return true;
}

/*
 *  target = 0.1 °/unit posizione finale
 *  index = contesto da utilizzare
 */
#define TIMEOUT(speed,delay) (5000*1000/(speed*delay))
bool trxUploadActivationContext(unsigned char index, short target){
#ifdef _CANDEVICE_SIMULATION
    return true;
#endif
    uint32_t targetSpeed=0;
    uint32_t targetAcc=0;
    uint32_t targetDec=0;

    if(index==CONTEXT_TRX_ZEROSETTING){

        // Caricamento registri del target
        _canopen_ObjectDictionary_t odspeed={OD_6099_01,cGRADsec_TO_ROT_min(trxConfig.zero_setting.speed_approach)};
        if(canopenWriteSDO(&odspeed, CANOPEN_TRX_CONTEXT)==false) return false;

        _canopen_ObjectDictionary_t odrev={OD_6099_02,cGRADsec_TO_ROT_min(trxConfig.zero_setting.speed_reverse)};
        if(canopenWriteSDO(&odrev, CANOPEN_TRX_CONTEXT)==false) return false;

        _canopen_ObjectDictionary_t odacc={OD_609A_00,cGRADsec_TO_ROT_min(trxConfig.zero_setting.accell)};
        if(canopenWriteSDO(&odacc, CANOPEN_TRX_CONTEXT)==false) return false;

        _canopen_ObjectDictionary_t odofs={OD_607C_00,cGRAD_TO_POS(trxConfig.zero_setting.offset)};
        if(canopenWriteSDO(&odofs, CANOPEN_TRX_CONTEXT)==false) return false;

        return true;
    }

    switch(index){

     case CONTEXT_TRX_2D:
        targetSpeed = trxConfig.context2D.speed;
        targetAcc = trxConfig.context2D.accell;
        targetDec = trxConfig.context2D.decell;
        break;
    case CONTEXT_TRX_NARROW:
        targetSpeed = trxConfig.tomo.n.speed;
        targetAcc = trxConfig.tomo.n.accell;
        targetDec = trxConfig.tomo.n.decell;
        break;
    case CONTEXT_TRX_INTERMEDIATE:
        targetSpeed = trxConfig.tomo.i.speed;
        targetAcc = trxConfig.tomo.i.accell;
        targetDec = trxConfig.tomo.i.decell;
        break;
    case CONTEXT_TRX_WIDE:
        targetSpeed = trxConfig.tomo.w.speed;
        targetAcc = trxConfig.tomo.w.accell;
        targetDec = trxConfig.tomo.w.decell;
        break;   

    case CONTEXT_TRX_STEP_SHOT:
        break;

    case CONTEXT_TRX_SLOW_MOTION:
            targetSpeed = trxConfig.zero_setting.speed_manual_approach;
            targetAcc = targetSpeed;
            targetDec = targetSpeed;
    break;

    default: return false;
    }

    printf("SPEED=%d, ACC=%d, DEC=%d\n",targetSpeed, targetAcc, targetDec);
    printf("IMPOSTAZIONE ATTIVAZIONE TRX: TARGET=%d, SPEED=%d, ACC=%d, DEC=%d\n",target,cGRADsec_TO_ROT_min(targetSpeed), cGRADsec_TO_ROT_min(targetAcc), cGRADsec_TO_ROT_min(targetDec));

    // Caricamento registri del target
    _canopen_ObjectDictionary_t odspeed={OD_6081_00,cGRADsec_TO_ROT_min(targetSpeed)};
    if(canopenWriteSDO(&odspeed, CANOPEN_TRX_CONTEXT)==false) return false;
    _canopen_ObjectDictionary_t odacc={OD_6083_00,cGRADsec_TO_ROT_min(targetAcc)};
    if(canopenWriteSDO(&odacc, CANOPEN_TRX_CONTEXT)==false) return false;
    _canopen_ObjectDictionary_t oddec={OD_6084_00,cGRADsec_TO_ROT_min(targetDec)};
    if(canopenWriteSDO(&oddec, CANOPEN_TRX_CONTEXT)==false) return false;

    // Caricamento del target di posizione
    driver_stat.position_target = cGRAD_TO_POS(target);
    _canopen_ObjectDictionary_t odtarget={OD_607A_00,driver_stat.position_target};
    if(canopenWriteSDO(&odtarget, CANOPEN_TRX_CONTEXT)==false) return false;

    driver_stat.activation_timeout = TIMEOUT(targetSpeed,50);
    return true;
}

bool readTrxPosition(void){
#ifdef _CANDEVICE_SIMULATION
    driver_stat.dAngolo = 0;
    return true;
#endif

    _canopen_ObjectDictionary_t odencoder={OD_6064_00};
    int attempt=10;

    // Lettura posizione corrente
    while(attempt--){
        if(canopenReadSDO(&odencoder, CANOPEN_TRX_CONTEXT)==true){
            driver_stat.dAngolo = POS_TO_cGRAD(odencoder.val);            
            return true;
        }
        _time_delay(10);
    }
    return false;

}


uint32_t getTrxPosition(void){
    if(driver_stat.can_error) return 0;
    if(readTrxPosition()== false) return 0;
    else return driver_stat.dAngolo;
}

uint32_t getTrxInputs(void){
    // Lettura Inputs
    return driver_stat.inputs;
}

uint32_t getTrxVbus(void){
#ifdef _CANDEVICE_SIMULATION
    return 40000;
#endif

    // Lettura Inputs
    _canopen_ObjectDictionary_t vbus={OD_4014_01};
    if(canopenReadSDO(&vbus, CANOPEN_TRX_CONTEXT)==true) return vbus.val;
    else return 0;
}

uint32_t getTrxVlogic(void){
#ifdef _CANDEVICE_SIMULATION
    return 24000;
#endif
    // Lettura configurazione Hardware
    _canopen_ObjectDictionary_t vlog={OD_4013_01};
    if(canopenReadSDO(&vlog, CANOPEN_TRX_CONTEXT)==true) return vlog.val;
    else return 0;
}

uint32_t getTrxTemp(void){
#ifdef _CANDEVICE_SIMULATION
    return 25;
#endif
    // Lettura Inputs
    _canopen_ObjectDictionary_t temp={OD_4014_03};
    if(canopenReadSDO(&temp, CANOPEN_TRX_CONTEXT)==true) return temp.val;
    else return 0;
}

uint32_t getTrxFault(void){

    return driver_stat.errors;

}

bool trxGetObstacleStat(void){
#ifdef _CANDEVICE_SIMULATION
    return false;
#endif
    // Attivo basso
    return !(driver_stat.inputs & TRX_OBSTACLE_DETECTION_INPUT);
}

// Funzione locale per l'acquisizione dello status dal driver
// Se la funzione fallisce invia un evento di fault e attiva il comando di quick stop
// Da utilizzare durante i movimenti
bool trxGetPositionActivationStatus(int attempt){
    _canopen_ObjectDictionary_t status={OD_6041_00};

    // Tentativo di lettura dello status
    while(canopenReadSDO(&status, CANOPEN_TRX_CONTEXT)==false){
        attempt--;
        if(!attempt) return false;
        _time_delay(10);
    }
    driver_stat.statusword = status.val;
    return true;
}

bool trxSafetyDuringMotion(int activation_mode){

    // Richiesa Quick stop
    if(driver_stat.quickstop){
        Pd4CiA402QuickStop(CANOPEN_TRX_CONTEXT, &driver_stat);

        printf("%s: RICHIESTA DI QUICK STOP\n",DEVICE);
        driver_stat.positionOk = true;
        driver_stat.event_type = activation_mode;
        driver_stat.event_code = TRX_NO_ERRORS;
        driver_stat.event_data = driver_stat.dAngolo;
        _EVSET(_EV0_TRX_EVENT);

        // Reset OMS bit of the control word
        Pd4CiA402SetControlOD(PD4_RESET_OMS,CANOPEN_TRX_CONTEXT,&driver_stat);

        // Change status to Switched On status
        CiA402_OperationEnabled_To_SwitchedOn(0,0,CANOPEN_TRX_CONTEXT,&driver_stat);

        return false;
    }

    // Acquisizione dello status
    if(trxGetPositionActivationStatus(10)==false){

        // Quick Stop
        Pd4CiA402QuickStop(CANOPEN_TRX_CONTEXT, &driver_stat);

        printf("%s: TARGET NOK PER RICHIESTA STATUS:%d\n",DEVICE,driver_stat.dAngolo);
        driver_stat.positionOk = false;
        driver_stat.event_type = activation_mode;
        driver_stat.event_code = TRX_CAN_ERROR;
        driver_stat.event_data = driver_stat.dAngolo;
        _EVSET(_EV0_TRX_EVENT);

        // Reset OMS bit of the control word
        Pd4CiA402SetControlOD(PD4_RESET_OMS,CANOPEN_TRX_CONTEXT,&driver_stat);

        // Change status to Switched On status
        CiA402_OperationEnabled_To_SwitchedOn(0,0,CANOPEN_TRX_CONTEXT,&driver_stat);
        return false;
    }

    // Fine movimento per Ostacolo
    driver_stat.activation_timeout--; // Timeout movimento
    if((trxGetObstacleStat()) || (driver_stat.statusword & 0x2000) || (!driver_stat.activation_timeout)){

            // Quick Stop
            Pd4CiA402QuickStop(CANOPEN_TRX_CONTEXT, &driver_stat);

            if(driver_stat.statusword & 0x2000){
                printf("%s: TARGET NOK PER ERRORE DI INSEGUIMENTO:%d\n",DEVICE,driver_stat.dAngolo);
                driver_stat.event_code = TRX_OBSTACLE_OBSTRUCTION_ERROR;
            }else if(!driver_stat.activation_timeout){
                printf("%s: TARGET NOK PER TIMEOUT:%d\n",DEVICE,driver_stat.dAngolo);
                driver_stat.event_code = TRX_TIMEOUT_ERROR;
            }else{
                printf("%s: TARGET NOK PER DISPOSITIVO RILEVAMENTO OSTACOLO:%d\n",DEVICE,driver_stat.dAngolo);
                driver_stat.event_code = TRX_OBSTACLE_ERROR;
            }

            driver_stat.positionOk = false;
            driver_stat.event_type = activation_mode;

            driver_stat.event_data = driver_stat.dAngolo;
            _EVSET(_EV0_TRX_EVENT);

            // Reset OMS bit of the control word
            Pd4CiA402SetControlOD(PD4_RESET_OMS,CANOPEN_TRX_CONTEXT,&driver_stat);

            // Change status to Switched On status
            CiA402_OperationEnabled_To_SwitchedOn(0,0,CANOPEN_TRX_CONTEXT,&driver_stat);
            return false;
    }

    // printf("TIMEOUT COUNT:%d\n", driver_stat.activation_timeout);
    return true;
}

// Funzione di Interfaccia per attivare il reset del Driver
bool trxResetModule(void){
    driver_stat.resetModule = true;
    return true;
}


bool _readInputs(void){
    _canopen_ObjectDictionary_t inputs_sdo={OD_3240_05};
    if(canopenReadSDO(&inputs_sdo, CANOPEN_TRX_CONTEXT)==true){
        driver_stat.inputs = inputs_sdo.val;
        return true;
    }else return false;
}

// Effettua la lettura degli inputs digitali e determina se
// c'è un errore sul CAN BUS.
// La funzione rimane in Loop sulla lettura degli inputs fino a che
// la comunicazione riprende regolarmente.
// In caso di anomalia sulla comunicazione viene attivato il flag di errore
// can_error che causerà l'azzeramento dell'Enable dell'alimentazione di potenza
void _manageInputs(void){
    int tentativi = 10;
    if(!_readInputs()){

        while(!_readInputs()){
            if(tentativi){
                tentativi--;
                if(tentativi==0){
                    printf("%s: ERRORE FATALE: COMUNICATION ERROR\n",DEVICE);

                    // Attenzione: se il problema è il cavo di connessione, potrebbe essere che la Vlogic
                    // sia stata scollegata. A questo punto, staccando anche la Vpower il driver si resetta
                    // del tutto. Si perde quindi l'info dell'encoder e pertanto, al ripristino, occorre
                    // effettuare l'azzeramento.
                    driver_stat.errors = TRX_ERROR_COMMUNICATION_CODE;
                    driver_stat.event_type = TRX_FAULT;
                    driver_stat.event_code = TRX_CAN_ERROR;
                    driver_stat.can_error = true;
                    driver_stat.zeroSettingOK = false; // Annulla l'azzeramento che potrebbere essere perso
                    driver_stat.event_data = driver_stat.errors;
                    _EVSET(_EV0_TRX_EVENT);
                }
                _time_delay(10);
            }else _time_delay(1000);
        }

        // Reset communication error
        if(driver_stat.errors==TRX_ERROR_COMMUNICATION_CODE){
            printf("%s: RESET COMUNICATION ERROR\n",DEVICE);
            driver_stat.errors = 0;
            driver_stat.event_type = TRX_FAULT;
            driver_stat.event_code = 0;
            driver_stat.event_data = 0;
            driver_stat.can_error = false;
            _EVSET(_EV0_TRX_EVENT);
        }
    }

    // Errore Fatale da Input di sicurezza
    if((driver_stat.inputs&TRX_FAULT_DETECTION_INPUT)&&(ENABLE_SAFETY_FAULT)){
        if(driver_stat.errors!=TRX_ERROR_SAFETY_FAULT_CODE){
            driver_stat.errors = TRX_ERROR_SAFETY_FAULT_CODE;

            // Effettua il quick stop
            Pd4CiA402QuickStop(CANOPEN_TRX_CONTEXT, &driver_stat);
            printf("ERRORE FATALE: MECCANISMO DI SICUREZZA PENDOLAZIONE\n");
            driver_stat.event_type = TRX_FAULT;
            driver_stat.event_code = TRX_SAFETY_FAULT;
            driver_stat.event_data = driver_stat.errors;
            driver_stat.fatal_error = true;
            _EVSET(_EV0_TRX_EVENT);
        }
     }

}

// Esegue il reset del driver e ricarica firmware e registri.
void _resetDriver(void)
{

    driver_stat.resetModule = false;
}



void _printTrxConfig(void){
#ifndef PRINTCFG
return;
#endif

    printf("---------- TRX CONFIG:-----------------------\n");

    printf("ANGOLO BIOPSIA:%d\n", trxConfig.angolo_biopsia);

    printf("TOMO - wide home_position:%d\n", trxConfig.tomo.w.home_position);
    printf("TOMO - wide end_position:%d\n", trxConfig.tomo.w.end_position);
    printf("TOMO - wide speed:%d\n", trxConfig.tomo.w.speed);
    printf("TOMO - wide accell:%d\n", trxConfig.tomo.w.accell);
    printf("TOMO - wide decell:%d\n", trxConfig.tomo.w.decell);

    printf("TOMO - intermediate home_position:%d\n", trxConfig.tomo.i.home_position);
    printf("TOMO - intermediate end_position:%d\n", trxConfig.tomo.i.end_position);
    printf("TOMO - intermediate speed:%d\n", trxConfig.tomo.i.speed);
    printf("TOMO - intermediate accell:%d\n", trxConfig.tomo.i.accell);
    printf("TOMO - intermediate decell:%d\n", trxConfig.tomo.i.decell);

    printf("TOMO - narrow home_position:%d\n", trxConfig.tomo.n.home_position);
    printf("TOMO - narrow end_position:%d\n", trxConfig.tomo.n.end_position);
    printf("TOMO - narrow speed:%d\n", trxConfig.tomo.n.speed);
    printf("TOMO - narrow accell:%d\n", trxConfig.tomo.n.accell);
    printf("TOMO - narrow decell:%d\n", trxConfig.tomo.n.decell);


    printf("BIOPSIA:%d\n", trxConfig.angolo_biopsia);

    printf("ZERO OFFSET:%d\n", trxConfig.zero_setting.offset);
    printf("ZERO APPROACH:%d\n", trxConfig.zero_setting.speed_approach);
    printf("ZERO REF:%d\n", trxConfig.zero_setting.speed_reverse);
    printf("ZERO ACCELL:%d\n", trxConfig.zero_setting.accell);
    printf("MANUAL ZERO :%d\n", trxConfig.zero_setting.speed_manual_approach);

}

/* EOF */

