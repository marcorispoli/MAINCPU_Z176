#define _PCB249U2_C
#include "dbt_m4.h" 

#define TARGET_ADDRESS 0x15
#define _DEF_PCB249U2_DRIVER_DELAY 500

#define CONTEST PCB249U2_CONTEST
#define STATUS  (*((_PCB249U2_Stat_Str*)(&PCB249U2_CONTEST.Stat)))
#define ERROR_HANDLER   pcb249U2DriverCommunicationError

// Funzione di servizio interne al modulo
static bool GetFwRevision(void);
static void ERROR_HANDLER(void);
static bool pcb249U2ResetFaults(void);
static bool pcb249U2MirrorOutCmd(void);
static bool pcb249U2MirrorHomeCmd(void);
static bool pcb249WaitBusy(int timeout);
static int  pcb249U2getFilterCurrentPosition(void);
static bool pcb249U2SetFiltroCmd(unsigned char cmd);
bool pcb249U2LampCmd(unsigned char cmd, unsigned char tmo);
static unsigned char back, front;
static unsigned char comando_filtro;
static unsigned char target_filtro;
static unsigned char posizioneFiltro;   // Posizione corrente
void pcb249U2_driver(uint32_t taskRegisters)
{
  int i;
  _Ser422_Command_Str frame;
  _SER422_Error_Enum err_ret;
  bool write_ok;   
  

    // Costruzione del contesto
   CONTEST.pReg = PCB249U2_Registers;
   CONTEST.nregisters = PCB249U2_NREGISTERS;
   CONTEST.evm = _EVM(_EV0_PCB249U2_CFG_UPD);
   CONTEST.evr = &_EVR(_EV0_PCB249U2_CFG_UPD);
   CONTEST.address = TARGET_ADDRESS;
   printf("ATTIVAZIONE DRIVER PCB249U2: \n");
    
   //////////////////////////////////////////////////////////////////////////
   //                   FINE FASE DI INIZIALIZZAZIONE DRIVER               //             
   //        Inizia il ciclo di controllo e gestione della periferica      //
   //////////////////////////////////////////////////////////////////////////

    // In caso di errore di compilazione in questo punto 
    // significa errore in compilazione della struttura registri
    SIZE_CHECK((sizeof(PCB249U2_Registers)/sizeof(_DeviceRegItem_Str))!=PCB249U2_NREGISTERS);
      
    // Segnalazione driver disconnesso
    _EVCLR(_EV1_PCB249U2_CONNECTED);
    
    // Retrive Task ID
    CONTEST.ID =  _task_get_id();
    
    // Init registro di stato
    memset((void*)&(STATUS), 0,sizeof(_Device_Stat_Str ));

   // Inizializzazione delle mutex
    if (_mutex_init(&(CONTEST.reglist_mutex), NULL) != MQX_OK)
    {
      printf("PCB249U2: ERRORE INIT MUTEX. FINE PROGRAMMA");
      _mqx_exit(-1);
    }

    if (_mutex_init(&(CONTEST.pollinglist_mutex), NULL) != MQX_OK)
    {
      printf("PCB249U2: ERRORE INIT MUTEX. FINE PROGRAMMA");
      _mqx_exit(-1);
    }
      
    // Reitera fino ad ottenere il risultato
    while(GetFwRevision()==FALSE) _time_delay(100);
    printf("PCB249U2:REVISIONE FW TARGET:%d.%d\n",STATUS.maj_code,STATUS.min_code); 
    
    // Segnalazione driver connesso
   _EVSET(_EV1_PCB249U2_CONNECTED);
    
    // Attende l'autorizzazione ad effetuare il refresh registri
    _EVWAIT_ANY(_EV1_UPDATE_REGISTERS);

    // Upload contenuto registri 
   for(i=0;i<PCB249U2_NREGISTERS;i++)
   {
      err_ret = Ser422ReadRegister(i,4,&CONTEST);
      if(err_ret!=_SER422_NO_ERROR)
      {
         ERROR_HANDLER();
         break;
      }
   }
    
    // Attende la ricezione della configurazione se necessario
   _EVSET(_EV2_PCB249U2_STARTUP_OK);
   printf("PCB249U2: ATTENDE CONFIGURAZIONE..\n");
   _EVWAIT_ANY(_EV1_DEV_CONFIG_OK);
   printf("PCB249U2: CONFIGURAZIONE OK. INIZIO LAVORO\n");

   
    // il registro RG249U2_PR_CALIBRATED se == 1 significa che è stata già calibrata
    // e dunque bisogna utilizzarne i valori.
    generalConfiguration.colli_filter_update = false;

    if(_DEVREGL(RG249U2_PR_CALIBRATED,CONTEST)==1){

        printf("ACQUISIZIONE POSIZIONE FILTRI CALIBRATI A BANCO\n");
        if((generalConfiguration.colli_filter[0]!=_DEVREGL(RG249U2_PR_FILTER0,CONTEST))||
           (generalConfiguration.colli_filter[1]!=_DEVREGL(RG249U2_PR_FILTER1,CONTEST))||
           (generalConfiguration.colli_filter[2]!=_DEVREGL(RG249U2_PR_FILTER2,CONTEST))||
           (generalConfiguration.colli_filter[3]!=_DEVREGL(RG249U2_PR_FILTER3,CONTEST))
           ) {
              printf("AGGIORNAMENTO FILTRI SU MASTER....\n");
              generalConfiguration.colli_filter[0]=_DEVREGL(RG249U2_PR_FILTER0,CONTEST);
              generalConfiguration.colli_filter[1]=_DEVREGL(RG249U2_PR_FILTER1,CONTEST);
              generalConfiguration.colli_filter[2]=_DEVREGL(RG249U2_PR_FILTER2,CONTEST);
              generalConfiguration.colli_filter[3]=_DEVREGL(RG249U2_PR_FILTER3,CONTEST);
              generalConfiguration.colli_filter[4]=1; // Indica che il dispositivo risulta calibrato

              // Invio dei nuovi dati al Master affinchè li salvi..
              mccGuiNotify(_COLLI_ID,MCC_CALIB_FILTRO,generalConfiguration.colli_filter,5);
        }

        // Reset del flag di calibrazione sulla scheda
        pcb249U2ResetCalibFilterFlag();

    }else{
        printf("CARICAMENTO TARGET PER I FILTRI\n");
        Ser422WriteRegister(_REGID(RG249U2_PR_FILTER0),generalConfiguration.colli_filter[0] ,10,&CONTEST);
        Ser422WriteRegister(_REGID(RG249U2_PR_FILTER1),generalConfiguration.colli_filter[1] ,10,&CONTEST);
        Ser422WriteRegister(_REGID(RG249U2_PR_FILTER2),generalConfiguration.colli_filter[2] ,10,&CONTEST);
        Ser422WriteRegister(_REGID(RG249U2_PR_FILTER3),generalConfiguration.colli_filter[3] ,10,&CONTEST);

    }


   ////////////////////////////////////////////////////////////////////////
   /*
                  GESTIONE DEL CICLO DI LAVORO ORDINARIO
       Il driver effettua un polling sui principali registri di lettura
       iscritti nella lista di polling ed effettua un controllo periodico
       sui registri di scrittura per eventuali udates in caso di differenze 
   */
   /////////////////////////////////////////////////////////////////////////
    _EVCLR(_EV0_PCB249U2_COLLI);
    _EVCLR(_EV0_PCB249U2_FILTRO);
    posizioneFiltro= _DEVREGL(RG249U2_FILTER_CURPOS,CONTEST);
    backfront_eseguito = false;
    filtro_eseguito = false;

    // Prima di entrare in ciclo di lavoro, posiziona lo specchio in HOME
    int tentativi=10;
    while((!pcb249U2MirrorHomeCmd()) && (tentativi)) _time_delay(500);
    if(!tentativi) printf("PCB249U2: TENTATIVO DI PORTARE LO SPECCHIO IN HOME FALLITO\n");

    STATUS.ready=1;
    while(1)
    {
        unsigned char data[4];

        // Attende di essere svegliato da un comand
        _EVWAIT_ANY(_MOR2(_EV0_PCB249U2_COLLI,_EV0_PCB249U2_FILTRO)); // Attende lo sbloccoo

        printf("DRIVER PCB249U2 IN ESECUZIONE\n");

        // AZIONAMENTO FILTRO _____________________________________________________
        if(_IS_EVENT(_EV0_PCB249U2_FILTRO)){
            //_EVCLR(_EV0_PCB249U2_FILTRO);

            // Copia ila richiesta nei dati effettivi
            comando_filtro = filtro_req;
            target_filtro = pos_req;

            printf("POSIZIONAMENTO FILTRO IN ESECUZIONE: INDEX=%d, POS=%d\n", filtro_req,pos_req);

            // Legge la posizione corrente
            pcb249U2getFilterCurrentPosition();

            // Se la posizione correnter è quella attesa allora termina subito qui
            if(posizioneFiltro==pos_req){
                printf("POSIZIONAMENTO FILTRO OK! -CMD:%d PF:%d\n",comando_filtro,posizioneFiltro);
                data[0]=1; // OK
                filtro_eseguito = true;
                data[1] = comando_filtro;  // Indice filtro
                data[2] = posizioneFiltro; // Posizione filtro corrente
                data[3] = pos_req;         // Posizione filtro richiesto
                mccGuiNotify(id_filtro,MCC_SET_FILTRO,data,4);
                _EVCLR(_EV0_PCB249U2_FILTRO);
                continue;
            }

            // Attende un eventuale busy
            if(pcb249WaitBusy(50)==false){
                pcb249U2getFilterCurrentPosition();
                printf("ERRORE POSIZIONAMENTO FILTRO: ATTESA BUSY! -CMD:%d PREQ:%d, PF:%d\n",comando_filtro,pos_req, posizioneFiltro);
                data[0] = 0; // Errore
                data[1] = comando_filtro;  // Indice filtro
                data[2] = posizioneFiltro; // Posizione filtro
                data[3] = pos_req;         // posizione filtro richiesto
                mccGuiNotify(id_filtro,MCC_SET_FILTRO,data,4);
                _EVCLR(_EV0_PCB249U2_FILTRO);
                continue;
            }

            // Reset Faults
            if(_TEST_BIT(PCB249U2_FAULT))    pcb249U2ResetFaults();

            // Carica il target
            if(comando_filtro==0)
                Ser422WriteRegister(_REGID(RG249U2_PR_FILTER0),target_filtro,10,&CONTEST);
            else if(comando_filtro==1)
                Ser422WriteRegister(_REGID(RG249U2_PR_FILTER1),target_filtro,10,&CONTEST);
            else if(comando_filtro==2)
                Ser422WriteRegister(_REGID(RG249U2_PR_FILTER2),target_filtro,10,&CONTEST);
            else if(comando_filtro==3)
                Ser422WriteRegister(_REGID(RG249U2_PR_FILTER3),target_filtro,10,&CONTEST);

            // Invia il comando
            if(!pcb249U2SetFiltroCmd(comando_filtro)){               
                pcb249U2getFilterCurrentPosition();
                printf("ERRORE POSIZIONAMENTO FILTRO: ERRORE COMANDO! -CMD:%d PREQ:%d, PF:%d\n",comando_filtro,target_filtro, posizioneFiltro);
                data[0] = 0; // Errore
                data[1] = comando_filtro;  // Indice filtro
                data[2] = posizioneFiltro; // Posizione filtro
                data[3] = target_filtro;         // posizione filtro richiesto
                mccGuiNotify(id_filtro,MCC_SET_FILTRO,data,4);
                _EVCLR(_EV0_PCB249U2_FILTRO);
                continue;
            }

           _time_delay(50);

           // Attende un eventuale busy
           if(pcb249WaitBusy(50)==false){
               pcb249U2getFilterCurrentPosition();
               printf("ERRORE POSIZIONAMENTO FILTRO: ATTESA BUSY DOPO COMANDO! -CMD:%d PREQ:%d, PF:%d\n",comando_filtro,target_filtro, posizioneFiltro);
               data[0] = 0; // Errore
               data[1] = comando_filtro;  // Indice filtro
               data[2] = posizioneFiltro; // Posizione filtro
               data[3] = target_filtro;   // posizione filtro richiesto
               mccGuiNotify(id_filtro,MCC_SET_FILTRO,data,4);
               _EVCLR(_EV0_PCB249U2_FILTRO);
               continue;

           }

           // Legge la posizione finale
           pcb249U2getFilterCurrentPosition();

           // Rilegge il registro di fault se necessario
           if(_TEST_BIT(PCB249U2_FAULT)){
               printf("ERRORE POSIZIONAMENTO FILTRO: ESECUZIONE FALLITA! -CMD:%d PREQ:%d, PF:%d\n",comando_filtro,target_filtro, posizioneFiltro);
               data[0] = 0; // Errore
               data[1] = comando_filtro;  // Indice filtro
               data[2] = posizioneFiltro; // Posizione filtro
               data[3] = target_filtro;   // posizione filtro richiesto
               mccGuiNotify(id_filtro,MCC_SET_FILTRO,data,4);
               _EVCLR(_EV0_PCB249U2_FILTRO);
               continue;
            }

           // Verifica per un altro comando richiesto in successione
           else{
               printf("POSIZIONAMENTO FILTRO COMPLETATA!  -CMD:%d PREQ:%d, PF:%d\n",comando_filtro,target_filtro, posizioneFiltro);
               data[0]=1; // OK
               filtro_eseguito = true;
           }

           // Se la richiesta è ancora uguale a quanto comandato
           // allora vuol dire che non ci sono altri comandi oppure che
           // il nuovo comando è uguale allo stato attuale
           if((comando_filtro==filtro_req)&&(target_filtro==pos_req)){
               _EVCLR(_EV0_PCB249U2_FILTRO);
               filtro_eseguito = true;
               data[0] = 1; // OK
               data[1] = comando_filtro;  // Indice filtro
               data[2] = posizioneFiltro; // Posizione filtro
               data[3] = target_filtro;   // posizione filtro richiesto
               mccGuiNotify(id_filtro,MCC_SET_FILTRO,data,4);
               _EVCLR(_EV0_PCB249U2_FILTRO);
               continue;

           }

           // Continua con il comando successivo
        }



        // Comando impostazione collimazione fronte retro _________________
        if(_IS_EVENT(_EV0_PCB249U2_COLLI)){
            //_EVCLR(_EV0_PCB249U2_COLLI);

            // Copia ila richiesta nei dati effettivi
            back = backcolli_req;
            front = frontcolli_req;
            printf("DRIVER ESECUZIONE COLLIMAZIONE FRONTALE: B:%d, F:%d\n",back, front);

            if(pcb249WaitBusy(50)==false){
                    printf("DRIVER COLLIMAZIONE FRONTALE: TIMEOUT ATTESA BUSY\n");
                    _EVCLR(_EV0_PCB249U2_COLLI);
                    continue;
            }

            // Reset Faults
            if(_TEST_BIT(PCB249U2_FAULT))    pcb249U2ResetFaults();

            if(!pcb249U2ColliCmd(back, front)){
                printf("DRIVER COLLIMAZIONE FRONTALE COMANDO FALLITO\n");
                _EVCLR(_EV0_PCB249U2_COLLI);
                continue;
            }
            _time_delay(50);

            // Attesa fine operazioni e rilettura registro di Target
            printf("DRIVER F+B ATTESA COMPLETAMENTO\n");
            if(pcb249WaitBusy(50)==false){
                    printf("DRIVER COLLIMAZIONE FRONTALE TIMEOUT\n");
                    _EVCLR(_EV0_PCB249U2_COLLI);
                    continue;
            }

            // Fine comando
            if(_TEST_BIT(PCB249U2_FAULT)){
                printf("DRIVER COLLIMAZIONE FRONTALE FALLITA\n");
                _EVCLR(_EV0_PCB249U2_COLLI);
                continue;
            }

            // Se la richiesta è ancora uguale a quanto comandato
            // allora vuol dire che non ci sono altri comandi oppure che
            // il nuovo comando è uguale allo stato attuale
            if((back==backcolli_req)&&(front==frontcolli_req)){
                printf("DRIVER COLLIMAZIONE FRONT-BACK CONCLUSA CON SUCCESSO\n");
                _EVCLR(_EV0_PCB249U2_COLLI);
                backfront_eseguito = true;
            }else{
                printf("DRIVER NUOVA COLLIMAZIONE FRONT-BACK IN CODA ..\n");
            }

        }
    }

}

int pcb249U2getFilterCurrentPosition(void){
    // Legge la posizione corrente
    if(Ser422ReadRegister(_REGID(RG249U2_FILTER_CURPOS),10,&CONTEST)==_SER422_NO_ERROR) posizioneFiltro = _DEVREGL(RG249U2_FILTER_CURPOS,CONTEST);
    else posizioneFiltro=-1;

    return posizioneFiltro;
}


//////////////////////////////////////////////////////////////////////////////
/*
_PCB249U2_Error_Enum GetFwRevision(void)
        La funzione legge il codice di revisione del firmware del 
        target.

PARAM:
        -
RETURN:
      TRUE: Lettura avvenuta con successo
      FALSE: Problema di comunicazione con il target

      PCB249U2_Stat.maj_code/PCB249U2_Stat.min_code = codice revisione
Autore: M. Rispoli
Data: 24/10/2014
*/
//////////////////////////////////////////////////////////////////////////////
bool GetFwRevision(void)
{
_Ser422_Command_Str frame;

  
  frame.address = TARGET_ADDRESS;
  frame.attempt = 4;
  frame.cmd=SER422_COMMAND;
  
  // Scrive il codice comando
  frame.data1=_CMD1(PCB249U2_GET_REV);
  frame.data2=_CMD2(PCB249U2_GET_REV);
  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
  
  // Eventuali errori di comunicazione
  if(frame.retcode!=SER422_DATA) return FALSE;

  STATUS.maj_code = frame.data2;
  STATUS.min_code = frame.data1;
  generalConfiguration.revisioni.pcb249U2.maj = STATUS.maj_code; 
  generalConfiguration.revisioni.pcb249U2.min = STATUS.min_code; 

    
  return TRUE;
}

bool pcb249U2ColliCmd(unsigned char back, unsigned char front)
{
   if(generalConfiguration.collimator_model_error) return false;

  unsigned char buffer[4];
  int i = 20;
  printf("pcb249U2ColliCmd: F=%d, B=%d\n",front, back);
  
  while(--i)
  {
    buffer[0]=0;

    // Invio comando speciale
    Ser422SendRaw(0x55, back, front, buffer, 10);  
    if(buffer[0]!=0x15) continue;                       // Errore 
    if((buffer[1]==255)&&(buffer[2]==0)) 
    {
      // Busy
      _time_delay(100);

      continue;
    }

    return TRUE;
  }

  return FALSE;  
}

bool pcb249U2MirrorHomeCmd(void)
{
   if(generalConfiguration.collimator_model_error) return false;


   _Ser422_Command_Str frame;

  // Prepara il comando di download
  frame.address = TARGET_ADDRESS;
  frame.attempt = 10;
  frame.cmd=SER422_COMMAND;
  frame.data1=_CMD1(PCB249U2_MIRROR_HOME);
  frame.data2=_CMD2(PCB249U2_MIRROR_HOME);   

  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
  if(frame.retcode==SER422_COMMAND_OK) return TRUE;
  else return FALSE;
}

bool pcb249U2SetFiltroCmd(unsigned char cmd)
{
    if(generalConfiguration.collimator_model_error) return false;


   _Ser422_Command_Str frame;

  // Prepara il comando di download
  frame.address = TARGET_ADDRESS;
  frame.attempt = 10;
  frame.cmd=SER422_COMMAND;
  printf("FILTRO: %d\n",cmd);
  frame.data1=_CMD1(PCB249U2_FILTER);
  frame.data2=cmd;   
  
  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
  if(frame.retcode==SER422_COMMAND_OK) return TRUE;
  else return FALSE;
}

// Comando di impostazione filtro durante sequenze raggi: driver in FREEZE
bool pcb249U2RxSetFiltroCmd(unsigned char cmd)
{
   if(generalConfiguration.collimator_model_error) return false;


   _Ser422_Command_Str frame;

  // Prepara il comando di download
  frame.address = TARGET_ADDRESS;
  frame.attempt = 10;
  frame.cmd=SER422_COMMAND;
  printf("FILTRO: %d\n",cmd);
  frame.data1=_CMD1(PCB249U2_FILTER);
  frame.data2=cmd;

  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
  if(frame.retcode==SER422_COMMAND_OK) return TRUE;
  else return FALSE;
}

// Comando di store
bool pcb249U2StoreCmd(void)
{
   _Ser422_Command_Str frame;

  // Prepara il comando di download
  frame.address = TARGET_ADDRESS;
  frame.attempt = 1;
  frame.cmd=SER422_COMMAND;
  frame.data1=_CMD1(PCB249U2_EEPROM_STORE_ALL);
  frame.data2=_CMD2(PCB249U2_EEPROM_STORE_ALL);;

  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
  if(frame.retcode==SER422_COMMAND_OK) return TRUE;
  else return FALSE;
}

void pcb249U2ResetCalibFilterFlag(void)
{
  // Azzera il flag di calibrazione dato che il sistema ha già acquisito i dati
  if(Ser422WriteRegister(_REGID(RG249U2_PR_CALIBRATED),0,40,&CONTEST)==_SER422_NO_ERROR){
      printf("PCB249U2: RESET FLAG FILTRI E STORE ALL COMMAND!\n");
      pcb249U2StoreCmd();
      _time_delay(100);
  }

  return;
}

/*
    CMD: 0:3
    TMO:0x3F MAX
*/
bool pcb249U2LampCmd(unsigned char cmd, unsigned char tmo)
{
    if(generalConfiguration.collimator_model_error) return false;


   _Ser422_Command_Str frame;

  // Prepara il comando di download
  frame.address = TARGET_ADDRESS;
  frame.attempt = 10;
  frame.cmd=SER422_COMMAND;
  frame.data1=_CMD1(PCB249U2_LAMP);
  frame.data2=((cmd & 0x3)<<6)|(tmo&0x3F);
  
  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
  if(frame.retcode==SER422_COMMAND_OK) return TRUE;
  else return FALSE;
}

bool pcb249U2MirrorOutCmd(void)
{
    if(generalConfiguration.collimator_model_error) return false;


   _Ser422_Command_Str frame;


  // Prepara il comando di download
  frame.address = TARGET_ADDRESS;
  frame.attempt = 10;
  frame.cmd=SER422_COMMAND;
  frame.data1=_CMD1(PCB249U2_MIRROR_OUT);
  frame.data2=_CMD2(PCB249U2_MIRROR_OUT);
  
  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
  if(frame.retcode==SER422_COMMAND_OK) return TRUE;
  else return FALSE;

}

void ERROR_HANDLER(void)
{
   // Segnalazione driver disconnesso
   _EVCLR(_EV1_PCB249U2_CONNECTED);

   // Riconfigurazione del driver a seguito della ripartenza
   printf("PCB249U2 ERRORE: ATTESA RICONNESSIONE E RICONFIGURAZIONE REGISTRI\n"); 
  
   while(1){
    
    _time_delay(100);
    
    // Richiesta revisione firmware a target
    while(GetFwRevision()==FALSE) _time_delay(100);
    printf("PCB249U2:REVISIONE FW TARGET:%d.%d\n",STATUS.maj_code,STATUS.min_code);     

    // Carica sulla periferica lo stato dei registri cosi come erano prima del reset
    printf("PCB249U2: DOWNLOAD REGISTRI ...\n");
    if(Ser422UploadRegisters(10, &CONTEST)== FALSE)   continue;  
   
    // Carica Tutti i registri RD / RW
    int i;
    for(i=0;i<PCB249U2_NREGISTERS;i++) 
    {
      if(Ser422ReadRegister(i,4,&CONTEST)!=_SER422_NO_ERROR) break; 
    }
    if(i!=PCB249U2_NREGISTERS) continue;  
    break;
  }
  

  // Segnalazione driver connesso
  _EVSET(_EV1_PCB249U2_CONNECTED);
  
  // Ripartenza completata. Può tornare da dove aveva lasciato
  printf("PCB249U2 RIPARTITA CORRETTAMENTE\n"); 

  return;

}


bool pcb249U2ResetFaults(void)
{
   _Ser422_Command_Str frame;
    
  // Prepara il comando di download
  frame.address = TARGET_ADDRESS;
  frame.attempt = 10;
  frame.cmd=SER422_COMMAND;
  frame.data1=_CMD1(PCB249U2_CLEAR_ERRORS);
  frame.data2=_CMD2(PCB249U2_CLEAR_ERRORS);
    
  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);  
  if(frame.retcode == SER422_COMMAND_OK) return TRUE;  
  return FALSE;  

}
    
//_________________________________________________________________________________________________
//                               FUNZIONI DI INTERFACCIA


bool pcb249U2Mirror(unsigned char cmd)
{
  bool ris;
  if(generalConfiguration.collimator_model_error) return false;


   if(pcb249WaitBusy(50)==false) return false;

   pcb249U2ResetFaults();
   
   if(cmd==0)
   {
     ris = pcb249U2MirrorHomeCmd();
     goto fine_mirror;
   }
   
   // Tentativo di Mirror OUT
   ris = pcb249U2MirrorOutCmd();
   if(ris) goto fine_mirror;
   
   // Se Mirror Out fallisce allora prima prova a portarlo in HOME
   if(pcb249U2MirrorHomeCmd()==FALSE) return FALSE;
   _time_delay(500);   
   if(pcb249WaitBusy(50)==false) return false;

   // Tentativo di Mirror OUT
   if(!pcb249U2MirrorOutCmd()) return false;

   // Attesa fine operazioni e rilettura registro di Target
fine_mirror:
   if(pcb249WaitBusy(50)==false) return false;
   Ser422ReadRegister(_REGID(RG249U2_MIRROR_STAT),10,&CONTEST);
   return true;

}

// BIT6: 1 = LUCE ON; 0 = LUCE OFF
// BIT7: 1 = MUOVE SPECCHIO, 0 = NON MUOVE SPECCHIO
// BIT[0:5] = Timeout (0==INFINITO)

/*
  cmd==0 -> Lampada OFF
  cmd==1 -> Lampada ON
  cmd==2 -> Lampada OFF + MIRROR HOME
  cmd==3 -> Lampada ON + MIRROR OUT
tmo= Timout Lampada (0:64) (32 secondi max), 0 = infinito
*/
bool pcb249U2Lamp(unsigned char cmd, unsigned char tmo, bool wait)
{ 
    if(generalConfiguration.collimator_model_error) return false;

    if(pcb249WaitBusy(50)==false) return false;

   // Comanda la lampada
   if(!pcb249U2LampCmd(cmd,tmo)) return false;
   if(wait==FALSE)  return true;
   _time_delay(50);
   if(pcb249WaitBusy(50)==false) return false;

   // Rilegge i registri di stato
   Ser422ReadRegister(_REGID(RG249U2_MIRROR_STAT),10,&CONTEST);
   Ser422ReadRegister(_REGID(RG249U2_LAMP_STAT),10,&CONTEST);
   return true;

}

/*
    IMPOSTAZIONE FORMATO DI COLLIMAZIONE
*/
bool pcb249U2SetColli(unsigned char backin, unsigned char frontin)
{
    if(generalConfiguration.collimator_model_error) return false;


    // Mette nella coda di comando il prossimo movimento
    printf("RICHIESTA LAME B+F IN POSIZIONE B:%d, F:%d\n",backin,frontin);
    backcolli_req = backin;
    frontcolli_req = frontin;
    backfront_eseguito = false;
    _EVSET(_EV0_PCB249U2_COLLI);
    return true;
}

/* ______________________________________________________________________________________
 Funzione di interfaccia per l'impostazione del filtro.

 La funzione richiede di posizionare il filtro di indice <cmd>
 il cui target posizionale dovrebbe essere <posizione_target>
 Il driver aggiornerà la posizione target del filtro selezionato
 e poi ne richiederà la selezione

 Più comandi in successione verranno gestiti dando priorità all'ultimo
 comando ricevuto
*/
void pcb249U2SetFiltro(unsigned char cmd, unsigned char posizione_target, unsigned char id)
{
    if(generalConfiguration.collimator_model_error) return ;


    filtro_req = cmd;
    pos_req = posizione_target;
    id_filtro = id;
    filtro_eseguito = false;
    _EVSET(_EV0_PCB249U2_FILTRO);
    return ;

}

// Attende il READY con timeout di 100ms unit
bool pcb249WaitBusy(int timeout){
    while(timeout){
        if(Ser422ReadRegister(_REGID(RG249U2_SYS_FLAGS0),10,&CONTEST) == _SER422_NO_ERROR) {
            if(!_TEST_BIT(PCB249U2_BUSY)) return true;
        }

        _time_delay(100);
        timeout--;
    }
    printf("Timeout waiting pcb249WaitBusy()\n");
    return false;
}



/*
  Funzione configuratrice
*/
bool config_pcb249U2(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len){
  
   // Assegna la configurazione dei filtri 
   generalConfiguration.colli_filter[0] = buffer[0];
   generalConfiguration.colli_filter[1] = buffer[1];
   generalConfiguration.colli_filter[2] = buffer[2];
   generalConfiguration.colli_filter[3] = buffer[3];

   // Hotfix 11C
   generalConfiguration.filterTomoEna = buffer[4];
   generalConfiguration.filterTomo[0] = buffer[5];
   generalConfiguration.filterTomo[1] = buffer[6];
   generalConfiguration.filterTomo[2] = buffer[7];

   // Configurazione specchio
   generalConfiguration.mirror_position = buffer[8]+256*buffer[9];

   // Scrive il target specchio nel device
   Ser422WriteRegister(_REGID(RG249U2_PR_MIRROR_STEPS),generalConfiguration.mirror_position ,10,&CONTEST);

#ifdef PRINTCFG

   printf("CONFIG FILTRI: F0=%d, F1=%d, F2=%d, F3=%d\n",  buffer[0], buffer[1], buffer[2], buffer[3]);
   printf("CONFIG SPCCHIO: STEPS=%d\n",  generalConfiguration.mirror_position);
   printf("CONFIG TOMO FILTRI: ENA:%d A0=%d, A1=%d, A2=%d\n",  buffer[4], buffer[5], buffer[6], buffer[7]);

#endif


   return true;
}



// Hotfix 11C
// Questa funzione è pensata sotto RX, quindi con il driver fermo
bool pcb249U2SetFiltroRaw(unsigned char val)
{
   if(generalConfiguration.collimator_model_error) return false;


  _Ser422_Command_Str frame;

  // Prepara il comando di download
  frame.address = TARGET_ADDRESS;
  frame.attempt = 10;
  frame.cmd=SER422_COMMAND;  
  frame.data1=_CMD1(PCB249U2_FILTER_RAW);
  frame.data2=val;   
  
  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);  
  if(frame.retcode==SER422_COMMAND_OK){
      return TRUE;
  }
  else return FALSE;

}


int getTomoDeltaFilter(int angolo){
        if(angolo>generalConfiguration.filterTomo[2]) return -3;
        else if(angolo>generalConfiguration.filterTomo[1]) return - 2;
        else if(angolo>generalConfiguration.filterTomo[0]) return - 1;
        else if(angolo>-generalConfiguration.filterTomo[0]) return 0 ;
        else if(angolo>-generalConfiguration.filterTomo[1]) return 1;
        else if(angolo>-generalConfiguration.filterTomo[2]) return 2;
        else return 3;
}

/* __________________________________________________________________________________________________
    Funzione di controllo sullo stato del filtro.
    La funzione viene utilizzato dalle pocedure raggi per verificare e correggere eventuali errori
    di posizionamento del filtro.
    Nel caso in cui il filtro si trovasse fuori posizione, la funzione cercherà di riposizionarlo
    La funzione Attenderà il completamento dell'operazione oppure uscirtà in errore.

 */
bool waitRxFilterCompletion(void){
    int timeout = 50;

    int tmo=timeout;

    // Attende il completamento di un posizionamento pendente
    while(_IS_EVENT(_EV0_PCB249U2_FILTRO)){
        _time_delay(100);
        tmo--;
        if(!tmo){
            printf("TIMEOUT ATTESA COMPLETAMENTO POSIZIONAMENTO FILTRO DURANTE RAGGI\n");
            return false;
        }
    }

    // Verifica se la posizione è quella attesa
    if(!filtro_eseguito){

        // Riprova a selezionare il filtro per non perdere l'esposizione
        pcb249U2SetFiltro(filtro_req,pos_req,0);

        // Verifica dello stato di posizionamento del filtro
        tmo=timeout;

        // Attende il completamento di un posizionamento pendente
        while(_IS_EVENT(_EV0_PCB249U2_FILTRO)){
            _time_delay(100);
            tmo--;
            if(!tmo){
               printf("TIMEOUT ATTESA COMPLETAMENTO POSIZIONAMENTO FILTRO DURANTE RAGGI\n");
                return false;
            }
        }
    }

    // Se infine non è riuscito allora si ferma qui
    if(!filtro_eseguito) return false;

    // Controllo completato con successo
    return true;
}

// Funzione usata dalle procedure raggi per accertarsi della
// corretta collimazione
bool wait2DBackFrontCompletion(int timeout){

    int tmo = timeout;


    // Attende il completamento di una collimazione pendente
    while(_IS_EVENT(_EV0_PCB249U2_COLLI)){
        _time_delay(100);
        tmo--;
        if(!tmo){
            printf("TIMEOUT ATTESA EVENTO COMPLETAMENTO COLLIMAZIONE FRONT+BACK DURANTE RAGGI\n");
            return false;
        }
    }

    // Se il comando è fallito, riprova a collimare
    if(!backfront_eseguito){
        printf("RIPROVA AD ESEGUIRE IL COMANDO F+B CHE ERA FALLITO!\n");
        pcb249U2SetColli(backcolli_req ,frontcolli_req); // ripete il comando
        _time_delay(50);
        tmo = timeout;
        while(_IS_EVENT(_EV0_PCB249U2_COLLI)){
            _time_delay(100);
            tmo--;
            if(!tmo){
                printf("NIENTE, NON E' PROPRIO RIUSCITO A COMPETARE F+B\n");
                return false;
            }
        }
    }

    // Se infine non è riuscito allora si ferma qui
    if(!backfront_eseguito){
        printf("COMANDO F+B COMPLETATO MA FALLITO ANCORA! \n");
        return false;
    }

    // Controllo completato con successo
    printf("CONTROLLO F+B OK! \n");
    return true;
}

/* EOF */
 
