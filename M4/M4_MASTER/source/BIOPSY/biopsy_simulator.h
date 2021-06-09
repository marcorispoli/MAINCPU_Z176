/*

Aut: M. Rispoli
Data di Creazione: 19/06/2014
*/
#ifndef _BIOPSY_SIMULATOR_H
#define _BIOPSY_SIMULATOR_H

#ifdef ext
#undef ext
#undef extrd
#endif
#ifdef _BIOPSY_SIMULATOR_C
  #define ext 
  #define extrd 
#else
  #define ext extern
  #define extrd extern const
#endif

    #ifdef __BIOPSY_SIMULATOR

        // Thread di simulazione
        ext void BIOPSY_simdriver(uint32_t taskRegisters);
        ext void sim_serialCommand(unsigned char* rxdata, unsigned char* txdata);


        // Interfaccia con la GUI per la simulazione della Biopsia
        ext void SimConnessione(bool stat);
        ext void SimSetPush(bool stat);
        ext void SimSetAdapter(int id);
        ext void SimSetConsolePush(unsigned char push);
        ext void SimSetJXY(unsigned short X, unsigned short Y);

    #endif

#endif
