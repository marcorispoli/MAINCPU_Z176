#ifndef _CANOPEN_H
#define _CANOPEN_H

/*************************************************************************************************
*                           PROTOCOLLO DI COMUNICAZIONE CAN OPEN
*
*   NMT         ID:           0x000
*   SYNC        ID:           0x80
*   Emergency   ID+node_id:   0x80
*   PDO TX      ID+node_id:   0x180, 0x280, 0x380, 0x480
*   PDO RX      ID+node_id:   0x200, 0x300, 0x400, 0x500
*   SDO TX      ID+node_id:   0x580
*   SDO RX      ID+node_id:   0x600
*   BOOT        ID+node_id:   0x700
*   Nodeguard   ID+node_id:   0x700
*
*
*                        (MASTER)
*       MASTER(2)        SLAVE(1)        TRX(3)             ARM(4)           LENZE(5)
*       tx(2) ---------> RX(2)
*       rx(2) <--------- tx(2)
*                        tx(3)------------>
*                        RX(3)<-----------
*
*                        tx(4)-------------------------------->
*                        RX(4)<-------------------------------
*
*                        tx(5)-------------------------------------------------->
*                        RX(5)<-------------------------------------------------
*
*
*
*************************************************************************************************/
//                  TABELLA DEFINIZIONE NODI DEL SISTEMA
#define CANOPEN_BAUDRATE    1000000
#define CANOPEN_SLAVE_NODE  1 // (CAN MASTER ROLE IN COMMUNICATION)
#define CANOPEN_MASTER_NODE 2
#define CANOPEN_TRX_NODE    3
#define CANOPEN_ARM_NODE    4
#define CANOPEN_LENZE_NODE  5

// Master/Slave services (first 4 bit reserved for the node id code)
#define CANOPEN_SRV_IO          0x10
#define CANOPEN_SRV_ACTUATORS   0x20

// Error bit for flexcan bus
#define CANERR_BUSOFF    0x0004
#define CANERR_INTERR    0x0002

/*____________________________________________________________________________________________________________
                                          SDO PROTOCOL
  ___________________________________________________________________________________________________________*/

// Definizione ID associati ai servizi SDO attivi sul nodo TRX
#define BOOTUP 0x700
#define TXSDO  0x600       // Target ID of the remote SDO node
#define RXSDO  0x580       // received SDO ID for read operations


// Type definition (for write commands)
typedef enum {
    CANOPEN_SDO_TX_1BYTE = 0x2F,    // Download expedited 1 byte
    CANOPEN_SDO_TX_2BYTE = 0x2B,    // Download expedited 2 byte
    CANOPEN_SDO_TX_3BYTE = 0x27,    // Download expedited 3 byte
    CANOPEN_SDO_TX_4BYTE = 0x23    // Download expedited 4 byte
}_canopen_enum_od_t;


// Write commands (Download)
#define CANOPEN_SDO_TX_1BYTE    0x2F    // Download expedited 1 byte
#define CANOPEN_SDO_TX_2BYTE    0x2B    // Download expedited 2 byte
#define CANOPEN_SDO_TX_3BYTE    0x27    // Download expedited 3 byte
#define CANOPEN_SDO_TX_4BYTE    0x23    // Download expedited 4 byte
#define CANOPEN_SDO_TXOK        0x60    // OK Acknowledge
#define CANOPEN_SDO_TXERROR     0x80    // NOK Acknowledge frame

// Read commands (Upload)
#define CANOPEN_SDO_RX_READ     0x40    // Read object dictionary code
#define CANOPEN_SDO_RX_1BYTE    0x4F    // Download expedited 1 byte
#define CANOPEN_SDO_RX_2BYTE    0x4B    // Download expedited 2 byte
#define CANOPEN_SDO_RX_3BYTE    0x47    // Download expedited 3 byte
#define CANOPEN_SDO_RX_4BYTE    0x43    // Download expedited 4 byte
#define CANOPEN_SDO_RXERROR     0x80    // NOK Acknowledge frame

// STRUTTURE DATI COMANDI DI NETWORK MANAGEMENT NMT
#define NMT_SET_OPERATION       0x01
#define NMT_SET_STOP            0x02
#define NMT_SET_PRE_OPERATION   0x80
#define NMT_RESET_NODE          0x81
#define NMT_RESET_COMMUNICATON  0x82


typedef struct DICTIONARY{
    uint16_t            index;
    uint8_t             sbidx;
    _canopen_enum_od_t  type;
    uint32_t            val;
}_canopen_ObjectDictionary_t;

MUTEX_STRUCT can_mutex;

#define CANOPEN_CONTEXT_DEC         uint32_t txnmb, flexcan_mb_code_status_tx_t* txmb, uint32_t node,uint32_t rxnmb, MQX_TICK_STRUCT* tick
#define CANOPEN_CONTEXT             txnmb, txmb, node, rxnmb, tick
#define _OD_TEMPLATE                uint16_t index, uint8_t sbx, uint8_t dim, uint32_t val
#define _OD_TEMPLATE_VAL            index, sbx, dim, val

// CiA 402 Power State Machine
typedef enum{
    CiA402_UndefinedStat  = 0,
    CiA402_NotReadyToSwitchOn,
    CiA402_SwitchOnDisabled,
    CiA402_ReadyToSwitchOn,
    CiA402_SwitchedOn,
    CiA402_OperationEnabled,
    CiA402_QuickStopActive,
    CiA402_FaultReactionActive,
    CiA402_Fault
}_CiA402_stat_t;

typedef enum{
    CANOPEN_NETWORK_UNDEFINED = 0,
    CANOPEN_NETWORK_INITIALIZATION,
    CANOPEN_NETWORK_PRE_OPERATIONAL,
    CANOPEN_NETWORK_OPERATIONAL,
    CANOPEN_NETWORK_STOPPED
}_canopen_network_stat_t;



#define _CiA402MASK(x,y) (x)
#define CiA402MASK(x) _CiA402MASK(x)
#define _CiA402VAL(x,y) (y)
#define CiA402VAL(x) _CiA402VAL(x)



//____________________________________________________________________________________________
//                      APPLICATION API

void canopenPrintSDOErrors(uint32_t errcode, unsigned short index, unsigned char subidx);
bool canopenWriteSDO(_canopen_ObjectDictionary_t* od, CANOPEN_CONTEXT_DEC);
bool canopenWriteDomainSDO(_canopen_ObjectDictionary_t* od, CANOPEN_CONTEXT_DEC, unsigned char* data, unsigned int size);


bool canopenReadSDO(_canopen_ObjectDictionary_t* od, CANOPEN_CONTEXT_DEC);
bool canopenResetNode(uint32_t delay, CANOPEN_CONTEXT_DEC);
bool canopenResetCommunication(uint32_t delay, CANOPEN_CONTEXT_DEC);
bool canopenSetNetworkOperation(uint32_t delay, CANOPEN_CONTEXT_DEC);
bool canopenSetNetworkPreOperation(uint32_t delay, CANOPEN_CONTEXT_DEC);

bool canopenUploadObjectDictionaryList(const _canopen_ObjectDictionary_t* pDictionary, int delay, CANOPEN_CONTEXT_DEC);



#endif
