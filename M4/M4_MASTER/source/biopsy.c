#define _BIOPSY_C
#include "dbt_m4.h" 

static void BiopsySetX(void);
static void BiopsySetY(void);
static void BiopsySetZ(void);
static void BiopsySetStepUpZ(void);
static void BiopsySetStepDwnZ(void);
static bool biopsyGetZ(void);
static bool biopsyGetX(void);
static bool biopsyGetY(void);
static bool BiopsyGetStat(void);  
static bool BiopsyGetRevision(void);
static bool moveX(unsigned char* dati);
static bool moveY(unsigned char* dati);
static bool moveZ(unsigned char* dati);

#define TARGET_C 0x0D
#define TARGET_X 0x0E
#define TARGET_Y 0x0F
#define TARGET_Z 0x10

#define _DEF_BIOPSY_DRIVER_DELAY_NC 1000        // Tempo di attesa quando scollegato
#define _DEF_BIOPSY_DRIVER_DELAY_CONNECTED 200   // Tempo di attesa quando scollegato
#define _DEF_BIOPSY_DRIVER_TIMEOUT 3000        // Tempo massimo di attesa per una risposta
#define _DEF_BIOP_MOT_TMO          30           // timeout movimenti

#define CONTEST BIOPSY_CONTEST
#define STATUS  (*((_BIOPSY_Stat_Str*)(&BIOPSY_CONTEST.Stat)))

unsigned short driverDelay;
bool verifyBiopsyDataInit=FALSE;
static bool  biopTestPosition = FALSE;

//////////////////////////////////////////////////////////////////////////////
/*
void BIOPSY_driver(uint32_t taskRegisters)


Autore: M. Rispoli
Data: 22/06/2015
*/
//////////////////////////////////////////////////////////////////////////////
void BIOPSY_driver(uint32_t taskRegisters)
{
  int i;
  unsigned char dati[_BP_DATA_LEN];
  unsigned char polling=5;
  unsigned char mag;
  static unsigned short timeout; // Tempo massimo di attesa senza risposta
  static bool sbloccoReq=FALSE; 
  static unsigned char adapterId;
  static unsigned short needleZ=0xFFFF;
  static unsigned char maxZ = 255;
  bool notifica=FALSE;
  int repeat=5;
  int motionTmo =_DEF_BIOP_MOT_TMO;
  
    // Costruzione del contesto
   CONTEST.pReg = BIOPSY_Registers;
   CONTEST.nregisters = BIOPSY_NREGISTERS;
   //   CONTEST.evm = _EVM(_EV0_BIOPSY_CFG_UPD);
   //   CONTEST.evr = &_EVR(_EV0_BIOPSY_CFG_UPD);
   //   CONTEST.address = TARGET_ADDRESS;
   printf("ATTIVAZIONE DRIVER BIOPSY: \n");
   driverDelay =   _DEF_BIOPSY_DRIVER_DELAY_NC;
   //////////////////////////////////////////////////////////////////////////
   //                   FINE FASE DI INIZIALIZZAZIONE DRIVER               //             
   //        Inizia il ciclo di controllo e gestione della periferica      //
   //////////////////////////////////////////////////////////////////////////

    // In caso di errore di compilazione in questo punto 
    // significa errore in compilazione della struttura registri
    SIZE_CHECK((sizeof(BIOPSY_Registers)/sizeof(_DeviceRegItem_Str))!=BIOPSY_NREGISTERS);
          
    // Retrive Task ID
    CONTEST.ID =  _task_get_id();
    
    // Init registro di stato
    memset((void*)&(STATUS), 0,sizeof(_Device_Stat_Str ));
 
    // Init della mutex
    if (_mutex_init(&(CONTEST.pollinglist_mutex), NULL) != MQX_OK)
    {
      printf("BIOPSY: ERRORE INIT MUTEX. FINE PROGRAMMA");
      _mqx_exit(-1);
    }
      
    // Init della configurazione corrente del dispositivo
    generalConfiguration.biopsyCfg.connected = FALSE;
    generalConfiguration.biopsyCfg.needlePresent = FALSE;
    generalConfiguration.biopsyCfg.adapterId = 0;    
    generalConfiguration.biopsyCfg.sbloccoReq=FALSE;
    generalConfiguration.biopsyCfg.armEna = FALSE;
    generalConfiguration.biopsyCfg.needleZ = 0;    
    generalConfiguration.biopsyCfg.movimento = FALSE;
    generalConfiguration.biopsyCfg.movimento_x = FALSE;
    generalConfiguration.biopsyCfg.movimento_y = FALSE;
    generalConfiguration.biopsyCfg.movimento_z = FALSE;
    generalConfiguration.biopsyCfg.stepUp_z = FALSE;
    generalConfiguration.biopsyCfg.stepDwn_z = FALSE;
    generalConfiguration.biopsyCfg.checksum_h = 0;
    generalConfiguration.biopsyCfg.checksum_l = 0;
    generalConfiguration.biopsyCfg.revisione = 0;
    

    // Attende la configurazione del sistema prima di procedere
    while(generalConfiguration.deviceConfigOk==FALSE) _time_delay(1000);
    timeout = _DEF_BIOPSY_DRIVER_TIMEOUT/driverDelay;
    
   ////////////////////////////////////////////////////////////////////////
   /*
                  GESTIONE DEL CICLO DI LAVORO ORDINARIO
       Il driver effettua un polling sui principali registri di lettura
       iscritti nella lista di polling ed effettua un controllo periodico
       sui registri di scrittura per eventuali udates in caso di differenze
    
      data[0]: 0=NULLA, 1 = ->CONNESSO, 2->NON CONNESSO
      data[1]: 0=NULLA, 1 = ->SBLOCCO ON, 2-> SBLOCCO OFF
      data[2:3], Posizione corrente Z
      data[4]: 1= muove X, 2=muoveY, 3=muoveZ 4=fine movimenti 5 = TIMEOUT TORRETTA (reset)
      data[5] Risultato movimento (se data[4]==4)
          .0: 1 = X-ok, X-nok
          .1: 1 = Y-ok, Y-nok
          .2: 1 = Z-ok, Z-nok


   */
   /////////////////////////////////////////////////////////////////////////
   while(1)
   {
     if(STATUS.freeze)
     {
        // Entra in Freeze
        _EVCLR(_EV1_BIOPSY_RUN);
        _EVSET(_EV1_BIOPSY_FREEZED); // Notifica l'avvenuto Blocco
        _EVWAIT_ANY(_MOR2(_EV1_DEVICES_RUN,_EV1_BIOPSY_RUN)); // Attende lo sblocco
        _EVSET(_EV1_BIOPSY_RUN);
        STATUS.freeze = 0;
     }
     
     
     _mutex_lock(&(CONTEST.pollinglist_mutex));
     notifica=FALSE;
     dati[_BP_CONNESSIONE]=0;   // Connessione
     dati[_BP_SBLOCCO]=0;       // Pulsante sblocco
     dati[_BP_ACCESSORIO]=255;  // Codice accessorio     
     dati[_BP_ZL]=0;            // Posizione ZL
     dati[_BP_ZH]=0;            // Posizione ZH
     dati[_BP_MOTION]=0;        // movimento ..
     dati[_BP_MOTION_END]=0xFF; // Esito movimenti (se applicabile)
     dati[_BP_CHKH]=generalConfiguration.biopsyCfg.checksum_h;          // Checksum H
     dati[_BP_CHKL]=generalConfiguration.biopsyCfg.checksum_l;          // Checksum L
     dati[_BP_REVIS]=generalConfiguration.biopsyCfg.revisione;          // Revisione

    // ALLO STARTUP O QUANDO SERVE RINFRESCARE LO STATO ALL'APPLICAZIONE
    if(verifyBiopsyDataInit==TRUE)
    {
      verifyBiopsyDataInit = FALSE;
      notifica = TRUE; 
      if(generalConfiguration.biopsyCfg.connected==TRUE) dati[_BP_CONNESSIONE] = 1; // Notifica cambio stato in connessione
      else  dati[_BP_CONNESSIONE] = 2; // Notifica cambio stato in connessione        
      maxZ=255;
    }
  
     // Il test sulla presenza dipende dal fatto che non sia riconosciuto nessun POTTER
     if(generalConfiguration.potterCfg.potId == POTTER_UNDEFINED)
     {
       driverDelay =  _DEF_BIOPSY_DRIVER_DELAY_CONNECTED;
       
       // Se c'è un comando di movimento in corso lo gestisce in maniera speciale
       if(generalConfiguration.biopsyCfg.movimento)
       {                    
          
          if((biopsyGetZ()==FALSE) || (biopsyGetY()==FALSE) || (biopsyGetX()==FALSE) ) 
          {
            // Verifica il timeout sul movimento
            motionTmo--;
            if(!motionTmo) 
            {
              generalConfiguration.biopsyCfg.movimento = FALSE;
              generalConfiguration.biopsyCfg.movimento_x = FALSE;
              generalConfiguration.biopsyCfg.movimento_y = FALSE;
              generalConfiguration.biopsyCfg.movimento_z = FALSE;
              dati[_BP_MOTION ] = 5; // Timeout         
              mccBiopsyNotify(1,BIOP_NOTIFY_STAT,dati, sizeof(dati));
              timeout = _DEF_BIOPSY_DRIVER_TIMEOUT/driverDelay;
              printf("BIOPSIA: TIMEOUT TORRETTA\n");
            }

            // Se non è in  timeout allora attende ancora ...
            goto termine_biop_driver;
          }
          motionTmo =_DEF_BIOP_MOT_TMO;
          

          // Aggiorno i dati sulla Z        
          dati[_BP_ZL] = (unsigned char) (generalConfiguration.biopsyCfg.needleZ & 0x00FF);
          dati[_BP_ZH] = (unsigned char) (generalConfiguration.biopsyCfg.needleZ >> 8);
          needleZ = generalConfiguration.biopsyCfg.needleZ;
          

          if(generalConfiguration.biopsyCfg.stepUp_z)
          {
            generalConfiguration.biopsyCfg.stepUp_z = FALSE;
            BiopsySetStepUpZ();           
            dati[_BP_MOTION]=3;
            mccBiopsyNotify(1,BIOP_NOTIFY_STAT,dati, sizeof(dati));
            printf("BIOPSIA: MOVIMENTO STEP UP Z \n");
            goto termine_biop_driver;
          }

          if(generalConfiguration.biopsyCfg.stepDwn_z)
          {
              generalConfiguration.biopsyCfg.stepDwn_z = FALSE;
              BiopsySetStepDwnZ();           
              dati[_BP_MOTION]=3;
              mccBiopsyNotify(1,BIOP_NOTIFY_STAT,dati, sizeof(dati));
              printf("BIOPSIA: MOVIMENTO STEP DOWN Z \n");
              goto termine_biop_driver;
          }

          // La sequenza di spostamento dipende dalla direzione della Z (up o Dwn)
          // Quando la Z muove verso l'alto allora è proprio la Z a muovere per prima.
          // Quando la Z muove verso il basso allora la Z è l'ultima a muovere
          if(generalConfiguration.biopsyCfg.z_up)
          {
              if(moveZ(dati)) goto termine_biop_driver;
              else if(moveY(dati)) goto termine_biop_driver;
              else  if(moveX(dati)) goto termine_biop_driver;
          }else
          {
              if(moveX(dati)) goto termine_biop_driver;
              else if(moveY(dati)) goto termine_biop_driver;
              else  if(moveZ(dati)) goto termine_biop_driver;
          }
               
          // Se arriva qui allora il movimento è terminato
          generalConfiguration.biopsyCfg.movimento = FALSE;
          dati[_BP_MOTION] = 4;
          
          printf("TGX=%d, TGY=%d, TGZ=%d\n",generalConfiguration.biopsyCfg.target_x,generalConfiguration.biopsyCfg.target_y,generalConfiguration.biopsyCfg.target_z);
          printf("X=%d, Y=%d, Z=%d\n",generalConfiguration.biopsyCfg.needleX,generalConfiguration.biopsyCfg.needleY,generalConfiguration.biopsyCfg.needleZ);
          
          // Controllo sul risultato se tale controllo viene abilitato
          if(biopTestPosition)
          {
            biopTestPosition = FALSE;
            int dif;
            
            // Il controllo sulla XYZ da HOME a HOME (movimenti verso l'origine) richiedono un controllo più lasco
            // perchè il sistema non è in grado di posizionarsi in salita a meno di 2 millimetri dall'origine             
            if(generalConfiguration.biopsyCfg.target_z == 0){
              dif = generalConfiguration.biopsyCfg.target_z-generalConfiguration.biopsyCfg.needleZ;
              if((dif> 30) || (dif<-30) ) dati[_BP_MOTION_END]&= ~0x4;
            }else{
              dif = generalConfiguration.biopsyCfg.target_z-generalConfiguration.biopsyCfg.needleZ;
              if((dif> 10) || (dif<-10) ) dati[_BP_MOTION_END]&= ~0x4;
            }

            if(generalConfiguration.biopsyCfg.target_x == 0){
              dif = generalConfiguration.biopsyCfg.target_x-generalConfiguration.biopsyCfg.needleX;
              if((dif> 30) || (dif<-30) ) dati[_BP_MOTION_END]&= ~0x1;
            }else{
              dif = generalConfiguration.biopsyCfg.target_x-generalConfiguration.biopsyCfg.needleX;
              if((dif> 10) || (dif<-10) ) dati[_BP_MOTION_END]&= ~0x1;
            }

            if(generalConfiguration.biopsyCfg.target_y == 0){
              dif = generalConfiguration.biopsyCfg.target_y-generalConfiguration.biopsyCfg.needleY;
              if((dif> 30) || (dif<-30) ) dati[_BP_MOTION_END]&= ~0x2;
            }else{
              dif = generalConfiguration.biopsyCfg.target_y-generalConfiguration.biopsyCfg.needleY;
              if((dif> 10) || (dif<-10) ) dati[_BP_MOTION_END]&= ~0x2;
            }
          }
          
          mccBiopsyNotify(1,BIOP_NOTIFY_STAT,dati, sizeof(dati));
          timeout = _DEF_BIOPSY_DRIVER_TIMEOUT/driverDelay;
          printf("BIOPSIA: FINE MOVIMENTI! ESITO:%d\n",dati[_BP_MOTION_END]);
          goto termine_biop_driver;
       }
       else if(BiopsyGetStat()==TRUE)
       {      
         timeout = _DEF_BIOPSY_DRIVER_TIMEOUT/driverDelay;
         
         biopsyGetZ(); // Acquisisce la Z
        
         // Verifica prima connessione
         if(generalConfiguration.biopsyCfg.connected==FALSE)
         {
           // Richiede revisione e checksum caricati
           BiopsyGetRevision();
           
           // Blocca il braccio
           generalConfiguration.biopsyCfg.armEna = FALSE;
           actuatorsManageEnables();
           
           // Diminuisce il tempo di polling durante la connessione
           generalConfiguration.biopsyCfg.connected = TRUE;
           printf("RICONOSCIUTA BIOPSIA\n");
           dati[_BP_CONNESSIONE] = 1; // Notifica cambio stato in connessione         
           notifica=TRUE;
         }
         
         // Controlla se è stato premuto il pulsante di sblocco
         if(sbloccoReq!=generalConfiguration.biopsyCfg.sbloccoReq)
         {
           notifica=TRUE;
           sbloccoReq=generalConfiguration.biopsyCfg.sbloccoReq;
           if(sbloccoReq) dati[_BP_SBLOCCO] = 1;
           else dati[_BP_SBLOCCO] = 2;
           if(sbloccoReq) 
           {
             printf("BIOPSIA: RICHIESTA SBLOCCO BRACCIO\n");
             generalConfiguration.biopsyCfg.armEna = TRUE;           
             actuatorsManageEnables();
           }else
           {
             printf("BIOPSIA: RICHIESTA BLOCCO BRACCIO\n");
             generalConfiguration.biopsyCfg.armEna = FALSE; 
             actuatorsManageEnables();
           }
         }
         
         // Verifica l'accessorio Id
         if(adapterId!=generalConfiguration.biopsyCfg.adapterId)
         {
           notifica=TRUE;
           adapterId=generalConfiguration.biopsyCfg.adapterId;
           printf("BIOPSIA: CAMBIO ACCESSORIO:%d\n",adapterId);
         }
         dati[_BP_ACCESSORIO] = generalConfiguration.biopsyCfg.adapterId;           
         
         // Verifica la posizione corrente della Z        
          if((needleZ > generalConfiguration.biopsyCfg.needleZ + 10) || (needleZ < generalConfiguration.biopsyCfg.needleZ - 10))
          {
            needleZ=generalConfiguration.biopsyCfg.needleZ;
            notifica=TRUE;
          }

         // Aggiornamento dati sul limite di posizionamento meccanico(in millimetri)
         // solo se c'è compressione in corso
         if(_TEST_BIT(PCB215_COMPRESSION))
         {
           dati[_BP_MAX_Z] = generalConfiguration.biopsyCfg.conf.offsetZ
                            + generalConfiguration.biopsyCfg.conf.offsetPad 
                            - generalConfiguration.biopsyCfg.conf.marginePosizionamento            
                            - _DEVREG(RG215_DOSE,PCB215_CONTEST);     

           static unsigned char valore=255;
           if(dati[_BP_MAX_Z]!=valore){
               valore = dati[_BP_MAX_Z];
               //printf("BIOPSIA: ofz=%d, ofsPad=%d, margPos=%d, pad=%d, maxZ=%d\n",generalConfiguration.biopsyCfg.conf.offsetZ,generalConfiguration.biopsyCfg.conf.offsetPad,
               //       generalConfiguration.biopsyCfg.conf.marginePosizionamento,_DEVREG(RG215_DOSE,PCB215_CONTEST),valore);
           }
         }else
         {
           dati[_BP_MAX_Z] = 0; // Se non è in compressione non consente spostamenti di Z
          
         }
          if(maxZ!=dati[_BP_MAX_Z])
          {
            maxZ=dati[_BP_MAX_Z];
            notifica = TRUE;
          }
          
          //printf("OFFSET=%d, PAD=%d, MARG=%d, MAXZ = %d\n",generalConfiguration.biopsyCfg.conf.offsetZ,generalConfiguration.biopsyCfg.conf.offsetPad,generalConfiguration.biopsyCfg.conf.marginePosizionamento, dati[_BP_MAX_Z]); 
          // Sicuramente ogni movimento è attualmente terminato
       }else
       {
         if(!(timeout))
         {
             // Se non risponde entro un timeout il sistema viene dato per scollegato
              if(generalConfiguration.biopsyCfg.connected==TRUE)
              {// Cambio stato
                generalConfiguration.biopsyCfg.connected = FALSE;
                printf("BIOPSIA SCOLLEGATA\n");
                notifica=TRUE;
                dati[_BP_CONNESSIONE]=2; 
              }
         }else 
         {           
           timeout--;
           // printf("BIOPSIA: TEMPO AL TIMEOUT:%d\n", timeout);
         }
       }
     }else 
     {
        // Il potter è stato rilevato: comunica l'avvenuto cambio contesto
        if(generalConfiguration.biopsyCfg.connected==TRUE)
        {
          notifica=TRUE;
          dati[_BP_CONNESSIONE]=2; 
          generalConfiguration.biopsyCfg.connected = FALSE;
          driverDelay =  _DEF_BIOPSY_DRIVER_DELAY_NC;
        }
     }
     // Effettua la notifica dello stato se necessario
     if(notifica) 
     {
        // Aggiorno i dati sulla Z        
        dati[_BP_ZL] = (unsigned char) (generalConfiguration.biopsyCfg.needleZ & 0x00FF);
        dati[_BP_ZH] = (unsigned char) (generalConfiguration.biopsyCfg.needleZ >> 8);
        mccBiopsyNotify(1,BIOP_NOTIFY_STAT,dati, sizeof(dati));
        //printf("NOTIFICA BIOPSIA\n");
     }
     
      // Termine della routine di driver
 termine_biop_driver:
      _mutex_unlock(&(CONTEST.pollinglist_mutex));
      STATUS.ready=1;
      _EVSET(_EV1_BIOPSY_RUN);
     _time_delay(driverDelay);
   }
}

 // Gestisce il movimento sull'asse Z se necessario
 // Ritorna TRUE se il movimento è iniziato
 // FALSE se il movimento non è consentito/non è attivato
bool moveZ(unsigned char* dati)
{           
    if(generalConfiguration.biopsyCfg.movimento_z==FALSE) return FALSE;
    generalConfiguration.biopsyCfg.movimento_z = FALSE;
    BiopsySetZ();           
    dati[_BP_MOTION]=3;
    mccBiopsyNotify(1,BIOP_NOTIFY_STAT,dati, sizeof(dati));
    printf("BIOPSIA: MOVIMENTO Z \n");
    return TRUE; 
}

 // Gestisce il movimento sull'asse X se necessario
 // Ritorna TRUE se il movimento è iniziato
 // FALSE se il movimento non è consentito/non è attivato
bool moveX(unsigned char* dati)
{
  if(generalConfiguration.biopsyCfg.movimento_x==FALSE) return FALSE;

  generalConfiguration.biopsyCfg.movimento_x = FALSE;
  dati[_BP_MOTION]=1;
  BiopsySetX();
  mccBiopsyNotify(1,BIOP_NOTIFY_STAT,dati, sizeof(dati));
  printf("BIOPSIA: MOVIMENTO X \n");
  return TRUE;
}
         

 // Gestisce il movimento sull'asse X se necessario
 // Ritorna TRUE se il movimento è iniziato
 // FALSE se il movimento non è consentito/non è attivato
bool moveY(unsigned char* dati)
{
  if(generalConfiguration.biopsyCfg.movimento_y==FALSE) return FALSE;
  generalConfiguration.biopsyCfg.movimento_y = FALSE;
  BiopsySetY();
  dati[_BP_MOTION]=2;
  mccBiopsyNotify(1,BIOP_NOTIFY_STAT,dati, sizeof(dati));
  printf("BIOPSIA: MOVIMENTO Y \n");
  return TRUE; 
}

/* 
   Questa funzione richiede il contenuto della variabile di stato 
   della Biopsia, nella versione DBT. 
   Lo STATUS è così configurato:

   #bit [0:2] ACCESSORIO   
   #bit 7: pulsante di sblocco
   
   La funzione è interna al driver, pertanto non 
   richiede l'uso della mutex
       
*/
bool BiopsyGetStat(void)
{  
  unsigned char buffer[4];
  
  Ser422SendRaw(0x8E, 0x5B, 0, buffer, 5);    
  if(buffer[0]==0xE) 
  {
    // Gestione pressione del pulsante di sblocco
    if(buffer[2]&0x80) generalConfiguration.biopsyCfg.sbloccoReq = TRUE;
    else generalConfiguration.biopsyCfg.sbloccoReq = FALSE;
    
    // Gestione dell'identificazione dell'accessorio
    generalConfiguration.biopsyCfg.adapterId= buffer[2] & 0x07;
    return TRUE;
  }
  
  return FALSE;
}

/*
  La funzione chiede info sulla Z corrente del posizionamento
*/
bool biopsyGetZ(void)
{ 
  unsigned char buffer[4];
  unsigned short needleZ;
  
  // Si utilizza la funzione RAW perchè il comando è fuori protocollo
  Ser422SendRaw(0x90, 0x71, 0, buffer, 10);
  
  if(buffer[0]==0) return FALSE;
  generalConfiguration.biopsyCfg.needleZ = buffer[1]*256 + buffer[2];
  
  return TRUE;
}

/*
  La funzione chiede info sulla X corrente del posizionamento
*/
bool biopsyGetX(void)
{ 
  unsigned char buffer[4];
  unsigned short needleZ;
  
  // Si utilizza la funzione RAW perchè il comando è fuori protocollo
  Ser422SendRaw(0x90, 0x74, 0, buffer, 10);
  
  if(buffer[0]==0) return FALSE;
  generalConfiguration.biopsyCfg.needleX = buffer[1]*256 + buffer[2];
  
  return TRUE;
}

bool biopsyGetY(void)
{ 
  unsigned char buffer[4];
  unsigned short needleZ;
  
  // Si utilizza la funzione RAW perchè il comando è fuori protocollo
  Ser422SendRaw(0x90, 0x75, 0, buffer, 10);
  
  if(buffer[0]==0) return FALSE;
  generalConfiguration.biopsyCfg.needleY = buffer[1]*256 + buffer[2];
  
  return TRUE;
}

void BiopsySetX(void)
{
  unsigned char H,L;
  unsigned char buffer[4];
  
  H = (unsigned char) (generalConfiguration.biopsyCfg.target_x>>8);
  L = (unsigned char) (generalConfiguration.biopsyCfg.target_x&0x00FF);
  Ser422SendRaw(0x4E, H, L, buffer, 10);
  
  if(buffer[0]==0) generalConfiguration.biopsyCfg.movimento_x = TRUE; // Ripete
  biopTestPosition = TRUE;
}

void BiopsySetY(void)
{
  unsigned char H,L;
  unsigned char buffer[4];
  
  H = (unsigned char) (generalConfiguration.biopsyCfg.target_y>>8);
  L = (unsigned char) (generalConfiguration.biopsyCfg.target_y&0x00FF);
  Ser422SendRaw(0x4F, H, L, buffer, 10);
  
  if(buffer[0]==0) generalConfiguration.biopsyCfg.movimento_y = TRUE; // Ripete
  biopTestPosition = TRUE;

}

void BiopsySetStepUpZ(void)
{
  unsigned char buffer[4];
  
  Ser422SendRaw(0x8D, 0x74, 0, buffer, 10);  
  if(buffer[0]==0) generalConfiguration.biopsyCfg.stepUp_z = TRUE; // Ripete
  biopTestPosition = FALSE;

}

void BiopsySetStepDwnZ(void)
{
  unsigned char buffer[4];
  
  Ser422SendRaw(0x8D, 0x75, 0, buffer, 10);  
  if(buffer[0]==0) generalConfiguration.biopsyCfg.stepDwn_z = TRUE; // Ripete
  biopTestPosition = FALSE;

}

void BiopsySetZ(void)
{
  unsigned char H,L;
  unsigned char buffer[4];
  
  H = (unsigned char) (generalConfiguration.biopsyCfg.target_z>>8);
  L = (unsigned char) (generalConfiguration.biopsyCfg.target_z&0x00FF);
  Ser422SendRaw(0x50, H, L, buffer, 10);
  
  if(buffer[0]==0) generalConfiguration.biopsyCfg.movimento_z = TRUE; // Ripete
  biopTestPosition = TRUE;

}

/* 
   La funzione seguente permette di leggere revisione  e checksum della 
   torretta in un unico comando. 
   La funzione restituisce FALSE in caso uno dei comandi 
   non dovesse ricevere risposta. In tal caso entrambi
   i codici verranno azzerati
      generalConfiguration.biopsyCfg.checksum = 0;
      generalConfiguration.biopsyCfg.revisione = 0;
*/
bool BiopsyGetRevision(void)
{
  unsigned char H,L,i;
  unsigned char buffer[4];
  
  // Ripete il comando per un certo tempo per essere certi che 
  // non ci sia un problema semplicemente di comunicazione
  i = 20;
  buffer[0]=0;
  while(--i)
  {
    Ser422SendRaw(0x90, 0x72, 0, buffer, 10);  
    if(buffer[0]!=0x10) continue;
    break;
  }
  if(buffer[0]!=0x10) 
  {
    generalConfiguration.biopsyCfg.checksum_h = 0;
    generalConfiguration.biopsyCfg.checksum_l = 0;
    generalConfiguration.biopsyCfg.revisione = 0;
    return FALSE;
  }
  generalConfiguration.biopsyCfg.checksum_h = buffer[1];
  generalConfiguration.biopsyCfg.checksum_l = buffer[2];

  // Acquisizione revisione corrente
  i = 20;
  buffer[0]=0;
  while(--i)
  {
    Ser422SendRaw(0x90, 0x73, 0, buffer, 10);  
    if(buffer[0]!=0x10) continue;
    break;
  }
  if(buffer[0]!=0x10) 
  {
    generalConfiguration.biopsyCfg.checksum_h = 0;
    generalConfiguration.biopsyCfg.checksum_l = 0;
    generalConfiguration.biopsyCfg.revisione = 0;
    return FALSE;
  }
    
  generalConfiguration.biopsyCfg.revisione = buffer[2];  
  
  printf("BIOPSIA - REVISIONE:%d, CHK=%x%x\n",generalConfiguration.biopsyCfg.revisione,generalConfiguration.biopsyCfg.checksum_h,generalConfiguration.biopsyCfg.checksum_l);
  return TRUE;
}

/* ATTIVAZIONE MOVIMENTI DA THREAD ESTERNA
    Occorre utilizzare la mutex per evitare conflitti
*/
bool biopsySetXYZ(unsigned short X, bool XGO, unsigned short Y, bool YGO, unsigned short Z, bool ZGO)
{
  if(generalConfiguration.biopsyCfg.movimento) return FALSE; // E' già in corso
  
  printf("TGX:%d, TGY:%d, TGZ:%d\n",X,Y,Z);

  _mutex_lock(&(CONTEST.pollinglist_mutex));
  if(XGO)
  {
    generalConfiguration.biopsyCfg.target_x = X;
    generalConfiguration.biopsyCfg.movimento_x=TRUE;
    generalConfiguration.biopsyCfg.movimento=TRUE;
    biopTestPosition = TRUE;

  }
  if(YGO)
  {
    generalConfiguration.biopsyCfg.target_y = Y;
    generalConfiguration.biopsyCfg.movimento_y=TRUE;
    generalConfiguration.biopsyCfg.movimento=TRUE;
    biopTestPosition = TRUE;
 }
  if(ZGO)
  {
    // Determina se muove su o giù
    if(Z>generalConfiguration.biopsyCfg.needleZ) generalConfiguration.biopsyCfg.z_up = FALSE;
    else generalConfiguration.biopsyCfg.z_up = TRUE;
    
    generalConfiguration.biopsyCfg.target_z = Z;
    generalConfiguration.biopsyCfg.movimento_z=TRUE;
    generalConfiguration.biopsyCfg.movimento=TRUE;
    biopTestPosition = TRUE;
  }
     
  _mutex_unlock(&(CONTEST.pollinglist_mutex));
  
  return TRUE;
}

bool  biopsySetZLimit(unsigned char zlimit)
{
  unsigned char buffer[4];
  
  if(generalConfiguration.biopsyCfg.movimento) return FALSE; // E' già in corso
  
  _mutex_lock(&(CONTEST.pollinglist_mutex));
  Ser422SendRaw(0xCD, 0xF1, zlimit, buffer, 1);
  generalConfiguration.biopsyCfg.zlimit = zlimit; 
  _mutex_unlock(&(CONTEST.pollinglist_mutex));
  return TRUE;
}
bool  biopsySetZLesione(unsigned char zlesione)
{
  unsigned char buffer[4];
  
  if(generalConfiguration.biopsyCfg.movimento) return FALSE; // E' già in corso
  
  _mutex_lock(&(CONTEST.pollinglist_mutex));
  Ser422SendRaw(0xCD, 0xF0, zlesione, buffer, 1);
  generalConfiguration.biopsyCfg.zlesione = zlesione; 
  _mutex_unlock(&(CONTEST.pollinglist_mutex));
  return TRUE;
}
bool  biopsySetLago(unsigned char lago)
{
  unsigned char buffer[4];
  
  if(generalConfiguration.biopsyCfg.movimento) return FALSE; // E' già in corso
  
  _mutex_lock(&(CONTEST.pollinglist_mutex));
  Ser422SendRaw(0xCD, 0xF2, lago, buffer, 1);
  generalConfiguration.biopsyCfg.lago = lago; 
  _mutex_unlock(&(CONTEST.pollinglist_mutex));
  return TRUE;
}

void biopsyReset(void)
{
  _mutex_lock(&(CONTEST.pollinglist_mutex));
  verifyBiopsyDataInit = TRUE;
  _mutex_unlock(&(CONTEST.pollinglist_mutex));
}

void  biopsyStepZ(unsigned char dir, unsigned char val)
{  
  if(dir==1)
  {// Incremento
    generalConfiguration.biopsyCfg.stepDwn_z = TRUE;
  }else
  {// Decremento
    generalConfiguration.biopsyCfg.stepUp_z = TRUE;
  }  
  generalConfiguration.biopsyCfg.movimento=TRUE;

}

/*
  Funzione configuratrice biopsia
  buffer[0]: offsetZ
  buffer[1]: offsetPad
  buffer[2]: margine risalita compressore
  buffer[3]: margine posizionamento
*/
bool config_biopsy(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len){
  
  printf("AGGIORNAMENTO CONFIG BIOPSIA:\n");
  
  printf(" offsetZ=%d\n",buffer[0]);
  printf(" offsetPad=%d\n",buffer[1]);
  printf(" margineRisalita=%d\n",buffer[2]);
  printf(" marginePosizionamento=%d\n",buffer[3]);
  
  generalConfiguration.biopsyCfg.conf.offsetZ = buffer[0];
  generalConfiguration.biopsyCfg.conf.offsetPad = buffer[1];
  generalConfiguration.biopsyCfg.conf.margineRisalita = buffer[2];
  generalConfiguration.biopsyCfg.conf.marginePosizionamento = buffer[3];
  
  // L'impostazione dei margini di posizionamento richiede un aggiornamento 
  // dei parametri del compressore chge si deve adeguare a nuovi vincoli
  pcb215ForceUpdateData();
  
  // Effettua il reset della biopsia
  biopsyReset();
  
  return true;
}

/* EOF */
 

