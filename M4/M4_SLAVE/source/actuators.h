/*
  Protocollo di comunicazione con la periferica PCB240
  Conforme al documento di specifica: "CPU to PCB240 Protocol Requirements Specification"
  
  Descrizione sintetica del protocollo di comunicazione e dell'interfaccia 
  di utilizzo

  TX DATA FRAME (TO PCB240):
  ["S"],[ID],[<COMANDO>],[<DATO1>],[<DATO2>],[OUTPUTS ....],[CRC]

  RX DATA FRAME (FROMPCB240):
  ["S"],[ID],[<COMANDO>],[<RETURN1>],[<RETURN2>],[INPUTS ....],[CRC]

  RX ERROR FRAME (FROM PCB240):
  ["E"],[ID],[<COMANDO>],[<RETURN1>],[<RETURN2>],[INPUTS ....],[CRC]

  --------------------------------- API ---------------------------------------
    REGISTRI DISPONIBILI ALL'APPLICAZIONE
      Notazione: [<attributi modulo>,<attributi applicazione>]
      Attributi:    r = attributo lettura; w = attributo di scrittura;
        
      - SystemInputs[INPUT_LEN]: Inputs Bit ricevuti dalla PCB240. [rw,r]
      - SystemOutputs[OUTPUT_LEN]: Outputs Bit inviati alla PCB240. [r,rw]
      - PCB240_Stat: Registro di stato del modulo. [rw,r]
    
     CODE DI ATTESA EVENTI, da usare con _tasq_suspend() (Read Only)
       - void* pcb240_cmd_queue: coda per task bloccato da comando in corso
         
     MUTEX PER DATI GLOBALI, da usare con _mutex_lock()/_unlock() (Read Only)   
       - MUTEX_STRUCT pcb240_input_mutex;   // Mutex per pcb240 inputs
       - MUTEX_STRUCT pcb240_output_mutex;  // Mutex per pcb240 outputs
       - MUTEX_STRUCT pcb240_stat_mutex;    // Mutex per pcb240 stat reg

     TIPI DISPONIBILI
      - _appPCB240_Commands: enumerativo dei codici comandi / errori 
      - _PCB240_Command_Str: struttura blocco comando
      - enumPCB240Flags: enumerativo flags per API di richiesta invio comando  
  
     FUNZIONI 
       - PCB240SendCommand(...): Funzione per inviare un comando alla PCB240
       - void pcb240_polling_task: Task di gestione della rice/trasmissione
 
     NOTE DI FUNZIONAMENTO (vedi documento di specifica per i dettagli)
     
     La comunicazione avviene con un polling costante ogni PCB240_POLLING_TMO(ms);
     In ogni frame inviato vengono inseriti i SystemOutputs;
     In Ogni frame ricevuto vengono prelevati i SystemInputs;
     L'applicazione può aggiungere al prossimo frame un comando aggiuntivo
     allegando il codice e 2 parametri opzionali. Per fare ciò deve utilizzare 
     la funzione PCB240SendCommand(...):

     La funzione può essere bloccante o non bloccante relativamente alla
     conclusione della transazione.
       
     Al termine della transazione, l'applicazione potrà vedere il risultato
     della stessa consultando il contenuto del blocco-comando 
     passato come puntatore alla funzione precedente, e del registro
     di stato PCB240_Stat.
     
Aut: M. Rispoli
Data di Creazione: 6/09/2014

Data Ultima Modifica:6/09/2014
*/
#ifndef _ACTUATORS_H
#define _ACTUATORS_H

#ifdef ext
#undef ext
#endif
#ifdef _ACTUATORS_C
  #define ext 
#else
  #define ext extern
#endif


void actuators_rx_master(uint32_t parameter); // Reception of messages from actuator Master thread
void actuators_rx_devices(uint32_t parameter);// Reception of events coming from DEVICES

typedef struct master_commands{
    MUTEX_STRUCT    command_mutex;
    bool            busy;
    uint8_t*        data;

} actuatorCommands_t;

ext actuatorCommands_t actuatorCommand;


#endif
