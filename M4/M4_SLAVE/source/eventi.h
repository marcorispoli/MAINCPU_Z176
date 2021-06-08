#ifndef _EVENTI_H
#define _EVENTI_H

//////////////////////////////////////////////////////////////////////////////////
//      DEFINIZIONE DELLA STRUTTURA EVENTI DELL'APPLICAZIONE
//////////////////////////////////////////////////////////////////////////////////
#define _EV0_INPUT_CAMBIATI      0x00000001,Ev0      // EVENTO INPUTS CAMBIATI 
#define _EV0_OUTPUT_CAMBIATI     0x00000002,Ev0      // EVENTO CAMBIO OUTPUT
#define _EV0_TRX_IDLE            0x00000004,Ev0      //
#define _EV0_ARM_IDLE            0x00000008,Ev0      //
#define _EV0_5                   0x00000010,Ev0      //
#define _EV0_6                   0x00000020,Ev0      //
#define _EV0_7                   0x00000040,Ev0      //
#define _EV0_8                   0x00000080,Ev0      //
#define _EV0_TRX_CONNECTED       0x00000100,Ev0      //
#define _EV0_ARM_CONNECTED       0x00000200,Ev0      //
#define _EV0_LENZE_CONNECTED     0x00000400,Ev0      //
#define _EV0_TRX_EVENT           0x00000800,Ev0      // Evento ricevuto da TRX
#define _EV0_ARM_EVENT           0x00001000,Ev0      // Evento ricevuto da ARM
#define _EV0_LENZE_EVENT         0x00002000,Ev0      // Evento ricevuto da LENZE
#define _EV0_POWER_EVENT         0x00004000,Ev0
#define _EV0_16                  0x00008000,Ev0 
#define _EV0_17                  0x00010000,Ev0 
#define _EV0_18                  0x00020000,Ev0 
#define _EV0_19                  0x00040000,Ev0 
#define _EV0_20                  0x00080000,Ev0 
#define _EV0_21                  0x00100000,Ev0 
#define _EV0_22                  0x00200000,Ev0 
#define _EV0_23                  0x00400000,Ev0 
#define _EV0_24                  0x00800000,Ev0 
#define _EV0_25                  0x01000000,Ev0 
#define _EV0_26                  0x02000000,Ev0 
#define _EV0_27                  0x04000000,Ev0   
#define _EV0_28                  0x08000000,Ev0      
#define _EV0_COM_STARTED         0x10000000,Ev0      // Driver 422 Partito 
#define _EV1_PCB215_CONNECTED    0x20000000,Ev0      // Driver PCB 215 Connesso
#define _EV1_31                  0x40000000,Ev0      // Driver PCB 204 Connesso
#define _EV0_SYSTEM_FAULT        0x80000000,Ev0      // FAULT DI SISTEMA


// Gestione degli Eventi
#ifdef _MAIN_C_
  LWEVENT_STRUCT Ev0;
#else
  extern LWEVENT_STRUCT Ev0;
#endif  

#endif
