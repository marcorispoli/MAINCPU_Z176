#ifndef _SLAVE_CAN
#define _SLAVE_CAN

#ifdef ext
#undef ext
#undef extrd
#endif
#ifdef _SLAVE_CAN_C
  #define ext 
  #define extrd 
#else
  #define ext extern
  #define extrd extern const
#endif

// Mailbox to transmit dato to slave
ext flexcan_mb_code_status_tx_t txmb_inputs_to_master;
#define MB_TX_INPUTS_TO_MASTER     1

// Reception mailboxes from Slave
ext flexcan_mb_code_status_rx_t rxmb_outputs_from_master;
#define MB_RX_OUTPUT_FROM_MASTER   2

// Mailbox to transmit dato to slave
ext flexcan_mb_code_status_tx_t txmb_actuator_to_master;
#define MB_TX_ACTUATOR_TO_MASTER    3

// Reception mailboxes from Slave
ext flexcan_mb_code_status_rx_t rxmb_actuator_from_master;
#define MB_RX_ACTUATOR_FROM_MASTER  4


// Mailbox to transmit dato to trx
ext flexcan_mb_code_status_tx_t txmb_to_trx;
#define MB_TX_TO_TRX    5

// Reception mailboxes from trx
ext flexcan_mb_code_status_rx_t rxmb_from_trx;
#define MB_RX_FROM_TRX  6

// Mailbox to transmit dato to arm
ext flexcan_mb_code_status_tx_t txmb_to_arm;
#define MB_TX_TO_ARM    7

// Reception mailboxes from arm
ext flexcan_mb_code_status_rx_t rxmb_from_arm;
#define MB_RX_FROM_ARM  8

// Mailbox to transmit dato to lenze
ext flexcan_mb_code_status_tx_t txmb_to_lenze;
#define MB_TX_TO_LENZE    9

// Reception mailboxes from lenze
ext flexcan_mb_code_status_rx_t rxmb_from_lenze;
#define MB_RX_FROM_LENZE  10

/////////////////////////////////////////////////////////////////////////
void Can_RxErrors_Task(uint32_t parameter);

void Inputs_To_Master_Task(uint32_t parameter);
void Outputs_From_Master_Task(uint32_t parameter);

void trx_tx_Task(uint32_t parameter);

void arm_tx_Task(uint32_t parameter);
void arm_rx_Task(uint32_t parameter);
void lenze_tx_Task(uint32_t parameter);
void lenze_rx_Task(uint32_t parameter);

// API ////////////////////////////////////////////////////////////////// 
bool canOpenSlaveInit(void); // Inizializza il CAN ed attiva le threads
bool canopenTrxReadSDO(unsigned short index, unsigned char subidx, uint32_t* result, MQX_TICK_STRUCT* tick);
bool sendActuatorFrameToMaster(unsigned char* buffer);


// MACRO //////////////////////////////////////////////////////////////////

#endif
