#ifndef _PCB244_A_H
#define _PCB244_A_H

#ifdef ext
#undef ext
#undef extrd
#endif
#ifdef _PCB244_A_C
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
  }_PCB244_A_Stat_Str;

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
  #define _pSYS   0x20

  #define RG244_A_SYSFLAGS0          0,_pSYS   ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0
  #define RG244_A_SETTINGS           1,_pSYS+1   ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0
  #define RG244_A_FAULTS             2,_pSYS+2   ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0
  #define RG244_A_BUCKY              3,_pSYS+3   ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0
  #define RG244_A_RXCMD              4,_pSYS+8   ,_BNK01,_8BIT ,_RW, _VL, (unsigned short)0
  #define RG244_A_RXSTAT               5,_pSYS+9  ,_BNK01,_8BIT ,_RD, _VL, (unsigned short)0
  #define RG244_A_PULSES             6,_pSYS+11   ,_BNK01,_16BIT ,_RW, _VL, (unsigned short)0
  #define RG244_A_PULSES_EXIT        7,_pSYS+13   ,_BNK01,_16BIT ,_RD, _VL, (unsigned short)0
  #define RG244_A_RAD1               8,_pSYS+17  ,_BNK01,_16BIT ,_RD, _VL, (unsigned short)0
  #define RG244_A_RAD5               9,_pSYS+19  ,_BNK01,_16BIT ,_RD, _VL, (unsigned short)0
  #define RG244_A_RAD25              10,_pSYS+21  ,_BNK01,_16BIT ,_RD, _VL, (unsigned short)0
  #define RG244_A_OFFSET             11,_pSYS+23  ,_BNK01,_16BIT ,_RW, _VL, (unsigned short)0
  #define RG244_A_PRE_OFFSET         12,_pSYS+15  ,_BNK01,_16BIT ,_RW, _VL, (unsigned short)0

  #define PCB244_A_NREGISTERS    13

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
  #ifdef _PCB244_A_C
  volatile _DeviceRegItem_Str  PCB244_A_Registers[]=
  {
    _REGDEF(RG244_A_SYSFLAGS0),
    _REGDEF(RG244_A_SETTINGS),
    _REGDEF(RG244_A_FAULTS),
    _REGDEF(RG244_A_BUCKY),
    _REGDEF(RG244_A_RXCMD),
    _REGDEF(RG244_A_RXSTAT),
    _REGDEF(RG244_A_PULSES),
    _REGDEF(RG244_A_PULSES_EXIT),
    _REGDEF(RG244_A_RAD1),
    _REGDEF(RG244_A_RAD5),
    _REGDEF(RG244_A_RAD25),
    _REGDEF(RG244_A_OFFSET),
    _REGDEF(RG244_A_PRE_OFFSET)


  };
  #else
  extern volatile const _DeviceRegItem_Str  PCB244_A_Registers[PCB244_A_NREGISTERS];
  #endif

  #if MAX_NLIST < PCB244_A_NREGISTERS
    #error "PCB244-A: LISTA REGISTRI INSUFFICIENTE"
  #endif

  #define COMMAND_AEC_EXPOSE            1
  #define COMMAND_AEC_PULSE             2
  #define COMMAND_STD_EXPOSE            3
  #define COMMAND_STD_EXPOSE_NO_GRID    4
  #define COMMAND_SMP_RAD               5
  #define COMMAND_CALIB_TUBE_EXPOSE     6

  #define ENABLE_RD_RAD 0x4

  //////////////////////////////////////////////////////////////////////////////
  //                    DEFINIZIONE DI BITFIELD NEL TARGET
  //////////////////////////////////////////////////////////////////////////////
  //  #define PCB244_BIT            _REGID(RG_FLAGS0),0,PCB215_Registers

  //////////////////////////////////////////////////////////////////////////////
  //                    CODICI COMANDI SERIALI
  //////////////////////////////////////////////////////////////////////////////
    #define PCB244_A_RESET_BOARD            1,1 // Software Reset
    #define PCB244_A_GET_FWREV              2,0 // Chiede la revisione corrente
    #define PCB244_A_RESET_FAULT            5,0 // Reset Faults

    // SEQUENZE RAGGI
    #define PCB244_A_START_PRE        6,COMMAND_AEC_PULSE
    #define PCB244_A_START_STD        6,COMMAND_STD_EXPOSE
    #define PCB244_A_START_STD_NO_GRID        6,COMMAND_STD_EXPOSE_NO_GRID
    #define PCB244_A_START_AEC        6,COMMAND_AEC_EXPOSE
    #define PCB244_A_START_CALIB_TUBE 6,COMMAND_CALIB_TUBE_EXPOSE



    #define PCB244_A_SAMPLE_RAD             7,0

    #define PCB244_A_SET_FIELD_FRONT        8,0
    #define PCB244_A_SET_FIELD_MEADDLE      8,1
    #define PCB244_A_SET_FIELD_BACK         8,2
    #define PCB244_A_SET_FIELD_OPEN         8,3

    // Impostazione dell'Offset
    #define PCB244_A_SET_OFFSET             9,0

    // Impostazione dell'Offset
    #define PCB244_A_TEST_GRID              10,10

  //////////////////////////////////////////////////////////////////////////////
  //                    DEFINIZIONE DI BITFIELD NEL TARGET
  //////////////////////////////////////////////////////////////////////////////
  #define PCB244_A_GRID_PRESENT      _REGID(RG244_A_SETTINGS),0,PCB244_A_Registers
  #define PCB244_A_CASSETTE          _REGID(RG244_A_SETTINGS),3,PCB244_A_Registers
  #define PCB244_A_CASSETTE_EXPOSED  _REGID(RG244_A_SETTINGS),4,PCB244_A_Registers


  #define PCB244_A_RESET             _REGID(RG244_A_SYSFLAGS0),0,PCB244_A_Registers
  #define PCB244_A_RESET_ON          _REGID(RG244_A_SYSFLAGS0),1,PCB244_A_Registers
  #define PCB244_A_FAULT             _REGID(RG244_A_SYSFLAGS0),2,PCB244_A_Registers
  #define PCB244_A_RX_COMPLETED      _REGID(RG244_A_SYSFLAGS0),3,PCB244_A_Registers
  #define PCB244_A_BUSY              _REGID(RG244_A_SYSFLAGS0),5,PCB244_A_Registers

  //////////////////////////////////////////////////////////////////////////////
  //                    CODICI PER ATTIVAZIONE REGISTRO EXE
  //////////////////////////////////////////////////////////////////////////////

  ext _DeviceContext_Str PCB244_A_CONTEST;
  ext bool pcb244_A_isPresent;

  // API //////////////////////////////////////////////////////////////////
  ext void pcb244_A_driver(uint32_t initial_data);  // Driver PCB244

  ext bool GetPcb244AFwRevision(void);

  ext bool pcb244_A_ResetFaults(void);
  ext bool pcb244_A_StartRxPre(void);
  ext bool pcb244_A_StartRxAec(void);
  ext bool pcb244_A_StartManual(bool grid_off_mode);
  ext bool pcb244_A_StartCalibTube(void);


  ext bool pcb244_A_uploadAECPulses(unsigned short pulses);
  ext bool pcb244_A_uploadManualPulses(unsigned short pulses);

  ext bool pcb244_A_GetPostRxRegisters(void);

  ext bool PCB244_A_sampleRad(void);        // Comando di campionamento RAD
  ext bool PCB244_A_GetPreRad(int attempt);
  ext bool PCB244_A_GetRad1(int attempt);
  ext bool PCB244_A_GetRad5(int attempt);
  ext bool PCB244_A_GetRad25(int attempt);

  ext void PCB244_A_SetRxStop(void);
  ext bool pcb244_A_ResetBoard(void);
  ext bool config_pcb244_A(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len);
  ext bool PCB244_A_setDetectorField(unsigned char val);

  ext bool pcb244AGetAccessorio(void);
  ext bool pcb244AGetCassette(void);

  ext bool PCB244_A_setOffset(unsigned short val);
  ext bool PCB244_A_getOffset(unsigned int* pRet);
  ext bool PCB244_A_zeroOffset(void);

  ext bool PCB244_A_waitRxCompletedFlag(void);
  ext bool PCB244_A_waitReady(void);
  ext bool PCB244_A_readRxStat(void);
  ext void setCassetteStat(bool val);

  ext void pcb244A_Stop2dGrid(void);
  ext void pcb244A_Start2dGrid(unsigned char nTest);

#endif
