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
#ifndef _TRX_H
#define _TRX_H

#ifdef ext
#undef ext
#endif
#ifdef _TRX_C
  #define ext 
#else
  #define ext extern
#endif

#include <pd4.h>


ext void trx_tx_task(void);

#define DELAY_TRX_INIT  2000 // Delay after Init node is provided


// Main status loop, following the standard CiA402
void  CiA402_Trx_Stat(void);



// IO Configuration
#define TRX_INPUT_ENA_FUNC               0x7            // All Special functions activated
                                                        // Input1= Negative Switch
                                                        // Input2= Positive Switch
                                                        // Input3= Homing Switch
#define TRX_INPUT_INVERT                 0x0            // Logic Input inversion config bit
#define TRX_INPUT_LEVEL                  0x0            // 5V (0) 24V (1)

// Definizione degli IO
#define TRX_EXPWIN_DETECTION_INPUT          0x1
#define TRX_OBSTACLE_DETECTION_INPUT        0x2         // Attivo basso
#define TRX_ZERO_DETECTION_INPUT            0x4
#define TRX_FAULT_DETECTION_INPUT           0x8         // Attivo alto

typedef enum{
    TRX_NO_COMMAND=0,
    TRX_POLLING_STATUS,         // Evento inviato ogni secondo per segnalare un dato stato    
    TRX_IDLE ,                  // IDLE mode: the commands start from here
    TRX_ZERO_SETTING,           // Zero setting acquisition
    TRX_MOVE_WITH_TRIGGER,      // Move with constant speed to a given target, starting with IO
    TRX_MANUAL_MOVE_TO_POSITION, // Move with CW and CCW buttons
    TRX_MOVE_TO_POSITION,       // Move to a target position
    TRX_FAULT,                  // Stato di fault
    TRX_RUN,                    // Evento di RUN
    TRX_QUICK_STOP              // Quick Stop command
}_trx_command_t;


typedef enum {
    ZERO_SETTING_PROFILE=0,
    HOLDING_PROFILE,
    POSITION_PROFILE,
    TOMO1_PROFILE,
    TOMO2_PROFILE,
    TOMO3_PROFILE,
    TOMO4_PROFILE
}_profileIndexSet_t;

typedef struct {
    short targetPosition;
    unsigned char contextIndex;
}_trx_positioning_data_t;

// EVENTS RELATED TO COMMAND COMPLETION
#define TRX_ZERO_SETTING_TMO    15000   // Timeout command zero setting
#define TRX_POSITIONING_TMO     10000   // Timeout command zero setting


// API for the application
ext bool trxSetCommand(_trx_command_t command, void* data);    // Activate a given command
ext void trxUpdateConfiguration(void);

bool trxResetModule(void);                      // Causes the software module reset
_PD4_Status_t* trxGetStatus(void);
ext trxConfig_Str trxConfig;        // configurazione trx
ext uint32_t getTrxInputs(void);
ext uint32_t getTrxVbus(void);
ext uint32_t getTrxVlogic(void);
ext uint32_t getTrxTemp(void);
ext uint32_t getTrxFault(void);
ext bool trxGetObstacleStat(void);

ext uint32_t getTrxPosition(void);
ext void trxGetNanojSamples(void);

// Definizione struttura errori:
// [Categoria][Classe][codeh,codel]
// Categoria ==  0xFF definisce errori esterni al driver


#endif
