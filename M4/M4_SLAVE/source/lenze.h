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
#ifndef _LENZE_H
#define _LENZE_H

#ifdef ext
#undef ext
#endif
#ifdef _LENZE_C
  #define ext 
#else
  #define ext extern
#endif
#include "drivers/lenze/i510.h"
#include "dbt_m4.h"

ext void lenze_tx_task(void);

// Main status loop, following the standard CiA402
void  driver_Lenze_Stat(void);

// Funzioni di interfaccia
ext _i510_Status_t* lenzeGetStatus(void);
ext lenzeConfig_Str lenzeConfig;

ext void lenzeUpdateConfiguration(void);
ext bool lenzeReadPosition(void);
ext bool lenzeSetSpeedManual(uint16_t soglia_bassa, uint16_t soglia_alta);
ext bool lenzeSetSpeedAuto(uint16_t soglia_bassa, uint16_t soglia_alta, uint32_t speed_preset);
ext bool lenzeActivatePositionCompensation(int angolo_iniziale, int angolo_finale);
ext bool lenzeActivateAuto(bool upward);
ext int  lenzeGetVBUS(void);
ext bool lenzeGetObstacleStat(void);
ext void lenzeSetCommand(unsigned char command, unsigned char param);
ext bool lenzeSetSpeedManualPark(bool state);
ext bool lenzeActivateUnpark(void);

ext uint32_t lenzeGetAn1(void);
ext uint32_t lenzeGetAn2(void);
ext uint16_t lenzeGetInternalErrors(void);
ext unsigned char lenzeGetDiagnosticErrors(void);
ext uint32_t lenzeGetHighThreshold(void);
ext uint32_t lenzeGetLowThreshold(void);
ext uint32_t lenzeGetIO(void);
ext uint32_t lenzeGetTemp(void);

#define PRESET_MANUAL   i510_2860_01_PRESET1
#define PRESET_AUTO     i510_2860_01_PRESET2
#define PRESET_PARKING  i510_2860_01_PRESET3

#define SETPOINT_MANUAL i510_2911_01_OD // PRESET 1
#define SETPOINT_AUTO   i510_2911_02_OD // PRESET 2
#define SETPOINT_PARKING i510_2911_03_OD // PRESET 3

// DEFINIZIONE IO DI COLLEGAMENTO LENZE
#define ENABLE_TRIGGER          i510_TRIGGER_IN4
#define RUNCW_MANUAL_TRIGGER    i510_TRIGGER_IN1
#define RUNCCW_MANUAL_TRIGGER   i510_TRIGGER_IN3

#define RUNCW_AUTO_TRIGGER  i510_TRIGGER_IN5

typedef enum{
    LENZE_NO_COMMAND=0,
    LENZE_INIT = 1,
    LENZE_IDLE ,
    LENZE_AUTO_MOVE,
    LENZE_MANUAL_MOVE,
    LENZE_MOVE_TO_POSITION, // tbd
    LENZE_UNLOCK_PARKING,
    LENZE_SET_PARKING,
    LENZE_FAULT,
    LENZE_RUN,
    LENZE_POT_UPDATE
}_lenze_command_t;

#endif
