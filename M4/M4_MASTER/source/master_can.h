/*

Aut: M. Rispoli
Data: 06/10/2014
Data Modifica:
*/
#ifndef _MASTER_CAN
#define _MASTER_CAN

#ifdef ext
#undef ext
#undef extrd
#endif
#ifdef _MASTER_CAN_C
  #define ext 
  #define extrd 
#else
  #define ext extern
  #define extrd extern const
#endif

// ATTENZIONE QUALCHE COGLIONE HA SCRITTO MALE IL DRIVER
// QUINDI LA MAIL BOX ZERO NON SI PUO' USARE

// Mailbox  IO  to slave
ext flexcan_mb_code_status_tx_t txmb_to_io_slave;
#define MB_TX_TO_IO_SLAVE    1

// Reception mailboxes from IO Slave
ext flexcan_mb_code_status_rx_t rxmb_from_io_slave;
#define MB_RX_FROM_IO_SLAVE  2

// Mailbox  ACTUATORS  to slave
ext flexcan_mb_code_status_tx_t txmb_to_actuators_slave;
#define MB_TX_TO_ACTUATORS_SLAVE    3

// Reception mailboxes from ACTUATORS Slave
ext flexcan_mb_code_status_rx_t rxmb_from_actuators_slave;
#define MB_RX_FROM_ACTUATORS_SLAVE  4


/////////////////////////////////////////////////////////////////////////
  
void Can_Tx_Task(uint32_t parameter);
void Can_Rx_Task(uint32_t parameter);
void Can_RxErrors_Task(uint32_t parameter);
void Can_RxActuators_Task(uint32_t parameter);

// API ////////////////////////////////////////////////////////////////// 
bool canOpenMasterInit(void); // Inizializza il CAN ed attiva le threads
void CanSendToActuatorsSlave(unsigned char* buf);

// MACRO ////////////////////////////////////////////////////////////////// 

#endif
