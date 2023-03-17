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
#define _LENZE_C

#include "dbt_m4.h"
#include "i510.h"
#include "lenze.h"
#include "math.h"

// Context to be used with the canopen SDO functions (compliance with CANOPEN_CONTEXT_DEC)
#define CANOPEN_LENZE_CONTEXT  MB_TX_TO_LENZE, &txmb_to_lenze, CANOPEN_LENZE_NODE, MB_RX_FROM_LENZE, &driver_stat.tmo_sdo_tk
#define DEVICE  "LENZE"
static _i510_Status_t driver_stat;
static void lenzeLoop(void);
static void lenze_manage_soglie(void);
static int lenzeReadMotorFreq(void);
static void lenzeStartupConfiguration(void);
static void lenzeRunConfiguration(void);
static int lenzeVBUS=-1;

#define ERR_SOGLIA_H 0x7081
#define ERR_SOGLIA_L 0x7082

static const _canopen_ObjectDictionary_t lenzeGeneralMotorProfile[]={

    {i510_2631_18_OD,0},    // Assegnazione trigger "INVERSIONE"ACTIVATE PRESET 0"
    {i510_2631_19_OD,0},    // Assegnazione trigger "INVERSIONE"ACTIVATE PRESET 1"
    {i510_2633_1_OD,10},    // Debounce Input 1 (ms)
    {i510_2633_2_OD,10},    // Debounce Input 2 (ms)
    {i510_2633_3_OD,10},    // Debounce Input 3 (ms)
    {i510_2633_4_OD,10},    // Debounce Input 4 (ms)
    {i510_2633_5_OD,10},    // Debounce Input 5 (ms)
    {i510_2634_01_OD,56},   // Assegnazione trigger output RELAY
    {i510_2839_04_OD,50},   // Fault: trouble counter reset time
    {i510_2860_02_OD,13},   // Freq. control input selection
    {i510_2910_02_OD,1070}, // Load Inerzia
    {i510_2917_00_OD,2},    // Accell Time s/10
    {i510_2918_00_OD,2},    // Decell Time s/10
    {i510_2C11_06_OD,80},   // Stall monitoring %
    {i510_2C12_01_OD,100},  // SM-low speed range Accell current %
    {i510_2C12_02_OD,40},    // SM-low speed range Standstill current %
    {0,0,0,0} // Last element always present!!
};

// Inizializzazione modulo
void driver_Lenze_Stat(void){
    static bool switch_on=false;
    unsigned char data;

    // Mutex init
    if (_mutex_init(&(driver_stat.req_mutex), NULL) != MQX_EOK)
    {
      printf("%s: Mutex Init failed!!\n",DEVICE);
      _mqx_exit(-1);
    }
    _EVCLR(_EV0_LENZE_CONNECTED);
    driver_stat.connected = false;

    // Tick definition for the timout
    TIME_STRUCT tmo_ms;             // time in second+ms
    tmo_ms.MILLISECONDS = 100;
    tmo_ms.SECONDS = 0;
    _time_to_ticks(&tmo_ms, &driver_stat.tmo_sdo_tk);

#ifdef _CANDEVICE_SIMULATION
    while(1){
        debugPrint("LENZE SIMULATOR MODULE RESTART");
        generalConfiguration.lenzeConnected = true;
        driver_stat.analog1 = driver_stat.analog2 = 500;

        // Attende infine l'ok generale prima di iniziare il ciclo di lavoro
        while(!generalConfiguration.deviceConnected) _time_delay(500);

        driver_stat.switch_on = true;

        _EVSET(_EV0_LENZE_CONNECTED);
        driver_stat.connected = true;

        while(1){
            _time_delay(3000);

            driver_stat.event_type = LENZE_POT_UPDATE;
            driver_stat.event_code = driver_stat.analog1;
            driver_stat.event_data = driver_stat.analog2;
            _EVSET(_EV0_LENZE_EVENT);

            _time_delay(3000);
            lenzeVBUS = 290;
            _EVSET(_EV0_POWER_EVENT);

            _time_delay(3000);

        }
    }
#endif

    while(1){
    // Reset moule starts here
        printf("LENZE MODULE RESTART\n");

        // Init Driver status
        driver_stat.memNetworkStat = -1; // Force the next change status
        driver_stat.resetModule = false;

        printf("LENZE INIT CONFIGURATION\n");

        // Upload of the Object Dictionary general parameters to configure the motor device
        while(canopenUploadObjectDictionaryList(lenzeGeneralMotorProfile,10,CANOPEN_LENZE_CONTEXT)==false){
            printf("LENZE ERROR IN UPLOADING THE GENERAL OBJECT DICTIONARY ITEMS\n");
            _time_delay(500);
        }
        printf("LENZE UPLOAD PARAMETRI GENERALI, COMPLETATO\n");

        switch_on = driver_stat.switch_on;
        canopenSetNetworkPreOperation(1000, CANOPEN_LENZE_CONTEXT);
        lenzeRunConfiguration();

        generalConfiguration.lenzeConnected = true;

        // Inizializza la lettura del potenziometro per evitare diagnostiche finte
        driver_stat.analog1 = driver_stat.analog2 = 500;

        // Attende infine l'ok generale prima di iniziare il ciclo di lavoro
        while(!generalConfiguration.deviceConnected) _time_delay(500);


        while(1){

            if(!SystemOutputs.CPU_MASTER_ENA) driver_stat.switch_on = false;
            else driver_stat.switch_on = true;

           if(SystemInputs.CPU_POWER_DOWN){
               printf("LENZE POWER DOWN CONDITION\n");
               while(SystemInputs.CPU_POWER_DOWN){
                   // Attende che il Powerdown finisca
                   _time_delay(1000);
               }
               printf("LENZE POWER DOWN TERMINATED: RECONFIGURATION..\n");
               canopenSetNetworkPreOperation(1000, CANOPEN_LENZE_CONTEXT);
               lenzeRunConfiguration();
               switch_on = driver_stat.switch_on ;
           }

           // Set the switch_on flag based on the MAINS_ON system flag
           if(switch_on!=driver_stat.switch_on){
                switch_on = driver_stat.switch_on ;
                printf("LENZE SWITCH ON CHANGED\n");
                driver_stat.switch_on = !driver_stat.switch_on;
                lenzeRunConfiguration();
           }

           // Get the current internal status
           if(getI510NetworkStatus(CANOPEN_LENZE_CONTEXT, &driver_stat)==false){
                printf("LENZE Read Status FAILED\n");
                 _time_delay(100);
                continue;
            }

            lenzeLoop();
            _time_delay(10);
        }

    }

    return;
}

// Loop eseguito ogni 10ms
void lenzeLoop(void)
{
    static uint16_t error=0;
    static unsigned char time_counter=0;
    static int position1, position2;
    static bool manual_mode = false;


    if(driver_stat.statChanged){
        driver_stat.statChanged=false;

        // Se il driver arriva qui allora significa che risulta essere connesso
        if(driver_stat.networkStat == CANOPEN_NETWORK_PRE_OPERATIONAL){
             _EVSET(_EV0_LENZE_CONNECTED);
             driver_stat.connected = true;
             lenzeStartupConfiguration(); // Inizializza la configurazione
        }

    }


    // Lettura periodica degli Inputs
    if(generalConfiguration.lenzeCalibPot){
        if(lenzeReadPosition()){
            if( ((driver_stat.analog1 > position1+5)||(driver_stat.analog1 < position1-5)) ||
                ((driver_stat.analog2 > position2+5)||(driver_stat.analog2 < position2-5))){

                position1 = driver_stat.analog1;
                position2 = driver_stat.analog2;
                driver_stat.event_type = LENZE_POT_UPDATE;
                driver_stat.event_code = driver_stat.analog1;
                driver_stat.event_data = driver_stat.analog2;
                _EVSET(_EV0_LENZE_EVENT);
                printf("LENZE POSITION1:%d POS2:%d\n",driver_stat.analog1,driver_stat.analog2);
            }
        }
    }else{
        if(++time_counter==100){
            time_counter=0;
            if(lenzeReadPosition()){
                if((driver_stat.analog1 > position1+10)||(driver_stat.analog1 < position1-10)){
                    position1 = driver_stat.analog1;
                    position2 = position1;
                    printf("LENZE POSITION:%d\n",driver_stat.analog1);
                }
            }

        }
    }
    // Lettura periodica del Potenziometro
    if(driver_stat.manual_mode_limit){
        if(lenzeReadMotorFreq()==0){
            // Se la posizione esce dai limiti, ripristina le soglie consuete
            if(lenzeReadPosition()){
                if((driver_stat.analog1 > lenzeConfig.min_lenze_position * 10 + 10) && (driver_stat.analog1 < lenzeConfig.max_lenze_position * 10 - 10))
                      lenzeSetSpeedManual(lenzeConfig.min_lenze_position * 10, lenzeConfig.max_lenze_position * 10);
            }
        }
    }

    // Gestione dell'ostacolo durante il movimento automatico del braccio
    if(driver_stat.manual_mode == false){
        if(lenzeGetObstacleStat()){
            setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2634_02_OD, 0);   // DIGOUT1 = OFF
            lenze_manage_soglie(); // Resetta la modalità Manuale
        }
    }

    if(driver_stat.gestione_soglie){
        if(lenzeReadMotorFreq()==0){
            lenze_manage_soglie();
        }
    }

    // Lettura registro errori interni
    uint16_t internal_error = getI510ErrorRegister(CANOPEN_LENZE_CONTEXT);
    if((internal_error==ERR_SOGLIA_H)||(internal_error==ERR_SOGLIA_L)) {
        driver_stat.err_soglie = internal_error;
        driver_stat.gestione_soglie = true;
        driver_stat.internal_errors = 0;
    }else driver_stat.internal_errors = internal_error;

    // Verifica diagnostica di altro tipo
    if(SystemInputs.CPU_LIFT_DROP){
        driver_stat.diagnostic_errors = LENZE_DROP_ARM;
    }else if((driver_stat.analog1<100)||(driver_stat.analog1>900)){
        driver_stat.diagnostic_errors = LENZE_ANALOG_INPUT_ERROR;
    }else if((driver_stat.analog2<100)||(driver_stat.analog2>900)){
        driver_stat.diagnostic_errors = LENZE_ANALOG_INPUT_ERROR;
    }else if((driver_stat.analog1 > driver_stat.analog2 + 2)||(driver_stat.analog1 < driver_stat.analog2 - 2)){
        driver_stat.diagnostic_errors = LENZE_ANALOG_CONNECTION_ERROR;
    }else if((driver_stat.analog1<100)||(driver_stat.analog1>900)){
        driver_stat.diagnostic_errors = LENZE_ANALOG_INPUT_ERROR;
    }else  driver_stat.diagnostic_errors = 0;


    if(driver_stat.internal_errors){
        if(driver_stat.internal_errors!=error){
            printf("LENZE ERRORE INTERNO:\n");
            printf(i510ErrorString(driver_stat.internal_errors));
            driver_stat.event_type = LENZE_FAULT;
            driver_stat.event_code = LENZE_DEVICE_ERROR;
            driver_stat.event_data = driver_stat.internal_errors;
            _EVSET(_EV0_LENZE_EVENT);
            error =  driver_stat.internal_errors;
        }
    }else if(driver_stat.diagnostic_errors){
        if(driver_stat.diagnostic_errors!=error){
            printf("LENZE ERRORE DIAGNOSTICO:%d\n",driver_stat.diagnostic_errors);
            driver_stat.event_type = LENZE_FAULT;
            driver_stat.event_code = driver_stat.diagnostic_errors;
            driver_stat.event_data = 0;
            _EVSET(_EV0_LENZE_EVENT);
            error =  driver_stat.diagnostic_errors;
        }
    }else{
        if(error) {
            // Reset Errors
            printf("LENZE RESET ERRORS\n");
            driver_stat.event_type = LENZE_FAULT;
            driver_stat.event_code = 0;
            driver_stat.event_data = 0;
            _EVSET(_EV0_LENZE_EVENT);
            error =  0;
        }
    }

    // Lettura periodica della tensione di BUS
    if(driver_stat.connected){
        int val = getI510BusVoltage(CANOPEN_LENZE_CONTEXT);
        if((val>lenzeVBUS+2)||(val<lenzeVBUS-2)){
            lenzeVBUS = val;
            _EVSET(_EV0_POWER_EVENT);
        }
    }else lenzeVBUS=-1;



    // Segnale di notifica di fine movimento automatizzato
    if(manual_mode!=driver_stat.manual_mode){
        manual_mode = driver_stat.manual_mode;

        // Segnalazione cambio di stato
        if(manual_mode) printf("LENZE CAMBIO STATO -> MANUAL MODE\n");
        else printf("LENZE CAMBIO STATO -> AUTO MODE\n");

        driver_stat.event_type = LENZE_RUN;
        if(manual_mode) driver_stat.event_code = 1; // Manual Mode
        else driver_stat.event_code = 0; // Auto mode
        driver_stat.event_data = driver_stat.analog1; // Posizione corrente
        _EVSET(_EV0_LENZE_EVENT);
    }

    // powerManagement();
    return;

}

_i510_Status_t* lenzeGetStatus(void){
    return &driver_stat;
}

void printLenzeConfig(void){
    printf("---------- LENZE CONFIG:-----------------------\n");
    printf("MIN POSITION:%d\n", lenzeConfig.min_lenze_position);
    printf("MAX POSITION:%d\n", lenzeConfig.max_lenze_position);
    printf("MANUAL SPEED:%d\n", lenzeConfig.manual_speed);
    printf("AUTO SPEED:%d\n", lenzeConfig.automatic_speed);
    printf("PARK SPEED:%d\n", lenzeConfig.parking_speed);
    printf("PARK TARGET:%d\n", lenzeConfig.parkingTarget);
    printf("PARK SAFE POINT:%d\n", lenzeConfig.parkingSafePoint);

    if(lenzeConfig.startupInParkingMode){
        printf("LENZE PARTE IN MODALITA PARCHEGGIO\n");
    }
    printf("---------------------------------\n");

}

// Aggiorna la configurazione dei registri lenze e aggiorna il device
void lenzeUpdateConfiguration(void){

    driver_stat.configured = true;
    printLenzeConfig();

#ifdef _CANDEVICE_SIMULATION
    return;
#endif

    // Prova ad attivare la configurazione operativa
    lenzeRunConfiguration();
}

// Legge gli inputs Analogici
// verifica che entrambi abbiano lo stesso valore
bool lenzeReadPosition(void){
#ifdef _CANDEVICE_SIMULATION
    return true;
#endif
    _canopen_ObjectDictionary_t odan1= {i510_2DA4_01_OD};
    _canopen_ObjectDictionary_t odan2= {i510_2DA5_01_OD};

    // Verifies the current status
    if(canopenReadSDO(&odan1, CANOPEN_LENZE_CONTEXT)== false) return false;
    if(canopenReadSDO(&odan2, CANOPEN_LENZE_CONTEXT)== false) return false;

    driver_stat.analog1 = odan1.val;           // Valore analogica 1
    driver_stat.analog2 = odan2.val;           // Valore analogica 2

    // printf("%s ANALOG1=%d, ANALOG2=%d \n", DEVICE, odan1.val, odan2.val);
    return true;
}



int lenzeReadMotorFreq(void){
    _canopen_ObjectDictionary_t odan1= {i510_2DDD_00_OD};

    // Verifies the current status
    if(canopenReadSDO(&odan1, CANOPEN_LENZE_CONTEXT)== false) return -1;
    return (int) odan1.val;
}

// Questa funzione attiva Lenze per compensare la posizione corrente
// dovuta ad una rotazione del braccio a C
// angolo: espresso in decimi di grado con il segno
// Se l'angolo richiesto è 200 o - 200 allora si intende che il
// movimento è una richiesta di parcheggio. Il braccio dunque
// viene portato all'altezza necessaria per evitare impatti con il
// basamento e viene attivata la movimentazione lenta fino al successivo
// comando automatico
//
#define _PIGRECO 3.141592
#define _LBRACCIO 166 //238 // 10x % potenziometro rispetto alla corsa 850mm:70%=198.25:L
bool lenzeActivatePositionCompensation(int angolo_iniziale, int angolo_finale){
#ifdef _CANDEVICE_SIMULATION
    return true;
#endif

    uint16_t targetPosition;
    uint16_t currentPosition;

    bool     upward_dir;
    if(generalConfiguration.lenzeCalibPark) return false;
    if(driver_stat.configured==false) return false;
    if(lenzeReadMotorFreq()!=0) return false;

    float rad_iniziale = angolo_iniziale * _PIGRECO/1800;
    float rad_finale = angolo_finale * _PIGRECO/1800;
    float Linit = _LBRACCIO*cos(rad_iniziale);
    float Lfine = _LBRACCIO*cos(rad_finale);

    // Acquisisce il valore della posizione attuale
    if(lenzeReadPosition()==false) return false;
    currentPosition = driver_stat.analog1;


    printf("LENZE ATTIVAZIONE COMPENSAZIONE: INIZIO %d, TRAGET:%d\n",angolo_iniziale, angolo_finale);
    targetPosition = currentPosition + Lfine - Linit;


    // Decide la direzione
    if(targetPosition > currentPosition) upward_dir = true;
    else upward_dir = false; // Deve scendere;

    // Controllo sui limiti
    if(targetPosition > lenzeConfig.max_lenze_position*10) targetPosition =  lenzeConfig.max_lenze_position*10;
    else if(targetPosition < lenzeConfig.min_lenze_position*10) targetPosition =  lenzeConfig.min_lenze_position*10;

    // Imposta le finestre di movimento
    if(upward_dir){
        if(targetPosition-driver_stat.analog1 < 10) return true; // Distanza troppo piccola    
        lenzeSetSpeedAuto(0,targetPosition,PRESET_AUTO);
     }else{
        if(driver_stat.analog1 - targetPosition < 10) return true; // Distanza troppo piccola
        lenzeSetSpeedAuto(targetPosition,1000,PRESET_AUTO);
    }

    // Partenza braccio
    lenzeActivateAuto(upward_dir);
    return true;
}

bool lenzeActivateUnpark(void){
#ifdef _CANDEVICE_SIMULATION
    return true;
#endif

    uint16_t targetPosition;
    uint16_t currentPosition;


    bool     upward_dir;
    if(driver_stat.configured==false) return false;
    if(lenzeReadMotorFreq()!=0) return false;

    // Acquisisce il valore della posizione attuale
    if(lenzeReadPosition()==false) return false;
    currentPosition = driver_stat.analog1;

    targetPosition =  lenzeConfig.parkingSafePoint + 50;
    printf("LENZE UNPARKING. TARGET: %d, POSITION:%d\n", targetPosition, driver_stat.analog1);

    // Controllo sui limiti
    if(targetPosition > lenzeConfig.max_lenze_position*10) targetPosition =  lenzeConfig.max_lenze_position*10;
    else if(targetPosition < lenzeConfig.min_lenze_position*10) targetPosition =  lenzeConfig.min_lenze_position*10;

    // Già in posizione di sblocco
    if(targetPosition <= currentPosition){
        printf("LENZE UNPARKING ALREADY IN POSITION. TARGET: %d, POSITION:\n", targetPosition, driver_stat.analog1);
        driver_stat.event_type = LENZE_RUN;
        driver_stat.event_code = 1; // Manual Mode
        driver_stat.event_data = driver_stat.analog1; // Posizione corrente
        _EVSET(_EV0_LENZE_EVENT);
        return true;
    }

    if(targetPosition-driver_stat.analog1 < 10) targetPosition = driver_stat.analog1 + 20;

    upward_dir = true; // Deve salire;
    lenzeSetSpeedAuto(0,targetPosition,PRESET_MANUAL);

    // Partenza braccio
    printf("LENZE UNPARKING: ACTIVATION\n");
    lenzeActivateAuto(upward_dir);
    return true;
}

bool lenzeActivatePark(void){
#ifdef _CANDEVICE_SIMULATION
    return true;
#endif

    uint16_t targetPosition;
    uint16_t currentPosition;


    bool     upward_dir;
    if(driver_stat.configured==false) return false;
    if(lenzeReadMotorFreq()!=0) return false;

    // Acquisisce il valore della posizione attuale
    if(lenzeReadPosition()==false) return false;
    currentPosition = driver_stat.analog1;

    targetPosition =  lenzeConfig.parkingTarget;
    printf("LENZE PARKING. TARGET:%d\n", targetPosition);

    // Controllo sui limiti
    if(targetPosition > lenzeConfig.max_lenze_position*10) targetPosition =  lenzeConfig.max_lenze_position*10;
    else if(targetPosition < lenzeConfig.min_lenze_position*10) targetPosition =  lenzeConfig.min_lenze_position*10;

    // Già in posizione di sblocco
    if(targetPosition >= currentPosition){
        driver_stat.event_type = LENZE_RUN;
        driver_stat.event_code = 1; // Manual Mode
        driver_stat.event_data = driver_stat.analog1; // Posizione corrente

        _EVSET(_EV0_LENZE_EVENT);
        return true;
    }

    upward_dir = false; // Deve scendere;
    lenzeSetSpeedAuto(targetPosition,1000,PRESET_PARKING);

    // Partenza braccio
    lenzeActivateAuto(upward_dir);
    return true;
}

bool lenzeSetSpeedManual(uint16_t soglia_bassa, uint16_t soglia_alta){
    if(lenzeReadMotorFreq()!=0) return false;

    printf("LENZE IN MANUAL MODE\n");

    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_01_OD, ENABLE_TRIGGER);         // ENABLE
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_02_OD, ENABLE_TRIGGER);         // RUN = ENABLE

    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2634_02_OD, i510_TRIGGER_NC);      // Spegnere uscita DIGOUT1
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_13_OD, i510_TRIGGER_NC);      // INVERT = NON USATO
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_08_OD, RUNCW_MANUAL_TRIGGER); // RUN-CW  // Direzione Down
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_09_OD, RUNCCW_MANUAL_TRIGGER);// RUN-CCW  // Direzione UP


    if(generalConfiguration.lenzeCalibPark) setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2860_01_OD, PRESET_PARKING);
    else setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2860_01_OD, PRESET_MANUAL);

    driver_stat.upmode_enable = true;
    driver_stat.dwnmode_enable = true;

    // Assegna la soglia alta e bassqa
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2636_08_OD, soglia_alta);
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2637_08_OD, soglia_bassa);

    driver_stat.manual_mode = true;
    driver_stat.manual_mode_limit = false;

    return true;
}

bool lenzeSetSpeedManualPark(bool state){
    if(lenzeReadMotorFreq()!=0) return false;

    if(state){
        printf("LENZE IN CALIBRATION PARKING MANUAL MODE\n");
        setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2860_01_OD, PRESET_PARKING);
    }else{
        printf("LENZE IN MANUAL MODE\n");
        setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2860_01_OD, PRESET_MANUAL);
    }
    return true;
}

// Imposta la modalità automatica e le relative sogloie di funzionamento
bool lenzeSetSpeedAuto(uint16_t soglia_bassa, uint16_t soglia_alta, uint32_t  speed_preset){

    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_01_OD, ENABLE_TRIGGER);         // ENABLE
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_02_OD, ENABLE_TRIGGER);         // RUN = ENABLE

    printf("ATTIVAZIONE AUTO MODE. SOGLIA-L:%d, SOGLIA-H:%d\n",soglia_bassa,soglia_alta);
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2634_02_OD, i510_TRIGGER_NC);    // Spegnere uscita DIGOUT1
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_08_OD, RUNCW_AUTO_TRIGGER); // RUN-CW
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_09_OD, i510_TRIGGER_NC);    // RUN-CCW  = NC
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2860_01_OD, speed_preset);        // SELEZIONA PRESET PER MOVIMENTO AUTOMATICO

    // Assegna la soglia alta
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2636_08_OD, soglia_alta);

    // Assegna la soglia bassa
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2637_08_OD, soglia_bassa);

    driver_stat.manual_mode = false;

    // Stato attivazione manuale disabilitato
    driver_stat.upmode_enable = false;
    driver_stat.dwnmode_enable = false;

    return true;
}


bool lenzeActivateAuto(bool upward){

    if(driver_stat.manual_mode) return false;

    if(!upward){
        setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_13_OD, i510_TRIGGER_NC);   // INVERT = NC
        setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2634_02_OD, i510_TRIGGER_TRUE);   // DIGOUT1 = ON
    }else{
        setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_13_OD, i510_TRIGGER_TRUE);   // INVERT = TRUE
        setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2634_02_OD, i510_TRIGGER_TRUE);   // DIGOUT1 = ON
    }

    return true;
}


void lenze_manage_soglie(void){
    driver_stat.gestione_soglie = false;

    if(!driver_stat.manual_mode){

        // Fine aggiustamento automatico
        printf("LENZE FINE MOVIMENTO AUTOMATICO, SET MANUAL MODE\n");
        lenzeSetSpeedManual(lenzeConfig.min_lenze_position*10, lenzeConfig.max_lenze_position*10);        

    }else{
        driver_stat.manual_mode_limit = true; // Segnala l'avvenuto raggiungimento dei limiti di movimento

        if(driver_stat.err_soglie==ERR_SOGLIA_H){
            // Abilita solo movimento verso il basso
            setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_13_OD, i510_TRIGGER_NC);   // INVERT = NC
            setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_08_OD, RUNCW_MANUAL_TRIGGER); // RUN-CW  // Direzione Down
            setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_09_OD, i510_TRIGGER_NC);// RUN-CCW  // Direzione UP
            setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2636_08_OD, 1000);    // Sblocca la soglia alta
            setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2637_08_OD,lenzeConfig.min_lenze_position*10);      // Ripristina la soglia bassa
            driver_stat.upmode_enable = false;
            driver_stat.dwnmode_enable = true;

        }else{
            // Abilita solo movimento verso alto
            setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_13_OD, i510_TRIGGER_NC);   // INVERT = NC
            setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_08_OD, i510_TRIGGER_NC); // RUN-CW  // Direzione Down
            setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_09_OD, RUNCCW_MANUAL_TRIGGER);// RUN-CCW  // Direzione UP

            setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2637_08_OD, 0); // Sblocca la soglia bassa
            setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2636_08_OD, lenzeConfig.max_lenze_position*10);  // Ripristina la  soglia alta
            driver_stat.upmode_enable = true;
            driver_stat.dwnmode_enable = false;
        }
    }
}



//  Configurazione operativa Lenze
void lenzeRunConfiguration(void){
    printf("LENZE: RUN CONFIGURATION\n");

    // In caso di cambio stato da switch_off/ switch_on
    // si verifica se ci sono le condizioni per l'attivazione
    if(lenzeConfig.calibrated==0){
        printf("%s, DRIVER DISABILITATO: NON CALIBRATO\n",DEVICE);
        driver_stat.upmode_enable = false;
        driver_stat.dwnmode_enable = false;
        if(driver_stat.run_configuration) lenzeStartupConfiguration();
        return;
    }
    if(driver_stat.configured==0){
        printf("%s, DRIVER DISABILITATO: NON CONFIGURATO\n",DEVICE);
        driver_stat.upmode_enable = false;
        driver_stat.dwnmode_enable = false;
        if(driver_stat.run_configuration) lenzeStartupConfiguration();
        return;
    }

    if(!SystemOutputs.CPU_MASTER_ENA){
        printf("%s, DRIVER DISABILITATO: NON ABILITATO\n",DEVICE);
        driver_stat.upmode_enable = false;
        driver_stat.dwnmode_enable = false;
        if(driver_stat.run_configuration) lenzeStartupConfiguration();
        return;
    }
    driver_stat.run_configuration = true;


    // Impostazione caratteristice soglie per fine movimento su analog1 e analog2
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,SETPOINT_AUTO, lenzeConfig.automatic_speed * 10); // ASSEGNA LA VELOCITA' AL SETPOINT AUTO
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,SETPOINT_MANUAL, lenzeConfig.manual_speed *  10); // ASSEGNA LA VELOCITA' AL SETPOINT MANUAL
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,SETPOINT_PARKING, lenzeConfig.parking_speed *10); // ASSEGNA LA VELOCITA' AL SETPOINT DI PARCHEGGIO


    lenzeSetSpeedManual(lenzeConfig.min_lenze_position*10, lenzeConfig.max_lenze_position*10);

    // Verifica se si trova in situazione di soglie limiti
    lenzeReadPosition();
    driver_stat.err_soglie = 0;
    if(driver_stat.analog1 > lenzeConfig.max_lenze_position*10 -10) driver_stat.err_soglie = ERR_SOGLIA_H;
    else if(driver_stat.analog1 < lenzeConfig.min_lenze_position*10 +10) driver_stat.err_soglie = ERR_SOGLIA_L;
    if(driver_stat.err_soglie) lenze_manage_soglie();

}

void lenzeStartupConfiguration(void){

    printf("LENZE: STARTUP CONFIGURATION\n");
    driver_stat.run_configuration = false;
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_04_OD, i510_TRIGGER_TRUE);      // RESET ERROR

    // Qualsiasi input di azionamento viene scollegato
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_01_OD, ENABLE_TRIGGER);         // ENABLE FUNCTION
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_02_OD, i510_TRIGGER_NC);         // RUN FUNCTION

    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_08_OD, i510_TRIGGER_NC);        // RUN-CW = NC
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_09_OD, i510_TRIGGER_NC);        // RUN-CCW = NC
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_13_OD, i510_TRIGGER_NC);        // INVERT = NC

    // Impostazione caratteristice soglie per fine movimento su analog1 e analog2
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2636_09_OD, i510_GREATER_CODE);
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2636_10_OD, i510_ERROR_TROUBLE);
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2637_09_OD, i510_LOWER_CODE);
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2637_10_OD, i510_ERROR_TROUBLE);

    // azzeramento soglie per prevenire errori iniziali
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2637_08_OD,0);      // Ripristina la soglia bassa
    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2636_08_OD,1000);   // Ripristina la  soglia alta


    setI510WriteSDO(CANOPEN_LENZE_CONTEXT,i510_2631_04_OD, i510_TRIGGER_NC);     // Reset error disabled

    driver_stat.upmode_enable = false;
    driver_stat.dwnmode_enable = false;

    return;
}


int lenzeGetVBUS(void){
    return lenzeVBUS;
}
uint32_t lenzeGetAn1(void){
    return driver_stat.analog1;
}
uint32_t lenzeGetAn2(void){
    return driver_stat.analog2;
}
uint16_t lenzeGetInternalErrors(void){
    return driver_stat.internal_errors;
}
unsigned char lenzeGetDiagnosticErrors(void){
    return driver_stat.diagnostic_errors;
}
uint32_t lenzeGetHighThreshold(void){
    _canopen_ObjectDictionary_t threshold_od= {i510_2636_08_OD};
    canopenReadSDO(&threshold_od, CANOPEN_LENZE_CONTEXT);
    return threshold_od.val;
}
uint32_t lenzeGetLowThreshold(void){
    _canopen_ObjectDictionary_t threshold_od= {i510_2637_08_OD};
    canopenReadSDO(&threshold_od, CANOPEN_LENZE_CONTEXT);
    return threshold_od.val;
}

// o1,sto,i5,i4,i3,i2,i1
uint32_t lenzeGetIO(void){
#ifdef _CANDEVICE_SIMULATION
    return 0;
#endif
    _canopen_ObjectDictionary_t in= {i510_60FD_00_OD};
    _canopen_ObjectDictionary_t out= {i510_4016_05_OD};
    _canopen_ObjectDictionary_t sto= {i510_282A_01_OD};



    canopenReadSDO(&in, CANOPEN_LENZE_CONTEXT);
    canopenReadSDO(&out, CANOPEN_LENZE_CONTEXT);
    canopenReadSDO(&sto, CANOPEN_LENZE_CONTEXT);

    uint32_t result = ((in.val>>16)&0x1F) | ((out.val&0x1)<<7);
    if(sto.val&0x4000) result|=0x20;

    return  result;
}

uint32_t lenzeGetTemp(void){
#ifdef _CANDEVICE_SIMULATION
    return 25;
#endif
    _canopen_ObjectDictionary_t temp_od= {i510_2D84_01_OD};
    canopenReadSDO(&temp_od, CANOPEN_LENZE_CONTEXT);
    return temp_od.val;
}



bool lenzeGetObstacleStat(void){
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

/*
bool isParkingMode(void){
    if(lenzeConfig.startupInParkingMode) return true;
    return false;
}
*/
void lenzeSetCommand(unsigned char command, unsigned char param){
    if(command==LENZE_UNLOCK_PARKING){
        lenzeActivateUnpark();
    }else if(command==LENZE_SET_PARKING){
        lenzeActivatePark();
    }
}
/*
void lenzeActivateParkingMode(void){
    lenzeConfig.startupInParkingMode = true;
}
*/
/*

*/
/* EOF */
