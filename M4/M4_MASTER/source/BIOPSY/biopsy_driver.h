/*

Aut: M. Rispoli
Data di Creazione: 19/06/2014
*/
#ifndef _BIOPSY_DRIVER_H
#define _BIOPSY_DRIVER_H

#ifdef ext
#undef ext
#undef extrd
#endif
#ifdef _BIOPSY_DRIVER_C
  #define ext 
  #define extrd 
#else
  #define ext extern
  #define extrd extern const
#endif


#define  _ADAPTER_NEEDLE_LEVEL_H    1100
#define  _ADAPTER_NEEDLE_LEVEL_L    950

#define  _ADAPTER_A_LEVEL_H         550
#define  _ADAPTER_A_LEVEL_L         750

#define  _ADAPTER_SHORT_LEVEL_H     100
#define  _ADAPTER_SHORT_LEVEL_L     0

#define  _ADAPTER_OPEN_LEVEL_H      2100
#define  _ADAPTER_OPEN_LEVEL_L      1950

// Funzioni di servizio
ext bool BiopsyDriverGetStat(unsigned char* statL, unsigned char* statH, bool reset);
ext bool BiopsyDriverGetRevision(unsigned char* val);
ext bool BiopsyDriverGetChecksum(unsigned char* chkl, unsigned char* chkh);
ext bool BiopsyDriverReset(void);

// Joystic
ext bool BiopsyDriverGetJoyX(unsigned short* val);
ext bool BiopsyDriverGetJoyY(unsigned short* val);

// Needle
ext bool BiopsyDriverGetNeedle(unsigned short* val);

// Position
ext bool BiopsyDriverGetX(unsigned short* val);
ext bool BiopsyDriverGetY(unsigned short* val);
ext bool BiopsyDriverGetZ(unsigned short* val);

// Targets
ext bool BiopsyDriverGetTGX(unsigned short* val);
ext bool BiopsyDriverGetTGY(unsigned short* val);
ext bool BiopsyDriverGetTGZ(unsigned short* val);
ext bool BiopsyDriverSetTGX(unsigned short val);
ext bool BiopsyDriverSetTGY(unsigned short val);
ext bool BiopsyDriverSetTGZ(unsigned short val);

// Activations
ext bool BiopsyDriverMoveXYZ(unsigned char* statusL, unsigned char* statusH);
ext bool BiopsyDriverMoveHOME(unsigned char* statusL, unsigned char* statusH);
ext bool BiopsyDriverMoveDecZ(unsigned char* statusL, unsigned char* statusH);
ext bool BiopsyDriverMoveIncZ(unsigned char* statusL, unsigned char* statusH);
ext bool BiopsyDriverMoveDecX(unsigned char* statusL, unsigned char* statusH);
ext bool BiopsyDriverMoveIncX(unsigned char* statusL, unsigned char* statusH);
ext bool BiopsyDriverMoveDecY(unsigned char* statusL, unsigned char* statusH);
ext bool BiopsyDriverMoveIncY(unsigned char* statusL, unsigned char* statusH);

// Registers
ext bool BiopsyDriverSetZlim(unsigned short val, unsigned short* zlim);
ext bool BiopsyDriverSetStepVal(unsigned char val, unsigned char* stepval);


#endif
