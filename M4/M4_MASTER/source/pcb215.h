/*

Aut: M. Rispoli
Data di Creazione: 17/09/2014

Data Ultima Modifica:17/09/2014
*/
#ifndef _PCB215
#define _PCB215

#ifdef ext
#undef ext
#undef extrd
#endif
#ifdef _PCB215_C
  #define ext 
  #define extrd 
#else
  #define ext extern
  #define extrd extern const
#endif


  // Elenco codici di fault
  typedef enum
  {
   // Sezione errori diagnostica del Target
   _PCB215_FAULT_MOTOR,         // Target segnala stato di fault causa guasto motore          
   _PCB215_FAULT_SAFETY,        // Target segnala stato di fault causa intervento sicurezza compressione          
   _PCB215_FAULT_BATTERY,       // Target segnala stato di fault causa batteria guasta          
   _PCB215_FAULT_BOARD          // Altri errori interni alla board non ben definiti
  }_PCB215_Error_Enum;
  
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
  }_PCB215_Stat_Str;

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
  //            #define PCB215_NREGISTERS     <last id+1>
  //
  //////////////////////////////////////////////////////////////////////////////
  #define RG215_FLAGS0          0,0+0x20   ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0
  #define RG215_FLAGS1          1,1+0x20   ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0  
  
  #define RG215_ERRORS          2,7+0x20  ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0
  #define RG215_RAW_PADDLE      3,8+0x20  ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0
  #define RG215_RAW_DOSE        4,10+0x20 ,_BNK01,_16BIT,_RD, _NVL, (unsigned short)0
  #define RG215_DOSE            5,12+0x20 ,_BNK01,_16BIT,_RD, _NVL, (unsigned short)0
  #define RG215_RAW_STRENGHT    6,16+0x20 ,_BNK01,_16BIT,_RD, _NVL, (unsigned short)0
  #define RG215_STRENGTH        7,17+0x20 ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0
  #define RG215_BAT             8,18+0x20 ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0
  #define RG215_SPESSORE        9,24+0x20 ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0
  #define RG215_ROT             10,26+0x20 ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0
  #define RG215_EXTERNAL        11,28+0x20 ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0
  #define RG215_FUNC            12,36+0x20 ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0

  #define COMPRESSOR_KF0        13,2+0xA0  ,_BNK01,_16BIT ,_RW, _VL, (unsigned short)0
  #define COMPRESSOR_F0         14,4+0xA0  ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0
  #define COMPRESSOR_KF1        15,5+0xA0  ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0
  #define COMPRESSOR_F1         16,6+0xA0  ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0
  #define POSITION_PAD_TARA     17,7+0xA0   ,_BNK01,_8BIT,_RW, _VL, (unsigned short)0

  #define POSITION_LOW_MODO_0   18,8+0xA0   ,_BNK01,_16BIT,_RW, _NVL, (unsigned short)0
  #define COMPRESSOR_POS_K      19,10+0xA0  ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0
  #define COMPRESSOR_STR_K      20,20+0x20 ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0
  #define COMPRESSOR_POS_OFS    21,12+0xA0  ,_BNK01,_16BIT ,_RW, _VL, (unsigned short)0
  
  #define POSITION_LIMIT        22,15+0xA0  ,_BNK01,_16BIT,_RW, _NVL, (unsigned short)0
  
  #define COMPRESSION_THRESHOLD_H 23,33+0xA0  ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0
  #define COMPRESSION_THRESHOLD_L 24,14+0xA0  ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0
  #define COMPRESSION_TARGET    25,35+0xA0  ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0
  #define COMPRESSION_LIMIT     26,36+0xA0  ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0
  #define RG215_PEDALS          27,0x6d     ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0


  #define PCB215_NREGISTERS     28

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
  
  #ifdef _PCB215_C
  volatile _DeviceRegItem_Str  PCB215_Registers[]=
  {
   _REGDEF(RG215_FLAGS0),
   _REGDEF(RG215_FLAGS1),
   _REGDEF(RG215_ERRORS),
   _REGDEF(RG215_RAW_PADDLE),
   _REGDEF(RG215_RAW_DOSE),
   _REGDEF(RG215_DOSE),
   _REGDEF(RG215_RAW_STRENGHT),
   _REGDEF(RG215_STRENGTH),
   _REGDEF(RG215_BAT),   
   _REGDEF(RG215_SPESSORE),
   _REGDEF(RG215_ROT),
   _REGDEF(RG215_EXTERNAL),
   _REGDEF(RG215_FUNC),
   
   _REGDEF(COMPRESSOR_KF0),
   _REGDEF(COMPRESSOR_F0),
   _REGDEF(COMPRESSOR_KF1),
   _REGDEF(COMPRESSOR_F1),
   _REGDEF(POSITION_PAD_TARA),

   _REGDEF(POSITION_LOW_MODO_0),
   _REGDEF(COMPRESSOR_POS_K),
   _REGDEF(COMPRESSOR_STR_K),     
   _REGDEF(COMPRESSOR_POS_OFS),
  
    _REGDEF(POSITION_LIMIT),
  
   _REGDEF(COMPRESSION_THRESHOLD_H),
   _REGDEF(COMPRESSION_THRESHOLD_L),
   _REGDEF(COMPRESSION_TARGET),
   _REGDEF(COMPRESSION_LIMIT),
   _REGDEF(RG215_PEDALS)
  
  }; 
  #else
  extern volatile const _DeviceRegItem_Str  PCB215_Registers[PCB215_NREGISTERS]; 
  #endif
 
  #if MAX_NLIST < PCB215_NREGISTERS     
    #error "PCB215: LISTA REGISTRI INSUFFICIENTE" 
  #endif

  //////////////////////////////////////////////////////////////////////////////
  //                    DEFINIZIONE DI BITFIELD NEL TARGET
  //////////////////////////////////////////////////////////////////////////////
  #define PCB215_DISABLE_MODE   _REGID(RG215_FLAGS0),0,PCB215_Registers
  #define PCB215_PDWN_MODE      _REGID(RG215_FLAGS0),1,PCB215_Registers
  #define PCB215_AUTO_MODE      _REGID(RG215_FLAGS0),2,PCB215_Registers
  #define PCB215_MANUAL_MODE    _REGID(RG215_FLAGS0),3,PCB215_Registers
  #define PCB215_UP_MODE        _REGID(RG215_FLAGS0),4,PCB215_Registers
  #define PCB215_SBLOCCO        _REGID(RG215_FLAGS0),5,PCB215_Registers
  #define PCB215_IDLE           _REGID(RG215_FLAGS0),6,PCB215_Registers
  #define PCB215_FAULT          _REGID(RG215_FLAGS0),7,PCB215_Registers
  
  #define PCB215_STARTUP         _REGID(RG215_FLAGS1),0,PCB215_Registers
  #define PCB215_COMPRESSION     _REGID(RG215_FLAGS1),1,PCB215_Registers
  #define PCB215_TARGET          _REGID(RG215_FLAGS1),2,PCB215_Registers
  #define PCB215_DENSITY         _REGID(RG215_FLAGS1),3,PCB215_Registers
  #define PCB215_CMP_LIMIT       _REGID(RG215_FLAGS1),4,PCB215_Registers
  #define PCB215_RELEASE         _REGID(RG215_FLAGS1),5,PCB215_Registers
  #define PCB215_UPPER_POS       _REGID(RG215_FLAGS1),6,PCB215_Registers
  #define PCB215_SPAREFL1        _REGID(RG215_FLAGS1),7,PCB215_Registers
  
  #define COMP_PEDAL_UP           (_DEVREGL(RG215_PEDALS,CONTEST) & 0x1)
  #define COMP_PEDAL_DWN          (_DEVREGL(RG215_PEDALS,CONTEST) & 0x2)
  #define COMP_PEDAL_SBL          (_DEVREGL(RG215_PEDALS,CONTEST) & 0x4)
  #define COMP_PEDALS             (_DEVREGL(RG215_PEDALS,CONTEST) & 0x7)

  //////////////////////////////////////////////////////////////////////////////
  //                    CODICI COMANDI SERIALI
  //////////////////////////////////////////////////////////////////////////////
  #define PCB215_GET_FWREV        15,0
  #define PCB215_RESET_BOARD       2,1
  #define PCB215_SET_SBLOCCO       5,0
  #define PCB215_SET_IDLE          6,0
  #define PCB215_MOVE_UP           4,0  // <par> 255= Limit, 0 = step, else (mm)
  #define PCB215_MOVE_DWN          3,0  // <par> 0= step, 1=Auto-Mode
  #define PCB215_RST_FAULTS       11,0

  ext _DeviceContext_Str PCB215_CONTEST;

  // Struttura di gestione funzioni del compressore
  // Usato dalla variabile generale delle configurazioni
  typedef struct
  {
      //unsigned char pad_thresholds[10];      // Soglie per il riconoscimento del pad
      compressoreCnf_Str calibration;        // Passato da GUI come configurazione
      unsigned char padSelezionato;          // PAD correntemente selezionato
      bool protezionePaziente;               // Presenza protezione paziente
      bool calibrationMode;                  // Modo calibrazione attivo
      int  errors;                           // NON UTILIZZATO!
      
  }compressoreCfg_Str;
 
  // API ////////////////////////////////////////////////////////////////// 
  ext void pcb215_driver(uint32_t initial_data);                // Driver PCB215
  ext bool pcb215SetSblocco();                                  // Il carrello si porta in posizione di sblocco
  ext bool pcb215MovePadUpward(unsigned char mm);               // Il carrello si muove verso l'alto di tot mm
  ext bool pcb215MovePadDownward(unsigned char mode, bool force); // Il carrello si muove verso il basso
  ext bool pcb215SetIdle();                                     // Forza il carrello a IDLE mode

  ext bool pcb215ConfigNacchera(bool wait);                     // Carica la nuova configurazione della calibrazione nacchera
  ext bool pcb215ConfigForza(bool wait);                        // Carica la nuova configurazione della calibrazione forza

  ext void pcb215ForceUpdateData(void);                         // Forza Update dei dati 
  
  ext bool config_pcb215(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len);
  ext void pcb215SetXRaySblocco(void);
  
  ext void pcb215ActivateCalibMode(bool status);                // Forza l'ingresso/uscita dal modo calibrazione
  ext bool pcb215ResetBoard(void);
  ext int pcb215GetSpessoreNonCompresso(void); // Richiede la posizione del compressore anche se non in compressione
#endif
