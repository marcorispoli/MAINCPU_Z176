/*

Aut: M. Rispoli
Data di Creazione: 01/11/2014
*/
#ifndef _PCB249U1_H
#define _PCB249U1_H

#ifdef ext
#undef ext
#undef extrd
#endif
#ifdef _PCB249U1_C
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
  }_PCB249U1_Stat_Str;

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
  //            #define PCB249U1_NREGISTERS     <last id+1>
  //
  //////////////////////////////////////////////////////////////////////////////

  #define WRITE_MODE_ADDR 0x41
  #define N_ARRAY_L     0x110
  #define P_ARRAY_L     0x129
  #define N_ARRAY_R     0xAF
  #define P_ARRAY_R     0xC8
  #define N_ARRAY_B     0x190
  #define P_ARRAY_B     0x1A9
  
  #define _PSYS         0x38
  #define _PADDR        0xA0 
  #define _PGONIO       0x5A
  
    
  #define RG249U1_SYS_FLAGS0       0,_PSYS+0    ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0 // Flag di stato per la collimazione
  #define RG249U1_RG_TEMP          1,_PSYS+5    ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0 // Flag di stato per la collimazione
  #define RG249U1_RG_PROT          2,_PSYS+6    ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0 // Flag di stato per la collimazione

  // Parametri per il controllo della temperatura della cuffia
  #define RG249U1_PR_TEMP_L        3,_PADDR+0    ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0 // posizionamento 2D, formato User        
  #define RG249U1_PR_TEMP_H        4,_PADDR+1    ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0 // posizionamento 2D, formato User        
  #define RG249U1_PR_TEMP_ALR      5,_PADDR+2    ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0 // posizionamento 2D, formato User        
  #define RG249U1_PR_TEMP_ALR_OFF  6,_PADDR+3    ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0 // posizionamento 2D, formato User        


  // Parametri per il posizionamento delle lame in modalità 2D
  #define RG249U1_PR_2D_L_USER     7,_PADDR+6    ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0 // posizionamento 2D, formato User        
  #define RG249U1_PR_2D_R_USER     8,_PADDR+7    ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0 // posizionamento 2D, formato User        
  #define RG249U1_PR_2D_B_USER     9,_PADDR+8    ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0 // posizionamento 2D, formato User        
  
  #define PR_L_OUT                 10,_PADDR+9     ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0 // Min pos L 
  #define PR_L_IN                  11,_PADDR+10    ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0 // Max pos L
  #define PR_R_OUT                 12,_PADDR+11    ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0 // Min pos R
  #define PR_R_IN                  13,_PADDR+12    ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0 // Max pos R
  #define PR_TRAP_L                14,_PADDR+13    ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0 // Min pos Trap
  #define PR_TRAP_R                15,_PADDR+14    ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0 // Max pos Trap
  #define PR_COLLI_MODEL           16,_PADDR+4     ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0  // Modello collimatore [0/1]

  // ALTRI REGISTRI
  #define RG249U1_GONIO_OFS        17,_PGONIO+1    ,_BNK01,_16BIT ,_RW, _VL, (unsigned short)0 // Offset per impostazione inclinometro
  #define RG249U1_GONIO16_TRX      18,_PGONIO+4    ,_BNK01,_16BIT ,_RD, _VL, (unsigned short)0 // Angolo 16 bit modulo due Tubo
  #define RG249U1_GONIO16_ARM      19,_PGONIO+6    ,_BNK01,_16BIT ,_RW, _NVL,(unsigned short)0 // Angolo 16 bit modulo due Detector, da impostare

  #define RG249U1_GONIO_REL        20,_PGONIO+0    ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0 // Angolo relativo Tubo/braccio

  #define RG249U1_RIGHT_SENS       21,0x44         ,_BNK23,_8BIT ,_RD, _VL, (unsigned short)0 // APosizione effettiva lama destra
  #define RG249U1_LEFT_SENS        22,0x45         ,_BNK23,_8BIT ,_RD, _VL, (unsigned short)0 // APosizione effettiva lama sinistra
  #define RG249U1_TRAP_SENS        23,0x46         ,_BNK23,_8BIT ,_RD, _VL, (unsigned short)0 // APosizione effettiva lama trapezio

  #define PCB249U1_NREGISTERS      24


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
  #ifdef _PCB249U1_C
  

volatile _DeviceRegItem_Str  PCB249U1_Registers[]=
  {
    _REGDEF(RG249U1_SYS_FLAGS0),
    _REGDEF(RG249U1_RG_TEMP),
    _REGDEF(RG249U1_RG_PROT),
    
    _REGDEF(RG249U1_PR_TEMP_L),
    _REGDEF(RG249U1_PR_TEMP_H),
    _REGDEF(RG249U1_PR_TEMP_ALR),
    _REGDEF(RG249U1_PR_TEMP_ALR_OFF),    
    
    _REGDEF(RG249U1_PR_2D_L_USER),
    _REGDEF(RG249U1_PR_2D_R_USER),
    _REGDEF(RG249U1_PR_2D_B_USER),
    
    _REGDEF(PR_L_OUT),   
    _REGDEF(PR_L_IN),   
    _REGDEF(PR_R_OUT),   
    _REGDEF(PR_R_IN),   
    _REGDEF(PR_TRAP_L),   
    _REGDEF(PR_TRAP_R),       
    _REGDEF(PR_COLLI_MODEL),
    
    _REGDEF(RG249U1_GONIO_OFS),
    _REGDEF(RG249U1_GONIO16_TRX),
    _REGDEF(RG249U1_GONIO16_ARM),

    _REGDEF(RG249U1_GONIO_REL ),

    _REGDEF(RG249U1_RIGHT_SENS ),
    _REGDEF(RG249U1_LEFT_SENS ),
    _REGDEF(RG249U1_TRAP_SENS )

  }; 
  #else
  extern volatile const _DeviceRegItem_Str  PCB249U1_Registers[PCB249U1_NREGISTERS]; 
  #endif
 
  #if MAX_NLIST < PCB249U1_NREGISTERS     
    #error "PCB249U1: LISTA REGISTRI INSUFFICIENTE" 
  #endif

  //////////////////////////////////////////////////////////////////////////////
  //                    DEFINIZIONE DI BITFIELD NEL TARGET
  //////////////////////////////////////////////////////////////////////////////
  //  #define PCB249U1_BIT            _REGID(RG_FLAGS0),0,PCB215_Registers

  //////////////////////////////////////////////////////////////////////////////
  //                    CODICI COMANDI SERIALI
  //////////////////////////////////////////////////////////////////////////////
  #define PCB249U1_RESET_BOARD        1,1 // Software Reset 
  #define PCB249U1_GET_REV            2,1 // Software Reset 
  #define PCB249U1_CLEAR_ERRORS       3,0 // Cancella gli errori dal registro errori
  #define PCB249U1_RESET_GONIO        4,0 // Reset inclinometro all'angolo definito
  #define PCB249U1_GET_GONIO          5,0 // Richiesta valore angolo relativo
  #define PCB249U1_SET_COLLI_24x30    6,0 // Impostazione Modo collimazione 2D 24x30
  #define PCB249U1_SET_COLLI_18x24    6,1 // Impostazione Modo collimazione 2D 18x24
  #define PCB249U1_SET_COLLI_USER     6,2 // Impostazione Modo collimazione 2D USER
  #define PCB249U1_SET_COLLI_TOMO     6,3 // Impostazione Modo collimazione 3D 24x30
  #define PCB249U1_TEACH_ZERO         7,0 // Ricerca le posizioni di zero del collimatore
  #define PCB249U1_WRITE_B23          8,1 // Imposta il modo scrittura a banco 23
  #define PCB249U1_WRITE_B01          8,0 // Imposta il modo scrittura a banco 23
  
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
  #define PCB249U1_TARGET_24x30   _REGID(RG249U1_SYS_FLAGS0),0,PCB249U1_Registers
  #define PCB249U1_TARGET_18x24   _REGID(RG249U1_SYS_FLAGS0),1,PCB249U1_Registers
  #define PCB249U1_TARGET_USER    _REGID(RG249U1_SYS_FLAGS0),2,PCB249U1_Registers
  #define PCB249U1_TARGET_TOMO    _REGID(RG249U1_SYS_FLAGS0),3,PCB249U1_Registers
  #define PCB249U1_RESET          _REGID(RG249U1_SYS_FLAGS0),4,PCB249U1_Registers
  #define PCB249U1_FAULT          _REGID(RG249U1_SYS_FLAGS0),5,PCB249U1_Registers
  #define PCB249U1_BUSY           _REGID(RG249U1_SYS_FLAGS0),6,PCB249U1_Registers
  #define PCB249U1_SPARE          _REGID(RG249U1_SYS_FLAGS0),7,PCB249U1_Registers
 
  //////////////////////////////////////////////////////////////////////////////
  //                    CODICI PER ATTIVAZIONE REGISTRO EXE
  //////////////////////////////////////////////////////////////////////////////
  #define PCB249U1_START_VC       1 // Start voice coil
  #define PCB249U1_START_2D       2 // Start 2d GRID
  #define PCB249U1_STOP_GRID      0 // Stop Grid

  ext _DeviceContext_Str PCB249U1_CONTEST;
  ext bool pcb249U1SetColliCmd(unsigned char mode); // IMpostazione dei modi di collimazione Lame

  // API ////////////////////////////////////////////////////////////////// 
  ext void pcb249U1_driver(uint32_t initial_data);  // Driver PCB249U1
  ext bool pcb249U1ResetGonio(unsigned short angolo)  ; // Imposta l'angolo corrente del tubo al valore dato
  ext void pcb249U1SetColli(unsigned char left, unsigned char right, unsigned char trap, unsigned char id);
  ext bool pcb249U1initCollimator(void); // Attiva la ricerca automatica degli zeri
  ext bool config_pcb249U1(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len);
  ext bool pcb249U1ResetFaults(void);
  ext bool wait2DLeftRightTrapCompletion(int timeout);

  ext unsigned char getDynamicColliTargetR(float angolo);
  ext unsigned char getDynamicColliTargetL(float angolo);
  ext bool setColliArray(void);
  ext void pcb249U1_readGonio(void);

  // Variabili utilizzate per richiedre lo spostamento delle lame lateralio + trapezio
  ext unsigned char leftcolli_req;
  ext unsigned char rightcolli_req;
  ext unsigned char trapcolli_req;
  ext unsigned char u1colli_id;
  ext bool u1colli_result;
#endif
