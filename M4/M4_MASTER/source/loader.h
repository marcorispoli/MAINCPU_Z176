/*
  

Aut: M. Rispoli
Data di Creazione: 6/09/2014

Data Ultima Modifica:6/09/2014
*/
#ifndef _LOADER
#define _LOADER

#ifdef ext
#undef ext
#endif
#ifdef _LOADER_C
  #define ext 
#else
  #define ext extern
#endif

  #define SERCMD_INC_PC	        1       // INCREMENTA IL PROGRAM COUNTER
  #define SERCMD_PROG_ERASE  	2	// CODICE NON PROTETTO: Cancella memoria programma
  #define SERCMD_EEPROM_ERASE  	3       // CODICE NON PROTETTO: Cancella EEPROM
  #define SERCMD_ID_ERASE  	4	// CODICE NON PROTETTO: Cancella ID
  #define SERCMD_CHIP_ERASE  	5	// Cancella tutto, per codice protetto

  #define SERCMD_SET_CONFIG  	6	// Carica da ICSP tutta la sezione di configurazione
  #define SERCMD_READ_MEM  	7	// Legge la memoria
  #define SERCMD_READ_EEPROM	8	// Legge contenuto della EEPROM

  #define SERCMD_LOAD_ID	9	// Programma una word in memoria programma
  #define SERCMD_LOAD_CFGW	10	// Programma la config word

  #define SERCMD_LOADER		11	// ATTIVAZIONE LOADER SOLO A 4800 BR
  #define SERCMD_BROADCAST_EXIT	12      // COMANDO GENERALE DI USCITA (Target=0x1F) NO RISPOSTA

  #define SERCMD_ENTER_USER	13	// Inizializza il PC nell'Area User
  #define SERCMD_READ_EE_BLK	14	// Acquisisce 32 byte di EEPROM
  #define SERCM_READ_PRG_BLK	15	// Acquisisce 32 byte di MEMORIA PROGRAMMA

  #define SERCMD_GET_REV	16	// Richiesta codice firmware del loader

  //______________________________________________________________________________________________
  // REGISTRI LOADER
  #define BASE_CONFIG             0xAF
  #define REG16_PROG_ID0          BASE_CONFIG+0,0,1
  #define REG16_PROG_ID1          BASE_CONFIG+2,0,1
  #define REG16_PROG_ID2          BASE_CONFIG+4,0,1        
  #define REG16_PROG_ID3          BASE_CONFIG+6,0,1
  #define REG16_PROG_DEVICEID     BASE_CONFIG+8,0,1
  #define REG16_PROG_DEVCOD       BASE_CONFIG+10,0,1
  #define REG_PROG_DEVREV         BASE_CONFIG+12,0,0
  #define REG16_PROG_CONFWORD     BASE_CONFIG+13,0,1
  #define REG16_PROG_LOAD         BASE_CONFIG+18,0,1
  #define REG16_PROG_PC           BASE_CONFIG+20,0,1
    
  typedef struct
  {
    unsigned short startAddr;       // Indirizzo di partenza
    unsigned short val[16];         // Blocchi da 16 word
    unsigned short len;             // Numero di word validi
  }_addrStr;


  typedef struct
  {
      unsigned short id0;
      unsigned short id1;
      unsigned short id2;
      unsigned short id3;
      unsigned short devId;
      unsigned short devCod;
      unsigned short devRev;
      unsigned short config;
  }_cfgStr;



  typedef struct
  {
    unsigned char target; 
    unsigned char uC;
    bool isActive;
    _cfgStr cfg;          // Sezione configurazione dispositivo

  }_LoaderStruct; 
  
  ext _LoaderStruct Loader; 
  ext bool loaderActivation(unsigned char address, unsigned char uC);
  ext bool loaderChipErase(void);
  ext bool loaderLoadSegment(_addrStr* blk);
  ext bool loaderWriteConfig(unsigned char* data);
  ext void loaderExit(bool driverRun);


#endif