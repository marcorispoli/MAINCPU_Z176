/*

Aut: M. Rispoli
Data di Creazione: 01/11/2014
*/
#ifndef _PCB244_H
#define _PCB244_H

#ifdef ext
#undef ext
#undef extrd
#endif
#ifdef _PCB244_C
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
  }_PCB244_Stat_Str;

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
  //
  //   **: Attenzione, l'attributo _VL (volatile) deve essere applicato 
  //       ai soli registri di scrittura che possano essere scritti da altre fonti
  //       oltre che dal driver (ad esempio da interfacce connesse al Target)
  //   Al termine della dichiarazione occorre impostare:
  //            #define PCB244_NREGISTERS     <last id+1>
  //
  //////////////////////////////////////////////////////////////////////////////
  #define RG244_EXE                0,0x3C    ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0 //
  #define RG244_VC_FREQ            1,0x31    ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0 //
  #define RG244_VC_AMP             2,0x32    ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0 //
  //#define RG244_GRID_SPEED_CYC     3,0x2E    ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0 //
  //#define RG244_GRID_SLOW_CYC      4,0x32    ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0 //
  #define RG244_IO_EXP_B           3,0x48    ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0 //
  #define RG244_SETTINGS           4,0x24    ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0 //
  
  #define PCB244_NREGISTERS    5	

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
  #ifdef _PCB244_C
  volatile _DeviceRegItem_Str  PCB244_Registers[]=
  {
    _REGDEF(RG244_EXE),   
    _REGDEF(RG244_VC_FREQ),   
    _REGDEF(RG244_VC_AMP),   
    _REGDEF(RG244_IO_EXP_B),   
    _REGDEF(RG244_SETTINGS)
  }; 
  #else
  extern volatile const _DeviceRegItem_Str  PCB244_Registers[PCB244_NREGISTERS]; 
  #endif
 
  #if MAX_NLIST < PCB244_NREGISTERS     
    #error "PCB244: LISTA REGISTRI INSUFFICIENTE" 
  #endif

  //////////////////////////////////////////////////////////////////////////////
  //                    DEFINIZIONE DI BITFIELD NEL TARGET
  //////////////////////////////////////////////////////////////////////////////
//  #define PCB244_BIT            _REGID(RG_FLAGS0),0,PCB215_Registers

  //////////////////////////////////////////////////////////////////////////////
  //                    CODICI COMANDI SERIALI
  //////////////////////////////////////////////////////////////////////////////
    #define PCB244_RESET_BOARD            1,1 // Software Reset
    #define PCB244_GET_FWREV              2,0 // Chiede la revisione corrente
    #define PCB244_RESET_FAULT            5,0 // Reset Faults

    #define PCB244_START_3D               7,0 //  Chiede la partenza della griglia 2D
    #define PCB244_STOP_3D                8,0 //  Chiede la partenza della griglia 2D

    #define PCB244_START_2D               9,0 //  Chiede la partenza della griglia 2D
    #define PCB244_STOP_2D                10,0 //  Chiede la partenza della griglia 2D


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
 // #define PCB244_FAULT          _REGID(RG244_FLAGS0),1,PCB244_Registers
 
  //////////////////////////////////////////////////////////////////////////////
  //                    CODICI PER ATTIVAZIONE REGISTRO EXE
  //////////////////////////////////////////////////////////////////////////////
  #define PCB244_START_VC       1 // Start voice coil
  #define PCB244_STOP_GRID      0 // Stop Grid

  ext _DeviceContext_Str PCB244_CONTEST;
  ext bool pcb244isPresent;

  // API ////////////////////////////////////////////////////////////////// 
  ext void pcb244_driver(uint32_t initial_data);  // Driver PCB244
  ext bool pcb244UpdateRegisters(void);           // Update manuale dei registri di base
  
  // Attivazione griglia 3D
  ext bool pcb244StartVoiceCoil(void);
  ext bool pcb244StopVoiceCoil(void);
  ext bool pcb244SetFreqVoiceCoil(unsigned char val);
  ext bool pcb244SetAmplVoiceCoil(unsigned char val);


  // Attivazione griglia 2D da seriale
  ext bool pcb244Start2d(void);
  ext bool pcb244Stop2d(void);
  ext bool pcb244ResetFaults(void);
  ext bool pcb244ResetBoard(void);
  ext bool config_pcb244(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len);

  
#endif
