/*

Aut: M. Rispoli
Data di Creazione: 24/10/2014
*/
#ifndef _PCB190_H
#define _PCB190_H

#ifdef ext
#undef ext
#undef extrd
#endif
#ifdef _PCB190_C
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
    unsigned char i_test:1;       // Corrente di test attivata
    unsigned char spare:3;
    //--------------------------
    
    // OBBLIGATORIO
    unsigned char maj_code;        // Codice Firmware letto da periferica
    unsigned char min_code;        // Codice Firmware letto da periferica
    unsigned char error;           // Fault code
  }_PCB190_Stat_Str;

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
  //            #define PCB190_NREGISTERS     <last id+1>
  //
  //////////////////////////////////////////////////////////////////////////////
  #define _MAP_0  0x38
  #define _pAR_DATA 0xC0       // BANCHI 0/1

  #define _pPAR   0xA0
  #define _pMAS   0x6b          // Banco 2/3
  #define _pSAMPLE 0x10         // BANCO 2/3 
 #define _pSAMPLE_TIME 0x90     // BANCO 2/3

  #define _pMainData 0x60       // BANCO 2,3
  #define PCB190_NSAMPLES 0x20 
  #define PCB190_NSAMPLES_TIME 0x20
          
  #define RG190_FLAGS0             0,0+_MAP_0  ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0 // Flags 
  #define RG190_FAULTS             1,6+_MAP_0  ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0 // Flags 
  #define RG190_RXHVTMO            2,7+_MAP_0  ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0 // Tempo massimo di esposizione in 100ms unit
  #define RG190_RXHVEXP            3,8+_MAP_0  ,_BNK01,_16BIT,_RW, _VL, (unsigned short)0 // Livello di alta tensione da impostare (0:4095)
  #define RG190_RXI                4,10+_MAP_0 ,_BNK01,_16BIT,_RW, _VL, (unsigned short)0 // Corrente target filamento selezionato (0:4095)
  #define RG190_RXMAS              5,12+_MAP_0 ,_BNK01,_16BIT,_RW, _VL, (unsigned short)0 // Totale MAS da contare durante i raggi
  #define RG190_MIN_HV             6,14+_MAP_0 ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0 // Diagnostica Minimo valore HV
  #define RG190_MAX_HV             7,15+_MAP_0 ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0 // Diagnostica Massimo valore HV
  #define RG190_MIN_IANODICA       8,16+_MAP_0 ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0 // Diagnostica Minimo valore I-ANODICA
  #define RG190_MAX_IANODIA        9,17+_MAP_0 ,_BNK01,_8BIT ,_RW,  _VL, (unsigned short)0 // Diagnostica Massimo valore I-ANODICA
  #define RG190_RXCHK             10,18+_MAP_0 ,_BNK01,_8BIT ,_RW,  _VL, (unsigned short)0 // Byte checksum per validazione dati RX
  #define RG190_MAS_EXIT          11,19+_MAP_0 ,_BNK01,_16BIT,_RD, _NVL, (unsigned short)0 // Valore residuale MAS dopo un'interruzione raggi
  //#define RG190_TOMO_SAMPLES      12,21+_MAP_0 ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0  // Tomo Pre sampling
  //#define RG190_TOMO_PREWIN       13,22+_MAP_0 ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0  // Tomo Sampling
  #define RG190_I_ANODICA         12,23+_MAP_0 ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0  // Lettura corrente anodica
  #define RG190_HV                13,25+_MAP_0 ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0  // Lettura HV
  #define RG190_IFIL              14,26+_MAP_0 ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0  // Lettura I FIL
  #define RG190_VFIL              15,27+_MAP_0 ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0  // Lettura V FIL
  #define RG190_FUOCO             16,31+_MAP_0 ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0 // Selezione corrente del fuoco
  #define RG190_GRID              17,32+_MAP_0 ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0 // Selezione corrente Griglia
  
  #define PR190_IWARM             18, 8+_pPAR ,_BNK01,_16BIT,_RW, _NVL, (unsigned short)0  // Corrente di riscaldamento filamento
  #define PR190_STARTER           19,10+_pPAR ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0  // 0 = INTERNO, 1 = STARTER IAE

  #define REG190_RX_TIME          20,0+_pSAMPLE ,_BNK23,_16BIT ,_RD, _VL, (unsigned short)0  // Indice campioni BUFFER 0 I ANODICA
  #define REG190_RX_TIME_PRE      21,2+_pSAMPLE ,_BNK23,_16BIT ,_RD, _VL, (unsigned short)0  // Indice campioni BUFFER 0 I ANODICA
  #define REG190_RX_TIME_PLS      22,4+_pSAMPLE ,_BNK23,_16BIT ,_RD, _VL, (unsigned short)0  // Indice campioni BUFFER 0 I ANODICA

  #define PR190_N_SAMPLE_AEC      23,6+_pSAMPLE ,_BNK23,_8BIT ,_RD, _NVL, (unsigned short)0  // Indice campioni BUFFER 0 I ANODICA
  #define PR190_N_SAMPLE_I        24,7+_pSAMPLE ,_BNK23,_8BIT ,_RD, _NVL, (unsigned short)0  // Indice campioni BUFFER 0 I ANODICA
  #define PR190_N_SAMPLE_V        25,8+_pSAMPLE ,_BNK23,_8BIT ,_RD, _NVL, (unsigned short)0  // Indice campioni BUFFER 1 HV
  #define PR190_SAMPLES_I         26,9+_pSAMPLE ,_BNK23,_8BIT ,_RD, _NVL, (unsigned short)0  // Indice campioni BUFFER 0 I ANODICA
  #define PR190_SAMPLES_V         27,PCB190_NSAMPLES+9+_pSAMPLE ,_BNK23,_8BIT ,_RD, _NVL, (unsigned short)0  // Indice campioni BUFFER 0 I ANODICA

  //#define PR190_NSAMPLES_TIME     30,1+_pSAMPLE_TIME ,_BNK23,_8BIT ,_RD, _VL, (unsigned short)0  // Indice campionamenti di tempo in tomo
  //#define PR190_SAMPLES_TIME      31,2+_pSAMPLE_TIME ,_BNK23,_8BIT ,_RD, _VL, (unsigned short)0  // Indice campioni BUFFER 0 I ANODICA


  #define RG190_AEC               28,_MAP_0+4 ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0  // Configurazione

  // Nuovi indirizzi per diagnostica tensioni
  #define RG190_HV_RXEND          29,34+_MAP_0 ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0 // HV campionata al termine raggi
  #define RG190_V_HV              30,36+_MAP_0 ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0 // HV campionata in IDLE
  #define RG190_VAMPL_FIL         31,37+_MAP_0 ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0 // V Temperatura amplificatore filamento
  #define RG190_MAS_TEST          32,01+_pMAS  ,_BNK23,_16BIT ,_RD, _NVL, (unsigned short)0 // Lettura MAS di test
  #define RG190_MAS_DATA          33,03+_pMAS  ,_BNK23,_16BIT ,_RD, _NVL, (unsigned short)0 // Lettura MAS rx

  // Diagnostica tensioni
  #define RG190_V_32              34,0+_pMainData ,_BNK23,_8BIT ,_RD, _NVL, (unsigned short)0
  #define RG190_V_M32             35,1+_pMainData ,_BNK23,_8BIT ,_RD, _NVL, (unsigned short)0
  #define RG190_V_15              36,2+_pMainData ,_BNK23,_8BIT ,_RD, _NVL, (unsigned short)0
  #define RG190_V_12              37,3+_pMainData ,_BNK23,_8BIT ,_RD, _NVL, (unsigned short)0
  #define RG190_V_M12             38,4+_pMainData ,_BNK23,_8BIT ,_RD, _NVL, (unsigned short)0
  #define RG190_V_15EXT           39,5+_pMainData ,_BNK23,_8BIT ,_RD, _NVL, (unsigned short)0

  #define RG_SAMPLED_IFIL         40,35+_MAP_0 ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0
  #define RG16_IFIL_DAC           41,38+_MAP_0 ,_BNK01,_16BIT ,_RD, _NVL, (unsigned short)0

  #define PR16_MAX_IFIL           42, 0+_pPAR ,_BNK01,_16BIT ,_RW, _NVL, (unsigned short)0  // Configurazione Raggi
  #define PR_IFIL_LIMIT           43, 5+_pPAR ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0  // Configurazione Raggi

  #define PR190_DELAY_DGN         44, 2+_pPAR ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0  // Configurazione Raggi

  // Registri di campinamento correnti Starter interno (Se configurato)
  #define RG190_AR_STAT           45,0+_pAR_DATA ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0
  #define RG190_AR_MAIN_IRUN      46,8+_pAR_DATA ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0
  #define RG190_AR_SHIFT_IRUN     47,9+_pAR_DATA ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0
  #define RG190_AR_MAIN_IKEEP     48,10+_pAR_DATA ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0
  #define RG190_AR_SHIFT_IKEEP    49,11+_pAR_DATA ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0
  #define RG190_AR_MAIN_IOFF      50,12+_pAR_DATA ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0
  #define RG190_AR_SHIFT_IOFF     51,13+_pAR_DATA ,_BNK01,_8BIT ,_RD, _NVL, (unsigned short)0

  // Contatori di lancio
  #define RG190_ARLS_COUNT        52,15+_pAR_DATA ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0
  #define RG190_ARHS_COUNT        53,28+_MAP_0 ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0


  // Parametri diagnostica starter
  #define PR_IMAIN_OFF_MAX        54,15+_pPAR ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0
  #define PR_ISHIFT_OFF_MAX       55,16+_pPAR ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0
  #define PR_IMAIN_RUN_MAX        56,17+_pPAR ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0
  #define PR_ISHIFT_RUN_MAX       57,18+_pPAR ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0
  #define PR_IMAIN_KEEP_MAX       58,19+_pPAR ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0
  #define PR_ISHIFT_KEEP_MAX      59,20+_pPAR ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0
  #define PR_IMAIN_RUN_MIN        60,21+_pPAR ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0
  #define PR_ISHIFT_RUN_MIN       61,22+_pPAR ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0
  #define PR_IMAIN_KEEP_MIN       62,23+_pPAR ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0
  #define PR_ISHIFT_KEEP_MIN      63,24+_pPAR ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0

  #define PR_RX_OPT               64,4+_pPAR ,_BNK01,_8BIT ,_RW, _NVL, (unsigned short)0


  #define PCB190_NREGISTERS       65

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

  #ifdef _PCB190_C
  volatile _DeviceRegItem_Str  PCB190_Registers[]=
  {
    _REGDEF(RG190_FLAGS0),
    _REGDEF(RG190_FAULTS),
    _REGDEF(RG190_RXHVTMO),   
    _REGDEF(RG190_RXHVEXP),
    _REGDEF(RG190_RXI),   
    _REGDEF(RG190_RXMAS),   
    _REGDEF(RG190_MIN_HV),
    _REGDEF(RG190_MAX_HV),   
    _REGDEF(RG190_MIN_IANODICA),
    _REGDEF(RG190_MAX_IANODIA),   
    _REGDEF(RG190_RXCHK),
    _REGDEF(RG190_MAS_EXIT),  

    _REGDEF(RG190_I_ANODICA),  
    _REGDEF(RG190_HV),  
    _REGDEF(RG190_IFIL),  
    _REGDEF(RG190_VFIL),  
    _REGDEF(RG190_FUOCO),
    _REGDEF(RG190_GRID),  
   


    _REGDEF(PR190_IWARM),
    _REGDEF(PR190_STARTER),   

    _REGDEF(REG190_RX_TIME),
    _REGDEF(REG190_RX_TIME_PRE),
    _REGDEF(REG190_RX_TIME_PLS),

    _REGDEF(PR190_N_SAMPLE_AEC),
    _REGDEF(PR190_N_SAMPLE_I),
    _REGDEF(PR190_N_SAMPLE_V),
    _REGDEF(PR190_SAMPLES_I),
    _REGDEF(PR190_SAMPLES_V),



    _REGDEF(RG190_AEC),
    
    _REGDEF(RG190_HV_RXEND),
    _REGDEF(RG190_V_HV),
    _REGDEF(RG190_VAMPL_FIL),
    _REGDEF(RG190_MAS_TEST),
    _REGDEF(RG190_MAS_DATA),
    
    _REGDEF(RG190_V_32),
    _REGDEF(RG190_V_M32),
    _REGDEF(RG190_V_15),
    _REGDEF(RG190_V_12),
    _REGDEF(RG190_V_M12),
    _REGDEF(RG190_V_15EXT),
    _REGDEF(RG_SAMPLED_IFIL),
    _REGDEF(RG16_IFIL_DAC),

    _REGDEF(PR16_MAX_IFIL),
    _REGDEF(PR_IFIL_LIMIT),
    _REGDEF(PR190_DELAY_DGN),


    _REGDEF(RG190_AR_STAT),
    _REGDEF(RG190_AR_MAIN_IRUN),
    _REGDEF(RG190_AR_SHIFT_IRUN),
    _REGDEF(RG190_AR_MAIN_IKEEP),
    _REGDEF(RG190_AR_SHIFT_IKEEP),
    _REGDEF(RG190_AR_MAIN_IOFF),
    _REGDEF(RG190_AR_SHIFT_IOFF),

      _REGDEF(RG190_ARLS_COUNT),
      _REGDEF(RG190_ARHS_COUNT),

      _REGDEF(PR_IMAIN_OFF_MAX),
      _REGDEF(PR_ISHIFT_OFF_MAX),
      _REGDEF(PR_IMAIN_RUN_MAX),
      _REGDEF(PR_ISHIFT_RUN_MAX),
      _REGDEF(PR_IMAIN_KEEP_MAX),
      _REGDEF(PR_ISHIFT_KEEP_MAX),
      _REGDEF(PR_IMAIN_RUN_MIN),
      _REGDEF(PR_ISHIFT_RUN_MIN),
      _REGDEF(PR_IMAIN_KEEP_MIN),
      _REGDEF(PR_ISHIFT_KEEP_MIN),
      _REGDEF(PR_RX_OPT)


  }; 
  
  #else
  extern volatile const _DeviceRegItem_Str  PCB190_Registers[PCB190_NREGISTERS]; 
  #endif
 
  #if MAX_NLIST < PCB190_NREGISTERS     
    #error "PCB190: LISTA REGISTRI INSUFFICIENTE" 
  #endif

  //////////////////////////////////////////////////////////////////////////////
  //                    DEFINIZIONE DI BITFIELD NEL TARGET
  //////////////////////////////////////////////////////////////////////////////
//  #define PCB190_BIT            _REGID(RG_FLAGS0),0,PCB215_Registers

  //////////////////////////////////////////////////////////////////////////////
  //                    CODICI COMANDI SERIALI
  //////////////////////////////////////////////////////////////////////////////
  #define PCB190_RESET_BOARD       1,1 // Software Reset 
  #define PCB190_CLR_RESET         1,0 // Clear flag di Reset
  #define PCB190_GET_FWREV         2,0 // Chiede revisione software
  #define PCB190_RST_FAULTS        3,0 // Reset Faults correnti
  #define PCB190_SELECT_FUOCO      4,0 // Selezione Fuoco <par>
  #define PCB190_GRID_ON           5,0 // Accende/Spegne la griglia <par>
  #define PCB190_START_RX_STD      6,1 // Attivazione sequenza RX Standard
  #define PCB190_START_RX_STD_AEC  6,2 // Attivazione sequenza RX Standard
  #define PCB190_START_RX_TOMO     6,4 // Attivazione sequenza Tomo Standard
  #define PCB190_START_RX_TOMO_AEC 6,8 // Attivazione sequenza Tomo Aec

  #define PCB190_START_AR_L        7,1 // Attivazione Starter bassa velocità
  #define PCB190_START_AR_H        7,2 // Attivazione Starter alta Velocità

  #define PCB190_OFF_AR            7,0 // Stop Starter
  #define PCB190_STOP_AR           7,3 // Stop Starter with brake (Internal starter only)

  #define PCB190_SET_IMAS          9,1 // Attivazione corrente di test Anodica
  #define PCB190_CLR_IMAS          9,0 // Disattivazione corrente di test Anodica
  
  #define PCB190_READ_MAS          10,0 // Attivazione Masmetro per x time (10ms) param
  #define PCB190_SET_FILON         11,0 // Attivazione Filamento per x time (100ms) param
 
  #define PCB190_GET_MAS_LOW       14,4 // Lettura Masmetro dopo Test (LB)
  #define PCB190_GET_MAS_HIGH      14,5 // Lettura Masmetro dopo Test (HB)
  
  // ---------------------------------------------------------------------------
  //                            COMANDI PER TARATURE 
  // ---------------------------------------------------------------------------
  #define PCB190_SET_IA_TEST        9,0 // Attivazione corrente anodica di Test <par>
  #define PCB190_RD_MAS_TEST       10,0 // Lettura MAS a Tempo <par>
  #define PCB190_FILON_TEST        11,0 // Accensione/Spegnimento filamento a tempo <par>

  // ---------------------------------------------------------------------------
  //                            COMANDI PER DIAGNOSTICA 
  // ---------------------------------------------------------------------------
  #define PCB190_GET_SAMPLE0       12,0 // Lettura campione vettore SAMPLE0 <par>
  #define PCB190_GET_SAMPLE1       13,0 // Lettura campione vettore SAMPLE1 <par>
  #define PCB190_GET_SPECIAL       14,0 // Lettura registri speciali <par>

  // ---------------------------------------------------------------------------
  //                            COMANDI PER COLLAUDO
  // ---------------------------------------------------------------------------
  #define PCB190_SET_COLLAUDO      15,0 // Attivazione COllaudo

  #define PCB190_ANALOG_RXSTOP     16,0 // AtTermina una sequenza Analogica


  //////////////////////////////////////////////////////////////////////////////
  //                    DEFINIZIONE DI BITFIELD NEL TARGET
  //////////////////////////////////////////////////////////////////////////////
  #define PCB190_RESET          _REGID(RG190_FLAGS0),0,PCB190_Registers
  #define PCB190_FAULT          _REGID(RG190_FLAGS0),1,PCB190_Registers
  #define PCB190_RX_BUSY        _REGID(RG190_FLAGS0),2,PCB190_Registers
  #define PCB190_FILON          _REGID(RG190_FLAGS0),3,PCB190_Registers
  #define PCB190_GRIDON         _REGID(RG190_FLAGS0),4,PCB190_Registers

  #define PCB190_AROFF_FLG      _REGID(RG190_AR_STAT),0,PCB190_Registers
  #define PCB190_ARRUN_FLG      _REGID(RG190_AR_STAT),1,PCB190_Registers
  #define PCB190_ARKEEP_FLG     _REGID(RG190_AR_STAT),2,PCB190_Registers
  #define PCB190_ARBRK_FLG      _REGID(RG190_AR_STAT),3,PCB190_Registers

  //////////////////////////////////////////////////////////////////////////////
  //                    CODICI FUOCHI
  //////////////////////////////////////////////////////////////////////////////
  #define PCB190_F1G	1       // Codice associato al fuoco F1 Grande
  #define PCB190_F1P	2       // Codice associato al fuoco F1 Piccolo
  #define PCB190_F2G	3       // Codice associato al fuoco F2 Grande
  #define PCB190_F2P	4       // Codice associato al fuoco F2 Piccolo

  ext _DeviceContext_Str PCB190_CONTEST;
  
  // API ////////////////////////////////////////////////////////////////// 
  ext void pcb190_driver(uint32_t initial_data);                // Driver PCB190
  ext int pcb190StartRxStd(void);      // Richiede attivazione sequenza standard manuale
  ext int pcb190StartRxAecStd(void);
  ext bool pcb190UpdateRegisters(void); // Update manuale dei registri di base
  ext bool pcb190StopStarter(void);
  ext bool pcb190OffStarter(void);

  ext bool pcb190StarterL(void);
  ext bool pcb190StarterH(void);
  ext int pcb190StartRxTomo(void);
  ext bool pcb190ResetFault(void);
  ext bool pcb190SetITest(bool stat); // Attiva / Disattiva la corrente di Test del masmetro
  ext bool pcb190SetFuoco(unsigned char fuoco); // Imposta il fuoco corrente
  ext signed short pcb190SendCommand(unsigned char data1, unsigned char data2);
  ext bool pcb190UploadExpose(_RxStdSeq_Str* Param, bool isAEC);
  ext bool pcb190UploadExposeAE(_RxStdSeq_Str* Param);

  ext bool pcb190UploadTomoExpose(_RxStdSeq_Str* Param, bool isAEC);
  ext int pcb190StartRxTomoAec(void);
  ext bool pcb204SetExpWinMode(bool stat);
  ext bool waitPcb190Ready(unsigned char attempt); // Attesa attempt*100ms il ready

  ext bool config_pcb190(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len);
  
  ext bool pcb190GetPostRxRegisters(void);
  // ext bool pcb190SetXrayRegisters(_RxStdSeq_Str* Param);
  ext float pcb190ConvertKvRead(unsigned char val);
  ext void  pcb190updateStarterRegisters(void);
  ext bool pcb190getUpdateStarterRegisters(void);

  ext void pcb190TestLSStarter(void);
  ext bool pcb190ConfigLSStarter(void);
  ext bool pcb190DisableLSStarterDiagnostic(void);

  ext bool pcb190UploadAnalogOnlyPreExpose(_RxStdSeq_Str* Param);
  ext bool pcb190UploadAnalogPreExpose(_RxStdSeq_Str* Param);
  ext bool pcb190UploadAnalogCalibTubeExpose(_RxStdSeq_Str* Param);
  ext bool pcb190UploadAnalogManualExpose(_RxStdSeq_Str* Param);

  ext bool pcb190AnalogRxStop(void);

  ext unsigned short pcb190GetPremAsData(void);

#endif
