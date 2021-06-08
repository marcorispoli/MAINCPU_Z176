
#define _ACTUATORS_C
#include "dbt_m4.h" 
#include "actuators.h"
#include "trx.h"

#define DEVICE "ACTUATORS"

void actuatorTestCommands(void);
void masterCommandExecution(void);
static void actuatorMoveArm(void);
static void actuatorMoveManualArm(void);
static void actuatorMoveManualTrx(void);

static void actuatorMoveTrx(void);
static void actuatorTrxQuickStop(void);

static _i510_Status_t* pLenzeStat;
static _PD4_Status_t*  pArmStat;
static _PD4_Status_t*  pTrxStat;
static uint32_t  actuators_event_timeout;

static void manageTrxEvents(void);
static void manageArmEvents(void);
static void manageLenzeEvents(void);
static void managePowerEvents(void);

// PROVVISORIO: comandi di test pendolazione loop
static    _trx_positioning_data_t test_trxpos;
static bool test_on=false;


#define ACTUATOR_EVENTS    (_EVBIT(_EV0_TRX_EVENT)|_EVBIT(_EV0_ARM_EVENT)|_EVBIT(_EV0_LENZE_EVENT)|_EVBIT(_EV0_POWER_EVENT)),_EVVAR(_EV0_TRX_EVENT)

// ___________________________________________________________________________
//  HANDLER EVENTI RICEVUTI DAI DEVICES A SEGUITO DI COMANDI ESEGUITI
//  O EVENTY ASINCRONI (ERRORI, ...)
// ___________________________________________________________________________
void actuators_rx_devices(uint32_t parameter){
    printf("ACTUATOR DEVICE EVENTS HANDLER STARTED\n");
    actuators_event_timeout = 0xFFFFFFFF;

    _EVCLR(_EV0_TRX_EVENT);
    _EVCLR(_EV0_ARM_EVENT);
    _EVCLR(_EV0_LENZE_EVENT);
    _EVCLR(_EV0_POWER_EVENT);

    while(1){
        if(_EVWAIT_TANY(ACTUATOR_EVENTS,actuators_event_timeout)==false){
            // Evento timeout
            printf("TIMEOUT EVENTI ACTUATOR\n");

            actuators_event_timeout = 0xFFFFFFFF;
            continue;
        }

        if(_IS_EVENT(_EV0_TRX_EVENT)){ manageTrxEvents();  _EVCLR(_EV0_TRX_EVENT);}
        if(_IS_EVENT(_EV0_ARM_EVENT)){ manageArmEvents();  _EVCLR(_EV0_ARM_EVENT);}
        if(_IS_EVENT(_EV0_LENZE_EVENT)){ manageLenzeEvents();  _EVCLR(_EV0_LENZE_EVENT);}
        if(_IS_EVENT(_EV0_POWER_EVENT)){ managePowerEvents();  _EVCLR(_EV0_POWER_EVENT);}


    }
}
// Funzione di gestione della situazione energetica della macchina
// SystemInputs.bat1V -> EnableBatteria / Tensione Batteria 1
// SystemInputs.bat2V -> Power Down/ Tensione Batteria 2


// VBAT2: 0 = POWERDOW
// VBAT1: 0 = BATTERIE DISABILITATE
void managePowerEvents(void){
    unsigned char mccbuffer[PWRMANAGEMENT_SIZE];
    static unsigned char memMccbuffer[PWRMANAGEMENT_SIZE];
    int vbus=-1;

    // Non esegue durante lo startup della macchina
    if(!SystemOutputs.CPU_MASTER_ENA) return;

    mccbuffer[PWRMANAGEMENT_STAT] = PWRMANAGEMENT_STAT_OK;

    if(SystemInputs.CPU_POWER_DOWN){
        // -----------------    Powerdown condition ----------------------- //
        isPowerdown = true;
        mccbuffer[PWRMANAGEMENT_STAT] = PWRMANAGEMENT_STAT_POWERDOWN;
    }else{
        // -----------------    Emergency condition ----------------------- //
        isPowerdown = false;
        vbus = lenzeGetVBUS();
        if((vbus!=-1)&&(vbus<100)) mccbuffer[PWRMANAGEMENT_STAT] = PWRMANAGEMENT_STAT_EMERGENCY;
        else if(!SystemInputs.CPU_MAINS_ON) mccbuffer[PWRMANAGEMENT_STAT] = PWRMANAGEMENT_STAT_BLITERS_ON;
     }

    // Caricamento della tensione del Lenze

    int fvbus = (10 * vbus * 210 / 287);
    mccbuffer[PWRMANAGEMENT_VLENZE_L] = (unsigned char) fvbus;
    mccbuffer[PWRMANAGEMENT_VLENZE_H] = (unsigned char) (fvbus>>8);


    // Caricamento della tensione della batterie in percentuale
    if((SystemInputs.CPU_BATT_DISABLED)||(SystemInputs.CPU_POWER_DOWN)){
        mccbuffer[PWRMANAGEMENT_VBAT1] = 0;
        mccbuffer[PWRMANAGEMENT_VBAT2] = 0;
    }else{
        // Conversione = (13.67/13.8) * (100/164) * x
        mccbuffer[PWRMANAGEMENT_VBAT1] = (unsigned char) (0.604 * (float) SystemInputs.bat1V );
        mccbuffer[PWRMANAGEMENT_VBAT2] = (unsigned char) (0.604 * (float) SystemInputs.bat2V );
    }

    // Solo per la stampa dello stato
    if(memMccbuffer[PWRMANAGEMENT_STAT]!=mccbuffer[PWRMANAGEMENT_STAT]){
        if(mccbuffer[PWRMANAGEMENT_STAT]==PWRMANAGEMENT_STAT_POWERDOWN)          printf("POWERDOWN CONDITION!!!\n");
        else if(mccbuffer[PWRMANAGEMENT_STAT]==PWRMANAGEMENT_STAT_EMERGENCY)     printf("EMERGENCY CONDITION!!!\n");
        else if(mccbuffer[PWRMANAGEMENT_STAT]==PWRMANAGEMENT_STAT_BLITERS_ON)     printf("BLITERS ON CONDITION!!!\n");
        else printf("POWER MANAGEMENT OK\n");
        printf("VLENZE=%d\n",vbus);
    }


    // Aggiorna la PCB240 sullo stato di EMERGENCY per gestire al meglio
    // la riattivazione dei Bliters
    if(mccbuffer[PWRMANAGEMENT_STAT]==PWRMANAGEMENT_STAT_EMERGENCY)     SystemOutputs.CPU_EMERGENCY_BUTTON = 1;
    else              SystemOutputs.CPU_EMERGENCY_BUTTON = 0;


    // Verifica lo stato degli enables rotazione e lift: disponibili solo con la potenza ok
    mccbuffer[PWRMANAGEMENT_ACTUATOR_STATUS] = 0;
    if(mccbuffer[PWRMANAGEMENT_STAT]==PWRMANAGEMENT_STAT_OK){
        if(SystemOutputs.CPU_PEND_ENA) mccbuffer[PWRMANAGEMENT_ACTUATOR_STATUS]|=ACTUATOR_STATUS_ENABLE_PEND_FLAG;
        if(SystemOutputs.CPU_ROT_ENA) mccbuffer[PWRMANAGEMENT_ACTUATOR_STATUS]|=ACTUATOR_STATUS_ENABLE_ROT_FLAG;
        if(SystemOutputs.CPU_LIFT_ENA) mccbuffer[PWRMANAGEMENT_ACTUATOR_STATUS]|=ACTUATOR_STATUS_ENABLE_LIFT_FLAG;
    }

    // Copia degli IO nel buffer
    memcpy(&mccbuffer[PWRMANAGEMENT_ACTUATOR_IO],&SystemInputs,sizeof(SystemInputs));
    memcpy(&mccbuffer[PWRMANAGEMENT_ACTUATOR_IO+sizeof(SystemInputs)],&SystemOutputs,sizeof(SystemOutputs));

    if(memcmp(mccbuffer,memMccbuffer,PWRMANAGEMENT_SIZE)!=0)
    {
        memcpy(memMccbuffer,mccbuffer,PWRMANAGEMENT_SIZE);
        mccGuiNotify(1,MCC_POWER_MANAGEMENT,mccbuffer,PWRMANAGEMENT_SIZE);
    }

    return;
    //---------------- IO -------------------------------------------------
    printf("---------- INPUTS -------------------------: \n");
    printf("VBAT1 = %d\n",SystemInputs.bat1V);
    printf("VBAT2 = %d\n",SystemInputs.bat2V);

    if(SystemInputs.CPU_XRAY_COMPLETED) printf("XRAY_COMPLETED\n");
    if(SystemInputs.CPU_DETECTOR_ON) printf("CPU_DETECTOR_ON\n");
    if(SystemInputs.CPU_COMPR_ENABLED) printf("CPU_COMPR_ENABLED\n");
    if(SystemInputs.CPU_ARM_PED_UP) printf("CPU_ARM_PED_UP\n");
    if(SystemInputs.CPU_ARM_PED_DWN) printf("CPU_ARM_PED_DWN\n");
    if(SystemInputs.CPU_ROT_CW) printf("CPU_ROT_CW\n");
    if(SystemInputs.CPU_ROT_CCW) printf("CPU_ROT_CCW\n");
    if(SystemInputs.CPU_CLOSED_DOOR) printf("CPU_CLOSED_DOOR\n");

    if(SystemInputs.CPU_POWER_DOWN) printf("CPU_POWER_DOWN\n");

    if(SystemInputs.CPU_UPS_FAULT) printf("CPU_UPS_FAULT\n");
    if(SystemInputs.CPU_MAINS_ON) printf("CPU_MAINS_ON\n");
    if(SystemInputs.CPU_XRAY_REQ) printf("CPU_XRAY_REQ\n");
    if(SystemInputs.CPU_REQ_POWER_OFF) printf("CPU_REQ_POWER_OFF\n");
    if(SystemInputs.CPU_BATT_DISABLED) printf("CPU_BATT_DISABLED\n");
    if(SystemInputs.CPU_LIFT_DROP) printf("CPU_LIFT_DROP\n");
    if(SystemInputs.CPU_LIFT_ENABLED) printf("CPU_LIFT_ENABLED\n");
    if(SystemInputs.CPU_EXT_ROT_ENA) printf("CPU_EXT_ROT_ENA\n");
    if(SystemInputs.CPU_XRAY_ENA_ACK) printf("CPU_XRAY_ENA_ACK\n");
    if(SystemInputs.CPU_LENZ_PED_FAULT) printf("CPU_LENZ_PED_FAULT\n");
    if(SystemInputs.CPU_ARM_PED_FAULT) printf("CPU_ARM_PED_FAULT");
    if(SystemInputs.CPU_XRAYPUSH_FAULT) printf("CPU_XRAYPUSH_FAULT");


    printf("\n ------------ OUTPUTS ------------: \n");
    if(SystemOutputs.CPU_MASTER_ENA) printf("CPU_MASTER_ENA\n");

    if(SystemOutputs.CPU_LIFT_ENA) printf("CPU_LIFT_ENA\n");
    if(SystemOutputs.CPU_ROT_ENA) printf("CPU_ROT_ENA\n");
    if(SystemOutputs.CPU_PEND_ENA) printf("CPU_PEND_ENA\n");
    if(SystemOutputs.CPU_COMPRESSOR_ENA) printf("CPU_COMPRESSOR_ENA\n");
    if(SystemOutputs.CPU_XRAY_ENA) printf("CPU_XRAY_ENA\n");
    if(SystemOutputs.CPU_BURNING) printf("CPU_BURNING\n");
    if(SystemOutputs.CPU_LOADER_PWR_ON) printf("CPU_LOADER_PWR_ON\n");
    if(SystemOutputs.CPU_LMP_SW1) printf("CPU_LMP_SW1\n");
    if(SystemOutputs.CPU_LMP_SW2) printf("CPU_LMP_SW2\n");
    if(SystemOutputs.CPU_XRAY_LED) printf("CPU_XRAY_LED\n");
    if(SystemOutputs.CPU_DEMO_ACTIVATION) printf("CPU_DEMO_ACTIVATION\n");

    printf("-------------------------------------------: \n");

}

void manageTrxEvents(void){
    unsigned char event_type = pTrxStat->event_type;    // Tipo di evento
    uint32_t      event_code = pTrxStat->event_code;    // Codice evento
    uint32_t      event_data = pTrxStat->event_data;    // dato associato all'evento
    uint32_t val;
    unsigned char buffer[8];


    switch(event_type){
    case TRX_IDLE:
        buffer[0]= ACTUATORS_TRX_IDLE;
        sendActuatorFrameToMaster(buffer);
        break;

    case TRX_POLLING_STATUS:
        buffer[0]= ACTUATORS_TRX_POLLING_STATUS;
        buffer[1]= event_code;
        buffer[2] = 0;
        TO_LE16(&buffer[3],pTrxStat->dAngolo);    // Aggiunge il valore dell'angolo corrente
        sendActuatorFrameToMaster(buffer);
        break;
    case TRX_ZERO_SETTING:
        printf("ZERO SETTING TERMINATO: esito=%d, data=%d\n",event_code,event_data);
        buffer[0]= ACTUATORS_SET_TRX_ZERO;
        buffer[1]= event_code;  // Esito comando
        buffer[2] = event_data; // Dato associato all'esito
        TO_LE16(&buffer[3],pTrxStat->dAngolo);    // Aggiunge il valore dell'angolo finale
        sendActuatorFrameToMaster(buffer);
        break;

    case TRX_MOVE_WITH_TRIGGER:
    case TRX_MOVE_TO_POSITION:
        val = getTrxPosition(); // Legge l'angolo del Tubo

        printf("TRX: POSIZIONAMENTO TERMINATO: esito=%d, data=%d\n",event_code,event_data);
        buffer[0]= ACTUATORS_MOVE_TRX;
        buffer[1]= event_code;  // Esito comando
        buffer[2]= event_data;
        TO_LE16(&buffer[3],val);    // Aggiunge il valore dell'angolo finale
        sendActuatorFrameToMaster(buffer);
        break;

    case TRX_MANUAL_MOVE_TO_POSITION:
        val = getTrxPosition(); // Legge l'angolo del Tubo
        printf("POSIZIONAMENTO MANUALE TERMINATO:%d\n", pTrxStat->dAngolo);
        buffer[0]= ACTUATORS_MOVE_MANUAL_TRX;
        buffer[1]= event_code;  // Esito comando
        buffer[2]= event_data;
        TO_LE16(&buffer[3],val);    // Aggiunge il valore dell'angolo finale
        sendActuatorFrameToMaster(buffer);
        break;

    case TRX_QUICK_STOP:
        val = getTrxPosition(); // Legge l'angolo del Tubo
        printf("QUICK STOP TERMINATO: posizione=%d\n",pTrxStat->dAngolo);
        buffer[0]= ACTUATORS_TRX_QUICK_STOP;
        buffer[1]= event_code;  // Esito comando
        buffer[2]= event_data;  // Angolo finale
        TO_LE16(&buffer[3],val);    // Aggiunge il valore dell'angolo finale
        sendActuatorFrameToMaster(buffer);
        break;
    case TRX_RUN: // Feedback di movimento iniziato
        buffer[0]= ACTUATORS_MOVE_TRX_ON;
        buffer[1]= event_code;
        buffer[2]= event_data;
        sendActuatorFrameToMaster(buffer);
        break;

    case TRX_FAULT:

        // Aggiornamento stato di Fault dispositivo
        buffer[0]= ACTUATORS_FAULT_TRX;
        buffer[1]= event_code;
        buffer[2]= (unsigned char) event_data;
        buffer[3]= (unsigned char) (event_data>>8);
        buffer[4]= (unsigned char) (event_data>>16);
        buffer[5]= (unsigned char) (event_data>>24);

        val = getTrxPosition(); // Legge l'angolo del Tubo
        printf("TRX FAULT: Posizione:%x\n",val);
        TO_LE16(&buffer[6],val);    // Aggiunge il valore dell'angolo corrente
        sendActuatorFrameToMaster(buffer);
        break;

    }


    return;
}

void manageArmEvents(void){

    unsigned char event_type = pArmStat->event_type;    // Tipo di evento
    uint32_t      event_code = pArmStat->event_code;    // Codice evento
    uint32_t      event_data = pArmStat->event_data;    // dato associato all'evento
    unsigned char buffer[8];


    switch(event_type){
    case ARM_POLLING_STATUS:
        buffer[0]= ACTUATORS_ARM_POLLING_STATUS;
        buffer[1]= event_code;
        buffer[2] = event_data;
        sendActuatorFrameToMaster(buffer);
        break;

    case ARM_MOVE_TO_POSITION:
        printf("POSIZIONAMENTO TERMINATO: esito=%d, data=%d\n",event_code,event_data);
        buffer[0]= ACTUATORS_MOVE_ARM;
        buffer[1]= event_code;  // Esito comando
        buffer[2] = event_data; // Dato associato all'esito
        sendActuatorFrameToMaster(buffer);
        break;

    case ARM_MOVE_MANUAL:
        printf("POSIZIONAMENTO MANUALE TERMINATO: esito=%d, data=%d\n",event_code,event_data);
        buffer[0]= ACTUATORS_MOVE_MANUAL_ARM;
        buffer[1]= event_code;  // Esito comando
        buffer[2] = event_data; // Dato associato all'esito
        sendActuatorFrameToMaster(buffer);
        break;

    case ARM_RUN: // Feedback di movimento iniziato
        buffer[0]= ACTUATORS_MOVE_ARM_ON;
        buffer[1]= event_code;
        buffer[2]= event_data;
        sendActuatorFrameToMaster(buffer);
        break;

    case ARM_FAULT:

        // Aggiornamento stato di Fault dispositivo
        buffer[0]= ACTUATORS_FAULT_ARM;
        buffer[1]= event_code;
        buffer[2]= (unsigned char) event_data;
        buffer[3]= (unsigned char) (event_data>>8);
        buffer[4]= (unsigned char) (event_data>>16);
        buffer[5]= (unsigned char) (event_data>>24);
        sendActuatorFrameToMaster(buffer);
    }

    return;
}

void manageLenzeEvents(void){

    unsigned char event_type = pLenzeStat->event_type;    // Tipo di evento
    uint32_t      event_code = pLenzeStat->event_code;    // Codice evento
    uint32_t      event_data = pLenzeStat->event_data;    // dato associato all'evento
    unsigned char buffer[8];


    switch(event_type){

    case LENZE_FAULT:
        // Il Master non è interessato a quest'evento.
        // Viene solo mandato allo Slave la Notifica del Fault in corso
        //buffer[0]= ACTUATORS_FAULT_LENZE;
        //sendActuatorFrameToMaster(buffer);
        buffer[0]= event_code;
        buffer[1]= (unsigned char) event_data;
        buffer[2]= (unsigned char) (event_data>>8);
        buffer[3]= (unsigned char) (event_data>>16);
        buffer[4]= (unsigned char) (event_data>>24);
        mccGuiNotify(1,MCC_LENZE_ERRORS,buffer,5);
    break;
    case LENZE_POT_UPDATE:
        // Aggiorna il potenziometro verso terminale SLAVE
        buffer[0] = (unsigned char) event_code;    // ANALOG 1 (\millesimi)
        buffer[1] = (unsigned char) (event_code>>8);
        buffer[2] = (unsigned char) event_data;    // ANALOG 2 (\millesimi)
        buffer[3] = (unsigned char) (event_data>>8);
        mccGuiNotify(1,MCC_CALIB_LENZE,buffer,4);
    break;

    case LENZE_RUN:
        buffer[0]= ACTUATORS_LENZE_RUN_STAT;
        buffer[1]= event_code; // 1 = MANUALE, 0=AUTOMATICO
        sendActuatorFrameToMaster(buffer);
        break;

    }

    return;
}


// ___________________________________________________________________________
//  HANDLER MESSAGGI RICEVUTI DA MASTER
//
// ___________________________________________________________________________
void actuators_rx_master(uint32_t parameter){
    flexcan_mb_t masterFrame;

    // Acquisisce lo stato dei drivers
    pLenzeStat  = lenzeGetStatus();
    pArmStat    = armGetStatus();
    pTrxStat    = trxGetStatus();

    printf("ACTUATOR MASTER MESSAGES HANDLER STARTED\n");

    // Init command mutex
    if (_mutex_init(&actuatorCommand.command_mutex, NULL) != MQX_OK)
    {
      printf("%s: Mutex Init failed!!\n",DEVICE);
      _mqx_exit(-1);
    }


    while(1){

        // Verifies if the Master requested the execution of a task
        if(flexcan_test_receive(MB_RX_ACTUATOR_FROM_MASTER,&masterFrame)==true){
            actuatorCommand.busy=true;
            actuatorCommand.data = masterFrame.data;
            masterCommandExecution();
        }
    }
}


//______________________________________________________________________________________________
// The following functions are requested from the MASTER TERMINAL
void masterCommandExecution(void){
    uint8_t buffer[8];
    unsigned char* pData;
    int i;

    // Verify the command to be executed
    switch((actuatorEnumCommands_t) actuatorCommand.data[0]){
    case ACTUATORS_CMD_TEST:
        printf("comando ricevuto\n");
        actuatorTestCommands();
        break;


    case ACTUATORS_MOVE_ARM:
        /*
         *  Activates the ARM positioning
         *  ActuatoCommand.data[] description:
         *
         *  data[1:2] = init position in 0.1°/unit. type short.
         *  data[2:3] = target position in 0.1°/unit.  type short.
         */
          printf("COMANDO ARM\n");
        actuatorMoveArm(); // Attivazione Braccio: eventuali errori sono segnalati all'interno
        break;

    case ACTUATORS_MOVE_MANUAL_ARM:
        /*
         *  Activates the ARM positioning
         *  ActuatoCommand.data[] description:
         *
         *  data[1:2] = init position in 0.1°/unit. type short.
         *  data[3:4] = target position in 0.1°/unit.  type short.
         *  data[5] = modo calib/standard
         */
        printf("COMANDO MANUAL ARM\n");
        actuatorMoveManualArm(); // Attivazione Braccio: eventuali errori sono segnalati all'interno
        break;

    case ACTUATORS_MOVE_TRX:
        /*
         *  Activates the TRX positioning
         *  ActuatoCommand.data[] description:
         *
         *  data[1:2]   = target position in 0.1°/unit. type short.
         *  data[3]     = context index 0,1,2,3
         *  data[4] =   EXP_WIN MODE
         */
        actuatorMoveTrx(); // Attivazione tubo: eventuali errori sono segnalati all'interno
        break;

    case ACTUATORS_MOVE_MANUAL_TRX:
        /*
         *  Activates the TRX positioning
         *  ActuatoCommand.data[] description:
         *
         *  data[1:2]   = target position in 0.1°/unit. type short.
         *  data[3]     = context index 0,1,2,3
         *  data[4] =   EXP_WIN MODE
         */
        printf("COMANDO MANUAL TRX\n");
        actuatorMoveManualTrx(); // Attivazione tubo: eventuali errori sono segnalati all'interno
        break;

    case ACTUATORS_SET_TRX_ZERO:
        /*
         *  Activates the TRX positioning
         *  ActuatoCommand.data[] description:
         *
         *  data[1:2]   = target position in 0.1°/unit. type short.
         *  data[3]     = context index 0,1,2,3
         *  data[4] =   EXP_WIN MODE
         */
        printf("COMANDO ZERO SETTING TRX\n");
        trxSetCommand(TRX_ZERO_SETTING,null);
        break;


    case ACTUATORS_TRX_QUICK_STOP:
        actuatorTrxQuickStop();
        break;

    case ACTUATORS_GET_STATUS:

        // The following command requests the actuator status.
        buffer[0]= ACTUATORS_GET_STATUS;
        buffer[BYTE_GET_STATUS_CONNECTIONS]= 0;
        if(generalConfiguration.deviceConnected)  buffer[BYTE_GET_STATUS_CONNECTIONS]|= SLAVE_DEVICE_CONNECTED;
        if(generalConfiguration.lenzeConnected) buffer[BYTE_GET_STATUS_CONNECTIONS]|= LENZE_CONNECTED_STATUS;
        if(generalConfiguration.armConnected) buffer[BYTE_GET_STATUS_CONNECTIONS]|= ARM_CONNECTED_STATUS;
        if(generalConfiguration.trxConnected) buffer[BYTE_GET_STATUS_CONNECTIONS]|= TRX_CONNECTED_STATUS;
        if(generalConfiguration.pcb240connected) buffer[BYTE_GET_STATUS_CONNECTIONS]|= PCB240_CONNECTED_STATUS;

        buffer[BYTE_GET_STATUS_PCB240_MAJ] = revPCB240.maj;
        buffer[BYTE_GET_STATUS_PCB240_MIN] = revPCB240.min;
        buffer[BYTE_GET_STATUS_M4_MAJ] = (unsigned char) REVMAJ;
        buffer[BYTE_GET_STATUS_M4_MIN] = (unsigned char) REVMIN;

        // Aggiunta byte di info errori in corso per agevolare lo startup
        buffer[BYTE_GET_STATUS_ERRORS] = 0;
        if((lenzeGetStatus()->internal_errors == 0x3222)||(lenzeGetStatus()->internal_errors == 0x3221)||(lenzeGetStatus()->internal_errors == 0x3220)) buffer[BYTE_GET_STATUS_ERRORS]|=1; // Undervoltage detection

        sendActuatorFrameToMaster(buffer);
        break;

    case ACTUATORS_START_PROCESSES:
        // Invia il feedback di avvenuta partenza processi
        if(actuatorCommand.data[1]&0x1)  generalConfiguration.lenzeDriver = true;
        else generalConfiguration.lenzeDriver = false;
        if(actuatorCommand.data[1]&0x2)  generalConfiguration.trxDriver = true;
        else generalConfiguration.trxDriver = false;
        if(actuatorCommand.data[1]&0x4)  generalConfiguration.armDriver = true;
        else generalConfiguration.armDriver = false;

        buffer[0]= ACTUATORS_START_PROCESSES;
        sendActuatorFrameToMaster(buffer);
        mainStartProcesses();

        break;
    case ACTUATORS_SET_LENZE_CONFIG:

        // The following command sets the configuration of the Lenze device
        lenzeUpdateConfiguration(actuatorCommand.data);
        break;

    case ACTUATORS_SET_ARM_CONFIG:
        memcpy(buffer,actuatorCommand.data,8);
        // data[0] = comando; data[1]= offset; data[2]=size
        if(actuatorCommand.data[1]==255){

            // Configurazione completata
            armUpdateConfiguration();
        }else{

            pArmStat->configured = false;
            pData = (unsigned char*) &armConfig;
            for(i=0; i<actuatorCommand.data[2];i++){
                pData[actuatorCommand.data[1]+i] = actuatorCommand.data[3+i];
            }

            // Feedback al Master
            sendActuatorFrameToMaster(buffer);

        }

        break;

    case ACTUATORS_SET_TRX_CONFIG:
        memcpy(buffer,actuatorCommand.data,8);
        // data[0] = comando; data[1]= offset; data[2]=size
        if(actuatorCommand.data[1]==255){

            // Configurazione completata
            trxUpdateConfiguration();
        }else{

            pTrxStat->configured = false;
            pData = (unsigned char*) &trxConfig;
            for(i=0; i<actuatorCommand.data[2];i++){
                pData[actuatorCommand.data[1]+i] = actuatorCommand.data[3+i];
            }

            // Feedback al Master
            sendActuatorFrameToMaster(buffer);

        }

        break;


    default:
        break;
    }

    // Unlock the command
    actuatorCommand.busy=false;
    return;
}

uint32_t getU32val(unsigned char* buf){
    return buf[0] + 256 * buf[1] + 256 * 256 * buf[2] + 256 * 256 * 256 * buf[3];
}

/*
 *  data[0]=MASTER_CMD_TEST
 *  data[1]=<command code>
 *  data[2,3]=(ushort) param1
 *  data[4,5,6,7]=(uint) param2
 *
 */
void actuatorTestCommands(void){
    int target;
    unsigned char index;
    unsigned short usval,usval1;

    _arm_positioning_data_t position;
    _trx_positioning_data_t trxpos;

    switch(actuatorCommand.data[1]){
    case 1: // Reset TRX Module Software
        trxResetModule();
        break;


    case 3: // Zero setting non blocking
       trxSetCommand(TRX_ZERO_SETTING,null);
       break;

    case 4: // Trx Position
        trxpos.targetPosition = (short) (actuatorCommand.data[2]+256*actuatorCommand.data[3]);
        trxpos.contextIndex = actuatorCommand.data[4];
        printf("%s: TARGET:%d index=%d\n",DEVICE,trxpos.targetPosition,trxpos.contextIndex);

        trxSetCommand(TRX_MOVE_TO_POSITION, (void*) &trxpos);


       break;

    case 5: // Arm Position

        // Data in 0.1°/unit format signed
        FROM_LE16(usval,&actuatorCommand.data[2]);
        position.initPosition = -1*(short) usval;

        FROM_LE16(usval,&actuatorCommand.data[4]);
        position.targetPosition = -1*(short) usval;

        armSetCommand(ARM_MOVE_TO_POSITION, (void*) &position);
        break;

     case 6:
        armSetCommand(ARM_IDLE, 0); // Non blocking command
        break;

    case 7:
        // Legge il potenziometro della posizione
        lenzeReadPosition();
        break;

    case 8:
        // Attivazione automatica
        // Data in 0.1°/unit format signed
        FROM_LE16(usval,&actuatorCommand.data[2]);
        FROM_LE16(usval1,&actuatorCommand.data[4]);
        lenzeActivatePositionCompensation((int) usval, (int) usval1);

        break;

    case 9:
        // Legge il potenziometro della posizione
        _EVSET(_EV0_TRX_EVENT);
        break;
    case 10:
        // Legge il potenziometro della posizione
        _EVSET(_EV0_ARM_EVENT);
        break;
    case 11:
        // Legge il potenziometro della posizione
        _EVSET(_EV0_LENZE_EVENT);
        break;

    case 12:
        // Legge il potenziometro della posizione
        _EVSET(_EV0_LENZE_EVENT);
        _EVSET(_EV0_ARM_EVENT);
        break;

    case 13: //TRX WITH TRIGGER

        trxpos.targetPosition = (short) (actuatorCommand.data[2]+256*actuatorCommand.data[3]);
        trxpos.contextIndex = actuatorCommand.data[4];
        printf("%s: TRIGGER TRX TEST CMD- TARGET:%d index=%d\n",DEVICE,trxpos.targetPosition,trxpos.contextIndex);

        trxSetCommand(TRX_MOVE_WITH_TRIGGER, (void*) &trxpos);
        break;
    case 20:
        if(test_on){
            test_on=false;
            printf("FINE TEST TRX\n");
            return;
        }

        test_trxpos.targetPosition = (short) (actuatorCommand.data[2]+256*actuatorCommand.data[3]);
        test_trxpos.contextIndex = actuatorCommand.data[4];
        test_on = true;
        trxSetCommand(TRX_MOVE_TO_POSITION, (void*) &test_trxpos);

        break;


    }
}

/*
 *  Activates the ARM positioning
 *  ActuatoCommand.data[] description:
 *  data[0] = reserved
 *  data[1:2] = init position in 0.1°/unit. type short.
 *  data[2:3] = target position in 0.1°/unit.  type short.
 */
void actuatorMoveArm(void){
    _arm_positioning_data_t position;

    // Data in 0.1°/unit format signed
    FROM_LE16(position.initPosition,&actuatorCommand.data[1]);
    FROM_LE16(position.targetPosition,&actuatorCommand.data[3]);
    position.initPosition = -1 *position.initPosition;
    position.targetPosition = -1 * position.targetPosition;
    armSetCommand(ARM_MOVE_TO_POSITION, (void*) &position);

}

// data[5] = modo di azionamento
void actuatorMoveManualArm(){
    _arm_positioning_data_t position;

    // Data in 0.1°/unit format signed
    FROM_LE16(position.initPosition,&actuatorCommand.data[1]);
    FROM_LE16(position.targetPosition,&actuatorCommand.data[3]);
    position.initPosition = -1 *position.initPosition;
    position.targetPosition = -1 * position.targetPosition;
    position.mode = actuatorCommand.data[5];
    armSetCommand(ARM_MOVE_MANUAL, (void*) &position);

}
/*
 *  Activates the TRX positioning
 *  ActuatoCommand.data[] description:
 *  data[0]     = reserved
 *  data[1:2]   = target position in 0.1°/unit. type short.
 *  data[3]     = context index 0,1,2,3
 *  data[4]     = attivazione con o senza EXP-WIN
 */
void actuatorMoveTrx(void){
    _trx_positioning_data_t position;

    // Data in 0.1°/unit format signed
    FROM_LE16(position.targetPosition,&actuatorCommand.data[1]);
    position.contextIndex = actuatorCommand.data[3];

    if(actuatorCommand.data[4]==0)
        trxSetCommand(TRX_MOVE_TO_POSITION, (void*) &position);
    else
        trxSetCommand(TRX_MOVE_WITH_TRIGGER, (void*) &position);

    return;
}


/*
 *  Activates the TRX positioning by push buttonns CW and CCW
 *  ActuatoCommand.data[] description:
 *  data[0]     = reserved
 *  data[1:2]   = target position in 0.1°/unit. type short.
 *  data[3]     = context index 0,1,2,3
 *
 */
void actuatorMoveManualTrx(void){
    _trx_positioning_data_t position;

    // Data in 0.1°/unit format signed
    FROM_LE16(position.targetPosition,&actuatorCommand.data[1]);
    position.contextIndex = actuatorCommand.data[3];
    trxSetCommand(TRX_MANUAL_MOVE_TO_POSITION, (void*) &position);

    return;
}



void actuatorTrxQuickStop(void){

    trxSetCommand(TRX_QUICK_STOP, (void*) null);

    return;
}


/* EOF */
