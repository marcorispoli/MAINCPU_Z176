#define _PCB249U1_C
#include "dbt_m4.h" 

#define TARGET_ADDRESS 0x16
#define _DEF_PCB249U1_DRIVER_DELAY 500

#define CONTEST PCB249U1_CONTEST
#define STATUS  (*((_PCB249U1_Stat_Str*)(&PCB249U1_CONTEST.Stat)))
#define ERROR_HANDLER   pcb249U1DriverCommunicationError

// Funzione di servizio interne al modulo
static bool GetFwRevision(void);
static void enterFreezeMode(void);
static bool pcb249U1UpdateRegisters(void);
static void ERROR_HANDLER(void);

static bool resetDriver = TRUE;
static bool verifyUpdateRegisters = TRUE;

static bool identificazioneAccessorio(void);
static bool pcb249U1WaitBusy(int timeout);
static unsigned char left, right, trap;

void pcb249U1_driver(uint32_t taskRegisters)
{
  int i;
   _SER422_Error_Enum err_ret;
  bool write_ok;   
  unsigned char data[10];


    // Costruzione del contesto
   CONTEST.pReg = PCB249U1_Registers;
   CONTEST.nregisters = PCB249U1_NREGISTERS;
   CONTEST.evm = _EVM(_EV0_PCB249U1_CFG_UPD);
   CONTEST.evr = &_EVR(_EV0_PCB249U1_CFG_UPD);
   CONTEST.address = TARGET_ADDRESS;
   printf("ATTIVAZIONE DRIVER PCB249U1: \n");
    
   //////////////////////////////////////////////////////////////////////////
   //                   FINE FASE DI INIZIALIZZAZIONE DRIVER               //             
   //        Inizia il ciclo di controllo e gestione della periferica      //
   //////////////////////////////////////////////////////////////////////////

    // In caso di errore di compilazione in questo punto 
    // significa errore in compilazione della struttura registri
    SIZE_CHECK((sizeof(PCB249U1_Registers)/sizeof(_DeviceRegItem_Str))!=PCB249U1_NREGISTERS);
      
    // Segnalazione driver disconnesso
    _EVCLR(_EV1_PCB249U1_CONNECTED);
    
    // Retrive Task ID
    CONTEST.ID =  _task_get_id();
    
    // Init registro di stato
    memset((void*)&(STATUS), 0,sizeof(_Device_Stat_Str ));

   // Inizializzazione delle mutex
    if (_mutex_init(&(CONTEST.reglist_mutex), NULL) != MQX_OK)
    {
      printf("PCB249U1: ERRORE INIT MUTEX. FINE PROGRAMMA");
      _mqx_exit(-1);
    }

    if (_mutex_init(&(CONTEST.pollinglist_mutex), NULL) != MQX_OK)
    {
      printf("PCB249U1: ERRORE INIT MUTEX. FINE PROGRAMMA");
      _mqx_exit(-1);
    }
      
    // Reitera fino ad ottenere il risultato
    while(GetFwRevision()==FALSE)_time_delay(100);
    printf("PCB249U1:REVISIONE FW TARGET:%d.%d\n",STATUS.maj_code,STATUS.min_code); 

    // Legge il registro che identifica il modello del Collimatore
    while(Ser422ReadRegister(_REGID(PR_COLLI_MODEL),10,&CONTEST)!=_SER422_NO_ERROR) _time_delay(200);
    if(_DEVREGL(PR_COLLI_MODEL,CONTEST)==0){
        printf("PCB-14/249-2 ASSY 01 DETECTED\n");
        generalConfiguration.revisioni.pcb249U1.model = _COLLI_TYPE_ASSY_01;
    }else{
        printf("PCB-14/249-2 ASSY 02 DETECTED\n");
        generalConfiguration.revisioni.pcb249U1.model = _COLLI_TYPE_ASSY_02;
    }

    // Verifica della correttezza  dell'assy assegnato al collimatore rispetto alla revisione corrente
    generalConfiguration.collimator_model_error = false;
    if( ((STATUS.maj_code <3 ) && (generalConfiguration.revisioni.pcb249U1.model == _COLLI_TYPE_ASSY_02)) ||
         ((STATUS.maj_code >=3 ) && (generalConfiguration.revisioni.pcb249U1.model == _COLLI_TYPE_ASSY_01))
      ){
        generalConfiguration.collimator_model_error = true;
        printf("INVALID COLLIMATOR ASSEMBLY VS FIRMWARE DETECTED!\n");
    }

    // Segnalazione driver connesso
   _EVSET(_EV1_PCB249U1_CONNECTED);
    
    // Attende l'autorizzazione ad effetuare il refresh registri
    _EVWAIT_ANY(_EV1_UPDATE_REGISTERS);
        

    // Verifica inizializzazione Collimatore se necessario e se consentito
    if(!generalConfiguration.collimator_model_error){

        Ser422ReadRegister(_REGID(PR_L_OUT),10,&CONTEST);
        if(_DEVREGL(PR_L_OUT,CONTEST)==0) pcb249U1initCollimator();
   }

    // Upload contenuto registri
   for(i=0;i<PCB249U1_NREGISTERS;i++)
   {
      err_ret = Ser422ReadRegister(i,4,&CONTEST);
      if(err_ret!=_SER422_NO_ERROR)
      {        
         ERROR_HANDLER();
         break;
      }
   }
   
   // Attende la ricezione della configurazione se necessario
   _EVSET(_EV2_PCB249U1_STARTUP_OK);
   printf("PCB249U1: ATTENDE CONFIGURAZIONE..\n");

   _EVWAIT_ANY(_EV1_DEV_CONFIG_OK);
   printf("PCB249U1: CONFIGURAZIONE OK. INIZIO LAVORO\n");



   ////////////////////////////////////////////////////////////////////////
   /*
                  GESTIONE DEL CICLO DI LAVORO ORDINARIO
       Il driver effettua un polling sui principali registri di lettura
       iscritti nella lista di polling ed effettua un controllo periodico
       sui registri di scrittura per eventuali udates in caso di differenze 
   */
   /////////////////////////////////////////////////////////////////////////
   _EVCLR(_EV0_PCB249U1_COLLI);
   u1colli_result = false;
   u1colli_id = 0;


   while(1)
   {
     if(STATUS.freeze) enterFreezeMode();

     // Gestione collimazione 2D
     if(_IS_EVENT(_EV0_PCB249U1_COLLI)){

         u1colli_result = false;
         printf("GESTIONE COLLIMAZIONE LAME LATERALI:L=%d, R=%d, T=%D\n",leftcolli_req,rightcolli_req,trapcolli_req);
         left = leftcolli_req;
         right = rightcolli_req;
         trap = trapcolli_req;

         // Impostazioni target lame
         if((Ser422WriteRegister(_REGID(RG249U1_PR_2D_L_USER),left,10,&PCB249U1_CONTEST) != _SER422_NO_ERROR) ||
           (Ser422WriteRegister(_REGID(RG249U1_PR_2D_R_USER),right,10,&PCB249U1_CONTEST) != _SER422_NO_ERROR)||
           (Ser422WriteRegister(_REGID(RG249U1_PR_2D_B_USER),trap,10,&PCB249U1_CONTEST) != _SER422_NO_ERROR)){
             printf("ERRORE SCRITTURA REGISTRI TARGET COLLI LEFT RIGHT TRAP\n");
             // Se è stata richiesta da GUI una risposta la invia
             if(u1colli_id){
                 Ser422ReadRegister(_REGID(RG249U1_RIGHT_SENS),10,&CONTEST);
                 Ser422ReadRegister(_REGID(RG249U1_LEFT_SENS),10,&CONTEST);
                 Ser422ReadRegister(_REGID(RG249U1_TRAP_SENS),10,&CONTEST);

                 data[0]=0;
                 data[1]=1; // Collimazione lame laterali
                 data[2]=_DEVREGL(RG249U1_LEFT_SENS,CONTEST);
                 data[3]=_DEVREGL(RG249U1_RIGHT_SENS,CONTEST);
                 data[4]=_DEVREGL(RG249U1_TRAP_SENS,CONTEST);
                 mccGuiNotify(u1colli_id,MCC_SET_COLLI,data,5);
             }
             _EVCLR(_EV0_PCB249U1_COLLI);
             continue;
         }

         // Attesa READY dal dispositivo
         if(pcb249U1WaitBusy(80)==false){
             printf("TIMEOUT COLLI U1 IN ATTESA DEL READY\n");
             // Se è stata richiesta da GUI una risposta la invia
             if(u1colli_id){
                 Ser422ReadRegister(_REGID(RG249U1_RIGHT_SENS),10,&CONTEST);
                 Ser422ReadRegister(_REGID(RG249U1_LEFT_SENS),10,&CONTEST);
                 Ser422ReadRegister(_REGID(RG249U1_TRAP_SENS),10,&CONTEST);

                 data[0]=0;
                 data[1]=1; // Collimazione lame laterali
                 data[2]=_DEVREGL(RG249U1_LEFT_SENS,CONTEST);
                 data[3]=_DEVREGL(RG249U1_RIGHT_SENS,CONTEST);
                 data[4]=_DEVREGL(RG249U1_TRAP_SENS,CONTEST);
                 mccGuiNotify(u1colli_id,MCC_SET_COLLI,data,5);
             }
             _EVCLR(_EV0_PCB249U1_COLLI);
             continue;
         }

         // Reset Faults
         if(_TEST_BIT(PCB249U1_FAULT))    pcb249U1ResetFaults();

         if(pcb249U1SetColliCmd(2)==false){
             printf("ERRORE COMANDO COLLIMAZIONE U1\n");
             // Se è stata richiesta da GUI una risposta la invia
             if(u1colli_id){
                 Ser422ReadRegister(_REGID(RG249U1_RIGHT_SENS),10,&CONTEST);
                 Ser422ReadRegister(_REGID(RG249U1_LEFT_SENS),10,&CONTEST);
                 Ser422ReadRegister(_REGID(RG249U1_TRAP_SENS),10,&CONTEST);

                 data[0]=0;
                 data[1]=1; // Collimazione lame laterali
                 data[2]=_DEVREGL(RG249U1_LEFT_SENS,CONTEST);
                 data[3]=_DEVREGL(RG249U1_RIGHT_SENS,CONTEST);
                 data[4]=_DEVREGL(RG249U1_TRAP_SENS,CONTEST);
                 mccGuiNotify(u1colli_id,MCC_SET_COLLI,data,5);
             }
             _EVCLR(_EV0_PCB249U1_COLLI);
             continue;
         }

         _time_delay(50);

         // Attesa READY dal dispositivo
         if(pcb249U1WaitBusy(80)==false){
             printf("TIMEOUT POSIZIONAMENTO LAME U1\n");
             // Se è stata richiesta da GUI una risposta la invia
             if(u1colli_id){
                 Ser422ReadRegister(_REGID(RG249U1_RIGHT_SENS),10,&CONTEST);
                 Ser422ReadRegister(_REGID(RG249U1_LEFT_SENS),10,&CONTEST);
                 Ser422ReadRegister(_REGID(RG249U1_TRAP_SENS),10,&CONTEST);

                 data[0]=0;
                 data[1]=1; // Collimazione lame laterali
                 data[2]=_DEVREGL(RG249U1_LEFT_SENS,CONTEST);
                 data[3]=_DEVREGL(RG249U1_RIGHT_SENS,CONTEST);
                 data[4]=_DEVREGL(RG249U1_TRAP_SENS,CONTEST);
                 mccGuiNotify(u1colli_id,MCC_SET_COLLI,data,5);
              }
             _EVCLR(_EV0_PCB249U1_COLLI);
             continue;
         }

         // Risultato del posizionamento
         if(_TEST_BIT(PCB249U1_FAULT)) {
             printf("ERRORE POSIZIONAMENTO LAME U1\n");
             // Se è stata richiesta da GUI una risposta la invia
             if(u1colli_id){
                 Ser422ReadRegister(_REGID(RG249U1_RIGHT_SENS),10,&CONTEST);
                 Ser422ReadRegister(_REGID(RG249U1_LEFT_SENS),10,&CONTEST);
                 Ser422ReadRegister(_REGID(RG249U1_TRAP_SENS),10,&CONTEST);

                 data[0]=0;
                 data[1]=1; // Collimazione lame laterali
                 data[2]=_DEVREGL(RG249U1_LEFT_SENS,CONTEST);
                 data[3]=_DEVREGL(RG249U1_RIGHT_SENS,CONTEST);
                 data[4]=_DEVREGL(RG249U1_TRAP_SENS,CONTEST);
                 mccGuiNotify(u1colli_id,MCC_SET_COLLI,data,5);
               }
             _EVCLR(_EV0_PCB249U1_COLLI);
             continue;
         }

         // Operazione conclusa con successo
         // Se la richiesta è ancora uguale a quanto comandato
         // allora vuol dire che non ci sono altri comandi oppure che
         // il nuovo comando è uguale allo stato attuale
         if((left==leftcolli_req)&&(right==rightcolli_req)&&(trap==trapcolli_req)){
             _EVCLR(_EV0_PCB249U1_COLLI);
             u1colli_result = true;
             printf("COLLIMAZIONE U1 CONCLUSA: L:%d, R:%d, T:%d\n",left, right, trap);
             if(u1colli_id){
                 Ser422ReadRegister(_REGID(RG249U1_RIGHT_SENS),10,&CONTEST);
                 Ser422ReadRegister(_REGID(RG249U1_LEFT_SENS),10,&CONTEST);
                 Ser422ReadRegister(_REGID(RG249U1_TRAP_SENS),10,&CONTEST);

                 data[0]=1; // Conclusa con successo
                 data[1]=1; // Collimazione lame laterali
                 data[2]=_DEVREGL(RG249U1_LEFT_SENS,CONTEST);
                 data[3]=_DEVREGL(RG249U1_RIGHT_SENS,CONTEST);
                 data[4]=_DEVREGL(RG249U1_TRAP_SENS,CONTEST);
                 mccGuiNotify(u1colli_id,MCC_SET_COLLI,data,5);
               }
         }

         continue;
     }
     // Legge i registri di Base che devono sempre essere aggiornati
     if(pcb249U1UpdateRegisters()==FALSE)  ERROR_HANDLER();
      
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
        printf("PCB249U1:CONFIG UPDATED!\n");
        
        // Invia segnale di aggiornamento cfg      
        _EVSET(_EV0_PCB249U1_CFG_UPD);
       }
     }
     _mutex_unlock(&(CONTEST.reglist_mutex));
     
     // Termine della routine di driver
      STATUS.ready=1;
      _EVSET(_EV1_PCB249U1_RUN);
     _time_delay(_DEF_PCB249U1_DRIVER_DELAY);
   }
}

//////////////////////////////////////////////////////////////////////////////
/*
_PCB249U1_Error_Enum GetFwRevision(void)
        La funzione legge il codice di revisione del firmware del 
        target.

PARAM:
        -
RETURN:
      TRUE: Lettura avvenuta con successo
      FALSE: Problema di comunicazione con il target

      PCB249U1_Stat.maj_code/PCB249U1_Stat.min_code = codice revisione
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
  frame.data1=_CMD1(PCB249U1_GET_REV);
  frame.data2=_CMD2(PCB249U1_GET_REV);
  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
  
  // Eventuali errori di comunicazione
  if(frame.retcode!=SER422_DATA) return FALSE;

  STATUS.maj_code = frame.data2;
  STATUS.min_code = frame.data1;
  generalConfiguration.revisioni.pcb249U1.maj = STATUS.maj_code; 
  generalConfiguration.revisioni.pcb249U1.min = STATUS.min_code; 

  
  return TRUE;

   
}

void ERROR_HANDLER(void)
{
   // Segnalazione driver disconnesso
   _EVCLR(_EV1_PCB249U1_CONNECTED);

   // Riconfigurazione del driver a seguito della ripartenza
   printf("PCB249U1 ERRORE: ATTESA RICONNESSIONE E RICONFIGURAZIONE REGISTRI\n"); 
  
   while(1){
    
    _time_delay(100);
    
    // Richiesta revisione firmware a target
    while(GetFwRevision()==FALSE) _time_delay(100);
    printf("PCB249U1:REVISIONE FW TARGET:%d.%d\n",STATUS.maj_code,STATUS.min_code);     

    // Carica sulla periferica lo stato dei registri cosi come erano prima del reset
    printf("PCB249U1: DOWNLOAD REGISTRI ...\n");
    if(Ser422UploadRegisters(10, &CONTEST)== FALSE)   continue;  
   
    // Carica Tutti i registri RD / RW
    int i;
    for(i=0;i<PCB249U1_NREGISTERS;i++) 
    {
      if(Ser422ReadRegister(i,4,&CONTEST)!=_SER422_NO_ERROR) break; 
    }
    if(i!=PCB249U1_NREGISTERS) continue;  
    break;
  }
  
  // Provoca l'inizializzazione delle variabili di cambio stato
  verifyUpdateRegisters = TRUE;   

  // Segnalazione driver connesso
  _EVSET(_EV1_PCB249U1_CONNECTED);
  
  // Ripartenza completata. Può tornare da dove aveva lasciato
  printf("PCB249U1 RIPARTITA CORRETTAMENTE\n"); 

  return;
}

// MOD_ACCESSORIO
/*
  Da schema elettrico
  0	        V0
  63,75	        V1      (Pb)
  95,625	V2
  127,5	        V3      (Cal)
  159,375	V4      (3D)
  191,25	V5      (2D)
  223,125	V6
  255	        V7
 
SOGLIE:
    31,875
    79,6875
    111,5625
    143,4375
    175,3125
    207,1875
    239,0625

Codici accessori: 
- Piombo:               V1
- Fantoccio Calib:      V3
- Fantoccio Prot 2D:    V5
- Fantoccio 3D eco      V4
*/

bool identificazioneAccessorio(void)
{
  static unsigned char accessorio=255;
  unsigned char codice;

  Ser422ReadRegister(_REGID(RG249U1_RG_PROT),10,&CONTEST);
  unsigned char raw = _DEVREGL(RG249U1_RG_PROT,CONTEST); 

  // Identificazione del codice
  if(raw<31) codice = 0;
  else if(raw<80) codice = 1;
  else if(raw<112) codice = 2;
  else if(raw<143) codice = 3;
  else if(raw<175) codice = 4;
  else if(raw<207) codice = 5;
  else if(raw<239) codice = 6;
  else codice = 7;
  
  // Assegnazione accessorio
  switch(codice){
  case 7: generalConfiguration.colliCfg.codiceAccessorio = COLLI_ACCESSORIO_ND; break;
  case 1: generalConfiguration.colliCfg.codiceAccessorio = COLLI_ACCESSORIO_PIOMBO; break;  
  case 3: generalConfiguration.colliCfg.codiceAccessorio = COLLI_ACCESSORIO_CALIB_PLEXYGLASS; break;
  case 4: generalConfiguration.colliCfg.codiceAccessorio = COLLI_ACCESSORIO_PROTEZIONE_PAZIENTE_3D; break;
  case 5: generalConfiguration.colliCfg.codiceAccessorio = COLLI_ACCESSORIO_PROTEZIONE_PAZIENTE_2D; break;
  default: generalConfiguration.colliCfg.codiceAccessorio = COLLI_ACCESSORIO_FAULT; 
  }

  // Verifica cambio di stato
  if(accessorio!=generalConfiguration.colliCfg.codiceAccessorio)
  {
    accessorio = generalConfiguration.colliCfg.codiceAccessorio;
    printf("RILEVATO CAMBIO ACCESSORIO COLLIMATORE: %d Codice: V%d, RAW: %d\n",accessorio,codice,raw);
    return TRUE;
  }
  else return FALSE;
}

// Effettua l'update dei registri di sistema
bool pcb249U1UpdateRegisters(void)
{
  unsigned char data[3];
 
  static float temperatura = 255.0;
  static float temperatura_b = 0;
  static int back_temperatura = 0;
  
  static bool update = true;
  static int polling_timer = 20;
  
  // Restart driver
  if(verifyUpdateRegisters)
  {
    verifyUpdateRegisters = FALSE;
    temperatura = 255.0;
    update=true;
  }

   // Richiede lo stato dei targets
   if(Ser422ReadRegister(_REGID(RG249U1_SYS_FLAGS0),10,&CONTEST)!=_SER422_NO_ERROR) return FALSE;
   
   // Attiva gli eventi relativi ai flags di interfaccia
   if ((_DEVREGL(RG249U1_SYS_FLAGS0,CONTEST)&0x0F)>0) _EVSET(_EV1_PCB249U1_TARGET);
   if (_TEST_BIT(PCB249U1_BUSY)) _EVSET(_EV1_PCB249U1_BUSY);
   else _EVSET(_EV1_PCB249U1_READY);

   // Richiede il codice accessorio   
   if(identificazioneAccessorio()==TRUE) update=TRUE;
   
   // Richiede la temperatura
   if(Ser422ReadRegister(_REGID(RG249U1_RG_TEMP),10,&CONTEST)!=_SER422_NO_ERROR) return FALSE;
   

   // Non considera valori che si riferiscano a temperature sotto lo zero!
   if((_DEVREGL(RG249U1_RG_TEMP,CONTEST)>140)&&(_DEVREGL(RG249U1_RG_TEMP,CONTEST)<190)){  
     
     if(temperatura==255.0) temperatura = (((float) _DEVREGL(RG249U1_RG_TEMP,CONTEST) * 500.0 / 255.0) - 273.0);
     else temperatura =  temperatura * 0.9 + 0.1 * (((float) _DEVREGL(RG249U1_RG_TEMP,CONTEST) * 500.0 / 255.0) - 273.0);     
     if(fabs(temperatura_b-temperatura) > 0.5) temperatura_b = temperatura;       
     int val = (int) round(temperatura_b);
     if(back_temperatura != val)
     {
       back_temperatura = val;
       printf("TEMPERATURA TUBO: %d\n", back_temperatura);
       update=TRUE;
     }
   }
   
   // Legge il contenuto dell'inclinometro di bordo ma solo se non ci sono movimenti del tubo in corso
   if((generalConfiguration.trxExecution.run == false) ){
       if(Ser422Read16BitRegister(_REGID(RG249U1_GONIO16_TRX),4,&CONTEST) == _SER422_NO_ERROR){
           // Trasforma da 0.025° a 0.1°
           generalConfiguration.armExecution.dAngolo_inclinometro = (short) _DEVREG(RG249U1_GONIO16_TRX,CONTEST) / 4;
          // printf("ANGOLO INCLINOMETRO=%d\n", generalConfiguration.armExecution.dAngolo_inclinometro);
           actuatorsUpdateAngles(); // Richiede di aggiornare l'insieme degli angoli
       }
   }





   // Aggiornamento comunque a polling
   if((update)||(--polling_timer == 0))
   {
     data[0] = _DEVREGL(RG249U1_SYS_FLAGS0,CONTEST);
     data[1] = back_temperatura;
     data[2] = generalConfiguration.colliCfg.codiceAccessorio;     
     update = ! mccPCB249U1Notify(1,PCB249U1_NOTIFY_DATA,data,sizeof(data));
     polling_timer = 20;
   }
     

    return TRUE;
}
 
//_________________________________________________________________________________________________
//                               FUNZIONI DI INTERFACCIA

//////////////////////////////////////////////////////////////////////////////
/*
bool pcb249U1SetColliMode(unsigned char mode)
  La funzione imposta il target corrente per le lame di collimazione.
  
PARAM:
  - <mode>:
    0 = Target 2D, 24x30
    1 = Target 2D, 18x24
    2 = Target 2D, user
    3 = Target 2D, Tomo (inseguimento)

RETURN:
      TRUE: Comando eseguito con successo.
      FALSE: 
        STATUS.fault=1;
        STATUS.error = _SER422_TARGET_BUSY ---- La condizione è BUSY
        STATUS.error = _SER422_COMMUNICATION_ERROR ---- Problema sulla comunicazione

Autore: M. Rispoli
Data: 09/01/2015
*/
//////////////////////////////////////////////////////////////////////////////
bool pcb249U1SetColliCmd(unsigned char mode)
{
  if(generalConfiguration.collimator_model_error) return false;

  _Ser422_Command_Str frame;

  
  // Prepara il comando di sblocco
  frame.address = TARGET_ADDRESS;
  frame.attempt = 10;
  frame.cmd=SER422_COMMAND;

  switch(mode)
  {
  case 0: // Modo 2D 24x30
      printf("COLLIMATORE IN MODALITA 24x30!!\n");
      frame.data1=_CMD1(PCB249U1_SET_COLLI_24x30);
      frame.data2=_CMD2(PCB249U1_SET_COLLI_24x30);
    break;
  case 1: // Modo 2D 18x24
      printf("COLLIMATORE IN MODALITA 18x24!!\n");
      frame.data1=_CMD1(PCB249U1_SET_COLLI_18x24);
      frame.data2=_CMD2(PCB249U1_SET_COLLI_18x24);
    break;
  case 2: // Modo 2D USER
      printf("COLLIMATORE IN MODALITA USER!!\n");
      frame.data1=_CMD1(PCB249U1_SET_COLLI_USER);
      frame.data2=_CMD2(PCB249U1_SET_COLLI_USER);
    break;
  case 3: // Modo Tomografia con inseguimento su formato 24x30
      printf("COLLIMATORE IN MODALITA INSEGUIMENTO!!\n");
      frame.data1=_CMD1(PCB249U1_SET_COLLI_TOMO);
      frame.data2=_CMD2(PCB249U1_SET_COLLI_TOMO);
    break;
  }

  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);

  if(frame.retcode==SER422_COMMAND_OK) return TRUE;
  else return FALSE;

}


//////////////////////////////////////////////////////////////////////////////
/*
bool pcb249U1ResetGonio(unsigned short angolo)
  La funzione resetta il Gonio al valore angolo espresso in 0.025 °/unit
  
PARAM:
  - <angolo>: 0.025°/unit
    
RETURN:
      TRUE: Comando eseguito con successo.
      FALSE: 
        STATUS.fault=1;
        STATUS.error = _SER422_TARGET_BUSY ---- La condizione è BUSY
        STATUS.error = _SER422_COMMUNICATION_ERROR ---- Problema sulla comunicazione

Autore: M. Rispoli
Data: 09/01/2015
*/
//////////////////////////////////////////////////////////////////////////////
bool pcb249U1ResetGonio(unsigned short angolo)
{
  _Ser422_Command_Str frame;

  // Sospende il driver bloccando la mutex del polling
  // Il driver si blocca esattamente dopo aver letto i registri di stato
  _mutex_lock(&(CONTEST.pollinglist_mutex));
  
  // Scrittura dei registri contenenti il valore di azzeramento
  if(Ser422WriteRegister(_REGID(RG249U1_GONIO_OFS),angolo,10,&CONTEST)!=_SER422_NO_ERROR)
  {   
    STATUS.fault=1;
    STATUS.error=_SER422_COMMUNICATION_ERROR;
    return FALSE;
  } 
  
  // Invio comando di reset inclinometro
  
  // Prepara il comando di sblocco
  frame.address = TARGET_ADDRESS;
  frame.attempt = 10;
  frame.cmd=SER422_COMMAND;

  // Comando reset Gonio
  frame.data1=_CMD1(PCB249U1_RESET_GONIO);
  frame.data2=_CMD2(PCB249U1_RESET_GONIO);

  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
  _mutex_unlock(&(CONTEST.pollinglist_mutex));
  switch(frame.retcode)
  {
  case SER422_COMMAND_OK: return TRUE;  //Comando eseguito con successo
  case SER422_BUSY:
    STATUS.fault=1;
    STATUS.error=_SER422_TARGET_BUSY;
    return FALSE;
    break;
  default:
    STATUS.fault=1;
    STATUS.error=_SER422_COMMUNICATION_ERROR;
    return FALSE;
    break;
  }

}



bool pcb249U1ResetFaults(void)
{
   _Ser422_Command_Str frame;
  
  // Prepara il comando di download
  frame.address = TARGET_ADDRESS;
  frame.attempt = 10;
  frame.cmd=SER422_COMMAND;

  frame.data1=_CMD1(PCB249U1_CLEAR_ERRORS);
  frame.data2=_CMD2(PCB249U1_CLEAR_ERRORS);
    
  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
  
  if(frame.retcode == SER422_COMMAND_OK) return TRUE;  
  return FALSE;  

}


// Imposta una richiesta di collimazione lame laterali.
// L'ultima richiesta che arriva sovrascrive quella in attesa precedente
void pcb249U1SetColli(unsigned char left, unsigned char right, unsigned char trap, unsigned char id)
{
    if(generalConfiguration.collimator_model_error) return;

    leftcolli_req = left;
    rightcolli_req = right;
    trapcolli_req=trap;
    u1colli_id = id;
    u1colli_result = false;
    _EVSET(_EV0_PCB249U1_COLLI);
    return;

}

// Attiva la procedura di ricerca degli zeri per il collimatore
bool pcb249U1initCollimator(void)
{
   if(generalConfiguration.collimator_model_error) return false;

   _Ser422_Command_Str frame;
  
   printf("PROCEDURA DI AZZERAMENTO AUTOMATICA COLLIMATORE\n");
   
  // Sospende il driver bloccando la mutex del polling
  // Il driver si blocca esattamente dopo aver letto i registri di stato
  _mutex_lock(&(CONTEST.pollinglist_mutex));
  
  // Prepara il comando di download
  frame.address = TARGET_ADDRESS;
  frame.attempt = 10;
  frame.cmd=SER422_COMMAND;

  frame.data1=_CMD1(PCB249U1_TEACH_ZERO);
  frame.data2=_CMD2(PCB249U1_TEACH_ZERO);
    
  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID);
  _mutex_unlock(&(CONTEST.pollinglist_mutex));
  
  if(frame.retcode != SER422_COMMAND_OK) return FALSE;
  _time_delay(10000);
  
   // Attende il termine della procedura (BUSY==0)   
   int i=20;
   while(i--)
   {
     Ser422ReadRegister(_REGID(RG249U1_SYS_FLAGS0),10,&CONTEST);
     if(_DEVREGL(RG249U1_SYS_FLAGS0,CONTEST)&0x40) _time_delay(1000);
     else return TRUE;
   }
   
   printf("PROCEDURA DI AZZERAMENTO AUTOMATICA COLLIMATORE FALLITA!!\n");
   return FALSE;
   
}

void colliArrayPrint(void){
  int i;

#ifndef PRINTCFG
  return ;
#endif

  printf("COLLIMATORE: SETUP TEMPERATURA CUFFIA:\n");
  printf("TEMP ON:%d  TEMP OFF:%d\n\n\n",generalConfiguration.colliCfg.tempcuffia_on,generalConfiguration.colliCfg.tempcuffia_off);


  printf("CONFIGURAZIONE COLLIMATORE DINAMICO:\n");
  
  printf("\nLEFT-P: ");
  for(i=0; i<COLLI_DYNAMIC_SAMPLES; i++) printf("%d ",generalConfiguration.colliCfg.dynamicArray.tomoLeftBladeP[i]);
  printf("\nLEFT-N: ");
  for(i=0; i<COLLI_DYNAMIC_SAMPLES; i++) printf("%d ",generalConfiguration.colliCfg.dynamicArray.tomoLeftBladeN[i]);
    
  printf("\nRIGHT-P: ");
  for(i=0; i<COLLI_DYNAMIC_SAMPLES; i++) printf("%d ",generalConfiguration.colliCfg.dynamicArray.tomoRightBladeP[i]);
  printf("\nRIGHT-N: ");
  for(i=0; i<COLLI_DYNAMIC_SAMPLES; i++) printf("%d ",generalConfiguration.colliCfg.dynamicArray.tomoRightBladeN[i]);

  printf("\nTRAP-P: ");
  for(i=0; i<COLLI_DYNAMIC_SAMPLES; i++) printf("%d ",generalConfiguration.colliCfg.dynamicArray.tomoBackTrapP[i]);
  printf("\nTRAP-N: ");
  for(i=0; i<COLLI_DYNAMIC_SAMPLES; i++) printf("%d ",generalConfiguration.colliCfg.dynamicArray.tomoBackTrapN[i]);
  
  printf("\n FRONT:%d BACK:%d: \n", generalConfiguration.colliCfg.dynamicArray.tomoFront, generalConfiguration.colliCfg.dynamicArray.tomoBack);
}
/*
  Funzione configuratrice
*/
bool config_pcb249U1(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len){
  switch(blocco){
  case 0:
    memcpy(&generalConfiguration.colliCfg.dynamicArray.tomoLeftBladeP, buffer, COLLI_DYNAMIC_SAMPLES);
    break;
  case 1:
    memcpy(&generalConfiguration.colliCfg.dynamicArray.tomoLeftBladeN, buffer, COLLI_DYNAMIC_SAMPLES);
    break;
  case 2:
    memcpy(&generalConfiguration.colliCfg.dynamicArray.tomoRightBladeP, buffer, COLLI_DYNAMIC_SAMPLES);
    break;
  case 3:
    memcpy(&generalConfiguration.colliCfg.dynamicArray.tomoRightBladeN, buffer, COLLI_DYNAMIC_SAMPLES);
    break;
  case 4:
    memcpy(&generalConfiguration.colliCfg.dynamicArray.tomoBackTrapP, buffer, COLLI_DYNAMIC_SAMPLES);
    break;
  case 5:
    memcpy(&generalConfiguration.colliCfg.dynamicArray.tomoBackTrapN, buffer, COLLI_DYNAMIC_SAMPLES);
    break;
  case 6:
    generalConfiguration.colliCfg.dynamicArray.tomoFront=buffer[0];
    generalConfiguration.colliCfg.dynamicArray.tomoBack=buffer[1];

    // Formula temperatura cuffia: TRAW = 139 + 0.51 * T(°C)
    generalConfiguration.colliCfg.tempcuffia_on = ((unsigned char)  ((float)buffer[2] * 0.51 + 139));
    generalConfiguration.colliCfg.tempcuffia_off = ((unsigned char)  ((float)buffer[3] * 0.51 + 139));

    // Scrittura limiti di temperatura cuffia
    Ser422WriteRegister(_REGID(RG249U1_PR_TEMP_ALR),generalConfiguration.colliCfg.tempcuffia_on, 10,&CONTEST);
    Ser422WriteRegister(_REGID(RG249U1_PR_TEMP_ALR_OFF),generalConfiguration.colliCfg.tempcuffia_off, 10,&CONTEST);


    // Stampa il contenuto della collimazione dinamica
    colliArrayPrint();
    setColliArray(); // Aggiornamento periferica..
    break;
    
  }
  return true;
}

void enterFreezeMode(void){
    // Entra in Freeze
    printf("PB249U1 ENTRA IN FREEZE\n");
    _EVCLR(_EV1_PCB249U1_RUN);
    _EVSET(_EV1_PCB249U1_FREEZED); // Notifica l'avvenuto Blocco
    _EVWAIT_ANY(_MOR2(_EV1_DEVICES_RUN,_EV1_PCB249U1_RUN)); // Attende lo sblocco
    printf("PB249U1 ESCE DAL FREEZE\n");
    STATUS.freeze = 0;
}

bool pcb249U1SetWriteMode01(void)
{
   _Ser422_Command_Str frame;
  
  // Prepara il comando di download
  frame.address = TARGET_ADDRESS;
  frame.attempt = 10;
  frame.cmd=SER422_COMMAND;

  frame.data1=_CMD1(PCB249U1_WRITE_B01);
  frame.data2=_CMD2(PCB249U1_WRITE_B01);
    
  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID); 
  if(frame.retcode != SER422_COMMAND_OK) return FALSE;
  return true;   
}
bool pcb249U1SetWriteMode23(void)
{
   _Ser422_Command_Str frame;
  
  // Prepara il comando di download
  frame.address = TARGET_ADDRESS;
  frame.attempt = 10;
  frame.cmd=SER422_COMMAND;

  frame.data1=_CMD1(PCB249U1_WRITE_B23);
  frame.data2=_CMD2(PCB249U1_WRITE_B23);
    
  Ser422Send(&frame, SER422_BLOCKING,CONTEST.ID); 
  if(frame.retcode != SER422_COMMAND_OK) return FALSE;
  return true;   
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//      COLLIMAZIONE DINAMEICA
#define _RAD(x) ((float) x * 3.141593 / 180.0)
#define APPROX(x) ((((x)-floor(x))>0.5) ? ceil(x): floor(x))

#define FC 613      // Distanza fuoco-centro di rotazione (mm)
#define CD 45       // Distanza centro di rotazione - piano del detector (mm)
#define LF 150      // Distanza Lame-Fuoco (mm)
#define rC 0.109    // 32/256 Risoluzione spostamento lame (mm/unit)

#define DISPERSIONE -3
#define NREF        80
#define U           1.2737 // atan((150+dispersione)/CD)
#define A           153.7335   // sqrt((150+dispersione)*(150+dispersione)+CD*CD);
#define A0          33.5106     // LF * (sin(teta) / (FC/A + cos(teta)));

#define NTARGET_R   74      // nRef - LF * (dispersione+footprint) / (rC * (FC + CD)); // Target N per apertura a 0° per avere le lame alla dispersione voluta
#define NTARGET_L   68      // nRef - LF * (dispersione+footprint) / (rC * (FC + CD)); // Target N per apertura a 0° per avere le lame alla dispersione voluta

unsigned char getDynamicColliTargetL(float angolo){
  int i;  
  float teta= -1* _RAD(angolo) + U;
 
  return (unsigned char) APPROX(NTARGET_L - (LF * (sin(teta) / (FC/A + cos(teta))) - A0) / rC);

}

unsigned char getDynamicColliTargetR(float angolo){
  int i;  
  float teta= _RAD(angolo) + U;
 
  return (unsigned char) APPROX(NTARGET_R - (LF * (sin(teta) / (FC/A + cos(teta))) - A0) / rC);

}

bool setColliArray(void){
  unsigned char array_ptr;
  int ii;
  unsigned char dato;

    
  // ----------- Array lama L -------------------------------------------------------
  if(N_ARRAY_L > 0xFF) {
    if(!pcb249U1SetWriteMode23()) goto fallito;        
  }else
  {
    if(!pcb249U1SetWriteMode01()) goto fallito;                
  }
  for(array_ptr = (N_ARRAY_L&0xFF),ii=0;ii<COLLI_DYNAMIC_SAMPLES;ii++,array_ptr++){
    dato = generalConfiguration.colliCfg.dynamicArray.tomoLeftBladeN[ii];
    if(!Ser422WriteRaw(TARGET_ADDRESS, array_ptr, dato, 10)) goto fallito;
   }
  
  for(array_ptr = (P_ARRAY_L&0xFF),ii=0;ii<COLLI_DYNAMIC_SAMPLES;ii++,array_ptr++){
    dato = generalConfiguration.colliCfg.dynamicArray.tomoLeftBladeP[ii];
    if(!Ser422WriteRaw(TARGET_ADDRESS, array_ptr, dato, 10)) goto fallito;        
  }

  // ----------- Array lama R -------------------------------------------------------
  if(N_ARRAY_R > 0xFF) {
    if(!pcb249U1SetWriteMode23()) goto fallito;                
  }else
  {
    if(!pcb249U1SetWriteMode01()) goto fallito;                
  }
  
  for(array_ptr = (N_ARRAY_R&0xFF),ii=0;ii<COLLI_DYNAMIC_SAMPLES;ii++,array_ptr++){
    dato = generalConfiguration.colliCfg.dynamicArray.tomoRightBladeN[ii];
    if(!Ser422WriteRaw(TARGET_ADDRESS, array_ptr, dato, 10)) goto fallito;        
  }
  for(array_ptr = (P_ARRAY_R&0xFF),ii=0;ii<COLLI_DYNAMIC_SAMPLES;ii++,array_ptr++){
    dato = generalConfiguration.colliCfg.dynamicArray.tomoRightBladeP[ii];
    if(!Ser422WriteRaw(TARGET_ADDRESS, array_ptr, dato, 10)) goto fallito;        
  }

  // ----------- Array lama T -------------------------------------------------------
  if(N_ARRAY_B > 0xFF) {
    if(!pcb249U1SetWriteMode23()) goto fallito;                
  }else
  {
    if(!pcb249U1SetWriteMode01()) goto fallito;                
  }
  
  for(array_ptr = (N_ARRAY_B&0xFF),ii=0;ii<COLLI_DYNAMIC_SAMPLES;ii++,array_ptr++){
    dato = generalConfiguration.colliCfg.dynamicArray.tomoBackTrapN[ii];
    if(!Ser422WriteRaw(TARGET_ADDRESS, array_ptr, dato, 10)) goto fallito;            
  }
  for(array_ptr = (P_ARRAY_B&0xFF),ii=0;ii<COLLI_DYNAMIC_SAMPLES;ii++,array_ptr++){
    dato = generalConfiguration.colliCfg.dynamicArray.tomoBackTrapP[ii];
    if(!Ser422WriteRaw(TARGET_ADDRESS, array_ptr, dato, 10)) goto fallito;            
  }


  printf("PCB249U1 AGGIORNATA ..\n");
  pcb249U1SetWriteMode01();
  return true;
  
fallito:
  printf("PCB249U1 AGGIORNAMENTO FALLITO ..\n");
  pcb249U1SetWriteMode01();
  return false;
}


void pcb249U1_readGonio(void){

    // Legge il contenuto dell'inclinometro di bordo ma solo se non ci sono movimenti in corso
    if(Ser422Read16BitRegister(_REGID(RG249U1_GONIO16_TRX),10,&CONTEST) == _SER422_NO_ERROR){
        // Trasforma da 0.025° a 0.1°
        generalConfiguration.armExecution.dAngolo_inclinometro = (short) _DEVREG(RG249U1_GONIO16_TRX,CONTEST) / 4;
        actuatorsUpdateAngles(); // Richiede di aggiornare l'insieme degli angoli
    }

}

// Attende il READY con timeout di 100ms unit
bool pcb249U1WaitBusy(int timeout){
    while(timeout){
        if(Ser422ReadRegister(_REGID(RG249U1_SYS_FLAGS0),10,&CONTEST)!=_SER422_NO_ERROR) {
            _time_delay(100);
            timeout--;
            continue;
        };

        if(!_TEST_BIT(PCB249U1_BUSY)) return true;
        timeout--;
        _time_delay(100);
    }

    return false;
}

// Funzione usata dalle procedure raggi per accertarsi della
// corretta collimazione
bool wait2DLeftRightTrapCompletion(int timeout){

    int tmo = timeout;

    // Attende il completamento di una collimazione pendente
    while(_IS_EVENT(_EV0_PCB249U1_COLLI)){
        _time_delay(100);
        tmo--;
        if(!tmo){
            printf("TIMEOUT ATTESA COMPLETAMENTO COLLIMAZIONE LEFT+RIHT DURANTE RAGGI\n");
            return false;
        }
    }

    // Se il comando è fallito, riprova a collimare
    if(!u1colli_result){
        pcb249U1SetColli(leftcolli_req ,rightcolli_req,trapcolli_req,0); // ripete il comando
        _time_delay(50);
        tmo = timeout;
        while(_IS_EVENT(_EV0_PCB249U1_COLLI)){
            _time_delay(100);
            tmo--;
            if(!tmo){
                printf("TIMEOUT ATTESA COMPLETAMENTO COLLIMAZIONE LEFT+RIGHT DURANTE RAGGI\n");
                return false;
            }
        }
    }

    // Se infine non è riuscito allora si ferma qui
    if(!backfront_eseguito) return false;

    // Controllo completato con successo
    return true;
}
/* EOF */
 
  
 
