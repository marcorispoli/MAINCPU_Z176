#ifndef _SEQRX_TOMO_H
#define _SEQRX_TOMO_H
/*

Aut: M. Rispoli
Data di Creazione: 4/11/2014
Data Ultima Modifica:
*/

 ext void tomo_rx_task(uint32_t initial_data);                // Driver PCB190
 ext void tomo_aec_rx_task(uint32_t taskRegisters);

#define _WAIT_EXP_WIN      10000
#define _WAIT_BUSY_HOME         5000
#endif