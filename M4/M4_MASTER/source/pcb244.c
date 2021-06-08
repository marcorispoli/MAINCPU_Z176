#define _PCB244_C
#include "dbt_m4.h" 

#define TARGET_ADDRESS 0x14
#define _DEF_PCB244_DRIVER_DELAY 1000

#define CONTEST PCB244_CONTEST
#define STATUS  (*((_PCB244_Stat_Str*)(&PCB244_CONTEST.Stat)))
#define ERROR_HANDLER   pcb244DriverCommunicationError

// Funzione di servizio interne al modulo
static bool GetFwRevision(void);
static void enterFreezeMode(void);
static void ERROR_HANDLER(void);

void pcb244_driver(uint32_t taskRegisters)
{
  int i;
  bool repeat;
  unsigned char dati[5];
  unsigned char polling=5;
  unsigned char mag=255;

    // Costruzione del contesto
   CONTEST.pReg = PCB244_Registers;
   CONTEST.nregisters = PCB244_NREGISTERS;
   CONTEST.evm = _EVM(_EV0_PCB244_CFG_UPD);
   CONTEST.evr = &_EVR(_EV0_PCB244_CFG_UPD);
   CONTEST.address = TARGET_ADDRESS;
   printf("ATTIVAZIONE DRIVER PCB244: \n");
    
   //////////////////////////////////////////////////////////////////////////
   //                   FINE FASE DI INIZIALIZZAZIONE DRIVER               //             
   //        Inizia il ciclo di controllo e gestione della periferica      //
   //////////////////////////////////////////////////////////////////////////

    // In caso di errore di compilazione in questo punto 
    // significa errore in compilazione della struttura registri
    SIZE_CHECK((sizeof(PCB244_Registers)/sizeof(_DeviceRegItem_Str))!=PCB244_NREGISTERS);
          
    // Retrive Task ID
    CONTEST.ID =  _task_get_id();
    
    // Init registro di stato
    memset((void*)&(STATUS), 0,sizeof(_Device_Stat_Str ));
  
    repeat = FALSE;
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
     if(STATUS.freeze) enterFreezeMode();
     
     
     // POlling per testare lo stato corrente dell'accessorio
     if(--polling==0)
     {
       repeat = TRUE;
       polling=5;
     }
          
     // Verifica la presenza e contestualmente richiede anche l'identificazione
     if(Ser422ReadRegister(_REGID(RG244_SETTINGS),2,&CONTEST)==_SER422_NO_ERROR)
     {
        _EVSET(_EV1_PCB244_CONNECTED); // Segnala evento connessione
       
        // Identificazione Potter
        switch((_DEVREGL(RG244_SETTINGS,CONTEST)>>3)&0x7)
        {
          case 1: generalConfiguration.potterCfg.potId = POTTER_2D;
          break;
          case 2: generalConfiguration.potterCfg.potId = POTTER_TOMO ;
          break;
          case 4: generalConfiguration.potterCfg.potId = POTTER_MAGNIFIER ;
          break;
          default:
            generalConfiguration.potterCfg.potId = POTTER_UNDEFINED ;        
        }
        
        // Nel caso di Ingranditore, viene chiesto anche il fattore di ingrandimento.. (se Digitale)
        // Nel caso di Analogico, il fattore di ingrandimento viene indirettamente calcolato
        // sulla base dello spessore di compressione (vedere PCB215 driver)
        if(generalConfiguration.potterCfg.potId  == POTTER_MAGNIFIER)
        {
          if(Ser422ReadRegister(_REGID(RG244_IO_EXP_B),2,&CONTEST)==_SER422_NO_ERROR)
          {
            mag = ((~(_DEVREGL(RG244_IO_EXP_B,CONTEST)))) & 0x7 ;
            if(generalConfiguration.potterCfg.potMagFactor!= mag)
            {
              generalConfiguration.potterCfg.potMagFactor = mag;
              printf("INGRANDITORE: FATTORE %d\n", generalConfiguration.potterCfg.potMagFactor);
              repeat = TRUE;
            }
          }
        }

        if((pcb244isPresent==FALSE)||(repeat))
        {
          if(pcb244isPresent==FALSE)
          {
            printf("CONNESSIONE POTTER ID:%d\n",generalConfiguration.potterCfg.potId);
            GetFwRevision();          
            pcb244isPresent=TRUE;
          }
          polling = 5;
          repeat = FALSE;
          dati[0] = generalConfiguration.potterCfg.potId;
          dati[1] = generalConfiguration.potterCfg.potMagFactor;// generalConfiguration.comprCfg.calibration.fattoreIngranditore[generalConfiguration.potterCfg.potMagFactor &0x7];
          mccGuiNotify(1,MCC_POTTER_ID,dati,2); // Notifica l'applicazione          
        }
     }else
     {
        generalConfiguration.potterCfg.potId = POTTER_UNDEFINED;
        generalConfiguration.potterCfg.potMagFactor=255;
        _EVCLR(_EV1_PCB244_CONNECTED); // Segnala evento disconnessione
        if((pcb244isPresent==TRUE)||(repeat))
        {
          if(pcb244isPresent==TRUE) printf("POTTER SCOLLEGATO\n");
          polling = 5;
          pcb244isPresent=FALSE;
          dati[0]=generalConfiguration.potterCfg.potId;
          mccGuiNotify(1,MCC_POTTER_ID,dati,1); // Notifica l'applicazione          
        }
     }
     
      // Termine della routine di driver
      STATUS.ready=1;
      _EVSET(_EV1_PCB244_RUN);
     _time_delay(_DEF_PCB244_DRIVER_DELAY);
   }
}

//////////////////////////////////////////////////////////////////////////////
/*
_PCB244_Error_Enum GetFwRevision(void)
        La funzione legge il codice di revisione del firmware del 
        target.

PARAM:
        -
RETURN:
      TRUE: Lettura avvenuta con successo
      FALSE: Problema di comunicazione con il target

      PCB244_Stat.maj_code/PCB244_Stat.min_code = codice revisione
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
  frame.data1=_CMD1(PCB244_GET_FWREV);
  frame.data2=_CMD2(PCB244_GET_FWREV);
  Ser422Send(&frame, SER422_BLOCKING, CONTEST.ID);
  
  // Eventuali errori di comunicazione
  if(frame.retcode!=SER422_DATA) 
  {
    printf("ERRORE REVISIONE FW PCB244\n");
    return FALSE;
  }
  
  STATUS.maj_code = frame.data2;
  STATUS.min_code = frame.data1;
  
  generalConfiguration.revisioni.pcb244.maj = STATUS.maj_code; 
  generalConfiguration.revisioni.pcb244.min = STATUS.min_code; 
 
  printf("REVISIONE PCB244: %d.%d\n",STATUS.maj_code,STATUS.min_code);  
  return TRUE;
   
}

bool pcb244StartVoiceCoil(void)
{  
  if(Ser422WriteRegister(_REGID(RG244_EXE),PCB244_START_VC,5,&CONTEST)== _SER422_NO_ERROR) return TRUE;
  printf("POTTER START VOICE ERRORE\n");
  return FALSE;
  
}

bool pcb244StopVoiceCoil(void)
{
  if(Ser422WriteRegister(_REGID(RG244_EXE),PCB244_STOP_GRID,5,&CONTEST)==_SER422_NO_ERROR) return TRUE;
  return FALSE;
  
}

bool pcb244SetFreqVoiceCoil(unsigned char val)
{
  printf("Voice Coil frequency setting: %d\n",val);
  if(Ser422WriteRegister(_REGID(RG244_VC_FREQ),val,5,&CONTEST)==_SER422_NO_ERROR) return TRUE;
  return FALSE;
  
}

bool pcb244SetAmplVoiceCoil(unsigned char val)
{
  printf("Voice Coil amplitude setting: %d\n",val);
  if(Ser422WriteRegister(_REGID(RG244_VC_AMP),val,5,&CONTEST)==_SER422_NO_ERROR) return TRUE;
  return FALSE;
  
}

bool pcb244Start2d(void)
{
    _Ser422_Command_Str frame;

     frame.address = TARGET_ADDRESS;
     frame.attempt = 4;
     frame.cmd=SER422_COMMAND;

     // Scrive il codice comando
     frame.data1=_CMD1(PCB244_START_2D);
     frame.data2=_CMD2(PCB244_START_2D);
     Ser422Send(&frame, SER422_BLOCKING, CONTEST.ID);

     if(frame.retcode==SER422_COMMAND_OK) {
         printf("PCB244 - STARTED!!!! \n");
         return true;
     }

     // Condizione di BUSY
     if(frame.retcode==SER422_BUSY) {
         printf("PCB244 - BUSY!!!! \n");
         return false;
     }

     printf("PCB244 - ERROR: [%d] [%d] !!!! \n", frame.data1,frame.data2);
     return FALSE;

 }

bool pcb244Stop2d(void)
{
    _Ser422_Command_Str frame;

     frame.address = TARGET_ADDRESS;
     frame.attempt = 4;
     frame.cmd=SER422_COMMAND;

     // Scrive il codice comando
     frame.data1=_CMD1(PCB244_STOP_2D);
     frame.data2=_CMD2(PCB244_STOP_2D);
     Ser422Send(&frame, SER422_BLOCKING, CONTEST.ID);

     if(frame.retcode==SER422_COMMAND_OK) {
         printf("PCB244 - STARTED!!!! \n");
         return true;
     }

     // Condizione di BUSY
     if(frame.retcode==SER422_BUSY) {
         printf("PCB244 - BUSY!!!! \n");
         return false;
     }

     printf("PCB244 - ERROR: [%d] [%d] !!!! \n", frame.data1,frame.data2);
     return FALSE;

 }


bool pcb244ResetFaults(void)
{
    _Ser422_Command_Str frame;

     frame.address = TARGET_ADDRESS;
     frame.attempt = 4;
     frame.cmd=SER422_COMMAND;

     // Scrive il codice comando
     frame.data1=_CMD1(PCB244_RESET_FAULT);
     frame.data2=_CMD2(PCB244_RESET_FAULT);
     Ser422Send(&frame, SER422_BLOCKING, CONTEST.ID);

     if(frame.retcode==SER422_COMMAND_OK) {
         printf("PCB244 - RESET FAULT OK!!!! \n");
         return true;
     }

     // Condizione di BUSY
     if(frame.retcode==SER422_BUSY) {
         printf("PCB244 - BUSY!!!! \n");
         return false;
     }

     printf("PCB244 - ERROR: [%d] [%d] !!!! \n", frame.data1,frame.data2);
     return FALSE;

 }

bool pcb244ResetBoard(void)
{
    _Ser422_Command_Str frame;

     frame.address = TARGET_ADDRESS;
     frame.attempt = 4;
     frame.cmd=SER422_COMMAND;

     // Scrive il codice comando
     frame.data1=_CMD1(PCB244_RESET_BOARD);
     frame.data2=_CMD2(PCB244_RESET_BOARD);
     Ser422Send(&frame, SER422_BLOCKING, CONTEST.ID);

     if(frame.retcode==SER422_COMMAND_OK) {
         printf("PCB244 - RESET FAULT OK!!!! \n");
         return true;
     }

     // Condizione di BUSY
     if(frame.retcode==SER422_BUSY) {
         printf("PCB244 - BUSY!!!! \n");
         return false;
     }

     printf("PCB244 - ERROR: [%d] [%d] !!!! \n", frame.data1,frame.data2);
     return FALSE;

 }

/*
  Funzione configuratrice
*/
bool config_pcb244(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len){
  return true;
}

void enterFreezeMode(void){
  // Entra in Freeze
  printf("PB244 ENTRA IN FREEZE\n");
  _EVCLR(_EV1_PCB244_RUN);
  _EVSET(_EV1_PCB244_FREEZED); // Notifica l'avvenuto Blocco
  _EVWAIT_ANY(_MOR2(_EV1_DEVICES_RUN,_EV1_PCB244_RUN)); // Attende lo sblocco
  _EVSET(_EV1_PCB244_RUN);
  printf("PB244 ESCE DAL FREEZE\n");
  STATUS.freeze = 0;

}


/* EOF */
 

