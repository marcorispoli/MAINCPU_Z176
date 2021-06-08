/*

Aut: M. Rispoli
Data di Creazione: 22/09/2014

Data Ultima Modifica:22/09/2014
*/
#ifndef _ACTUATORS_H
#define _ACTUATORS_H

#ifdef ext
#undef ext
#undef extrd
#endif
#ifdef _ACTUATORS_C
  #define ext 
  #define extrd 
#else
  #define ext extern
  #define extrd extern const
#endif

ext void actuatorsRxFromArm(uint8_t* data);
ext void actuatorsRxFromTrx(uint8_t* data);
ext void actuatorsRxFromLenze(uint8_t* data);
ext void actuatorsRxFromActuators(uint8_t* data);

// Funzioni per sequenze raggi
ext bool actuatorsTrxWaitReady(int tmo);                    // Attende posizionamento con timeout in 100ms / unit
ext bool actuatorsActivateTrxTriggerStart(void);
ext bool actuatorsMoveTomoTrxHome(unsigned char tomoMode);
ext bool actuatorsMoveTomoTrxEnd(unsigned char tomoMode, bool expwin_trigger);


ext void actuatorsTrxStop(int tmo); // Ferma il movimento del tubo in corso
ext bool actuatorsArmStop(void); // Ferma il movimento del braccio in corso
ext void actuatorsManualArmMove(unsigned char mode);
ext void actuatorsManualTrxMove(unsigned char mode);
ext bool actuatorsTrxActivateZeroSetting(void);

// Comandi di azionamento
ext int actuatorsArmMove(int angolo);
ext bool actuatorsTrxMove(int angolo); // Angoli espressi in decimi di grado

// Configuration functions
ext bool config_trx(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len);
ext bool config_arm(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len);
ext bool config_lenze(bool setmem, unsigned char blocco, unsigned char* buffer, unsigned char len);
ext void actuatorsGetStatus(void);

ext void actuatorsUpdateAngles(void);
ext void actuatorsManageEnables(void);
ext void actuatorsStartProcess(bool lenze, bool trx, bool arm);
#endif
