#define _ACTUATORS_C
#include "dbt_m4.h" 
#define DEVICE  "ACTUATORS"

static bool actuatorStartConfirmed = false;
#define IS_VALID_PAD (generalConfiguration.comprCfg.padSelezionato<PAD_ENUM_SIZE)

// Funzione di attivazione dei processi remoti
void actuatorsStartProcess(bool lenze, bool trx, bool arm){
    uint8_t buffer[8];

    actuatorEnaTest = false;


    // Ripete il comando fino a che lo SLAVE non conferma di averlo gestito
    actuatorStartConfirmed = false;
    while(!actuatorStartConfirmed){
        buffer[0]= ACTUATORS_START_PROCESSES;
        buffer[1]=0;
        if(lenze)   buffer[1]|=1;
        if(trx)     buffer[1]|=2;
        if(arm)     buffer[1]|=4;
        CanSendToActuatorsSlave(buffer);
        _time_delay(200);
    }
    return;
}

unsigned char pending_arm_cmd_id;

// Muove TRX di un angolo espresso in centesimi di grado
bool actuatorsTrxMove(int angolo)
{
    uint8_t buffer[8];

    // TRX non motorizzato
    if(!generalConfiguration.gantryCfg.trxMotor){
        generalConfiguration.trxExecution.completed=true;
        generalConfiguration.trxExecution.success=true;
        generalConfiguration.trxExecution.id=0;
        return true;
    }

    generalConfiguration.trxExecution.completed=false;

    // Reset della modalità di movimentazione manuale
    generalConfiguration.manual_mode_activation == _MANUAL_ACTIVATION_ARM_STANDARD;


    buffer[0]= ACTUATORS_MOVE_TRX;
    TO_LE16(&buffer[1],angolo);
    buffer[3] = CONTEXT_TRX_2D;
    buffer[4]=0;
    CanSendToActuatorsSlave(buffer);

    debugPrintI("ACTUATOR_TRX_MOVE_COMMAND",angolo);
    return true;
}

bool actuatorsActivateTrxTriggerStart(void){
    // TRX non motorizzato
    if(!generalConfiguration.gantryCfg.trxMotor){
        generalConfiguration.trxExecution.completed=true;
        generalConfiguration.trxExecution.success=true;
        generalConfiguration.trxExecution.id=0;
        return true;
    }

    return true;
}


/*
 *  Attesa con timeout del completamento del comando di
 *  posizionamento del Tubo
 */
bool actuatorsTrxWaitReady(int timeout){
    // TRX non motorizzato
    if(!generalConfiguration.gantryCfg.trxMotor){
        return true;
    }

    while(timeout--){
        if(generalConfiguration.trxExecution.faultcode) return false;
        if((generalConfiguration.trxExecution.completed)&&(generalConfiguration.trxExecution.idle)) return generalConfiguration.trxExecution.success;
        _time_delay(100);
    }

    // Fine per timeout
    return false;
}

/*
 *  Attesa con timeout del completamento del comando di
 *  posizionamento del Tubo
 */
bool actuatorsTrxActivateZeroSetting(void){
    uint8_t buffer[8];

    // TRX non motorizzato
    if(!generalConfiguration.gantryCfg.trxMotor){
        generalConfiguration.trxExecution.completed=true;
        generalConfiguration.trxExecution.success=true;
        generalConfiguration.trxExecution.id=0;
        return true;
    }

    generalConfiguration.trxExecution.completed=false;

    buffer[0]= ACTUATORS_SET_TRX_ZERO;
    CanSendToActuatorsSlave(buffer);    
    debugPrint("ACTUATOR_TRX_ZERO_COMMAND");
    return true;
}

/*
 *  Ferma un movimento del tubo
 *  tmo: 100ms unit tmo di attesa
 */
void actuatorsTrxStop(int tmo ){
    uint8_t buffer[8];

    // TRX non motorizzato
    if(!generalConfiguration.gantryCfg.trxMotor){
        generalConfiguration.trxExecution.completed=true;
        generalConfiguration.trxExecution.success=true;
        generalConfiguration.trxExecution.id=0;
        return ;
    }

    // Se il TRX è già fermo finisce subito
    if(generalConfiguration.trxExecution.run==false){
        generalConfiguration.trxExecution.completed=true;
        generalConfiguration.trxExecution.success=true;
        return ;
    }

    generalConfiguration.trxExecution.completed=false;
    buffer[0]= ACTUATORS_TRX_QUICK_STOP;
    CanSendToActuatorsSlave(buffer);
    debugPrint("ACTUATOR_TRX_QUICK_STOP_COMMAND");

    if(tmo==0) return;
    while(tmo--){
        _time_delay(100);
        if((generalConfiguration.trxExecution.completed)&&(generalConfiguration.trxExecution.idle)) return;
    }

    return ;
}



/*
 * This function causes the Lenz to Move in Auto mode to a given target
 */
void actuatorsLenzeUnpark(void){
    uint8_t buffer[8];
    generalConfiguration.armExecution.id=0; // Non comunica il fine movimento lenze
    generalConfiguration.armExecution.lenze_run=true; // Anticipa il flag di attivazione
    buffer[0]= ACTUATORS_LENZE_UNPARK;
    CanSendToActuatorsSlave(buffer);
    return ;
}

/*
 * This function causes the Lenz to Move in Auto mode to a given target
 */
void actuatorsLenzPark(void){
    uint8_t buffer[8];
    generalConfiguration.armExecution.id=0; // Non comunica il fine movimento lenze
    generalConfiguration.armExecution.lenze_run=true; // Anticipa il flag di attivazione
    buffer[0]= ACTUATORS_LENZE_PARK;
    CanSendToActuatorsSlave(buffer);
    return ;
}

/*
 *  Ferma un movimento del bracico
 */
bool actuatorsArmStop(void ){
    return true;
}


/*
 *  This function requests to move the ARM to a target angle.
 *  The admitted angle is -180° to 180°.
 *  It is intended that the negative angle is Left side.
 *
 *  The function calculates the current absolute ARM angle
 *  position based on the TRX angle and the PCB249 inclinometer angle:
 *
 */
int actuatorsArmMove(int angolo){
    uint8_t buffer[8];

    // ARM non motorizzato
    if(!generalConfiguration.gantryCfg.armMotor){
        generalConfiguration.armExecution.completed=true;
        generalConfiguration.armExecution.success=true;
        generalConfiguration.armExecution.id=0;
        return 0;
    }

    debugPrintI("ACTUATOR_ARM_MOVE_COMMAND", angolo);

    generalConfiguration.armExecution.completed=false;

    // Reset della modalità di movimento manuale
    generalConfiguration.manual_mode_activation == _MANUAL_ACTIVATION_ARM_STANDARD;

    // Lettura inclinometro PCB249 per sicurezza
    pcb249U1_readGonio();

    // Gli angoli devono essere nel formato decimi di grado!!
    buffer[0]= ACTUATORS_MOVE_ARM;
    TO_LE16(&buffer[1],generalConfiguration.armExecution.dAngolo); // Imposta l'angolo di partenza
    TO_LE16(&buffer[3], angolo * 10);   // Imposta il target Angolo di arrivo

    generalConfiguration.armExecution.valid_target = false;
    generalConfiguration.armExecution.dAngolo_target =  angolo * 10; // Preparazione per la visualizzazione in caso di successo
    CanSendToActuatorsSlave(buffer);
    return 0;
}

/*
 *  This function requests to move the ARM with push buttons
 *  The admitted angle is -180° to 180°.
 *  It is intended that the negative angle is Left side.
 *
 *  The function calculates the current absolute ARM angle
 *  position based on the TRX angle and the PCB249 inclinometer angle:
 *
 */
void actuatorsManualArmMove(unsigned char mode){
    uint8_t buffer[8];
    short angolo;

    // ARM non motorizzato
    if(!generalConfiguration.gantryCfg.armMotor){
        generalConfiguration.armExecution.completed=true;
        generalConfiguration.armExecution.success=true;
        generalConfiguration.armExecution.id=0;
        generalConfiguration.armExecution.valid_target = false;
        return;
    }

    if((generalConfiguration.trxExecution.run)||(generalConfiguration.armExecution.run)){        
        debugPrint("ACTUATOR_ARM_BUSY");
        return ;
    }

    // Rotation enable Bus Hardware test
    if(!SystemOutputs.CPU_ROT_ENA)
    {
        debugPrint("ACTUATOR_ARM_DISABLED_BY_ROTENA");
        return;
    }


    if(mode==_MANUAL_ACTIVATION_ARM_STANDARD){
        int target;
        if(generalConfiguration.biopsyCfg.connected) target = 90;
        else target = 180;

        // Verifica quale input è attivo
        if(SystemInputs.CPU_ROT_CW) angolo = target;
        else if(SystemInputs.CPU_ROT_CCW) angolo = -target;
        else return;
    }else if(mode==_MANUAL_ACTIVATION_ARM_CALIB){
        // Verifica quale input è attivo
        if(SystemInputs.CPU_ROT_CW) angolo = 10;
        else if(SystemInputs.CPU_ROT_CCW) angolo = -10;
        else return;
    }else return;

    generalConfiguration.armExecution.id = 0;
    generalConfiguration.armExecution.completed=false;

    // Gli angoli devono essere nel formato decimi di grado!!
    buffer[0]= ACTUATORS_MOVE_MANUAL_ARM;
    TO_LE16(&buffer[1],generalConfiguration.armExecution.dAngolo); // Imposta l'angolo di partenza
    TO_LE16(&buffer[3], angolo * 10);   // Imposta il target Angolo di arrivo

    buffer[5] = mode;
    debugPrintI("ACTUATOR_ARM_MANUAL_MODE_CMD",(int) mode);


    // Invalida il target e imposta un valore che sia riconoscibile come esito di un movimento manuale.
    generalConfiguration.armExecution.dAngolo_target = -3000;
    generalConfiguration.armExecution.valid_count = 1;
    generalConfiguration.armExecution.valid_target = false;
    CanSendToActuatorsSlave(buffer);
    return ;
}


/*
 *  Attivazione manuale del TRX tramite i pulsanti
 *  di rotazione manuale
 */
void actuatorsManualTrxMove(unsigned char mode){
    // TRX non motorizzato
    if(!generalConfiguration.gantryCfg.trxMotor){
        return ;
    }

    if((generalConfiguration.trxExecution.run)||(generalConfiguration.armExecution.run)){
        debugPrint("ACTUATOR_TRX_BUSY");
        return ;
    }

    // Rotation enable Bus Hardware test
    if(!SystemOutputs.CPU_PEND_ENA)
    {
        debugPrint("ACTUATOR_TRX_DISABLED_BY_PENDENA");
        return; // TRX_DISABLED_ERROR;
    }


    // Verifica quale input è attivo e imposta un range limitato di movimento +-10°
    int angolo;
    unsigned char buffer[8];
    if(SystemInputs.CPU_ROT_CW) angolo = 1000; // In centesimi di grado
    else if(SystemInputs.CPU_ROT_CCW) angolo = -1000;
    else return;

    buffer[0]= ACTUATORS_MOVE_MANUAL_TRX;
    TO_LE16(&buffer[1],angolo);
    if(mode==_MANUAL_ACTIVATION_TRX_STANDARD){
        buffer[3] = CONTEXT_TRX_2D;
        debugPrintI("ACTUATOR_TRX_MANUAL_MOVE_STANDARD",angolo);

    }else{
        buffer[3] = CONTEXT_TRX_SLOW_MOTION;
        debugPrintI("ACTUATOR_TRX_MANUAL_MOVE_SLOW",angolo);
    }
    buffer[4]=0;

    CanSendToActuatorsSlave(buffer);
    return ;
}


/*
  Funzione configuratrice
*/
bool config_trx(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len){

    if(!generalConfiguration.gantryCfg.trxMotor){
        return true;
    }

  if(setmem){    
    if(len!=sizeof(generalConfiguration.trxCfg)) return false;
    memcpy(&generalConfiguration.trxCfg,buffer,len);
  }

  // Invia i dati di configurazione al Lenze
  uint8_t data[8];
  unsigned char dim;
  int i;
  unsigned char* pData;
  unsigned char offset;

  offset = 0;
  pData = (unsigned char*) &generalConfiguration.trxCfg;

  data[0]= ACTUATORS_SET_TRX_CONFIG;
  data[1]= offset;                // Offset

  if((sizeof(trxConfig_Str)) >=5) dim=5;
  else dim = (sizeof(trxConfig_Str));
  data[2] = dim;

  for(i=0; i<dim; i++){
      data[3+i] = pData[i];
  }

  _EVCLR(_EV1_TRX_CONFIGURED);
  CanSendToActuatorsSlave(data);
  _EVWAIT_ALL(_EV1_TRX_CONFIGURED);
  return true;
}

bool config_arm(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len){

    // ARM non motorizzato
    if(!generalConfiguration.gantryCfg.armMotor){
        return true;
    }

    if(setmem){
      if(len!=sizeof(generalConfiguration.armCfg)) return false;
      memcpy(&generalConfiguration.armCfg,buffer,len);
    }


    // Invia i dati di configurazione al driver
    uint8_t data[8];
    unsigned char dim;
    int i;
    unsigned char* pData;
    unsigned char offset;

    offset = 0;
    pData = (unsigned char*) &generalConfiguration.armCfg;

    data[0]= ACTUATORS_SET_ARM_CONFIG;
    data[1]= offset;                // Offset

    if((sizeof(armConfig_Str)) >=5) dim=5;
    else dim = (sizeof(armConfig_Str));
    data[2] = dim;

    for(i=0; i<dim; i++){
        data[3+i] = pData[i];
    }

    _EVCLR(_EV1_ARM_CONFIGURED);
    CanSendToActuatorsSlave(data);
    _EVWAIT_ALL(_EV1_ARM_CONFIGURED);
    // Attende il completamento della configurazione

    return true;
}

bool config_lenze(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len){

  if(setmem){
    if(len!=sizeof(generalConfiguration.lenzeCfg)) return false;
    memcpy(&generalConfiguration.lenzeCfg,buffer,len);
  }

  // Invia i dati di configurazione al Lenze
  uint8_t data[8];
  unsigned char dim;
  int i;
  unsigned char* pData;
  unsigned char offset;

  offset = 0;
  pData = (unsigned char*) &generalConfiguration.lenzeCfg;

  data[0]= ACTUATORS_SET_LENZE_CONFIG;
  data[1]= offset;                // Offset

  if((sizeof(lenzeConfig_Str)) >=5) dim=5;
  else dim = (sizeof(lenzeConfig_Str));
  data[2] = dim;

  for(i=0; i<dim; i++){
      data[3+i] = pData[i];
  }

  debugPrint("ACTUATORS CONFIGURAZIONE LENZE\n");
  _EVCLR(_EV1_LENZE_CONFIGURED);
  CanSendToActuatorsSlave(data);
  _EVWAIT_ALL(_EV1_LENZE_CONFIGURED);

  return true;
}



void actuatorsRxFromActuators(uint8_t* data){


    switch(data[0]){

    // ____________________________________________________________________________________________________________________________________________________________
    case ACTUATORS_GET_STATUS:

        // This is the Slave answer to a status request
        if(data[BYTE_GET_STATUS_CONNECTIONS] & LENZE_CONNECTED_STATUS) generalConfiguration.lenzeConnectedStatus = true;
        else  generalConfiguration.lenzeConnectedStatus = false;

        if(data[BYTE_GET_STATUS_CONNECTIONS] & ARM_CONNECTED_STATUS) generalConfiguration.armConnectedStatus = true;
        else  generalConfiguration.armConnectedStatus = false;

        if(data[BYTE_GET_STATUS_CONNECTIONS] & TRX_CONNECTED_STATUS) generalConfiguration.trxConnectedStatus = true;
        else  generalConfiguration.trxConnectedStatus = false;

        if(data[BYTE_GET_STATUS_CONNECTIONS] & PCB240_CONNECTED_STATUS) generalConfiguration.pcb240ConnectedStatus = true;
        else  generalConfiguration.pcb240ConnectedStatus = false;


        if((data[BYTE_GET_STATUS_PCB240_MAJ]!=0)&&(data[BYTE_GET_STATUS_PCB240_MIN]!=0)){
            generalConfiguration.revisioni.pcb240.maj = data[BYTE_GET_STATUS_PCB240_MAJ];
            generalConfiguration.revisioni.pcb240.min = data[BYTE_GET_STATUS_PCB240_MIN];
        }else{
            generalConfiguration.revisioni.pcb240.maj = 0;
            generalConfiguration.revisioni.pcb240.min = 0;
        }

        if((data[BYTE_GET_STATUS_M4_MAJ]!=0)||(data[BYTE_GET_STATUS_M4_MIN]!=0)){
            generalConfiguration.revisioni.m4_slave.maj = data[BYTE_GET_STATUS_M4_MAJ];
            generalConfiguration.revisioni.m4_slave.min = data[BYTE_GET_STATUS_M4_MIN];
        }else{
            generalConfiguration.revisioni.m4_slave.maj = 0;
            generalConfiguration.revisioni.m4_slave.min = 0;
        }

        if(data[BYTE_GET_STATUS_CONNECTIONS] & SLAVE_DEVICE_CONNECTED) generalConfiguration.slaveDeviceConnected = true;
        else  generalConfiguration.slaveDeviceConnected = false;

        generalConfiguration.candevice_error_startup = data[BYTE_GET_STATUS_ERRORS];


        break;


    case ACTUATORS_START_PROCESSES: // Feedback sull'avvenuta gestione di partenza dei processi
            actuatorStartConfirmed = true;
        break;

    }
}



void actuatorsGetStatus(void){
    uint8_t buffer[8];

    buffer[0]= ACTUATORS_GET_STATUS;
    CanSendToActuatorsSlave(buffer);
    return ;
}


void actuatorsUpdateAngles(void){
    static short trx=2000;
    static short arm=2000;
    static unsigned short arm_dir=100;
    unsigned char buffer[7];

    // Se non c'e' la pendolazione l'angolo TRX è sempre 0
    if(!generalConfiguration.gantryCfg.trxMotor) generalConfiguration.trxExecution.cAngolo =0;


    generalConfiguration.armExecution.dAngolo = generalConfiguration.armExecution.dAngolo_inclinometro - (generalConfiguration.trxExecution.cAngolo / 10 );

    // Gestione dell'angolo critico a cavallo di 180. Viene memorizzato il quadrante del braccio quando si trovi in un angolo "sicuro"
    if(generalConfiguration.armCfg.direction_memory == MEM_ARM_DIR_CW){
        if((generalConfiguration.armExecution.dAngolo>-1600)&&(generalConfiguration.armExecution.dAngolo<-50)) generalConfiguration.armCfg.direction_memory = MEM_ARM_DIR_CCW;
    }else if(generalConfiguration.armCfg.direction_memory == MEM_ARM_DIR_CCW){
        if((generalConfiguration.armExecution.dAngolo>=50)&&(generalConfiguration.armExecution.dAngolo<1600)) generalConfiguration.armCfg.direction_memory = MEM_ARM_DIR_CW;
    }else{

        // Caso con direzione non definita
        if((generalConfiguration.armExecution.dAngolo>-1600)&&(generalConfiguration.armExecution.dAngolo<0)) generalConfiguration.armCfg.direction_memory = MEM_ARM_DIR_CCW;
        else if((generalConfiguration.armExecution.dAngolo>=0)&&(generalConfiguration.armExecution.dAngolo<1600)) generalConfiguration.armCfg.direction_memory = MEM_ARM_DIR_CW;
    }


    // Correzione dell'angolo se necessario
    if((generalConfiguration.armExecution.dAngolo<-1700)&&(generalConfiguration.armCfg.direction_memory==MEM_ARM_DIR_CW)) generalConfiguration.armExecution.dAngolo += 3600;
    else if((generalConfiguration.armExecution.dAngolo>1700)&&(generalConfiguration.armCfg.direction_memory==MEM_ARM_DIR_CCW)) generalConfiguration.armExecution.dAngolo -= 3600;

    // Verifica se deve avvisare il sistema del cambio stato
    short trxDAngolo = generalConfiguration.trxExecution.cAngolo/10;
    if((trx!=trxDAngolo)||(arm!=generalConfiguration.armExecution.dAngolo)||(arm_dir!=generalConfiguration.armCfg.direction_memory)){

        // Se il target è stato validato allora si passa il target altrimenti si passa il valore relativo dell'inclinometro
        if(!generalConfiguration.armExecution.valid_target) {
            TO_LE16(&buffer[0],generalConfiguration.armExecution.dAngolo);
        }else if(generalConfiguration.armExecution.dAngolo_target == -3000){
            // Esito di un movimento manuale:
            // Dopo  tot secondi l'aggiornamento viene bloccato
            if(!generalConfiguration.armExecution.valid_count) generalConfiguration.armExecution.dAngolo_target = generalConfiguration.armExecution.dAngolo;
            TO_LE16(&buffer[0],generalConfiguration.armExecution.dAngolo);
        }else {
            TO_LE16(&buffer[0],generalConfiguration.armExecution.dAngolo_target);
        }
        //TO_LE16(&buffer[0],generalConfiguration.armExecution.dAngolo);

        TO_LE16(&buffer[2],generalConfiguration.trxExecution.cAngolo);
        TO_LE16(&buffer[4],generalConfiguration.armExecution.dAngolo_inclinometro);
        buffer[6] = (unsigned char) generalConfiguration.armCfg.direction_memory;
        mccGuiNotify(1,MCC_GET_GONIO, buffer, 7);

        trx = trxDAngolo;
        arm = generalConfiguration.armExecution.dAngolo;
        arm_dir = (unsigned char) generalConfiguration.armCfg.direction_memory;
    }
}

// Funzione di gestione centralizzata delle condizioni di enable generali
// Attenzione, l'effettiva abilitazione può ulteriormente essere limitata
// da condizioni gestite dal terminale Slave.
void actuatorsManageEnables(void){
    static bool lift_drop=false;

    bool rotena = SystemOutputs.CPU_ROT_ENA;
    bool liftena = SystemOutputs.CPU_LIFT_ENA;
    bool pendena = SystemOutputs.CPU_PEND_ENA;

    if(actuatorEnaTest){
        if(!actuatorTrxEna) SystemOutputs.CPU_PEND_ENA = 0;
        else SystemOutputs.CPU_PEND_ENA = 1;
        if(!actuatorArmEna) SystemOutputs.CPU_ROT_ENA = 0;
        else SystemOutputs.CPU_ROT_ENA = 1;

        // Verifica se deve aggiornare gli IO
        if((SystemOutputs.CPU_PEND_ENA!=pendena) ||
           (SystemOutputs.CPU_ROT_ENA!=rotena) ||
           (SystemOutputs.CPU_LIFT_ENA!=liftena)){

            _mutex_lock(&output_mutex);
            _EVSET(_EV0_OUTPUT_CAMBIATI);
            _mutex_unlock(&output_mutex);
        }
        return;
    }



    // Se c'è un allarme caduta braccio, disabilita tutto
    if(SystemInputs.CPU_LIFT_DROP)  lift_drop=true; // Non ripristinabile!

    // Avverte la PCB240 circa le modalità di funzionamento del freno
    if(generalConfiguration.gantryCfg.armMotor) SystemOutputs.CPU_FRENO_MODE = false;
    else SystemOutputs.CPU_FRENO_MODE = true;

    if(lift_drop){
        SystemOutputs.CPU_ROT_ENA=false;
        SystemOutputs.CPU_LIFT_ENA=false;
        SystemOutputs.CPU_PEND_ENA=false;

        // Verifica se deve aggiornare gli IO
        if((SystemOutputs.CPU_PEND_ENA!=pendena) ||
           (SystemOutputs.CPU_ROT_ENA!=rotena) ||
           (SystemOutputs.CPU_LIFT_ENA!=liftena)){

            _mutex_lock(&output_mutex);
            _EVSET(_EV0_OUTPUT_CAMBIATI);
            _mutex_unlock(&output_mutex);
        }
        return;
    }


    // Non c'è la pendolazione
    if(!generalConfiguration.gantryCfg.trxMotor) SystemOutputs.CPU_PEND_ENA = 0;
    else SystemOutputs.CPU_PEND_ENA = 1;

    if((generalConfiguration.biopsyCfg.connected) && (!generalConfiguration.biopsyCfg.armEna)){
        // In caso di Biopsia connessa, ogni attivazione del braccio
        // e del Lenze deve essere autorizzata dallo sblocco
        SystemOutputs.CPU_ROT_ENA=0;
        SystemOutputs.CPU_LIFT_ENA=0;

    }else{

        // Rotazione e Alto/Basso dipendono dallo stato della compressione
        // Tuttavia durante una rotazione in corso si evita di bloccarla
        // per eventuali compressioni finte dovute al peso della nacchera
        if(generalConfiguration.armExecution.run){
            SystemOutputs.CPU_ROT_ENA=1;
            SystemOutputs.CPU_LIFT_ENA=1;
        }else{
            if((generalConfiguration.isInCompression)){
                SystemOutputs.CPU_ROT_ENA=0;
                SystemOutputs.CPU_LIFT_ENA=0;

            }else{
                SystemOutputs.CPU_ROT_ENA=1;                
                SystemOutputs.CPU_LIFT_ENA=1;
            }
        }
    }

#ifdef __ROT_WITH_COMPRESSION
    SystemOutputs.CPU_ROT_ENA=true;
#endif

    // Verifica se deve aggiornare gli IO
    if((SystemOutputs.CPU_PEND_ENA!=pendena) ||
       (SystemOutputs.CPU_ROT_ENA!=rotena) ||
       (SystemOutputs.CPU_LIFT_ENA!=liftena)){

        _mutex_lock(&output_mutex);
        _EVSET(_EV0_OUTPUT_CAMBIATI);
        _mutex_unlock(&output_mutex);
    }


}


//________________________________________________________________________________________________________
// HANDLER NOTIFICHE EVENTI DA PROCESSO TRX
void actuatorsRxFromArm(uint8_t* data){
    unsigned char buffer[8];
    unsigned char dim;
    int i;
    unsigned char* pData;
    unsigned char offset;
    short sval;


    switch(data[0]){

    // _______________ FEEDBACK DI EFFETTIVA ATTIVAZIONE COMANDI BRACCIO E TUBO ________________
    case ACTUATORS_MOVE_ARM_ON: // Feedback di comando di movimento accettato
        if(data[1]==1) debugPrint("ACTUATOR_ARM_ACTIVATED");
        else debugPrint("ACTUATOR_ARM_MANUAL_ACTIVATED");
        generalConfiguration.armExecution.run=true;
        break;


    // _________________ FEEDBACK DI FINE COMANDO______________________________________________
    case ACTUATORS_MOVE_ARM: // Feedback di termine movimento automatico

        generalConfiguration.armExecution.run=false;
        generalConfiguration.armExecution.completed=true;

        // Esito movimento
        if(data[1]){
            generalConfiguration.armExecution.success = false;
            debugPrintI2("ACTUATOR_ARM_ERROR: TYPE=",data[1],"CODE=",data[2]);
            generalConfiguration.armExecution.valid_target = false; // Invalida  il target: verrà visualizzato l'angolo derivato dall'inclinometro

            // Se ID==0 GuiNotify non invia, dunque occorre inviare l'errore tramite altro comando ..
            if(generalConfiguration.armExecution.id==0){
                buffer[0] = data[1]; // Codice esito proveniente dai drivers..
                buffer[1] = 0; // sub codice in caso di errore da fault
                buffer[2] = 0; // sub codice in caso di errore da fault
                buffer[3] = 0; // sub codice in caso di errore da fault
                buffer[4] = 0; // sub codice in caso di errore da fault
                mccGuiNotify(1,MCC_ARM_ERRORS,buffer,5);
            }else{
                buffer[0] = data[1]; // Codice esito proveniente dai drivers..
                buffer[1] = data[2];
                mccGuiNotify(generalConfiguration.armExecution.id,MCC_CMD_ARM,buffer,2);
                generalConfiguration.armExecution.id=0;
            }
            return;
        }

        generalConfiguration.armExecution.valid_target = true; // Angolo Valido!
        generalConfiguration.armExecution.success = true;
        debugPrint("ACTUATOR_ARM_COMPLETED");

        // Se il Lenze è ancora in movimento, allora delega il risultato al LENZE
        if(generalConfiguration.armExecution.lenze_run) return;

        buffer[0] = data[1]; // Codice esito proveniente dai drivers..
        buffer[1] = data[2];
        mccGuiNotify(generalConfiguration.armExecution.id,MCC_CMD_ARM,buffer,2);
        generalConfiguration.armExecution.id=0;
        break;


    case ACTUATORS_MOVE_MANUAL_ARM: // Feedback di termine movimento manuale

        generalConfiguration.armExecution.run=false;
        generalConfiguration.armExecution.completed=true;


        // Esito movimento
        if(data[1]){
            generalConfiguration.armExecution.success = false;            
            debugPrintI2("ACTUATOR_ARM_MANUAL_ERROR: TYPE=",data[1],"CODE=", data[2]);
            generalConfiguration.armExecution.valid_target = false;
        }else{
            generalConfiguration.armExecution.success = true;
            debugPrint("ACTUATOR_ARM_MANUAL_COMPLETED");
            generalConfiguration.armExecution.dAngolo_target = -3000;
            generalConfiguration.armExecution.valid_target = true;
        }
        break;


    case ACTUATORS_FAULT_ARM:

        buffer[0] = data[1]; // Codice esito proveniente dai drivers..
        buffer[1] = data[2]; // sub codice in caso di errore da fault
        buffer[2] = data[3]; // sub codice in caso di errore da fault
        buffer[3] = data[4]; // sub codice in caso di errore da fault
        buffer[4] = data[5]; // sub codice in caso di errore da fault
        mccGuiNotify(1,MCC_ARM_ERRORS,buffer,5);

        generalConfiguration.armExecution.run = false;
        generalConfiguration.armExecution.completed = true;
        generalConfiguration.armExecution.success = false;
        generalConfiguration.armExecution.faultcode = buffer[0];
        generalConfiguration.armExecution.faultsubcode = buffer[1];

        if(data[1]){
            debugPrintI("ACTUATOR_ARM_FAUL CODE:", (int) data[1]);
        }else{
            debugPrint("ACTUATOR_ARM_RESET_FAULT");
        }
        break;

    case ACTUATORS_SET_ARM_CONFIG:

        // Feedback di avvenuta ricezione blocco dati di configurazione
        if(data[1]+data[2]<(sizeof(armConfig_Str))){
            // Invia un altro blocco
            offset = data[1]+data[2];
            pData = (unsigned char*) &generalConfiguration.armCfg;

            buffer[0]= ACTUATORS_SET_ARM_CONFIG;
            buffer[1]= offset;                // Offset

            if((sizeof(armConfig_Str)-offset) >=5) dim=5;
            else dim = (sizeof(armConfig_Str)-offset);
            buffer[2] = dim;

            for(i=0; i<dim; i++){
                buffer[3+i] = pData[offset+i];
            }
            CanSendToActuatorsSlave(buffer);

        }else{
            // Invia blocco di termine
            buffer[0]=ACTUATORS_SET_ARM_CONFIG;
            buffer[1]=255;
            buffer[2]=0;
            CanSendToActuatorsSlave(buffer);            
            _EVSET(_EV1_ARM_CONFIGURED);
        }

        break;

    case ACTUATORS_ARM_POLLING_STATUS:

        if(data[1]==ACUATORS_ARM_POLLING_IDLE){
            // Messaggio in polling per resettare eventuali situazioni rimaste appese
            if(generalConfiguration.armExecution.valid_count) generalConfiguration.armExecution.valid_count--;

            if(generalConfiguration.armExecution.run){
                debugPrint("ACTUATOR_ARM_RESET_POLLING");
                generalConfiguration.armExecution.run=false;
                generalConfiguration.armExecution.completed = true;
                generalConfiguration.armExecution.success=false;
                if(generalConfiguration.armExecution.id){
                    buffer[0] = ARM_ERROR_UNDEFINED; // Evento non definibile
                    buffer[1] = 0;
                    mccGuiNotify(generalConfiguration.armExecution.id,MCC_CMD_ARM,buffer,2);
                }
                generalConfiguration.armExecution.id=0;
            }else{
                // Reset Fault se necessario
                buffer[0] = 0; // Codice esito proveniente dai drivers..
                buffer[1] = 0; // sub codice in caso di errore da fault
                mccGuiNotify(1,MCC_ARM_ERRORS,buffer,2);
            }
        }
        break;

    }
}

//______________________________________________________________________________________________
void actuatorsRxFromTrx(uint8_t* data){
    unsigned char buffer[8];
    unsigned char dim;
    int i;
    unsigned char* pData;
    unsigned char offset;
    short sval;


    switch(data[0]){
    case ACTUATORS_TRX_IDLE:
        debugPrint("ACTUATOR_TRX_IDLE");
        generalConfiguration.trxExecution.run=false;
        generalConfiguration.trxExecution.completed=true;
        generalConfiguration.trxExecution.idle=true;
        break;

    case ACTUATORS_MOVE_TRX_ON: // Feedback di comando di movimento accettato
        if(data[1]==1)  debugPrint("ACTUATOR_TRX_ACTIVATED");
        else  debugPrint("ACTUATOR_TRX_ZERO_ACTIVATED");

        generalConfiguration.trxExecution.run=true;
        generalConfiguration.trxExecution.idle=false;
        break;

    case ACTUATORS_MOVE_TRX: // Feedback di termine movimento automatico

        generalConfiguration.trxExecution.run=false;
        generalConfiguration.trxExecution.completed=true;
        FROM_LE16(sval,&data[3]);
        generalConfiguration.trxExecution.cAngolo = sval;

        // Esito movimento
        if(data[1]){
            generalConfiguration.trxExecution.success = false;
            debugPrintI3("ACTUATOR_TRX_ERROR. Type=",data[1],"SUB=",data[2],"ANGOLO=",generalConfiguration.trxExecution.cAngolo);

            // Se ID==0 GuiNotify non invia, dunque occorre inviare l'errore tramite altro comando ..
            if(generalConfiguration.trxExecution.id==0){
                buffer[0] = data[1]; // Codice esito proveniente dai drivers..
                buffer[1] = 0; // sub codice in caso di errore da fault
                buffer[2] = 0; // sub codice in caso di errore da fault
                buffer[3] = 0; // sub codice in caso di errore da fault
                buffer[4] = 0; // sub codice in caso di errore da fault
                mccGuiNotify(1,MCC_TRX_ERRORS,buffer,5);
                break;
            }
        }else{
            generalConfiguration.trxExecution.success = true;
            debugPrintI("ACTUATOR_TRX_COMPLETED. ANGOLO=",generalConfiguration.trxExecution.cAngolo);
        }
        buffer[0] = data[1]; // Codice esito proveniente dai drivers..
        buffer[1] = data[2]; // sub codice in caso di errore da fault
        mccGuiNotify(generalConfiguration.trxExecution.id,MCC_CMD_TRX,buffer,2);
        break;



    case ACTUATORS_MOVE_MANUAL_TRX: // Feedback di termine movimento manuale
        generalConfiguration.trxExecution.run=false;
        generalConfiguration.trxExecution.completed=true;
        FROM_LE16(sval,&data[3]);
        generalConfiguration.trxExecution.cAngolo = sval;
        generalConfiguration.trxExecution.success = true;
        debugPrintI("ACTUATOR_TRX_MANUAL_COMPLETED. ANGOLO=",generalConfiguration.trxExecution.cAngolo);
        break;

    case ACTUATORS_TRX_QUICK_STOP:
        debugPrint("ACTUATOR_TRX_QUICKSTOP_COMPLETED");

        generalConfiguration.trxExecution.run=false;
        generalConfiguration.trxExecution.completed=true;
        FROM_LE16(sval,&data[3]);
        generalConfiguration.trxExecution.cAngolo = sval;

        // Esito movimento
        if(data[1]) generalConfiguration.trxExecution.success = false;
        else generalConfiguration.trxExecution.success = true;

        break;

    case ACTUATORS_SET_TRX_ZERO:

        generalConfiguration.trxExecution.run=false;
        generalConfiguration.trxExecution.completed=true;
        FROM_LE16(sval,&data[3]);
        generalConfiguration.trxExecution.cAngolo = sval;

        // Esito movimento
        if(data[1]){
            generalConfiguration.trxExecution.success = false;
            debugPrintI("ACTUATOR_TRX_ZERO_ERROR. ANGOLO=",generalConfiguration.trxExecution.cAngolo);

        }else{
            generalConfiguration.trxExecution.success = true;
            debugPrintI("ACTUATOR_TRX_ZERO_COMPLETED. ANGOLO=",generalConfiguration.trxExecution.cAngolo);
        }

        buffer[0] = data[1]; // Codice esito proveniente dai drivers..
        buffer[1] = data[2]; // sub codice in caso di errore da fault
        mccGuiNotify(generalConfiguration.trxExecution.id,MCC_CMD_TRX,buffer,2);

        break;


    case ACTUATORS_FAULT_TRX:

        buffer[0] = data[1]; // Codice esito proveniente dai drivers..
        buffer[1] = data[2]; // sub codice in caso di errore da fault
        buffer[2] = data[3]; // sub codice in caso di errore da fault
        buffer[3] = data[4]; // sub codice in caso di errore da fault
        buffer[4] = data[5]; // sub codice in caso di errore da fault
        mccGuiNotify(1,MCC_TRX_ERRORS,buffer,5);

        generalConfiguration.trxExecution.run = false;
        generalConfiguration.trxExecution.completed = true;
        generalConfiguration.trxExecution.success = false;
        generalConfiguration.trxExecution.faultcode = buffer[0];
        generalConfiguration.trxExecution.faultsubcode = buffer[1];
        FROM_LE16(sval,&data[6]);
        generalConfiguration.trxExecution.cAngolo = sval;

        if(data[1]) debugPrintI2("ACTUATOR_TRX_FAULT. CODE=", data[1],"ANGOLO=", generalConfiguration.trxExecution.cAngolo);
        else debugPrintI("ACTUATOR_TRX_RESET_FAULT. ANGOLO=", generalConfiguration.trxExecution.cAngolo);

        break;

    case ACTUATORS_SET_TRX_CONFIG:

        // Feedback di avvenuta ricezione blocco dati di configurazione
        if(data[1]+data[2]<(sizeof(trxConfig_Str))){
            // Invia un altro blocco
            offset = data[1]+data[2];
            pData = (unsigned char*) &generalConfiguration.trxCfg;

            buffer[0]= ACTUATORS_SET_TRX_CONFIG;
            buffer[1]= offset;                // Offset

            if((sizeof(trxConfig_Str)-offset) >=5) dim=5;
            else dim = (sizeof(trxConfig_Str)-offset);
            buffer[2] = dim;

            for(i=0; i<dim; i++){
                buffer[3+i] = pData[offset+i];
            }
            CanSendToActuatorsSlave(buffer);

        }else{
            // Invia blocco di termine
            buffer[0]=ACTUATORS_SET_TRX_CONFIG;
            buffer[1]=255;
            buffer[2]=0;
            CanSendToActuatorsSlave(buffer);            
            _EVSET(_EV1_TRX_CONFIGURED);
        }


        break;

    case ACTUATORS_TRX_POLLING_STATUS:
        // Messaggio in polling per resettare eventuali situazioni rimaste appese
        if(data[1]==ACUATORS_TRX_POLLING_IDLE){
            if(generalConfiguration.trxExecution.run){
                debugPrint("ACTUATOR_TRX_RESET_POLLING");

                generalConfiguration.trxExecution.run=false;
                generalConfiguration.trxExecution.completed = true;
                generalConfiguration.trxExecution.success=false;
                FROM_LE16(sval,&data[3]);
                generalConfiguration.trxExecution.cAngolo = sval;


                buffer[0] = TRX_ERROR_UNDEFINED; // Evento non definibile
                buffer[1] = 0;
                mccGuiNotify(generalConfiguration.trxExecution.id,MCC_CMD_TRX,buffer,2);
                generalConfiguration.trxExecution.id=0;

            }else{
                // Reset Fault se necessario
                buffer[0] = 0; // Codice esito proveniente dai drivers..
                buffer[1] = 0; // sub codice in caso di errore da fault
                mccGuiNotify(1,MCC_TRX_ERRORS,buffer,2);
            }
        }
        break;

    }
}

//______________________________________________________________________________________________
void actuatorsRxFromLenze(uint8_t* data){
    unsigned char buffer[8];
    unsigned char dim;
    int i;
    unsigned char* pData;
    unsigned char offset;



    switch(data[0]){

    case ACTUATORS_FAULT_LENZE:
        break;


    case ACTUATORS_LENZE_RUN_STAT:

        if(data[1]==0){
            generalConfiguration.armExecution.lenze_run = true;
            debugPrint("ACTUATORS LENZE ATTIVATO IN MODO AUTOMATICO\n");
            return;
        }

        // Segnalazione di fine movimento lenze
        debugPrint("ACTUATORS COMUNICAZIONE LENZE FINE MOVIMENTO AUTOMATICO\n");
        generalConfiguration.armExecution.lenze_run = false;
        generalConfiguration.armExecution.lenze_pot = data[2] + 256 * data[3];

        if(generalConfiguration.armExecution.id==0) return; // Non c'è nessuna comunicazione da inviare
        if(generalConfiguration.armExecution.run) return; // Arm ancora in movimento

        // Segnalazione a GUI di completo azionamento avvenuto
        buffer[0] = 0;
        buffer[1] = 0;
        mccGuiNotify(generalConfiguration.armExecution.id,MCC_CMD_ARM,buffer,2);
        generalConfiguration.armExecution.id=0;


        break;

    case ACTUATORS_SET_LENZE_CONFIG:

            // Feedback di avvenuta ricezione blocco dati di configurazione
            if(data[1]+data[2]<(sizeof(lenzeConfig_Str))){
                // Invia un altro blocco
                offset = data[1]+data[2];
                pData = (unsigned char*) &generalConfiguration.lenzeCfg;

                buffer[0]= ACTUATORS_SET_LENZE_CONFIG;
                buffer[1]= offset;                // Offset

                if((sizeof(lenzeConfig_Str)-offset) >=5) dim=5;
                else dim = (sizeof(lenzeConfig_Str)-offset);
                buffer[2] = dim;

                for(i=0; i<dim; i++){
                    buffer[3+i] = pData[offset+i];
                }
                CanSendToActuatorsSlave(buffer);

            }else{
                // Invia blocco di termine
                buffer[0]=ACTUATORS_SET_LENZE_CONFIG;
                buffer[1]=255;
                buffer[2]=0;
                CanSendToActuatorsSlave(buffer);
                _EVSET(_EV1_LENZE_CONFIGURED);
            }

            break;
    }
}

// ______________________________________________________________________________________________

/* EOF */
   


