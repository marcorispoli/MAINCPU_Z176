/*

Aut: M. Rispoli
Data di Creazione: 30/09/2014

Data Ultima Modifica:22/09/2014
*/
#ifndef _MCCCLASS_H
#define _MCCCLASS_H

#ifdef ext
#undef ext
#undef extrd
#endif
#ifdef _MCCCLASS_C
  #define ext 
  #define extrd 
#else
  #define ext extern
  #define extrd extern const
#endif

#define _MCC_DIM 150
#define MAX_MCC_QUEUE 5



typedef struct
{
  unsigned char cmd; // Comando da eseguire
  unsigned char id;  // Id di riconoscimento inviato da chiamante e restituito nelle notifiche
  unsigned char len; // Lunghezza del buffer
  unsigned char checksum; // Checksum del buffer
  unsigned char buffer[_MCC_DIM];
}_MccFrame_Str;


ext bool mccSendBuffer(MCC_ENDPOINT* ep, unsigned char cmd, unsigned char id, unsigned char* buffer, unsigned char len); // Crea il frame da spedire
ext bool mccSendBufferFlags(MCC_ENDPOINT* ep, unsigned char cmd, unsigned char id, unsigned char* buffer, unsigned char len,unsigned char data1, unsigned char data2);

ext bool mccSendFrame(MCC_ENDPOINT* ep,_MccFrame_Str* mccframe); // Crea il frame da spedire
ext bool mccIsFrameCorrect(_MccFrame_Str* mccframe); // Verifica se il frame è corretto
ext bool mccOpenPort(MCC_ENDPOINT *ep);  // Apertura canale di ricezione
ext bool mccRxFrame(MCC_ENDPOINT* ep, _MccFrame_Str* mccframe); // Routine di ricezione 
ext unsigned char mccSetChecksum(_MccFrame_Str* mcc_cmd); // Imposta il checksum di un pacchetto 
ext void mccNotify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen);
ext bool mccPCB215Notify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen);
ext bool mccGuiNotify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen);
ext bool mccConfigNotify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen);
ext bool mccPCB190Notify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen);
ext bool mccPCB215Notify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen);
ext bool mccActuatorNotify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen);
ext bool mccPCB244ANotify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen);
ext bool mccPCB249U1Notify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen);
ext bool mccBiopsyNotify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen);
ext bool mccServiceNotify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen);
ext bool mccLoaderNotify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen);
ext bool mccInputsNotify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen);

#endif
