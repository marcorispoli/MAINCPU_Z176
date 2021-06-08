/*
  Protocollo di comunicazione con le periferiche di sistema
   tramite protocollo standard Metaltronica: "RS422 Protocol Requirements Specification"
  
  Descrizione sintetica del protocollo di comunicazione e dell'interfaccia 
  di utilizzo

  TX DATA FRAME (TO DEVICES):
  [ADDRESS],[COMMAND],[DATA],[CRC]

  RX DATA FRAME (FROM DEVICES):
  [ADDRESS],[COMMAND],[DATA],[CRC]

  --------------------------------- API ---------------------------------------

RISPOSTE DI PROTOCOLLO:
  A seguito di un comando seriale il chiamante può attendersi i seguenti 
  codici di ritorno:
  
- Codici in risposta dal Target (vedere specifiche protocollo seriale):
    SER422_BUSY
    SER422_ILLEGAL_ADDRESS,
    SER422_ILLEGAL_DATA,
    SER422_ILLEGAL_FUNCTION,
    SER422_UNIMPLEMENTED_FUNCTION,
    SER422_WAIT_FOR_CONFIRMATION,
    SER422_RESERVED,
    SER422_COMMAND_OK,
    SER422_COMMAND_SPECIAL_OK,
    SER422_READ_OK,
    SER422_WRITE_OK,
    SER422_DATA,
    
    L'applicazione deve valutare caso per caso su come gestire 
    i codici di ritorno precedentemente descritti.

- Codici su errore di comunicazione:
    SER422_ERRFRAME      -> Frame non corretto/corrotto
    SER422_ERRTMO        -> Nessuna risposta

    L'applicazione deve gestire la mancanza di comunicazione 
    con il/i dispositivi su cui si è verificato l'errore.

GESTIONE ERRORI INTERNI

  Il modulo non gestisce errori. In caso di errori software 
  l'applicazione si chiude. Tale evento può capitare esclusimaente
  durante l'inizializzazione e non può dipendere da condizioni run time
  ma solo da problematiche di programmazione che, una volta terminato
  lo sviluppo, non hanno motivo di sussistere (mancanza i memoria, 
  errori di indicizzazione etc)

Aut: M. Rispoli
Data di Creazione: 6/09/2014

Data Ultima Modifica:6/09/2014
*/
#ifndef _SER422
#define _SER422

#ifdef ext
#undef ext
#endif
#ifdef _SER422_C
  #define ext 
#else
  #define ext extern
#endif


  // Verifica attivazione degli IO del BSP
  #if ! BSPCFG_ENABLE_IO_SUBSYSTEM
  #error This application requires BSPCFG_ENABLE_IO_SUBSYSTEM defined non-zero in user_config.h. Please recompile BSP with this option.
  #endif

// Definizione del registro di stato
  typedef struct
  {
    unsigned char timeout:1;    // timeout ricezione
    unsigned char received:1;   // risposta a comando ricevuto
    unsigned char busy:1;       // transazione in corso
    unsigned char frame_err:1;  // frame error
    unsigned char slave_err:1;  // slave error
  }_Ser422_Stat_Str ;
  #ifdef _SER422_C
    _Ser422_Stat_Str  Ser422_Stat; // Registro di stato privato
  #else
    extern const _Ser422_Stat_Str  Ser422_Stat; // Read only
  #endif

  // Codici Comandi Di Protocollo
  typedef enum
  {
    SER422_COMMAND=0,
    SER422_SPECIAL,
    SER422_READ,
    SER422_WRITE
  } _Ser422CmdCodes_Enum;

// Codici di fine comando
  typedef enum
  {
    // Risposte di protocollo
    SER422_BUSY=0,
    SER422_ILLEGAL_ADDRESS,
    SER422_ILLEGAL_DATA,
    SER422_ILLEGAL_FUNCTION,
    SER422_UNIMPLEMENTED_FUNCTION,
    SER422_WAIT_FOR_CONFIRMATION,
    SER422_RESERVED,
    SER422_COMMAND_OK,
    SER422_COMMAND_SPECIAL_OK,
    SER422_READ_OK,
    SER422_WRITE_OK,
    SER422_DATA,
    
    // Errori di ricezione 
    SER422_ERRFRAME,
    SER422_ERRTMO,
    SER422_RAWOK
  } _Ser422RetCodes_Enum;

  // Definizione del byte di indirizzo di protocollo
  typedef struct
  {
    unsigned char address:5;    // Indirizzo periferica
    unsigned char reserved:1;   // riservato per protocollo
    unsigned char cmd:2;        // comando da inviare
  }_Ser422_Addr_Str ;

  // Struttura buffer di comando
  typedef struct SER422_COMMAND_STR
  {
    unsigned char address;              // indirizzo periferica
    unsigned char attempt;              // numero di tentativi in caso di errore
    unsigned char confirmation_attempt; // numero di tentativi di ripetizione

    _Ser422CmdCodes_Enum cmd;           // Comando
    _Ser422RetCodes_Enum retcode;       // Codice risposta
    
    unsigned char data1;                // Data 1
    unsigned char data2;                // Data 2
    unsigned char ret_attempt;          // Ripetizioni residue (diagnostica)
    unsigned char ret_conf_attempt;     // Ripetizioni conferme residue (diagnostica)
    
    bool isLoader;                      // Indica se trattasi di pacchetto loader o normale
  }_Ser422_Command_Str ;

  // Tipo di comando
  typedef enum
  {
    SER422_BLOCKING=1,
    SER422_NONBLOCKING=2,
  }_Ser422Flags_Enum;
    
    
  #ifdef _SER422_C
    void* ser422_send_queue=NULL;       // Coda di attesa per invio pachetto 
    void* ser422_cmd_queue=NULL;        // TASK_Q PER ATTESA COMANDO 
    void* ser422_answ_queue=NULL;       // TASK_Q PER COMANDO BLOCCANTE 
    MUTEX_STRUCT ser422_stat_mutex;     // Mutex per ser422 stat reg
  #else
   extern  void* const ser422_send_queue; 
   extern  void* const ser422_cmd_queue;
   extern  void* const ser422_answ_queue;
   extern const MUTEX_STRUCT ser422_stat_mutex;
  #endif
   
  #define BAUDRATE      4800
  #define BAUDLOADER    19200 


  // API ////////////////////////////////////////////////////////////////// 
  ext void ser422_driver(uint32_t initial_data); // Task di polling
  ext void Ser422Send(_Ser422_Command_Str* pCmd, _Ser422Flags_Enum flags,_task_id id);

  // Struttura per gestione stato di funzionamento devices
  typedef struct
  {
    unsigned char fault:1;        // Fault condition
    unsigned char updconf:1;      // Richiesta aggiornamento configurazione
    unsigned char ready:1;        // Driver Operativo
    unsigned char freeze:1;       // Driver Momentaneamente bloccato
    
    // Riservato ai dispositivi
    unsigned char reserved:4;
    
    unsigned char maj_code;        // Codice Firmware letto da periferica
    unsigned char min_code;        // Codice Firmware letto da periferica
    unsigned char error;           // Fault code
  }_Device_Stat_Str;

  typedef struct
  {
    unsigned char address;                      // Target Address
    _task_id ID;                                // ID driver Task
    volatile _DeviceRegItem_Str* pReg;          // Lista registri
    unsigned char nregisters;                   // Numero registri
    MUTEX_STRUCT reglist_mutex;                 // Mutex per Config List
    MUTEX_STRUCT pollinglist_mutex;             // Mutex per polling List
    _DeviceAppRegister_Str locPollingList;      // Polling List
    _DeviceAppRegister_Str ConfigurationRegistersList; // Config List    
    int evm;                                    // Evento Maschera configurazione registri
    LWEVENT_STRUCT* evr;                        // Evento Registro configurazione registri
    _Device_Stat_Str Stat;                      // Puntatore a stato di funzionamento
  }_DeviceContext_Str;
    
  typedef enum
  {
   // Errori di sistema
   _SER422_NO_ERROR=0,
   _SER422_MQX_ERROR,           // Errore di sistema
   _SER422_INVALID_FUNCDATA,    // Registro invalido passato a funzione
   _SER422_INVALID_ACCESSMODE,  // Tentativo di scrittura di registro Read Only
   _SER422_INVALID_PARAMLIST,   // Lista di parametri non valida
   _SER422_COMMUNICATION_ERROR, // PCB215 errore di comunicazione
   _SER422_TARGET_ERROR,        // Target comunica che il messaggio non è corretto
   _SER422_TARGET_BUSY         // Target Segnala Busy su ultimo comando ricevuto
   
   // Sezione errori diagnostica del Target
  }_SER422_Error_Enum;
  
// Macro per definire La lista di registri da mandare ai driver dispositivi
#define __ADDITEMLIST(x1,x2,x3,x4,x5,x6,x7,lista,value) lista.id[lista.nitem]=x1;lista.val[lista.nitem]=value;lista.nitem++; 
#define _ADDITEMLIST(x,lista,value)       __ADDITEMLIST(x,lista,value) 
#define _LISTCREATE(x,lista,value)    lista.nitem=0; __ADDITEMLIST(x,lista,value) 

  ext void Ser422LoaderSend(_Ser422_Command_Str* pCmd); // Invio pacchetto da Loader
  
  ext bool Ser422TestRegisterList(_DeviceAppRegister_Str* pList,  _DeviceContext_Str* contest);
  ext _SER422_Error_Enum Ser422WriteRegister(unsigned char id, unsigned short data, unsigned char attempt,_DeviceContext_Str* contest);
  ext bool Ser422UploadRegisters(unsigned char attempt, _DeviceContext_Str* contest);
  ext _SER422_Error_Enum Ser422ReadRegister(unsigned char id, unsigned char attempt, _DeviceContext_Str* contest);
  ext _SER422_Error_Enum Ser422ReadAddrRegister(unsigned char addr, unsigned char banco,unsigned char attempt, unsigned char* dato, _DeviceContext_Str* contest);
  ext void Ser422SendRaw(unsigned char addr, unsigned char data1, unsigned char data2, unsigned char* buffer, unsigned char attempt);

  ext _SER422_Error_Enum Ser422Read16BitRegister(unsigned char id, unsigned char attempt, _DeviceContext_Str* contest);
  ext void ser422SetBaud(int baud); // Imposta il Baudrate
  
  ext bool Ser422FormatRegListCrc(_DeviceAppRegister_Str* ptr, _DeviceContext_Str* contest);
  ext bool Ser422AddRegToPollingList(unsigned char id, _DeviceContext_Str* contest);
  ext bool Ser422DelRegToPollingList(unsigned char id, _DeviceContext_Str* contest);
  ext bool Ser422SetPollingList(_DeviceAppRegister_Str* pList, _DeviceContext_Str* contest);
  ext bool Ser422SetConfigList(_DeviceAppRegister_Str* pList, _DeviceContext_Str* contest);
  ext bool Ser422DriverFreeze(int evmask,LWEVENT_STRUCT *Event,int timeout, _DeviceContext_Str* contest);
  ext bool Ser422DriverSetReadyAll(int timeout); // Sblocca tutti i Drivers
  ext bool Ser422DriverFreezeAll(int timeout); // Ferma tutti i drivers
  ext bool Ser422WriteRaw(unsigned char addr, unsigned char data1, unsigned char data2, unsigned char attempt);
  ext bool Ser422ReadRaw(unsigned char addr, unsigned short reg, unsigned char attempt, unsigned char* result);

  #define __DEVREG(x1,x2,x3,x4,x5,x6,x7,y) y.pReg[x1].val.val16 
  #define _DEVREG(x,y) __DEVREG(x,y)
  #define __DEVREGL(x1,x2,x3,x4,x5,x6,x7,y) y.pReg[x1].val.bytes.lb 
  #define _DEVREGL(x,y) __DEVREGL(x,y)
  #define __DEVREGH(x1,x2,x3,x4,x5,x6,x7,y) y.pReg[x1].val.bytes.hb 
  #define _DEVREGH(x,y) __DEVREGH(x,y)
  
  #define __EVSTR(x,y) (y)
  #define _EVSTR(x) __EVSTR(x)
  #define __EVMASK(x,y) (x)
  #define _EVMASK(x) __EVMASK(x)
  #define __DRIVER_FREEZE(m,ev,tmo,cst) Ser422DriverFreeze(m,&ev,tmo,cst)
  #define _DRIVER_FREEZE(x,y,z) __DRIVER_FREEZE(x,y,z) 

  ext bool displaySeriale;
  
#endif
