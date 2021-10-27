#define _BIOPSY_C
#include "dbt_m4.h" 

#define CONTEST BIOPSY_CONTEST
#define STATUS  (*((_BIOPSY_Stat_Str*)(&BIOPSY_CONTEST.Stat)))

#define _DEF_BIOPSY_HIGH        193 // Posizione needle in zero rispetto al piano deel detector
#define _DEF_MARGINE_HIGH       10 // millimetri di margine sulla posizione massima

// Definizione delays relativi agli stati
#define _BYM_DISCONNECTED_STAT_DELAY 500
#define _BYM_CONNECTED_STAT_DELAY    100
#define _BYM_ACTIVATED_STAT_DELAY    500

// Definizione macchina a stati
#define _BYM_DRIVER_STAT_DISCONNECTED 0
#define _BYM_DRIVER_STAT_CONNECTED    1
#define _BYM_DRIVER_STAT_ACTIVATED    2

// Definizione comandi di attivazione cursore
#define _BYM_NO_COMMAND             0
#define _BYM_MOVE_TO_XYZ            1
#define _BYM_MOVE_TO_HOME           2
#define _BYM_MOVE_TO_STEP_DECZ      3
#define _BYM_MOVE_TO_STEP_INCZ      4
#define _BYM_MOVE_TO_STEP_DECX      5
#define _BYM_MOVE_TO_STEP_INCX      6
#define _BYM_MOVE_TO_STEP_DECY      7
#define _BYM_MOVE_TO_STEP_INCY      8


// _________________________________________________________________________________
//  Prototipi funzioni locali
static void BIOPSY_manageDriverDisconnectedStatus(void);
static void BIOPSY_manageDriverConnectedStatus(void);
static void BIOPSY_manageDriverActivatedStatus(void);
static void BIOPSY_manageActivationLoop(void);
static bool BIOPSY_manageConsoleButtons(void);
//___________________________________________________________________________________

// _________________________________________________________________________________
//  Registri driver
static unsigned char dati[_BP_DATA_LEN];
static unsigned char chg_dati[_BP_DATA_LEN];
static unsigned char driverStatus;
static unsigned char activationCommands;
static int targetX, targetY, targetZ;    // Target di posizionamento XYZ

//___________________________________________________________________________________


void BIOPSY_driver(uint32_t taskRegisters)
{

   driverStatus = _BYM_DRIVER_STAT_DISCONNECTED;
   activationCommands = _BYM_NO_COMMAND;


    printf("ATTIVAZIONE DRIVER BIOPSY: \n");

    // API esterne al driver
    generalConfiguration.biopsyCfg.connected = FALSE;
    generalConfiguration.biopsyCfg.adapterId = 0;    
    generalConfiguration.biopsyCfg.armEna = FALSE;
    generalConfiguration.biopsyCfg.lunghezzaAgo = 0;
    generalConfiguration.biopsyCfg.stepVal = 10; // Default = 10


    // Attende la configurazione del sistema prima di procedere
    while(generalConfiguration.deviceConfigOk==FALSE) _time_delay(1000);
    
    // Inizializzazione dei campi di notifica alla GUI
    dati[_BP_MOTION]=_BP_NO_MOTION;
    dati[_BP_MOTION_END]=0;
    dati[_BP_CHKH]=0;
    dati[_BP_CHKL]=0;
    dati[_BP_REVIS]=0;
    dati[_BP_XL] = 0;
    dati[_BP_XH] = 0;
    dati[_BP_YL] = 0;
    dati[_BP_YH] = 0;
    dati[_BP_ZL] = 0;
    dati[_BP_ZH] = 0;
    dati[_BP_CONSOLE_PUSH] =  _BP_BIOP_PUSH_NO_EVENT;
    dati[_BP_PUSH_SBLOCCO] = _BP_PUSH_SBLOCCO_DISATTIVO;
    dati[_BP_ADAPTER_ID] = 0;
    dati[_BP_CONNESSIONE] = _BP_CONNESSIONE_DISCONNECTED;
    while(!mccBiopsyNotify(1,BIOP_NOTIFY_STAT,dati, sizeof(dati))) _time_delay(50);
    for(int i =0; i< _BP_DATA_LEN; i++){
        chg_dati[i] = dati[i];
    }

#ifdef __BIOPSY_SIMULATOR
    // Partenza processo gestione biopsia (opzionale)
    _task_create(0,_IDTASK(BIOPSYM),(uint32_t) NULL);

#endif


    // Gestione dello stato del sistema secondo la macchina a stati
    while(1){
        if(STATUS.freeze)
        {
           // Entra in Freeze
           _EVCLR(_EV1_BIOPSY_RUN);
           _EVSET(_EV1_BIOPSY_FREEZED); // Notifica l'avvenuto Blocco
           _EVWAIT_ANY(_MOR2(_EV1_DEVICES_RUN,_EV1_BIOPSY_RUN)); // Attende lo sblocco
           _EVSET(_EV1_BIOPSY_RUN);
           STATUS.freeze = 0;
        }
        STATUS.ready=1;
        _EVSET(_EV1_BIOPSY_RUN);

        if(driverStatus == _BYM_DRIVER_STAT_DISCONNECTED)   BIOPSY_manageDriverDisconnectedStatus();
        else if(driverStatus == _BYM_DRIVER_STAT_CONNECTED) BIOPSY_manageDriverConnectedStatus();
        else if(driverStatus == _BYM_DRIVER_STAT_ACTIVATED) BIOPSY_manageDriverActivatedStatus();
        else _time_delay(1000);

        // Verifica se bisogna notificare
        bool notifica = false;
        for(int i =0; i< _BP_DATA_LEN; i++){
            if(chg_dati[i] != dati[i]){
                chg_dati[i] = dati[i];
                notifica = true;
            }
        }

        // Effettua la notifica dello stato se necessario
        if(notifica) while(!mccBiopsyNotify(1,BIOP_NOTIFY_STAT,dati, sizeof(dati))) _time_delay(50);

    }
}


void BIOPSY_manageDriverDisconnectedStatus(void){
    _time_delay(_BYM_DISCONNECTED_STAT_DELAY);
    if(!BiopsyDriverGetStat(&generalConfiguration.biopsyCfg.statusL, &generalConfiguration.biopsyCfg.statusH, false)) return;

    // Se i motori sono attivi attende
    if(generalConfiguration.biopsyCfg.statusL & 0x01) return;

    // Con la torretta disconnessa occorre verificare se è stata resettata
    /*
    if(!generalConfiguration.biopsyCfg.connected){

        // Verifica se la torretta non è ancora stata resettata (dovrebbe esserlo!!)
        if(!(generalConfiguration.biopsyCfg.statusH & 0x80)){
            printf("BYM: EXECUTE RESET\n");
            BiopsyDriverReset();
            return;
        }
    }*/

    // Lo stato della torretta ora è di connessione, ma occorre completare ancora qualche passaggio
    generalConfiguration.biopsyCfg.connected = true;

    // Reset status
    if(!BiopsyDriverGetStat(&generalConfiguration.biopsyCfg.statusL, &generalConfiguration.biopsyCfg.statusH, true)) return;

    // Acquisizione checksum
    if(!BiopsyDriverGetChecksum(&generalConfiguration.biopsyCfg.checksum_l, &generalConfiguration.biopsyCfg.checksum_h)) return;

    // Acquisizione revisione
    if(!BiopsyDriverGetRevision(&generalConfiguration.biopsyCfg.revisione)) return;

    // Cambio stato in stato di connessione
    driverStatus = _BYM_DRIVER_STAT_CONNECTED;
    activationCommands = _BYM_NO_COMMAND;

    // Notifica PC
    dati[_BP_CONNESSIONE] = _BP_CONNESSIONE_CONNECTED;
    dati[_BP_CHKH]=generalConfiguration.biopsyCfg.checksum_h;
    dati[_BP_CHKL]=generalConfiguration.biopsyCfg.checksum_l;
    dati[_BP_REVIS]=generalConfiguration.biopsyCfg.revisione;

    printf("BYM DRIVER: STATUS CHANGE TO CONNECTED STATUS\n");
    printf("BYM REVISION: %d CHKSUM %x%x\n",generalConfiguration.biopsyCfg.revisione,generalConfiguration.biopsyCfg.checksum_h,generalConfiguration.biopsyCfg.checksum_l );
    return;
}

void BIOPSY_manageDriverConnectedStatus(void){
    static unsigned char slot = 0;

    static int timer_stat = 2000 /_BYM_CONNECTED_STAT_DELAY;
    _time_delay(_BYM_CONNECTED_STAT_DELAY);
    dati[_BP_MOTION]=_BP_NO_MOTION;


    // Evento di Timeout!!
    if(--timer_stat == 0){

        generalConfiguration.biopsyCfg.connected = false;
        driverStatus = _BYM_DRIVER_STAT_DISCONNECTED;
        dati[_BP_CONNESSIONE] = _BP_CONNESSIONE_DISCONNECTED;
        timer_stat = 2000 /_BYM_CONNECTED_STAT_DELAY;
        return;
    }

    // Calcolo massima escursione Z per evitare l'impatto con il compressore
    if(generalConfiguration.comprCfg.padSelezionato == PAD_BIOP_3D){
        int posizione_staffa_compressore = _DEVREG(RG215_DOSE,PCB215_CONTEST) - generalConfiguration.biopsyCfg.conf.offsetPad;
        dati[_BP_MAX_Z] = (generalConfiguration.biopsyCfg.conf.offsetFibra + 20) - posizione_staffa_compressore - generalConfiguration.biopsyCfg.conf.marginePosizionamento;
    }else{
        dati[_BP_MAX_Z] = (generalConfiguration.biopsyCfg.conf.offsetFibra + 20);
    }

    // Chiede lo status
    if(!BiopsyDriverGetStat(&generalConfiguration.biopsyCfg.statusL, &generalConfiguration.biopsyCfg.statusH, true)) return;

    // Verifica se il pulsante di sblocco
    if(generalConfiguration.biopsyCfg.statusH & 0x40){
        generalConfiguration.biopsyCfg.armEna = true;
        dati[_BP_PUSH_SBLOCCO] = _BP_PUSH_SBLOCCO_ATTIVO;
    }else{
        generalConfiguration.biopsyCfg.armEna = false;
        dati[_BP_PUSH_SBLOCCO] = _BP_PUSH_SBLOCCO_DISATTIVO;
    }

    // Gestione pulsanti Biopsia
    if(!BIOPSY_manageConsoleButtons()) return;

    if(slot==0){
        // Gestione needle
        if(!BiopsyDriverGetNeedle(&generalConfiguration.biopsyCfg.adapterId)) return;

        // Conversione valori NEEDLE -> IDENTIFICATORE
        if((generalConfiguration.biopsyCfg.adapterId >= _ADAPTER_NEEDLE_LEVEL_L) && (generalConfiguration.biopsyCfg.adapterId <= _ADAPTER_NEEDLE_LEVEL_L))
            dati[_BP_ADAPTER_ID] = _BP_ADAPTER_NEEDLE;
        else if((generalConfiguration.biopsyCfg.adapterId >= _ADAPTER_A_LEVEL_L) && (generalConfiguration.biopsyCfg.adapterId <= _ADAPTER_A_LEVEL_H))
            dati[_BP_ADAPTER_ID] = _BP_ADAPTER_A;
        else if((generalConfiguration.biopsyCfg.adapterId >= _ADAPTER_SHORT_LEVEL_L) && (generalConfiguration.biopsyCfg.adapterId <= _ADAPTER_SHORT_LEVEL_H))
            dati[_BP_ADAPTER_ID] = _BP_ADAPTER_SHORT;
        else if((generalConfiguration.biopsyCfg.adapterId >= _ADAPTER_OPEN_LEVEL_L) && (generalConfiguration.biopsyCfg.adapterId <= _ADAPTER_OPEN_LEVEL_H))
            dati[_BP_ADAPTER_ID] = _BP_ADAPTER_OPEN;
        else
            dati[_BP_ADAPTER_ID] = _BP_ADAPTER_B;


    }else if(slot==2){
        // Acquisisce posizione attuale
        if(!BiopsyDriverGetX(&generalConfiguration.biopsyCfg.X)) return;
        if(!BiopsyDriverGetY(&generalConfiguration.biopsyCfg.Y)) return;
        if(!BiopsyDriverGetZ(&generalConfiguration.biopsyCfg.Z)) return;

        dati[_BP_XL] = (unsigned char) (generalConfiguration.biopsyCfg.X & 0x00FF);
        dati[_BP_XH] = (unsigned char) (generalConfiguration.biopsyCfg.X >> 8);
        dati[_BP_YL] = (unsigned char) (generalConfiguration.biopsyCfg.Y & 0x00FF);
        dati[_BP_YH] = (unsigned char) (generalConfiguration.biopsyCfg.Y >> 8);
        dati[_BP_ZL] = (unsigned char) (generalConfiguration.biopsyCfg.Z & 0x00FF);
        dati[_BP_ZH] = (unsigned char) (generalConfiguration.biopsyCfg.Z >> 8);

    }else if(slot==3){
        // Impostazione ZLIMIT come minimo tra la massima z prima dell'impatto con il compressore
        // e la massima distanza prima dell'impatto dell'ago sul piano di compressione     
        dati[_BP_ZLIMIT] = dati[_BP_MAX_Z];        
        if( (generalConfiguration.biopsyCfg.conf.offsetFibra - generalConfiguration.biopsyCfg.lunghezzaAgo ) < dati[_BP_MAX_Z])
            dati[_BP_ZLIMIT] = (generalConfiguration.biopsyCfg.conf.offsetFibra - generalConfiguration.biopsyCfg.lunghezzaAgo);

        // Scrittura zlimit sul target
        if(!BiopsyDriverSetZlim((unsigned short) dati[_BP_ZLIMIT] * 10, 0 )) printf("bym: error zlimit");

        // Scrittura dello stepval
        if(!BiopsyDriverSetStepVal(generalConfiguration.biopsyCfg.stepVal, 0 )) printf("bym: error stepval");
    }

    // Tutto OK
    slot++;
    if(slot>3) slot = 0;
    timer_stat = 2000 /_BYM_CONNECTED_STAT_DELAY;

    // Gestione comandi posizionatore
    if((activationCommands != _BYM_NO_COMMAND) || (generalConfiguration.biopsyCfg.statusL & 0x01)){
        driverStatus = _BYM_DRIVER_STAT_ACTIVATED;
        dati[_BP_MOTION]=_BP_MOTION_ON;
        slot = 0;
        printf("BYM DRIVER: GESTIONE ATTIVAZIONE");
    }


    return;
}

void BIOPSY_manageDriverActivatedStatus(void){
    int tentativi;

    switch(activationCommands){
    case _BYM_MOVE_TO_XYZ:

        // caricamento target
        tentativi = 10;
        while(!BiopsyDriverSetTGX(targetX)){
            tentativi--;
            if(!tentativi){
                dati[_BP_MOTION] =_BP_MOTION_TERMINATED;
                dati[_BP_MOTION_END] =_BP_TIMEOUT_COMANDO;
                activationCommands =_BYM_NO_COMMAND;
                driverStatus = _BYM_DRIVER_STAT_CONNECTED;
                printf("BYM DRIVER: ERRORE CARICAMENTO TARGET\n");
                printf("BYM DRIVER: CAMBIO STATO TO CONNECTED STATUS\n");
                return;
            }
            _time_delay(50);
        }
        tentativi = 10;
        while(!BiopsyDriverSetTGY(targetY)){
            tentativi--;
            if(!tentativi){
                dati[_BP_MOTION] =_BP_MOTION_TERMINATED;
                dati[_BP_MOTION_END] =_BP_TIMEOUT_COMANDO;
                activationCommands =_BYM_NO_COMMAND;
                driverStatus = _BYM_DRIVER_STAT_CONNECTED;
                printf("BYM DRIVER: ERRORE CARICAMENTO TARGET\n");
                printf("BYM DRIVER: CAMBIO STATO TO CONNECTED STATUS\n");
                return;
            }
            _time_delay(50);
        }
        tentativi = 10;
        while(!BiopsyDriverSetTGZ(targetZ)){
            tentativi--;
            if(!tentativi){
                dati[_BP_MOTION] =_BP_MOTION_TERMINATED;
                dati[_BP_MOTION_END] =_BP_TIMEOUT_COMANDO;
                activationCommands =_BYM_NO_COMMAND;
                driverStatus = _BYM_DRIVER_STAT_CONNECTED;
                printf("BYM DRIVER: ERRORE CARICAMENTO TARGET\n");
                printf("BYM DRIVER: CAMBIO STATO TO CONNECTED STATUS\n");
                return;
            }
            _time_delay(50);
        }

        // Attivazione comando
        BiopsyDriverMoveXYZ(&generalConfiguration.biopsyCfg.statusL, &generalConfiguration.biopsyCfg.statusH);
        BIOPSY_manageActivationLoop();
        break;

    case _BYM_MOVE_TO_HOME:
        // Attivazione comando
        BiopsyDriverMoveHOME(&generalConfiguration.biopsyCfg.statusL, &generalConfiguration.biopsyCfg.statusH);
        BIOPSY_manageActivationLoop();
        break;


    case _BYM_MOVE_TO_STEP_DECZ:
        // Attivazione comando
        BiopsyDriverMoveDecZ(&generalConfiguration.biopsyCfg.statusL, &generalConfiguration.biopsyCfg.statusH);
        BIOPSY_manageActivationLoop();
        break;

    case _BYM_MOVE_TO_STEP_INCZ:
        // Attivazione comando
        BiopsyDriverMoveIncZ(&generalConfiguration.biopsyCfg.statusL, &generalConfiguration.biopsyCfg.statusH);
        BIOPSY_manageActivationLoop();
        break;

    case _BYM_MOVE_TO_STEP_DECX:
        // Attivazione comando
        BiopsyDriverMoveDecX(&generalConfiguration.biopsyCfg.statusL, &generalConfiguration.biopsyCfg.statusH);
        BIOPSY_manageActivationLoop();
        break;

    case _BYM_MOVE_TO_STEP_INCX:
        // Attivazione comando
        BiopsyDriverMoveIncX(&generalConfiguration.biopsyCfg.statusL, &generalConfiguration.biopsyCfg.statusH);
        BIOPSY_manageActivationLoop();
        break;

    case _BYM_MOVE_TO_STEP_DECY:
        // Attivazione comando
        BiopsyDriverMoveDecY(&generalConfiguration.biopsyCfg.statusL, &generalConfiguration.biopsyCfg.statusH);
        BIOPSY_manageActivationLoop();
        break;

    case _BYM_MOVE_TO_STEP_INCY:
        // Attivazione comando
        BiopsyDriverMoveIncY(&generalConfiguration.biopsyCfg.statusL, &generalConfiguration.biopsyCfg.statusH);
        BIOPSY_manageActivationLoop();
        break;

    default:
        activationCommands =_BYM_NO_COMMAND;
        driverStatus = _BYM_DRIVER_STAT_CONNECTED;
        printf("BYM DRIVER: NESSUN COMANDO DI ATTIVAZIONE\n");
        printf("BYM DRIVER: CAMBIO STATO TO CONNECTED STATUS\n");
        return;
    }

}

void BIOPSY_manageActivationLoop(void){
    // Attesa completamento comando: max 20 secondi
    int tentativi = 40;
    while(!BiopsyDriverGetStat(&generalConfiguration.biopsyCfg.statusL, &generalConfiguration.biopsyCfg.statusH, false)){
        _time_delay(500);
        tentativi--;
        if(tentativi == 0){
            dati[_BP_MOTION] =_BP_MOTION_TERMINATED;
            dati[_BP_MOTION_END] =_BP_TIMEOUT_COMANDO;
            activationCommands =_BYM_NO_COMMAND;
            driverStatus = _BYM_DRIVER_STAT_CONNECTED;
            printf("BYM DRIVER: TIMEOUT ATIVAZIONE\n");
            printf("BYM DRIVER: CAMBIO STATO TO CONNECTED STATUS\n");
            return;
        }
    }

    // Verifica risultato
    dati[_BP_MOTION] =_BP_MOTION_TERMINATED;
    if(generalConfiguration.biopsyCfg.statusL & 0x80) dati[_BP_MOTION_END] =_BP_ERROR_POSITIONINIG;
    else dati[_BP_MOTION_END] =_BP_POSITIONINIG_OK;

    // Azzeramento bit resettabili
    BiopsyDriverGetStat(&generalConfiguration.biopsyCfg.statusL, &generalConfiguration.biopsyCfg.statusH, true);


    // Acquisisce posizione attuale
    BiopsyDriverGetX(&generalConfiguration.biopsyCfg.X);
    BiopsyDriverGetY(&generalConfiguration.biopsyCfg.Y);
    BiopsyDriverGetZ(&generalConfiguration.biopsyCfg.Z);

    dati[_BP_XL] = (unsigned char) (generalConfiguration.biopsyCfg.X & 0x00FF);
    dati[_BP_XH] = (unsigned char) (generalConfiguration.biopsyCfg.X >> 8);
    dati[_BP_YL] = (unsigned char) (generalConfiguration.biopsyCfg.Y & 0x00FF);
    dati[_BP_YH] = (unsigned char) (generalConfiguration.biopsyCfg.Y >> 8);
    dati[_BP_ZL] = (unsigned char) (generalConfiguration.biopsyCfg.Z & 0x00FF);
    dati[_BP_ZH] = (unsigned char) (generalConfiguration.biopsyCfg.Z >> 8);

    activationCommands =_BYM_NO_COMMAND;
    driverStatus = _BYM_DRIVER_STAT_CONNECTED;
    printf("BYM DRIVER: ATIVAZIONE TERMINATA\n");
    printf("BYM DRIVER: CAMBIO STATO TO CONNECTED STATUS\n");
    return;
}

bool BIOPSY_manageConsoleButtons(void){

    static unsigned char buttons = 0;
    unsigned char new_buttons = generalConfiguration.biopsyCfg.statusH & 0x1F;

    // Considero solo i fronti positivi 0->1
    // Solo un bottone alla volta viene considerato, con una definita priorità
    if(!(buttons ^ new_buttons)){
        dati[_BP_CONSOLE_PUSH] = new_buttons;
        return true;
    }

    // Se i dati sono cambiati allora verifica se deve leggere XY campionato
    if(new_buttons & _BP_BIOP_PUSH_SEQ){

        // In caso di pulsante SEQ premuto viene anche acquisito il campionamento dei due assi X-Y del Joystic
        if(!BiopsyDriverGetJoyX(&generalConfiguration.biopsyCfg.JX)) return false;
        if(!BiopsyDriverGetJoyY(&generalConfiguration.biopsyCfg.JY)) return false;


        dati[_BP_CONSOLE_PUSH] = _BP_BIOP_PUSH_SEQ;

        // Converte la lettura in unità calibrate (decimi di millimetro)
        JoysticXYtoXY();
        dati[_BP_JXL] = generalConfiguration.biopsyCfg.dmmJX & 0xFF;
        dati[_BP_JXH] = generalConfiguration.biopsyCfg.dmmJX / 256;
        dati[_BP_JYL] = generalConfiguration.biopsyCfg.dmmJY & 0xFF;
        dati[_BP_JYH] = generalConfiguration.biopsyCfg.dmmJY / 256;
        printf("BIOPSY: <SEQ> BUTTON PRESSED -> X.Y=%d.%d\n", generalConfiguration.biopsyCfg.dmmJX ,generalConfiguration.biopsyCfg.dmmJY);

    }else printf("BIOPSY: CONSOLE BUTTON=%x\n", new_buttons);

    /*
    if( (!(buttons &  _BP_BIOP_PUSH_SEQ)) && (generalConfiguration.biopsyCfg.statusH &  _BP_BIOP_PUSH_SEQ) ){

        // In caso di pulsante SEQ premuto viene anche acquisito il campionamento dei due assi X-Y del Joystic
        if(!BiopsyDriverGetJoyX(&generalConfiguration.biopsyCfg.JX)) return false;
        if(!BiopsyDriverGetJoyY(&generalConfiguration.biopsyCfg.JY)) return false;

        printf("BIOPSY: <SEQ> BUTTON PRESSED\n");
        dati[_BP_CONSOLE_PUSH] = _BP_BIOP_PUSH_SEQ;

        // Converte la lettura in unità calibrate (decimi di millimetro)
        JoysticXYtoXY();
        dati[_BP_JXL] = generalConfiguration.biopsyCfg.dmmJX & 0xFF;
        dati[_BP_JXH] = generalConfiguration.biopsyCfg.dmmJX / 256;
        dati[_BP_JYL] = generalConfiguration.biopsyCfg.dmmJY & 0xFF;
        dati[_BP_JYH] = generalConfiguration.biopsyCfg.dmmJY / 256;
    }else  if( (!(buttons &  _BP_BIOP_PUSH_RESET)) && (generalConfiguration.biopsyCfg.statusH &  _BP_BIOP_PUSH_RESET) ){
        printf("BIOPSY: <RST> BUTTON PRESSED\n");
        dati[_BP_CONSOLE_PUSH] = _BP_BIOP_PUSH_RESET;

    }else if( (!(buttons &  _BP_BIOP_PUSH_BACK)) && (generalConfiguration.biopsyCfg.statusH &  _BP_BIOP_PUSH_BACK) ){
        printf("BIOPSY: <BACK> BUTTON PRESSED\n");
        dati[_BP_CONSOLE_PUSH] = _BP_BIOP_PUSH_BACK;

    }else if( (!(buttons &  _BP_BIOP_PUSH_AGO_1)) && (generalConfiguration.biopsyCfg.statusH &  _BP_BIOP_PUSH_AGO_1) ){
        printf("BIOPSY: <+1> BUTTON PRESSED\n");
        dati[_BP_CONSOLE_PUSH] = _BP_BIOP_PUSH_AGO_1;

    }else if( (!(buttons &  _BP_BIOP_PUSH_AGO_10)) && (generalConfiguration.biopsyCfg.statusH &  _BP_BIOP_PUSH_AGO_10) ){
        printf("BIOPSY: <+1> BUTTON PRESSED\n");
        dati[_BP_CONSOLE_PUSH] = _BP_BIOP_PUSH_AGO_10;

    }else dati[_BP_CONSOLE_PUSH] = _BP_BIOP_PUSH_NO_EVENT;

    */

    dati[_BP_CONSOLE_PUSH] = new_buttons | 0x80; // Flag di dati cambiati
    return true;
}
/*
   while(1)
   {
     _time_delay(driverDelay);
     if(STATUS.freeze)
     {
        // Entra in Freeze
        _EVCLR(_EV1_BIOPSY_RUN);
        _EVSET(_EV1_BIOPSY_FREEZED); // Notifica l'avvenuto Blocco
        _EVWAIT_ANY(_MOR2(_EV1_DEVICES_RUN,_EV1_BIOPSY_RUN)); // Attende lo sblocco
        _EVSET(_EV1_BIOPSY_RUN);
        STATUS.freeze = 0;
     }
     
     STATUS.ready=1;
     _EVSET(_EV1_BIOPSY_RUN);

     connection = BiopsyDriverGetStat();

    // Se la Biopsia risultava non connessa allora ne testa la connessione
    if(generalConfiguration.biopsyCfg.connected==false){

        // Verifica ogni tot se la periferifa risponde
        if(connection){

            // Impostazioni variabili di stato
            dati[_BP_MOTION]=0;   // Nessun movimento
            dati[_BP_MOTION_END]=0;   // Nessun fine movimento

            // Richiede revisione e checksum caricati
            BiopsyDriverGetRevision();
            dati[_BP_CHKH]=generalConfiguration.biopsyCfg.checksum_h;          // Checksum H
            dati[_BP_CHKL]=generalConfiguration.biopsyCfg.checksum_l;          // Checksum L
            dati[_BP_REVIS]=generalConfiguration.biopsyCfg.revisione;          // Revisione

            // Acquisisce le coordinate correnti
            BiopsyDriverGetZ();
            BiopsyDriverGetY();
            BiopsyDriverGetX();
            dati[_BP_XL] = (unsigned char) (generalConfiguration.biopsyCfg.needleX & 0x00FF);
            dati[_BP_XH] = (unsigned char) (generalConfiguration.biopsyCfg.needleX >> 8);
            dati[_BP_YL] = (unsigned char) (generalConfiguration.biopsyCfg.needleY & 0x00FF);
            dati[_BP_YH] = (unsigned char) (generalConfiguration.biopsyCfg.needleY >> 8);
            dati[_BP_ZL] = (unsigned char) (generalConfiguration.biopsyCfg.needleZ & 0x00FF);
            dati[_BP_ZH] = (unsigned char) (generalConfiguration.biopsyCfg.needleZ >> 8);

            // Pulsanti della console
            console_push = dati[_BP_CONSOLE_PUSH] =  _BP_BIOP_PUSH_NO_EVENT;

            // Stato pulsante di sblocco
            sbloccoReq = generalConfiguration.biopsyCfg.sbloccoReq;
            if(generalConfiguration.biopsyCfg.sbloccoReq) dati[_BP_PUSH_SBLOCCO] = _BP_PUSH_SBLOCCO_ATTIVO;
            else  dati[_BP_PUSH_SBLOCCO] = _BP_PUSH_SBLOCCO_DISATTIVO;

            // Controllo sblocco del braccio

            if(generalConfiguration.biopsyCfg.sbloccoReq) generalConfiguration.biopsyCfg.armEna = TRUE;
            else generalConfiguration.biopsyCfg.armEna = FALSE;
            actuatorsManageEnables();

            // Adattatore riconosciuto           
            dati[_BP_ADAPTER_ID] = generalConfiguration.biopsyCfg.adapterId;

            generalConfiguration.biopsyCfg.connected = TRUE;
            printf("RICONOSCIUTA BIOPSIA\n");

            dati[_BP_CONNESSIONE] = _BP_CONNESSIONE_CONNECTED; // Notifica cambio stato in connessione
            mccBiopsyNotify(1,BIOP_NOTIFY_STAT,dati, sizeof(dati));

            // Diminuisce il tempo di polling durante la connessione
            driverDelay =  _DEF_BIOPSY_DRIVER_DELAY_CONNECTED;
            timeout = _DEF_BIOPSY_DRIVER_TIMEOUT/driverDelay;
        }

        continue;
    }

    // Se c'è un comando di movimento in corso lo gestisce in maniera speciale
    if(generalConfiguration.biopsyCfg.movimento){
        manageBiopsyActivations();
        timeout = _DEF_BIOPSY_DRIVER_TIMEOUT/driverDelay;
        continue;
    }

    // Verifica Timeout
    if(!connection){
        // Verifica Timeout connessione
        if(!(timeout))
        {
           // Cambio stato
           generalConfiguration.biopsyCfg.connected = FALSE;
           printf("BIOPSIA SCOLLEGATA\n");
           dati[_BP_CONNESSIONE]=_BP_CONNESSIONE_DISCONNECTED;
           mccBiopsyNotify(1,BIOP_NOTIFY_STAT,dati, sizeof(dati));
           driverDelay =  _DEF_BIOPSY_DRIVER_DELAY_NC;
        }else  timeout--;

        continue;
    }

    // Gestione Connessione in IDLE
    dati[_BP_CONNESSIONE] = _BP_CONNESSIONE_CONNECTED;
    timeout = _DEF_BIOPSY_DRIVER_TIMEOUT/driverDelay;

    // Posizione corrente della torretta


    // Controlla se è stato premuto il pulsante di sblocco
    if(sbloccoReq!=generalConfiguration.biopsyCfg.sbloccoReq)
    {
      sbloccoReq=generalConfiguration.biopsyCfg.sbloccoReq;

      if(generalConfiguration.biopsyCfg.sbloccoReq){
          dati[_BP_PUSH_SBLOCCO] = _BP_PUSH_SBLOCCO_ATTIVO;
          printf("BIOPSIA: RICHIESTA SBLOCCO BRACCIO\n");
          generalConfiguration.biopsyCfg.armEna = TRUE;
          actuatorsManageEnables();
      }else{
          dati[_BP_PUSH_SBLOCCO] = _BP_PUSH_SBLOCCO_DISATTIVO;
          printf("BIOPSIA: RICHIESTA BLOCCO BRACCIO\n");
          generalConfiguration.biopsyCfg.armEna = FALSE;
          actuatorsManageEnables();
      }

    }

    // Pulsanti console biopsia
    manageBiopsyConsoleButtons();


    // Verifica l'accessorio Id
    if(adapterId!=generalConfiguration.biopsyCfg.adapterId)
    {
      adapterId=generalConfiguration.biopsyCfg.adapterId;
      printf("BIOPSIA: CAMBIO ACCESSORIO:%d\n",adapterId);
    }
    dati[_BP_ADAPTER_ID] = generalConfiguration.biopsyCfg.adapterId;

    // Aggiornamento dati sul limite di posizionamento meccanico(in millimetri)
    dati[_BP_MAX_Z] = generalConfiguration.biopsyCfg.conf.offsetFibra
                   + generalConfiguration.biopsyCfg.conf.offsetPad
                   - generalConfiguration.biopsyCfg.conf.marginePosizionamento
                   - _DEVREG(RG215_DOSE,PCB215_CONTEST);


    // Verifica se bisogna notificare
    notifica = false;
    for(int i =0; i< _BP_DATA_LEN; i++){
        if(chg_dati[i] != dati[i]){
            chg_dati[i] = dati[i];
            notifica = true;
        }
    }

    // Effettua la notifica dello stato se necessario
    if(notifica) mccBiopsyNotify(1,BIOP_NOTIFY_STAT,dati, sizeof(dati));


  } // while

}
*/


/*  Elettricamente la X aumenta verso destra e la Y verso il basso.
 *  Il sistema richiede che la X aumenti verso sinistra e quindi va girata.
 *
 */
void JoysticXYtoXY(void){
    generalConfiguration.biopsyCfg.dmmJX = (unsigned short) (4096- generalConfiguration.biopsyCfg.JX);
    generalConfiguration.biopsyCfg.dmmJY = (unsigned short) generalConfiguration.biopsyCfg.JY;
}


//____________________________________________________________________________________________________
// API DI PROTOCOLLO
bool biopsyMoveXYZ(unsigned short X, unsigned short Y, unsigned short Z)
{
  if(driverStatus != _BYM_DRIVER_STAT_CONNECTED) return FALSE; // E' già in corso

  targetX = X;
  targetY = Y;
  targetZ = Z;
  activationCommands = _BYM_MOVE_TO_XYZ;
  printf("BYM COMMAND: MOVE XYX, mTGX:%d, TGY:%d, TGZ:%d\n",X,Y,Z);
  return TRUE;
}

bool biopsyMoveHome(void)
{
  if(driverStatus != _BYM_DRIVER_STAT_CONNECTED) return FALSE; // E' già in corso
  activationCommands = _BYM_MOVE_TO_HOME;
  printf("BYM COMMAND: MOVE HOME\n");
  return TRUE;
}


bool  biopsyStepIncZ(void)
{
    if(driverStatus != _BYM_DRIVER_STAT_CONNECTED) return FALSE; // E' già in corso
    activationCommands = _BYM_MOVE_TO_STEP_INCZ;
    printf("BYM COMMAND: MOVE STEP INC Z\n");
    return TRUE;
}

bool  biopsyStepDecZ(void)
{  
    if(driverStatus != _BYM_DRIVER_STAT_CONNECTED) return FALSE; // E' già in corso
    activationCommands = _BYM_MOVE_TO_STEP_DECZ;
    printf("BYM COMMAND: MOVE STEP DEC Z\n");
    return TRUE;

}
bool  biopsyStepIncX(void)
{
    if(driverStatus != _BYM_DRIVER_STAT_CONNECTED) return FALSE; // E' già in corso
    activationCommands = _BYM_MOVE_TO_STEP_INCX;
    printf("BYM COMMAND: MOVE STEP INC X\n");
    return TRUE;
}

bool  biopsyStepDecX(void)
{
    if(driverStatus != _BYM_DRIVER_STAT_CONNECTED) return FALSE; // E' già in corso
    activationCommands = _BYM_MOVE_TO_STEP_DECX;
    printf("BYM COMMAND: MOVE STEP DEC X\n");
    return TRUE;

}
bool  biopsyStepIncY(void)
{
    if(driverStatus != _BYM_DRIVER_STAT_CONNECTED) return FALSE; // E' già in corso
    activationCommands = _BYM_MOVE_TO_STEP_INCY;
    printf("BYM COMMAND: MOVE STEP INC Y\n");
    return TRUE;
}

bool  biopsyStepDecY(void)
{
    if(driverStatus != _BYM_DRIVER_STAT_CONNECTED) return FALSE; // E' già in corso
    activationCommands = _BYM_MOVE_TO_STEP_DECY;
    printf("BYM COMMAND: MOVE STEP DEC Y\n");
    return TRUE;

}





/*
  Funzione configuratrice biopsia
  buffer[0]: offsetFibra
  buffer[1]: offsetPad
  buffer[2]: margine risalita compressore
  buffer[3]: margine posizionamento
*/
bool config_biopsy(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len){
  
  printf("AGGIORNAMENTO CONFIG BIOPSIA:\n");
  
  printf(" offsetFibra=%d\n",buffer[0]);
  printf(" offsetPad=%d\n",buffer[1]);
  printf(" margineRisalita=%d\n",buffer[2]);
  printf(" marginePosizionamento=%d\n",buffer[3]);
  
  generalConfiguration.biopsyCfg.conf.offsetFibra = buffer[0];
  generalConfiguration.biopsyCfg.conf.offsetPad = buffer[1];
  generalConfiguration.biopsyCfg.conf.margineRisalita = buffer[2];
  generalConfiguration.biopsyCfg.conf.marginePosizionamento = buffer[3];
  
  // L'impostazione dei margini di posizionamento richiede un aggiornamento 
  // dei parametri del compressore chge si deve adeguare a nuovi vincoli
  pcb215ForceUpdateData();
  return true;
}




/* EOF */
 

