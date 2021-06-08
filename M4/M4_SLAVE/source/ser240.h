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
#ifndef _SER240
#define _SER240

#ifdef ext
#undef ext
#endif
#ifdef _SER240_C
  #define ext 
#else
  #define ext extern
#endif

  // Verifica attivazione degli IO del BSP
  #if ! BSPCFG_ENABLE_IO_SUBSYSTEM
  #error This application requires BSPCFG_ENABLE_IO_SUBSYSTEM defined non-zero in user_config.h. Please recompile BSP with this option.
  #endif
      
  // Definizione del polling Time
  #define PCB240_POLLING_TMO    50         // ms units
  #define PCB240_POLLING_INPUTS 20         // POlling minimo per rinfrescare A5 con lo stato degli Inputs 


  // Definizione dimensioni Data frames 
  #define PCB240_OUTPUT_BUF_SIZE        6+sizeof(SystemOutputs) 
  #define PCB240_INPUT_BUF_SIZE         4+sizeof(SystemInputs) 
//  #define PCB240_WAITING_RX             (((uint32_t)(10*PCB240_INPUT_BUF_SIZE*1000/BAUDRATE))+5) // ms  
  #define PCB240_WAITING_RX             (((uint32_t)(10*PCB240_INPUT_BUF_SIZE*1000/BAUDRATE))+30) // ms

  #define B_START 'S'
  #define B_ERROR 'E'

  // Output Buffer Index
  #define TX_ID_INDEX      (1) 
  #define TX_CMD_INDEX     (2)
  #define TX_DATA1_INDEX   (3) 
  #define TX_DATA2_INDEX   (4)
  #define TX_OUTPUT_INDEX  (5)
  #define TX_CRC_INDEX     (TX_OUTPUT_INDEX+sizeof(SystemOutputs))
  
  // Input Command Buffer Index
  #define RX_ID_INDEX      (1) 
  #define RX_CMD_INDEX     (2)
  #define RX_DATA1_INDEX   (3) 
  #define RX_DATA2_INDEX   (4)
  #define RX_INPUT_INDEX   (5)
  #define RX_CRC_INDEX     (RX_DATA1_INDEX+sizeof(SystemInputs))
  

  // Definizione del registro di stato:  L'applicazione usa PCB240_STAT
  typedef struct
  {
    unsigned char timeout:1;    // timeout ricezione
    unsigned char received:1;   // risposta a comando ricevuto
    unsigned char busy:1;       // transazione in corso
    unsigned char frame_err:1;  // frame error
    unsigned char slave_err:1;  // slave error
  }_PCB240_Stat_Str ;
  #ifdef _SER240_C
    _PCB240_Stat_Str volatile PCB240_Stat; // Registro di stato privato
  #else
    extern const volatile _PCB240_Stat_Str PCB240_Stat; // Read only
  #endif

   // Comandi per la comunicazione con la PCB240
  typedef enum
  {
    PCB240_NO_CMD=0, // MANDATORY
    PCB240_PWR_OFF=1,
    PCB240_ENABLE_ONEPOINT=2,
    PCB240_RST_ONEPOINT_SERVICE=3,
    PCB240_GET_REV = 4  
  } _appPCB240_Commands;
    
  // Definizione del registro di comando
  typedef struct
  {
    _appPCB240_Commands command;    // Comando eseguito
    unsigned char error;      // Eventuale codice di errore
    unsigned char data1;      // dato1 restituito da slave
    unsigned char data2;      // dato2 restituito da slave
    unsigned char id;         // ID del comando eseguito
    unsigned char attempt;    // Riservato al modulo di trasmissione per le ripetizioni
  }_PCB240_Command_Str ;

  typedef enum
  {
    PCB240_BLOCKING=1,
    PCB240_NONBLOCKING=2,
  }enumPCB240Flags;
    
    
  #ifdef _SER240_C
    void* pcb240_cmd_queue=NULL; // TASQ PER ATTESA COMANDO 
    MUTEX_STRUCT pcb240_stat_mutex;    // Mutex per pcb240 stat reg
  #else
   extern  void* const pcb240_cmd_queue;// TASQ PER ATTESA COMANDO 
   extern const MUTEX_STRUCT pcb240_stat_mutex;    // Mutex per pcb240 stat reg
  #endif
   
  
  ext revStr revPCB240;
  
  // API ////////////////////////////////////////////////////////////////// 
  ext void pcb240_driver(uint32_t initial_data); // Task di polling
  ext int PCB240SendCommand(_PCB240_Command_Str* pCmd,  enumPCB240Flags flags);
  ext void PrintOutputs(void);
  ext bool pcb240UpdateRevision(void); // Comando di richiesta revisione firmware 
  ext bool pcb240PowerOffCommand(unsigned char timeout);
  ext bool isPcb240Timeout(); // Restituisce un eventuale stato di Timeout in corso
  
#endif
