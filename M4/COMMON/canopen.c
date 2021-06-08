#define _CANOPEN_C
#include "dbt_m4.h"

const flexcan_config_t flex_canopen_data = {
/* The number of Message Buffers needed        */ 16,
/* The maximum number of Message Buffers       */ 16,
/* The number of total RXIMR registers         */ kFlexCanTotalRxImrNumber,
/* The number of RX FIFO ID filters needed     */ kFlexCanRxFifoIDFilters_8,
/* RX fifo needed?                             */ false,
/* RX message buffer needed?                   */ true
};

void canopenPrintSDOErrors(uint32_t errcode, unsigned short index, unsigned char subidx){
    printf("SDO IDX:0x%x SBX:0x%x ERROR: ",index,subidx);
    switch(errcode){
        case 0x05030000: printf("Toggle bit not changed!");break;
        case 0x05040001: printf("Command specifier unknown!");break;
        case 0x06010000: printf("Unsupported access!");break;
        case 0x06010002: printf("Read Only Entry!");break;
        case 0x06020000: printf("Object not existing!");break;
        case 0x06040041: printf("Object cannot be PDO mapped!");break;
        case 0x06040042: printf("Mapped PDO exceeds PDO!");break;
        case 0x06070012: printf("Parameter lenght too long!");break;
        case 0x06070013: printf("Parameter lenght too short!");break;
        case 0x06090011: printf("Sub index not existing!");break;
        case 0x06090031: printf("Value too great!");break;
        case 0x08000022: printf("Data cannot be read or stored in this state!");break;
    default: printf("Unrecognized error code: 0x%x", errcode);
    }

    printf("\n");
}

uint32_t canopenGetValFromBuffer(int code, unsigned char* buffer){
    uint32_t valore=0;
    switch(code){
        case CANOPEN_SDO_RX_4BYTE: valore += (uint32_t) (buffer[3]) * 256 * 256 * 256;
        case CANOPEN_SDO_RX_3BYTE: valore += (uint32_t) (buffer[2]) * 256 * 256;
        case CANOPEN_SDO_RX_2BYTE: valore += (uint32_t) (buffer[1]) * 256 ;
        case CANOPEN_SDO_RX_1BYTE: valore += (uint32_t) (buffer[0]);
    }

    return valore;
}

bool canopenResetNode(uint32_t delay, CANOPEN_CONTEXT_DEC){
    unsigned char buffer[2];
    flexcan_mb_t rxbuffer;
    uint32_t     result;

    buffer[0] = NMT_RESET_NODE;
    buffer[1] = node;

    _mutex_lock(&can_mutex);
    result = flexcan_txrx_node(txnmb, txmb, 0,2,buffer,rxnmb,&rxbuffer, tick);
    _mutex_unlock(&can_mutex);

    if((result==kFlexCan_OK)&&(rxbuffer.msg_id==node+BOOTUP)){
        printf("BOOPTUP OF NODE 0x%x COMPLETED\n",node);
        _time_delay(delay);
        return true;
    }

    _time_delay(delay);
    return false;
}

bool canopenResetCommunication(uint32_t delay, CANOPEN_CONTEXT_DEC){
    unsigned char buffer[2];
    flexcan_mb_t rxbuffer;
    uint32_t     result;

    buffer[0] = NMT_RESET_COMMUNICATON;
    buffer[1] = node;

    _mutex_lock(&can_mutex);
    result = flexcan_txrx_node(txnmb, txmb, 0,2,buffer,rxnmb,&rxbuffer, tick);
    _mutex_unlock(&can_mutex);

    _time_delay(delay);
    if(result==kFlexCan_OK) return true;
    return false;
}

bool canopenSetNetworkOperation(uint32_t delay, CANOPEN_CONTEXT_DEC){
    unsigned char buffer[2];
    flexcan_mb_t rxbuffer;
    uint32_t     result;

    buffer[0] = NMT_SET_OPERATION;
    buffer[1] = node;

    _mutex_lock(&can_mutex);
    result = flexcan_txrx_node(txnmb, txmb, 0,2,buffer,rxnmb,&rxbuffer, tick);
    _mutex_unlock(&can_mutex);

    _time_delay(delay);
    if(result==kFlexCan_OK) return true;
    return false;
}

bool canopenSetNetworkPreOperation(uint32_t delay, CANOPEN_CONTEXT_DEC){
    unsigned char buffer[2];
    flexcan_mb_t rxbuffer;
    uint32_t     result;

    buffer[0] = NMT_SET_PRE_OPERATION;
    buffer[1] = node;

    _mutex_lock(&can_mutex);
    result = flexcan_txrx_node(txnmb, txmb, 0,2,buffer,rxnmb,&rxbuffer, tick);
    _mutex_unlock(&can_mutex);

     _time_delay(delay);
    if(result==kFlexCan_OK) return true;
    return false;
}

bool canopenReadSDO(_canopen_ObjectDictionary_t* od, CANOPEN_CONTEXT_DEC){

    unsigned char buffer[8];
    flexcan_mb_t rxbuffer;
    uint32_t     result;

    buffer[0] = (unsigned char) CANOPEN_SDO_RX_READ;
    buffer[1] = (unsigned char) (od->index&0xFF);
    buffer[2] = (unsigned char) (od->index>>8);
    buffer[3] = (unsigned char) (od->sbidx);

    _mutex_lock(&can_mutex);
    result = flexcan_txrx_node(txnmb, txmb, TXSDO+node,8,buffer,rxnmb,&rxbuffer, tick);
    _mutex_unlock(&can_mutex);
    if(result!=kFlexCan_OK){
        printf("NODE:%d Idx:0x%x Sbx:0x%x - flexcan error in read sdo\n", node, od->index, od->sbidx);
        return false;
    }



    // Analizza il risultato
    if(rxbuffer.msg_id!=node+RXSDO){
        printf("Idx:0x%x Sbx:0x%x - wrong ID returned code. Expected: 0x%x, Received 0x%x\n", od->index, od->sbidx,node+RXSDO,rxbuffer.msg_id);
        for(int i=0; i<8; i++) printf(" %x",rxbuffer.data[i]);
        printf("\n");

        return false;
    }
    if((rxbuffer.data[1]+rxbuffer.data[2]*256)!=od->index){
        printf("Idx:0x%x Sbx:0x%x - wrong Index returned code. Expected: 0x%x, Received 0x%x\n", od->index, od->sbidx,od->index,(rxbuffer.data[1]+rxbuffer.data[2]*256));
        return false;
    }
    if(rxbuffer.data[3]!=od->sbidx){
        printf("Idx:0x%x Sbx:0x%x - wrong Sub-Index returned code. Expected: 0x%x, Received 0x%x\n", od->index, od->sbidx,od->sbidx,rxbuffer.data[3]);
        return false;
    }

    if(rxbuffer.data[0]==CANOPEN_SDO_RXERROR){
        canopenPrintSDOErrors(canopenGetValFromBuffer(CANOPEN_SDO_RX_4BYTE, &rxbuffer.data[4]),od->index, od->sbidx);
        return false;
    }

    od->val =canopenGetValFromBuffer(rxbuffer.data[0], &rxbuffer.data[4]);
    return true;

}



bool canopenWriteSDO(_canopen_ObjectDictionary_t* od, CANOPEN_CONTEXT_DEC){

    unsigned char buffer[8];
    flexcan_mb_t rxbuffer;
    uint32_t     result;

    buffer[0] = (unsigned char) od->type;
    buffer[1] = (unsigned char) (od->index&0xFF);
    buffer[2] = (unsigned char) (od->index>>8);
    buffer[3] = (unsigned char) (od->sbidx);

    uint32_t val = od->val;
    buffer[4] = val & 0xFF; val= (val>>8);
    buffer[5] = val & 0xFF; val= (val>>8);
    buffer[6] = val & 0xFF; val= (val>>8);
    buffer[7] = val & 0xFF;

    /*
    printf("BUFFER TO TRX : ");
    for(int i=0; i<8; i++) printf(" %x",buffer[i]);
    printf("\n");
    */
    _mutex_lock(&can_mutex);
    result = flexcan_txrx_node(txnmb, txmb, TXSDO+node,8,buffer,rxnmb,&rxbuffer, tick);
    _mutex_unlock(&can_mutex);
    if(result!=kFlexCan_OK){
        printf("Node:%d Idx:0x%x Sbx:0x%x - flexcan error in trx read sdo\n", node, od->index, od->sbidx);
        return false;
    }

    /*
    printf("BUFFER FROM TRX :ID=%x, ",rxbuffer.msg_id);
    for(int i=0; i<8; i++) printf(" %x",rxbuffer.data[i]);
    printf("\n");
    */

    // Analizza il risultato
    if(rxbuffer.msg_id!=node+RXSDO){
        printf("Idx:0x%x Sbx:0x%x - wrong ID returned code. Expected: 0x%x, Received 0x%x\n", od->index, od->sbidx,CANOPEN_TRX_NODE+RXSDO,rxbuffer.msg_id);
        return false;
    }
    if((rxbuffer.data[1]+rxbuffer.data[2]*256)!=od->index){
        printf("Idx:0x%x Sbx:0x%x - wrong Index returned code. Expected: 0x%x, Received 0x%x\n", od->index, od->sbidx,od->index,(rxbuffer.data[1]+rxbuffer.data[2]*256));
        return false;
    }
    if(rxbuffer.data[3]!=od->sbidx){
        printf("Idx:0x%x Sbx:0x%x - wrong Sub-Index returned code. Expected: 0x%x, Received 0x%x\n", od->index, od->sbidx,od->sbidx,rxbuffer.data[3]);
        return false;
    }

    if(rxbuffer.data[0]==CANOPEN_SDO_TXERROR){
        canopenPrintSDOErrors(canopenGetValFromBuffer(CANOPEN_SDO_RX_4BYTE, &rxbuffer.data[4]),od->index, od->sbidx);
        return false;
    }

    return true;

}

// Funzione per trasferire un blocco di dati di tipo DOMAIN ad un dato Index/Subindx
bool canopenWriteDomainSDO(_canopen_ObjectDictionary_t* od, CANOPEN_CONTEXT_DEC, unsigned char* data, unsigned int size){
    unsigned char buffer[8];
    flexcan_mb_t rxbuffer;
    uint32_t     result;

    // Init trasferimento
    buffer[0] = 0x20; // Normal transfer, no size indication
    buffer[1] = (unsigned char) (od->index&0xFF);
    buffer[2] = (unsigned char) (od->index>>8);
    buffer[3] = (unsigned char) (od->sbidx);

    buffer[4] = 0;
    buffer[5] = 0;
    buffer[6] = 0;
    buffer[7] = 0;

    printf("INIT DOWNLOAD BLOCK\n");
    _mutex_lock(&can_mutex);
    result = flexcan_txrx_node(txnmb, txmb, TXSDO+node,8,buffer,rxnmb,&rxbuffer, tick);
    _mutex_unlock(&can_mutex);
    if(result!=kFlexCan_OK){
        printf("Node:%d Idx:0x%x Sbx:0x%x - flexcan error in trx read sdo\n", node, od->index, od->sbidx);
        return false;
    }
    // Analizza il risultato
    if(rxbuffer.msg_id!=node+RXSDO){
        printf("Idx:0x%x Sbx:0x%x - wrong ID returned code. Expected: 0x%x, Received 0x%x\n", od->index, od->sbidx,CANOPEN_TRX_NODE+RXSDO,rxbuffer.msg_id);
        return false;
    }
    if((rxbuffer.data[1]+rxbuffer.data[2]*256)!=od->index){
        printf("Idx:0x%x Sbx:0x%x - wrong Index returned code. Expected: 0x%x, Received 0x%x\n", od->index, od->sbidx,od->index,(rxbuffer.data[1]+rxbuffer.data[2]*256));
        return false;
    }
    if(rxbuffer.data[3]!=od->sbidx){
        printf("Idx:0x%x Sbx:0x%x - wrong Sub-Index returned code. Expected: 0x%x, Received 0x%x\n", od->index, od->sbidx,od->sbidx,rxbuffer.data[3]);
        return false;
    }
    if(rxbuffer.data[0]!=0x60){
        printf("Wrong normal transfer Block initiator answer \n");
        return false;
    }

    // Sequenza iniziata correttamente
    unsigned int index = 0;
    int n;
    bool t=false;
    unsigned char answer;
    while(index<size){
        // BUFFER 0: CTRL
        buffer[0] = 0;
        if(t) buffer[0]|=0x10; // Toggle bit
        answer = buffer[0]|0x20; // risposta attesa

        if(index+7 >= size) buffer[0]|=0x1; // No more byte to be downloaded (flag c=1)
        n = 7 - (size-index);
        if(n<0) n=0;
        buffer[0]|=(n*2);

        // Data block
        if(index<size) buffer[1] = data[index];
        if(index+1<size) buffer[2] = data[index+1];
        if(index+2<size) buffer[3] = data[index+2];
        if(index+3<size) buffer[4] = data[index+3];
        if(index+4<size) buffer[5] = data[index+4];
        if(index+5<size) buffer[6] = data[index+5];
        if(index+6<size) buffer[7] = data[index+6];

        printf("INDEX BLOCCO:%d, B[0]=%x, B[1]=%x,B[2]=%x,B[3]=%x,B[4]=%x,B[5]=%x,B[6]=%x,B[7]=%x\n",index,buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7]);


        _mutex_lock(&can_mutex);
        result = flexcan_txrx_node(txnmb, txmb, TXSDO+node,8,buffer,rxnmb,&rxbuffer, tick);
        _mutex_unlock(&can_mutex);
        if(result!=kFlexCan_OK){
            printf("Node:%d Idx:0x%x Sbx:0x%x - flexcan error in trx read sdo\n", node, od->index, od->sbidx);
            return false;
        }
        // Analizza il risultato
        if(rxbuffer.msg_id!=node+RXSDO){
            printf("Idx:0x%x Sbx:0x%x - wrong ID returned code. Expected: 0x%x, Received 0x%x\n", od->index, od->sbidx,CANOPEN_TRX_NODE+RXSDO,rxbuffer.msg_id);
            return false;
        }

        if(rxbuffer.data[0]!=answer){
            printf("Wrong normal transfer Block initiator answer \n");
            return false;
        }

        index+=7;
        t=!t;
    }



    return true;
}


/* EOF */
