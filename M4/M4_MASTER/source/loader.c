#define _LOADER_C
#include "dbt_m4.h" 

bool loaderCommand(unsigned char comando, unsigned char dato, unsigned char tentativi);
bool loaderRead(unsigned char address, unsigned char banco, unsigned char size, unsigned short *buffer,int tentativi);
bool loaderWrite(unsigned char address, unsigned char banco, unsigned char size, unsigned short buffer,int tentativi);
bool loaderWriteBlk(unsigned short val, bool init, bool isProgramSegment, unsigned short currPc);
bool loaderWriteConfigWord(unsigned short val);

/*
  ATTIVAZIONE LOADER INDIRIZZO <ADDRESS>
*/

/*
  Invio di un comando verso un Loader già attivato!!
*/
bool loaderCommand(unsigned char comando, unsigned char dato, unsigned char tentativi)
{
  _Ser422_Command_Str frame;
  
  int attempt=tentativi;
  while(attempt--)
  {
     // Preparazione pacchetto
    frame.address = Loader.target;  
    frame.attempt = tentativi;
    frame.cmd=SER422_COMMAND;
    
    frame.data1=comando;
    frame.data2=dato;
    Ser422LoaderSend(&frame);
    if(frame.retcode==SER422_COMMAND_OK) return TRUE;
    else if(frame.retcode==SER422_BUSY) 
    {
      _time_delay(50);
      attempt=tentativi;
    }
  } 
  return FALSE;
  
}

bool loaderRead(unsigned char address, unsigned char banco, unsigned char size, unsigned short* buffer,int tentativi)
{
  _Ser422_Command_Str frame;
  unsigned short usval=0;
  
  // Legge byte basso
  int attempt=tentativi;
  while(attempt--)
  {
     // Preparazione pacchetto
    frame.address = Loader.target;  
    frame.attempt = tentativi;
    frame.cmd=SER422_READ;    
    frame.data1=address;
    frame.data2=banco;
    Ser422LoaderSend(&frame);
    if(frame.retcode==SER422_READ_OK) break;
  } 
  if(frame.retcode!=SER422_READ_OK) return FALSE;
  if(size==0)
  {
    *buffer = (unsigned short) frame.data2;
    return TRUE;
  }
  else usval= frame.data2;
  
  // Legge byte alto
  attempt=tentativi;
  while(attempt--)
  {
     // Preparazione pacchetto
    frame.address = Loader.target;  
    frame.attempt = tentativi;
    frame.cmd=SER422_READ;    
    frame.data1=address+1;
    frame.data2=banco;
    Ser422LoaderSend(&frame);
    if(frame.retcode==SER422_READ_OK)
    {
      usval += frame.data2*256;
      *buffer = usval;
      return TRUE;
    }
  } 
  
  return FALSE;  
}

bool loaderWrite(unsigned char address, unsigned char banco, unsigned char size, unsigned short buffer,int tentativi)
{
  _Ser422_Command_Str frame;
  unsigned short usval=0;
  
  // scrive byte basso
  int attempt=tentativi;
  while(attempt--)
  {
     // Preparazione pacchetto
    frame.address = Loader.target;  
    frame.attempt = tentativi;
    frame.cmd=SER422_WRITE;    
    frame.data1=address;
    frame.data2=(unsigned char) buffer;
    Ser422LoaderSend(&frame);
    if(frame.retcode==SER422_WRITE_OK) break;
  } 
  if(frame.retcode!=SER422_WRITE_OK) return FALSE;
  if(size==0) return TRUE;
  
  // Scrive byte alto
  attempt=tentativi;
  while(attempt--)
  {
     // Preparazione pacchetto
    frame.address = Loader.target;  
    frame.attempt = tentativi;
    frame.cmd=SER422_WRITE;    
    frame.data1=address+1;
    frame.data2=(unsigned char) (buffer>>8);
    Ser422LoaderSend(&frame);
    if(frame.retcode==SER422_WRITE_OK) return TRUE;
  } 
  
  return FALSE;  
}

bool loaderWriteBlk(unsigned short val, bool init, bool isProgramSegment, unsigned short currPc)
{
  _Ser422_Command_Str frame;
  unsigned short pc;
  
  // Preparazione pacchetto
  while(1)
  {
    frame.address = Loader.target;  
    frame.attempt = 1;
    frame.cmd=SER422_SPECIAL;    
    frame.data1=(unsigned char) val;
    frame.data2=(unsigned char) (val>>8);
    if(isProgramSegment) frame.data2 |= 0x40; // Bit per il Program segment
    else frame.data2 &= 0xBF; // Bit per il Data segment 
    if(!init)  frame.data2 |= 0x80; // no Init
    else frame.data2 &= 0x7F; // Init
    Ser422LoaderSend(&frame);
    if(frame.retcode==SER422_COMMAND_SPECIAL_OK)
    {
      // Controlla il caso Busy
      if((frame.data1==0xFF)&&(frame.data2==0)) _time_delay(10);
      else return TRUE;
    }
    else 
    { // In caso di fallimento si verifica se il comando è stato effettivamente
      // eseguito rileggendo il PC e controllando se risulta quello corrente +1
      _time_delay(50);
      if(loaderRead(REG16_PROG_PC,&pc,5)==FALSE) return FALSE;
      printf("PC RILETTO:%x CURRENT ADDRESS:%x\n", pc, currPc);
      if(pc==currPc+1) return TRUE;
      else if(pc<currPc) return FALSE;
      else if(pc>currPc+1) return FALSE;      
      // Ripete la scrittura
      printf("RIPETIZIONE SCRITTURA ..\n");
    }
  }
}

// Comando in Broadcast senza attesa di risposta
void loaderExit(bool driverRun)
{
    _Ser422_Command_Str frame;
    
    frame.address = 0x1F;  
    frame.attempt = 5;
    frame.cmd=SER422_COMMAND;    
    frame.data1=SERCMD_BROADCAST_EXIT;
    frame.data2=0;
    Ser422LoaderSend(&frame);
    
    ser422SetBaud(BAUDRATE);
    _time_delay(100);
    if(driverRun) Ser422DriverSetReadyAll(5000);
    return ;
}

bool loaderActivation(unsigned char address, unsigned char uC)
{
   _Ser422_Command_Str frame;
 
   printf("ATTIVAZIONE LOADER: BLOCCO ALIMENTAZIONE..\n");
   // Impostazione Segnale XRAY_ENA su Bus Hardware
    _mutex_lock(&output_mutex);
    SystemOutputs.CPU_LOADER_PWR_ON=1;   // Attivazione blocco alimentazione
    _EVSET(_EV0_OUTPUT_CAMBIATI);         
    _mutex_unlock(&output_mutex);
    
   printf("ATTIVAZIONE LOADER: BLOCCO DRIVERS..\n");
   
   // Vengono bloccati tutti i driver, se può .. 
   Ser422DriverFreezeAll(4000);
   _time_delay(500); // Scarico messaggi in coda

   printf("EXIT LOADERS..\n");

   // Prima di eseguire l'attivazione fa uscire tutti (nel caso
   // di ripetizione si evita un blocco)
   ser422SetBaud(BAUDLOADER);
   _time_delay(100);
   loaderExit(FALSE);
   printf("ATTIVAZIONE LOADERS..\n");
   
   // Prosegue con l'attivazione del particolare Loader
   Loader.target = address;
   Loader.uC = uC;
   if(loaderCommand(SERCMD_LOADER,uC,5)==FALSE) return FALSE;
   Loader.isActive = FALSE;
  
  // Il driver remoto si è impostato a 19200 BR
  // Deve essere inviata la conferma a tale velocità
  // Imposta Baud Rate
  ser422SetBaud(BAUDLOADER);
  _time_delay(100);
  
  // Invia nuovamente il comando di attivazione
  // a conferma
  if(loaderCommand(SERCMD_LOADER,uC,5)==FALSE) return FALSE;
  Loader.isActive = TRUE;
  printf("LOADER ATTIVATO A 19200 BAUD.\n");
  
  printf("LOADER: LETTURA CONFIGURAZIONE ..\n");
  
  // Si procede con l'acquisizione della configurazione del target
  if(loaderCommand(SERCMD_SET_CONFIG,0,5)==FALSE){
    printf("FALLITO!!!\n");
    return FALSE;
  }
  
  // Lettura configurazione da memoria Loader
  printf("LETTURA CONFIGURAZIONE ID0..");
  if(loaderRead(REG16_PROG_ID0,&Loader.cfg.id0,5)==FALSE) {
    printf("FALLITO!!!\n");
    return FALSE;
  }
  printf("%x\nLETTURA CONFIGURAZIONE ID1.. ",Loader.cfg.id0);
  
  if(loaderRead(REG16_PROG_ID1,&Loader.cfg.id1,5)==FALSE) {
    printf("FALLITO!!!\n");
    return FALSE;
  }
  printf("%x\nLETTURA CONFIGURAZIONE ID2.. ",Loader.cfg.id1);

  // Lettura configurazione da memoria Loader
  if(loaderRead(REG16_PROG_ID2,&Loader.cfg.id2,5)==FALSE) {
    printf("FALLITO!!!\n");
    return FALSE;
  }
  printf("%x\nLETTURA CONFIGURAZIONE ID3.. ",Loader.cfg.id2);

  // Lettura configurazione da memoria Loader
  if(loaderRead(REG16_PROG_ID3,&Loader.cfg.id3,5)==FALSE) {
    printf("FALLITO!!!\n");
    return FALSE;
  }
  printf("%x\nLETTURA CONFIGURAZIONE DEV-ID.. ",Loader.cfg.id3);

  // Lettura configurazione da memoria Loader
  if(loaderRead(REG16_PROG_DEVICEID,&Loader.cfg.devId,5)==FALSE) {
    printf("FALLITO!!!\n");
    return FALSE;
  }
  printf("%x\nLETTURA CONFIGURAZIONE DEV-CODE.. ",Loader.cfg.devId);
  
  // Lettura configurazione da memoria Loader
  if(loaderRead(REG16_PROG_DEVCOD,&Loader.cfg.devCod,5)==FALSE) {
    printf("FALLITO!!!\n");
    return FALSE;
  }
  printf("%x\nLETTURA CONFIGURAZIONE DEV-REV.. ",Loader.cfg.devCod);

  // Lettura configurazione da memoria Loader
  if(loaderRead(REG_PROG_DEVREV,&Loader.cfg.devRev,5)==FALSE) {
    printf("FALLITO!!!\n");
    return FALSE;
  }
  printf("%x\nLETTURA CONFIG-WORD.. ",Loader.cfg.devRev);
  _time_delay(100); 
  
  // Lettura configurazione da memoria Loader
  if(loaderRead(REG16_PROG_CONFWORD,&Loader.cfg.config,5)==FALSE) {
    printf("FALLITO!!!\n");
    return FALSE;
  }
  printf("%x\n",Loader.cfg.config);
    
  // Attivazione completata con successo
  Loader.isActive = TRUE;
  
  return TRUE;
}

/*
  Comando di cancellazione integrale del chip.
  Al termine di questo comando il chip è in modo ICSP
  e la memoria FLASH + EEPROM + CONFIG è completamente cancellata.
  L'operazione non può essere annullata.
  Questo comando deve sempre precedere la programmazione del dispositivo
*/
bool loaderChipErase(void)
{
  // Eseguibile solo con Loader attivo
  if(Loader.isActive==FALSE) return FALSE;
 
  printf("CANCELLAZIONE DELLA FLASH .. \n");
  // Procede con la cancellazione del chip
  if(loaderCommand(SERCMD_CHIP_ERASE,0,5)==FALSE) return FALSE;
  _time_delay(100); // Attesa di 100ms prima di proseguire
  
  return TRUE;
}

 

/*
  Invio blocco di programmazione
*/
bool loaderLoadSegment(_addrStr* blk)
{
  int i;
  static unsigned short curAddress = 0;
  static unsigned short baseAddress=0;
  unsigned short pc;
  bool isProgram=FALSE;
  bool isInit=FALSE;
  
  // Verifica il tipo di blocco da scrivere
  if(blk->startAddr<0x2000) isProgram=TRUE;
 
  // Init del blocco
  if((blk->startAddr==0x2100)||(blk->startAddr==0)) 
  {
    if(isProgram) printf("INIZIO SCRITTURA BLOCCHI IN FLASH ..\n");
    else printf("INIZIO SCRITTURA BLOCCHI EEPROM ..\n");    
    baseAddress = curAddress = blk->startAddr;
    isInit=TRUE;
  }

  // Calcolo del numero di incrementi del PC per impostare 
  // il corretto indirizzo di scrittura. Il comando di 
  // incremento accetta un massimo di 255 incrementi totali. Per incrementi 
  // superiori bisogna iterare l'operazione.
  int incremento;
  while(1)
  {
    // Calcolo incrementi con massimo 255
    incremento = blk->startAddr - curAddress;
    if(incremento==0) break;
    if(incremento<0) return FALSE;
    if(incremento>255) incremento = 255;
    if(loaderCommand(SERCMD_INC_PC,(unsigned char) incremento,20)==FALSE) return FALSE;
    curAddress += incremento;    
  }
  
  // Scrive l'intero blocco nella zona di memoria corretta
  for(i=0; i< blk->len; i++)
  {
    if(loaderWriteBlk(blk->val[i],isInit,isProgram,(curAddress-baseAddress))==FALSE) return FALSE;    
    isInit=FALSE;
    curAddress++;
  }  
  
  // Rilegge il PC per verificare che sia allineato
  if(loaderRead(REG16_PROG_PC,&pc,5)==FALSE) return TRUE;
  if(pc!=curAddress-baseAddress) return FALSE;    
  return TRUE;
}  
  
  
bool loaderWriteConfig(unsigned char* data)
{
  unsigned short usval;
   
   // Scrive ID0
   usval = data[0]+data[1]*256;
   if(loaderWrite(REG16_PROG_LOAD,usval,5)==FALSE) return FALSE;
   if(loaderCommand(SERCMD_LOAD_ID,0,5)==FALSE) return FALSE;
   _time_delay(50);

   // Scrive ID1
   usval = data[2]+data[3]*256;
   if(loaderWrite(REG16_PROG_LOAD,usval,5)==FALSE) return FALSE;

   if(loaderCommand(SERCMD_LOAD_ID,1,5)==FALSE) return FALSE;     
   _time_delay(50);

   // Scrive ID2
   usval = data[4]+data[5]*256;
   if(loaderWrite(REG16_PROG_LOAD,usval,5)==FALSE) return FALSE;
   if(loaderCommand(SERCMD_LOAD_ID,1,5)==FALSE) return FALSE;
   _time_delay(50);

   // Scrive ID3
   usval = data[6]+data[7]*256;
   if(loaderWrite(REG16_PROG_LOAD,usval,5)==FALSE) return FALSE;
   if(loaderCommand(SERCMD_LOAD_ID,1,5)==FALSE) return FALSE;
   _time_delay(50);

   // Scrive CONFIG WORD
   usval = data[8]+data[9]*256;
   if(loaderWrite(REG16_PROG_LOAD,usval,5)==FALSE) return FALSE;
   if(loaderCommand(SERCMD_LOAD_CFGW,0,5)==FALSE) return FALSE;
   _time_delay(50);
   
   return TRUE;
}

  
/* EOF */
