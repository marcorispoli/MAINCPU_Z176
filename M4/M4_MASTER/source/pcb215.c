#define _PCB215_C
#include "dbt_m4.h" 

#define TARGET_ADDRESS 0x11
#define _DEF_PCB215_DRIVER_DELAY 100

#define CONTEST PCB215_CONTEST
#define STATUS  (*((_PCB215_Stat_Str*)(&PCB215_CONTEST.Stat)))
#define ERROR_HANDLER   pcb215DriverCommunicationError

static bool resetDriver = TRUE; // Indica la prima accensione

// Funzione di servizio interne al modulo
static bool GetFwRevision(void);
static void enterFreezeMode(void);
static bool pcb215ResetFault(void);


static void ERROR_HANDLER(void);
static bool pcb215UpdateRegisters(void);
static void pcb215VerifyComprData(void);

static unsigned char getStandardPad(unsigned char pad);
static void updatePadDependentRegisters(void);
static bool classifyPad(void);

static bool verifyComprDataInit=TRUE;
static bool verifyClassPadInit=TRUE;
static bool executeConfig; // Appena può deve effettuare la configurazione
static unsigned char padLevel=200;

// Modo Calibrazione
static void pcb215ConfigCalibMode(void);
static void pcb215CalibMode(void);
static bool executeCalibConfig; // Flag di inizializzazione alla calibrazione

static bool pcb215_startup = true;

// DEFINE PARTICOLARI PER AGEVOLARE LA SCRITTURA DEL CODICE
#define PAD (generalConfiguration.comprCfg.calibration.pads[generalConfiguration.comprCfg.padSelezionato])
#define CONFIG (generalConfiguration.comprCfg)
#define CALIBRATION (CONFIG.calibration)
#define IS_VALID_PAD (generalConfiguration.comprCfg.padSelezionato<PAD_ENUM_SIZE)
#define POTTER (generalConfiguration.potterCfg.potId)
#define POTMAG (generalConfiguration.potterCfg.potMagFactor & 7)
#define INGRANDIMENTO (generalConfiguration.comprCfg.calibration.sbalzoIngranditore[POTMAG])
#define THRESHOLD_NO_PAD 15 // (N) Soglia di compressione senza il PAD riconosciuto
#define THRESHOLD_CC     30 // (N) Soglia di compressione in zona CC
#define THRESHOLD_INCL   60 // (N) Soglia di compressione in zona angolata
#define THRESHOLD_ANG    50 // (°) Soglia per la definizione di area angolata     

//////////////////////////////////////////////////////////////////////////////
/*
void pcb215_driver(uint32_t taskRegisters)

        La funzione è un task indipendente e si occupa di gestire
        la comunicazione con la PCB215 e il suo funzionamento.

        Il task effettua una prima fase di startup ed una seconda fase 
        di Loop. Le due fasi sono di seguito descritte:
  
        STARTUP:
            - Vengono inizializzate le variabili interne e di interfaccia;
            - Viene inviato il comando di seriale di Reset Faults;
              se il Target non è attivo/connesso e quindi non risponde
              il task riprova ad inviare il comando ogni 100ms;
            - Viene inviato il comando di richiesta codice firmware;
            - Vengono letti tutti i registri del Target configurati
              nella struttura PCB215_Registers.
            - Viene controllata (opzionale) la lista dei registri scrivibili
              passata al driver per la pre-impostazione sul Target. Se la lista
              è corretta, il driver tenta di scrivere sul Target il contenuto
              di tale lista;
        
        LOOP DI LAVORO:
            - (1)Vengono letti i registri di stato principali: 
              SYS_FLAGS0 e SYS_FLAGS1. Viene quindi controllata un'eventuale 
              condizione di fault del Target e in caso di fault viene richiesto 
              al target il codice di fault;
            - Viene gestita la lista CONTEST.locPollingList che contiene 
              la lista di registri, passata/aggiornata dall'applicazione,
              che devono essere tenuti aggiornati. L'applicazione può in
              ogni momento modificare tale lista.
            - Viene gestita la lista locRegistersList che contiene 
              i registri da scrivere. L'applicazione tenterà la riscrittura
              di tale lista fino a successo completo. Solo allora attiverà
              un flag nel registro di stato STATUS.updated == 1
            - L'applicazione in ogni momento potrà utilizzare alcune funzioni
              di seguito descritte che invieranno comandi particolari
              al Target. In tale situazione il driver si sospenderà 
              subito dopo la lettura dei registri di base (1) fino al termie 
              della sequenza di comando in corso.

      API MODULO ---------------------------------------------------------

      - CREAZIONE DI UNA LISTA DI REGISTRI DA POLLARE:
        L'applicazione può creare una lista di registri che il driver deve 
        tenere aggiornata. Per creare la lista occorre:
      - Dichiarare una lista di tipo _DeviceAppRegister_Str:
                    _DeviceAppRegister_Str lista;
      - Utilizzare le macro di riempimento della lista:
        _LISTCREATE(<id-registro>,<lista>,<valore 16bit registro>) 
        per il primo elemento della lista;
        _ADDITEMLIST(<id-registro>,<lista>,<valore 16bit registro>) 
        per gli elementi successivi della lista;
      - Formattare la lista con la funzione:
        bool pcb215FormatRegListCrc(&lista); 
      - Attivare la lista cosi' creata:
        bool pcb215SetPollingList(&lista) ; 

      - MODIFICA DI UNA LISTA ATTIVA DI REGISTRI DA POLLARE:
      Una lista attiva può essere modficata dall'applicazione, 
      aggiungendo o togliendo registri alla stessa senza dover ricreare
      una nuova lista. 
      - Per aggiungere un registro alla lista:
        bool pcb215AddRegToPollingList(unsigned char id);
      - Per rimuovere un registro dalla lista:
        bool pcb215DelRegToPollingList(unsigned char id);

      - CREAZIONE DI UNA LISTA DI REGISTRI DA MODIFICARE:
        L'applicazione può creare una lista di registri che il driver deve 
        aggiornare. Il driver effettua una copia interna di tale lista
        che cancella non appena le modifiche sono state trasmesse al Target.
        La lista originale dell'Applicazione tuttavia non viene distrutta
        e può essere riutilizzata in qualsiasi momento.
        Il driver NON invia comandi di scrittura al Target per registri
        il cui valore contenuto nella lista non sia variato rispetto 
        all'ultima scrittura. Ovviamente il meccanismo di scrittura tramite
        lista deve essere applicato ai soli registri che non siano scrivibili
        da altre fonti.
        Per poter gestire registri di tale natura occorrerà impostare la
        caratteristica di _VOLATILE al registro stesso. In tal caso 
        il driver forzerà la scrittura dello stesso senza controllare il valore 
        locale. Tale flag dovrà essere applicato ai soli registri scrivibili
        da altre fonti per ottimizzare i tempi di accesso alla seriale.

      - dichiarare una lista di tipo _DeviceAppRegister_Str:
        _DeviceAppRegister_Str lista;
      - utilizzare le macro di riempimento della lista:
        _LISTCREATE(<id-registro>,<lista>,<valore 16bit registro>) 
        per il primo elemento della lista;
        _ADDITEMLIST(<id-registro>,<lista>,<valore 16bit registro>) 
        per gli elementi successivi della lista;
      - formattare la lista con la funzione:
        bool pcb215FormatRegListCrc(&lista); 
      - Attivare la lista cosi' creata:
        bool pcb215SetRegisterList(&lista) ; 
        Immediatamente dopo la chiamata alla funzione, 
        il bit di stato STATUS.updated ==  0. 
        Non appena il driver completerà la scrittura, tale flag
        verrà impostato a 1. 

  
bool pcb215SetSblocco();                                  // Il carrello si porta in posizione di sblocco
bool pcb215MovePadUpward(unsigned char mm);               // Il carrello si muove verso l'alto di tot mm
         // Aggiunge un registro alla lista polling corrente

       
      
      ERRORI / FAULTS


Autore: M. Rispoli
Data: 19/09/2014
*/
//////////////////////////////////////////////////////////////////////////////
void pcb215_driver(uint32_t taskRegisters)
{
  int i;
  unsigned char data;
  _SER422_Error_Enum err_ret;
  bool write_ok;   

  // Soglie per il riconoscimento del pad
  generalConfiguration.comprCfg.calibration.thresholds[0] = _PAD_THRESHOLD_0;
  generalConfiguration.comprCfg.calibration.thresholds[1] = _PAD_THRESHOLD_1;
  generalConfiguration.comprCfg.calibration.thresholds[2] = _PAD_THRESHOLD_2;
  generalConfiguration.comprCfg.calibration.thresholds[3] = _PAD_THRESHOLD_3;
  generalConfiguration.comprCfg.calibration.thresholds[4] = _PAD_THRESHOLD_4;
  generalConfiguration.comprCfg.calibration.thresholds[5] = _PAD_THRESHOLD_5;
  generalConfiguration.comprCfg.calibration.thresholds[6] = _PAD_THRESHOLD_6;
  generalConfiguration.comprCfg.calibration.thresholds[7] = _PAD_THRESHOLD_7;
  generalConfiguration.comprCfg.calibration.thresholds[8] = _PAD_THRESHOLD_8;
  generalConfiguration.comprCfg.calibration.thresholds[9] = _PAD_THRESHOLD_9;

    // Costruzione del contesto
   CONTEST.pReg = PCB215_Registers;
   CONTEST.nregisters = PCB215_NREGISTERS;
   CONTEST.evm = _EVM(_EV0_PCB215_CFG_UPD);
   CONTEST.evr = &_EVR(_EV0_PCB215_CFG_UPD);
   CONTEST.address = TARGET_ADDRESS;
   printf("PARTENZA DRIVER DRIVER 215: \n");
    
   //////////////////////////////////////////////////////////////////////////
   //                   FINE FASE DI INIZIALIZZAZIONE DRIVER               //             
   //        Inizia il ciclo di controllo e gestione della periferica      //
   //////////////////////////////////////////////////////////////////////////

    // In caso di errore di compilazione in questo punto 
    // significa errore in compilazione della struttura registri
    SIZE_CHECK((sizeof(PCB215_Registers)/sizeof(_DeviceRegItem_Str))!=PCB215_NREGISTERS);
      
    // Segnalazione driver disconnesso
    _EVCLR(_EV1_PCB215_CONNECTED);
    
    // Retrive Task ID
    CONTEST.ID =  _task_get_id();
    
    // Init registro di stato
    memset((void*)&(STATUS), 0,sizeof(_Device_Stat_Str ));

   // Inizializzazione delle mutex
    if (_mutex_init(&(CONTEST.reglist_mutex), NULL) != MQX_OK)
    {
      printf("PCB215: ERRORE INIT MUTEX. FINE PROGRAMMA");
      _mqx_exit(-1);
    }

    if (_mutex_init(&(CONTEST.pollinglist_mutex), NULL) != MQX_OK)
    {
      printf("PCB215: ERRORE INIT MUTEX. FINE PROGRAMMA");
      _mqx_exit(-1);
    }
     
    _mutex_lock(&output_mutex);
    SystemOutputs.CPU_COMPRESSOR_ENA = 0;
    _EVSET(_EV0_OUTPUT_CAMBIATI);
    _mutex_unlock(&output_mutex);

    // Richiesta revisione firmware a target
    while(GetFwRevision()==FALSE) _time_delay(100);      
    printf("PCB215:REVISIONE FW TARGET:%d.%d\n",STATUS.maj_code,STATUS.min_code); 

    // Segnalazione driver connesso
   _EVSET(_EV1_PCB215_CONNECTED);
    
    // Attende l'autorizzazione ad effetuare il refresh registri
    _EVWAIT_ANY(_EV1_UPDATE_REGISTERS);
    
   // Invia comando di reset faults al target e reitera fino a risposta
   pcb215ResetFault();
   
   // Carica Tutti i registri RD / RW
   for(i=0;i<PCB215_NREGISTERS;i++) 
   {
      err_ret = Ser422ReadRegister(i,4,&CONTEST);
      if(err_ret!=_SER422_NO_ERROR)
      {
         ERROR_HANDLER();
         break;
      }
   }   

   
   // Attende la ricezione della configurazione se necessario
   _EVSET(_EV2_PCB215_STARTUP_OK);
   printf("PCB215: ATTENDE CONFIGURAZIONE..\n");
   _EVWAIT_ANY(_EV1_DEV_CONFIG_OK);
   printf("PCB215: CONFIGURAZIONE OK. INIZIO LAVORO\n");

   // Verifica se c'è un pedale di compressione attivo. Nel caso va in errore
   // e rimane in errore fino a che i pedali non si sbloccano per almeno 10 secondi
   if(COMP_PEDALS){
       data=PCB215_ERROR_PEDALS_STARTUP;
       while(mccPCB215Notify(1,PCB215_NOTIFY_ERRORS,&data,1)==false) _time_delay(200);

       while(COMP_PEDALS){
           // Un pedale è premuto: attende qualche secondo prima di proseguire
           _time_delay(1000);
            if(Ser422ReadRegister(_REGID(RG215_PEDALS),4,&CONTEST)!=_SER422_NO_ERROR)  continue;
       }
       data=PCB215_NO_ERRORS;
       while(mccPCB215Notify(1,PCB215_NOTIFY_ERRORS,&data,1)==false) _time_delay(200);

   }


   _mutex_lock(&output_mutex);
   SystemOutputs.CPU_COMPRESSOR_ENA = 1;
   _EVSET(_EV0_OUTPUT_CAMBIATI);
   _mutex_unlock(&output_mutex);


   ////////////////////////////////////////////////////////////////////////
   /*
                  GESTIONE DEL CICLO DI LAVORO ORDINARIO
       Il driver effettua un polling sui principali registri di lettura
       iscritti nella lista di polling ed effettua un controllo periodico
       sui registri di scrittura per eventuali udates in caso di differenze 
   */
   /////////////////////////////////////////////////////////////////////////
   while(1)
   {
     // In caso di Freeze attende di essere liberato
     if(STATUS.freeze) enterFreezeMode();

     if(generalConfiguration.comprCfg.calibrationMode==FALSE) 
     {
       if(executeConfig) 
       {
         executeConfig = FALSE;
         verifyComprDataInit=TRUE;
         verifyClassPadInit=TRUE;
         config_pcb215(false, 0,null,0);
       }else
       {
          // Legge i registri di Base per effettuare la calibrazione
          if(pcb215UpdateRegisters()==FALSE) ERROR_HANDLER();
       }
     }else 
     {
        // All'ingresso effettua il caricamento dei registri sulla PCB215
        if(executeCalibConfig) 
        {
          executeCalibConfig = FALSE;
          pcb215ConfigCalibMode();
        }
       
        // Legge i registri di Base che devono sempre essere aggiornati
        pcb215CalibMode();
        
        // Non appena finisce la calibrazione deve procedere ad una ri-configurazione
        executeConfig = TRUE; 
     }
    
     
     // Aggiornamento registri passati dall'applicazione: in caso di BUSY ripete 
     // La scrittura a meno che non venga annullata dall'applicazione
     _mutex_lock(&(CONTEST.reglist_mutex));
     if(STATUS.updconf)
     {
       // Passa tutta la lista di scrittura
       write_ok=TRUE;
       for(i=0; i<CONTEST.ConfigurationRegistersList.nitem; i++)
       {
          switch(Ser422WriteRegister(CONTEST.ConfigurationRegistersList.id[i],CONTEST.ConfigurationRegistersList.val[i],4,&CONTEST))
          {   
          case _SER422_NO_ERROR: // Tutto oK
          break;
          case _SER422_TARGET_BUSY: // Interrompe il ciclo for
             i=CONTEST.ConfigurationRegistersList.nitem;
             write_ok=FALSE;
          break;
          default: // Errore 
              // Set the error condition and restart blocked the task
              ERROR_HANDLER();
          break;
          } 
       } // for
       
       if(write_ok==TRUE)
       {
        STATUS.updconf = 0;
        printf("PCB215:CONFIG UPDATED!\n");
        
        // Invia segnale di aggiornamento cfg      
        _EVSET(_EV0_PCB215_CFG_UPD);
       }
     }
     _mutex_unlock(&(CONTEST.reglist_mutex));
     
     // Termine della routine di driver
      STATUS.ready=1;
      _EVSET(_EV1_PCB215_RUN);
     _time_delay(_DEF_PCB215_DRIVER_DELAY);
   }
}

//////////////////////////////////////////////////////////////////////////////
/*
_PCB215_Error_Enum GetFwRevision(void)
        La funzione legge il codice di revisione del firmware del 
        target.

PARAM:
        -
RETURN:
      TRUE: Lettura avvenuta con successo
      FALSE: Problema di comunicazione con il target

      STATUS.maj_code/STATUS.min_code = codice revisione
Autore: M. Rispoli
Data: 17/09/2014
*/
//////////////////////////////////////////////////////////////////////////////
bool GetFwRevision(void)
{
_Ser422_Command_Str frame;

  
  frame.address = TARGET_ADDRESS;
  frame.attempt = 4;
  frame.cmd=SER422_COMMAND;
  
  // Scrive il codice comando
  frame.data1=_CMD1(PCB215_GET_FWREV);
  frame.data2=_CMD2(PCB215_GET_FWREV);
  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
  
  // Eventuali errori di comunicazione
  if(frame.retcode!=SER422_DATA) return FALSE;

  STATUS.maj_code = frame.data2;
  STATUS.min_code = frame.data1;
  generalConfiguration.revisioni.pcb269.maj = STATUS.maj_code;
  generalConfiguration.revisioni.pcb269.min = STATUS.min_code;
    

  return TRUE;
   
}

//////////////////////////////////////////////////////////////////////////////
/*
bool pcb215SetSblocco()
        La funzione permette di attivare il carrello compressore
        fino ad una posizione di decompressione.

        La funzione prima si accerta di portare il sistema in IDLE
        (precondizione per l'accetazione del comando) e poi chiama 
        il comando di attivazione dello sblocco.

        Se il sistema si trova già in posizione di sblocco(non compressione)
        il comando viene ignorato.
        
        L'applicazione deve monitorare il flag STATUS.compression
        per sapere quando l'operazione è terminata.

        La funzione si blocca fino alla ricezione del Command Ok dal target
        e fino al riconoscimento del comando in corso.
          
PARAM:
        -
RETURN:
      TRUE: Comando attivato
      FALSE: Comando fallito

VARIABILI DI STATO:
      STATUS.compression: stato della compressione
      STATUS.idle: se 1, comando eseguito 


Autore: M. Rispoli
Data: 18/09/2014
*/
//////////////////////////////////////////////////////////////////////////////
bool pcb215SetSblocco()
{
  _Ser422_Command_Str frame;

  // Sospende il driver bloccando la mutex del polling
  // Il driver si blocca esattamente dopo aver letto i registri di stato
  _mutex_lock(&(CONTEST.pollinglist_mutex));

  // Verifica che non sia già sbloccato o in sblocco ..
  if((!_TEST_BIT(PCB215_COMPRESSION))||(_TEST_BIT(PCB215_SBLOCCO)))
  {
    _mutex_unlock(&(CONTEST.pollinglist_mutex));
    return TRUE;
  }
  
  // Prepara il comando di sblocco
  frame.address = TARGET_ADDRESS;
  frame.attempt = 4;
  frame.cmd=SER422_COMMAND;

  // Verifica se si trova in IDLE
  if(!_TEST_BIT(PCB215_IDLE))
  {
    // Invia il comando di IDLE come condizione necessaria
    frame.data1=_CMD1(PCB215_SET_IDLE);
    frame.data2=_CMD2(PCB215_SET_IDLE);
    Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
    if(frame.retcode!=SER422_COMMAND_OK) 
    {
      _mutex_unlock(&(CONTEST.pollinglist_mutex));
      return FALSE;
    }
  }
  
  // Invia il comando di SBLOCCO che ora deve essere accettato 
  frame.data1=_CMD1(PCB215_SET_SBLOCCO);
  frame.data2=_CMD2(PCB215_SET_SBLOCCO);
  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
  if(frame.retcode!=SER422_COMMAND_OK) 
  {
    _mutex_unlock(&(CONTEST.pollinglist_mutex));
    return FALSE;
  }
 
  // Sblocca il driver ed attende il tempo necessario di 
  // vedere il comando attivo sui registri opportuni
  _mutex_unlock(&(CONTEST.pollinglist_mutex));
  return TRUE;
  
}

// Funzione usata sotto raggi con DRIVER in freeze
// Il sistema DEVE essere in IDLE duraNTE RAGGI..
void pcb215SetXRaySblocco(void)
{
  _Ser422_Command_Str frame;

  // Prepara il comando di sblocco
  frame.address = TARGET_ADDRESS;
  frame.attempt = 4;
  frame.cmd=SER422_COMMAND;
  
  // Invia il comando di SBLOCCO che ora deve essere accettato 
  frame.data1=_CMD1(PCB215_SET_SBLOCCO);
  frame.data2=_CMD2(PCB215_SET_SBLOCCO);
  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
  return ;  
}

//////////////////////////////////////////////////////////////////////////////
/*
bool pcb215MovePadUpward(unsigned char mm)
        
        La funzione permette di attivare il carrello compressore
        fino ad una posizione di x mm verso l'alto;

        Il Comando viene accettato solo se il sistema si trova in IDLE;
        
        Un valore di mm==0 effettua un singolo step verso l'alto;
        Un valore di mm==255 muove il carrello fino al limite superiore;
        Un valore intermedio muove il carrello di tot mm verso l'alto
          
PARAM:
        - mm: millimetri di spostamento verso l'alto (vedi sopra) 
RETURN:
      TRUE: Comando attivato
      FALSE: Comando fallito o errore di comunicazione
            - STATUS.fault==0 -> TARGET BUSY
            - STATUS.fault==1 
                * STATUS.error==:
                    _SER422_COMMUNICATION_ERROR: (errori di comunicazione)
                    _SER422_TARGET_ERROR: (comando illegale)

NOTE:
      In caso di comando fallito l'applicazione deve controllare
      lo stato del flag STATUS.fault e STATUS.error per identificare
      la motivazione del fallimento del comando.

Autore: M. Rispoli
Data: 25/09/2014
*/
//////////////////////////////////////////////////////////////////////////////
bool pcb215MovePadUpward(unsigned char mm)
{
  _Ser422_Command_Str frame;

  // Sospende il driver bloccando la mutex del polling
  // Il driver si blocca esattamente dopo aver letto i registri di stato
  _mutex_lock(&(CONTEST.pollinglist_mutex));
  if((!_TEST_BIT(PCB215_IDLE)))
  {// Solo in IDLE viene accettato il comando
    _mutex_unlock(&(CONTEST.pollinglist_mutex));
    return FALSE;
  }
 
  // Prepara il comando di sblocco
  frame.address = TARGET_ADDRESS;
  frame.attempt = 4;
  frame.cmd=SER422_COMMAND;
  frame.data1=_CMD1(PCB215_MOVE_UP);
  frame.data2=mm; // Imposta i millimetri passati come parametro
  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
  _mutex_unlock(&(CONTEST.pollinglist_mutex));
  switch(frame.retcode)
  {
  case SER422_COMMAND_OK: return TRUE;  //Comando eseguito con successo
  case SER422_BUSY: return FALSE;       // Condizione di busy, sebbene inaspettata!
    
  // Condizioni di errore più gravi
  case  SER422_ILLEGAL_ADDRESS:
  case  SER422_ILLEGAL_DATA:
  case  SER422_ILLEGAL_FUNCTION:
  case  SER422_UNIMPLEMENTED_FUNCTION:
  case  SER422_WAIT_FOR_CONFIRMATION:
    STATUS.fault=1;
    STATUS.error=_SER422_TARGET_ERROR;
    return FALSE;
    break;
  default:
    STATUS.fault=1;
    STATUS.error=_SER422_COMMUNICATION_ERROR;
    return FALSE;
    break;
  }
 
}



//////////////////////////////////////////////////////////////////////////////
/*
bool pcb215MovePadDownward(unsigned char mode, bool force)
        La funzione permette di attivare il carrello in basso
        per effettuare uno step simile ad uno step dell'encoder
        oppure di attivare l amodalità AUTOMATICA portando 
        il carrello compressore a raggiungere uno dei possibili target
        disponibili. 

        Il Comando viene accettato solo se il sistema si trova in IDLE;
        e se abilitato dal profilo corrente.

          
PARAM:
      - mode: modalità di attivazione:
        Un valore di mode==0 effettua un singolo step verso il basso;
        Un valore di mode!=0 attiva il modo automatico
      - force: TRUE, forza IDLE ed esegue il comando
 
RETURN:
      TRUE: Comando attivato
      FALSE: Comando fallito o errore di comunicazione
            - STATUS.fault==0 -> TARGET BUSY/FUNZIONE NON ABILITATA
            - STATUS.fault==1 
                * STATUS.error==:
                    _SER422_COMMUNICATION_ERROR: (errori di comunicazione)
                    _SER422_TARGET_ERROR: (comando illegale)

NOTE:
      In caso di comando fallito l'applicazione deve controllare
      lo stato del flag STATUS.fault e STATUS.error per identificare
      la motivazione del fallimento del comando.

Autore: M. Rispoli
Data: 25/09/2014
*/
//////////////////////////////////////////////////////////////////////////////
bool pcb215MovePadDownward(unsigned char mode, bool force)
{
  _Ser422_Command_Str frame;

  // Sospende il driver bloccando la mutex del polling
  // Il driver si blocca esattamente dopo aver letto i registri di stato
 if(!force)
 {
  _mutex_lock(&(CONTEST.pollinglist_mutex));
  if((!_TEST_BIT(PCB215_IDLE)))
  {// Solo in IDLE viene accettato il comando
    printf("NON IDLE MODE!");
    _mutex_unlock(&(CONTEST.pollinglist_mutex));
    return FALSE;
  }
 }else
 {
      // Verifica se si trova in IDLE
     _mutex_lock(&(CONTEST.pollinglist_mutex));
      if(!_TEST_BIT(PCB215_IDLE))
      {
        // Invia il comando di IDLE come condizione necessaria
        frame.address = TARGET_ADDRESS;
        frame.attempt = 4;
        frame.cmd=SER422_COMMAND;
        frame.data1=_CMD1(PCB215_SET_IDLE);
        frame.data2=_CMD2(PCB215_SET_IDLE);
        Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
        if(frame.retcode!=SER422_COMMAND_OK) 
        {
          printf("SET IDLE FALLITO!:(error %d)  buf1:%d, buf2:%d --",frame.retcode,frame.data1,frame.data2);
          _mutex_unlock(&(CONTEST.pollinglist_mutex));
          return FALSE;
        }
      }
  }
  
  // Prepara il comando da eseguire
  frame.address = TARGET_ADDRESS;
  frame.attempt = 4;
  frame.cmd=SER422_COMMAND;
  frame.data1=_CMD1(PCB215_MOVE_DWN);
  frame.data2=mode; // Imposta la modalità di funzionamento
  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
  _mutex_unlock(&(CONTEST.pollinglist_mutex));
  switch(frame.retcode)
  {
  case SER422_COMMAND_OK: return TRUE;  //Comando eseguito con successo
  case SER422_BUSY: return FALSE;       // Condizione di busy
  case  SER422_ILLEGAL_FUNCTION: return FALSE; // Funzione non abilitata dal profilo
    
  // Condizioni di errore più gravi
  case  SER422_ILLEGAL_ADDRESS:
  case  SER422_ILLEGAL_DATA:
  case  SER422_UNIMPLEMENTED_FUNCTION:
  case  SER422_WAIT_FOR_CONFIRMATION:
    STATUS.fault=1;
    STATUS.error=_SER422_TARGET_ERROR;
    return FALSE;
    break;
  default:
    STATUS.fault=1;
    STATUS.error=_SER422_COMMUNICATION_ERROR;
    return FALSE;
    break;
  }
 
}

//////////////////////////////////////////////////////////////////////////////
/*
pcb215SetIdle()
        La funzione forza il carrello compressore a 
        fermarsi in IDLE mode.

        L'unica limitazione di questa funzione 
        è che non può essere usata durante lo startup del carrello.

PARAM:
        -
RETURN:
      TRUE: Comando attivato
      FALSE: Comando fallito


Autore: M. Rispoli
Data: 25/09/2014
*/
//////////////////////////////////////////////////////////////////////////////
bool pcb215SetIdle()
{
  _Ser422_Command_Str frame;

  // Sospende il driver bloccando la mutex del polling
  // Il driver si blocca esattamente dopo aver letto i registri di stato
  _mutex_lock(&(CONTEST.pollinglist_mutex));

  // In IDLE esce subito perchè il motore è fermo
  if(_TEST_BIT(PCB215_IDLE))
  {
      _mutex_unlock(&(CONTEST.pollinglist_mutex));
      return TRUE;
  }
  
  // Prepara il comando di sblocco
  frame.address = TARGET_ADDRESS;
  frame.attempt = 10;
  frame.cmd=SER422_COMMAND;
  frame.data1=_CMD1(PCB215_SET_IDLE);
  frame.data2=_CMD2(PCB215_SET_IDLE);
  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
  _mutex_unlock(&(CONTEST.pollinglist_mutex));
  switch(frame.retcode)
  {
  case SER422_COMMAND_OK: return TRUE;  //Comando eseguito con successo
  case SER422_ILLEGAL_FUNCTION: return FALSE; // Funzione non abilitata dal profilo
    
  // Condizioni di errore più gravi
  case  SER422_ILLEGAL_ADDRESS:
  case  SER422_ILLEGAL_DATA:
  case  SER422_UNIMPLEMENTED_FUNCTION:
  case  SER422_WAIT_FOR_CONFIRMATION:
  case SER422_BUSY:
    STATUS.fault=1;
    STATUS.error=_SER422_TARGET_ERROR;
    return FALSE;
    break;
  default:
    STATUS.fault=1;
    STATUS.error=_SER422_COMMUNICATION_ERROR;
    return FALSE;
    break;
  }

  
}
extern bool pcb249U2LampCmd(unsigned char cmd, unsigned char tmo);
// Effettua l'update dei registri di sistema
bool pcb215UpdateRegisters(void)
{
  static bool alternate=FALSE;
  static bool fault_pedals=false;
  static int  timer_fault_pedals = 0;
  static bool luce_centratore=false;
  static int timer_luce_centratore;
  unsigned char data;
  
  // Legge i seguenti registri in maniera alternata
  if(alternate) 
  {// ______________________________ SLOT 1 ____________________________________________________
      // Legge i registri di Base che devono sempre essere aggiornati
      if(Ser422ReadRegister(_REGID(RG215_FLAGS0),4,&CONTEST)!=_SER422_NO_ERROR)  return FALSE;
      if(Ser422ReadRegister(_REGID(RG215_FLAGS1),4,&CONTEST)!=_SER422_NO_ERROR)  return FALSE;
      if(Ser422ReadRegister(_REGID(RG215_PEDALS),4,&CONTEST)!=_SER422_NO_ERROR)  return FALSE;
      if(_TEST_BIT(PCB215_IDLE))  _EVSET(_EV0_PCB215_IDLE);

      // VErifica se attivare la luce del centratore
      if(_DEVREGL(RG215_FLAGS1,CONTEST) & 0x80){
          if(!luce_centratore){
              // La luce del centratore deve essere attivata
              printf("PCB215: LUCE CENTRATORE\n");
              luce_centratore = pcb249U2LampCmd(3,20);
              timer_luce_centratore=10;
          } else{
              if(timer_luce_centratore) timer_luce_centratore--;
              if(!timer_luce_centratore) luce_centratore=false; // refresh
          }

      }else luce_centratore = false;

      // Diagnostica sui pedali: misura il timeout sulla pressione dei pedali
      if(COMP_PEDALS){
          if(!fault_pedals){
              if(timer_fault_pedals>30){   // Set time
                  timer_fault_pedals = 5; // Reset time
                  fault_pedals=true;
                  _mutex_lock(&output_mutex);
                  SystemOutputs.CPU_COMPRESSOR_ENA = 0;
                  _EVSET(_EV0_OUTPUT_CAMBIATI);
                  _mutex_unlock(&output_mutex);
                  data=PCB215_ERROR_PEDALS_STARTUP;
                  while(mccPCB215Notify(1,PCB215_NOTIFY_ERRORS,&data,1)==false) _time_delay(200);
                  printf("ERRORE PEDALI COMPRESSORE\n");
              }else timer_fault_pedals++;
              printf("timer:%d\n",timer_fault_pedals);
          }else timer_fault_pedals = 10; // Reset time
      }else{
          if(fault_pedals){
              if(timer_fault_pedals==0){
                  printf("RESET FAULT PEDALI\n");
                  fault_pedals=false;
                  _mutex_lock(&output_mutex);
                  SystemOutputs.CPU_COMPRESSOR_ENA = 1;
                  _EVSET(_EV0_OUTPUT_CAMBIATI);
                  _mutex_unlock(&output_mutex);
                  data=PCB215_NO_ERRORS;
                  while(mccPCB215Notify(1,PCB215_NOTIFY_ERRORS,&data,1)==false) _time_delay(200);
              }else timer_fault_pedals--;
              printf("timer:%d\n",timer_fault_pedals);
          }else timer_fault_pedals = 0;
      }


      // Se c'è un qualsiasi movimento in corso la compressione viene disabilitata
      
  }else 
  {//________________________________ SLOT 2 ____________________________________________________
      // Legge il registro per il riconoscimento del PAD 
      if(Ser422ReadRegister(_REGID(RG215_RAW_PADDLE),4,&CONTEST)!=_SER422_NO_ERROR) return FALSE;
      if(Ser422ReadRegister(_REGID(RG215_DOSE),4,&CONTEST)!=_SER422_NO_ERROR)  return FALSE;
      if((_TEST_BIT(PCB215_COMPRESSION))&&(IS_VALID_PAD))
      {
        // In compressione aggiorna il livello di comnpressione e lo spessore
        _EVSET(_EV0_PCB215_COMPRESSION);
        generalConfiguration.isInCompression = true;
        //printf("in compressione: pad=%d\n",generalConfiguration.comprCfg.padSelezionato);
        //if(IS_VALID_PAD)
        if(Ser422ReadRegister(_REGID(RG215_STRENGTH),4,&CONTEST)!=_SER422_NO_ERROR) return FALSE;      

        
      }
      else 
      {
        generalConfiguration.isInCompression = false;
        _EVSET(_EV0_PCB215_FREE);

        // Fuori dalla compressione legge la compressione Target (da visualizzare solo quando non in compressione)
        if(Ser422ReadRegister(_REGID(COMPRESSION_TARGET),4,&CONTEST)!=_SER422_NO_ERROR)  return FALSE;

      }

      actuatorsManageEnables();
      pcb215VerifyComprData();
  }
  
  alternate = ! alternate;  
  return TRUE;
}


// Effettua l'update dei registri di sistema
void pcb215CalibMode(void)
{
  static int count=5;
  static int angolo_braccio=255;
  static unsigned char tara=0;
  static unsigned short spessore=0;
  unsigned char data[12];
  float fval;
  
  Ser422ReadRegister(_REGID(RG215_FLAGS0),4,&CONTEST);
  Ser422ReadRegister(_REGID(RG215_FLAGS1),4,&CONTEST);
  Ser422ReadRegister(_REGID(RG215_RAW_DOSE),4,&CONTEST);
  Ser422ReadRegister(_REGID(RG215_DOSE),4,&CONTEST);
  Ser422ReadRegister(_REGID(RG215_RAW_STRENGHT),4,&CONTEST);
  Ser422ReadRegister(_REGID(RG215_STRENGTH),4,&CONTEST);
  Ser422ReadRegister(_REGID(RG215_RAW_PADDLE),4,&CONTEST);
   
  classifyPad();
  
  data[0] = _DEVREGL(RG215_FLAGS0,CONTEST);
  data[1] = _DEVREGL(RG215_FLAGS1,CONTEST);
  data[2] = _DEVREGL(RG215_RAW_DOSE,CONTEST);
  data[3] = _DEVREGH(RG215_RAW_DOSE,CONTEST);
  data[4] = _DEVREGL(RG215_RAW_STRENGHT,CONTEST);
  data[5] = padLevel; // generalConfiguration.comprCfg.padSelezionato; 
  data[6] = _DEVREGL(RG215_STRENGTH,CONTEST); 
  data[7] = _DEVREGL(RG215_DOSE,CONTEST);
  data[8] = _DEVREGH(RG215_DOSE,CONTEST);
  data[9] = _DEVREGL(RG215_RAW_PADDLE,CONTEST);//(unsigned char) spessore&0xFF;
  // data[10] = (unsigned char) ((spessore>>8)&0xFF);
  // data[11] = tara;
  
  if(--count) count = 5;
  
  
  mccPCB215Notify(1,PCB215_NOTIFY_CALIB_DATA,data,sizeof(data));
  return;
}


    
// Verifica se è il caso di notificare
void pcb215VerifyComprData(void)
{
  static unsigned char count = 50; // Polling notifiche oltre i cambi stato
  static unsigned char strenght = 255;
  static unsigned char thickL = 0xFF;
  static unsigned char thickH = 0xFF;
  static unsigned char flags0=0;
  static unsigned char flags1=0;
  static unsigned char pad=255;
  static bool protezionePaziente=FALSE;

  // Calcolo dello spessore medio
  static int mean_spessore=0;
  static int spessore_cnt=0;
  static int spessore=0;

  static unsigned char target_compressione=0;
  static bool modo_zero = FALSE;

  // Gestione cambio angolo braccio: occorre sempre una fase di init
  // imponendo l'angolo ad un valore "impossibile"
  static int angolo_braccio=255;

  // Tara e soglia di compressione 
  static unsigned char backup_tara=255;
  static unsigned char backup_soglia_compressione=255;
  static unsigned short biopsyZ=0; 
  
  unsigned char tara, soglia_compressione;
  unsigned short padLimitPosition;
  int offset;
  float fval;
  bool notifyGui=FALSE;
  bool updateLimitPos=FALSE;
  unsigned short usval;
  unsigned char i;
  unsigned char data[PCB215_NOTIFY_COMPR_DATA_LEN+1];
  
  // Non procede fino a che la configurazione non è giunta
  if(generalConfiguration.deviceConfigOk==FALSE) return;

  // In caso di reset del driver o init del driver
  // vengono inizializzate tutte le variabili statiche per non perdere
  // eventuali cambi di stato in caso di reset / errore del driver stesso
  if(verifyComprDataInit==TRUE)
  {
    verifyComprDataInit = FALSE;
    strenght = 255;
    thickL = 0xFF;
    thickH = 0xFF;
    flags0=0;
    flags1=0;
    pad=255;
    protezionePaziente=FALSE;

    // Calcolo dello spessore medio
    mean_spessore=0;
    spessore_cnt=0;
    spessore=0;

    target_compressione=0;
    modo_zero = FALSE;

    // Gestione cambio angolo braccio: occorre sempre una fase di init
    // imponendo l'angolo ad un valore "impossibile"
    angolo_braccio=255;

    // Tara e soglia di compressione 
    backup_tara=255;
    backup_soglia_compressione=255;
    biopsyZ=255; 
  }
  
  // Carica i Flags
  data[COMPRESSORE_FLAG0] = _DEVREGL(RG215_FLAGS0,CONTEST);
  data[COMPRESSORE_FLAG1] = _DEVREGL(RG215_FLAGS1,CONTEST);
  data[COMPRESSORE_FORZA] = _DEVREGL(RG215_STRENGTH,CONTEST);
  data[COMPRESSORE_TARGET] = _DEVREGL(COMPRESSION_TARGET,CONTEST);
  data[COMPRESSORE_POSL] =  _DEVREGL(RG215_DOSE,CONTEST);
  data[COMPRESSORE_POSH] =  _DEVREGH(RG215_DOSE,CONTEST);

  if(classifyPad()) // Determina il PAD rilevato dalla PCB215  
  {
    printf("RILEVATO CAMBIO PAD:%d\n", generalConfiguration.comprCfg.padSelezionato);
    
    // Se il Pad è cambiato ..
    notifyGui = TRUE; // Effettua la notifica dei dati all'applicazione
    updateLimitPos = TRUE;
    
    // Scrive il registro di compensazione della forza
    if(IS_VALID_PAD) 
    {
      Ser422WriteRegister(_REGID(COMPRESSOR_STR_K),PAD.kF,4,&CONTEST);
      printf("NUOVO PAD: kF = %d\n",PAD.kF);
    }
    if(!IS_VALID_PAD)  
    {
      // In caso di PAD non valido si aggiornano i dati di gestione del compressore
      soglia_compressione = THRESHOLD_NO_PAD;
      tara=0;
      Ser422WriteRegister(_REGID(POSITION_PAD_TARA), tara,4,&CONTEST);      
      Ser422WriteRegister(_REGID(COMPRESSION_THRESHOLD_H), soglia_compressione,4,&CONTEST);
      Ser422WriteRegister(_REGID(COMPRESSION_THRESHOLD_L), soglia_compressione-10,4,&CONTEST);
      printf("TARA:%d\n",tara);
      printf("SOGLIA DI COMPRESSIONE:%d\n",soglia_compressione);
    }   
    
  } //// CAMBIO PAD

  // GESTIONE SOGLIA DI COMPRESSIONE E TARA COMPRESSORE
  // Verifica della condizione di angolo ribaltato per rinforzare la soglia
  // di compressione. L'angolo di soglia è 90°. Il controllo viene effettuato
  // solo se c'è un PAD valido altrementi non viene modificata la tara
  // e viene impostata una soglia di compressione molto bassa per agevolare
  // la modalità di Compressione Zero
  int arm = generalConfiguration.armExecution.dAngolo / 10;
  if(((angolo_braccio>arm+1)||(angolo_braccio<arm-1))&& (IS_VALID_PAD) )
  {
    angolo_braccio = arm;
    int absarm=angolo_braccio;
    if(absarm<0) absarm=-absarm;


    // Regola la soglia di compressione per angoli superiori a 50°
    if(absarm>THRESHOLD_ANG)  soglia_compressione = THRESHOLD_INCL;
    else soglia_compressione = THRESHOLD_CC;

    if(soglia_compressione!=backup_soglia_compressione)
    {
      printf("SOGLIA DI COMPRESSIONE:%d\n",soglia_compressione);      
      backup_soglia_compressione = soglia_compressione;
    }

    // Riscrivere sempre perchè il registro viene riscritto in altre sezioni di codice
    Ser422WriteRegister(_REGID(COMPRESSION_THRESHOLD_H), soglia_compressione,4,&CONTEST);
    Ser422WriteRegister(_REGID(COMPRESSION_THRESHOLD_L), soglia_compressione-10,4,&CONTEST);

    // Regola la Tara per angoli superiori a 90°
    if(absarm>90)
    {
      // Con il Pad presente calcola la componente della forza peso
      fval = -1* cos(absarm*3.1415/180) * PAD.peso;
      tara = (unsigned char) fval;
    }else tara = 0;
    
    if(tara!=backup_tara)
    {
      printf("TARA:%d\n",tara);
      backup_tara = tara;
    }
 
    // Riscrivere sempre perchè il registro viene riscritto in altre sezioni di codice
    Ser422WriteRegister(_REGID(POSITION_PAD_TARA), tara,4,&CONTEST); 
 
  }

  // Verifica se è cambiato lo stato della presenza della protezione paziente
  if ( (generalConfiguration.colliCfg.codiceAccessorio == COLLI_ACCESSORIO_PROTEZIONE_PAZIENTE_3D)||
       (generalConfiguration.colliCfg.codiceAccessorio == COLLI_ACCESSORIO_PROTEZIONE_PAZIENTE_2D)
     ) CONFIG.protezionePaziente = TRUE;
  else CONFIG.protezionePaziente = FALSE;

  if(protezionePaziente != CONFIG.protezionePaziente)
  {
    protezionePaziente = CONFIG.protezionePaziente;
    if(protezionePaziente) printf("RILEVATA PRESENZA PROTEZIONE PAZIENTE 2D/3D");
    else printf("RILEVATA ASSENZA PROTEZIONE PAZIENTE 2D/3D");
    updateLimitPos = TRUE;
  }

  // Verificase se c'è la torretta ed è cambiata la z corrente
  if(generalConfiguration.biopsyCfg.connected==TRUE)
  {
    int dif = biopsyZ - generalConfiguration.biopsyCfg.Z;
    if((dif>=10) || (dif<= -10))
    {
      biopsyZ=generalConfiguration.biopsyCfg.Z;
      updateLimitPos = TRUE;
    }
  }
  
  // Gestione modi di compressione
  if((POTTER==POTTER_UNDEFINED)&&(generalConfiguration.biopsyCfg.connected==FALSE)) // Potter scoperto
  {
    if(!IS_VALID_PAD) Ser422WriteRegister(_REGID(POSITION_LOW_MODO_0), 30,4,&CONTEST);
    else
    {
      // Calcola l'altezza minima in funzione dello sbalzo
      if(PAD.offset>0) offset = 30 ;
      else offset = 30 - PAD.offset;
      Ser422WriteRegister(_REGID(POSITION_LOW_MODO_0), offset,4,&CONTEST);
    }
    
    if(_DEVREG(RG215_FUNC,CONTEST)!=1) printf("ATTIVAZIONE MODO ZERO\n");
    Ser422WriteRegister(_REGID(RG215_FUNC), 1,4,&CONTEST);
    Ser422WriteRegister(_REGID(COMPRESSION_THRESHOLD_H), 15,4,&CONTEST);
    Ser422WriteRegister(_REGID(COMPRESSION_THRESHOLD_L), 5,4,&CONTEST);

  }else if(!IS_VALID_PAD)
  {
    Ser422WriteRegister(_REGID(POSITION_LOW_MODO_0), 30,4,&CONTEST);
    if(_DEVREG(RG215_FUNC,CONTEST)!=1) printf("ATTIVAZIONE MODO ZERO\n");
    Ser422WriteRegister(_REGID(RG215_FUNC), 1,4,&CONTEST);
    Ser422WriteRegister(_REGID(COMPRESSION_THRESHOLD_H), 15,4,&CONTEST);
    Ser422WriteRegister(_REGID(COMPRESSION_THRESHOLD_L), 5,4,&CONTEST);
  }else
  {
     Ser422WriteRegister(_REGID(POSITION_LOW_MODO_0), 0,4,&CONTEST);
    if(_DEVREG(RG215_FUNC,CONTEST)!=0) printf("ATTIVAZIONE COMPRESSIONE NORMALE\n");
    Ser422WriteRegister(_REGID(RG215_FUNC), 0,4,&CONTEST);
  }
  
  data[COMPRESSORE_PAD] = generalConfiguration.comprCfg.padSelezionato;

  // Se è stato richiesto l'aggiornamento della posizione limite, ricalcola ..
  // e aggiorna il dispositivo
  if(updateLimitPos)
  {
      // Bisogna tenere in considerazione che il piano di compressione della fibra del BYM è più alto di 20mm rispetto
      // al piano del potter.
      if(generalConfiguration.biopsyCfg.connected==TRUE)
      {
        padLimitPosition = generalConfiguration.biopsyCfg.conf.offsetPad +  (generalConfiguration.biopsyCfg.conf.offsetFibra + 20) - generalConfiguration.biopsyCfg.conf.margineRisalita - generalConfiguration.biopsyCfg.Z/10;
      }
      else if(IS_VALID_PAD)
      {
        // Calcola la nuova posizione limite della nacchera
        if(CONFIG.protezionePaziente)
          padLimitPosition = CALIBRATION.maxProtection - PAD.offset ;
        else
          padLimitPosition = CALIBRATION.maxPosition - PAD.offset;     
      }else padLimitPosition = CALIBRATION.maxPosition ;        
      
      // Limite fisico escursione carrello
      if(padLimitPosition > CALIBRATION.maxMechPosition) padLimitPosition = CALIBRATION.maxMechPosition;  
            
      Ser422WriteRegister(_REGID(POSITION_LIMIT), padLimitPosition,4,&CONTEST);
      printf("NUOVA POSIZIONE LIMITE NACCHERA:%d\n",padLimitPosition);
  }
  

  // Calcola lo spessore in funzione del PAD/Potter corrente solo quando in compressione e in IDLE
  // Lo spessore viene calcolato effettuando una media di N letture, al termine della quale viene
  // inviato il dato alla GUI per visualizzarne il risultato.
  // La media viene iniziata al termine del posizionamento del compressore.
  // Quando non in compressione o durante il posizionamento il valore dello spessore viene azzerato
  // in quanto non attendibile
  if((IS_VALID_PAD)&&(_TEST_BIT(PCB215_IDLE))&&(_TEST_BIT(PCB215_COMPRESSION)))
  {
      spessore = _DEVREG(RG215_DOSE,CONTEST);
      spessore+= PAD.offset; // Aggiunge l'offset del Pad utilizzato
      if(spessore<1) spessore =1;


      int mag_factor=0;
      int sbalzo=0;

      if(POTTER==POTTER_MAGNIFIER){
          for(int mj=0;mj<8;mj++){
            if(generalConfiguration.comprCfg.calibration.sbalzoIngranditore[mj]==0) continue; // Solo quelli configurati
            if((generalConfiguration.comprCfg.calibration.fattoreIngranditore[mj]!=15)&&(generalConfiguration.comprCfg.calibration.fattoreIngranditore[mj]!=20)) continue; // Limita a 1.5 e 2x
            if((spessore>generalConfiguration.comprCfg.calibration.sbalzoIngranditore[mj]-5)&&(generalConfiguration.comprCfg.calibration.sbalzoIngranditore[mj]>sbalzo)){
                sbalzo = generalConfiguration.comprCfg.calibration.sbalzoIngranditore[mj];
                mag_factor  = mj;
            }
          }

          // Ricalcolo Spessore
          spessore-=sbalzo;
          generalConfiguration.potterCfg.potMagFactor = mag_factor;
          //printf("POTTER MAG FACTOR ANALOGICO = %d\n", mag_factor);
    }

    data[COMPRESSORE_THICKL] = (unsigned char) (spessore &0xFF);
    data[COMPRESSORE_THICKH] = (unsigned char) ((spessore>>8) &0xFF);;
  }else
  {
    generalConfiguration.potterCfg.potMagFactor = 255; // Undefined magnifier factor
    data[COMPRESSORE_THICKL]=0;
    data[COMPRESSORE_THICKH]=0;
    spessore=0;
    mean_spessore=0;
    spessore_cnt = 0;
  }
  
  
  // Verifica Flags0
  if(data[COMPRESSORE_FLAG0]!=flags0)
  {
    notifyGui = TRUE;
    flags0 = data[COMPRESSORE_FLAG0];
  }
  
  // Verifica Flags1
  // Correzione del flag di compressione
  if(!IS_VALID_PAD) data[COMPRESSORE_FLAG1] = data[COMPRESSORE_FLAG1] & 0xFD;
  if(data[COMPRESSORE_FLAG1]!=flags1)
  {
    notifyGui = TRUE;
    flags1 = data[COMPRESSORE_FLAG1];
  }

  // Verifica forza
  if(data[COMPRESSORE_FORZA]!=strenght)
  {
    notifyGui = TRUE;
    strenght = data[COMPRESSORE_FORZA];
  }
 
  if(data[COMPRESSORE_TARGET]!=target_compressione)
  {
    notifyGui = TRUE;
    target_compressione = data[COMPRESSORE_TARGET];
  }

  // Verifica Spessore
  if((thickH!=data[COMPRESSORE_THICKH ])||(thickL!=data[COMPRESSORE_THICKL]))
  {
    notifyGui = TRUE;
    thickH=data[COMPRESSORE_THICKH];
    thickL=data[COMPRESSORE_THICKL];
  }
  
  // Aggiorna comunque ogni tot
  if(!--count)
  {
    notifyGui=TRUE;
    count = 25;
  }
      
  // Notifica il livello individuato
  if(notifyGui) mccPCB215Notify(1,PCB215_NOTIFY_COMPR_DATA,data,sizeof(data));
  
}


// Determina quale PAD è stato riconociuto e imposta in ..selectedPad
// il codice standard assegnato a quel dato compressore. raw viene invece 
// scritto con l'intervallo identificato
// La funzione restituisce TRUE se il PAD è Cambiato
bool classifyPad(void)
{
  unsigned char usval;
  unsigned char i;
  static unsigned char padDetected=200;
  static unsigned char padRawLevel=200;

  if(verifyClassPadInit)
  {
    verifyClassPadInit  = FALSE;
    padDetected=200;
  }
  
  usval=_DEVREGL(RG215_RAW_PADDLE,CONTEST);
  for(i=0; i<10;i++)
    if(usval<generalConfiguration.comprCfg.calibration.thresholds[i]) break;

  if(i!=padLevel)
  {
    padLevel = i;
    printf("CLASSIFICATO N:%d, LIVELLO RAW:%d\n", padLevel, usval);    
  }
  
  // <TBD> Classificazione PAD BIOP_3D
  if(generalConfiguration.biopsyCfg.connected==TRUE)
  {
    if(i==_BIOP_3D_LEVEL)
      generalConfiguration.comprCfg.padSelezionato = PAD_BIOP_3D;
    else
      generalConfiguration.comprCfg.padSelezionato = PAD_ND;
  }
  
  // La classificazione avviene mappando il livello identificato in funzione
  // del tipo di Potter presente su tre famiglie: POTTER,INGRANDITORE,BIOPSIA
  else if(i==_INVALID_PAD_LEVEL) generalConfiguration.comprCfg.padSelezionato = PAD_ND;
  else if(i==_UNLOCK_PAD_LEVEL) generalConfiguration.comprCfg.padSelezionato = PAD_UNLOCKED;
  else if(i==_UNLOCK_COMPR_LEVEL) generalConfiguration.comprCfg.padSelezionato = PAD_UNMOUNTED;
  else if(POTTER==POTTER_UNDEFINED) 
  {
    generalConfiguration.comprCfg.padSelezionato = POTTER_DISCONNECTED;
  }else if((POTTER==POTTER_TOMO)||(POTTER==POTTER_2D)) 
  {
      switch(i)
      {
        case _POTTER_24x30_LEVEL: // 24x30
            generalConfiguration.comprCfg.padSelezionato = PAD_24x30;
            break;
        case _POTTER_18x24_LEVEL: // 18x24 Center
            generalConfiguration.comprCfg.padSelezionato = PAD_18x24;
            break;
        case _POTTER_10x24_LEVEL: // 10x24 SBALZATO
            generalConfiguration.comprCfg.padSelezionato = PAD_10x24;
            break;
        case _POTTER_BP2D_LEVEL: // Biopsia 2D
            generalConfiguration.comprCfg.padSelezionato = PAD_BIOP_2D;
            break;
        case _POTTER_TOMO_LEVEL: // PAD PER TOMO
            generalConfiguration.comprCfg.padSelezionato = PAD_TOMO_24x30;
            break;
        case _POTTER_PROSTHESIS: // 10x24 per protesi
            generalConfiguration.comprCfg.padSelezionato = PAD_PROSTHESIS;
            break;
        case _POTTER_18x24L_LEVEL: // 18x24 Left
            generalConfiguration.comprCfg.padSelezionato = PAD_18x24_LEFT;
            break;
        case _POTTER_18x24R_LEVEL: // 18x24 Right
            generalConfiguration.comprCfg.padSelezionato = PAD_18x24_RIGHT;
            break;
        default:
          generalConfiguration.comprCfg.padSelezionato = PAD_ND;
        break;
      }
  }else if(POTTER==POTTER_MAGNIFIER) 
  {
      switch(i)
      {
        case _MAG_9x21_LEVEL:
            generalConfiguration.comprCfg.padSelezionato = PAD_9x21;
            break;
        case _MAG_D75_LEVEL: // D75 Sbalzato per ingranditore Center
            generalConfiguration.comprCfg.padSelezionato = PAD_D75_MAG;
            break;

      case _MAG_9x9_LEVEL:
          generalConfiguration.comprCfg.padSelezionato = PAD_9x9_MAG;
          break;

      default:
          generalConfiguration.comprCfg.padSelezionato = PAD_ND;
        break;
      }
  }
  
  if(padDetected!=generalConfiguration.comprCfg.padSelezionato)
  {
    padDetected = generalConfiguration.comprCfg.padSelezionato;    
    return TRUE; // Segnala avvenuto cambiamento del PAD
  }
  
  return FALSE;
}


bool pcb215ConfigNacchera(bool wait)
{
   _DeviceAppRegister_Str ConfList;  
   // Aggiorna i dati di calibrazione passati dall'applicazione
  _LISTCREATE(COMPRESSOR_POS_OFS,ConfList,generalConfiguration.comprCfg.calibration.calibPosOfs); 
  _ADDITEMLIST(COMPRESSOR_POS_K,ConfList,generalConfiguration.comprCfg.calibration.calibPosK); 
  Ser422FormatRegListCrc(&ConfList,&CONTEST);
  Ser422SetConfigList(&ConfList,&CONTEST);
  
  if(!wait) return TRUE;
  
  // Attende completamento configurazione
  _EVWAIT_TALL(_EV0_PCB215_CFG_UPD,3000);
  return TRUE;
}



/*
  Configurazione dei parametri di calibrazione della Forza.
  La PCB215 viene aggiornata con i coefficienti caricati 
  nella struttura generale di configurazione del sistema

  La funzione DEVE esere utilizzata con il driver non in FREEZE
  mode. 
*/
bool pcb215ConfigForza(bool wait)
{
   _DeviceAppRegister_Str ConfList;  
   
   // Aggiorna i dati di calibrazione passati dall'applicazione
  _LISTCREATE(COMPRESSOR_F0,ConfList,generalConfiguration.comprCfg.calibration.F0); 
  _ADDITEMLIST(COMPRESSOR_KF0,ConfList,generalConfiguration.comprCfg.calibration.KF0); 
  _ADDITEMLIST(COMPRESSOR_F1,ConfList,generalConfiguration.comprCfg.calibration.F1); 
  _ADDITEMLIST(COMPRESSOR_KF1,ConfList,generalConfiguration.comprCfg.calibration.KF1); 
  
  Ser422FormatRegListCrc(&ConfList,&CONTEST);
  Ser422SetConfigList(&ConfList,&CONTEST);
  
  if(!wait) return TRUE;
  
  // Attende completamento configurazione
  _EVWAIT_TALL(_EV0_PCB215_CFG_UPD,3000);
  return TRUE;
}

/*
  Funzione chiamata dalla GUI per l'attivazione della modalità di calibrazione
*/
void pcb215ActivateCalibMode(bool status){
  if(status==true){
    executeCalibConfig = true; // Appena può effettua il caricamento dei registri per la calibrazione
    generalConfiguration.comprCfg.calibrationMode = TRUE;
  }else{
    executeConfig = true; 
    generalConfiguration.comprCfg.calibrationMode = FALSE;
  }
}

void pcb215ConfigCalibMode(void)
{
  _DeviceAppRegister_Str ConfList;  

  if(generalConfiguration.comprCfg.calibrationMode == false){
    printf("Impossibile configurare PCB215 per la calibrazione.\n");
    printf("Il sistema NON è in modo calibrazione\n");
    return;
  }

  Ser422WriteRegister(_REGID(COMPRESSOR_STR_K), 0,4,&CONTEST);       // Elimina la compensazione della forza
  Ser422WriteRegister(_REGID(POSITION_LIMIT), 0xFFFF,4,&CONTEST);    // Posizione nacchera al massimo
  Ser422WriteRegister(_REGID(COMPRESSION_LIMIT), 255,4,&CONTEST);    // Compressione limite al massimo
  Ser422WriteRegister(_REGID(COMPRESSION_THRESHOLD_H), 15,4,&CONTEST); // Compressione soglia bassa per movimenti lenti
  Ser422WriteRegister(_REGID(COMPRESSION_THRESHOLD_L), 5,4,&CONTEST); // Compressione soglia bassa per movimenti lenti
  Ser422WriteRegister(_REGID(POSITION_LOW_MODO_0), 0,4,&CONTEST);    // Consentire di arrivare a posizione zero
  Ser422WriteRegister(_REGID(RG215_FUNC), 0,4,&CONTEST);             // Disattivazione modo zero
  Ser422WriteRegister(_REGID(POSITION_PAD_TARA), 0,4,&CONTEST);      // Annulla la tara
  
  return ;
}



// Forza l'update dei dati
void pcb215ForceUpdateData(void)
{
  verifyComprDataInit = TRUE;
  verifyClassPadInit = TRUE;
  return;
}


void pcb215PrintConfig(void){
  printf("CONFIGURAZIONE PCB215:---------------------------------\n");
  printf("POS-K =%d\n", generalConfiguration.comprCfg.calibration.calibPosK);
  printf("POS-OFS =%d\n", generalConfiguration.comprCfg.calibration.calibPosOfs);
  
  printf("\n");
  for(int i=0; i< PAD_ENUM_SIZE; i++){    
    printf("PAD-%d, OFFSET:%d, KF:%d, PESO:%d\n",i, generalConfiguration.comprCfg.calibration.pads[i].offset,generalConfiguration.comprCfg.calibration.pads[i].kF,generalConfiguration.comprCfg.calibration.pads[i].peso);
  }

  printf("\nF0 =%d\n", generalConfiguration.comprCfg.calibration.F0);
  printf("KF0 =%d\n", generalConfiguration.comprCfg.calibration.KF0);
  printf("F1 =%d\n", generalConfiguration.comprCfg.calibration.F1);
  printf("KF1 =%d\n", generalConfiguration.comprCfg.calibration.KF1);
  
  printf("MAX MECH =%d\n", generalConfiguration.comprCfg.calibration.maxMechPosition);
  printf("MAX POS =%d\n", generalConfiguration.comprCfg.calibration.maxPosition);
  printf("MAX PROT =%d\n", generalConfiguration.comprCfg.calibration.maxProtection);
  
  for(int i=0; i< 8; i++){    
    printf("INGRANDITORE-%d, SBALZO:%d, FATTORE:%d\n",i, generalConfiguration.comprCfg.calibration.sbalzoIngranditore[i],generalConfiguration.comprCfg.calibration.fattoreIngranditore[i]);
  }

  // Soglie di riconoscimento pad
  for(int i=0; i< 10; i++){
    printf("THRESHOLD[%d]=%d\n",i, generalConfiguration.comprCfg.calibration.thresholds[i]);
  }

  printf("---------------------------------------------------\n");

  
}
/*
Funzione configuratrice:
  
*/
bool config_pcb215(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len){
  
   
  // Salva nella struttura locale i dati
  if(setmem){
    if(len!=sizeof(compressoreCnf_Str))
    {
      printf("PCB215 CONFIG FALLITA PER LEN: RIC=%d CUR=%d\n",len,sizeof(compressoreCnf_Str));
      return false;    
    }
    memcpy((unsigned char*)&(generalConfiguration.comprCfg.calibration), (compressoreCnf_Str*) buffer, sizeof(compressoreCnf_Str));
    pcb215PrintConfig();    
  }

  // Forza l'uscita dal modo calibrazione se necessario
  generalConfiguration.comprCfg.calibrationMode = FALSE;
  
  // Aggiorna i dati di calibrazione passati dall'applicazione
  if(Ser422WriteRegister(_REGID(COMPRESSOR_POS_OFS), generalConfiguration.comprCfg.calibration.calibPosOfs,10,&CONTEST)!=_SER422_NO_ERROR) return false;
  if(Ser422WriteRegister(_REGID(COMPRESSOR_POS_K), generalConfiguration.comprCfg.calibration.calibPosK,10,&CONTEST)!=_SER422_NO_ERROR) return false;
  if(Ser422WriteRegister(_REGID(COMPRESSOR_STR_K), 0,10,&CONTEST)!=_SER422_NO_ERROR) return false;

  if(Ser422WriteRegister(_REGID(COMPRESSOR_F0), generalConfiguration.comprCfg.calibration.F0,10,&CONTEST)!=_SER422_NO_ERROR) return false;
  if(Ser422WriteRegister(_REGID(COMPRESSOR_KF0), generalConfiguration.comprCfg.calibration.KF0,10,&CONTEST)!=_SER422_NO_ERROR) return false;
  if(Ser422WriteRegister(_REGID(COMPRESSOR_F1), generalConfiguration.comprCfg.calibration.F1,10,&CONTEST)!=_SER422_NO_ERROR) return false;
  if(Ser422WriteRegister(_REGID(COMPRESSOR_KF1), generalConfiguration.comprCfg.calibration.KF1,10,&CONTEST)!=_SER422_NO_ERROR) return false;
  if(Ser422WriteRegister(_REGID(POSITION_PAD_TARA), 0,10,&CONTEST)!=_SER422_NO_ERROR) return false;
  if(Ser422WriteRegister(_REGID(POSITION_LOW_MODO_0), 30,10,&CONTEST)!=_SER422_NO_ERROR) return false;
  if(Ser422WriteRegister(_REGID(COMPRESSION_LIMIT), 200,10,&CONTEST)!=_SER422_NO_ERROR) return false;

  return true;
}


bool pcb215ResetFault(void)
{
  _Ser422_Command_Str frame;

  // Sospende il driver bloccando la mutex del polling
  // Il driver si blocca esattamente dopo aver letto i registri di stato
  _mutex_lock(&(CONTEST.pollinglist_mutex));
  
  // Prepara il comando 
  frame.address = TARGET_ADDRESS;
  frame.attempt = 10;
  frame.cmd=SER422_COMMAND;
  frame.data1=_CMD1(PCB215_RST_FAULTS);
  frame.data2=_CMD2(PCB215_RST_FAULTS);

  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
  _mutex_unlock(&(CONTEST.pollinglist_mutex));
  
  if(frame.retcode==SER422_COMMAND_OK) return TRUE; 
  else
  {
    printf("RETCODE:%d\n",frame.retcode);
  }
  return FALSE;
  
}


bool pcb215ResetBoard(void)
{
  _Ser422_Command_Str frame;

  // Sospende il driver bloccando la mutex del polling
  // Il driver si blocca esattamente dopo aver letto i registri di stato
  _mutex_lock(&(CONTEST.pollinglist_mutex));
  
  // Prepara il comando 
  frame.address = TARGET_ADDRESS;
  frame.attempt = 10;
  frame.cmd=SER422_COMMAND;
  frame.data1=_CMD1(PCB215_RESET_BOARD);
  frame.data2=_CMD2(PCB215_RESET_BOARD);

  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
  _mutex_unlock(&(CONTEST.pollinglist_mutex));
  
  if(frame.retcode==SER422_COMMAND_OK) return TRUE; 
  else
  {
    printf("RETCODE:%d\n",frame.retcode);
  }
  return FALSE;
  
}

void enterFreezeMode(void){
  
    printf("PB215 ENTRA IN FREEZE\n");
    _EVCLR(_EV1_PCB215_RUN);
    _EVSET(_EV1_PCB215_FREEZED); // Notifica l'avvenuto Blocco
    _EVWAIT_ANY(_MOR2(_EV1_DEVICES_RUN,_EV1_PCB215_RUN)); // Attende lo sblocco
    printf("PB215 ESCE DAL FREEZE\n");
    STATUS.freeze = 0;
}

void ERROR_HANDLER(void)
{
   // Segnalazione driver disconnesso
   _EVCLR(_EV1_PCB215_CONNECTED);

   // Riconfigurazione del driver a seguito della ripartenza
   printf("PCB215 ERRORE: ATTESA RICONNESSIONE E RICONFIGURAZIONE REGISTRI\n"); 
  
   while(1){
    
    _time_delay(100);
    
    // Richiesta revisione firmware a target
    while(GetFwRevision()==FALSE) _time_delay(100);
    printf("PCB215:REVISIONE FW TARGET:%d.%d\n",STATUS.maj_code,STATUS.min_code);     

    // Carica sulla periferica lo stato dei registri cosi come erano prima del reset
    printf("PCB215: DOWNLOAD REGISTRI ...\n");
    if(Ser422UploadRegisters(10, &CONTEST)== FALSE)   continue;  
   
    // Carica Tutti i registri RD / RW
    int i;
    for(i=0;i<PCB215_NREGISTERS;i++) 
    {
      if(Ser422ReadRegister(i,4,&CONTEST)!=_SER422_NO_ERROR) break; 
    }
    if(i!=PCB215_NREGISTERS) continue;  
    break;
  }

  // Riaggiorna tutti gli eventi che lavorano sul cambio stato
  verifyComprDataInit = TRUE;   
  verifyClassPadInit  = TRUE;

  // Invia comando di reset faults al target e reitera fino a risposta
  pcb215ResetFault();

  // Segnalazione driver connesso
  _EVSET(_EV1_PCB215_CONNECTED);
  
  // Ripartenza completata. Può tornare da dove aveva lasciato
  printf("PCB215 RIPARTITA CORRETTAMENTE\n"); 

  return;
}

//____________________________________________________________
/*   pcb215GetSpessoreNonCompresso()
 *
 *   Questa funzione restituisce la posizione del
 *   piano compressore rispetto al piano di compressione
 *   sulla base della posizione del carrello e del PAD montato.
 *   Nel caso di ingranditore, la posizione viene ricalcolata
 *   rispetto al piano di compressione innalzato in funzione
 *   del grado di ingrandimento impostato.
 *   La funzione rilegge il registro posizione della pCB215
______________________________________________________________ */
int pcb215GetSpessoreNonCompresso(void){

    int spessore = 0;

    if(!IS_VALID_PAD) return 0;

    // Se fallisce la lettura restituisce 0
    if(Ser422ReadRegister(_REGID(RG215_DOSE),10,&CONTEST)!=_SER422_NO_ERROR)  return 0;

    spessore = _DEVREG(RG215_DOSE,CONTEST);

    printf("POSIZIONE CARRELLO:%d\n", spessore);

    // Verifica se c'è inserito l'ingranditore
    if(POTTER==POTTER_MAGNIFIER){
        printf("OFFSET INGRANDIMENTO:%d\n", INGRANDIMENTO);
        spessore-=INGRANDIMENTO;
    }else{
        printf("OFFSET PAD:%d\n", PAD.offset);
        spessore+= PAD.offset; // Aggiunge l'offset del Pad utilizzato
    }
    printf("POSIZIONE COMPRESSORE:%d\n", spessore);

    // Se lo spessore è <=1 viene fissato a 1mm
    if(spessore<1) spessore =1;
    return spessore;
}

/* EOF */
 

