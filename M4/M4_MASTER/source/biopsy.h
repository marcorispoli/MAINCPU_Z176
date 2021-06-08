/*

Aut: M. Rispoli
Data di Creazione: 19/06/2014
*/
#ifndef _BIOPSY_H
#define _BIOPSY_H

#ifdef ext
#undef ext
#undef extrd
#endif
#ifdef _BIOPSY_C
  #define ext 
  #define extrd 
#else
  #define ext extern
  #define extrd extern const
#endif

  typedef struct
  {
    // OBBLIGATORIO
    unsigned char fault:1;        // Fault condition
    unsigned char updconf:1;      // Richiesta aggiornamento configurazione
    unsigned char ready:1;        // Driver Operativo
    unsigned char freeze:1;       // Driver Momentaneamente bloccato
    
    // SPECIFICO PER DRIVER
    unsigned char spare:4;
    //--------------------------
    
    // OBBLIGATORIO
    unsigned char maj_code;        // Codice Firmware letto da periferica
    unsigned char min_code;        // Codice Firmware letto da periferica
    unsigned char error;           // Fault code
  }_BIOPSY_Stat_Str;

  //////////////////////////////////////////////////////////////////////////////
  // DICHIARAZIONE REGISTRI DEL TARGET
  // Sintassi:
  // #define NOME-REGISTRO  <id>, <indirizzo>, <banco>, <size>, <access mode>, <access type>, <defval>  
  //
  //   <id>: identificativo univoco e crescente, a partire da 0.  
  //   <banco>: _BNK01 (registro in banchi 0 o 1) ,_BNK12 (registro in banchi 2 o 3)
  //   <size>:  _8BIT (registro 8 bit), _16BIT (registro a 16 bit)   
  //   <access mode>: _RD (Read Only), _RW (Read Write)
  //   <access type>: _NVL (non volatile), _VL (volatile)**
  //   <defval>: valore di inizializzazione nella struttura, tipicamente 0
  /*
        COMANDI BIOPSIA ATTUALMENTE GESTITI
  •	0x8D 73 	GET POS COMPRESSORE (NON USATO)
  •	0x8E 5B	STAT X
  •			0x40 Pulsante sblocco braccio non premuto
  •			0x41 Pulsante sblocco braccio premuto
  •	
  •	0x4E <H,L>	Movimento asse X
  •	0x4F <H,L>	Movimento Y
  •	0x50 <H,L>      Movimento Z
  •	0xCD F0 <x>	Scrive Z Lesione
  •	0xCD F1 <x>     Scrive Z Limite
  •	0xCD EF <x>     Scrive accessorio
  •	0xCD F2 <x>     Scrive Ago
  •	0x90 71	        Legge posizione corrente Z
  
        LA BIOPSIA RISPONDE FF FF come eco da comado di movimento
        Durante il movimento non risponde nulla
  
*/
  //   **: Attenzione, l'attributo _VL (volatile) deve essere applicato 
  //       ai soli registri di scrittura che possano essere scritti da altre fonti
  //       oltre che dal driver (ad esempio da interfacce connesse al Target)
  //   Al termine della dichiarazione occorre impostare:
  //            #define BIOPSY_NREGISTERS     <last id+1>
  //
  //////////////////////////////////////////////////////////////////////////////
  //#define RG244_EXE                0,0x3C    ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0 //
  //#define RG244_VC_FREQ            1,0x31    ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0 //
  //#define RG244_VC_AMP             2,0x32    ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0 //
  //#define RG244_GRID_SPEED_CYC     3,0x2E    ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0 //
  //#define RG244_GRID_SLOW_CYC      4,0x32    ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0 //
  //#define RG244_IO_EXP_B           1,0x48    ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0 //
  #define RG_BIOP_STAT               0,0x5B,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0 //

  #define BIOPSY_NREGISTERS    1	

  //////////////////////////////////////////////////////////////////////////////
  // ATTIVAZIONE DEI REGISTRI
  // Riempire la struttura seguente con tutti i registri dichiarati in
  // precedenza nello stesso ordine di ID dallo 0 al ID massimo 
  // Sintassi:
  //
  //  _DeviceRegItem_Str  PCB215_Registers[]=
  //    {
  //        _REGDEF(<regname ID = 0>),
  //        _REGDEF(<regname ID = 1>),
  //                .....
  //        _REGDEF(<regname ID = LAST ID>)
  //    }
  //  
  //////////////////////////////////////////////////////////////////////////////
  #ifdef _BIOPSY_C
  volatile _DeviceRegItem_Str  BIOPSY_Registers[]=
  {
    _REGDEF(RG_BIOP_STAT),
  }; 
  #else
  extern volatile const _DeviceRegItem_Str  BIOPSY_Registers[BIOPSY_NREGISTERS]; 
  #endif
 
  #if MAX_NLIST < BIOPSY_NREGISTERS     
    #error "BIOPSY: LISTA REGISTRI INSUFFICIENTE" 
  #endif

  //////////////////////////////////////////////////////////////////////////////
  //                    DEFINIZIONE DI BITFIELD NEL TARGET
  //////////////////////////////////////////////////////////////////////////////
 
  //////////////////////////////////////////////////////////////////////////////
  //                    CODICI COMANDI SERIALI
  //////////////////////////////////////////////////////////////////////////////
  #define BIOPSY_GET_STAT            0x5B,0 // Richiede Stato pulsante di sblocco


  // ---------------------------------------------------------------------------
  //                            COMANDI PER TARATURE 
  // ---------------------------------------------------------------------------

  // ---------------------------------------------------------------------------
  //                            COMANDI PER DIAGNOSTICA 
  // ---------------------------------------------------------------------------

  // ---------------------------------------------------------------------------
  //                            COMANDI PER COLLAUDO
  // ---------------------------------------------------------------------------

  //////////////////////////////////////////////////////////////////////////////
  //                    DEFINIZIONE DI BITFIELD NEL TARGET
  //////////////////////////////////////////////////////////////////////////////  
 // #define BIOPSY_FAULT          _REGID(RG244_FLAGS0),1,BIOPSY_Registers
 
  //////////////////////////////////////////////////////////////////////////////
  //                    CODICI PER ATTIVAZIONE REGISTRO EXE
  //////////////////////////////////////////////////////////////////////////////
  
  ext _DeviceContext_Str BIOPSY_CONTEST;

  // API ////////////////////////////////////////////////////////////////// 
  ext void  BIOPSY_driver(uint32_t initial_data);  // Driver BIOPSY
  ext bool  biopsySetXYZ(unsigned short X, bool XGO, unsigned short Y, bool YGO, unsigned short Z, bool ZGO);
  ext bool  biopsySetZLimit(unsigned char zlimit);
  ext bool  biopsySetZLesione(unsigned char zlesione);
  ext bool  biopsySetLago(unsigned char lago);
  ext void  biopsyReset(void);
  ext void  biopsyStepZ(unsigned char dir, unsigned char val);
  ext bool config_biopsy(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len);
  
#define _DEF_BIOPSY_HIGH        193 // Posizione needle in zero rispetto al piano deel detector
#define _DEF_MARGINE_HIGH       10 // millimetri di margine sulla posizione massima
  
#endif