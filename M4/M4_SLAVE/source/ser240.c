/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 1989-2008 ARC International
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale MQX RTOS License
* distributed with this Material.
* See the MQX_RTOS_LICENSE file distributed for more details.
*
* Brief License Summary:
* This software is provided in source form for you to use free of charge,
* but it is not open source software. You are allowed to use this software
* but you cannot redistribute it or derivative works of it in source form.
* The software may be used only in connection with a product containing
* a Freescale microprocessor, microcontroller, or digital signal processor.
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains the source for the rs485 example program.
*
*
*END************************************************************************/
#define _SER240_C
#include "dbt_m4.h" 

  /* Canale RS485 da usare */
  #define RS485_CHANNEL "ittyc:" // UART 2
//  #define BAUDRATE      57600
  #define BAUDRATE      38400

  // Definizione linea di handshacking
  #define RS485_TXENA_A LWGPIO_PIN_PTD3 // Porta D, Pin 3 CTS
  #define RS485_TXENA_B LWGPIO_PIN_PTD2 // Porta D, Pin 2 RTS


  _PCB240_Command_Str* pPCB240PendingCommand; // Puntatore a blocco comando in esecuzione  
  _PCB240_Command_Str* pPCB240NextCommand;    // Puntatore a prossimo blocco comando  
  unsigned char PCB240_ID=0; 
  
  // GEstione Timeout della PCB240 da comunicare alla GUI
  static int pcb240TimeoutCount = 0;              // Numero di timeout consecutivi prima di segnalare l'errore
  static bool isTimeout = false;
  #define PCB240_COUNT_TIMEOUT 50                   

  
/*TASK*-----------------------------------------------------
* 
* Task Name    : rs485_write_task
* Comments     :
*    This task send data_buffer to rs485
*
*END*-----------------------------------------------------*/
void pcb240_driver(uint32_t initial_data)
{
  bool disable_rx = TRUE;
  int rx_len;
  //bool data_changed;
  int ciclo;
  unsigned char crc;
  unsigned char rx_buffer[sizeof(SystemInputs)+10];
  unsigned char tx_buffer[sizeof(SystemOutputs)+10];
  MQX_FILE_PTR rs485_dev = NULL;
  //MQX_FILE_PTR rs485_gpio = NULL;
  //bool repeat_mcc=FALSE; // Ripetizione invio MCC
  unsigned char pollingInputCnt;
 // unsigned char backupErrors, backupFlags,pcb240Errors,pcb240Flags,pcb240ErrorCnt;
  int vbat1=0;
  int vbat2=0;
  int memvbat1=0;
  int memvbat2=0;
  int countbat=0;

  printf("DRIVER PCB240 PARTITO\n");
  _time_delay(1000);

  //  MCC_ENDPOINT  ep={_DEF_MCC_INPUTS_TO_APP_SLAVE};

    if (_mutex_init(&pcb240_stat_mutex, NULL) != MQX_OK)
    {
      printf("Initializing print mutex failed.\n");
      _task_block();
    }
    
   // Init registro di stato
    _mutex_lock(&pcb240_stat_mutex);
    {
        PCB240_Stat.timeout=0;
        PCB240_Stat.busy=0;
        PCB240_Stat.frame_err=0;
        PCB240_Stat.received=0;
        PCB240_Stat.slave_err=0;
    }
    _mutex_unlock(&pcb240_stat_mutex);

    // Creazione coda processi sospesi
    pcb240_cmd_queue =  _taskq_create(MQX_TASK_QUEUE_FIFO);
    if (pcb240_cmd_queue  == NULL)
    {
      printf("PCB240 DRIVER ERRORE INIT CODA PROCESSI. FINE PROGRAMMA");
      _mqx_exit(-1);
    }

   // Impostazione segnale di attivazione trasmissione basso (disabilita trasmissione) 
   LWGPIO_STRUCT rs485TxEnaA;
   if (lwgpio_init(&rs485TxEnaA,RS485_TXENA_A,LWGPIO_DIR_OUTPUT,LWGPIO_VALUE_LOW) != TRUE)
   {
     printf("PCB240: GPIO initialization failed.\n");
     _task_block();
   }

   LWGPIO_STRUCT rs485TxEnaB;
   if (lwgpio_init(&rs485TxEnaB,RS485_TXENA_B,LWGPIO_DIR_OUTPUT,LWGPIO_VALUE_LOW) != TRUE)
   {
     printf("PCB240: GPIO initialization failed.\n");
     _task_block();
   }
   
   // Apertura UART
   rs485_dev = fopen( RS485_CHANNEL, (char const *)NULL ); 
   if( rs485_dev == NULL )
   {
      /* device could not be opened */
     printf("PCB240: Unable to open UART\n");
     _task_block();
   }

   // Disabilita UART in ricezione
   disable_rx = TRUE;
   ioctl( rs485_dev, IO_IOCTL_SERIAL_DISABLE_RX, &disable_rx );

   // Imposta Baud Rate
   int baud=BAUDRATE;
   ioctl( rs485_dev, IO_IOCTL_SERIAL_SET_BAUD, &baud );
   
   // Init del codice ID usato per sincronizzare i messaggi con comandi
   PCB240_ID=1; 
   pollingInputCnt = PCB240_POLLING_INPUTS;
   //backupErrors= backupFlags=pcb240Errors=pcb240Flags=0;
   //pcb240ErrorCnt = 20;
   
   // Richiesta revisione scheda :TBD
   revPCB240.maj =0;
   revPCB240.min =0;
   
   // Attivazione ciclo di polling
   while(1)
   {
      // Sospende la funzione per il tempo minimo di polling
      _time_delay(PCB240_POLLING_TMO);
      
      // Verifica se è stato richiesto un comando (busy==1)
      // Nel caso sia in corso la richiesta di un comando attende..
      _mutex_lock(&pcb240_stat_mutex);
      {
        if(PCB240_Stat.busy)
        {       
            // Comando
            pPCB240PendingCommand = pPCB240NextCommand; // Associa il blocco comando da eseguire
            tx_buffer[TX_ID_INDEX]=pPCB240PendingCommand->id;    
            tx_buffer[TX_CMD_INDEX]=pPCB240PendingCommand->command;
            tx_buffer[TX_DATA1_INDEX]=pPCB240PendingCommand->data1;
            tx_buffer[TX_DATA2_INDEX]=pPCB240PendingCommand->data2;
        }else
        {
            // Comando nullo
            pPCB240PendingCommand = NULL;
            tx_buffer[TX_ID_INDEX]=0;    
            tx_buffer[TX_CMD_INDEX]=0;
            tx_buffer[TX_DATA1_INDEX]=0;
            tx_buffer[TX_DATA2_INDEX]=0;
        }
      }
      _mutex_unlock(&pcb240_stat_mutex);
      
      // Completamento prossimo frame
      tx_buffer[0]=(unsigned char) B_START;
      
      // Carica gli outputs da inviare alla PCB240
      _mutex_lock(&output_mutex);

          // Gestione casi particolari per driver di rotazione (se presenti)
          if(generalConfiguration.trxDriver){
              if(trxGetStatus()->fatal_error) SystemOutputs.CPU_PEND_ENA = 0; // E' stato rilevato un errore fatale
              else if(trxGetStatus()->init_module) SystemOutputs.CPU_PEND_ENA = 1; // Fase di inizializzazione driver vuole alimentazione
              else if(trxGetStatus()->can_error) SystemOutputs.CPU_PEND_ENA = 0; // Errore sulla comunicazione: toglie il consenso al movimento
          }


          if(generalConfiguration.armDriver){
              if(armGetStatus()->init_module) SystemOutputs.CPU_ROT_ENA = 1; // Fase di inizializzazione driver vuole alimentazione
              else if(armGetStatus()->can_error) SystemOutputs.CPU_ROT_ENA = 0; // Errore sulla comunicazione: toglie il consenso al movimento
          }


          SystemOutputs.SLAVE_TERMINAL_PRESENT = 1;
          memcpy(&tx_buffer[TX_OUTPUT_INDEX],&SystemOutputs,sizeof(SystemOutputs));
      _mutex_unlock(&output_mutex);

      // Calcolo CRC
      tx_buffer[TX_CRC_INDEX]=0;
      for(ciclo=0;ciclo<TX_CRC_INDEX;ciclo++)
        tx_buffer[TX_CRC_INDEX]^=tx_buffer[ciclo];
            
      // printf("TRASMISSIONE BUFFER\n");     
      // Imposta UART in blocking mode
      ioctl(rs485_dev,IO_IOCTL_SERIAL_SET_FLAGS,  NULL);
  
      // Svuota il buffer
      fflush( rs485_dev );
      ioctl( rs485_dev, IO_IOCTL_SERIAL_WAIT_FOR_TC, NULL );
     
      // Attivazione driver di trasmissione
      lwgpio_set_value(&rs485TxEnaA, LWGPIO_VALUE_HIGH);
      lwgpio_set_value(&rs485TxEnaB, LWGPIO_VALUE_HIGH);
      
      // Driver si sospende fino ad avvenuta trasmissione
      write( rs485_dev, tx_buffer, PCB240_OUTPUT_BUF_SIZE );
      fflush( rs485_dev );
      ioctl( rs485_dev, IO_IOCTL_SERIAL_WAIT_FOR_TC, NULL );

       // Imposta UART in non blocking mode
      ioctl(rs485_dev,IO_IOCTL_SERIAL_SET_FLAGS,  (void*) IO_SERIAL_NON_BLOCKING);
      
      // Svuota la coda di ricezione
      while(fread(rx_buffer,1,PCB240_INPUT_BUF_SIZE,rs485_dev)!=0){}
      
      // Attivazione driver in ricezione
      lwgpio_set_value(&rs485TxEnaA, LWGPIO_VALUE_LOW);
      lwgpio_set_value(&rs485TxEnaB, LWGPIO_VALUE_LOW);
     
      // Attivazione UART 
      disable_rx = FALSE;
      ioctl( rs485_dev, IO_IOCTL_SERIAL_DISABLE_RX, &disable_rx );
   
      // Sospende la funzione per il tempo minimo necessario a ricevere i byte
      _time_delay(PCB240_WAITING_RX);
      rx_len = fread(rx_buffer,1,PCB240_INPUT_BUF_SIZE,rs485_dev);
      
      // Controllo di Frame corretto (Start Bit, CRC, Lunghezza)
      if(rx_len==0)
      { 
         PCB240_Stat.timeout=1;
         printf("TIMEOUT!!\n");
      }else
      {
        PCB240_Stat.timeout=0;
        // Controllo numero byte e start bit
        if(((rx_buffer[0]!='S')&&(rx_buffer[0]!='E'))||(rx_len!=PCB240_INPUT_BUF_SIZE))
        {
            printf("FRAME ERROR!!:STARTCHAR=%c LEN=%d ATTESO:%d\n",rx_buffer[0],rx_len,PCB240_INPUT_BUF_SIZE);
            PCB240_Stat.frame_err=1;
        }else
        {
           // Controllo checksum del pacchetto ricevuto
            crc=0;
            for(ciclo=0;ciclo<=RX_CRC_INDEX;ciclo++){
                crc^=rx_buffer[ciclo];

            }


            if(crc)
            {
              PCB240_Stat.frame_err=1;
              printf("CRC ERROR!!\n");
            }
            else
            {
              // Controllo ID (solo per comandi)
              if(pPCB240PendingCommand!=NULL)
              {
                if(rx_buffer[RX_ID_INDEX]!=pPCB240PendingCommand->id)
                {
                  printf("COMMAND: ID ERROR!! IDRX:%d CMD:%d\n",rx_buffer[RX_ID_INDEX],pPCB240PendingCommand->id);
                  printf("CMD:%d DATA1:%d DATA2:%d\n",rx_buffer[RX_CMD_INDEX],rx_buffer[RX_DATA1_INDEX],rx_buffer[RX_DATA2_INDEX]);
                  
                  PCB240_Stat.frame_err=1;
                }
                else
                {
                  PCB240_Stat.frame_err=0;
                }
              }else
              {
                // Analisi comunicazioni da PCB240
                /*
                if(rx_buffer[RX_CMD_INDEX]==0)
                {// DATA1= ERRORI
                  pcb240Errors = rx_buffer[RX_DATA1_INDEX];
                  pcb240Flags = rx_buffer[RX_DATA2_INDEX];
                }*/
                PCB240_Stat.frame_err=0;
              }
            }
        }
      }

      // Nel caso di errori durante comandi in corso si verifica 
      // quanti tentativi ci sono ancora a disposizione prima di abortire
      if((PCB240_Stat.frame_err)||(PCB240_Stat.timeout))
      {
          // Sezione dedicata alla comuncazione verso GUI della condizione di errore
          // di comunicazione persistente: se la GUI non è pronta, il timeout vienr ribadito 
          if(isTimeout==false){
              pcb240TimeoutCount++;
              if(pcb240TimeoutCount>=PCB240_COUNT_TIMEOUT){            
                printf("PCB240 TIMEOUT RILEVATO!\n");
                isTimeout = true;
              }
          }
        

          if(pPCB240PendingCommand==NULL) continue; // Non Comando
          if(pPCB240PendingCommand->attempt--) continue; // Tentativi non esauriti 
          
          // Tentativi esauriti: abortisce il comando
          memset(pPCB240PendingCommand,0,sizeof(_PCB240_Command_Str));
          pPCB240PendingCommand = NULL;
          
          // Azzera il busy e finisce. Viene fatto senza mutex 
          // perchè non c'è pericolo di sovrapposizioni
          PCB240_Stat.busy = 0;
          
          // Sveglia il task bloccato (se definito non blocking)
          _taskq_resume(pcb240_cmd_queue,TRUE);
          continue;
      }

      // Uscita dalla condizione di timeout della PCB240
      if(isTimeout){
          pcb240TimeoutCount=0;
          isTimeout = false;
          printf("PCB240 RISPONDE!\n");          
      }

      if(pPCB240PendingCommand==NULL)
      { // Il Frame ricevuto risulta corretto.
        

        // Effettua un pre filtraggio del dato analogico delle batterie per
        // evitare troppi rinfreschi degli input.
        vbat1+=rx_buffer[RX_DATA1_INDEX];
        vbat2+=rx_buffer[RX_DATA2_INDEX];
        countbat++;
        if(countbat==20){
            memvbat1 = (vbat1 / countbat);
            memvbat2 = (vbat2 / countbat);
            vbat1=0;
            vbat2=0;
            countbat=0;
        }
        rx_buffer[RX_DATA1_INDEX] = (unsigned char) memvbat1;
        rx_buffer[RX_DATA2_INDEX] = (unsigned char) memvbat2;

        // Verifica dunque se il dato è cambiato
        if(memcmp(&rx_buffer[RX_DATA1_INDEX],&SystemInputs,sizeof(SystemInputs))!=0)
        {
            _mutex_lock(&input_mutex);
            memcpy(&SystemInputs,&rx_buffer[RX_DATA1_INDEX],sizeof(SystemInputs));        
            _mutex_unlock(&input_mutex);
            _EVSET(_EV0_INPUT_CAMBIATI);          
        }
          
        continue;      
      }
      
     
      // In caso di comando in corso prepara il blocco comando 
      // con la risposta e completa la sequenza di comando
      PCB240_Stat.received = 1;
      if(rx_buffer[0]=='E')
      {
        // SLAVE ERROR
        PCB240_Stat.slave_err=1;
        pPCB240PendingCommand->command=PCB240_NO_CMD;
        pPCB240PendingCommand->error=rx_buffer[RX_CMD_INDEX];
        pPCB240PendingCommand->data1=rx_buffer[RX_DATA1_INDEX];
        pPCB240PendingCommand->data2=rx_buffer[RX_DATA2_INDEX];
        printf("CMD ERR %d\n",pPCB240PendingCommand->error);
        
      }else
      {
        // Comando Eseguito
        pPCB240PendingCommand->error=0;
        pPCB240PendingCommand->command=(_appPCB240_Commands)rx_buffer[RX_CMD_INDEX];
        pPCB240PendingCommand->data1=rx_buffer[RX_DATA1_INDEX];
        pPCB240PendingCommand->data2=rx_buffer[RX_DATA2_INDEX];
        

      }

      // Fine comando
      pPCB240PendingCommand=NULL;    
      PCB240_Stat.busy=0;
      _taskq_resume(pcb240_cmd_queue,TRUE);
      
   }
   

}

/* 
int PCB240SendCommand(_PCB240_Command_Str* pCmd,  enumPCB240Flags flags)

Parametri:
    pCmd: puntatore al blocco Comando da eseguire preparato dall'Applicazione 
    flags: PCB240_BLOCKING/PCB240_NONBLOCKING 
           IN caso di opzione PCB240_BLOCKING, il task chiamante si blocchera
           fino al termine della transazione

    !!! ATTENZIONE: il blocco di memoria puntato da pCmd deve rimanere valido
    !!! durante tutta la transazione, fino a che PCB240_Stat.busy == 0.
    !!! In caso di deallocazione prematura, il processo potrebbe diventare 
    !!! instabile

Return: 
    ID del comando in corso / -1 se c'è un comando già in transazione 

Note di funzionamento:
    Se La funzione non è bloccante, l'applicazione dovrà monitorare
    PCB240_Stat.busy. Quando il flag si azzera allora la transazione sarà 
    terminata e l'Applicazione potrà monitorare l'esito.
    IN caso di funzione bloccante, essa ritornerà a transazione avvenuta.

    In caso di errore:
    Se (pCmd->error == 0) e (pCmd->comando ==0) -> Errore di Frame/Timeout;
    Se pCmd->error !=0 -> Slave ha comunicato una condizione di errore;
        pCmd->error: Codice di errore;
        pCmd->data1,pCmd->data2: dati opzionali comunicati dallo SLAVE 
    
    In caso di Successo:
        pCmd->comando: Codice di esecuzione da Slave;
        pCmd->data1,pCmd->data2: dati opzionali comunicati dallo SLAVE; 

Aut: M. Rispoli
Data: 6/09/2014
*/
int PCB240SendCommand(_PCB240_Command_Str* pCmd,  enumPCB240Flags flags)
{
  unsigned char id;
 
   // Blocca su Stat per evitare conflitti con il driver
  // Il driver al successivo slot di invio si bloccherà 
  // sul test del busy nel caso dovesse svegliarsi durante 
  // il blocco successivo di questo comando
  _mutex_lock(&pcb240_stat_mutex);
  
  // Non accetta altri comandi se si trova in busy
  // o se ci sono errori in corso. I comandi riprenderanno
  // Dopo il primo scambio utile di messaggi IO
  if((PCB240_Stat.busy)||(PCB240_Stat.timeout)||(PCB240_Stat.frame_err))
  {
    _mutex_unlock(&pcb240_stat_mutex);
    return -1;
  }
   
  memset((void*)&PCB240_Stat,0,sizeof(PCB240_Stat));
  PCB240_Stat.busy=1; // Blocca il comando

  // Prepara il blocco di comando da eseguire
  pPCB240NextCommand = pCmd;
  id=pPCB240NextCommand->id=PCB240_ID++;
  
  
  _mutex_unlock(&pcb240_stat_mutex);
  
  // Ora il driver è libero di prendersi carico del comando da inviare
  
  // Se la funzione è bloccante, il task richiedente si sospende
  // fino al completamento del ciclo di TX-RX o fino a errore
  if(PCB240_BLOCKING==flags) _taskq_suspend(pcb240_cmd_queue); 
  
  return id; 

}

// Comando di richiesta revisione firmware 
bool pcb240UpdateRevision(void)
{
  _PCB240_Command_Str frame;
  int attempt=100;
  
  frame.command = PCB240_GET_REV;
  while(--attempt)
  {
    _time_delay(100);
    frame.command = PCB240_GET_REV;
    if(PCB240SendCommand(&frame,PCB240_BLOCKING)==-1) continue;

    if((frame.error)||(frame.command==0)) continue;
    if(frame.data1==0) continue;
    break;
  }
  
  if(!attempt) return FALSE;
  
  // Comando eseguito e risposta pronta
  revPCB240.maj = frame.data1;
  revPCB240.min = frame.data2;

  printf("REVISIONE NUOVA PCB240: %d.%d\n", revPCB240.maj, revPCB240.min);
  generalConfiguration.pcb240connected = true;

  return TRUE;
  
}

// Comando di spegnimento generale
// Timeout sono i secondi di attesa
bool pcb240PowerOffCommand(unsigned char timeout)
{
  _PCB240_Command_Str frame;
  int attempt=100;

  frame.command = PCB240_PWR_OFF;
  while(--attempt)
  {
    _time_delay(100);
    frame.command = PCB240_PWR_OFF;
    frame.data1 = timeout;

    if(PCB240SendCommand(&frame,PCB240_BLOCKING)==-1) continue;

    if((frame.error)||(frame.command==0)) continue;
    if(frame.data1==0) continue;
    break;
  }

  if(!attempt) return FALSE;

  printf("POWER OFF ACCETTATO DALLA PCB240\n");
  return TRUE;

}

bool isPcb240Timeout(){
  return isTimeout;
}
/* EOF */
