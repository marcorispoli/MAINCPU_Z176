#ifndef _EVENTI_H
#define _EVENTI_H


//////////////////////////////////////////////////////////////////////////////////
//      DEFINIZIONE DELLA STRUTTURA EVENTI DELL'APPLICAZIONE EV0
//////////////////////////////////////////////////////////////////////////////////


#define _EV0_PCB204_READY        0x00000001,Ev0      // PCB204 Non è Busy
#define _EV0_PCB249U2_COLLI      0x00000002,Ev0      // Richiesta comando Collimazione fronte retro
#define _EV0_PCB249U2_FILTRO     0x00000004,Ev0      // Richiesta comando filtro
#define _EV0_PCB249U1_COLLI      0x00000008,Ev0      // Richiede collimazione lame laterali (non durante raggi)
//#define _EV0_PCB204_CC           0x00000010,Ev0      // PCB204 in posizione CC
//#define _EV0_PCB204_TRG          0x00000020,Ev0      // PCB204 ARM in posizione
//#define _EV0_PCB204_HOME         0x00000040,Ev0      // PCB204 in Home
//#define _EV0_PCB204_FAULT        0x00000080,Ev0      // PCB204 in FAULT
#define _EV0_PCB215_COMPRESSION  0x00000100,Ev0      // PCB215 IN COMPRESSIONE
#define _EV0_PCB215_FREE         0x00000200,Ev0      // PCB215 NON IN COMPRESSIONE
#define _EV0_PCB215_IDLE         0x00000400,Ev0      // PCB215 IN IDLE
#define _EV0_PCB215_FAULT        0x00000800,Ev0      // PCB215 IN FAULT
#define _EV0_PCB215_CFG_UPD      0x00001000,Ev0      // CONFIGURAZIONE PCB215 AGGIORNATA
#define _EV0_PCB204_CFG_UPD      0x00002000,Ev0      // CONFIGURAZIONE PCB204 AGGIORNATA
#define _EV0_PCB190_CFG_UPD      0x00004000,Ev0      // CONFIGURAZIONE PCB190 AGGIORNATA    
#define _EV0_PCB244_CFG_UPD      0x00008000,Ev0      // CONFIGURAZIONE PCB244 AGGIORNATA       
#define _EV0_PCB249U1_CFG_UPD    0x00010000,Ev0      // CONFIGURAZIONE PCB249U1 AGGIORNATA       
#define _EV0_OUTPUT_CAMBIATI     0x00020000,Ev0      // OUTPUT CAMBIATI
#define _EV0_PCB249U2_CFG_UPD    0x00040000,Ev0      // CONFIGURAZIONE PCB249U2 AGGIORNATA       
#define _EV0_PCB249U2_FAULT      0x00080000,Ev0      // PCB249U2 in FAULT
#define _EV0_PCB249U2_BUSY       0x00100000,Ev0      // PCB249U2 in BUSY
#define _EV0_PCB249U2_READY      0x00200000,Ev0      // PCB249U2 not BUSY
#define _EV0_PCB244_DISCONNECTED 0x00400000,Ev0      
#define _EV0_SYSTEM_INPUTS       0x00800000,Ev0      // Notifica di aggiornamento Inputs
#define _EV0_COM_STARTED         0x01000000,Ev0      // Driver COM Partito
#define _EV0_VMUSIC_EVENT        0x02000000,Ev0
#define _EV0_27                  0x04000000,Ev0      
#define _EV0_28                  0x08000000,Ev0      
#define _EV0_29                  0x10000000,Ev0      
#define _EV0_30                  0x20000000,Ev0      
#define _EV0_31                  0x40000000,Ev0      
#define _EV0_SYSTEM_FAULT        0x80000000,Ev0      // FAULT DI SISTEMA


#ifdef _MAIN_C_
  LWEVENT_STRUCT Ev0;
#else
  extern LWEVENT_STRUCT Ev0;
#endif 

//////////////////////////////////////////////////////////////////////////////////
//      DEFINIZIONE DELLA STRUTTURA EVENTI DELL'APPLICAZIONE EV1
//////////////////////////////////////////////////////////////////////////////////

#define _EV1_PCB215_FREEZED      0x00000001,Ev1      // PCB215 è andata in modo disabilitato
#define _EV1_2                   0x00000002,Ev1      // PCB204 è andata in modo disabilitato
#define _EV1_PCB190_FREEZED      0x00000004,Ev1      // PCB190 è andata in modo disabilitato
#define _EV1_PCB244_FREEZED      0x00000008,Ev1      // PCB244 è andata in modo disabilitato
#define _EV1_PCB249U1_FREEZED    0x00000010,Ev1      // PCB249 è andata in modo disabilitato
#define _EV1_PCB249U2_FREEZED    0x00000020,Ev1      // PCB249 è andata in modo disabilitato
#define _EV1_DEVICES_RUN         0x00000040,Ev1      // Tutti i Devices devono andare in modo RUN
#define _EV1_PCB215_RUN          0x00000080,Ev1      // La PCB215 Deve andare / è andata in RUN
#define _EV1_9                   0x00000100,Ev1      // La PCB204 Deve andare / è andata in RUN
#define _EV1_PCB190_RUN          0x00000200,Ev1      // La PCB190 Deve andare / è andata in RUN
#define _EV1_PCB244_RUN          0x00000400,Ev1      // La PCB244 Deve andare / è andata in RUN
#define _EV1_PCB249U1_RUN        0x00000800,Ev1      // La PCB249 Deve andare / è andata in RUN
//#define _EV1_PCB249U2_RUN        0x00001000,Ev1      // La PCB249 Deve andare / è andata in RUN
#define _EV1_PCB249U1_BUSY       0x00002000,Ev1      // PCB249U1 è BUSY           
#define _EV1_PCB249U1_READY      0x00004000,Ev1      // PCB249U1 è READY           
#define _EV1_PCB249U1_FAULT      0x00008000,Ev1      // PCB249U1 è in FAULT  
#define _EV1_PCB249U1_TARGET     0x00010000,Ev1      // Lame di collimazione in Target
#define _EV1_BIOPSY_FREEZED      0x00020000,Ev1 
#define _EV1_BIOPSY_RUN          0x00040000,Ev1 
#define _EV1_BIOPSY_READY        0x00080000,Ev1 
#define _EV1_PCB249U2_CONNECTED  0x00100000,Ev1      // Driver PCB249U2 Partito 
#define _EV1_PCB249U1_CONNECTED  0x00200000,Ev1      // Driver PCB249U1 Partito 
#define _EV1_PCB244_CONNECTED    0x00400000,Ev1      // Driver PCB244 Partito 
#define _EV1_PCB190_CONNECTED    0x00800000,Ev1      // Driver PCB 190 Connesso 
#define _EV1_PCB215_CONNECTED    0x01000000,Ev1      // Driver PCB 215 Connesso
#define _EV1_26                  0x02000000,Ev1      // Driver PCB 204 Connesso
#define _EV1_UPDATE_REGISTERS    0x04000000,Ev1      // Abilitazione all'update di tutti i registri
#define _EV1_28                  0x08000000,Ev1      //
#define _EV1_29                  0x10000000,Ev1      //
#define _EV1_TRX_CONFIGURED      0x20000000,Ev1      //
#define _EV1_ARM_CONFIGURED      0x40000000,Ev1      //
#define _EV1_DEV_CONFIG_OK       0x80000000,Ev1      // Configurazione dispositivi completata


#ifdef _MAIN_C_
  LWEVENT_STRUCT Ev1;
#else
  extern LWEVENT_STRUCT Ev1;
#endif   


//////////////////////////////////////////////////////////////////////////////////
//      DEFINIZIONE DELLA STRUTTURA EVENTI SU IO
//////////////////////////////////////////////////////////////////////////////////

#define _EV2_XRAY_COMPLETED      0x00000001,Ev2      // Segnale XRAY Completed Attivato
#define _EV2_XRAY_ENA_ON         0x00000002,Ev2      // Segnale XRAY ENA ATTIVATO
#define _EV2_XRAY_ENA_OFF        0x00000004,Ev2      // Segnale XRAY ENA DISATTIVATO
#define _EV2_EXPWIN_ON           0x00000008,Ev2      // Segnale detector EXP WIN
#define _EV2_ROT_ENA_OFF         0x00000010,Ev2      // 
#define _EV2_ROT_ENA_ON          0x00000020,Ev2      // 
#define _EV2_XRAY_REQ_OFF        0x00000040,Ev2      // 
#define _EV2_XRAY_REQ_ON         0x00000080,Ev2      //
#define _EV2_PCB190_STARTUP_OK   0x00000100,Ev2      // 
#define _EV2_PCB244_STARTUP_OK   0x00000200,Ev2      //
#define _EV2_PCB215_STARTUP_OK   0x00000400,Ev2      // 
#define _EV2_PCB249U1_STARTUP_OK 0x00000800,Ev2      // 
#define _EV2_PCB249U2_STARTUP_OK 0x00001000,Ev2      // 
#define _EV2_14                  0x00002000,Ev2 
#define _EV2_15                  0x00004000,Ev2 
#define _EV2_16                  0x00008000,Ev2 
#define _EV2_17                  0x00010000,Ev2 
#define _EV2_18                  0x00020000,Ev2 
#define _EV2_19                  0x00040000,Ev2 
#define _EV2_20                  0x00080000,Ev2 
#define _EV2_21                  0x00100000,Ev2 
#define _EV2_22                  0x00200000,Ev2 
#define _EV2_23                  0x00400000,Ev2 
#define _EV2_24                  0x00800000,Ev2 
#define _EV2_25                  0x01000000,Ev2 
#define _EV2_26                  0x02000000,Ev2      // 
#define _EV2_27                  0x04000000,Ev2      //
#define _EV2_28                  0x08000000,Ev2      //
#define _EV2_29                  0x10000000,Ev2      //
#define _EV2_30                  0x20000000,Ev2      //
#define _EV2_WAIT_AE_CONTEXT     0x40000000,Ev2      // Attende segnale di cambio contesto
#define _EV2_WAIT_AEC            0x80000000,Ev2      // Attende dati da AEC


#ifdef _MAIN_C_
  LWEVENT_STRUCT Ev2;
#else
  extern LWEVENT_STRUCT Ev2;
#endif   
    
//////////////////////////////////////////////////////////////////////////////////
//      DEFINIZIONE DEGLI EVENTI DELLE SEQUENZE (AUTO RESET)
//////////////////////////////////////////////////////////////////////////////////
#define _SEQEV_NOEVENT                          0,SeqEventi          // Nessun evento
#define _SEQEV_RX_TOMO_START                    0x00000001,SeqEventi // Attiva la thread
#define _SEQEV_RX_TOMO_TERMINATED               0x00000002,SeqEventi // La sequenza raggi in modalità Tomo è terminata
#define _SEQEV_RX_STD_START                     0x00000004,SeqEventi // La sequenza raggi in modalità Standard è terminata
#define _SEQEV_RX_STD_TERMINATED                0x00000008,SeqEventi // La sequenza raggi in modalità Standard è terminata
#define _SEQEV_RX_STD_AEC_START                 0x00000010,SeqEventi // La sequenza raggi AEC in modalità Standard è terminata
#define _SEQEV_RX_STD_AEC_TERMINATED            0x00000020,SeqEventi // Sequenza Demo Terminata
#define _SEQEV_RX_TOMO_AEC_START                0x00000040,SeqEventi // Attiva la thread
#define _SEQEV_RX_TOMO_AEC_TERMINATED           0x00000080,SeqEventi // La sequenza raggi in modalità Tomo è terminata
#define _SEQEV_RX_AE_START                      0x00000100,SeqEventi // La sequenza raggi AE è iniziata
#define _SEQEV_RX_AE_TERMINATED                 0x00000200,SeqEventi // La sequenza raggi AE è terminata
#define _SEQEV_RX_ANALOG_START                  0x00000400,SeqEventi // La sequenza raggi Analogica è iniziata
#define _SEQEV_RX_ANALOG_TERMINATED             0x00000008,SeqEventi // La sequenza raggi Analogica è terminata

#ifdef _MAIN_C_
  LWEVENT_STRUCT SeqEventi;
#else
  extern LWEVENT_STRUCT SeqEventi;
#endif


#endif
