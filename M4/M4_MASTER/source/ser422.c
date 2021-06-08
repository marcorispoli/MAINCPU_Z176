#define _SER422_C
#include "dbt_m4.h" 

///////////////////////////////////////////////////////////////////////////////
/*

GESTIONE ERRORI DEL MODULO:

Il Modulo non gestisce errori dinamici.
Eventuali problemi di comunicazione vengono gestiti dai livelli superiori
di codice.
Se fallisce l'inizializzazione, il programma termina.
NON SONO AMMESSI ERRORI IN QUESTO DRIVER
*/
///////////////////////////////////////////////////////////////////////////////
  /* Canale RS485 da usare */
  #define RS485_CHANNEL "ittyc:" // UART 2
  #define RS485_TXENA_REV_A LWGPIO_PIN_PTD3 // Porta D, Pin 3
  #define RS485_TXENA_REV_B LWGPIO_PIN_PTD2 // Porta D, Pin 2

  // Timing Protocollo
  #define SER422_WAITING_RX  20 //10 // ms  DBG
  #define SER422_WAITING_TIMEOUT  20 // ms  

  // Variabili private del modulo
  _Ser422_Command_Str* pSer422PendingCommand; // Puntatore a blocco comando Applicazione  

MQX_FILE_PTR rs485_dev = NULL;

void ser422SetBaud(int br)
{
  // Imposta Baud Rate
   int baud=br;
   ioctl( rs485_dev, IO_IOCTL_SERIAL_SET_BAUD, &baud );
   
}

void ser422_driver(uint32_t initial_data)
{
  bool disable_rx = TRUE;
  int rx_len;
  bool data_changed;
  int ciclo;
  unsigned char crc;
  unsigned char Target;
  unsigned char Attempt;
  unsigned char RepeatAttempt;  
  MQX_FILE_PTR rs485_gpio = NULL;
  unsigned char rx_buffer[10];
  unsigned char tx_buffer[10];
  bool rawTx =FALSE;         // Invio messgagi RAW

    _EVCLR(_EV0_COM_STARTED);
    displaySeriale = false;

    // Inizializzazione delle mutex
    if (_mutex_init(&ser422_stat_mutex, NULL) != MQX_OK)  
    {
      printf("SER422 ERRORE INIT MUTEX. FINE PROGRAMMA");
      _mqx_exit(-1);
    }
    
   // Init registro di stato
    _mutex_lock(&ser422_stat_mutex);
    {
        Ser422_Stat.timeout=0;
        Ser422_Stat.busy=1;
        Ser422_Stat.frame_err=0;
        Ser422_Stat.received=0;
        Ser422_Stat.slave_err=0;
    }
    _mutex_unlock(&ser422_stat_mutex);

  
   // Impostazione segnale di attivazione trasmissione basso (disabilita trasmissione) 
   LWGPIO_STRUCT rs485TxEnaA;
   if (lwgpio_init(&rs485TxEnaA,RS485_TXENA_REV_A,LWGPIO_DIR_OUTPUT,LWGPIO_VALUE_LOW) != TRUE)
    {
      printf("SER422 ERRORE INIT GPIO. FINE PROGRAMMA");
      _mqx_exit(-1);
    }

   LWGPIO_STRUCT rs485TxEnaB;
   if (lwgpio_init(&rs485TxEnaB,RS485_TXENA_REV_B,LWGPIO_DIR_OUTPUT,LWGPIO_VALUE_LOW) != TRUE)
    {
      printf("SER422 ERRORE INIT GPIO. FINE PROGRAMMA");
      _mqx_exit(-1);
    }
   
   // Apertura UART
   rs485_dev = fopen( RS485_CHANNEL, (char const *)NULL ); 
   if( rs485_dev == NULL )
    {
      printf("SER422 ERRORE APRTURA COM. FINE PROGRAMMA");
      _mqx_exit(-1);
    }

   // Disabilita UART in ricezione
   disable_rx = TRUE;
   ioctl( rs485_dev, IO_IOCTL_SERIAL_DISABLE_RX, &disable_rx );

   // Imposta Baud Rate
   int baud=BAUDRATE;
   ioctl( rs485_dev, IO_IOCTL_SERIAL_SET_BAUD, &baud );
   
   // Creazione coda processi sospesi
   ser422_send_queue =  _taskq_create(MQX_TASK_QUEUE_FIFO);
   if (ser422_send_queue  == NULL)
    {
      printf("SER422 ERRORE INIT CODA PROCESSI. FINE PROGRAMMA");
      _mqx_exit(-1);
    }


   ser422_cmd_queue =  _taskq_create(MQX_TASK_QUEUE_FIFO);
   if (ser422_cmd_queue  == NULL) 
    {
      printf("SER422 ERRORE CREAZIONE TASKQ. FINE PROGRAMMA");
      _mqx_exit(-1);
    }

   ser422_answ_queue =  _taskq_create(MQX_TASK_QUEUE_FIFO);
   if (ser422_answ_queue  == NULL) 
    {
      printf("SER422 ERRORE CREAZIONE TASKQ. FINE PROGRAMMA");
      _mqx_exit(-1);
    }

   printf("SER422: TASK PARTITO!\n");
   
   // Attivazione evento partenza driver seriale
    _EVSET(_EV0_COM_STARTED);

   // Attivazione ciclo di gestione comunicazione seriale
   while(1)
   {     

       // Verifica se ci sono nuovi processi in coda
       _mutex_lock(&ser422_stat_mutex);
       if(_taskq_get_value(ser422_send_queue)>0)
       {
          _taskq_resume(ser422_send_queue,FALSE); // only first task
       }else
       {
          Ser422_Stat.busy = 0; // Toglie il busy
       }
       _mutex_unlock(&ser422_stat_mutex);
       

      // Il Task si sospende fino alla prima richiesta utile
      // da parte del processo svegliato o di uno successivo
      _taskq_suspend(ser422_cmd_queue);

      // Preparazione Byte 0
      if(pSer422PendingCommand->address&0x80) rawTx=TRUE;
      else rawTx=FALSE;
    
      Target = pSer422PendingCommand->address & 0x1F;
      ((_Ser422_Addr_Str *)tx_buffer)->address = Target;
      ((_Ser422_Addr_Str *)tx_buffer)->cmd = pSer422PendingCommand->cmd;
      if(pSer422PendingCommand->isLoader) ((_Ser422_Addr_Str *)tx_buffer)->reserved = 1; // Loader
      else ((_Ser422_Addr_Str *)tx_buffer)->reserved = 0;// Standard
      
      // Preparazione restante buffer
      tx_buffer[1]=pSer422PendingCommand->data1;
      tx_buffer[2]=pSer422PendingCommand->data2;
      tx_buffer[3] = tx_buffer[0]^tx_buffer[1]^tx_buffer[2]; // CRC
 
      // Numero di tentativi di ritrasmissione dopo errore di frame
      Attempt = pSer422PendingCommand->attempt;
      RepeatAttempt = pSer422PendingCommand->confirmation_attempt;
        
      while(Attempt--)
      {
          // Imposta UART in blocking mode
          ioctl(rs485_dev,IO_IOCTL_SERIAL_SET_FLAGS,  NULL);
      
          // Svuota il buffer
          fflush( rs485_dev );
          ioctl( rs485_dev, IO_IOCTL_SERIAL_WAIT_FOR_TC, NULL );
         
          // Attivazione driver di trasmissione
          lwgpio_set_value(&rs485TxEnaA, LWGPIO_VALUE_HIGH);
          lwgpio_set_value(&rs485TxEnaB, LWGPIO_VALUE_HIGH);
  
          if(displaySeriale)  printf("TX0:%x TX1:%x TX2:%x TX3:%x\n",tx_buffer[0],tx_buffer[1],tx_buffer[2],tx_buffer[3]);

 //         if(pSer422PendingCommand->isLoader) printf("TX0:%x TX1:%x TX2:%x TX3:%x\n",tx_buffer[0],tx_buffer[1],tx_buffer[2],tx_buffer[3]);          
 //         if(pSer422PendingCommand->cmd == SER422_SPECIAL) printf("TX0:%x TX1:%x TX2:%x TX3:%x\n",tx_buffer[0],tx_buffer[1],tx_buffer[2],tx_buffer[3]);          
          
          
          // Driver si sospende fino ad avvenuta trasmissione
          write( rs485_dev, tx_buffer, 4 );
          fflush( rs485_dev );
          ioctl( rs485_dev, IO_IOCTL_SERIAL_WAIT_FOR_TC, NULL );

          // Imposta UART in non blocking mode
          ioctl(rs485_dev,IO_IOCTL_SERIAL_SET_FLAGS,  (void*) IO_SERIAL_NON_BLOCKING);
      
          // Svuota la coda di ricezione
          while(fread(rx_buffer,1,sizeof(rx_buffer),rs485_dev)!=0){}
      
          // Attivazione driver in ricezione
          lwgpio_set_value(&rs485TxEnaA, LWGPIO_VALUE_LOW);
          lwgpio_set_value(&rs485TxEnaB, LWGPIO_VALUE_LOW);
     
          // Attivazione UART 
          disable_rx = FALSE;
          ioctl( rs485_dev, IO_IOCTL_SERIAL_DISABLE_RX, &disable_rx );
   
          // Sospende la funzione per il tempo minimo necessario a ricevere i byte
          _time_delay(SER422_WAITING_RX);
          rx_len = _io_read(rs485_dev,rx_buffer,4);
      
          // Controllo di Frame corretto (Indirizzo chiamante, CRC, Lunghezza)
          if(rx_len==0)
          { 
             Ser422_Stat.timeout=1;
             if(Attempt){
                 _time_delay(SER422_WAITING_TIMEOUT);
                 continue;
             }else break;
          }          
          Ser422_Stat.timeout=0;
          if(displaySeriale)  printf(">> %x %x %x %x\n",rx_buffer[0],rx_buffer[1],rx_buffer[2],rx_buffer[3]);

          // Controllo numero byte e Indirizzo
          if((((_Ser422_Addr_Str*)(&rx_buffer[0]))->address!=Target)||(rx_len!=4))
          {
            Ser422_Stat.frame_err=1;
            _time_delay(SER422_WAITING_TIMEOUT); //--------------------------------------------------------------
            continue;// Riprova con un nuovo tentativo se necessario
          }

          // Controllo checksum del pacchetto ricevuto
          if(rx_buffer[0]^rx_buffer[1]^rx_buffer[2]^rx_buffer[3])
          {
            Ser422_Stat.frame_err=1;         
            if(Attempt){
                _time_delay(SER422_WAITING_TIMEOUT);
                continue;
            }else break;
          }

          Ser422_Stat.frame_err=0;

          // Verifica se trattasi di comando RAW
          if(rawTx) break;

          // Errore sul dato ricevuto in caso di scrittura registro
          if( (pSer422PendingCommand->cmd==SER422_WRITE) && ((pSer422PendingCommand->data1 != rx_buffer[1]) || (pSer422PendingCommand->data2 != rx_buffer[2]))  )
          {
              Ser422_Stat.frame_err=1;              
              if(Attempt){
                  _time_delay(SER422_WAITING_TIMEOUT);
                  continue;
              }else break;
          }

          // Verifica la condizione di ripetizione del comando richiesta da slave
          if((rx_buffer[1]==0xFF)&&(rx_buffer[2]==SER422_WAIT_FOR_CONFIRMATION))
          {
            if(RepeatAttempt--)
            {
              Attempt = pSer422PendingCommand->attempt;
              continue;
            }
          }
          
          // Ok messaggio arrivato correttamente o termine tentativi di riconferma
          break;
      } // Ripete per un certo numero di volte fino a fine errore/conferma


      // Casi di errore di frame: abort del comando
      if(Ser422_Stat.frame_err)
      {
        pSer422PendingCommand->retcode=SER422_ERRFRAME;    
        pSer422PendingCommand=NULL;    
        _taskq_resume(ser422_answ_queue,TRUE);
        _time_delay(40); //--------------------------------------------------------------
        continue;
      }

      // Casi di timeout: abort del comando
      if(Ser422_Stat.timeout)
      {
        pSer422PendingCommand->retcode=SER422_ERRTMO;    
        pSer422PendingCommand=NULL;    
        _taskq_resume(ser422_answ_queue,TRUE);
        continue;
      }
 
      if(rawTx)
      {
         rawTx=FALSE;
         pSer422PendingCommand->retcode=SER422_RAWOK;
         pSer422PendingCommand->data1 = rx_buffer[1];
         pSer422PendingCommand->data2 = rx_buffer[2];
         Ser422_Stat.received = 1;
         pSer422PendingCommand=NULL;
         _taskq_resume(ser422_answ_queue,TRUE);
         continue;
      }

      // Analisi del dato ricevuto dallo Slave 
      // Casi possibili
      if(pSer422PendingCommand->cmd==SER422_SPECIAL)
      {
        // Comando speciale: nessun protocollo sul dato ricevuto
         Ser422_Stat.slave_err=0;
         pSer422PendingCommand->retcode=SER422_COMMAND_SPECIAL_OK;
         pSer422PendingCommand->data1 = rx_buffer[1];         
         pSer422PendingCommand->data2 = rx_buffer[2];
      }
      else if(pSer422PendingCommand->cmd==SER422_READ)
      {
           // Comando di lettura registro
           Ser422_Stat.slave_err=0;
           pSer422PendingCommand->retcode=SER422_READ_OK;
           pSer422PendingCommand->data1 = rx_buffer[1];         
           pSer422PendingCommand->data2 = rx_buffer[2];
           if(displaySeriale)  printf(">> READ:%x %x\n",rx_buffer[1],rx_buffer[2]);
      }
      else if(pSer422PendingCommand->cmd==SER422_WRITE)
      {         
           // Comando di scrittura registro
           Ser422_Stat.slave_err=0;
           pSer422PendingCommand->retcode=SER422_WRITE_OK;
           pSer422PendingCommand->data1 = rx_buffer[1];         
           pSer422PendingCommand->data2 = rx_buffer[2];
           if(displaySeriale)  printf(">> WRITE:%x %x\n",rx_buffer[1],rx_buffer[2]);
      }  // Comando  
      else if(rx_buffer[1]==0xFF)
      {
        switch(*((_Ser422RetCodes_Enum*)(&rx_buffer[2])))
        {
        case SER422_BUSY:
            Ser422_Stat.slave_err=1;
            pSer422PendingCommand->retcode=SER422_BUSY;
          break;
        case SER422_ILLEGAL_ADDRESS:
            Ser422_Stat.slave_err=1;
            pSer422PendingCommand->retcode=SER422_ILLEGAL_ADDRESS;
         break;
        case SER422_ILLEGAL_DATA:
            Ser422_Stat.slave_err=1;
            pSer422PendingCommand->retcode=SER422_ILLEGAL_DATA;
          break;
        case SER422_ILLEGAL_FUNCTION:
           Ser422_Stat.slave_err=1;
           pSer422PendingCommand->retcode=SER422_ILLEGAL_FUNCTION;

          break;
        case SER422_UNIMPLEMENTED_FUNCTION:
           Ser422_Stat.slave_err=1;
           pSer422PendingCommand->retcode=SER422_ILLEGAL_FUNCTION;
          break;
       case SER422_RESERVED:
           Ser422_Stat.slave_err=1;
           pSer422PendingCommand->retcode=SER422_RESERVED;
          break;
        case SER422_WAIT_FOR_CONFIRMATION:
           Ser422_Stat.slave_err=1;
           pSer422PendingCommand->retcode=SER422_WAIT_FOR_CONFIRMATION;
          break;
        default: // Dati generici
           Ser422_Stat.slave_err=0;
           pSer422PendingCommand->retcode=SER422_DATA;
           pSer422PendingCommand->data1 = rx_buffer[1];         
           pSer422PendingCommand->data2 = rx_buffer[2];
          break;
        }
      }else if((rx_buffer[1]==0)&&(rx_buffer[2]== SER422_COMMAND_OK))
      {// Comando eseguito
           Ser422_Stat.slave_err=0;
           pSer422PendingCommand->retcode=SER422_COMMAND_OK;
           pSer422PendingCommand->data1 = 0;  
           pSer422PendingCommand->data2 = 0;  
      }
       else
      {// Dati generici
           Ser422_Stat.slave_err=0;
           pSer422PendingCommand->retcode=SER422_DATA;
           pSer422PendingCommand->data1 = rx_buffer[1];         
           pSer422PendingCommand->data2 = rx_buffer[2];
      }

      // Diagnostica connessione
      pSer422PendingCommand->ret_attempt = Attempt+1;
      pSer422PendingCommand->ret_conf_attempt = RepeatAttempt+1;
      Ser422_Stat.received = 1;

      // Fine comando
      pSer422PendingCommand=NULL;
      _taskq_resume(ser422_answ_queue,TRUE);
   } // while thread di trasmissione
   

}

/* 
unsigned char PCB240SendCommand(_PCB240_Command_Str* pCmd,  enumPCB240Flags flags)

Parametri:
 

Return: 

Note di funzionamento:


    In caso di errore:
    
    In caso di Successo:

Aut: M. Rispoli
Data: 6/09/2014
*/
void Ser422Send(_Ser422_Command_Str* pCmd, _Ser422Flags_Enum flags,_task_id id)
{
  // La funzione si sospende in coda ai processi che vogliono inviare dati
  _mutex_lock(&ser422_stat_mutex);
  if(Ser422_Stat.busy)
  {
    _mutex_unlock(&ser422_stat_mutex);
    _taskq_suspend(ser422_send_queue);
    // Il processo è stato svegliato
    _mutex_lock(&ser422_stat_mutex);
  }
  memset(&Ser422_Stat,0,sizeof(Ser422_Stat));
  Ser422_Stat.busy = 1;
  pCmd->isLoader = FALSE;
  pSer422PendingCommand = pCmd;
  
  _mutex_unlock(&ser422_stat_mutex);
  
  // Viene svegliato e messo in ready il task di trasmissione
  _taskq_resume(ser422_cmd_queue,TRUE);
  // Se la funzione è bloccante, il task richiedente si sospende
  // fino al completamento del ciclo di TX-RX o fino a errore
  if(SER422_BLOCKING==flags)
     _taskq_suspend(ser422_answ_queue);  
  return; 

}

/*
      INVIO PACCHETTO LOADER
*/
void Ser422LoaderSend(_Ser422_Command_Str* pCmd)
{
  // La funzione si sospende in coda ai processi che vogliono inviare dati
  _mutex_lock(&ser422_stat_mutex);
  if(Ser422_Stat.busy)
  {
    _mutex_unlock(&ser422_stat_mutex);
    _taskq_suspend(ser422_send_queue);
    // Il processo è stato svegliato
    _mutex_lock(&ser422_stat_mutex);
  }
  memset(&Ser422_Stat,0,sizeof(Ser422_Stat));
  Ser422_Stat.busy = 1;
  pCmd->isLoader = TRUE;
  pSer422PendingCommand = pCmd;
  pCmd->isLoader=TRUE;
  _mutex_unlock(&ser422_stat_mutex);
  
  // Viene svegliato e messo in ready il task di trasmissione
  _taskq_resume(ser422_cmd_queue,TRUE);

  // Si sospende fino al termine del comando  
  _taskq_suspend(ser422_answ_queue);  
 
  return; 

}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//                      FUNZIONI DI USO COMUNE NEI DRIVER
/////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/*
bool TestRegisterList(_DeviceAppRegister_Str* pList)
        La funzione testa la regolarità della lista passata;
        Questa funzione è interna al modulo e non viene usata dall'applicazione
PARAM:
        pList: puntatore alla lista da verificare
RETURN:
      TRUE: Lista corretta
      FALSE: Lista non corretta

Autore: M. Rispoli
Data: 17/09/2014
*/
//////////////////////////////////////////////////////////////////////////////
bool TestRegisterList(_DeviceAppRegister_Str* pList, _DeviceContext_Str* contest)
{
  unsigned char crc;
  unsigned char i;
    
  if(pList==NULL) return TRUE;
  if(pList->nitem == 0) return FALSE;
  if(pList->nitem >= contest->nregisters) return FALSE;
  
  // Controllo CRC
  for(crc=pList->crc, i=0; i<pList->nitem; i++)
  {
    crc^=pList->id[i];
    crc^=_LB(pList->val[i]);
    crc^=_HB(pList->val[i]);
  }
  if(crc) return FALSE;
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
/*
_SER422_Error_Enum WriteRegister(unsigned char id, unsigned short data, unsigned char attempt,_DeviceContext_Str* contest);

        Scrive sul target il registro con ID == id, effettuando
        un numero massimo di tentativi pari ad <attempt>.
        La funzione effettua una doppia scrittura nel caso di registro
        a 16 bit. 
        Se la scrittura va a buon fine, il contenuto PCB215_Registers[id]
        viene aggiornato con il valore corrente;
        Se il registro è dichiarato Non volatile (_NVL)
        la funzione controlla il contenuto locale e lo confronta con il valore
        da impostare; se tali valori sono uguali la funzione esce con successo
        senza impegnare la seriale.

PARAM:
        id: ID del registro da scrivere 
        data: valore da caricare nel registro
        attempt: numero massimo di tentativi di lettura prima di fallire
RETURN:
        _SER422_NO_ERROR = successo
        _SER422_INVALID_FUNCDATA: se ID punta fuori dalla lista
        _SER422_COMMUNICATION_TMO: esauriti i numeri di tentativi
        _SER422_INVALID_ACCESSMODE: scrittura di un registro Read Only

Autore: M. Rispoli
Data: 24/10/2014
*/
//////////////////////////////////////////////////////////////////////////////
_SER422_Error_Enum Ser422WriteRegister(unsigned char id, unsigned short data, unsigned char attempt, _DeviceContext_Str* contest)
{
  _Ser422_Command_Str frame;

  // ID non valido
  if(id>= contest->nregisters) return _SER422_INVALID_FUNCDATA;
  
  frame.address = contest->address;
  
  // Controllo modalità di accesso al registro
  if(contest->pReg[id].banco==_BNK23) return _SER422_INVALID_ACCESSMODE;
  if(contest->pReg[id].access==_RD) return _SER422_INVALID_ACCESSMODE;
  
  // Controlla se il valore locale è cambiato, ma solo se il registro
  // Non è volatile
  if((contest->pReg[id].val.val16==data)&&(contest->pReg[id].vl==_NVL))
    return _SER422_NO_ERROR;
  
  frame.attempt = attempt;
  frame.cmd=SER422_WRITE;
  
  // Scrive il byte basso
  frame.data1=contest->pReg[id].address;
  frame.data2=_LB(data);
  Ser422Send(&frame, SER422_BLOCKING, contest->ID);

  // Eventuali errori di comunicazione
  if(frame.retcode!=SER422_WRITE_OK)
  {    
    if(frame.retcode != SER422_BUSY) return _SER422_COMMUNICATION_ERROR;
    else return _SER422_TARGET_BUSY;
  }
  
  contest->pReg[id].val.bytes.lb=_LB(data);
  // Verifica se deve scrivere il byte alto
  if(contest->pReg[id].size==_8BIT)
  {
    contest->pReg[id].val.bytes.hb=0;
    return _SER422_NO_ERROR;
  }

  // Scrive il byte alto
  frame.data1=contest->pReg[id].address+1;
  frame.data2=_HB(data);
  Ser422Send(&frame, SER422_BLOCKING,contest->ID);

  // Eventuali errori di comunicazione
  if(frame.retcode!=SER422_WRITE_OK)
  {    
    if(frame.retcode != SER422_BUSY) return _SER422_COMMUNICATION_ERROR;
    else return _SER422_TARGET_BUSY;
  }
  contest->pReg[id].val.bytes.hb=_HB(data);

  return _SER422_NO_ERROR;
}

//////////////////////////////////////////////////////////////////////////////
/*
bool Ser422UploadRegisters(unsigned char attempt,_DeviceContext_Str* contest);

        Scrive tutto il contenuto dei registri R/W locali sul dispositivo remoto.
        Questa funzione viene utilizzata allo startup di un driver per
        scaricare il contenuto di tutti i registri allo stato corrente.
        

PARAM:
        attempt: numero massimo di tentativi di lettura prima di fallire
RETURN:
        TRUE = successo
        FALSE = fallito

Autore: M. Rispoli
Data: 24/10/2014
*/
//////////////////////////////////////////////////////////////////////////////
bool Ser422UploadRegisters(unsigned char attempt, _DeviceContext_Str* contest)
{
  _Ser422_Command_Str frame;
  int i;
  
  frame.address = contest->address;
  frame.attempt = attempt;
  frame.cmd=SER422_WRITE;
 
  for(i = 0; i< contest->nregisters; i++)
  {
    if(contest->pReg[i].access==_RD) continue;

    // Scrive il byte basso
    frame.data1=contest->pReg[i].address;
    frame.data2=contest->pReg[i].val.bytes.lb;
    Ser422Send(&frame, SER422_BLOCKING, contest->ID);
    
    // Eventuali errori di comunicazione: ripete
    if(frame.retcode!=SER422_WRITE_OK) return FALSE;
    
    // Verifica se deve scrivere il byte alto
    if(contest->pReg[i].size==_8BIT) continue;
    
    // Scrive il byte alto
    frame.data1=contest->pReg[i].address+1;
    frame.data2=contest->pReg[i].val.bytes.hb;
    Ser422Send(&frame, SER422_BLOCKING,contest->ID);
    
    // Eventuali errori di comunicazione
    if(frame.retcode!=SER422_WRITE_OK) return FALSE;
  }
  
  return TRUE;
}


//////////////////////////////////////////////////////////////////////////////
/*
_SER422_Error_Enum ReadRegister(id,attempt,, _DeviceContext_Str* contest);

        Legge dal target il registro con ID == id, effettuando
        un numero massimo di tentativi pari a <attempt>.
        La funzione effettua una doppia lettura nel caso di registro
        a 16 bit.
PARAM:
        id: ID del registro da leggere 
        attenmpt: numero massimo di tentativi di lettura prima di fallire
RETURN:
        _SER422_NO_ERROR = successo
        _SER422_INVALID_FUNCDATA: se ID punta fuori dalla lista
        _SER422_COMMUNICATION_TMO: esauriti i numeri di tentativi

Autore: M. Rispoli
Data: 17/09/2014
*/
//////////////////////////////////////////////////////////////////////////////
_SER422_Error_Enum Ser422ReadRegister(unsigned char id, unsigned char attempt, _DeviceContext_Str* contest)
{
  _Ser422_Command_Str frame;
  _ShortByte_t sval;
  
  // ID non valido
  if(id>= contest->nregisters) return _SER422_INVALID_FUNCDATA;
 
  sval.val16=0;
  frame.address = contest->address;  
  frame.attempt = attempt;
  frame.cmd=SER422_READ;
  
  // Legge il byte basso
  frame.data1=contest->pReg[id].address;
  frame.data2=contest->pReg[id].banco;
  Ser422Send(&frame, SER422_BLOCKING,(unsigned char) contest->ID);
  
  // Timout comunicazione 
  if(frame.retcode!=SER422_READ_OK) return _SER422_COMMUNICATION_ERROR;
  
  sval.bytes.lb=frame.data2;
    
  // Verifica se deve leggere il byte alto
  if(contest->pReg[id].size==_8BIT)
  {
    contest->pReg[id].val.val16=sval.val16;
    return _SER422_NO_ERROR;
  }

  // Legge il byte alto
  frame.data1=contest->pReg[id].address+1;
  frame.data2=contest->pReg[id].banco;
  Ser422Send(&frame, SER422_BLOCKING,(unsigned char) contest->ID);
  
  // Timout comunicazione 
  if(frame.retcode!=SER422_READ_OK) return _SER422_COMMUNICATION_ERROR;
  
  sval.bytes.hb=frame.data2;
  contest->pReg[id].val.val16 = sval.val16;

  return _SER422_NO_ERROR;
}

void Ser422SendRaw(unsigned char addr, unsigned char data1, unsigned char data2, unsigned char* buffer, unsigned char attempt)
{
  _Ser422_Command_Str frame;

  frame.address = (0x1F & addr) | 0x80;
  frame.attempt = attempt;
  frame.cmd= (_Ser422CmdCodes_Enum) (addr >> 6);
  
  // Legge il byte basso
  frame.data1=data1;
  frame.data2=data2;
  Ser422Send(&frame, SER422_BLOCKING,0);
  
  // Timout comunicazione 
  if(frame.retcode!=SER422_RAWOK) 
  {
    buffer[0]=0;
    buffer[1]=0;
    buffer[2]=0;
    return;
  }

  buffer[0]=(0x1F & addr);
  buffer[1]=frame.data1;
  buffer[2]=frame.data2;
  return;
  
}

bool Ser422WriteRaw(unsigned char addr, unsigned char data1, unsigned char data2, unsigned char attempt)
{
  _Ser422_Command_Str frame;

  frame.address = (0x1F & addr);
  frame.attempt = attempt;
  frame.cmd= SER422_WRITE;
  
  frame.data1=data1;
  frame.data2=data2;
  Ser422Send(&frame, SER422_BLOCKING,0);
  
  // Timout comunicazione 
  if(frame.retcode!=SER422_WRITE_OK) return false;
  
  return true;  
}

bool Ser422ReadRaw(unsigned char addr, unsigned short reg, unsigned char attempt, unsigned char* result)
{
  _Ser422_Command_Str frame;

  frame.address = (0x1F & addr);
  frame.attempt = attempt;
  frame.cmd= SER422_READ;
  
  frame.data1=reg&0xFF;
  frame.data2=(reg>>8);
  Ser422Send(&frame, SER422_BLOCKING,0);
  
  // Timout comunicazione 
  if(frame.retcode!=SER422_READ_OK) return false;
  *result = frame.data2;
  
  return true;  
}

_SER422_Error_Enum Ser422ReadAddrRegister(unsigned char addr, unsigned char banco,unsigned char attempt, unsigned char* dato, _DeviceContext_Str* contest)
{
  _Ser422_Command_Str frame;
  
  frame.address = contest->address;  
  frame.attempt = attempt;
  frame.cmd=SER422_READ;
  
  // Legge il solo byte basso
  frame.data1= addr;
  frame.data2=banco;
  //printf("LEGGE:TARGET=%x, ADDR=%x, BANCO=%x\n",frame.address,frame.data1,frame.data2);
  Ser422Send(&frame, SER422_BLOCKING,(unsigned char) contest->ID);
  
  // Timout comunicazione 
  if(frame.retcode!=SER422_READ_OK) return _SER422_COMMUNICATION_ERROR;
  //printf("DATO=%x\n",frame.data2);
  
  *dato = frame.data2;
  return _SER422_NO_ERROR;
 
}

//////////////////////////////////////////////////////////////////////////////
/*
_SER422_Error_Enum Ser422Read16BitRegister(id,attempt,, _DeviceContext_Str* contest);

        Legge un registro a 16 bit utilizzando il comando speciale di protocollo.
        E' utiulizzabile solo in caso di protocollo abilitato
PARAM:
        id: ID del registro da leggere 
        attempt: numero massimo di tentativi di lettura prima di fallire
RETURN:
        _SER422_NO_ERROR = successo
        _SER422_INVALID_FUNCDATA: se ID punta fuori dalla lista
        _SER422_COMMUNICATION_TMO: esauriti i numeri di tentativi

Autore: M. Rispoli
Data: 12/01/2015
*/
//////////////////////////////////////////////////////////////////////////////
_SER422_Error_Enum Ser422Read16BitRegister(unsigned char id, unsigned char attempt, _DeviceContext_Str* contest)
{
  _Ser422_Command_Str frame;
  _ShortByte_t sval;
  
  // ID non valido
  if(id>= contest->nregisters) return _SER422_INVALID_FUNCDATA;
 
  sval.val16=0;
  frame.address = contest->address;  
  frame.attempt = attempt;
  frame.cmd=SER422_SPECIAL;
  
  // Legge il byte basso
  frame.data1=contest->pReg[id].address;
  frame.data2=contest->pReg[id].banco;
  Ser422Send(&frame, SER422_BLOCKING,(unsigned char) contest->ID);
  
  // Timout comunicazione 
  if(frame.retcode!=SER422_COMMAND_SPECIAL_OK) return _SER422_COMMUNICATION_ERROR;
  
  // Copia il contenuto
  sval.bytes.hb=frame.data1;
  sval.bytes.lb=frame.data2;
  contest->pReg[id].val.val16 = sval.val16;

 
  return _SER422_NO_ERROR;
}


//////////////////////////////////////////////////////////////////////////////
/*
bool Ser422FormatRegListCrc(_DeviceAppRegister_Str* ptr, _DeviceContext_Str* contest);

        Valida il contenuto della lista di registri da usare
        e ne applica il CRC.
PARAM:
        ptr: puntatore alla lista di registri
RETURN:
        TRUE: OK
        FALSE: Lista non valida

Autore: M. Rispoli
Data: 17/09/2014
*/
//////////////////////////////////////////////////////////////////////////////
bool Ser422FormatRegListCrc(_DeviceAppRegister_Str* ptr, _DeviceContext_Str* contest)
{
  unsigned char crc;
  unsigned char i;
  
  // Validazione del contenuto
  if(ptr==NULL) return TRUE;
  if(ptr->nitem == 0) return FALSE;
  if(ptr->nitem >= contest->nregisters) return FALSE;
  
  // set CRC
  for(crc=0, i=0; i<ptr->nitem; i++)
  {
    crc^=ptr->id[i];
    crc^=_LB(ptr->val[i]);
    crc^=_HB(ptr->val[i]);
  }
  ptr->crc=crc;
  
  return TRUE;
  
}



//////////////////////////////////////////////////////////////////////////////
/*
bool Ser422SetPollingList(_DeviceAppRegister_Str* pList, _DeviceContext_Str* contest)

        La funzione aggiorna la lista dei registri da leggere;
        La funzione usa la mutex <pollinglist_mutex> per sincronizzarsi 
        con il driver;
        La funzione effettua una verifica di coerenza della lista passata 
PARAM:
        pList: puntatore alla lista polling
        pList=NULL azzera la lista polling
        Non è ammessa una lista con 0 item
RETURN:
      TRUE: Lista corretta e aggiornata
      FALSE: Lista non corretta

Autore: M. Rispoli
Data: 24/10/2014
*/
//////////////////////////////////////////////////////////////////////////////
bool Ser422SetPollingList(_DeviceAppRegister_Str* pList, _DeviceContext_Str* contest)
{
  unsigned char crc;
  unsigned char i;
    
  if(pList==NULL) 
  {
    _mutex_lock(&(contest->pollinglist_mutex));
    contest->locPollingList.nitem=0;
    _mutex_unlock(&(contest->pollinglist_mutex));
    return TRUE;
  }
  if(pList->nitem == 0) return FALSE;
  if(pList->nitem >= contest->nregisters) return FALSE;
  
  // Controllo CRC
  for(crc=pList->crc, i=0; i<pList->nitem; i++)
  {
    crc^=pList->id[i];
    crc^=_LB(pList->val[i]);
    crc^=_HB(pList->val[i]);
  }
  if(crc) return FALSE;
  
  // Copia la lista passata nella lista locale
  _mutex_lock(&(contest->pollinglist_mutex));
  contest->locPollingList.nitem = pList->nitem;
  for(i=0; i<pList->nitem; i++)
      contest->locPollingList.id[i]=pList->id[i];
  _mutex_unlock(&(contest->pollinglist_mutex));
  
  return TRUE;
}



//////////////////////////////////////////////////////////////////////////////
/*
bool Ser422AddRegToPollingList(unsigned char id, _DeviceContext_Str* contest)
        La funzione aggiunge il registro ID alla lista dei polling;
        Se ID è già presente allora non fa nulla.

PARAM:
      id: ID del registro da pollare

RETURN:
      TRUE: ID aggiunto
      FALSE: ID errato. Questo è un problema firmware

Autore: M. Rispoli
Data: 19/09/2014
*/
//////////////////////////////////////////////////////////////////////////////
bool Ser422AddRegToPollingList(unsigned char id, _DeviceContext_Str* contest)
{
  unsigned char i;

  // Controllo di coerenza dati in ingresso
  if(id>=contest->nregisters) return FALSE;
    
  // Se la lista non è vuota, controlla che non ci sia già
  if(contest->locPollingList.nitem)
  {
    for(i=0; i<contest->locPollingList.nitem;i++)
        if(contest->locPollingList.id[i]==id) return TRUE;
  }

  // Aggiunge il registro alla lista
  _mutex_lock(&(contest->pollinglist_mutex));
  contest->locPollingList.id[contest->locPollingList.nitem]=id;
  contest->locPollingList.val[contest->locPollingList.nitem]=0;
  contest->locPollingList.crc^= contest->locPollingList.nitem;
  contest->locPollingList.nitem++;
  contest->locPollingList.crc^= contest->locPollingList.nitem^id;
  _mutex_unlock(&(contest->pollinglist_mutex));
  return TRUE;

}


//////////////////////////////////////////////////////////////////////////////
/*
bool Ser422DelRegToPollingList(unsigned char id, _DeviceContext_Str* contest)
        La funzione elimina il registro ID dalla lista dei polling;
        Se ID non è presente allora non fa nulla.

PARAM:
      id: ID del registro da pollare

RETURN:
      TRUE: ID eliminato
      FALSE: ID errato. Questo è un problema firmware

Autore: M. Rispoli
Data: 19/09/2014
*/
//////////////////////////////////////////////////////////////////////////////
bool Ser422DelRegToPollingList(unsigned char id, _DeviceContext_Str* contest)
{
  unsigned char i;

  // Controllo di coerenza dati in ingresso
  if(id>=contest->nregisters) return FALSE;
  
  // Se la lista è vuota esce subito
  if(contest->locPollingList.nitem==0) return TRUE;
  
  // Se la lista non è vuota, controlla che ci sia già
  for(i=0; i<contest->locPollingList.nitem;i++)
      if(contest->locPollingList.id[i]==id) break;

  // Se ID non c'è esce subito
  if(i==contest->locPollingList.nitem) return TRUE; 

  // Sposta di una posizione tutti i registri
  _mutex_lock(&(contest->pollinglist_mutex));
  for(; i<contest->locPollingList.nitem-1;i++)
      contest->locPollingList.id[i]=contest->locPollingList.id[i+1];
  contest->locPollingList.crc^= contest->locPollingList.nitem^id;
  contest->locPollingList.nitem--;
  contest->locPollingList.crc^= contest->locPollingList.nitem;
  _mutex_unlock(&(contest->pollinglist_mutex));
  
  return TRUE;

}

//////////////////////////////////////////////////////////////////////////////
/*
bool Ser422SetConfigList(_DeviceAppRegister_Str* pList, _DeviceContext_Str* contest)
        La funzione aggiorna la lista di configurazione (aggiungendo);
        Per ricrearla occorre prima creare una lista NULL; 
        La funzione usa la mutex <reglist_mutex> per sincronizzarsi 
        con il driver;
        La funzione effettua una verifica di coerenza della lista passata;
        Se i valori contenuti nella lista risultano già aggiornati 
        allora non li inserisce nella lista finale. Se tutti sono 
        già aggiornati la funzione setta il flag <updated> senza richiedere
        al driver ulteriori scritture;

PARAM:
        pList: puntatore alla lista di registri da aggiornare
        pList=NULL annulla una eventuale lista in corso

RETURN:
      TRUE: Lista corretta e aggiornata
      FALSE: Lista non corretta

VARIABILI DI STATO:
      .updated: == 1 -> Lista aggiornata

Autore: M. Rispoli
Data: 17/09/2014
*/
//////////////////////////////////////////////////////////////////////////////
bool Ser422SetConfigList(_DeviceAppRegister_Str* pList, _DeviceContext_Str* contest)
{
  unsigned char crc,i,j,n,id;
  unsigned short val;
  
  // Annulla tutta la configurazione corrente
  if((pList==NULL) || (pList->nitem==0))
  {
    _mutex_lock(&(contest->reglist_mutex));
    contest->ConfigurationRegistersList.nitem=0;
    _lwevent_set(contest->evr, contest->evm);
    _mutex_unlock(&(contest->reglist_mutex));
    return TRUE;
  }
  
  if(pList->nitem >= contest->nregisters) return FALSE;
  
  // Controllo CRC
  for(crc=pList->crc, i=0; i<pList->nitem; i++)
  {
    crc^=pList->id[i];
    crc^=_LB(pList->val[i]);
    crc^=_HB(pList->val[i]);
  }
  if(crc) return FALSE;
  
  // Aggiunge i registri che non sono contenuti nella lista di configurazione
  _mutex_lock(&(contest->reglist_mutex));  
  for(i=0; i<pList->nitem; i++)
  { 
    n = contest->ConfigurationRegistersList.nitem;
    
    // Controllo presenza
    for(j=0; j<n; j++)
    {
      // Se è già presente, sostituisce il valore
      if(pList->id[i]==contest->ConfigurationRegistersList.id[j])
      { 
        contest->ConfigurationRegistersList.val[j] = pList->val[i];
        break;
      }
    }
    
    // Nuovo elemento: aggiunge alla configurazione
    if(j==n)
    {
      contest->ConfigurationRegistersList.id[n] = pList->id[i];
      contest->ConfigurationRegistersList.val[n] = pList->val[i];
      contest->ConfigurationRegistersList.nitem++;
    }
  }
  
  // Cancella evento di configurazione avvenuta e richiede al driver l'aggiornamento
  _lwevent_clear(contest->evr, contest->evm);
  contest->Stat.updconf=1;
  _mutex_unlock(&(contest->reglist_mutex));  
  return TRUE;
}

bool Ser422DriverFreeze( int evmask,LWEVENT_STRUCT *Event,int timeout, _DeviceContext_Str* contest)
{
  _EVCLR(_EV1_DEVICES_RUN);
  contest->Stat.freeze=1;
  if(timeout) return msEventWaitAll(evmask,Event,timeout);
  return TRUE;
}

bool Ser422DriverFreezeAll(int timeout)
{
  unsigned long maschera=0;
  
  _EVCLR(_EV1_DEVICES_RUN);
  PCB215_CONTEST.Stat.freeze=1;
  PCB190_CONTEST.Stat.freeze=1;
  PCB249U1_CONTEST.Stat.freeze=1;
  PCB249U2_CONTEST.Stat.freeze=1;
  PCB244_A_CONTEST.Stat.freeze=1;
  
  maschera|=_EVMASK(_EV1_PCB215_FREEZED);
  maschera|=_EVMASK(_EV1_PCB190_FREEZED);
  maschera|=_EVMASK(_EV1_PCB249U1_FREEZED);
  maschera|=_EVMASK(_EV1_PCB249U2_FREEZED);  
  maschera|=_EVMASK(_EV1_PCB244_FREEZED);

  if(timeout) return msEventWaitAll(maschera,&_EVSTR(_EV1_PCB215_FREEZED),timeout);
  return TRUE;
}

bool Ser422DriverSetReadyAll(int timeout)
{
  unsigned long maschera=0;
  
  _EVCLR(_EV1_PCB215_RUN);
  _EVCLR(_EV1_PCB190_RUN);
  _EVCLR(_EV1_PCB249U1_RUN);
  _EVCLR(_EV1_PCB244_RUN);
  
  // Attiva evento generale di sblocco
  _EVSET(_EV1_DEVICES_RUN);
  
  maschera|=_EVMASK(_EV1_PCB215_RUN);
  maschera|=_EVMASK(_EV1_PCB190_RUN);
  maschera|=_EVMASK(_EV1_PCB249U1_RUN);
  maschera|=_EVMASK(_EV1_PCB244_RUN);

  if(timeout) return msEventWaitAll(maschera,&_EVSTR(_EV1_PCB215_RUN),timeout);
  return TRUE;
}

/* EOF */
