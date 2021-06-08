/*

Aut: M. Rispoli
Data di Creazione: 22/09/2014

Data Ultima Modifica:22/09/2014
*/
#ifndef _ANALOG_RX_SEQUENCES_H
#define _ANALOG_RX_SEQUENCES_H

#ifdef ext
#undef ext
#undef extrd
#endif
#ifdef _ANALOG_RX_SEQUENCES_C
  #define ext 
  #define extrd 
#else
  #define ext extern
  #define extrd extern const
#endif


// CODICI SEQUENZE
#define EXPOSIMETER_CALIBRATION_PULSE   1
#define TUBE_CALIBRATION_PROFILE        2

#define MANUAL_MODE_EXPOSURE            3
#define AEC_MODE_EXPOSURE               4


void tune_esposimeter_rx(_RxStdSeq_Str* Param); // Funzione di esposizione per calibrazione hw esposimetro

int AnalogManualModeExposure(void);
int AnalogAECModeExposure(void);

int AnalogPreCalibration(void);
int AnalogTubeCalibration(void);


void RxAnalogSeqError(int codice);
void fineSequenza(void);
void getRxSamplesData(float* kv, unsigned char* kvraw, float* imed, int* time);

int setXrayEna(void);
int clrXrayEna(void);


#endif
