#define _GUI_C
#include "dbt_m4.h"
#include "vmusic3.h"


// Registri privati
_MccFrame_Str mcc_cmd;          // Comandi ricevuti da GUI
_DeviceAppRegister_Str listaRegistri; // Lista di compilazione registri
MCC_MEM_SIZE mcc_len;                 // Dimensione messaggio ricevuto

//_RxStdSeq_Str rx_param; // Usato per passare i parametri di esposizione

MCC_ENDPOINT  ep={_DEF_MCC_GUI_TO_M4_MASTER};

//////////////////////////////////////////////////////////////////////////////
/*
void gui_interface_task(uint32_t initial_data)

  TASK di gestione comandi provenienti dalla GUI su core A5
  


Autore: M. Rispoli
Data: 30/09/2014
*/
//////////////////////////////////////////////////////////////////////////////
void gui_interface_task(uint32_t initial_data)
{
  
  printf("GUI INTERFACE STARTED\n");
  
  // Creazione Endpoint per ricezione comandi da GUI
  if(mccOpenPort(&ep)==FALSE)
  {
    printf ("GUI INTERFACE: MEMORIA NON DISPONIBILE PER END POINT\n");
    _mqx_exit(-1);
  }


  // Attesa completamento attivazione dispositivi su BUS
  // In questa il Master richiede lo Status per sapere cosa sta accadendo;
  // Il Master potrebbe anche richiedere l'attivazione del Loader
  // La gestione operativa avviene solo se i dispositivi sono configurati
  while(1){
    if(mccRxFrame(&ep, &mcc_cmd)){

      if(mcc_cmd.cmd == MCC_TEST) mcc_test();

      if((mcc_cmd.cmd == MCC_LOADER) || (generalConfiguration.loaderOn)) manageMccLoader();
      else if(mcc_cmd.cmd == MCC_CONFIG) manageMccConfig();
      else if(generalConfiguration.deviceConfigured){
          analogManageMccOperativo();
      }
    }
  }
}
  
  //________________________________________________________________________________________________________________________
  //                                            INTERFACCIA LOADER
  // PROTOCOLLO SUL COMANDO:
  // mcc_cmd.buffer[0] = Sottocomando
  // mcc_cmd.buffer[1:N] = buffer dati del sottocomando

  // Prototipi dei sottocomandi
  void loader_activation(int id,unsigned char* data, int len);
  void loader_chip_erase(int id,unsigned char* data, int len);
  void loader_write_block(int id,unsigned char* data, int len);
  void loader_write_config(int id,unsigned char* data, int len);
  void loader_read_config(int id,unsigned char* data, int len);

  void manageMccLoader(void){
    // Accetta solo comandi di tipo Loader
    if(mcc_cmd.cmd!=MCC_LOADER) return;

    switch(mcc_cmd.buffer[0]){
       case LOADER_ACTIVATION:  // Attivazione Loader remoto a indirizzo ricevuto
        loader_activation(mcc_cmd.id,&mcc_cmd.buffer[1],mcc_cmd.len-1);
        break;
      case LOADER_CHIP_ERASE:
        loader_chip_erase(mcc_cmd.id,&mcc_cmd.buffer[1],mcc_cmd.len-1);
        break;
      case LOADER_WRITE_BLK:
        loader_write_block(mcc_cmd.id,&mcc_cmd.buffer[1],mcc_cmd.len-1);
        break;
      case LOADER_WRITE_CONFIG:
        loader_write_config(mcc_cmd.id,&mcc_cmd.buffer[1],mcc_cmd.len-1);
        break;
      case LOADER_READ_CONFIG:
        loader_read_config(mcc_cmd.id,&mcc_cmd.buffer[1],mcc_cmd.len-1);
        break;
    }

  }

/* COMANDA L'ATTIVAZIONE/DISATTIVAZIONE DEL LOADER
 data[0]: 1== ATTIVAZIONE, 0== DISATTIVAZIONE
 data[1]: Indirizzo Loader Periferica
 data[2]: 1 == uCA, 2== uCB (solo per attivazione)
 RISPOSTA:

 ATTIVAZIONE:
 La funzione effettua la connessione con il remoto richiesto
 ed effettua il Setup, acquisendo le info relative al target
 all'ID e alla config word (se disponibile).

*/
void loader_activation(int id,unsigned char* data, int len)
{       
  unsigned char buffer[1];
  
  if(loaderActivation(data[1], data[2])==FALSE)
  {
    loaderExit(TRUE); 
    buffer[0] = 0;
    generalConfiguration.loaderOn = false;
  }else  buffer[0] = 1;
  
  generalConfiguration.loaderOn = true;
  mccLoaderNotify(id,LOADER_ACTIVATION,buffer,1);
  return;
}

/* 
    COMANDA LA CANCELLAZIONE DELLA FLASH DEL TARGET REMOTO
    ED INIZIALIZZA LA PROGRAMMAZIONE
    Nessun parametro
  
   Il comando richiede che sia stato attivato il relativo loader
*/
void loader_chip_erase(int id,unsigned char* data, int len)
{       
  unsigned char buffer[1];
 
  if(loaderChipErase()==FALSE)
  {
    loaderExit(TRUE);
    generalConfiguration.loaderOn = false;
    buffer[0] = 0;
  }
  else buffer[0] = 1;
   
  mccLoaderNotify(id,LOADER_CHIP_ERASE,buffer,1);
  return;
}

void loader_write_block(int id,unsigned char* data, int len)
{       
  unsigned char buffer[1];
  int i;
  _addrStr blocco;
  
  blocco.startAddr = data[0] + 256*data[1];
  blocco.len = data[2];
 
  // Controllo sulla dimensione e coerenza del buffer dati
  if((3+ 2*blocco.len)!= len) 
  {
    printf("GUI: ERRORE LEN=%d BLOCCO LEN=%d\n",len,blocco.len );
    buffer[0] = 0;
    mccLoaderNotify(id,LOADER_WRITE_BLK,buffer,1);
    return;
  }

  // Carica i dati nella struttura
  for(i=0; i< blocco.len; i++) {
    blocco.val[i] = data[3+2*i] + 256 * data[4+2*i];
  }
  
  if(loaderLoadSegment(&blocco)==FALSE)
  {
    printf("GUI: SCRITTURA BLOCCO FALLITA\n");
    loaderExit(TRUE);
    buffer[0] = 0;
  }
  else buffer[0] = 1;
  
  mccLoaderNotify(id,LOADER_WRITE_BLK,buffer,1);
  return;
}

void loader_write_config(int id,unsigned char* data, int len)
{       
  unsigned char buffer[1];
  
  if(loaderWriteConfig(data)==FALSE)
  {
    buffer[0] = 0;
  }
  else buffer[0] = 1;
  
  // Questa deve essere l'ultima istruzione in un ciclo di scrittura dispositivo
  loaderExit(TRUE);
  generalConfiguration.loaderOn = false;
  mccLoaderNotify(id,LOADER_WRITE_CONFIG,buffer,1);
  return;
}

void loader_read_config(int id,unsigned char* data, int len)
{       
  unsigned char buffer[11];
  
  // Effettua un'attivazione per prelevare i valori richiesti
  if(loaderActivation(data[0], data[1])==FALSE)
  {
    loaderExit(TRUE);
    generalConfiguration.loaderOn = false;
    buffer[0]=0;
    mccLoaderNotify(id,LOADER_READ_CONFIG,buffer,1);
    return;
  }
  
  buffer[0]=1;
  buffer[1] = (unsigned char) Loader.cfg.id0;
  buffer[2] = (unsigned char) (Loader.cfg.id0>>8);
  buffer[3] = (unsigned char) Loader.cfg.id1;
  buffer[4] = (unsigned char) (Loader.cfg.id1>>8);
  buffer[5] = (unsigned char) Loader.cfg.id2;
  buffer[6] = (unsigned char) (Loader.cfg.id2>>8);
  buffer[7] = (unsigned char) Loader.cfg.id3;
  buffer[8] = (unsigned char) (Loader.cfg.id3>>8);
  buffer[9] = (unsigned char) Loader.cfg.config;
  buffer[10] = (unsigned char) (Loader.cfg.config>>8);
  
  mccLoaderNotify(id,LOADER_READ_CONFIG,buffer,11);
  loaderExit(TRUE);
  generalConfiguration.loaderOn = false;
  return;
}

  //_________________________________________________________________________________________________________________________
  
  
  
  //________________________________________________________________________________________________________________________
  //                                            INTERFACCIA CONFIGURAZIONE
  // PROTOCOLLO SUL COMANDO DI CONFIGURAZIONE:
  // mcc_cmd.buffer[0] = Dispositivo da configurare
  // mcc_cmd.buffer[1] = Blocco dati
  // mcc_cmd.buffer[2:len-3] = Blocco dati

void manageMccConfig(){

     // Accetta solo comandi di tipo Config
    if(mcc_cmd.cmd!=MCC_CONFIG) return;
    unsigned char data[40];
    unsigned char size=0;
    

    switch(mcc_cmd.buffer[0]){


     // Configurazione iniziale di tutto il sistema: questa configurazione definisce la struttura hardware
     // della macchina e viene ricevuto all'inizio della fase di startup.
     case CONFIG_GANTRY:
        memcpy(&generalConfiguration.gantryCfg,&mcc_cmd.buffer[2],sizeof(generalConfiguration.gantryCfg));
        generalConfiguration.gantryConfigurationReceived = true;
        mainPrintHardwareConfig();
        // Actuators invia il comando di attivazione device sullo slave
        // Main attiva tutte le thread relative alla architettura

        data[0]=  generalConfiguration.canConnected;
        size = 1;
     break;
    case CONFIG_STATUS:

      // Restituisce lo stato dei drivers e le revisioni
      data[0]= generalConfiguration.deviceConnected; // Tutti i dispositivi sono correttamente connessi
      data[1]=generalConfiguration.revisioni.m4_master.maj;
      data[2]=generalConfiguration.revisioni.m4_master.min;
      data[3]=generalConfiguration.revisioni.pcb269.maj;
      data[4]=generalConfiguration.revisioni.pcb269.min;
      data[5]=generalConfiguration.revisioni.pcb249U1.maj;
      data[6]=generalConfiguration.revisioni.pcb249U1.min;
      data[7]=generalConfiguration.revisioni.pcb249U2.maj;
      data[8]=generalConfiguration.revisioni.pcb249U2.min;
      data[9]=generalConfiguration.revisioni.pcb190.maj;
      data[10]=generalConfiguration.revisioni.pcb190.min;
      data[11]=generalConfiguration.revisioni.pcb244.maj;
      data[12]=generalConfiguration.revisioni.pcb244.min;
      data[13] = generalConfiguration.revisioni.m4_slave.maj;
      data[14] = generalConfiguration.revisioni.m4_slave.min;
      data[15] = generalConfiguration.revisioni.pcb240.maj;
      data[16] = generalConfiguration.revisioni.pcb240.min;
      data[17]=  generalConfiguration.armConnectedStatus ;
      data[18]=  generalConfiguration.trxConnectedStatus ;
      data[19]=  generalConfiguration.lenzeConnectedStatus ;
      data[20]=  generalConfiguration.pcb240ConnectedStatus ;
      data[21]=  generalConfiguration.slaveDeviceConnected;  // Gruppo dei dispositivi slave sono correttamente connessi
      data[22]=  generalConfiguration.canConnected;         // Il CAN bus è operativo
      data[23] = generalConfiguration.candevice_error_startup; // Errori durante startup
      data[24] = generalConfiguration.revisioni.pcb249U1.model; // Modello collimatore
      data[25] = generalConfiguration.rtc_present;
      data[26] = generalConfiguration.weekday;
      data[27] = generalConfiguration.year & 0xFF;
      data[28] = (generalConfiguration.year >>8) &0xFF;
      data[29] = generalConfiguration.month;
      data[30] = generalConfiguration.day;
      data[31] = generalConfiguration.hour;
      data[32] = generalConfiguration.min;
      data[33] = generalConfiguration.sec;
      size = 34;


     case CONFIG_GENERAL:      
      if(mcc_cmd.buffer[2]==1) {
        generalConfiguration.demoMode = 1;        
        printf(" ----------------  DRIVERS IN DEMO MODE --------------------- \n");
      }else{
        generalConfiguration.demoMode = 0;
        printf(" ----------------  DRIVERS IN OPERATING MODE --------------------- \n");
      }

      if(mcc_cmd.buffer[3]==1) {
        generalConfiguration.enableAudio = 1;
        generalConfiguration.volumeAudio = mcc_cmd.buffer[4];
        // vmInit();

        printf(" ----------------  AUDIO MODE --------------------- \n");
      }else{
          generalConfiguration.enableAudio = 0;
          printf(" ----------------  NO AUDIO MODE --------------------- \n");
      }


     break;
     case CONFIG_PCB190:  
        if(config_pcb190(true, mcc_cmd.buffer[1], &mcc_cmd.buffer[2], mcc_cmd.len-2)==true) data[0]=1;
        else data[0]= 0;
        size = 1;
      break;
    case CONFIG_PCB269_0:
       if(config_pcb215(mcc_cmd.buffer[1], &mcc_cmd.buffer[2])==true) data[0]=1;
       else data[0]= 0;
       size = 1;
     break;
   case CONFIG_PCB269_1:
      if(config_pcb215(mcc_cmd.buffer[1], &mcc_cmd.buffer[2])==true) data[0]=1;
      else data[0]= 0;
      size = 1;
    break;
     case CONFIG_TRX:
        if(config_trx(true, mcc_cmd.buffer[1], &mcc_cmd.buffer[2], mcc_cmd.len-2)==true) data[0]=1;
        else data[0]= 0;
        size = 1;
      break;
    case CONFIG_ARM:
       if(config_arm(true, mcc_cmd.buffer[1], &mcc_cmd.buffer[2], mcc_cmd.len-2)==true) data[0]=1;
       else data[0]= 0;
       size = 1;
     break;
    case CONFIG_LENZE:
       if(config_lenze(true, mcc_cmd.buffer[1], &mcc_cmd.buffer[2], mcc_cmd.len-2)==true) data[0]=1;
       else data[0]= 0;
       size = 1;
     break;

    case CONFIG_PCB249U1_1:
        if(config_pcb249U1(true, 0, &mcc_cmd.buffer[2], mcc_cmd.len-2)==true) data[0]=1;
        else data[0]= 0;
        size = 1;
      break;
     case CONFIG_PCB249U1_2:
        if(config_pcb249U1(true, 1, &mcc_cmd.buffer[2], mcc_cmd.len-2)==true) data[0]=1;
        else data[0]= 0;
        size = 1;
      break;
     case CONFIG_PCB249U1_3:
        if(config_pcb249U1(true, 2, &mcc_cmd.buffer[2], mcc_cmd.len-2)==true) data[0]=1;
        else data[0]= 0;
        size = 1;
      break;
     case CONFIG_PCB249U1_4:
        if(config_pcb249U1(true, 3, &mcc_cmd.buffer[2], mcc_cmd.len-2)==true) data[0]=1;
        else data[0]= 0;
        size = 1;
      break;
     case CONFIG_PCB249U1_5:
        if(config_pcb249U1(true, 4, &mcc_cmd.buffer[2], mcc_cmd.len-2)==true) data[0]=1;
        else data[0]= 0;
        size = 1;
      break;
     case CONFIG_PCB249U1_6:
        if(config_pcb249U1(true, 5, &mcc_cmd.buffer[2], mcc_cmd.len-2)==true) data[0]=1;
        else data[0]= 0;
        size = 1;
      break;
     case CONFIG_PCB249U1_7:
        if(config_pcb249U1(true, 6, &mcc_cmd.buffer[2], mcc_cmd.len-2)==true) data[0]=1;
        else data[0]= 0;
        size = 1;
      break;
      
     case CONFIG_PCB249U2:  
       if(config_pcb249U2(true, mcc_cmd.buffer[1], &mcc_cmd.buffer[2], mcc_cmd.len-2)==true){
         data[0]=1; // Configurazione OK. 
       }else{
         data[0]= 0; // Fallita configurazione per qualche motivo
       }
       size = 1;
       
      break;
     case CONFIG_PCB244:
        if(config_pcb244_A(true, mcc_cmd.buffer[1], &mcc_cmd.buffer[2], mcc_cmd.len-2)==true) data[0]=1;
        else data[0]= 0;
        size = 1;
      break;
     case CONFIG_BIOPSY:  
         if(config_biopsy(true, mcc_cmd.buffer[1], &mcc_cmd.buffer[2], mcc_cmd.len-2)==true) data[0]=1;
        else data[0]= 0;
        size = 1;
     break;

    case  CONFIG_COMPLETED:                    
        // Questo sblocca la fase di startup attivando tutti i polling
        printf("CONFIGURAZIONE DEVICES COMPLETATA\n");
        generalConfiguration.deviceConfigOk = TRUE; // La configurazione è arrivata
        _EVSET(_EV1_DEV_CONFIG_OK);

        // Attivazione del MASTER_ENA
        SystemOutputs.CPU_MASTER_ENA=1;
        _EVSET(_EV0_OUTPUT_CAMBIATI);
        data[0]= 1;
        size = 1;
      break;        

    case MCC_RTC_COMMANDS:
        mcc_rtc();
    break;

   //  case CONFIG_REVISIONS: Non usato
      // break;
    }

    // Notifica l'avvenuta esecuzione della configurazione
    // Viene restituito il numero di blocco salvato, per consentire eventualmente 
    // l'invio di più blocchi ..
    mccConfigNotify(1,mcc_cmd.buffer[0],data,size);
    return;
  }
  //_________________________________________________________________________________________________________________________
  

  void analogManageMccOperativo(void){

        switch(mcc_cmd.cmd)
        {
        case MCC_XRAY_ANALOG_MANUAL:
              mcc_xray_analog_manual();
        break;

        case MCC_XRAY_ANALOG_AUTO:
              mcc_xray_analog_auto();
        break;

        case MCC_XRAY_ANALOG_CALIB_PRE:
              mcc_xray_analog_pre_calib();
        break;

        case MCC_XRAY_ANALOG_CALIB_PROFILE:
              mcc_xray_analog_calib_profile();
        break;
        case MCC_XRAY_ANALOG_CALIB_TUBE:
              mcc_xray_analog_calib_tube();
        break;

        case MCC_CANOPEN:
            mcc_canopen();
        break;

          case MCC_SET_MIRROR:
            mcc_set_mirror(mcc_cmd.id, mcc_cmd.cmd,mcc_cmd.buffer[0]);
          break;
        case MCC_UPDATE_MIRROR_LAMP:
          mcc_set_update_mirror_lamp();
        break;
          case MCC_SET_LAMP:
            mcc_set_lamp(mcc_cmd.id, mcc_cmd.cmd);
          break;

          case MCC_GET_GONIO:
            mccGetGonio(mcc_cmd.id, mcc_cmd.cmd);
          break;

          case MCC_SET_ROT_TOOL_CONFIG: // Imposta la configurazione per il tool di gestione rotazioni
            mccSetRotationToolConfig();
          break;

          case MCC_RESET_GONIO: // Effettua il reset dell'inclinometro ad angolo prestabilito
            mccResetGonio(mcc_cmd.id,mcc_cmd.cmd);
          break;

          case  MCC_SET_COLLI: // Effettua il setting della collimazione
            mccSetColli(mcc_cmd.id,mcc_cmd.cmd);
          break;
          case MCC_SET_FUOCO:
            mccSetFuoco(mcc_cmd.id,mcc_cmd.cmd);
          break;
          case MCC_SET_FILTRO:
            mccSetFiltro();
          break;
          case MCC_CALIB_FILTRO:
            mccSetCalibFiltro();
          break;

          case MCC_BIOPSY_DEMO_CMD:
          break;

          case MCC_BIOPSY_SIMULATOR:
            mccBiopsySimulator();
            break;
          case MCC_BIOPSY_CMD:
            mccBiopsyCmd();
          break;


            case MCC_CMD_SBLOCCO:      // Richiede una sequenza di sblocco compressore
              if(pcb215SetSblocco()==FALSE)
              {
                if(_TEST_BIT(PCB215_FAULT))
                {
                  printf("COMANDO SBLOCCO: ERRORE %d\n", PCB215_CONTEST.Stat.error);
                }else  printf("GUI: IMPOSSIBILE ESEGUIRE LO SBLOCCO\n");
              }else
              {
                printf("GUI: EXEC SBLOCCO\n");
              }
              break;
           case MCC_CMD_PAD_UP:       // Richiede una sequenza di attivazione carrello compressore
              if(pcb215MovePadUpward(mcc_cmd.buffer[0])==FALSE)
              {
                if(_TEST_BIT(PCB215_FAULT))
                {// Caso di errore del driver
                  printf("COMANDO COMPRESSIONE: ERRORE %d\n", PCB215_CONTEST.Stat.error);
                }else  printf("GUI: IMPOSSIBILE ESEGUIRE IL COMANDO\n");
              }else
              {
                printf("GUI: EXEC UP %d\n",mcc_cmd.buffer[0]);
              }
              break;

            case MCC_CMD_CMP_STOP:
              if(pcb215SetIdle()==FALSE)
              {
                if(_TEST_BIT(PCB215_FAULT))
                {// Caso di errore del driver
                  printf("COMANDO STOP COMPRESSORE: ERRORE %d\n", PCB215_CONTEST.Stat.error);
                }else  printf("GUI: IMPOSSIBILE ESEGUIRE IL COMANDO\n");
              }else
              {
                printf("GUI: EXEC STOP\n");
              }
              break;


            // Richiesta movimento Tubo
            case MCC_CMD_TRX:
                mcc_cmd_trx();
             break;

            // Richiesta movimento Braccio: angoli in gradi (short)
            case MCC_CMD_ARM:
              mcc_cmd_arm();
            break;

            case MCC_CMD_PCB215_CALIB:
                mcc_pcb215_calibration();

            break;
            case MCC_GET_TROLLEY:
                mcc_pcb215_get_trolley();
            break;
            case MCC_STARTER:
              mcc_set_starter();
            break;

            case MCC_SET_OUTPUTS:
              setOutputs((_SystemOutputs_Str*) mcc_cmd.buffer, (_SystemOutputs_Str*) &(mcc_cmd.buffer[sizeof(SystemOutputs)]));
            break;

            case MCC_SERVICE: // Sottofamiglia di comandi per il service
              mcc_service_commands(mcc_cmd.id,mcc_cmd.buffer[0],&mcc_cmd.buffer[1],mcc_cmd.len-1);
            break;

            case MCC_CMD_CMP_AUTO:
              pcb215MovePadDownward(1,TRUE);
            break;
            case MCC_CALIB_ZERO:
                  mcc_calib_zero();
            break;
            case MCC_244_A_DETECTOR_FIELD:
                PCB244_A_setDetectorField(mcc_cmd.buffer[0]);
            break;
            case MCC_244_A_FUNCTIONS:
                mcc_244_A_functions();
            break;

            case MCC_AUDIO:
                mcc_audio();
            break;

            case MCC_RTC_COMMANDS:
                mcc_rtc();
            break;
            default:
              // printf("Ricevuto buffer di %d\n",mcc_len);
              break;
        }

}

/*_____________________________________________________________________________________
    data[0]= comando; // Comando variabile negativo
    data[2,3] = (se TRX_MOVE_ANGLE) angolo in gradi;
    data[1]=0; non usato
_____________________________________________________________________________________*/
void mcc_cmd_trx(void)
{
    printf("COMANDO MCC TRX\n");

    if(mcc_cmd.buffer[0]==TRX_MOVE_STOP){
        printf("MCC COMANDO STOP\n");
        actuatorsTrxStop(0);
        return;

    }
   // Se il comando è già in esecuzione deve rispondere un errore
    if((generalConfiguration.trxExecution.run == true)||(generalConfiguration.trxExecution.completed == false)){
        unsigned char buffer[2];
        printf("RICHIESTA MCC MOVIMENTO TRX: BUSY!");
        buffer[0] = TRX_BUSY;
        buffer[1] = 0; // sub codice in caso di errore da fault
        mccGuiNotify(mcc_cmd.id,MCC_CMD_TRX,buffer,2);
        return;
    }


    int angolo = (int) (mcc_cmd.buffer[2] + 256 * mcc_cmd.buffer[3]); // Imposta l'angolo
    generalConfiguration.trxExecution.id = mcc_cmd.id;
    generalConfiguration.trxExecution.test = false; // Comunque stoppa il loop di rodaggio in corso

    _tomo_profile* pTrxProfile;

    pTrxProfile=&generalConfiguration.trxCfg.tomo;

    // Attenzione, tutti gli angoli sono espressi in decimi di grado
    switch(mcc_cmd.buffer[0]){
    case TRX_MOVE_STOP:   actuatorsTrxStop(0);
    case TRX_MOVE_CC:     actuatorsTrxMove(0); break;
    case TRX_MOVE_M15:    actuatorsTrxMove(-1 * generalConfiguration.trxCfg.angolo_biopsia); break;
    case TRX_MOVE_P15:    actuatorsTrxMove(generalConfiguration.trxCfg.angolo_biopsia); break;
    case TRX_MOVE_HOME_W: actuatorsTrxMove(pTrxProfile->w.home_position); break;
    case TRX_MOVE_HOME_N: actuatorsTrxMove(pTrxProfile->n.home_position); break;
    case TRX_MOVE_HOME_I: actuatorsTrxMove(pTrxProfile->i.home_position); break;
    case TRX_MOVE_END_W:  actuatorsTrxMove(pTrxProfile->w.end_position); break;
    case TRX_MOVE_END_N:  actuatorsTrxMove(pTrxProfile->n.end_position); break;
    case TRX_MOVE_END_I:  actuatorsTrxMove(pTrxProfile->i.end_position); break;
    case TRX_MOVE_ANGLE:
        // Angolo deve essere espresso in centesimi di grado
        actuatorsTrxMove(100 * (int) angolo);
    break;

    }


    return;
}

/*_____________________________________________________________________________________

_____________________________________________________________________________________*/
void mcc_cmd_arm(void)
{
    unsigned char buffer[2];
    short angolo;

    // Carica l'angolo dai parametri ricevuti
    FROM_LE16(angolo,&mcc_cmd.buffer[0]);

    // Se il comando è già in esecuzione deve rispondere un errore
    if((generalConfiguration.armExecution.run == true)||(generalConfiguration.armExecution.completed == false)){
        printf("RICHIESTA MCC MOVIMENTO ARM: BUSY!");
        buffer[0] = ARM_BUSY;
        buffer[1] = 0; // sub codice in caso di errore da fault
        mccGuiNotify(mcc_cmd.id,MCC_CMD_ARM,buffer,2);
        return;
    }

    int error=0;

    // Test pending compression
    if(!SystemOutputs.CPU_ROT_ENA)
    {
      // Rotation enable Bus Hardware test
      printf("MOVIMENTO (ARM) FALLITO: SEGNALE ROT_ENA NON ATTIVO!\n");
      error = ARM_DISABLED_ERROR;
    }else if((angolo!=200)&&((angolo>180) ||(angolo<-180)))
    {
      // Errore angolo fuori range
      printf("MOVIMENTO (ARM) FALLITO: ANGOLO OUT OF RANGE: %d!\n",angolo);
      error =  ARM_RANGE_ERROR;
    }else if(generalConfiguration.armCfg.direction_memory==MEM_ARM_DIR_UNDEF){
        // Errore per mancanza di informazioni relative alla posizione del braccio
        printf("MOVIMENTO (ARM) FALLITO: ANGOLO OUT OF RANGE: %d!\n",angolo);
        error =  ARM_RANGE_ERROR;
    }

    if(error){
        buffer[0] = error;
        buffer[1] = 0; // sub codice in caso di errore da fault
        mccGuiNotify(mcc_cmd.id,MCC_CMD_ARM,buffer,2);
        return;

    }

    generalConfiguration.armExecution.id = mcc_cmd.id;

    if(angolo==200){
        if(generalConfiguration.armExecution.dAngolo<0) angolo = -200;
    }
    actuatorsArmMove(angolo);
    return;
}



/*
  IMPOSTA LA CONFIGURAZIONE DEI REGISTRI PER IL TOOL
  DI GESTIONE DELLA ROTAZIONE
*/
void mccSetRotationToolConfig()
{
   _SystemOutputs_Str output, mask;    
   _DeviceAppRegister_Str        ConfList;

    
    printf("GUI: CONFIGURAZIONE PER TOOL ROTAZIONI\n");
}

/*_____________________________________________________________________________

    Richiesta angolo Braccio, Tubo e inclinometro.
    I datri restituiti sono quelli aggiornati dai rispettivi drivers

    buffer[0,1] = ARM
    buffer[2,3] = TRX
    buffer[4,5] = INCLINOMETRO

    !!! Tutti i dati sono espressi in decimi di grado


    TO BE DONE: verificare che il formato dei dati sia
    in decimi di grado anche sul richiedente

*/
void mccGetGonio(unsigned char id, unsigned char mcccode)
{
    unsigned char buffer[6];

    printf("RICEVUTO RICHIESTA GONIO: TRX=(c)%d, ARM=(d)%d, GONIO=(d)%d\n", generalConfiguration.trxExecution.cAngolo,generalConfiguration.armExecution.dAngolo,generalConfiguration.armExecution.dAngolo_inclinometro);
    TO_LE16(&buffer[0],generalConfiguration.armExecution.dAngolo);
    TO_LE16(&buffer[2],generalConfiguration.trxExecution.cAngolo);
    TO_LE16(&buffer[4],generalConfiguration.armExecution.dAngolo_inclinometro);

    mccGuiNotify(1,mcccode, buffer, 6);
    return ;
    
}

void mcc_set_mirror(unsigned char id, unsigned char mcccode,unsigned char cmd)
{
  unsigned char data[2];
 
  if(pcb249U2Mirror(cmd)==TRUE) data[0]=1;
  else data[0] = 0;
  
  // Consulta il registro RG249U2_MIRROR_STAT per lo stato corrente dello specchio
  if(_TEST_BIT(PCB249U2_MIR_HOME_FLG)) data[1]=0;
  else if(_TEST_BIT(PCB249U2_MIR_OUT_FLG)) data[1]=1;
  else data[1]=2;
  
  mccGuiNotify(id,mcccode,data,2);
  return; 
}
 

// Aggiorna il numero di steps Mette in campo lo specchio con la luce
void mcc_set_update_mirror_lamp(void)
{
  unsigned char data[2];

  // Aggiorna gli steps
  int steps = mcc_cmd.buffer[0] + 256* mcc_cmd.buffer[1];
  generalConfiguration.mirror_position = steps;
  Ser422WriteRegister(_REGID(RG249U2_PR_MIRROR_STEPS),generalConfiguration.mirror_position ,10,&PCB249U2_CONTEST);

  /*
    cmd==0 -> Lampada OFF
    cmd==1 -> Lampada ON
    cmd==2 -> Lampada OFF + MIRROR HOME
    cmd==3 -> Lampada ON + MIRROR OUT
  tmo= Timout Lampada (0:64) (32 secondi max), 0 = infinito
  */
  pcb249U2Lamp(2, 0, true);
  _time_delay(1000);
  pcb249U2Lamp(3, 40, false);

  return;
}

/*
    data[0] =  cmd;
    data[1] =  timeout;
    data[2] =  steps specchio L
    data[3] = steps specchio H
*/
void mcc_set_lamp(unsigned char id, unsigned char mcccode)
{
  unsigned char data[3];
  unsigned short steps;
 

  // Preconfigura gli steps dello specchio
  steps = mcc_cmd.buffer[2] + mcc_cmd.buffer[3] * 256;
  if(Ser422WriteRegister(_REGID(RG249U2_PR_MIRROR_STEPS),steps,10,&PCB249U2_CONTEST) == _SER422_NO_ERROR)
  {
    if(pcb249U2Lamp(mcc_cmd.buffer[0],mcc_cmd.buffer[1],TRUE)==TRUE) data[0]=1;
    else data[0] = 0;
  }

  // Consulta il registro RG249U2_MIRROR_STAT per lo stato corrente dello specchio
  if(_TEST_BIT(PCB249U2_MIR_HOME_FLG)) data[2]=0;
  else if(_TEST_BIT(PCB249U2_MIR_OUT_FLG)) data[2]=1;
  else data[2]=2;
  
  if(_TEST_BIT(PCB249U2_LAMP_ON_FLG)) data[1]=1;
  else data[1]=0;

  mccGuiNotify(id,mcccode,data,3);
  
  if(data[0]==0) printf("MCC LAMP FALLITO!\n");
  else printf("MCC LAMP: CMD=%d,  TMO=%d, STEPS=%d\n", mcc_cmd.buffer[0], mcc_cmd.buffer[1], steps);
  return; 

}

/*
  BUFFER[0] == Fuoco da attivare:
  - 0 = F1G
  - 1 = F2G
  - 2 = F1P
  - 3 = F2P
  - 4 = FUOCO SPENTO

  NOTIFY: BUFFER[0] = RISULTATO
*/
void  mccSetFuoco(unsigned char id, unsigned char mcccode)
{
  bool ris;
  unsigned char data[1];
  
  printf("ESEGUE FUOCO %d\n",mcc_cmd.buffer[0]);
  switch(mcc_cmd.buffer[0])
  {
  case 0: ris = pcb190SetFuoco(PCB190_F1G);break;
  case 1: ris = pcb190SetFuoco(PCB190_F2G);break;
  case 2: ris = pcb190SetFuoco(PCB190_F1P);break;
  case 3: ris = pcb190SetFuoco(PCB190_F2P);break;
  default: ris = pcb190SetFuoco(0);break;               // Spegne il filamento
  }
   
  if(ris) data[0]=1;
  else data[0]=0;
  
  mccGuiNotify(id,mcccode,data,1);
}

/*
  mcc_cmd.buffer[0] = filtro
  mcc_cmd.buffer[1] = posizione target filtro
*/  
void mccSetFiltro(void)
{
    printf("GUI RICHIEDE POSIZIONAMENTO FILTRO: INDEX=%d, POS=%d\n", mcc_cmd.buffer[0],mcc_cmd.buffer[1]);
  pcb249U2SetFiltro(mcc_cmd.buffer[0],mcc_cmd.buffer[1], mcc_cmd.id);

}

// Effettua una nuova calibrazione del filtro, aggiornando la relativa configurazione
//   buffer[0] = filtro posizione 0
//   buffer[1] = filtro posizione 1
//   buffer[2] = filtro posizione 2
//   buffer[3] = filtro posizione 3
void mccSetCalibFiltro(void)
{
  bool ris;
  unsigned char data;
  
  generalConfiguration.colli_filter[0]=mcc_cmd.buffer[0];
  generalConfiguration.colli_filter[1]=mcc_cmd.buffer[1];
  generalConfiguration.colli_filter[2]=mcc_cmd.buffer[2];
  generalConfiguration.colli_filter[3]=mcc_cmd.buffer[3];           

  // Attiva il flag di aggiornamento e esce: l'aggiornamento verrà effettuato dal driver.
  generalConfiguration.colli_filter_update = true;
  return;
}

/*__________________________________________________________
  Impostazione collimazione 2D
NOTIFY:
    data[0] = Successo/Insuccesso

*/
void  mccSetColli(unsigned char id, unsigned char mcccode)
{  
  unsigned char data[1];
  
  generalConfiguration.colliCfg.colliDinamicaAbilitata = FALSE;
  
  // Assegnazione dell'impostazione statica delle lame
  generalConfiguration.colliCfg.lame2D.front = mcc_cmd.buffer[COLLI_F];
  generalConfiguration.colliCfg.lame2D.back = mcc_cmd.buffer[COLLI_B];
  generalConfiguration.colliCfg.lame2D.trap = mcc_cmd.buffer[COLLI_T];
  generalConfiguration.colliCfg.lame2D.left = mcc_cmd.buffer[COLLI_L];
  generalConfiguration.colliCfg.lame2D.right = mcc_cmd.buffer[COLLI_R];
  
  
  printf("SCRITTURA LAME PER COLLIMAZIONE 2D\n");
  printf("FRONT:%d\n",generalConfiguration.colliCfg.lame2D.front);
  printf("BACK:%d\n",generalConfiguration.colliCfg.lame2D.back );
  printf("LEFT:%d\n",generalConfiguration.colliCfg.lame2D.left);
  printf("RIGHT:%d\n",generalConfiguration.colliCfg.lame2D.right);
  printf("TRAP:%d\n",generalConfiguration.colliCfg.lame2D.trap);
  
  // Richiede l'esecuzione del posizionamento delle lame frontali e posteriori
  // Il comando non può fallire poichè semplicemente sovrascrive uno stato in corso d'opera..
  // Questo dovrebbe impedire una catena di continue collimazioni dovute al cambio di compressore
  // SOltanto l'ultimo stato vince
  pcb249U2SetColli( generalConfiguration.colliCfg.lame2D.back , generalConfiguration.colliCfg.lame2D.front);
  pcb249U1SetColli(generalConfiguration.colliCfg.lame2D.left,generalConfiguration.colliCfg.lame2D.right,generalConfiguration.colliCfg.lame2D.trap,id);

}

void mcc_test(void)
{
    unsigned long val;
    unsigned char data;
    char cdata;


      switch(mcc_cmd.buffer[0]){
      case 0:
          printf("GPIO INIT");
          spiInit();
          break;
      case 1:
          printf("SYNC");
          if(vmSync()) printf("SYNC OK\n");
          else printf("SYNC FAILED\n");
          break;

      case 2:
          printf("SCARICA LE SCRITTE");
          vmPrint(true,1000);
          break;
      case 3:
          printf("SHORT MSG MODE... ");
          if(!vmShortMsg()) printf("NOK!\n");
          else     printf("OK\n");
          break;
      case 4:
          if(!vmPrompt()) printf("USB BOMS NOT AVAILABLE!\n");
          else {
              printf("DIR:\n");
              vmDir();
          }
          break;

      case 5:

          break;
      case 6:

          break;
      case 7:

           break;

      case 8:

          break;

      case 9:

          break;

      default:
          spiInit();

          // Scarica le scritte varie
          vmPrint(true,100);

          printf("SHORT MSG MODE... ");
          if(!vmShortMsg()) printf("NOK!\n");
          else     printf("OK\n");

          if(!vmPrompt()) printf("USB BOMS NOT AVAILABLE!\n");
          else {
              printf("DIR:\n");
              vmDir();
          }
      }

    return;
}


/*
  ATTIVAZIONE STARTER IAE CON I SEGUENTI PARAMETRI
- 0 : STOP STARTER
- 1 : START LOW SPEED
- 2 : START HIGH SPEED
- 3 : OFF STARTER
*/
void mcc_set_starter(void)
{
  bool ris;
  printf("ATTIVAZIONE/DISATTIVAZIONE STARTER:%d\n",mcc_cmd.buffer[0]);
  
  // Reset fault sulla pcb190
  pcb190ResetFault();
  if(mcc_cmd.buffer[0]==0) pcb190StopStarter();
  else if(mcc_cmd.buffer[0]==1) pcb190StarterL();
  else if(mcc_cmd.buffer[0]==2)pcb190StarterH();
  else if(mcc_cmd.buffer[0]==3)pcb190OffStarter();
}

/*
  Attiva il rodaggio del tubo
  data[0] = num cicli
  data[1] = angolo 
  data[2] = velocita: 0 = STD, 1 = WIDE, 2 = NARROW
*/
void srv_rodaggio_tubo(int id, unsigned char* data,int len)
{
// TO BE DONE
/*
  generalConfiguration.trxloop_cicli =data[0]; 
  generalConfiguration.trxloop_seq = 0;
  generalConfiguration.trxloop = true; 
*/
}



//_______________________________________________________________________________
// Sottofamiglia di comandi di Service
void srv_freeze_mode(int id,unsigned char* data, int len);
void srv_send(int id,unsigned char* data, int len);
void srv_rodaggio_tubo(int id, unsigned char* data,int len);
void mcc_service_commands(int id,int subcmd,unsigned char* data,int len)
{

  switch(subcmd)
  {
    case SRV_TEST_LS_STARTER:
      // Effettua uno Start/Stop dello Starter
      pcb190TestLSStarter();
      break;
    case SRV_SERIAL_SEND:
        srv_send(id,data,len);
    break;
    case SRV_RODAGGIO_TUBO:
        srv_rodaggio_tubo(id,data,len);
    break;
    case SRV_ARM_STOP: // Blocca qualsiasi movimento del braccio in corso
        generalConfiguration.trxExecution.run=false;
        generalConfiguration.armExecution.run=false;
        generalConfiguration.trxExecution.test=false;
        generalConfiguration.armExecution.test=false;
        //actuatorsStop();
    break;

    case  SRV_STOP_POTTER_2D_GRID:
      if( generalConfiguration.potterCfg.potId != POTTER_2D){
          printf("COMMAND FAILURE: No Potter 2D available!\n");
          return;
      }
      pcb244A_Stop2dGrid();
    break;


  }
}



// COMANDA L'ATTIVAZIONE DEL MODO FREEZE (ON/OFF)
// data[0] = 1->ON, 0->OFF
// RISPOSTA:
// BUFFER[0]= RISULTATO
// BUFFER[1] = STATO ESEGUITO
void srv_freeze_mode(int id,unsigned char* data, int len)
{       
  bool risultato;
  unsigned char buffer[2];
  if(data[0])
  {
    // Disabilita tutti i drivers ed attende che tutti i driver siano fermi
    if(Ser422DriverFreezeAll(4000)==FALSE) risultato=FALSE;
    else risultato=TRUE;
  }else
  {
    // Sblocca i drivers dallo stato di freeze
    if(Ser422DriverSetReadyAll(5000) == FALSE) risultato=FALSE;
    else risultato=TRUE;
  }
    
  buffer[0] = risultato;
  buffer[1] = data[0];
  mccServiceNotify(id,SRV_FREEZE_DEVICE,buffer,2);
  return;
}

// COMANDA L'ATTIVAZIONE DEL MODO FREEZE (ON/OFF)
// data[0] = Addr, data[1] = dato1 data[2]=dato2
// RISPOSTA:
// BUFFER[0][1][2] Frame di risposta se c'è
void srv_send(int id,unsigned char* data, int len)
{       
  bool risultato;
  unsigned char buffer[3];
    
  Ser422SendRaw(data[0], data[1], data[2], buffer, 10);

  mccServiceNotify(id,SRV_SERIAL_SEND,buffer,3);
  return;
}




/* COMANDI VARI DURANTE LA CALIBRAZIONE DEL COMPRESSORE 
    mcc_cmd.buffer[0] = COMANDO
    0 = DISATTIVAZIONE CALIBRAZIONE
    1 = ATTIVAZIONE CALIBRAZIONE
        Entrando in calibrazione, vengono modificati alcuni registri della PCB215
        per favorire la calibrazione stessa.
    2 = IMPOSTAZIONE NUOVA CALIBRAZIONE NACCHERA
        buffer[1,2] = OFFSET, buffer[3] = K
    3 = IMPOSTAZIONE NUOVA CALIBRAZIONE FORZA
        buffer[1] = F0, buffer[2,3] = KF0, buffer[4] = F1, buffer[5] = KF1
    4 = IMPOSTAZIONE NUOVA CALIBRAZIONE PADS
       buffer[1] = pad, buffer[2,3] = Offset(int), buffer[4] = kF, buffer[5] = Peso
*/
void mcc_pcb215_calibration(void)
{
  switch(mcc_cmd.buffer[0])
  {
  case 0: // Disattivazione modo calibrazione        
          printf("PCB215 EXIT CALIB MODE\n");
          pcb215ActivateCalibMode(FALSE);
    break;
  case 1: // Attivazione modo calibrazione e impostazione particolare dei registri di calibrazione    
          printf("PCB215 ENTERING CALIB MODE\n");
          pcb215ActivateCalibMode(TRUE);
    break;
  case 2: // Configurazione Nacchera
          printf("CONFIGURAZIONE NACCHERA\n");
          generalConfiguration.comprCfg.calibration.calibPosOfs = mcc_cmd.buffer[1] + 256 * mcc_cmd.buffer[2];
          generalConfiguration.comprCfg.calibration.calibPosK = mcc_cmd.buffer[3];
          pcb215ConfigNacchera(FALSE);
          pcb215ForceUpdateData(); // Forza il rinnovo dei parametri di gestione del pad
    break;
    case 3: // Configurazione calibrazione forza

          generalConfiguration.comprCfg.calibration.F0 = mcc_cmd.buffer[1];
          generalConfiguration.comprCfg.calibration.KF0 = mcc_cmd.buffer[2] + mcc_cmd.buffer[3] * 256;
          generalConfiguration.comprCfg.calibration.F1 = mcc_cmd.buffer[4];
          generalConfiguration.comprCfg.calibration.KF1 = mcc_cmd.buffer[5];
          printf("CONFIGURAZIONE FORZA: F0=%d, KF0=%d, F1=%d, KF1=%d\n", generalConfiguration.comprCfg.calibration.F0,generalConfiguration.comprCfg.calibration.KF0,generalConfiguration.comprCfg.calibration.F1,generalConfiguration.comprCfg.calibration.KF1);
          pcb215ConfigForza(FALSE);
          pcb215ForceUpdateData(); // Forza il rinnovo dei parametri di gestione del pad
    break;
    case 4: // Configurazione pads
          printf("CONFIGURAZIONE PAD: %d\n",mcc_cmd.buffer[1]);
          generalConfiguration.comprCfg.calibration.pads[mcc_cmd.buffer[1]].offset = (int) (mcc_cmd.buffer[2] + mcc_cmd.buffer[3] * 256);
          generalConfiguration.comprCfg.calibration.pads[mcc_cmd.buffer[1]].kF = mcc_cmd.buffer[4];
          generalConfiguration.comprCfg.calibration.pads[mcc_cmd.buffer[1]].peso = mcc_cmd.buffer[5];
          printf("CONFIGURAZIONE PAD: %d offset=%d, kF=%d, Peso=%d\n",
                  mcc_cmd.buffer[1],
                  generalConfiguration.comprCfg.calibration.pads[mcc_cmd.buffer[1]].offset,
                  generalConfiguration.comprCfg.calibration.pads[mcc_cmd.buffer[1]].kF,
                  generalConfiguration.comprCfg.calibration.pads[mcc_cmd.buffer[1]].peso);
          
          pcb215ForceUpdateData(); // Forza il rinnovo dei parametri di gestione del pad
    
    break;

  }
  
}


/*
CODICI COMANDO BIOPSIA (DA GUI A M4): MCC_BIOPSY_CMD
#define _MCC_BIOPSY_CMD_MOVE_HOME   1
#define _MCC_BIOPSY_CMD_MOVE_XYZ    2
#define _MCC_BIOPSY_CMD_MOVE_INCX   3
#define _MCC_BIOPSY_CMD_MOVE_DECX   4
#define _MCC_BIOPSY_CMD_MOVE_INCY   5
#define _MCC_BIOPSY_CMD_MOVE_DECY   6
#define _MCC_BIOPSY_CMD_MOVE_INCZ   7
#define _MCC_BIOPSY_CMD_MOVE_DECZ   8
#define _MCC_BIOPSY_CMD_SET_STEPVAL 9
#define _MCC_BIOPSY_CMD_SET_LAGO    10

*/
void mccBiopsyCmd(void)
{
  unsigned short X,Y,Z,val;

  if(generalConfiguration.biopsyCfg.connected==FALSE) return;

  switch(mcc_cmd.buffer[0]){
  case _MCC_BIOPSY_CMD_MOVE_HOME:
      biopsyMoveHome();
      break;
  case _MCC_BIOPSY_CMD_MOVE_XYZ:
      X=mcc_cmd.buffer[1]+256*mcc_cmd.buffer[2];
      Y=mcc_cmd.buffer[3]+256*mcc_cmd.buffer[4];
      Z=mcc_cmd.buffer[5]+256*mcc_cmd.buffer[6];
      biopsyMoveXYZ(X, Y,Z);
      break;
  case _MCC_BIOPSY_CMD_MOVE_INCX:
      biopsyStepIncX();
      break;
  case _MCC_BIOPSY_CMD_MOVE_DECX:
      biopsyStepDecX();
      break;
  case _MCC_BIOPSY_CMD_MOVE_INCY:
      biopsyStepIncY();
      break;
  case _MCC_BIOPSY_CMD_MOVE_DECY:
      biopsyStepDecY();
      break;
  case _MCC_BIOPSY_CMD_MOVE_INCZ:
      biopsyStepIncZ();
      break;
  case _MCC_BIOPSY_CMD_MOVE_DECZ:
      biopsyStepDecZ();
      break;
  case _MCC_BIOPSY_CMD_SET_STEPVAL:
      generalConfiguration.biopsyCfg.stepVal =  mcc_cmd.buffer[1];
      break;
  case _MCC_BIOPSY_CMD_SET_LAGO:
      generalConfiguration.biopsyCfg.lunghezzaAgo =  mcc_cmd.buffer[1];
      break;
  case _MCC_BIOPSY_CMD_RESET_BYM:

      BiopsyDriverReset();
      break;

  }

  return;
}



// Questo comando restituisce il valore della posizione del piano di compressione
void mcc_pcb215_get_trolley(void){
    unsigned char data;

    // Chiede alla 215 la posizione del piano compressore
    // Notifica esito impostazione filtro
    int posizione = pcb215GetSpessoreNonCompresso();
    if(posizione>255) posizione = 255;
    data = (unsigned char) posizione;
    mccGuiNotify(mcc_cmd.id,mcc_cmd.cmd,&data,1);

}

uint32_t getU32val(unsigned char* buf){
    return buf[0] + 256 * buf[1] + 256 * 256 * buf[2] + 256 * 256 * 256 * buf[3];
}

void mcc_canopen(void)
{
  unsigned char buffer[8];

  buffer[0]= ACTUATORS_CMD_TEST;
  memcpy(&buffer[1],mcc_cmd.buffer,7);
  CanSendToActuatorsSlave(buffer);

}

//______________________________________________________________________________________________________________
//                      CALIBRAZIONE ZERO MECCANICO E GONIO
// Istruzioni per la calibrazione degli zeri della macchina
// mcc_cmd.id
// mcc_cmd.cmd = MCC_CALIB_ZERO
// mcc_cmd.buffer[0] = comando specifico
void mcc_calib_zero(void){
    unsigned char data[2];
    data[0] = mcc_cmd.buffer[0];
    data[1] = 255; // Comando accettato

    if(mcc_cmd.buffer[0] == CALIB_ZERO_MANUAL_ACTIVATION_TRX_CALIB){
        generalConfiguration.manual_mode_activation = _MANUAL_ACTIVATION_TRX_CALIB;
        printf("SELEZIONATA MODALITA DI MOVIMENTO MANUALE:%d\n",generalConfiguration.manual_mode_activation);
    }else if(mcc_cmd.buffer[0] == CALIB_ZERO_MANUAL_ACTIVATION_ARM_CALIB){
        generalConfiguration.manual_mode_activation = _MANUAL_ACTIVATION_ARM_CALIB;
        printf("SELEZIONATA MODALITA DI MOVIMENTO MANUALE:%d\n",generalConfiguration.manual_mode_activation);
    }else if(mcc_cmd.buffer[0] == CALIB_ZERO_MANUAL_ACTIVATION_TRX_STANDARD){
        generalConfiguration.manual_mode_activation = _MANUAL_ACTIVATION_TRX_STANDARD;
        printf("SELEZIONATA MODALITA DI MOVIMENTO MANUALE:%d\n",generalConfiguration.manual_mode_activation);
    }else if(mcc_cmd.buffer[0] == CALIB_ZERO_MANUAL_ACTIVATION_ARM_STANDARD){
        generalConfiguration.manual_mode_activation = _MANUAL_ACTIVATION_ARM_STANDARD;
        printf("SELEZIONATA MODALITA DI MOVIMENTO MANUALE:%d\n",generalConfiguration.manual_mode_activation);
    }else if(mcc_cmd.buffer[0] == CALIB_ZERO_ACTIVATE_TRX_ZERO_SETTING){
        printf("TRX ZERO SETTING ..\n");
        generalConfiguration.trxExecution.id = mcc_cmd.id;
        actuatorsTrxActivateZeroSetting();
    }else if(mcc_cmd.buffer[0] == CALIB_ZERO_ACTIVATE_ARM_ZERO_SETTING){
        printf("ARM ZERO SETTING ..\n");
    }else if(mcc_cmd.buffer[0] == CALIB_ZERO_ACTIVATE_GONIO_ZERO_SETTING){

       // Ferma tutti i driver per non sollecitare il gonio
       Ser422DriverFreezeAll(5000);
       printf("COMANDO RESET INCLINOMETRO....  ");

       // Carica i registri per impostare l'angolo
       if(pcb249U1ResetGonio(0)==FALSE) data[1]=0;
       else data[1]=1;
       _time_delay(500);
       Ser422DriverSetReadyAll(5000);
       if(data[1]) printf("INCLINOMETRO OK\n");
       else   printf("INCLINOMETRO  FALLITO\n");
    }

    // Feedback di ricezione
    mccGuiNotify(mcc_cmd.id,mcc_cmd.cmd,data,sizeof(data));


}

void mccResetGonio(unsigned char id, unsigned char mcccode)
{
   unsigned char risultato;

  // Ferma tutti i driver per non sollecitare il gonio
  Ser422DriverFreezeAll(5000);

  printf("COMANDO RESET INCLINOMETRO....  ");

  // Carica i registri per impostare l'angolo
  if(pcb249U1ResetGonio(0)==FALSE) risultato=0;
  else risultato=1;
  _time_delay(500);
  Ser422DriverSetReadyAll(5000);

  if(risultato) printf("INCLINOMETRO OK\n");
  else   printf("INCLINOMETRO  FALLITO\n");

  mccGuiNotify(id,mcccode,&risultato,1);
  return ;
}

/*
 *
 *
 */
void mcc_xray_analog_manual(void){
    unsigned char chk,i;

    // Codice sequenza
    rxStdParam.analog_sequence = MANUAL_MODE_EXPOSURE;
    rxStdParam.mcc_code = MCC_XRAY_ANALOG_MANUAL;
    rxStdParam.dmAs_released = 0;
    rxStdParam.dmAs_pre_released = 0;

    // Preparazione parametri sequenza
    rxStdParam.config=1; // Esposizione senza detector
    rxStdParam.esposizione.HV=(0x0fff & (mcc_cmd.buffer[0]+256*mcc_cmd.buffer[1]));   // Vdac
    rxStdParam.esposizione.HV|=(0x7000&((unsigned short)mcc_cmd.buffer[7]<<12));      // Aggiunge lo stato degli switch generatore + velocità starter
    rxStdParam.esposizione.I=(0x0FFF & (mcc_cmd.buffer[2]+256*mcc_cmd.buffer[3]));    // Idac
    rxStdParam.esposizione.I|=(0x7000&((unsigned short)mcc_cmd.buffer[8]<<12));       // Aggiunge la tensione di griglia
    rxStdParam.esposizione.MAS=mcc_cmd.buffer[4]+256*mcc_cmd.buffer[5];               // MAS-DAC
    rxStdParam.esposizione.TMO=mcc_cmd.buffer[6];                                     // Timout Esposizione
    rxStdParam.esposizione.MINHV=mcc_cmd.buffer[10];
    rxStdParam.esposizione.MAXHV=mcc_cmd.buffer[9];
    rxStdParam.esposizione.MINI=mcc_cmd.buffer[12];
    rxStdParam.esposizione.MAXI=mcc_cmd.buffer[11];
    rxStdParam.esposizione.CHK=0; // !!!!! DEVE ESSERE IMPOSTATO A ZERO PRIMA DEL CALCOLO
    chk=0;// Calcolo checksum sui parametri di esposizione
    for(i=0; i< sizeof(_RxParam_Str); i++) chk^=((unsigned char*) &rxStdParam.esposizione) [i];
    rxStdParam.esposizione.CHK= chk;

    // Nessuno sblocco compressore
    rxStdParam.compressor_unlock = mcc_cmd.buffer[13];

    // STAMPA I VALORI NOMINALI
    float kV = (float) (mcc_cmd.buffer[14]+ 256 * mcc_cmd.buffer[15]) / 10;
    float In = (float) (mcc_cmd.buffer[16]+ 256 * mcc_cmd.buffer[17]) / 10;
    float mAs =(float) (mcc_cmd.buffer[4]+256*mcc_cmd.buffer[5]) / 50;

    printf("COMANDO ESPOSIZIONE ANALOGICA MANUALE:\n");
    printf("KV:%f\n",kV);
    printf("mAs:%f\n",mAs);
    printf("In:%f\n\n\n\n",In);
    rxStdParam.mAs_nom=mAs;
    rxStdParam.In=In;

   // Partenza sequenza
   _EVCLR(_SEQEV_RX_ANALOG_TERMINATED);
   _EVSET(_SEQEV_RX_ANALOG_START);

 }

void mcc_xray_analog_auto(void){
    unsigned char chk,i;

        // Errore comunicato dalla GUI dopo il pre impulso
        if((mcc_cmd.buffer[21] !=0 ) && (mcc_cmd.buffer[20])){
            rxStdParam.guiError = mcc_cmd.buffer[21];
            printf("AEC Error code: %d\n", mcc_cmd.buffer[21] );
            _EVSET(_EV2_WAIT_AEC);
            return;
        }
        rxStdParam.guiError = 0;

        // Codice sequenza
        rxStdParam.analog_sequence = AEC_MODE_EXPOSURE;
        rxStdParam.mcc_code = MCC_XRAY_ANALOG_AUTO;
        rxStdParam.dmAs_released = 0;
        rxStdParam.dmAs_pre_released = 0;

        // Preparazione parametri sequenza
        rxStdParam.config=1; // Esposizione senza detector
        rxStdParam.esposizione.HV=(0x0fff & (mcc_cmd.buffer[0]+256*mcc_cmd.buffer[1]));   // Vdac
        rxStdParam.esposizione.HV|=(0x7000&((unsigned short)mcc_cmd.buffer[7]<<12));      // Aggiunge lo stato degli switch generatore + velocità starter
        rxStdParam.esposizione.I=(0x0FFF & (mcc_cmd.buffer[2]+256*mcc_cmd.buffer[3]));    // Idac
        rxStdParam.esposizione.I|=(0x7000&((unsigned short)mcc_cmd.buffer[8]<<12));       // Aggiunge la tensione di griglia
        rxStdParam.esposizione.MAS=mcc_cmd.buffer[4]+256*mcc_cmd.buffer[5];               // MAS-DAC
        rxStdParam.esposizione.TMO=mcc_cmd.buffer[6];                                     // Timout Esposizione
        rxStdParam.esposizione.MINHV=mcc_cmd.buffer[10];
        rxStdParam.esposizione.MAXHV=mcc_cmd.buffer[9];
        rxStdParam.esposizione.MINI=mcc_cmd.buffer[12];
        rxStdParam.esposizione.MAXI=mcc_cmd.buffer[11];
        rxStdParam.esposizione.CHK=0; // !!!!! DEVE ESSERE IMPOSTATO A ZERO PRIMA DEL CALCOLO
        chk=0;// Calcolo checksum sui parametri di esposizione
        for(i=0; i< sizeof(_RxParam_Str); i++) chk^=((unsigned char*) &rxStdParam.esposizione) [i];
        rxStdParam.esposizione.CHK= chk;

        // Sblocco compressore
        rxStdParam.compressor_unlock =  mcc_cmd.buffer[13];

        // STAMPA I VALORI NOMINALI
        float kV = (float) (mcc_cmd.buffer[14]+ 256 * mcc_cmd.buffer[15]) / 10;
        float In = (float) (mcc_cmd.buffer[16]+ 256 * mcc_cmd.buffer[17]) / 10;
        float mAs =(float) (mcc_cmd.buffer[4]+256*mcc_cmd.buffer[5]) / 50;
        unsigned short  pulses =(unsigned short) (mcc_cmd.buffer[18]+256*mcc_cmd.buffer[19]);
        rxStdParam.pulses = pulses;

        if(mcc_cmd.buffer[20]==0){
            rxStdParam.dmAs_released = 0;
            rxStdParam.pulses_released = 0;

            printf("________________________________________________________________________________\n");
            printf("                INIZIO PROCEDURA ESPOSIZIONE AUTOMATICA:\n");
            printf("\n>>PRE-KV:%f\n",kV);
            printf(">>PRE-mAs:%f\n",mAs);
            printf(">>PRE-In:%f\n\n",In);

           // Partenza sequenza
            _EVCLR(_EV2_WAIT_AEC);
            _EVCLR(_SEQEV_RX_ANALOG_TERMINATED);
            _EVSET(_SEQEV_RX_ANALOG_START);
            return;

         }else{
            // ____________ DATI PER IMPULSO
            printf("\n>>PULSE-KV:%f\n",kV);
            printf(">>PULSE-mAs:%f\n",mAs);
            printf(">>PULSE-In:%f\n",In);
            printf(">>PULSE-PULSES:%d\n\n",pulses);
            _EVSET(_EV2_WAIT_AEC);
         }
}

void mcc_xray_analog_pre_calib(void){
    unsigned char chk,i;

    // Codice sequenza
    rxStdParam.analog_sequence = EXPOSIMETER_CALIBRATION_PULSE;
    rxStdParam.mcc_code = MCC_XRAY_ANALOG_CALIB_PRE;
    rxStdParam.dmAs_released = 0;

    // Preparazione parametri sequenza
    rxStdParam.config=1; // Esposizione senza detector
    rxStdParam.esposizione.HV=(0x0fff & (mcc_cmd.buffer[0]+256*mcc_cmd.buffer[1]));   // Vdac
    rxStdParam.esposizione.HV|=(0x7000&((unsigned short)mcc_cmd.buffer[7]<<12));      // Aggiunge lo stato degli switch generatore + velocità starter
    rxStdParam.esposizione.I=(0x0FFF & (mcc_cmd.buffer[2]+256*mcc_cmd.buffer[3]));    // Idac
    rxStdParam.esposizione.I|=(0x7000&((unsigned short)mcc_cmd.buffer[8]<<12));       // Aggiunge la tensione di griglia
    rxStdParam.esposizione.MAS=mcc_cmd.buffer[4]+256*mcc_cmd.buffer[5];               // MAS-DAC
    rxStdParam.esposizione.TMO=mcc_cmd.buffer[6];                                     // Timout Esposizione
    rxStdParam.esposizione.MINHV=mcc_cmd.buffer[10];
    rxStdParam.esposizione.MAXHV=mcc_cmd.buffer[9];
    rxStdParam.esposizione.MINI=mcc_cmd.buffer[12];
    rxStdParam.esposizione.MAXI=mcc_cmd.buffer[11];
    rxStdParam.esposizione.CHK=0; // !!!!! DEVE ESSERE IMPOSTATO A ZERO PRIMA DEL CALCOLO
    chk=0;// Calcolo checksum sui parametri di esposizione
    for(i=0; i< sizeof(_RxParam_Str); i++) chk^=((unsigned char*) &rxStdParam.esposizione) [i];
    rxStdParam.esposizione.CHK= chk;

    // Nessuno sblocco compressore
    rxStdParam.compressor_unlock = FALSE;

    // STAMPA I VALORI NOMINALI
    float kV = (float) (mcc_cmd.buffer[14]+ 256 * mcc_cmd.buffer[15]) / 10;
    float In = (float) (mcc_cmd.buffer[16]+ 256 * mcc_cmd.buffer[17]) / 10;
    float mAs =(float) (mcc_cmd.buffer[4]+256*mcc_cmd.buffer[5]) / 50;

    printf("COMANDO VERIFICA/CALIBRAZIONE IMPULSO ESPOSIMETRO:\n");
    printf("KV:%f\n",kV);
    printf("mAs:%f\n",mAs);
    printf("In:%f\n\n\n\n",In);
    rxStdParam.mAs_nom=mAs;
    rxStdParam.In=In;

   // Partenza sequenza
   _EVCLR(_SEQEV_RX_ANALOG_TERMINATED);
   _EVSET(_SEQEV_RX_ANALOG_START);
}

// Funzione di attivazione e completamento sequenza aec per calibrazione profilo
// Il dato in posizione 18 indica se si tratta del PRE o dei dati dell'impulso
void mcc_xray_analog_calib_profile(void){
    unsigned char chk,i;

        // Codice sequenza
        rxStdParam.analog_sequence = AEC_MODE_EXPOSURE;
        rxStdParam.mcc_code = MCC_XRAY_ANALOG_CALIB_PROFILE;
        rxStdParam.dmAs_released = 0;

        // Preparazione parametri sequenza
        rxStdParam.config=1; // Esposizione senza detector
        rxStdParam.esposizione.HV=(0x0fff & (mcc_cmd.buffer[0]+256*mcc_cmd.buffer[1]));   // Vdac
        rxStdParam.esposizione.HV|=(0x7000&((unsigned short)mcc_cmd.buffer[7]<<12));      // Aggiunge lo stato degli switch generatore + velocità starter
        rxStdParam.esposizione.I=(0x0FFF & (mcc_cmd.buffer[2]+256*mcc_cmd.buffer[3]));    // Idac
        rxStdParam.esposizione.I|=(0x7000&((unsigned short)mcc_cmd.buffer[8]<<12));       // Aggiunge la tensione di griglia
        rxStdParam.esposizione.MAS=mcc_cmd.buffer[4]+256*mcc_cmd.buffer[5];               // MAS-DAC
        rxStdParam.esposizione.TMO=mcc_cmd.buffer[6];                                     // Timout Esposizione
        rxStdParam.esposizione.MINHV=mcc_cmd.buffer[10];
        rxStdParam.esposizione.MAXHV=mcc_cmd.buffer[9];
        rxStdParam.esposizione.MINI=mcc_cmd.buffer[12];
        rxStdParam.esposizione.MAXI=mcc_cmd.buffer[11];
        rxStdParam.esposizione.CHK=0; // !!!!! DEVE ESSERE IMPOSTATO A ZERO PRIMA DEL CALCOLO
        chk=0;// Calcolo checksum sui parametri di esposizione
        for(i=0; i< sizeof(_RxParam_Str); i++) chk^=((unsigned char*) &rxStdParam.esposizione) [i];
        rxStdParam.esposizione.CHK= chk;

        // Nessuno sblocco compressore
        rxStdParam.compressor_unlock = FALSE;

        // STAMPA I VALORI NOMINALI
        float kV = (float) (mcc_cmd.buffer[14]+ 256 * mcc_cmd.buffer[15]) / 10;
        float In = (float) (mcc_cmd.buffer[16]+ 256 * mcc_cmd.buffer[17]) / 10;
        float mAs =(float) (mcc_cmd.buffer[4]+256*mcc_cmd.buffer[5]) / 50;
        unsigned short  pulses =(unsigned short) (mcc_cmd.buffer[18]+256*mcc_cmd.buffer[19]);
        rxStdParam.pulses = pulses;

        if(mcc_cmd.buffer[20]==0){
            rxStdParam.dmAs_released = 0;
            rxStdParam.pulses_released = 0;

            printf("________________________________________________________________________________\n");
            printf("                INIZIO PROCEDURA CALIBRAZIONE PROFILO:\n");
            printf("\n>>PRE-KV:%f\n",kV);
            printf(">>PRE-mAs:%f\n",mAs);
            printf(">>PRE-In:%f\n\n",In);

           // Partenza sequenza
            _EVCLR(_EV2_WAIT_AEC);
            _EVCLR(_SEQEV_RX_ANALOG_TERMINATED);
            _EVSET(_SEQEV_RX_ANALOG_START);
            return;

         }else{
            // ____________ DATI PER IMPULSO
            printf("\n>>PULSE-KV:%f\n",kV);
            printf(">>PULSE-mAs:%f\n",mAs);
            printf(">>PULSE-In:%f\n",In);
            printf(">>PULSE-PULSES:%d\n\n",pulses);
            _EVSET(_EV2_WAIT_AEC);
         }
}


// Funzione di attivazione e completamento sequenza aec per calibrazione profilo
// Il dato in posizione 18 indica se si tratta del PRE o dei dati dell'impulso

/*
    data[0] =  (unsigned char) (pc_selected_vdac&0x00FF);
    data[1] =  (unsigned char) (pc_selected_vdac>>8);
    data[2] =  (unsigned char) (pc_selected_Idac&0x00FF);
    data[3] =  (unsigned char) (pc_selected_Idac>>8);
    data[4] =  (unsigned char) ((pc_selected_mAs*50)&0x00FF);
    data[5] =  (unsigned char) ((pc_selected_mAs*50)>>8);
    data[6] =  20 | 0x80; // 200ms massimi per ogni impulso: usa il timer corto da 10ms

    // Switch Generatore + Alta Velocita
    unsigned char SWA = pGeneratore->tube[pc_selected_kV-_MIN_KV].vRef.SWA;
    unsigned char SWB = pGeneratore->tube[pc_selected_kV-_MIN_KV].vRef.SWB;

    data[7]=0;
    if(SWA) data[7]|=1;
    if(SWB) data[7]|=2;
    data[7]|=4; // Starter HS always ON

    // Tensione Griglia
    data[8] =  0;

    // Diagnostica Tensione / Corrente anodica (determinato dal breve Timeout utilizzato)
    data[9] =  255;
    data[10] = 0;
    data[11] = 255;
    data[12] = 0;

    // No Sblocco compressore dopo esposizione
    data[13] = 0;

    // Aggiungo i valori nominali inviati al driver
    data[14] = (unsigned char) ((unsigned int) (pc_selected_kV * 10) & 0x00FF);
    data[15] = (unsigned char) ((unsigned int) (pc_selected_kV * 10) >> 8);
    data[16] = (unsigned char) ((unsigned int) (pc_selected_Ia * 10) & 0x00FF);
    data[17] = (unsigned char) ((unsigned int) (pc_selected_Ia * 10) >> 8);

    data[18] = 0; // Riservato per gli impulsi dell'esposimetro
    data[19] = 0;
    data[20] = 0; // 0=pre-pulse, 1 = Pulse

    // Prova ad inviare il comando
    if(pConsole->pGuiMcc->sendFrame(MCC_XRAY_ANALOG_CALIB_TUBE,1,data,21)==FALSE)
*/
void mcc_xray_analog_calib_tube(void){
    unsigned char chk,i;

        // Codice sequenza
        rxStdParam.analog_sequence = TUBE_CALIBRATION_PROFILE;
        rxStdParam.mcc_code = MCC_XRAY_ANALOG_CALIB_TUBE;
        rxStdParam.dmAs_released = 0;

        // Preparazione parametri sequenza
        rxStdParam.config=1; // Esposizione senza detector
        rxStdParam.esposizione.HV=(0x0fff & (mcc_cmd.buffer[0]+256*mcc_cmd.buffer[1]));   // Vdac
        rxStdParam.esposizione.HV|=(0x7000&((unsigned short)mcc_cmd.buffer[7]<<12));      // Aggiunge lo stato degli switch generatore + velocità starter
        rxStdParam.esposizione.I=(0x0FFF & (mcc_cmd.buffer[2]+256*mcc_cmd.buffer[3]));    // Idac
        rxStdParam.esposizione.I|=(0x7000&((unsigned short)mcc_cmd.buffer[8]<<12));       // Aggiunge la tensione di griglia
        rxStdParam.esposizione.MAS=mcc_cmd.buffer[4]+256*mcc_cmd.buffer[5];               // MAS-DAC
        rxStdParam.esposizione.TMO=mcc_cmd.buffer[6];                                     // Timout Esposizione
        rxStdParam.esposizione.MINHV=mcc_cmd.buffer[10];
        rxStdParam.esposizione.MAXHV=mcc_cmd.buffer[9];
        rxStdParam.esposizione.MINI=mcc_cmd.buffer[12];
        rxStdParam.esposizione.MAXI=mcc_cmd.buffer[11];
        rxStdParam.esposizione.CHK=0; // !!!!! DEVE ESSERE IMPOSTATO A ZERO PRIMA DEL CALCOLO
        chk=0;// Calcolo checksum sui parametri di esposizione
        for(i=0; i< sizeof(_RxParam_Str); i++) chk^=((unsigned char*) &rxStdParam.esposizione) [i];
        rxStdParam.esposizione.CHK= chk;

        // Nessuno sblocco compressore
        rxStdParam.compressor_unlock = FALSE;

        // STAMPA I VALORI NOMINALI
        float kV = (float) (mcc_cmd.buffer[14]+ 256 * mcc_cmd.buffer[15]) / 10;
        float In = (float) (mcc_cmd.buffer[16]+ 256 * mcc_cmd.buffer[17]) / 10;
        float mAs =(float) (mcc_cmd.buffer[4]+256*mcc_cmd.buffer[5]) / 50;

        rxStdParam.pulses = 0;
        rxStdParam.dmAs_released = 0;
        rxStdParam.pulses_released = 0;

        printf("________________________________________________________________________________\n");
        if(mcc_cmd.buffer[20]) printf("                INIZIO PROCEDURA CALIBRAZIONE CORRENTE ANODICA TUBO:\n");
        else printf("                INIZIO PROCEDURA CALIBRAZIONE KV TUBO:\n");

        printf("\n>>KV:%f\n",kV);
        printf(">>mAs:%f\n",mAs);
        printf(">>In:%f\n\n",In);

       // Partenza sequenza
        _EVCLR(_EV2_WAIT_AEC);
        _EVCLR(_SEQEV_RX_ANALOG_TERMINATED);
        _EVSET(_SEQEV_RX_ANALOG_START);
        return;

}


void mccBiopsySimulator(void){
#ifdef __BIOPSY_SIMULATOR
    unsigned short X, Y;

    if(mcc_cmd.buffer[0]== 1){ // Impostazione stato della connessione
        printf("EXEC SIM CONNECTION\n");
        if(mcc_cmd.buffer[1]==1) SimConnessione(true);
        else SimConnessione(false);
    }else if(mcc_cmd.buffer[0]== 2){ // Impostazione stato del pulsante di sblocco
        printf("EXEC SIM SBLOCCO\n");
        if(mcc_cmd.buffer[1]==1) SimSetPush(true);
        else SimSetPush(false);
    }else if(mcc_cmd.buffer[0]== 3){ // Impostazione Adapter Id
        printf("EXEC SIM ADAPTER\n");
        SimSetAdapter(mcc_cmd.buffer[1]);
    }else if(mcc_cmd.buffer[0]== 4){ // Simulazione pulsanti console
        printf("EXEC CONSOLE PUSH\n");
        SimSetConsolePush(mcc_cmd.buffer[1]);
    }else if(mcc_cmd.buffer[0]== 5){ // Simulazione pulsanti console
        printf("EXEC CONSOLE XY\n");
        // Il dato deve essere in millimetri rispetto al vertice in alto a sinistra (dmm)
        SimSetJXY(XtoJoysticX(mcc_cmd.buffer[1] + 256 * mcc_cmd.buffer[2]),YtoJoysticY(mcc_cmd.buffer[3] + 256 * mcc_cmd.buffer[4]));
    }
#endif
    return;
}

/*
 *  MESSAGGI AUDIO
 *  mcc_cmd.buffer[0] == 0 -> Impostazioni
 *      mcc_cmd.buffer[1] == 0 Inizializzazione
 *
 *  mcc_cmd.buffer[0] == 1 -> Riproduzione
 *      mcc_cmd.buffer[1] == Codice numerico messaggio
 *      mcc_cmd.buffer[2] == Volume (0=max, 255=min)
 */
void mcc_audio(void){    
    char filename[20];
    filename[19]='\0';

    // Se non è inizializzato non fa nulla
    if(!generalConfiguration.audioInitiated) return;

    switch(mcc_cmd.buffer[0]){
    case 0: // Impostazioni
            switch(mcc_cmd.buffer[1]){
                case 0:
                    vmInit();
                    return;
                break;
            }

        break; // Impostazioni

    case 1: // Riproduzione messaggi vocali
        // Cerca uno spazio libero nella coda
        if(nextAudioMessageCode !=0) {
            printf("VM MESSAGE %d BUSY\n",mcc_cmd.buffer[1] );
            return;
        }

        nextAudioMessageCode = mcc_cmd.buffer[1];
        nextAudioMessageVol =mcc_cmd.buffer[2];
        _EVSET(_EV0_VMUSIC_EVENT);

        break;
    }


}

void mcc_rtc(void){
    char* pDate;
    char buffer[30];
    int num;
    switch(mcc_cmd.buffer[0]){
    case 0: // Init RTC
        if(rtcInit()==0) printf("RTC INIIALIZED\n");
        else printf("RTC INIIT FAILED\n");
        break;

    case 1: // Comando di set
        if(!generalConfiguration.rtc_present){
            if(rtcInit()!=0) {
                printf("RTC DEVICE NOT PRESENT");
                return;
            }
            generalConfiguration.rtc_present = true;
        }

        if(mcc_cmd.len!=9) {
            printf("WRONG PARAMETERS\n");
            break;
        }
        num = (int)mcc_cmd.buffer[2] + (int)mcc_cmd.buffer[3]*256;
        sprintf(buffer, "%s %04d-%02d-%02d %02d:%02d:%02d",getWeekdayStr(mcc_cmd.buffer[1]), num,mcc_cmd.buffer[4],mcc_cmd.buffer[5],mcc_cmd.buffer[6],mcc_cmd.buffer[7],mcc_cmd.buffer[8]);
        printf("  set date: %s\n", buffer);
        rtcSetDate(buffer);
        break;
    case 2:
        if ((pDate = rtcGetDate()) == NULL) break;
        printf("  got date: %s\n", pDate);
        break;
    }

}

/*
 *  = 0,   // Restituisce la revisione corrente
    ,     // Restituisce il campionamento x1
    PCB244_A_ACTIVATE_GRID, // Attiva la griglia per un certo numero di passate
    PCB244_A_GET_CASSETTE,  // Legge la presenza/stato della cassetta
    ,        // Legge la codifica dell'accessorio
    ,     // Imposta il campo corrente
    ,   // Effettua il reset della scheda
           // Forza l'uscita da una sequenza
 */
void mcc_244_A_functions(void){
    unsigned char buffer[20];
    int size=0;
    unsigned int val;

    switch( mcc_cmd.buffer[0]){
    case MCC_PCB244_A_GET_REV:
        if(GetPcb244AFwRevision()){
            buffer[0]=0;
            buffer[1] = generalConfiguration.revisioni.pcb244.maj;
            buffer[2] = generalConfiguration.revisioni.pcb244.min;
            printf("PCB244A: richiesta revisione: %d.%d\n", buffer[1],buffer[2]);
            size=3;
        }else{
            printf("PCB244A: richiesta revisione fallita\n");
            buffer[0]=1;
            size=1;
        }
        break;
    case MCC_PCB244_A_GET_RADx1:
        if(!PCB244_A_sampleRad()){
            printf("PCB244A: comando di campionamento fallito\n");
            buffer[0]=1;
            size=1;
            break;
        }
        _time_delay(100);

        if(PCB244_A_GetRad1(10)){
            buffer[0]=0;
            buffer[1] = _DEVREGL(RG244_A_RAD1,PCB244_A_CONTEST);
            buffer[2] = _DEVREGH(RG244_A_RAD1,PCB244_A_CONTEST);
            val = _DEVREG(RG244_A_RAD1,PCB244_A_CONTEST);
            printf("PCB244A: richiesta radx1:0x%x - %d\n", val,val);
            size=3;
        }else {
            printf("PCB244A: richiesta radx1 fallita\n");
            buffer[0]=1;
            size=1;
        }
        break;
    case MCC_PCB244_A_GET_RADx5:
        if(!PCB244_A_sampleRad()){
            printf("PCB244A: comando di campionamento fallito\n");
            buffer[0]=1;
            size=1;
            break;
        }
        _time_delay(100);

        if(PCB244_A_GetRad5(10)){
            buffer[0]=0;
            buffer[1] = _DEVREGL(RG244_A_RAD5,PCB244_A_CONTEST);
            buffer[2] = _DEVREGH(RG244_A_RAD5,PCB244_A_CONTEST);
            val = _DEVREG(RG244_A_RAD5,PCB244_A_CONTEST);
            printf("PCB244A: richiesta radx5:0x%x - %d\n", val,val);
            size=3;
        }else {
            printf("PCB244A: richiesta radx5 fallita\n");
            buffer[0]=1;
            size=1;
        }
        break;
    case MCC_PCB244_A_GET_RADx25:
        if(!PCB244_A_sampleRad()){
            printf("PCB244A: comando di campionamento fallito\n");
            buffer[0]=1;
            size=1;
            break;
        }
        _time_delay(100);

        if(PCB244_A_GetRad25(10)){
            buffer[0]=0;
            buffer[1] = _DEVREGL(RG244_A_RAD25,PCB244_A_CONTEST);
            buffer[2] = _DEVREGH(RG244_A_RAD25,PCB244_A_CONTEST);
            val = _DEVREG(RG244_A_RAD25,PCB244_A_CONTEST);
            printf("PCB244A: richiesta radx25:0x%x - %d\n", val,val);
            size=3;
        }else {
            printf("PCB244A: richiesta radx25 fallita\n");
            buffer[0]=1;
            size=1;
        }
        break;

    case MCC_PCB244_A_SET_OFFSET:
        buffer[1]=mcc_cmd.buffer[1];
        size=8;
        if(mcc_cmd.buffer[1]==0){
            // Read section
            if(PCB244_A_getOffset(&val)==false){
                buffer[0]=1;
            }else{
                buffer[0]=0;
                printf("OFFSET = %d\n", val);
                buffer[2] = val & 0xFF;
                buffer[3] = (val>>8) & 0xFF;
                PCB244_A_sampleRad();
                PCB244_A_GetRad1(10);
                buffer[4] = _DEVREGL(RG244_A_RAD1,PCB244_A_CONTEST);
                buffer[5] = _DEVREGH(RG244_A_RAD1,PCB244_A_CONTEST);
                PCB244_A_GetRad5(10);
                buffer[6] = _DEVREGL(RG244_A_RAD5,PCB244_A_CONTEST);
                buffer[7] = _DEVREGH(RG244_A_RAD5,PCB244_A_CONTEST);
            }
        }else if(mcc_cmd.buffer[1]==1){
            // Write section
            val = mcc_cmd.buffer[2]+256*mcc_cmd.buffer[3];

            if(!PCB244_A_setOffset(val)){
                buffer[0]=1;
                printf("Setting Offset failed!\n", val);
            }else{
                buffer[0]=0;
                _time_delay(200);
                buffer[2] = val & 0xFF;
                buffer[3] = (val>>8) & 0xFF;
                PCB244_A_sampleRad();
                PCB244_A_GetRad1(10);
                buffer[4] = _DEVREGL(RG244_A_RAD1,PCB244_A_CONTEST);
                buffer[5] = _DEVREGH(RG244_A_RAD1,PCB244_A_CONTEST);
                PCB244_A_GetRad5(10);
                buffer[6] = _DEVREGL(RG244_A_RAD5,PCB244_A_CONTEST);
                buffer[7] = _DEVREGH(RG244_A_RAD5,PCB244_A_CONTEST);
                printf("Offset:%d, rad1:%d rad5:%d\n", val,_DEVREG(RG244_A_RAD1,PCB244_A_CONTEST),_DEVREG(RG244_A_RAD5,PCB244_A_CONTEST));
            }

        }else{
            printf("ESECUZIONE AZZERAMENTO OFFSET\n;");
            size=8;
            if(PCB244_A_zeroOffset()) buffer[0]=0;
            else buffer[0]=1;
            PCB244_A_sampleRad();
            PCB244_A_GetRad1(10);
            buffer[4] = _DEVREGL(RG244_A_RAD1,PCB244_A_CONTEST);
            buffer[5] = _DEVREGH(RG244_A_RAD1,PCB244_A_CONTEST);
            PCB244_A_GetRad5(10);
            buffer[6] = _DEVREGL(RG244_A_RAD5,PCB244_A_CONTEST);
            buffer[7] = _DEVREGH(RG244_A_RAD5,PCB244_A_CONTEST);

        }

        break;
    case MCC_PCB244_A_ACTIVATE_GRID:
        printf("ESECUZIONE TEST GRID\n;");
        pcb244A_Start2dGrid(mcc_cmd.buffer[1]);
        break;
    case MCC_PCB244_A_GET_CASSETTE:
        pcb244AGetCassette();
        buffer[0]=0;
        buffer[1]=generalConfiguration.potterCfg.cassette;
        buffer[2]=generalConfiguration.potterCfg.cassetteExposed;
        size=3;
        if((buffer[1]) && (!buffer[2])) printf("PCB244A:  CASSETTA PRESENTE NON ESPOSTA\n");
        else if((buffer[1]) && (buffer[2])) printf("PCB244A:  CASSETTA PRESENTE GIA' ESPOSTA\n");
        else      printf("PCB244A:  MANCANZA CASSETTA\n");
        break;

    case MCC_PCB244_A_GET_ID:
        pcb244AGetAccessorio();
        buffer[0]=0;
        buffer[1]=generalConfiguration.potterCfg.potId;
        buffer[2]=generalConfiguration.potterCfg.potDescriptor;
        buffer[3]=generalConfiguration.potterCfg.potMagFactor;
        size=4;

        switch(buffer[1]){
        case POTTER_2D:
            if(buffer[2]==POTTER_DESCR_18x24) printf("PCB244A:  POTTER 18x24\n");
            else printf("PCB244A:  POTTER 24x30\n");
            break;
        case POTTER_MAGNIFIER:
            printf("PCB244A:  POTTER MAGNIFIER. MAG FACTOR=%d\n",buffer[3]);
            break;
        case POTTER_UNDEFINED:
            printf("PCB244A:  POTTER UNDEFINED\n");
            break;
        }

        break;
    case MCC_PCB244_A_SET_FIELD:
        if(PCB244_A_setDetectorField(mcc_cmd.buffer[1])){

            switch(mcc_cmd.buffer[1]){
            case _ANALOG_DETECTOR_FRONT_FIELD:
                printf("PCB244A: impostato campo FRONT\n");
                break;
            case _ANALOG_DETECTOR_MIDDLE_FIELD:
                printf("PCB244A: impostato campo MIDDLE\n");
                break;
            case _ANALOG_DETECTOR_BACK_FIELD:
                printf("PCB244A: impostato campo BACK\n");
                break;
            default:
                printf("PCB244A: impostato campo OPEN\n");
            }

            buffer[0]=0;
            size=1;
        }else{
            printf("PCB244A: errore impostazione campo\n");
            buffer[0]=1;
            size=1;
        }


        break;
    case MCC_PCB244_A_RESET_BOARD:
        break;
    case MCC_PCB244_A_RX_ABORT:
        break;
    case MCC_PCB244_A_SET_CASSETTE:
        setCassetteStat(mcc_cmd.buffer[1]);
        break;
    }


    if(size) {
        mccPCB244ANotify(1, mcc_cmd.buffer[0],buffer,size);
    }
}

/* EOF */
