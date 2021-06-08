/*

Aut: M. Rispoli
Data di Creazione: 01/11/2014
*/
#ifndef _PCB249U2_H
#define _PCB249U2_H

#ifdef ext
#undef ext
#undef extrd
#endif
#ifdef _PCB249U2_C
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
    unsigned char error;           // Fault code / message code (dipende dal comando specifico)
  }_PCB249U2_Stat_Str;

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
  //            #define PCB249U2_NREGISTERS     <last id+1>
  //
  //////////////////////////////////////////////////////////////////////////////
  #define _PSYSU2       0x38 
  #define _PMIR         0x4E
  #define _PADDR        0xA0 
  #define _PLAMP        0x6B
  #define _pFILTER      0x66
  

  #define RG249U2_SYS_FLAGS0       0,_PSYSU2+0     ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0 // Flags di systema
  #define RG249U2_ERRORS           1,_PSYSU2+3     ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0 // Registro Errori
  #define RG249U2_MIRROR_STAT      2,_PMIR+0       ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0 // Registro Stato Specchio
  #define RG249U2_LAMP_STAT        3,_PLAMP+0      ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0 // Registro Stato Lampada

  // Parametri per il posizionamento del rullo filtri
  #define RG249U2_PR_FILTER0       4,_PADDR+0    ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0 // Posizione filtro 0        
  #define RG249U2_PR_FILTER1       5,_PADDR+1    ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0 // Posizione filtro 1        
  #define RG249U2_PR_FILTER2       6,_PADDR+2    ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0 // Posizione filtro 2        
  #define RG249U2_PR_FILTER3       7,_PADDR+3    ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0 // Posizione filtro 3        
  
  // Parametri per il posizionamento dello specchio
  #define RG249U2_PR_MIRROR_STEPS  8,_PADDR+4    ,_BNK01,_16BIT ,_RW, _NVL, (unsigned short)0 // Steps posizionamento specchio        
  #define RG249U2_PR_MAX_FR        9,_PADDR+14   ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0 // Max numero di steps posizionamento lama frontale        
  
  #define RG249U2_PR_CALIBRATED    10,_PADDR+15  ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0 // Sistema è sttato calibrato
    

  #define RG249U2_FILTER_CURPOS    11,_pFILTER+0 ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0 // Posizione corrente
  #define RG249U2_FILTER_STAT      12,_pFILTER+1 ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0 // Flag di target movimento filtro
  #define RG249U2_POS_TARGET       13,_pFILTER+2 ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0 // Target corrente filtro
  #define PCB249U2_NREGISTERS      14


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
  #ifdef _PCB249U2_C
  
volatile _DeviceRegItem_Str  PCB249U2_Registers[]=
  {
    _REGDEF(RG249U2_SYS_FLAGS0),
    _REGDEF(RG249U2_ERRORS),
    _REGDEF(RG249U2_MIRROR_STAT),
    _REGDEF(RG249U2_LAMP_STAT),
    _REGDEF(RG249U2_PR_FILTER0),
    _REGDEF(RG249U2_PR_FILTER1),
    _REGDEF(RG249U2_PR_FILTER2),
    _REGDEF(RG249U2_PR_FILTER3),
    _REGDEF(RG249U2_PR_MIRROR_STEPS),
    _REGDEF(RG249U2_PR_MAX_FR),
    _REGDEF(RG249U2_PR_CALIBRATED),

    _REGDEF(RG249U2_FILTER_CURPOS),
    _REGDEF(RG249U2_FILTER_STAT),
    _REGDEF(RG249U2_POS_TARGET)

  }; 
  #else
  extern volatile const _DeviceRegItem_Str  PCB249U2_Registers[PCB249U2_NREGISTERS]; 
  #endif
 
  #if MAX_NLIST < PCB249U2_NREGISTERS     
    #error "PCB249U2: LISTA REGISTRI INSUFFICIENTE" 
  #endif

  //////////////////////////////////////////////////////////////////////////////
  //                    DEFINIZIONE DI BITFIELD NEL TARGET
  //////////////////////////////////////////////////////////////////////////////
  //  #define PCB249U2_BIT            _REGID(RG_FLAGS0),0,PCB215_Registers

  //////////////////////////////////////////////////////////////////////////////
  //                    CODICI COMANDI SERIALI
  //////////////////////////////////////////////////////////////////////////////
  #define PCB249U2_RESET_BOARD        1,1 // Software Reset 
  #define PCB249U2_CLR_RESET_FLG      1,0 // Azzera il Flag di reset
  #define PCB249U2_GET_REV            2,0 // Software Reset   
  
  #define PCB249U2_CLEAR_ERRORS       3,0// Cancella gli errori dal registro errori

  #define PCB249U2_FACTORY_ALL        4,0// Set di tutti i registri al valore di factory
  #define PCB249U2_FACTORY_REG        4  // Set di un dato registro
  
  #define PCB249U2_EEPROM_STORE_ALL   5,0// Save di tutti i registri in EPROM
  #define PCB249U2_EEPROM_STORE_REG   5  // Save di un registro  in EPROM
  
  #define PCB249U2_FILTER             6,0// Posizionamento Filtro in 0
  #define PCB249U2_FILTER_POS0        6,0// Posizionamento Filtro in 0
  #define PCB249U2_FILTER_POS1        6,1// Posizionamento filtro in 1
  #define PCB249U2_FILTER_POS2        6,2// Posizionamento filtro in 2
  #define PCB249U2_FILTER_POS3        6,3// Posizionamento filtro in 3

  #define PCB249U2_FILTER_RAW         7,0  // Posizionamento filtro in posizione RAW

  
  #define PCB249U2_CALIB_FILTER0      8,0// Calibrazione Filtro in 0
  #define PCB249U2_CALIB_FILTER1      8,1// Calibrazione filtro in 1
  #define PCB249U2_CALIB_FILTER2      8,2// Calibrazione filtro in 2
  #define PCB249U2_CALIB_FILTER3      8,3// Calibrazione filtro in 3
  #define PCB249U2_VALIDATE_FILTER    8,4// Valida i dati di calibrazione filtri
  #define PCB249U2_UNVALIDATE_FILTER  8,5// Toglie la validazione ai dati di calibrazionefiltro
  #define PCB249U2_TEST_BUSY_FILTER   8,6// Se risponde OK allora il busy è OFF
  
 
  #define PCB249U2_MIRROR_HOME        9,0// Mirror in posizione Home
  #define PCB249U2_MIRROR_OUT         10,0// Mirror in Campo
  #define PCB249U2_MIRROR_FWD         11,0 // Muove fine (0:255 step) in campo
  #define PCB249U2_MIRROR_BCK         12,0 // Muove fine (0:255 step) in home
  
  /*
      BIT[0:5] = Timeout (0==INFINITO)        ; Sezione comandi LUCE centratore
      BIT6: 1 = LUCE ON; 0 = LUCE OFF
      BIT7: 1 = MUOVE SPECCHIO, 0 = NON MUOVE SPECCHIO
  */
  #define PCB249U2_LAMP               13,0 // Luce + MIrror 

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
  #define PCB249U2_RST_FLG        _REGID(RG249U2_SYS_FLAGS0),0,PCB249U2_Registers
  #define PCB249U2_FAULT          _REGID(RG249U2_SYS_FLAGS0),1,PCB249U2_Registers
  #define PCB249U2_BUSY           _REGID(RG249U2_SYS_FLAGS0),2,PCB249U2_Registers
  #define PCB249U2_COLLI_OK       _REGID(RG249U2_SYS_FLAGS0),3,PCB249U2_Registers
 
  #define PCB249U2_MIR_ON_FLG       _REGID(RG249U2_MIRROR_STAT),0,PCB249U2_Registers
  #define PCB249U2_MIR_TMO_FLG     _REGID(RG249U2_MIRROR_STAT),1,PCB249U2_Registers
  #define PCB249U2_MIR_HOME_FLG    _REGID(RG249U2_MIRROR_STAT),2,PCB249U2_Registers
  #define PCB249U2_MIR_OUT_FLG     _REGID(RG249U2_MIRROR_STAT),7,PCB249U2_Registers
    
  #define PCB249U2_LAMP_ON_FLG     _REGID(RG249U2_LAMP_STAT),0,PCB249U2_Registers

  ext _DeviceContext_Str PCB249U2_CONTEST;
 
  // API ////////////////////////////////////////////////////////////////// 
  ext void pcb249U2_driver(uint32_t initial_data);  // Driver PCB249U2
  ext bool pcb249U2Mirror(unsigned char cmd); // Attivazione Specchio
  ext bool pcb249U2Lamp(unsigned char cmd, unsigned char tmo, bool wait); // Attivazione lampada
  ext bool pcb249U2SetColli(unsigned char back, unsigned char front);   // Imposta il formato
  ext void pcb249U2SetFiltro(unsigned char cmd, unsigned char target_position, unsigned char id);   // Imposta il filtro
  ext bool config_pcb249U2(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len);
  ext void pcb249U2ResetCalibFilterFlag(void);
  ext bool pcb249U2ColliCmd(unsigned char back, unsigned char front); //Usata dalla funzione ragg

  ext bool pcb249U2SetFiltroRaw(unsigned char val);
  ext int getTomoDeltaFilter(int angolo);
  ext bool pcb249U2RxSetFiltroCmd(unsigned char cmd);
  ext bool waitRxFilterCompletion(void);
  ext bool wait2DBackFrontCompletion(int timeout);

  // Sezione dedicata al filtro e alla lampada
  ext unsigned char filtro_req;     // ultima richiesta di filtro
  ext unsigned char pos_req;        // ultima richiesta di posizione filtro
  ext unsigned char id_filtro;      // codice ID per feedback alla gui
  ext bool          filtro_eseguito;// esito ultimo comando


  //ext int comando_backfront;
  ext unsigned char backcolli_req,frontcolli_req; // Richiesta di spostamento lame frontali e posteriori
  ext bool backfront_eseguito;
#endif
