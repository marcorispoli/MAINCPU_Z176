#define _SEQRX_STD_C
#include "dbt_m4.h"
#undef _SEQERROR
#undef _SEQERRORFUNC
#undef PARAM
#undef RESULT
#undef ISRUNNING
#undef ERROR

///////////////////////////////////////////////////////////////////////////////
// Customizzazione dati sequenza
#define _SEQERRORFUNC RxStdSeqError
#define PARAM rxStdParam
#define RESULT rxStdSeqResult
#define ISRUNNING rxStdIsRunning
#define ERROR rxStdError
///////////////////////////////////////////////////////////////////////////////

static void _SEQERRORFUNC(int code);
_RxStdSeq_Str PARAM;
bool RESULT=FALSE;
bool ISRUNNING=FALSE;
unsigned char ERROR;
#define Param (&PARAM)
#define _SEQERROR(code) {_SEQERRORFUNC(code); continue;}
void std_rx_task(uint32_t taskRegisters)
{
  unsigned char data[10];
  unsigned char addr,banco;
  unsigned char i;
  unsigned short sval;
  unsigned char chk;
  
  printf("PARTENZA SEQUENZA PER GESTIONE RAGGI STANDARD\n");
  _EVCLR(_SEQEV_RX_STD_START);
  
  while(1)
  {
    // Attende fino a nuova partenza
    ISRUNNING=FALSE;
    _EVCLR(_SEQEV_RX_STD_START);
    _EVWAIT_ALL(_SEQEV_RX_STD_START);
    _EVCLR(_SEQEV_RX_STD_TERMINATED);
    RESULT=FALSE;
    ISRUNNING=TRUE;
    ERROR=0;
  

    DEBUG_PRINT(__DBG_INIT_2D_DIGITAL_RX);
    if(generalConfiguration.demoMode) DEBUG_PRINT(__DBG_INIT_2D_DIGITAL_RX_DEMO_MODE);
  
    // Prima di andare in freeze bisogna accertarsi che la collimazione 2D sia andata a buon fine
    if(wait2DBackFrontCompletion(100)==false) _SEQERROR(ERROR_INVALID_COLLI);
    if(wait2DLeftRightTrapCompletion(100)==false) _SEQERROR(ERROR_INVALID_COLLI);

    // Disabilita tutti i drivers ed attende che tutti i driver siano fermi
    Ser422DriverFreezeAll(0);    

    //________________________________________________________________________________________________
    // Prima di procedere bisogna verificare se il filtro ha terminato correttamente il posizionamento
    if(waitRxFilterCompletion()==FALSE)  _SEQERROR(ERROR_INVALID_FILTRO);

    // Specchio fuori campo se non è già stato  levato (comando compatibile FREEZE)
    if(pcb249U2Lamp(2,100,true) == FALSE) _SEQERROR(ERROR_MIRROR_LAMP);

    // Verifica Chiusura porta
    if((SystemInputs.CPU_CLOSED_DOOR==0) && (!generalConfiguration.demoMode)) _SEQERROR(ERROR_CLOSED_DOOR);

    // Reset Eventuale Fault della PCB190
    pcb190ResetFault();

    // Attiva Starter precocemente
    if(!generalConfiguration.demoMode){
      if(Param->esposizione.HV & 0x4000)
      {
        if(pcb190StarterH()==FALSE) DEBUG_PRINT(__DBG_INIT_2D_DIGITAL_RX_STARTER_FAILURE);
        else DEBUG_PRINT(__DBG_INIT_2D_DIGITAL_RX_STARTER_HS_ON);
      }else
      {
        if(pcb190StarterL()==FALSE) DEBUG_PRINT(__DBG_INIT_2D_DIGITAL_RX_STARTER_FAILURE);
        else DEBUG_PRINT(__DBG_INIT_2D_DIGITAL_RX_STARTER_LS_ON);
      }
    }    

    // Caricamento parametri di esposizione
    if(pcb190UploadExpose(Param, FALSE)==FALSE) _SEQERROR(_SEQ_UPLOAD190_PARAM);      
 
    printf("DATI IMPULSO --------------------------\n");
    printf("IDAC:%d\n",Param->esposizione.I & 0x0FFF);
    printf("VDAC:%d\n",Param->esposizione.HV & 0x0FFF);
    printf("MASDAC:%d\n",Param->esposizione.MAS);   
    printf("--------------------------------------\n");    
        
    // Verifica su XRAY_REQ(Pulsante raggi premuto)
    if(SystemInputs.CPU_XRAY_REQ==0)  _SEQERROR(ERROR_PUSHRX_NO_PREP);
    
    if(!generalConfiguration.demoMode){

      // Impostazione Segnale XRAY_ENA su Bus Hardware
        _mutex_lock(&output_mutex);
        SystemOutputs.CPU_XRAY_ENA=1;   // Attivazione segnale XRAY ENA
        SystemOutputs.CPU_DEMO_ACTIVATION = 0;   // Attivazione Buzzer
        _EVSET(_EV0_OUTPUT_CAMBIATI);         
        _mutex_unlock(&output_mutex);

        // Attende i segnali
        if(SystemInputs.CPU_XRAY_ENA_ACK==0)
        {   
          _EVCLR(_EV2_XRAY_ENA_ON);
          _EVCLR(_EV2_XRAY_REQ_OFF);  
           if(_EVWAIT_TANY(_MOR2(_EV2_XRAY_ENA_ON,_EV2_XRAY_REQ_OFF),_WAIT_XRAY_ENA)==FALSE) _SEQERROR(_SEQ_IO_TIMEOUT);
        }
        if(SystemInputs.CPU_XRAY_REQ==0)  _SEQERROR(ERROR_PUSHRX_NO_PREP);

        printf("Comando Raggi \n");
    
        // Comando Attivazione Raggi
        _time_delay(100); // Non levare il delay
        
        // Legge il Busy per attendere che sia tutto pronto
        if(waitPcb190Ready(50)==FALSE) _SEQERROR(_SEQ_PCB190_BUSY);

        // VErifica l'attivazione della PCB190
        int rc = pcb190StartRxStd();
        if(rc==SER422_BUSY) _SEQERROR(_SEQ_PCB190_BUSY);
        if(rc==SER422_ILLEGAL_FUNCTION) _SEQERROR(ERROR_PUSHRX_NO_PREP);

        // Un minimo di attesa per consentire ai vari segnali di sincronizzarsi
        _time_delay(1000);

        printf("Attesa Completamento \n");
        
        // Attesa XRAY COMPLETED da Bus Hardware
        if(SystemInputs.CPU_XRAY_COMPLETED==0)
        {
          _EVCLR(_EV2_XRAY_COMPLETED);
          if(_EVWAIT_TALL(_EV2_XRAY_COMPLETED,_WAIT_XRAY_COMPLETED)==FALSE) _SEQERROR(_SEQ_PCB190_TMO);      
        }
     
        // Lettura esito raggi
        if(pcb190GetPostRxRegisters()==FALSE){
            printf("ERRORE DURANTE LETTURA REGISTRI FINE RAGGI!!!!!!! \n");
            _SEQERROR(_SEQ_READ_REGISTER);

        }

        if(_TEST_BIT(PCB190_FAULT)) _SEQERROR(_DEVREGL(RG190_FAULTS,PCB190_CONTEST));
        printf("RISULTATO RX OK\n");        

        // Se la sequenza finisce correttamente i MAS ovviamente sono quelli previsti
        mAs_erogati = Param->esposizione.MAS;

        // Calcolo dei dati di post esposizione
        data[0]=RXOK;       
        data[1]=(unsigned char) ((mAs_erogati)&0xFF);  // (unsigned char) ((mAs_erogati/50)&0xFF);  // Aggiungere mas residui
        data[2]=(unsigned char) ((mAs_erogati>>8)&0xFF); // (unsigned char) (((mAs_erogati/50)>>8)&0xFF);
        
        // HV di fine raggi in formato RAW          
        data[3]= _DEVREGL(RG190_HV_RXEND,PCB190_CONTEST); 

        // Notifica anticipata per velocizzare il termine della sequenza
        mccGuiNotify(1,MCC_CMD_RAGGI_STD,data,4);

        rxNotifyData(0,RXOK);  

    
    }else{
        
        // Impostazione Output per DEMO
        printf("Impulso raggi Demo .. \n");
        
        _time_delay(2000);        
        
        _mutex_lock(&output_mutex);
        SystemOutputs.CPU_XRAY_ENA=0;   // Nessuna abilitazione ai raggi
        SystemOutputs.CPU_DEMO_ACTIVATION = 1;   // Buzzer Acceso
        _EVSET(_EV0_OUTPUT_CAMBIATI);         
        _mutex_unlock(&output_mutex);      
        
        _time_delay(800);
        
        _mutex_lock(&output_mutex);
        SystemOutputs.CPU_XRAY_ENA=0;   // Nessuna abilitazione ai raggi
        SystemOutputs.CPU_DEMO_ACTIVATION = 0;   // Buzzer Spento
        _EVSET(_EV0_OUTPUT_CAMBIATI);
        _mutex_unlock(&output_mutex);      
        
        _time_delay(1000);

        // Lettura mas residui
        mAs_erogati = Param->esposizione.MAS;
        
        // Calcolo dei dati di post esposizione
        data[0]=RXOK;       
        data[1]= (unsigned char) ((mAs_erogati)&0xFF);// (unsigned char) ((mAs_erogati/50)&0xFF);  // Aggiungere mas residui
        data[2]=(unsigned char) (((mAs_erogati)>>8)&0xFF); // (unsigned char) (((mAs_erogati/50)>>8)&0xFF);
        
        // Calcolo HV a fine raggi con formula lineare regressiva
        data[3]= 0; 

        mccGuiNotify(1,MCC_CMD_RAGGI_STD,data,4);


    }
    
    // Sblocca i drivers dallo stato di freeze
    if(Ser422DriverSetReadyAll(5000) == FALSE) _SEQERROR(_SEQ_DRIVER_READY);
    printf("SBLOCCO DRIVER OK\n");
    
    _mutex_lock(&output_mutex);
    SystemOutputs.CPU_XRAY_ENA=0;   // Disattivazione segnale XRAY ENA
    SystemOutputs.CPU_DEMO_ACTIVATION = 0;   // Disattivazione Buzzer
    _EVSET(_EV0_OUTPUT_CAMBIATI);         
    _mutex_unlock(&output_mutex);
    
    // Sblocca il compressore se deve
    if(Param->compressor_unlock)
        pcb215SetSblocco();
    
    
    //----------------------------------------------------------//
    //                       Fine sequenza
    //----------------------------------------------------------//
    RESULT=TRUE;

    // Stringa di debug
    printf("SEQUENZA RX STANDARD TERMINATA CON SUCCESSO. mAs:%f\n",(float) mAs_erogati/50);    
 
    _EVSET(_SEQEV_RX_STD_TERMINATED);

    // Se richiesto viene spento lo starter
    if(generalConfiguration.pcb190Cfg.starter_off_after_exposure){
        if(generalConfiguration.pcb190Cfg.starter_off_with_brake) pcb190StopStarter();
        else pcb190OffStarter();
    }

    
  } // while
  
}

void _SEQERRORFUNC(int code)
{
    unsigned char data[10];
    
    // In ogni caso ripristina gli IO
    _mutex_lock(&output_mutex);
    SystemOutputs.CPU_XRAY_ENA=0;   // Disattivazione segnale XRAY ENA
    SystemOutputs.CPU_DEMO_ACTIVATION = 0;   // Disattivazione Buzzer
    _EVSET(_EV0_OUTPUT_CAMBIATI);         
    _mutex_unlock(&output_mutex);
    
    // Sblocca il compressore se deve
    if(Param->compressor_unlock)
        pcb215SetSblocco();
    
    // Segnala il risultato
    RESULT=FALSE;

    // Verifica se c'è stata radiazione
    if((code>LAST_ERROR_NO_PREP)&&(code<LAST_ERROR_WITH_PREP))
    {
        // Lettura mas residui
        if(Ser422ReadRegister(_REGID(RG190_MAS_EXIT),10,&PCB190_CONTEST) == _SER422_NO_ERROR)
            mAs_erogati = _DEVREG(RG190_MAS_EXIT,PCB190_CONTEST);        
    }else mAs_erogati = 0;
    
    // Stringa di debug
    printf("STD SEQ ERROR:%d, mAs:%f\n",code, (float) mAs_erogati/50); 
    ERROR = code;
    data[0]=ERROR;       
    data[1]=(unsigned char) ((mAs_erogati)&0xFF);// (unsigned char) ((mAs_erogati/50)&0xFF);  // Aggiungere mas residui
    data[2]=(unsigned char) (((mAs_erogati)>>8)&0xFF);// (unsigned char) (((mAs_erogati/50)>>8)&0xFF);
    data[3]= _DEVREGL(RG190_HV_RXEND,PCB190_CONTEST); 

    mccGuiNotify(1,MCC_CMD_RAGGI_STD,data,4);

    // Carica i dati relativi all'esposizione se necessario
    if(!generalConfiguration.demoMode) rxNotifyData(0,ERROR);
    
    // Sblocca i drivers
    Ser422DriverSetReadyAll(5000);   
    
    _EVSET(_SEQEV_RX_STD_TERMINATED);

    // Se richiesto viene spento lo starter
    if(generalConfiguration.pcb190Cfg.starter_off_after_exposure){
        if(generalConfiguration.pcb190Cfg.starter_off_with_brake) pcb190StopStarter();
        else pcb190OffStarter();
    }

    return;
}


/* EOF */
