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


  ext _DeviceContext_Str BIOPSY_CONTEST;


  ext void  BIOPSY_driver(uint32_t initial_data);  // Driver BIOPSY

 // API //////////////////////////////////////////////////////////////////
  ext bool  biopsyMoveXYZ(unsigned short X, unsigned short Y, unsigned short Z);
  ext bool  biopsyMoveHome(void);
  ext bool  config_biopsy(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len);
  ext void  JoysticXYtoXY(void);

  ext bool  biopsyStepIncZ(void);
  ext bool  biopsyStepDecZ(void);
  ext bool  biopsyStepIncX(void);
  ext bool  biopsyStepDecX(void);
  ext bool  biopsyStepIncY(void);
  ext bool  biopsyStepDecY(void);


#endif

