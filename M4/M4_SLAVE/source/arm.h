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
#ifndef _ARM_H
#define _ARM_H

#ifdef ext
#undef ext
#endif
#ifdef _ARM_C
  #define ext 
#else
  #define ext extern
#endif


ext void arm_tx_task(void);
#define DELAY_ARM_INIT  2000 // Delay after Init node is provided


// Main status loop, following the standard CiA402
void  CiA402_Arm_Stat(void);


// System limit definition
#define MAX_ARM_CURRENT 6000                            // Max RMS current in the motor
#define MAX_ARM_SPEED GRADsec_TO_ROTmin(12)             // Max °/sec of system rotation
#define NOM_ARM_CURRENT 4000                            // Nominal phase current (mA)
#define MAXTIME_ARM_MAXCURRENT  1000                    // Max time for peak current (ms)

// IO Configuration
#define ARM_INPUT_ENA_FUNC               0x4            // All Special functions activated
                                                        // Input1= GPIO
                                                        // Input2= GPIO
                                                        // Input3= Homing Switch
#define ARM_INPUT_INVERT                 0x0            // Logic Input inversion config bit
#define ARM_INPUT_LEVEL                   0x0            // 5V (0) 24V (1)

#define ARM_OBSTACLE_DETECTION_INPUT        0x2         // Attivo basso

typedef enum{
    ARM_NO_COMMAND=0,
    ARM_POLLING_STATUS,         // Evento inviato ogni secondo per segnalare operating state
    ARM_IDLE ,                  // IDLE mode: the commands start from here
    ARM_ZERO_SETTING,           // Zero setting acquisition    
    ARM_MOVE_TO_POSITION,       // Move to a target position
    ARM_MOVE_MANUAL,            // Move to a target position with manual activation
    ARM_FAULT,                   // Stato di fault
    ARM_RUN

}_arm_command_t;


typedef enum {
    ARM_ZERO_SETTING_PROFILE=0,
    ARM_HOLDING_PROFILE,
    ARM_POSITION_PROFILE
}_arm_profileIndexSet_t;

typedef struct {
    short targetPosition;
    short initPosition;
    uint32_t initEncoder;
    unsigned char mode; // Modo di attivazione manuale
}_arm_positioning_data_t;

// EVENTS RELATED TO COMMAND COMPLETION
#define ARM_END_COMMAND_EVENT    (_EVBIT(_EV0_ARM_IDLE)|_EVBIT(_EV0_ARM_FAULT)|_EVBIT(_EV0_ARM_STOP)),_EVVAR(_EV0_ARM_IDLE)
#define ARM_ZERO_SETTING_TMO    15000   // Timeout command zero setting

// API for the application
bool armSetCommand(_arm_command_t command, void* data);    // Activate a given command
ext void armUpdateConfiguration(void);

bool armResetModule(void);                      // Causes the software module reset
_PD4_Status_t* armGetStatus(void);
ext armConfig_Str armConfig;        // configurazione trx
ext uint32_t getArmInputs(void);
ext uint32_t getArmVbus(void);
ext uint32_t getArmVlogic(void);
ext uint32_t getArmTemp(void);
ext uint32_t getArmFault(void);
ext bool armGetObstacleStat(void);

#endif
