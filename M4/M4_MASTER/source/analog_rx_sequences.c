#define _ANALOG_RX_SEQUENCES_C
#include "dbt_m4.h"


#undef PARAM
#undef RESULT
#undef ISRUNNING
#undef ERROR

///////////////////////////////////////////////////////////////////////////////
// Customizzazione dati sequenza
#define _SEQERRORFUNC RxAnalogSeqError
#define PARAM rxStdParam
#define ISRUNNING rxAnalogIsRunning
///////////////////////////////////////////////////////////////////////////////


_RxStdSeq_Str PARAM;
bool ISRUNNING=FALSE;
#define Param (&PARAM)

void analog_rx_task(uint32_t taskRegisters)
{
  int result=0;

  printf("PARTENZA SEQUENZA PER GESTIONE RAGGI MACCHINE ANALOGICHE\n");
  _EVCLR(_SEQEV_RX_ANALOG_START);

  while(1)
  {
    // Attende fino a nuova partenza
    ISRUNNING=FALSE;
    _EVCLR(_SEQEV_RX_ANALOG_START);
    _EVWAIT_ALL(_SEQEV_RX_ANALOG_START);
    _EVCLR(_SEQEV_RX_ANALOG_TERMINATED);
    ISRUNNING=TRUE;

    // Condizioni comuni a tutte le esposizioni
    if(generalConfiguration.demoMode) printf("DEMO MODE\n");
    else  printf("ESPOSIZIONE REALE\n");

    // Prima di andare in freeze bisogna accertarsi che la collimazione 2D sia andata a buon fine
    if(wait2DBackFrontCompletion(50)==false){
        RxAnalogSeqError(ERROR_INVALID_COLLI);
        fineSequenza();
        continue;
    }

    if(wait2DLeftRightTrapCompletion(50)==false){
        RxAnalogSeqError(ERROR_INVALID_COLLI);
        fineSequenza();
        continue;
    }

    // Disabilita tutti i drivers ed attende che tutti i driver siano fermi
    Ser422DriverFreezeAll(0);

    //________________________________________________________________________________________________
    // Prima di procedere bisogna verificare se il filtro ha terminato correttamente il posizionamento
    if(waitRxFilterCompletion()==FALSE) {
        RxAnalogSeqError(ERROR_INVALID_FILTRO);
        fineSequenza();
        continue;
    }

    // Specchio fuori campo se non è già stato  levato (comando compatibile FREEZE)
    if(pcb249U2Lamp(2,100,true) == FALSE){
        RxAnalogSeqError(ERROR_MIRROR_LAMP);
        fineSequenza();
        continue;
    }


    // Selezione della sequenza da eseguire
    switch(Param->analog_sequence){
    case EXPOSIMETER_CALIBRATION_PULSE: result = AnalogPreCalibration();
        break;
    case AEC_MODE_EXPOSURE: result = AnalogAECModeExposure();
        break;
    case TUBE_CALIBRATION_PROFILE: result = AnalogTubeCalibration();
        break;
    case MANUAL_MODE_EXPOSURE: result = AnalogManualModeExposure();
        break;
    }

    if(result!=0) RxAnalogSeqError(result);
    else printf("SEQUENZA COMPLETATA CON SUCCESSO\n");

    fineSequenza();


  } // while
} // Thread

int setXrayEna(void){
        if(SystemInputs.CPU_XRAY_ENA_ACK) return 0;

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
            if(_EVWAIT_TANY(_MOR2(_EV2_XRAY_ENA_ON,_EV2_XRAY_REQ_OFF),_WAIT_XRAY_ENA)==FALSE) return _SEQ_IO_TIMEOUT;
        }
        if(SystemInputs.CPU_XRAY_REQ==0)  return ERROR_PUSHRX_NO_PREP;
        _time_delay(100); // Non levare il delay
        return 0;
}

int clrXrayEna(void){

        if(SystemInputs.CPU_XRAY_ENA_ACK == 0) return 0;

        // Imposta il comando di azzeramento
        _mutex_lock(&output_mutex);
        SystemOutputs.CPU_XRAY_ENA=0;   // Attivazione segnale XRAY ENA
        SystemOutputs.CPU_DEMO_ACTIVATION = 0;   // Attivazione Buzzer
        _EVSET(_EV0_OUTPUT_CAMBIATI);
        _mutex_unlock(&output_mutex);

        // Attende il feedback
        if(SystemInputs.CPU_XRAY_ENA_ACK)
        {
            _EVCLR(_EV2_XRAY_ENA_OFF);
            if(_EVWAIT_TANY(_EV2_XRAY_ENA_OFF,_WAIT_XRAY_ENA)==FALSE) return -1;
        }

        return 0;
}

// Sequenza di calibrazione pre impulso esposimetro e calibrazione
// rilevamento PLOG
int AnalogTubeCalibration(void){

    int error=0;

    printf("ESECUZIONE PROCEDURA DI CALIBRAZIONE TUBO\n");

    // Reset Eventuale Fault della PCB190
    pcb190ResetFault();

    if(pcb190StarterH()==FALSE) printf("WARNING: COMANDO STARTER HIGH FALLITO\n");
    else printf("STARTER ATTIVATO AD ALTA VELOCITA'\n");

    // Caricamento parametri di esposizione
    if(pcb190UploadAnalogCalibTubeExpose(Param)==FALSE) return _SEQ_UPLOAD190_PARAM;

    printf("DATI IMPULSO --------------------------\n");
    printf("IDAC:%d\n",Param->esposizione.I & 0x0FFF);
    printf("VDAC:%d\n",Param->esposizione.HV & 0x0FFF);
    printf("MASDAC:%d\n",Param->esposizione.MAS);
    printf("--------------------------------------\n");


    // Verifica su XRAY_REQ(Pulsante raggi premuto)
    if(SystemInputs.CPU_XRAY_REQ==0)  return ERROR_PUSHRX_NO_PREP;

    // Impostazione Segnale XRAY_ENA su Bus Hardware
    error = setXrayEna();
    if(error) return error;

    // Legge il Busy per attendere che sia tutto pronto
    if(waitPcb190Ready(50)==FALSE) return _SEQ_PCB190_BUSY;


    // Parte l'esposimetro per la modalità manuale (libera il segnale di XRAY-DISABLE )
    if(pcb244_A_StartCalibTube()==false) return _SEQ_PCB190_BUSY;

    // Parte la PCB190
    int rc = pcb190StartRxStd();
    if(rc==SER422_BUSY) return _SEQ_PCB190_BUSY;
    if(rc==SER422_ILLEGAL_FUNCTION) return ERROR_PUSHRX_NO_PREP;

    // Un minimo di attesa per consentire ai vari segnali di sincronizzarsi
    _time_delay(1000);

    printf("Attesa Completamento \n");

    // Attesa XRAY COMPLETED da Bus Hardware
    if(SystemInputs.CPU_XRAY_COMPLETED==0)
    {
      _EVCLR(_EV2_XRAY_COMPLETED);
      if(_EVWAIT_TALL(_EV2_XRAY_COMPLETED,_WAIT_XRAY_COMPLETED)==FALSE) return _SEQ_PCB190_TMO;
    }
    printf("Completato\n");

    // Per sicurezza attiva il bit di stop sull'esposimetro
    PCB244_A_SetRxStop();

    // Lettura esito raggi
    if(pcb190GetPostRxRegisters()==FALSE){
        printf("ERRORE DURANTE LETTURA REGISTRI FINE RAGGI!!!!!!! \n");
        return _SEQ_READ_REGISTER;
    }
    Param->dmAs_released = _DEVREG(RG190_MAS_EXIT,PCB190_CONTEST)*10/50;
    Param->pulses_released = 0; // Non applicabile in questa sequenza di esposizione

    // Analisi della condizione di FAULT
    // ATTENZIONE BISOGNA ESCLUDERE IL TIMEOUT TRA LE CAUSE DI ERRORE
    if(_TEST_BIT(PCB190_FAULT)){
        if(_DEVREGL(RG190_FAULTS,PCB190_CONTEST) != ERROR_TMO_RX)   return(_DEVREGL(RG190_FAULTS,PCB190_CONTEST));
    }

    // Rileva i dati campionati per la risposta che avverrà con il codice MCC_RAGGI_DATA
    float KV;
    int TIME;
    unsigned char KVRAW;
    getRxSamplesData(&KV,&KVRAW,0,&TIME);

    // Stampa dati
    printf("mAs=%f\n", ((float)Param->dmAs_released)/10);
    printf("kV=%f KVRAW=%d \n", KV, KVRAW);
    printf("TIME(ms)=%d\n", TIME);
    printf("IMED(mA)=%d\n", Param->dmAs_released * 100 / TIME);

    // Invio risultato al Master
    unsigned char  data[8];
    data[0]=RXOK;
    data[1]=(unsigned char) ((Param->dmAs_released)&0xFF);
    data[2]=(unsigned char) ((Param->dmAs_released>>8)&0xFF);
    data[3]=(unsigned char) KVRAW; // KVRAW

    int val;
    val = (int) (KV * (float) 10); // KV * 10
    data[4]=(unsigned char) ((val)&0xFF);
    data[5]=(unsigned char) (((val)>>8)&0xFF);

    data[6]=(unsigned char) ((int)(TIME)&0xFF); // TIME
    data[7]=(unsigned char) (((int)(TIME)>>8)&0xFF);

    mccGuiNotify(1,Param->mcc_code,data,sizeof(data));

    printf("FINE SEQUENZA OK\n");
    return 0; // RISULTATO POSITIVO

} // AnalogPreCalibration



// Sequenza di calibrazione pre impulso esposimetro e calibrazione
// rilevamento PLOG
// In questa procedura si valuta il RAD sulla base di un impulso manuale di 50 mAs
// e sulla base degli impulsi ottenuti.
int AnalogPreCalibration(void){

    int error=0;

    printf("ESECUZIONE PROCEDURA DI CALIBRAZIONE DETECTOR\n");

    // Procede con l'azzeramento dell'offset
    PCB244_A_zeroOffset();

    // Reset Eventuale Fault della PCB190
    pcb190ResetFault();
    Param->dmAs_released = 0;
    Param->pulses_released = 0;

    // Attiva Starter precocemente
    if(Param->esposizione.HV & 0x4000)
    {
        if(pcb190StarterH()==FALSE) printf("WARNING: COMANDO STARTER HIGH FALLITO\n");
        else printf("STARTER ATTIVATO AD ALTA VELOCITA'\n");
    }else
    {
        if(pcb190StarterL()==FALSE) printf("WARNING: COMANDO STARTER LOW FALLITO\n");
        else printf("STARTER ATTIVATO A BASSA VELOCITA'\n");
    }

    // Caricamento parametri di esposizione
    if(pcb190UploadAnalogManualExpose(Param)==FALSE) return _SEQ_UPLOAD190_PARAM;

    // Caricamento impulsi esposimetro
    if(pcb244_A_uploadManualPulses(0xFFFF)==FALSE) return  _SEQ_UPLOAD_PCB244_A_PARAM ;


    printf("DATI IMPULSO CALIBRAZIONE RAD --------\n");
    printf("IDAC:%d\n",Param->esposizione.I & 0x0FFF);
    printf("VDAC:%d\n",Param->esposizione.HV & 0x0FFF);
    printf("MASDAC:%d\n",Param->esposizione.MAS);
    printf("--------------------------------------\n");

    // Verifica su XRAY_REQ(Pulsante raggi premuto)
    if(SystemInputs.CPU_XRAY_REQ==0)  return ERROR_PUSHRX_NO_PREP;

    // Impostazione Segnale XRAY_ENA su Bus Hardware
    error = setXrayEna();
    if(error) return error;

    // Legge il Busy per attendere che sia tutto pronto
    if(waitPcb190Ready(50)==FALSE) return _SEQ_PCB190_BUSY;

    // Parte l'esposimetro in modalità impulso singolo manuale
    // Bisogna prima caricare il valore degli impulsi da contare
    if(pcb244_A_StartManual(true)==false) return _SEQ_PCB244_A_BUSY;

    // Parte la PCB190 in modalità manuale
    int rc = pcb190StartRxStd();
    if(rc==SER422_BUSY) return _SEQ_PCB190_BUSY;
    if(rc==SER422_ILLEGAL_FUNCTION) return ERROR_PUSHRX_NO_PREP;

    // Un minimo di attesa per consentire ai vari segnali di sincronizzarsi
    _time_delay(1000);

    printf("Attesa Completamento \n");

    // Attesa XRAY COMPLETED da Bus Hardware
    if(SystemInputs.CPU_XRAY_COMPLETED==0)
    {
      _EVCLR(_EV2_XRAY_COMPLETED);
      if(_EVWAIT_TALL(_EV2_XRAY_COMPLETED,_WAIT_XRAY_COMPLETED)==FALSE) return _SEQ_PCB190_TMO;
    }
    printf("Completato\n");


    // Attesa ripresa comunicazione seriale post esecuzione raggi
    while(!PCB244_A_GetRad1(1)) _time_delay(100);
    // Lettura Offset inizio raggi
    if(Ser422ReadRegister(_REGID(RG244_A_PRE_OFFSET),10,&PCB244_A_CONTEST)!=_SER422_NO_ERROR) return false;

    // ATTENZIONE IL VALORE RAD E' LA SOMMA DI QUATTRO VALORI !!!
    // Il valore di riferimento progettuale fa corrispondere 2.2V all'ingresso del frequenzimetro
    // con 7cm di plexiglass senza cassetta (taratura detector)
    // In quelle condizioni il RAD deve essere convertito a 155
    // 2.2V -> 225*4 = 900. KRAD = 900/155 = 5.8
    int radraw =    _DEVREG(RG244_A_RAD1,PCB244_A_CONTEST)/4;
    int prerad =    _DEVREG(RG244_A_PRE_OFFSET,PCB244_A_CONTEST)/4;
    int plog = getPlog(radraw);

    // Lettura esito raggi
    if(pcb190GetPostRxRegisters()==FALSE){
        printf("ERRORE DURANTE LETTURA REGISTRI FINE RAGGI PCB190!!!!!!! \n");
        return _SEQ_READ_REGISTER;
    }

    Param->dmAs_released = _DEVREG(RG190_MAS_EXIT,PCB190_CONTEST)*10/50;

    if(pcb244_A_GetPostRxRegisters()==FALSE){
        printf("ERRORE DURANTE LETTURA REGISTRI FINE RAGGI ESPOSIMETRO!!!!!!! \n");
        return _SEQ_READ_REGISTER;
    }
    Param->pulses_released = _DEVREG(RG244_A_PULSES_EXIT,PCB244_A_CONTEST);

    // Analisi della condizione di FAULT
    if(_TEST_BIT(PCB190_FAULT)) return(_DEVREGL(RG190_FAULTS,PCB190_CONTEST));

    // Tempo medio impulso in mS
    //float time_pulse = (unsigned short)((float) _DEVREG(REG190_RX_TIME_PLS,PCB190_CONTEST) * 1.115);
    float time_pulse = Param->mAs_nom*1000 / Param->In;
    float meanRad = ((float) Param->pulses_released * 1024)/(10*time_pulse);
    unsigned int  rad = (unsigned int) meanRad;
    if((meanRad-(float) rad) >0.5) rad++;
    printf("mAs = %f, Pulse = %d, Time=%d, PLOG=%d, RAD=%d, RADRAW=%d, PRERAD=%d\n", ((float)Param->dmAs_released)/10, Param->pulses_released, (unsigned int) time_pulse, plog, rad,radraw,prerad);


    unsigned char  data[11];
    data[0]=RXOK;
    data[1]=(unsigned char) ((Param->dmAs_released)&0xFF);
    data[2]=(unsigned char) ((Param->dmAs_released>>8)&0xFF);
    data[3]=(unsigned char) ((plog)&0xFF);
    data[4]=(unsigned char) ((plog>>8)&0xFF);
    data[5]=(unsigned char) ((rad)&0xFF);
    data[6]=(unsigned char) ((rad>>8)&0xFF);

    data[7]= 0; //(unsigned char) ((rad5)&0xFF);
    data[8]= 0; //(unsigned char) ((rad5>>8)&0xFF);

    data[9]=(unsigned char) ((prerad)&0xFF);
    data[10]=(unsigned char) ((prerad>>8)&0xFF);

    mccGuiNotify(1,Param->mcc_code,data,sizeof(data));
    printf("FINE SEQUENZA OK\n");
    return 0; // RISULTATO POSITIVO

} // AnalogPreCalibration



// Sequenza di esposizione con esposimetro (sia operativa che di calibrazione)
int AnalogAECModeExposure(void){
    unsigned char  data[20];
    int error=0;

    // Procede con l'azzeramento dell'offset
    PCB244_A_zeroOffset();

    // Reset Eventuale Fault della PCB190
    pcb190ResetFault();
    Param->dmAs_pre_released = 0;

    // Attiva Starter precocemente
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

    // Caricamento parametri di esposizione
    if(pcb190UploadAnalogPreExpose(Param)==FALSE) return _SEQ_UPLOAD190_PARAM;

    // Verifica su XRAY_REQ(Pulsante raggi premuto)
    if(SystemInputs.CPU_XRAY_REQ==0)  return ERROR_PUSHRX_NO_PREP;

    // Impostazione Segnale XRAY_ENA su Bus Hardware
    error = setXrayEna();
    if(error) return error;

    // Legge il Busy per attendere che sia tutto pronto
    if(waitPcb190Ready(50)==FALSE) return _SEQ_PCB190_BUSY;

    // Parte l'esposimetro
    if(pcb244_A_StartRxAec()==false) return _SEQ_PCB190_BUSY;

    // Parte la PCB190 in modalità AEC
    int rc = pcb190StartRxAecStd();
    if(rc==SER422_BUSY) return _SEQ_PCB190_BUSY;
    if(rc==SER422_ILLEGAL_FUNCTION) return ERROR_PUSHRX_NO_PREP;

    // Un minimo di attesa per consentire ai vari segnali di sincronizzarsi
    _time_delay(1000);

    printf("ATTESA DATI ESPOSIMETRO..\n");
    int attempt=20; // Attesa di circa 8 secondi
    while(--attempt){
        if(PCB244_A_GetPreRad(10)==true) break; // 40ms * 10 = 400ms ogni blocco di tentativi
        if(SystemInputs.CPU_XRAY_REQ==0)  return ERROR_PUSHRX_NO_PREP;
    }
    if(!attempt) return  _SEQ_WAIT_AEC_DATA;

    int rad1 = _DEVREG(RG244_A_RAD1,PCB244_A_CONTEST) / 4;
    int rad5 = _DEVREG(RG244_A_RAD5,PCB244_A_CONTEST) / 4;
    int rad25 = _DEVREG(RG244_A_RAD25,PCB244_A_CONTEST) ;
    int prerad =  _DEVREG(RG244_A_PRE_OFFSET,PCB244_A_CONTEST);
    printf("OFFSET PRE RX: %d - %f\n", prerad, (float) prerad/4.0);



    int rad = rad1;
    int plog = getPlog(rad1);

    // Legge i mAs del pre impulso dalla PCB190
    Param->dmAs_pre_released =  ((10 * (float) pcb190GetPremAsData()) / 50);
    printf("mAs PRE IMPULSO = %f\n",(float)Param->dmAs_pre_released / 10.0);

        //__________________________________________________________   >>>>>>>>     FINE SEQEUNZA AEC
    printf("DATI ESPOSIMETRO: PLOG=%d, RAD=%d\n",plog,rad);

    // Se il RAD è maggiore di 1022 la sequenza raggi viene interrotta
    // per AEC sovra esposto
    if(rad>1022) return ESPOSIMETRO_AEC_SOVRAESPOSTO;

    // Richiede i dati per l'esposizione
    _EVCLR(_EV2_WAIT_AEC);
    data[0]=(unsigned char) ((plog)&0xFF);
    data[1]=(unsigned char) ((plog>>8)&0xFF);
    data[2]=(unsigned char) ((rad)&0xFF);
    data[3]=(unsigned char) ((rad>>8)&0xFF);
    data[4]=(unsigned char) ((rad5)&0xFF);
    data[5]=(unsigned char) ((rad5>>8)&0xFF);
    data[6]=(unsigned char) ((rad25)&0xFF);
    data[7]=(unsigned char) ((rad25>>8)&0xFF);
    mccGuiNotify(1,MCC_XRAY_ANALOG_REQ_AEC_PULSE,data,8);

    // Attesa dati da interfaccia
    printf("ATTESA DATI AEC..\n");
    attempt=40;
    while(--attempt){

        // Rilascio pulsante raggi
       if(SystemInputs.CPU_XRAY_REQ==0) return ERROR_PUSHRX_AFTER_PREP;

        // PCB190 in errore!
        if(SystemInputs.CPU_XRAY_COMPLETED){
            pcb190GetPostRxRegisters();
            printf("ERRORE SEQUENZA RAGGI DURANTE ATTESA AEC\n");
            return _DEVREGL(RG190_FAULTS,PCB190_CONTEST);
        }

        // Dati AEC arrivati
        if(_IS_EVENT(_EV2_WAIT_AEC)) break;
        _time_delay(100);
    }
    if(!attempt) return _SEQ_AEC_NOT_AVAILABLE;

    // Errore comunicato dalla GUI
    if(Param->guiError) return Param->guiError;

    //________________________________________________________________________________________________
    // Prima di procedere bisogna verificare se il filtro ha terminato correttamente il posizionamento
    // nel caso di cambio filtro
    if(waitRxFilterCompletion()==FALSE) {
        return ERROR_INVALID_FILTRO;
    }

    // Caricamento dati AEC su PCB190
    if(!pcb190UploadExpose(Param, TRUE)) return _SEQ_UPLOAD190_PARAM;

    // Caricamento impulsi esposimetro
    if(!pcb244_A_uploadAECPulses(Param->pulses)) return _SEQ_UPLOAD190_PARAM;

    printf("ATTESA COMPLETAMENTO..\n");
    bool EsitoConteggioEsposimetro = false;

    // Attende che l'esposimetro termini l'esecuzione dei raggi
    // !!!! Attenzione, per velocizzare il processo, l'esposimetro
    // attiva la seriale non appena termina il conteggio. Tuttavia la sequenza potrebbe
    // continuare fino al riposizionamento della griglia in Home.
    // La sequenza si riterrà completa solo quando il flag Busy sarà azzerato
    int i=100;
    while(i){
        i--;
        if(PCB244_A_waitRxCompletedFlag()) break;
        if(SystemInputs.CPU_XRAY_REQ==0) return ERROR_PUSHRX_AFTER_PREP;
         _time_delay(100);
    }
    if(i==0) printf("ERRORE ATTESA ESPOSIMETRO!!!");


    // Comanda fine sequenza a PCB190
    if(!pcb190AnalogRxStop()) printf("ERRORE PCB190 STOP !!!");

    // Attesa XRAY COMPLETED da Bus Hardware
    if(SystemInputs.CPU_XRAY_COMPLETED==0)
    {
      _EVCLR(_EV2_XRAY_COMPLETED);
      if(_EVWAIT_TALL(_EV2_XRAY_COMPLETED,_WAIT_XRAY_COMPLETED)==FALSE) return _SEQ_PCB190_TMO;
    }

    // Lettura esito raggi
    if(pcb190GetPostRxRegisters()==FALSE){
        printf("ERRORE LETTURA PCB190 FINE RAGGI\n");
        return _SEQ_READ_REGISTER;
    }

    // La lettura dei dati dell'esposimetro può essere fatta solo dopo l'azzeramento
    // del bit di BUSY. L'attesa viene consumata all'interno della funzione
    if(pcb244_A_GetPostRxRegisters()==FALSE){
        printf("ERRORE LETTURA PCB244A FINE RAGGI\n");
        return _SEQ_READ_REGISTER;
    }

    EsitoConteggioEsposimetro = PCB244_A_readRxStat(); // Legge la modalità di fine conteggio

    // Messaggio fine sequenza raggi ________________________________________
    Param->dmAs_released = _DEVREG(RG190_MAS_EXIT,PCB190_CONTEST)*10/50;
    if(EsitoConteggioEsposimetro) Param->pulses_released = Param->pulses;
    else Param->pulses_released = _DEVREG(RG244_A_PULSES_EXIT,PCB244_A_CONTEST);

    // Analisi della condizione di FAULT
    if(_TEST_BIT(PCB190_FAULT)) return(_DEVREGL(RG190_FAULTS,PCB190_CONTEST));


    printf("SEQUENZA TEMINATA CON SUCCESSO\n");
    data[0]=RXOK;
    data[1]=(unsigned char) ((Param->dmAs_released)&0xFF);
    data[2]=(unsigned char) ((Param->dmAs_released>>8)&0xFF);    
    data[3]=(unsigned char) ((Param->pulses_released)&0xFF);
    data[4]=(unsigned char) ((Param->pulses_released>>8)&0xFF);
    data[5]=(unsigned char) ((rxStdParam.dmAs_pre_released)&0xFF);

    mccGuiNotify(1,Param->mcc_code,data,6);

    printf("PLOG=%d, RAD=%d, mAs=%f Pulses:%d\n", plog,rad,(float) Param->dmAs_released/10, Param->pulses_released);

    printf("FINE SEQUENZA OK\n");
    return 0; // RISULTATO POSITIVO

} // AnalogProfileCalibration



// Sequenza di esposizione manuale
int AnalogManualModeExposure(void){
    int error=0;

    // Procede con l'azzeramento dell'offset
    PCB244_A_zeroOffset();

    // Reset Eventuale Fault della PCB190
    pcb190ResetFault();
    Param->dmAs_released = 0;
    Param->pulses_released = 0;

    // Attiva Starter precocemente
    if(Param->esposizione.HV & 0x4000)
    {
      if(pcb190StarterH()==FALSE) printf("WARNING: COMANDO STARTER HIGH FALLITO\n");
      else printf("STARTER ATTIVATO AD ALTA VELOCITA'\n");
    }else
    {
      if(pcb190StarterL()==FALSE) printf("WARNING: COMANDO STARTER LOW FALLITO\n");
      else printf("STARTER ATTIVATO A BASSA VELOCITA'\n");
    }

    // Caricamento parametri di esposizione
    if(pcb190UploadAnalogManualExpose(Param)==FALSE) return _SEQ_UPLOAD190_PARAM;

    // Caricamento impulsi esposimetro
    if(pcb244_A_uploadManualPulses(0xFFFF)==FALSE) return  _SEQ_UPLOAD_PCB244_A_PARAM ;

    // Verifica su XRAY_REQ(Pulsante raggi premuto)
    if(SystemInputs.CPU_XRAY_REQ==0)  return ERROR_PUSHRX_NO_PREP;

    // Impostazione Segnale XRAY_ENA su Bus Hardware
    error = setXrayEna();
    if(error) return error;

    // Legge il Busy per attendere che sia tutto pronto
    if(waitPcb190Ready(50)==FALSE) return _SEQ_PCB190_BUSY;

    // Parte l'esposimetro in modalità impulso singolo manuale
    // Bisogna prima caricare il valore degli impulsi da contare
    if(pcb244_A_StartManual(false)==false) return _SEQ_PCB244_A_BUSY;

    // Parte la PCB190 in modalità manuale
    int rc = pcb190StartRxStd();
    if(rc==SER422_BUSY) return _SEQ_PCB190_BUSY;
    if(rc==SER422_ILLEGAL_FUNCTION) return ERROR_PUSHRX_NO_PREP;

    // Un minimo di attesa per consentire ai vari segnali di sincronizzarsi
    _time_delay(1000);

    printf("Attesa Completamento \n");

    // Attesa XRAY COMPLETED da Bus Hardware
    if(SystemInputs.CPU_XRAY_COMPLETED==0)
    {
      _EVCLR(_EV2_XRAY_COMPLETED);
      if(_EVWAIT_TALL(_EV2_XRAY_COMPLETED,_WAIT_XRAY_COMPLETED)==FALSE) return _SEQ_PCB190_TMO;
    }
    printf("Completato\n");

    printf("Lettura campionamenti \n");

    // Attesa ripresa comunicazione seriale post esecuzione raggi
    while(!PCB244_A_GetRad1(1)) _time_delay(100);

    // Lettura campionamenti   
    PCB244_A_GetPreRad(10);
    int radraw =  _DEVREG(RG244_A_RAD1,PCB244_A_CONTEST)/4;
    int prerad =  _DEVREG(RG244_A_PRE_OFFSET,PCB244_A_CONTEST)/4;
    int plog = getPlog(radraw);


    // Lettura esito raggi
    if(pcb190GetPostRxRegisters()==FALSE){
        printf("ERRORE DURANTE LETTURA REGISTRI FINE RAGGI PCB190!!!!!!! \n");
        return _SEQ_READ_REGISTER;
    }
    Param->dmAs_released = _DEVREG(RG190_MAS_EXIT,PCB190_CONTEST)*10/50;

    if(pcb244_A_GetPostRxRegisters()==FALSE){
        printf("ERRORE DURANTE LETTURA REGISTRI FINE RAGGI ESPOSIMETRO!!!!!!! \n");
        return _SEQ_READ_REGISTER;
    }
    Param->pulses_released = _DEVREG(RG244_A_PULSES_EXIT,PCB244_A_CONTEST);

    // Analisi della condizione di FAULT
    if(_TEST_BIT(PCB190_FAULT)) return(_DEVREGL(RG190_FAULTS,PCB190_CONTEST));

    // Tempo medio impulso in mS
    //float time_pulse = (unsigned short)((float) _DEVREG(REG190_RX_TIME_PLS,PCB190_CONTEST) * 1.115);
    float time_pulse = Param->mAs_nom*1000 / Param->In;
    float meanRad = ((float) Param->pulses_released * 1024)/(10*time_pulse);
    unsigned int  rad = (unsigned int) meanRad;
    if((meanRad-(float) rad) >0.5) rad++;
    printf("mAs = %f, Pulse = %d, Time=%d, PLOG=%d, RAD=%d, RADRAW=%d, PRERAD=%d\n", ((float)Param->dmAs_released)/10, Param->pulses_released, (unsigned int) time_pulse, plog, rad,radraw,prerad);

    float KV,IMED;
    int TIME;
    getRxSamplesData(&KV,0,&IMED,&TIME);


    unsigned char  data[19];
    data[0]=RXOK;
    data[1]=(unsigned char) ((Param->dmAs_released)&0xFF);
    data[2]=(unsigned char) ((Param->dmAs_released>>8)&0xFF);
    data[3]=(unsigned char) ((Param->pulses_released)&0xFF);
    data[4]=(unsigned char) ((Param->pulses_released>>8)&0xFF);
    data[5]=(unsigned char) ((plog)&0xFF);
    data[6]=(unsigned char) ((plog>>8)&0xFF);
    data[7]=(unsigned char) ((rad)&0xFF);
    data[8]=(unsigned char) ((rad>>8)&0xFF);
    data[9]=0;  // Non usato
    data[10]=0; // Non usato
    data[11]=(unsigned char) ((prerad)&0xFF);
    data[12]=(unsigned char) ((prerad>>8)&0xFF);

    int val;
    val = (int) (KV * (float) 10);
    data[13]=(unsigned char) ((val)&0xFF);
    data[14]=(unsigned char) (((val)>>8)&0xFF);
    val = (int) (IMED * (float) 10);
    data[15]=(unsigned char) ((val)&0xFF);
    data[16]=(unsigned char) (((val)>>8)&0xFF);
    data[17]=(unsigned char) ((int)(TIME)&0xFF);
    data[18]=(unsigned char) (((int)(TIME)>>8)&0xFF);

    mccGuiNotify(1,Param->mcc_code,data,sizeof(data));
    printf("FINE SEQUENZA OK\n");
    return 0; // RISULTATO POSITIVO

}


//____________________________________________________________________________________________________________________________
// FUNZIONE DI GESTIONE PARTICOLARE DELL'ERRORE
void RxAnalogSeqError(int codice){
    unsigned char data[10];

    if(clrXrayEna()<0) printf("ERRORE CLEAR XRAY-ENA");

    // Per sicurezza attiva il bit di stop sull'esposimetro
    PCB244_A_SetRxStop();

    // Stringa di debug
    printf("ERRORE SEQUENZA [%d] MCC[%d]: ERRORE=%d, mAs:%f, PULSES:%d\n",Param->analog_sequence, Param->mcc_code, codice,  (float)Param->dmAs_released / 10,Param->pulses_released);
    data[0]=codice;
    data[1]=(unsigned char) ((Param->dmAs_released)&0xFF);
    data[2]=(unsigned char) (((Param->dmAs_released)>>8)&0xFF);
    data[3]=(unsigned char) ((Param->pulses_released)&0xFF);
    data[4]=(unsigned char) (((Param->pulses_released)>>8)&0xFF);
    data[5]=(unsigned char) ((rxStdParam.dmAs_pre_released)&0xFF);

    mccGuiNotify(1,Param->mcc_code,data,6);

    return;
}

// Operazioni comuni di terminazione della seqeunza
void fineSequenza(void){

    // Sequenza terminata con successo
    if(Ser422DriverSetReadyAll(5000) == FALSE) RxAnalogSeqError(_SEQ_DRIVER_READY);
    else  printf("SBLOCCO DRIVER OK\n");

    // Reset degli IO
    _mutex_lock(&output_mutex);
    SystemOutputs.CPU_XRAY_ENA=0;   // Disattivazione segnale XRAY ENA
    SystemOutputs.CPU_DEMO_ACTIVATION = 0;   // Disattivazione Buzzer
    _EVSET(_EV0_OUTPUT_CAMBIATI);
    _mutex_unlock(&output_mutex);

    // Sblocca il compressore se deve
    if(Param->compressor_unlock)
        pcb215SetSblocco();

    _EVSET(_SEQEV_RX_ANALOG_TERMINATED);

    // Se richiesto viene spento lo starter
    if(generalConfiguration.pcb190Cfg.starter_off_after_exposure){
        if(generalConfiguration.pcb190Cfg.starter_off_with_brake) pcb190StopStarter();
        else pcb190OffStarter();
    }


}


// Campionamenti interessanti ai fini della diagnostica
void getRxSamplesData(float* kv, unsigned char* kvraw, float* imed, int* time)
{
    unsigned char  is[PCB190_NSAMPLES+1];
    unsigned char  vs[PCB190_NSAMPLES+1];
    int i;
    float scarto_v = 0;
    float imean,vmean;
    int  samples, naec;
    unsigned char addr, banco;
    unsigned short tmed_pls;
    unsigned char ifil_rxend;

    // Legge il numero di campioni AEC
    Ser422ReadRegister(_REGID(PR190_N_SAMPLE_AEC),10,&PCB190_CONTEST);
    naec = (float) _DEVREGL(PR190_N_SAMPLE_AEC,PCB190_CONTEST);
    if(naec>1) naec = 1;

    // Legge il numero dei campioni totali
    Ser422ReadRegister(_REGID(PR190_N_SAMPLE_I),10,&PCB190_CONTEST);
    samples = (int) _DEVREGL(PR190_N_SAMPLE_I,PCB190_CONTEST);
    if(samples > PCB190_NSAMPLES) samples = (int) PCB190_NSAMPLES;

    // Legge i campioni della corrente
    addr = _DEVADDR(PR190_SAMPLES_I);
    banco = _DEVBANCO(PR190_SAMPLES_I);
    for(i=0;i<samples;i++,addr++){
        if(i>=sizeof(is)) break;
        Ser422ReadAddrRegister(addr, banco,10,&is[i], &PCB190_CONTEST);
    }

    // Legge i campioni della tensione
    addr = _DEVADDR(PR190_SAMPLES_V);
    banco = _DEVBANCO(PR190_SAMPLES_V);
    for(i=0;i<samples;i++,addr++){
        if(i>=sizeof(vs)) break;
        Ser422ReadAddrRegister(addr, banco,10,&vs[i], &PCB190_CONTEST);
    }

    // Calcola medie Impulso se ci sono campioni
    imean=vmean=0;
    float kvmean=0;
    if((samples-naec)>0){
        for(i=naec;i<samples;i++){
            printf("(PLS-%d): I[%f(mA), %d(RAW)]  V[%f(kV), %d(RAW)] \n",(int) (i-naec),((((float) is[i])*200.0)/255.0), is[i],pcb190ConvertKvRead(vs[i]),vs[i]);
            imean+=(float) is[i];
            vmean+=(float) vs[i];
        }
        imean=imean/(float)(samples-naec);
        vmean=vmean/(float)(samples-naec);
        scarto_v = 0;
        kvmean = pcb190ConvertKvRead(nearest(vmean));

        // Calcola scarto medio Impulso se ci sono campioni
        for(i=naec;i<samples;i++){
            scarto_v +=  ( ((kvmean - pcb190ConvertKvRead(vs[i])))* ((kvmean - pcb190ConvertKvRead(vs[i]))));
        }
        scarto_v = (10.0 * sqrt(scarto_v/(float)(samples-naec)));
    }

    // Legge il valore medio del tempo di impulso
    tmed_pls = 0;
    if(Ser422ReadRegister(_REGID(REG190_RX_TIME_PLS),10,&PCB190_CONTEST) == _SER422_NO_ERROR)
      tmed_pls = (unsigned short)((float) _DEVREG(REG190_RX_TIME_PLS,PCB190_CONTEST) * 1.115);


    // Lettura I FILAMENTO DURANTE RAGGI
    Ser422ReadRegister(_REGID(RG_SAMPLED_IFIL),10,&PCB190_CONTEST);
    ifil_rxend = _DEVREGL(RG_SAMPLED_IFIL,PCB190_CONTEST);

    // Stampa dei valori
    printf("HV-BUS(V):%f\n",(float) _DEVREGL(RG190_HV_RXEND,PCB190_CONTEST) * ((float) generalConfiguration.pcb190Cfg.HV_CONVERTION / 1000.0));
    printf("I_FIL (mA):%f\n",ifil_rxend * 47.98);

    if(samples-naec){
      printf("PLS-I(mA)=%f\n",(imean*200.)/255.);
      printf("PLS-V(kV)=%f, dKv:%f\n",kvmean,scarto_v/10.);
      printf("Tmed_Pulse(ms)=%d\n",tmed_pls );
   }

    // Risultati se richiesti
    if(kv)    *kv = kvmean;
    if(kvraw) *kvraw = vmean;
    if(imed)  *imed = imean * 200 / 255;
    if(time)  *time =  tmed_pls;

}
