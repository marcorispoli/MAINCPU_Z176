#define _SEQRX_TOMO_C
#include "dbt_m4.h"
#undef _SEQERROR
#undef Param

static void RxTomoSeqError(int code);
_RxStdSeq_Str tomoParam;
bool tomoSeqResult=FALSE;
bool tomoIsRunning=FALSE;
unsigned char tomoError;

// Hotfix 11C
int tomoCurrentFilterPosition;
int tomoFilterTarget;

#define Param (&tomoParam)
#define _SEQERROR(code) {RxTomoSeqError(code); continue;}

void tomo_rx_task(uint32_t taskRegisters)
{
  unsigned char data[5];
  unsigned short mAs_erogati=0;

  unsigned char i;
  unsigned short sval;
  unsigned char chk;
  _DeviceAppRegister_Str        ConfList;
  
   
  printf("PARTENZA SEQUENZA PER GESTIONE RAGGI TOMO\n");
  _EVCLR(_SEQEV_RX_TOMO_START);
  
  while(1)
  {
    // Attende fino a nuova partenza
    tomoIsRunning=FALSE;
    _EVCLR(_SEQEV_RX_TOMO_START);
    _EVWAIT_ALL(_SEQEV_RX_TOMO_START);
    _EVCLR(_SEQEV_RX_TOMO_TERMINATED);
    tomoSeqResult=FALSE;
    tomoIsRunning=TRUE;
    tomoError=0;
    
    
    if(generalConfiguration.demoMode) printf("SEQUENZA TOMO ATTIVATA IN DEMO MODE\n");
    else  printf("SEQUENZA TOMO ATTIVATA \n");

    // Prima di andare in freeze bisogna accertarsi che la collimazione 2D sia andata a buon fine
    if(wait2DBackFrontCompletion(100)==false) _SEQERROR(ERROR_INVALID_COLLI);
    if(wait2DLeftRightTrapCompletion(100)==false) _SEQERROR(ERROR_INVALID_COLLI);

    // Manda subito in FREEZE i drivers per non intralciare le operazioni
    // Non viene però atteso che effettivamente i drivers si fermino
    Ser422DriverFreezeAll(0);

    //________________________________________________________________________________________________
    // Prima di procedere bisogna verificare se il filtro ha terminato correttamente il posizionamento
    if(waitRxFilterCompletion()==FALSE)  _SEQERROR(ERROR_INVALID_FILTRO);

    // Verifica Chiusura porta
    if((SystemInputs.CPU_CLOSED_DOOR==0) && (!generalConfiguration.demoMode))
    {
        printf("PORTA STUDIO APERTA!\n");
        _SEQERROR(ERROR_CLOSED_DOOR);
    }

    // Reset Eventuale Fault della PCB190
    pcb190ResetFault();

    // Attiva Starter
    if(!generalConfiguration.demoMode){
      if(Param->esposizione.HV & 0x4000)
      {
        if(pcb190StarterH()==FALSE) printf("WARNING: COMANDO STARTER HIGH FALLITO\n");
        else printf("STARTER ATTIVATO AD ALTA VELOCITA'\n");
      }else
      {
        if(pcb190StarterL()==FALSE) printf("WARNING: COMANDO STARTER LOW FALLITO\n");
        else printf("STARTER ATTIVATO A BASSA VELOCITA'\n");
      }
    }

    // Manda subito il Braccio in Home Tomo.
    if(actuatorsMoveTomoTrxHome(Param->tomo_mode)==false) _SEQERROR(_SEQ_ERR_INTERMEDIATE_HOME);

    // Specchio fuori campo se non è già stato  levato (comando compatibile FREEZE)
    if(pcb249U2Lamp(2,100,true) == FALSE) _SEQERROR(ERROR_MIRROR_LAMP);

    // Verifica pulsante raggi
    if(SystemInputs.CPU_XRAY_REQ==0)  _SEQERROR(ERROR_PUSHRX_NO_PREP);

    // Impostazione collimazione Dinamica solo se non in calibrazione
    // Questi comandi sono compatibili con il modo FREEZE
    pcb249U1ResetFaults();
    if((Param->tomo_mode!=_TOMO_MODE_STATIC)&&(!generalConfiguration.demoMode))
    {
      if(generalConfiguration.filterTomoEna!=0){
          Ser422ReadRegister(_REGID(RG249U2_POS_TARGET),4,&PCB249U2_CONTEST);
          tomoCurrentFilterPosition = _DEVREGL(RG249U2_POS_TARGET,PCB249U2_CONTEST);           
      }
      
      printf("ANGOLO BRACCIO PER COLLIMAZIONE TOMO:%d\n",generalConfiguration.armExecution.dAngolo/10);

      // Si deve esprimere l'angolo in 0.025 °/unit per compatibilità con collimatore
      short angolo = generalConfiguration.armExecution.dAngolo * 4;
      if(Ser422WriteRegister(_REGID(RG249U1_GONIO16_ARM),angolo,10,&PCB249U1_CONTEST) != _SER422_NO_ERROR)
          _SEQERROR(_SEQ_WRITE_REGISTER);
      
      // Impostazione collimatori ..
      if(pcb249U1SetColliCmd(3)==FALSE) _SEQERROR(_SEQ_ERR_COLLI_TOMO); // Imposta la modalità tomo
      if(pcb249U2ColliCmd(generalConfiguration.colliCfg.dynamicArray.tomoBack, generalConfiguration.colliCfg.dynamicArray.tomoFront)==FALSE) _SEQERROR(_SEQ_ERR_COLLI_TOMO);
    }
    
    // Verifica pulsante raggi
    if(SystemInputs.CPU_XRAY_REQ==0)  _SEQERROR(ERROR_PUSHRX_NO_PREP);
    
    // Impostazione dei segnali di abilitazione
    if(!generalConfiguration.demoMode){
      
        // Preparazione Registri 190 per attivazione raggi 
        printf("PREPARAZIONE REGISTRI PCB190\n");
        if(pcb190UploadTomoExpose(Param,FALSE)==FALSE) _SEQERROR(_SEQ_UPLOAD190_PARAM);
        
        if(Param->tomo_mode==_TOMO_MODE_WIDE)
          printf("DATI TOMO: ESPOSIZIONE  WIDE --------------------------\n");
        else if(Param->tomo_mode==_TOMO_MODE_NARROW)
          printf("DATI TOMO: ESPOSIZIONE  NARROW --------------------------\n");
        else if(Param->tomo_mode==_TOMO_MODE_INTERMEDIATE)
          printf("DATI TOMO: ESPOSIZIONE  INTERMEDIATE --------------------------\n");
        else
          printf("DATI TOMO: ESPOSIZIONE  A BRACCIO FERMO --------------------------\n");

        printf("IDAC:%d\n",Param->esposizione.I);
        printf("VDAC:%d\n",Param->esposizione.HV);
        printf("MASDAC:%d\n",Param->esposizione.MAS);   
        printf("SAMPLES:%d\n",Param->tomo_samples);
        printf("PRE SAMPLES:%d\n",Param->tomo_pre_pulses);
        printf("--------------------------------------\n");

        // Impostazione Segnale XRAY_ENA su Bus Hardware
        _mutex_lock(&output_mutex);
        SystemOutputs.CPU_XRAY_ENA=1;   // Attivazione segnale XRAY ENA
        SystemOutputs.CPU_DEMO_ACTIVATION = 0;   // Attivazione Buzzer
        _EVSET(_EV0_OUTPUT_CAMBIATI);         
        _mutex_unlock(&output_mutex);
         
        // Attesa per debounce
        _time_delay(200);

        if(SystemInputs.CPU_XRAY_REQ==0)  _SEQERROR(ERROR_PUSHRX_NO_PREP);

        // Attesa completamento movimento tubo + preparazione per nuovo movimento con EXP-WIN
        // Se si rilascia il pulsante durante il posizionamento verrà segnalato l'errore sul posizionamento
        printf("ATTESA FINE POSIZIONAMENO..\n");
        if(actuatorsTrxWaitReady(100)==false) _SEQERROR(_SEQ_ERR_WIDE_HOME);
        if(Param->tomo_mode!=_TOMO_MODE_STATIC) actuatorsMoveTomoTrxEnd(Param->tomo_mode,true); // actuatorsActivateTrxTriggerStart();

        if((generalConfiguration.filterTomoEna!=0)&&(Param->tomo_mode!=_TOMO_MODE_STATIC)){
          Ser422ReadRegister(_REGID(RG249U1_GONIO_REL),4,&PCB249U1_CONTEST);
          int angolo = (int) _DEVREGL(RG249U1_GONIO_REL,PCB249U1_CONTEST);
          if(angolo&0x80) angolo = -1 * (angolo&0x7F); 
          tomoFilterTarget = getTomoDeltaFilter(angolo) +  tomoCurrentFilterPosition;        
          int i=20;
          while(i--){
              if( pcb249U2SetFiltroRaw(tomoFilterTarget)) break;
              _time_delay(50);
          }
          if(i==0) printf("FALLITO IMPOSTAZIONE FILTRO RAW\n");
        }

        // Attende i segnali e verifica l'uscita con pulsante raggi
        if(SystemInputs.CPU_XRAY_ENA_ACK==0)
        {   
          _EVCLR(_EV2_XRAY_ENA_ON);
          _EVCLR(_EV2_XRAY_REQ_OFF);  
           if(_EVWAIT_TANY(_MOR2(_EV2_XRAY_ENA_ON,_EV2_XRAY_REQ_OFF),_WAIT_XRAY_ENA)==FALSE) _SEQERROR(_SEQ_IO_TIMEOUT);
        }
        if(SystemInputs.CPU_XRAY_REQ==0)  _SEQERROR(ERROR_PUSHRX_NO_PREP);

        // Comando Attivazione Raggi
        if(waitPcb190Ready(50)==FALSE) _SEQERROR(_SEQ_PCB190_BUSY);
        printf("Comando Raggi \n");

        int rc = pcb190StartRxTomo();
        if(rc==SER422_BUSY) _SEQERROR(_SEQ_PCB190_BUSY);
        if(rc==SER422_ILLEGAL_FUNCTION) _SEQERROR(ERROR_PUSHRX_NO_PREP);

        printf("Comando a PCB190 OK. Sleep.....\n");

        // Spin Lock ogni 100ms per testare anche eventuali blocchi sul braccio in movimento
        long rxloop = (_WAIT_XRAY_COMPLETED / 100);        
        int delay = 4; // Per i primi 2 secondi non verifica il fine raggi
        while(rxloop--){

           // Controllo sull'attesa del comnpletamento della sequenza
           if(!delay){
             if(SystemInputs.CPU_XRAY_COMPLETED==1) break; // Fine sequenza
           }else delay--;

           if((generalConfiguration.filterTomoEna!=0)&&(Param->tomo_mode!=_TOMO_MODE_STATIC)){
              Ser422ReadRegister(_REGID(RG249U1_GONIO_REL),4,&PCB249U1_CONTEST);
              int angolo = (int) _DEVREGL(RG249U1_GONIO_REL,PCB249U1_CONTEST);
              if(angolo&0x80) angolo = -1 * (angolo&0x7F); 
              //printf("ANGOLO:%d\n", angolo);

              int  new_filter;
              new_filter = getTomoDeltaFilter(angolo) + tomoCurrentFilterPosition;        

              if(new_filter > tomoFilterTarget){ 
                  tomoFilterTarget = new_filter;
                  pcb249U2SetFiltroRaw(tomoFilterTarget);
                  //printf("ANGOLO:%d  FILTRO:%d\n",angolo, tomoFilterTarget);
              }
           }
           
           _time_delay(100);
        }
        
        if(rxloop==0){_SEQERROR(_SEQ_PCB190_TMO);}

        // Disattivazione XRAY-ENA
       _mutex_lock(&output_mutex);
       SystemOutputs.CPU_XRAY_ENA=0;   // Disattivazione segnale XRAY ENA
       SystemOutputs.CPU_DEMO_ACTIVATION = 0;   // Disattivazione Buzzer
       _EVSET(_EV0_OUTPUT_CAMBIATI);         
       _mutex_unlock(&output_mutex);

       // Ferma subito il braccio
       // actuatorsTrxStop(50); // Attende 2 secondi

       if((generalConfiguration.filterTomoEna!=0)&&(Param->tomo_mode!=_TOMO_MODE_STATIC)){
          if(tomoCurrentFilterPosition!=0){
            if(pcb249U2SetFiltroRaw(tomoCurrentFilterPosition) == false) {
                  printf("COMANDO IMPOSTAZIONE FILTRO STANDARD FALLITA!\n");
            }else{          
                  printf("FILTRO IN POSIZIONE: %d \n", tomoCurrentFilterPosition);
            }
          }
       }
       
       printf("PCB190 SEGNALA FINE SEQUENZA\n");

       // Lettura esito raggi
       if(pcb190GetPostRxRegisters()==FALSE){
           printf("ERRORE DURANTE LETTURA REGISTRI FINE RAGGI!!!!!!! \n");
           _SEQERROR(_SEQ_READ_REGISTER);

       }

    }else{      
      // DEMO MODE
           
      // Attesa completamento movimento tubo       
      int delay = 0;

      // Attesa completamento movimento tubo NO EXP WIN
      // Se si rilascia il pulsante durante il posizionamento verrà segnalato l'errore sul posizionamento
      printf("ATTESA FINE POSIZIONAMENO..\n");
      if(actuatorsTrxWaitReady(100)==false) _SEQERROR(_SEQ_ERR_WIDE_HOME);

      if(Param->tomo_mode==_TOMO_MODE_WIDE) delay = generalConfiguration.trxCfg.tomo.w.samples;
      else if(Param->tomo_mode==_TOMO_MODE_NARROW) delay = generalConfiguration.trxCfg.tomo.n.samples;
      else if(Param->tomo_mode==_TOMO_MODE_INTERMEDIATE) delay = generalConfiguration.trxCfg.tomo.i.samples;

      printf("ATTIVAZIONE MOVIMENTO DEMO TO END");

      _time_delay(200);
      actuatorsMoveTomoTrxEnd(Param->tomo_mode,false); // Partenza immediata braccio
      _time_delay(200);

      
      _mutex_lock(&output_mutex);
      if(generalConfiguration.trxCfg.tomo_mode == _TOMO_MODE_4F){
          SystemOutputs.CPU_DEMO_MODEL = 1;
          SystemOutputs.CPU_DEMO_MODEH = 1;
      }else if(generalConfiguration.trxCfg.tomo_mode == _TOMO_MODE_3F){
          SystemOutputs.CPU_DEMO_MODEL = 0;
          SystemOutputs.CPU_DEMO_MODEH = 1;
      }else if(generalConfiguration.trxCfg.tomo_mode == _TOMO_MODE_2F){
          SystemOutputs.CPU_DEMO_MODEL = 1;
          SystemOutputs.CPU_DEMO_MODEH = 0;
      }else{
          SystemOutputs.CPU_DEMO_MODEL = 0;
          SystemOutputs.CPU_DEMO_MODEH = 0;
      }
      SystemOutputs.CPU_DEMO_ACTIVATION = 0;   // Buzzer Acceso
      SystemOutputs.CPU_DEMO_TOMO = 1;
      _EVSET(_EV0_OUTPUT_CAMBIATI);
      _mutex_unlock(&output_mutex);

      // Ciclo di attesa finta sequenza. La durata dipende dal modo WIDE/NARROW/INTERMEDIATE
      bool error = false;
      while(delay--){

          if(generalConfiguration.trxCfg.tomo_mode == _TOMO_MODE_4F){
              _time_delay(250);
          }else if(generalConfiguration.trxCfg.tomo_mode == _TOMO_MODE_3F){
              _time_delay(330);
          }else if(generalConfiguration.trxCfg.tomo_mode == _TOMO_MODE_2F){
              _time_delay(500);
          }else{
              _time_delay(900);
          }

          if((SystemInputs.CPU_XRAY_REQ==0)) {
              error = true;
              break;                
          }
      } 

      _mutex_lock(&output_mutex);
      SystemOutputs.CPU_DEMO_ACTIVATION = 0;   // Buzzer Acceso
      SystemOutputs.CPU_DEMO_TOMO = 0;
      _EVSET(_EV0_OUTPUT_CAMBIATI);
      _mutex_unlock(&output_mutex);

      if(error) _SEQERROR(ERROR_PUSHRX_NO_PREP);
      _time_delay(1000);

      // Ferma subito il braccio
      //actuatorsTrxStop(20);

    }
    
    // Sblocca il compressore     
    if((Param->tomo_mode!=_TOMO_MODE_STATIC)&&(Param->compressor_unlock))   pcb215SetXRaySblocco();
   
    // Chiude con centratura braccio solo se non in calibrazione del detectror
    if(Param->tomo_mode!=_TOMO_MODE_STATIC){
        actuatorsTrxWaitReady(50); // Senza uomo morto attende il naturale fine movimento
        if(!tomoParam.tomo_deadman) actuatorsTrxMove(0); // Senza uomo morto muove sicuro
        else if(SystemInputs.CPU_XRAY_REQ) actuatorsTrxMove(0); // Se c'è l'uomo morto muove solo col pulsante raggi premuto
    }

    // Verifica  Esito da PCB190
    if(!generalConfiguration.demoMode){
       if(_TEST_BIT(PCB190_FAULT)) _SEQERROR(_DEVREGL(RG190_FAULTS,PCB190_CONTEST));
    }
    printf("RISULTATO RX OK\n");


    // Ritardo acquisizione campionamenti
    _time_delay(50); // Attesa 50ms

    if(!generalConfiguration.demoMode){
      // Lettura mas residui
//      mAs_erogati = _DEVREG(RG190_MAS_EXIT,PCB190_CONTEST)/50;
      mAs_erogati = _DEVREG(RG190_MAS_EXIT,PCB190_CONTEST);

    }else{
        // mAs_erogati = (Param->esposizione.MAS / 50) * Param->tomo_samples;
        mAs_erogati = (unsigned short) (((float) Param->esposizione.MAS  * Param->tomo_samples) * 50);
    }
    
    tomoSeqResult=TRUE;
        
    // Stringa di debug
    printf("SEQUENZA TOMO TERMINATA CON SUCCESSO: mAs=%d\n", mAs_erogati);
    data[0]=RXOK;       
    data[1]=(unsigned char) (mAs_erogati&0xFF);  // Aggiungere mas residui        
    data[2]=(unsigned char) ((mAs_erogati>>8)&0xFF);  
    data[3]= _DEVREGL(RG190_HV_RXEND,PCB190_CONTEST);
    
    // Aggiungere le curve registrate
    mccGuiNotify(1,MCC_CMD_RAGGI_TOMO,data,4); 
 
    // Scarica i dati dell'esposizione se necessario
    if(!generalConfiguration.demoMode) rxNotifyData(2,RXOK);
    
    // Attende fine centratura braccio solo su uomo morto
    if(Param->tomo_mode!=_TOMO_MODE_STATIC){
       if(tomoParam.tomo_deadman){
           // Attende il completamento del posizionamento con il pulsante premuto
           int i = 300;
           while(i--){
              if(SystemInputs.CPU_XRAY_REQ==0){
                  actuatorsTrxStop(20);
                  break;
              }
              if(generalConfiguration.trxExecution.run==false) break;
              _time_delay(100);
           }
           if(i==0) actuatorsTrxStop(20);
       }
    }


   // Sblocco dei processi prima di procedere
   if(Ser422DriverSetReadyAll(5000) == FALSE) printf("FALLITO SBLOCCO DRIVER!!\n");
   else printf("SBLOCCO DRIVER OK\n");

   // Re-imposta la collimazione 2D
   pcb249U2SetColli( generalConfiguration.colliCfg.lame2D.back , generalConfiguration.colliCfg.lame2D.front);
   pcb249U1SetColli(generalConfiguration.colliCfg.lame2D.left,generalConfiguration.colliCfg.lame2D.right,generalConfiguration.colliCfg.lame2D.trap,0);

    // Se richiesto viene spento lo starter
    if(generalConfiguration.pcb190Cfg.starter_off_after_exposure){
        if(generalConfiguration.pcb190Cfg.starter_off_with_brake) pcb190StopStarter();
        else pcb190OffStarter();
    }

    _EVSET(_SEQEV_RX_TOMO_TERMINATED);
    
  } // while
}

void RxTomoSeqError(int code)
{
    unsigned char data[5];
    unsigned short mAs_erogati=0;
    
    printf(" SEQUENZA TOMO STANDARD: ERRORE!!!\n");
    
    // Ferma subito il braccio
    actuatorsTrxStop(20);
    
    _mutex_lock(&output_mutex);
    SystemOutputs.CPU_XRAY_ENA=0;   // Disattivazione segnale XRAY ENA
    SystemOutputs.CPU_DEMO_ACTIVATION = 0;   // Disattivazione Buzzer
    _EVSET(_EV0_OUTPUT_CAMBIATI);         
    _mutex_unlock(&output_mutex);

    // Segnala il risultato
    tomoSeqResult=FALSE;
    
    // Sblocca i drivers
    Ser422DriverSetReadyAll(5000);
    
     // Sblocca il compressore se deve
    if(Param->compressor_unlock)
        pcb215SetSblocco();
    
    // Re-imposta la collimazione 2D
    pcb249U2SetColli( generalConfiguration.colliCfg.lame2D.back , generalConfiguration.colliCfg.lame2D.front);
    pcb249U1SetColli(generalConfiguration.colliCfg.lame2D.left,generalConfiguration.colliCfg.lame2D.right,generalConfiguration.colliCfg.lame2D.trap,0);

    // Verifica se c'è stata radiazione
    if((code>LAST_ERROR_NO_PREP)&&(code<LAST_ERROR_WITH_PREP))
    {
        
        // Lettura mas residui
        if(Ser422ReadRegister(_REGID(RG190_MAS_EXIT),10,&PCB190_CONTEST) == _SER422_NO_ERROR){
            // mAs_erogati = _DEVREG(RG190_MAS_EXIT,PCB190_CONTEST)/50;
            mAs_erogati = _DEVREG(RG190_MAS_EXIT,PCB190_CONTEST);
        }
    }
    
    // Stringa di debug
    printf("TOMO SEQ ERROR:%d\n",code); 
    tomoError = code;
    data[0]=tomoError;       
    data[1]=(unsigned char) (mAs_erogati&0xFF);  // Aggiungere mas residui        
    data[2]=(unsigned char) ((mAs_erogati>>8)&0xFF);      
    data[3]= _DEVREGL(RG190_HV_RXEND,PCB190_CONTEST); 
    mccGuiNotify(1,MCC_CMD_RAGGI_TOMO,data,4); 
      

    // Carica i dati relativi all'esposizione se necessario
    if(!generalConfiguration.demoMode) rxNotifyData(2,code);

    if((generalConfiguration.filterTomoEna!=0)&&(Param->tomo_mode!=_TOMO_MODE_STATIC)){
      if(tomoCurrentFilterPosition!=0){
        if(pcb249U2SetFiltroRaw(tomoCurrentFilterPosition) == false) {
            printf("COMANDO IMPOSTAZIONE FILTRO STANDARD FALLITA!\n");
        }else{          
            printf("FILTRO IN POSIZIONE: %d \n", tomoCurrentFilterPosition);
        }
      }
    }

    // Se richiesto viene spento lo starter
    if(generalConfiguration.pcb190Cfg.starter_off_after_exposure){
        if(generalConfiguration.pcb190Cfg.starter_off_with_brake) pcb190StopStarter();
        else pcb190OffStarter();
    }

    
    _EVSET(_SEQEV_RX_TOMO_TERMINATED);
    return;
}


/* EOF */
