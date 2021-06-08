#define _SLAVE_CAN_C
#include "dbt_m4.h"


static void  MasterDebugData(unsigned char* buf);

extern const flexcan_config_t flex_canopen_data;
#define CAN_ERROR_ENABLE    false


/***********************************************************************
 *  Inizializzazione comunicazione su CAN
 *  - node_id == [1:127], 1 -> Master
 ***********************************************************************/
 bool canOpenSlaveInit(void){
     uint32_t result;
     uint32_t u32;
    _task_id  created_task;

    // Inizializzazione delle mutex
    if (_mutex_init(&can_mutex, NULL) != MQX_OK)
    {
      printf("Initializing print mutex failed.\n");
      _mqx_exit(-1);
    }

     /* Impostazioni generali del driver
      * This function initializes
      * @param   instance                   The FlexCAN instance number.
      * @param   data                       The FlexCAN platform data.
      * @param   enable_err_interrupts      1 if enable it, 0 if not.
      * result:                             0 if successful; non-zero failed*/
     result = flexcan_init(BSP_CAN_DEVICE, &flex_canopen_data, CAN_ERROR_ENABLE);
     if (result){
         printf("\nMASTER CANOPEN INITIALIZATION ERROR: 0x%lx", result);
         return false;
     }

     // Impostazione del bit rate a 1 MB/s
     result = flexcan_set_bitrate(BSP_CAN_DEVICE, CANOPEN_BAUDRATE);
     if (result){
         printf("\nCANOPEN SET BITRATE ERROR: 0x%lx", result);
         return false;
     }

     result = flexcan_get_bitrate(BSP_CAN_DEVICE, &u32);
     if (result){
         printf("\r\nCANOPEN GET BITRATE ERROR: 0x%lx", result);
         return false;
     }else   printf("\r\nCANOPEN SET BITRATE TO: %d Hz", u32);

     // Set the Global Mask to filter the NODE-ID field
     result = flexcan_set_mask_type(BSP_CAN_DEVICE, kFlexCanRxMask_Global);
     if (result)
         printf("\nFLEXCAN set mask type. result: 0x%lx", result);
     result = flexcan_set_rx_mb_global_mask(BSP_CAN_DEVICE, kFlexCanMbId_Std, 0xFFF);
     if (result)
         printf("\nFLEXCAN set rx MB global mask. result: 0x%lx", result);


     //________________________________________________________________________________________________________
     //                              INPUTS SLAVE TO MASTER
     txmb_inputs_to_master.code = kFlexCanTX_Data;             // Messaggio tipo dati
     txmb_inputs_to_master.msg_id_type = kFlexCanMbId_Std;     // Standard ID format (11 bit)
     txmb_inputs_to_master.data_length = 8;                    // Always 8 data bit
     txmb_inputs_to_master.substitute_remote = 0;
     txmb_inputs_to_master.remote_transmission = 0;
     txmb_inputs_to_master.local_priority_enable = 0;
     txmb_inputs_to_master.local_priority_val = 0;

     result = flexcan_tx_mb_config(BSP_CAN_DEVICE, &flex_canopen_data, MB_TX_INPUTS_TO_MASTER, &txmb_inputs_to_master, CANOPEN_MASTER_NODE+CANOPEN_SRV_IO);
     if (result){
         printf("\nTransmit MB config error. Error Code: 0x%lx", result);
         return false;
     }

     // OUTPUTS FROM MASTER
     rxmb_outputs_from_master.code = kFlexCanRX_Ranswer;
     rxmb_outputs_from_master.msg_id_type = kFlexCanMbId_Std;
     rxmb_outputs_from_master.data_length = 1;
     rxmb_outputs_from_master.substitute_remote = 0;
     rxmb_outputs_from_master.remote_transmission = 0;
     rxmb_outputs_from_master.local_priority_enable = 0;
     rxmb_outputs_from_master.local_priority_val = 0;
     result = flexcan_rx_mb_config(BSP_CAN_DEVICE, &flex_canopen_data, MB_RX_OUTPUT_FROM_MASTER, &rxmb_outputs_from_master, CANOPEN_MASTER_NODE+CANOPEN_SRV_IO);
     if (result){
         printf("\nReceive IO MB config error. Error Code: 0x%lx", result);
         return false;
     }

     //________________________________________________________________________________________________________
     //                              ACTUATOR COMMAND FROM/TO MASTER
     txmb_actuator_to_master.code = kFlexCanTX_Data;             // Messaggio tipo dati
     txmb_actuator_to_master.msg_id_type = kFlexCanMbId_Std;     // Standard ID format (11 bit)
     txmb_actuator_to_master.data_length = 8;                    // Always 8 data bit
     txmb_actuator_to_master.substitute_remote = 0;
     txmb_actuator_to_master.remote_transmission = 0;
     txmb_actuator_to_master.local_priority_enable = 0;
     txmb_actuator_to_master.local_priority_val = 0;

     result = flexcan_tx_mb_config(BSP_CAN_DEVICE, &flex_canopen_data, MB_TX_ACTUATOR_TO_MASTER, &txmb_actuator_to_master, CANOPEN_MASTER_NODE+CANOPEN_SRV_ACTUATORS);
     if (result){
         printf("\nTransmit MB config error. Error Code: 0x%lx", result);
         return false;
     }


     rxmb_actuator_from_master.code = kFlexCanRX_Ranswer;
     rxmb_actuator_from_master.msg_id_type = kFlexCanMbId_Std;
     rxmb_actuator_from_master.data_length = 1;
     rxmb_actuator_from_master.substitute_remote = 0;
     rxmb_actuator_from_master.remote_transmission = 0;
     rxmb_actuator_from_master.local_priority_enable = 0;
     rxmb_actuator_from_master.local_priority_val = 0;
     result = flexcan_rx_mb_config(BSP_CAN_DEVICE, &flex_canopen_data, MB_RX_ACTUATOR_FROM_MASTER, &rxmb_actuator_from_master, CANOPEN_MASTER_NODE+CANOPEN_SRV_ACTUATORS);
     if (result){
         printf("\nReceive IO MB config error. Error Code: 0x%lx", result);
         return false;
     }


     //________________________________________________________________________________________________________
     //                                 SDO - TRX
     txmb_to_trx.code = kFlexCanTX_Data;             // Messaggio tipo dati
     txmb_to_trx.msg_id_type = kFlexCanMbId_Std;     // Standard ID format (11 bit)
     txmb_to_trx.data_length = 8;                    // Always 8 data bit
     txmb_to_trx.substitute_remote = 0;
     txmb_to_trx.remote_transmission = 0;
     txmb_to_trx.local_priority_enable = 0;
     txmb_to_trx.local_priority_val = 0;

     result = flexcan_tx_mb_config(BSP_CAN_DEVICE, &flex_canopen_data, MB_TX_TO_TRX, &txmb_to_trx, CANOPEN_TRX_NODE+TXSDO);
     if (result){
         printf("\nTransmit MB config error. Error Code: 0x%lx", result);
         return false;
     }

     rxmb_from_trx.code = kFlexCanRX_Ranswer;
     rxmb_from_trx.msg_id_type = kFlexCanMbId_Std;
     rxmb_from_trx.data_length = 1;
     rxmb_from_trx.substitute_remote = 0;
     rxmb_from_trx.remote_transmission = 0;
     rxmb_from_trx.local_priority_enable = 0;
     rxmb_from_trx.local_priority_val = 0;
     result = flexcan_rx_mb_config(BSP_CAN_DEVICE, &flex_canopen_data, MB_RX_FROM_TRX, &rxmb_from_trx, CANOPEN_TRX_NODE+RXSDO);
     if (result){
         printf("\nReceive IO MB config error. Error Code: 0x%lx", result);
         return false;
     }


     //________________________________________________________________________________________________________
     //                                 SLAVE - ARM
     txmb_to_arm.code = kFlexCanTX_Data;             // Messaggio tipo dati
     txmb_to_arm.msg_id_type = kFlexCanMbId_Std;     // Standard ID format (11 bit)
     txmb_to_arm.data_length = 8;                    // Always 8 data bit
     txmb_to_arm.substitute_remote = 0;
     txmb_to_arm.remote_transmission = 0;
     txmb_to_arm.local_priority_enable = 0;
     txmb_to_arm.local_priority_val = 0;

     result = flexcan_tx_mb_config(BSP_CAN_DEVICE, &flex_canopen_data, MB_TX_TO_ARM, &txmb_to_arm, CANOPEN_ARM_NODE+TXSDO);
     if (result){
         printf("\nTransmit MB config error. Error Code: 0x%lx", result);
         return false;
     }

     rxmb_from_arm.code = kFlexCanRX_Ranswer;
     rxmb_from_arm.msg_id_type = kFlexCanMbId_Std;
     rxmb_from_arm.data_length = 1;
     rxmb_from_arm.substitute_remote = 0;
     rxmb_from_arm.remote_transmission = 0;
     rxmb_from_arm.local_priority_enable = 0;
     rxmb_from_arm.local_priority_val = 0;
     result = flexcan_rx_mb_config(BSP_CAN_DEVICE, &flex_canopen_data, MB_RX_FROM_ARM, &rxmb_from_arm, CANOPEN_ARM_NODE+RXSDO);
     if (result){
         printf("\nReceive IO MB config error. Error Code: 0x%lx", result);
         return false;
     }


     //________________________________________________________________________________________________________
     //                                 SLAVE - LENZE
     txmb_to_lenze.code = kFlexCanTX_Data;             // Messaggio tipo dati
     txmb_to_lenze.msg_id_type = kFlexCanMbId_Std;     // Standard ID format (11 bit)
     txmb_to_lenze.data_length = 8;                    // Always 8 data bit
     txmb_to_lenze.substitute_remote = 0;
     txmb_to_lenze.remote_transmission = 0;
     txmb_to_lenze.local_priority_enable = 0;
     txmb_to_lenze.local_priority_val = 0;

     result = flexcan_tx_mb_config(BSP_CAN_DEVICE, &flex_canopen_data, MB_TX_TO_LENZE, &txmb_to_lenze, CANOPEN_LENZE_NODE+TXSDO);
     if (result){
         printf("\nTransmit MB config error. Error Code: 0x%lx", result);
         return false;
     }

     rxmb_from_lenze.code = kFlexCanRX_Ranswer;
     rxmb_from_lenze.msg_id_type = kFlexCanMbId_Std;
     rxmb_from_lenze.data_length = 1;
     rxmb_from_lenze.substitute_remote = 0;
     rxmb_from_lenze.remote_transmission = 0;
     rxmb_from_lenze.local_priority_enable = 0;
     rxmb_from_lenze.local_priority_val = 0;
     result = flexcan_rx_mb_config(BSP_CAN_DEVICE, &flex_canopen_data, MB_RX_FROM_LENZE, &rxmb_from_lenze, CANOPEN_LENZE_NODE+RXSDO);
     if (result){
         printf("\nReceive IO MB config error. Error Code: 0x%lx", result);
         return false;
     }


   return true;
 }

 //____________________________________________________________________________________
 //                     Thread of CAN ERRORS HANDLER
 void Can_RxErrors_Task(uint32_t parameter)
 {
     uint32_t can_errors;
     printf("CAN ERRORS TASK STARTED!\n");
      // Start receiving data
      while(1)
      {
          uint32_t result =flexcan_start_wait_errors(BSP_CAN_DEVICE, &can_errors);
          if (result){
              printf("\nFLEXCAN Receive Can errors code: 0x%lx\n", result);
              _time_delay(1000);
          }
          printf("CAN ERROR:");
          if(can_errors & CANERR_BUSOFF)  printf(" BUS-OFF");
          if(can_errors & CANERR_INTERR){ // ERROR INT
              printf(" INT(");
              if(can_errors & kFlexCan_Bit1Err) printf(" BIT1");
              if(can_errors & kFlexCan_Bit0Err) printf(" BIT0");
              if(can_errors & kFlexCan_AckErr) printf(" ACK");
              if(can_errors & kFlexCan_CrcErr) printf(" CRC");
              if(can_errors & kFlexCan_FrmErr) printf(" FRM");
              if(can_errors & kFlexCan_StfErr) printf(" STF");
              printf(")");
          }
          if(can_errors & kFlexCan_TxWrn)    printf(" TXWARN");
          if(can_errors & kFlexCan_RxWrn)    printf(" RXWARN");
          printf("\n");

          _time_delay(1000);
      }
 }





/*
    -----------   INPUTS TO MASTER TASK ----------------------------------------------
*/
void Inputs_To_Master_Task(uint32_t parameter)
{
   _CanSlaveFrame frame;

    _EVCLR(_EV0_INPUT_CAMBIATI);
    while(1)
    {
      // Attesa input cambiati con polling garantito ogni secondo
      if(_EVWAIT_TALL(_EV0_INPUT_CAMBIATI,1000)==FALSE) frame.changed = 0; // flag per dati in polling
      else frame.changed = 1; // Flag per cambio stato

      // Copia il contenuto degli input nel frame da trasmettere
      _mutex_lock(&input_mutex);
      _EVCLR(_EV0_INPUT_CAMBIATI);
      memcpy((void*) &frame.inputs, (void*) &SystemInputs, sizeof(SystemInputs));
      _mutex_unlock(&input_mutex);

      // Trasmissione
      frame.frcnt++; // Incrementa il progressivo
      if(flexcan_send(MB_TX_INPUTS_TO_MASTER, &txmb_inputs_to_master, CANOPEN_MASTER_NODE + CANOPEN_SRV_IO,8, (uint8_t*)&frame))
        printf("CAN TX ERROR\n");

      // Stampa il contenuto degli Inputs
      if(frame.changed){
          _EVSET(_EV0_POWER_EVENT);
          //printf("INPUT: %x %x %x %x %x %x %x\n", ((unsigned char*) &SystemInputs)[0],((unsigned char*) &SystemInputs)[1],((unsigned char*) &SystemInputs)[2],((unsigned char*) &SystemInputs)[3],((unsigned char*) &SystemInputs)[4],((unsigned char*) &SystemInputs)[5], ((unsigned char*) &SystemInputs)[6]);
      }
    }

}







/*TASK*-----------------------------------------------------------
*
* Task Name : Rx_Task
* Comments :
* 
*
*END*-----------------------------------------------------------*/
void Outputs_From_Master_Task(uint32_t parameter)
{
    flexcan_mb_t data_result;
    int lenght;
    printf("OUTPUTS FROM MASTER TASK STARTED..\n");

    // Start receiving data
    while(1)
    {

        if(flexcan_wait_receive(MB_RX_OUTPUT_FROM_MASTER,&data_result)==false){
            printf("\nFLEXCAN Receive Outputs Error \n");
            _time_delay(1000);
        }

        lenght = ((data_result.cs) >> 16) & 0xF;
        if(lenght!=8){
            printf("\nReceived wronght lenght CANOPEN frame\n");
            continue;
        }

        // The Outputs are downloaded to the PCB240 automatically
        _mutex_lock(&output_mutex);        
        memcpy(&SystemOutputs,data_result.data,sizeof(SystemOutputs));      
        _EVSET(_EV0_POWER_EVENT);
        _mutex_unlock(&output_mutex);

    }
}
          
void trx_tx_Task(uint32_t parameter)
{

    CiA402_Trx_Stat();

    printf("TRX CONTROLLER PROCESS EXIT!\n");
}


void arm_tx_Task(uint32_t parameter)
{
    CiA402_Arm_Stat();

    printf("ARM CONTROLLER PROCESS EXIT!\n");

}


void lenze_tx_Task(uint32_t parameter)
{
       driver_Lenze_Stat();

}


// Sends a buffer to Master for a max of 10 attempts
bool sendActuatorFrameToMaster(unsigned char* buffer){
    int attempt=10;
    while(attempt--){
        if(!flexcan_send(MB_TX_ACTUATOR_TO_MASTER, &txmb_actuator_to_master, CANOPEN_MASTER_NODE + CANOPEN_SRV_ACTUATORS,8, (uint8_t*)buffer)) return true;
        _time_delay(10);
    }

    printf("CAN SERVICE TX ERROR\n");
    return false;
}

/* EOF */
