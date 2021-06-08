#define _MASTER_CAN_C
#include "dbt_m4.h"

extern const flexcan_config_t flex_canopen_data;
static void updateInputs(_CanSlaveFrame* pFrame);

#define CAN_ERROR_ENABLE    false

/***********************************************************************
 *  Inizializzazione comunicazione su CAN
 *  - node_id == [1:127], 1 -> Master
 ***********************************************************************/
 bool canOpenMasterInit(void){
     uint32_t result;
     uint32_t u32;
    _task_id  created_task;

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

     //____________________________________________________________________________________________________________________
     //                                             IO
     txmb_to_io_slave.code = kFlexCanTX_Data;             // Messaggio tipo dati
     txmb_to_io_slave.msg_id_type = kFlexCanMbId_Std;     // Standard ID format (11 bit)
     txmb_to_io_slave.data_length = 8;                    // Always 8 data bit
     txmb_to_io_slave.substitute_remote = 0;
     txmb_to_io_slave.remote_transmission = 0;
     txmb_to_io_slave.local_priority_enable = 0;
     txmb_to_io_slave.local_priority_val = 0;

     result = flexcan_tx_mb_config(BSP_CAN_DEVICE, &flex_canopen_data, MB_TX_TO_IO_SLAVE, &txmb_to_io_slave, CANOPEN_MASTER_NODE+CANOPEN_SRV_IO);
     if (result){
         printf("\nTransmit MB config error. Error Code: 0x%lx", result);
         return false;
     }

     // RECEPTION FROM SLAVE
     rxmb_from_io_slave.code = kFlexCanRX_Ranswer;
     rxmb_from_io_slave.msg_id_type = kFlexCanMbId_Std;
     rxmb_from_io_slave.data_length = 1;
     rxmb_from_io_slave.substitute_remote = 0;
     rxmb_from_io_slave.remote_transmission = 0;
     rxmb_from_io_slave.local_priority_enable = 0;
     rxmb_from_io_slave.local_priority_val = 0;
     result = flexcan_rx_mb_config(BSP_CAN_DEVICE, &flex_canopen_data, MB_RX_FROM_IO_SLAVE, &rxmb_from_io_slave, CANOPEN_MASTER_NODE+CANOPEN_SRV_IO);
     if (result){
         printf("\nReceive IO MB config error. Error Code: 0x%lx", result);
         return false;
     }

     //____________________________________________________________________________________________________________________
     //                                             ACTUATORS
     txmb_to_actuators_slave.code = kFlexCanTX_Data;             // Messaggio tipo dati
     txmb_to_actuators_slave.msg_id_type = kFlexCanMbId_Std;     // Standard ID format (11 bit)
     txmb_to_actuators_slave.data_length = 8;                    // Always 8 data bit
     txmb_to_actuators_slave.substitute_remote = 0;
     txmb_to_actuators_slave.remote_transmission = 0;
     txmb_to_actuators_slave.local_priority_enable = 0;
     txmb_to_actuators_slave.local_priority_val = 0;

     result = flexcan_tx_mb_config(BSP_CAN_DEVICE, &flex_canopen_data, MB_TX_TO_ACTUATORS_SLAVE, &txmb_to_actuators_slave, CANOPEN_MASTER_NODE+CANOPEN_SRV_ACTUATORS);
     if (result){
         printf("\nTransmit MB config error. Error Code: 0x%lx", result);
         return false;
     }

     // RECEPTION FROM SLAVE
     rxmb_from_actuators_slave.code = kFlexCanRX_Ranswer;
     rxmb_from_actuators_slave.msg_id_type = kFlexCanMbId_Std;
     rxmb_from_actuators_slave.data_length = 1;
     rxmb_from_actuators_slave.substitute_remote = 0;
     rxmb_from_actuators_slave.remote_transmission = 0;
     rxmb_from_actuators_slave.local_priority_enable = 0;
     rxmb_from_actuators_slave.local_priority_val = 0;
     result = flexcan_rx_mb_config(BSP_CAN_DEVICE, &flex_canopen_data, MB_RX_FROM_ACTUATORS_SLAVE, &rxmb_from_actuators_slave, CANOPEN_MASTER_NODE+CANOPEN_SRV_ACTUATORS);
     if (result){
         printf("\nReceive IO MB config error. Error Code: 0x%lx", result);
         return false;
     }


     // Reception task creation
     created_task = _task_create(0, _IDTASK(CANRX_TASK), 0);
     if (created_task == MQX_NULL_TASK_ID)
     {
         printf("\nRx task: task creation failed.");
         _mqx_exit(-1);
     }

     // Reception task creation
     created_task = _task_create(0, _IDTASK(CANACTUATORS_TASK), 0);
     if (created_task == MQX_NULL_TASK_ID)
     {
         printf("\nRx Actuators task: task creation failed.");
         _mqx_exit(-1);
     }

     // Transmission task creation
     created_task = _task_create(0, _IDTASK(CANTX_TASK), 0);
     if (created_task == MQX_NULL_TASK_ID)
     {
         printf("\nTx task: task creation failed.");
         _mqx_exit(-1);
     }

     // Errorrs task creation
     created_task = _task_create(0, _IDTASK(CANRXERR_TASK), 0);
     if (created_task == MQX_NULL_TASK_ID)
     {
         printf("\nRx task: task creation failed.");
         _mqx_exit(-1);
     }

     return true;
 }




// Thread of CAN messages received from SLAVE
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


void updateInputs(_CanSlaveFrame* pFrame)
{
  int i;

  static unsigned char buffer[2*sizeof(SystemInputs)];
  _SystemInputs_Str changedInputs;

  //static bool prxStat = false;

  // Controllo di debug
  /*
  if(pFrame->inputs.CPU_XRAY_REQ){
      if(!prxStat){
          printf(">>>>>>>> PRX ON\n");
          prxStat = true;
          if(!pFrame->changed) printf("ERRORE TRASFERIMENTO PRX A MASTER\n");
      }

  }else{
      if(prxStat){
          printf("PRX OFF <<<<<<<\n");
          prxStat = false;
          if(!pFrame->changed) printf("ERRORE TRASFERIMENTO PRX A MASTER\n");
      }
  }*/

  // In ogni caso copia il contenuto aggiornato
  // Calcola i bit cambiati con un'operazione di XOR tra i nuovi e i precedenti
  pFrame->changed=0;
  _mutex_lock(&input_mutex);
  for(i=0; i< sizeof(SystemInputs); i++)
  {
    ((unsigned char*) &changedInputs)[i] = ((unsigned char*) &(pFrame->inputs))[i] ^ ((unsigned char*) &SystemInputs)[i];
    if( ((unsigned char*) &changedInputs)[i] ) pFrame->changed=1;

    ((unsigned char*) &SystemInputs)[i] = ((unsigned char*) &(pFrame->inputs))[i];
    buffer[i] = ((unsigned char*) &SystemInputs)[i];
    buffer[i+sizeof(SystemInputs)] = ((unsigned char*) &changedInputs)[i];
  }
  _mutex_unlock(&input_mutex);

  // Se inputs sono cambiati allora si determina chi è cambiato (changedInputs) e si aggiorna
  // la variabile globale SystemInputs
  if(pFrame->changed)
  {


      // Determina quali eventi devono essere segnalati all'interno
      _EVSET(_EV0_SYSTEM_INPUTS);

      // Usato dalle sequenze raggi
      if(SystemInputs.CPU_XRAY_COMPLETED) _EVSET(_EV2_XRAY_COMPLETED);

      // Usato dalle sequenze raggi
      if(SystemInputs.CPU_XRAY_ENA_ACK) _EVSET(_EV2_XRAY_ENA_ON);
      else      _EVSET(_EV2_XRAY_ENA_OFF);

      // Usato dalle sequenze raggi
      if(SystemInputs.CPU_XRAY_REQ){
          _EVSET(_EV2_XRAY_REQ_ON);
      }else{
          _EVSET(_EV2_XRAY_REQ_OFF);
      }

      // Attivazione rotazione braccio Manuale se la rotazione è motorizzata
      if(generalConfiguration.gantryCfg.armMotor){
          if(generalConfiguration.manual_mode_activation == _MANUAL_ACTIVATION_ARM_STANDARD){
              if((changedInputs.CPU_ROT_CCW)||(changedInputs.CPU_ROT_CW)){
                  if((SystemInputs.CPU_ROT_CCW)||(SystemInputs.CPU_ROT_CW)) actuatorsManualArmMove(generalConfiguration.manual_mode_activation);
              }
          }else if(generalConfiguration.manual_mode_activation == _MANUAL_ACTIVATION_TRX_STANDARD){
              if((changedInputs.CPU_ROT_CCW)||(changedInputs.CPU_ROT_CW)){
                  if((SystemInputs.CPU_ROT_CCW)||(SystemInputs.CPU_ROT_CW)) actuatorsManualTrxMove(generalConfiguration.manual_mode_activation);
              }
          }if(generalConfiguration.manual_mode_activation == _MANUAL_ACTIVATION_ARM_CALIB){
              if((changedInputs.CPU_ROT_CCW)||(changedInputs.CPU_ROT_CW)){
                  if((SystemInputs.CPU_ROT_CCW)||(SystemInputs.CPU_ROT_CW)) actuatorsManualArmMove(generalConfiguration.manual_mode_activation);
              }
          }if(generalConfiguration.manual_mode_activation == _MANUAL_ACTIVATION_TRX_CALIB){
              if((changedInputs.CPU_ROT_CCW)||(changedInputs.CPU_ROT_CW)){
                  if((SystemInputs.CPU_ROT_CCW)||(SystemInputs.CPU_ROT_CW)) actuatorsManualTrxMove(generalConfiguration.manual_mode_activation);
              }
          }
      }
  }

}

/*
    -----------   CAN TX TASK ----------------------------------------------
*/


 void CanSendToActuatorsSlave(unsigned char* buf){
     if(flexcan_send(MB_TX_TO_ACTUATORS_SLAVE, &txmb_to_actuators_slave, CANOPEN_MASTER_NODE + CANOPEN_SRV_ACTUATORS,8, (uint8_t*)buf)){
           printf("CAN TX TO SLAVE ERROR\n");
     }
 }

 void Can_Tx_Task(uint32_t parameter)
{
    _SystemOutputs_Str outputs;

    printf("CAN TX TASK STARTED..\n");
    while(1)
    {
      // Attesa Output cambiati con polling garantito ogni secondo
      _EVWAIT_TALL(_EV0_OUTPUT_CAMBIATI,1000);

      _mutex_lock(&output_mutex);
      _EVCLR(_EV0_OUTPUT_CAMBIATI);
      memcpy(&outputs,&SystemOutputs,sizeof(SystemOutputs));
      _mutex_unlock(&output_mutex);

      outputs.MASTER_TERMINAL_PRESENT=1;
      if(flexcan_send(MB_TX_TO_IO_SLAVE, &txmb_to_io_slave, CANOPEN_MASTER_NODE + CANOPEN_SRV_IO,8, (uint8_t*)&outputs))
        printf("CAN TX ERROR\n");
      else generalConfiguration.canConnected = true;

    }

}

//_______________________________________________________________________________________________________________________
//_______________________________________________________________________________________________________________________
// Thread of CAN messages received from SLAVE
 void Can_Rx_Task(uint32_t parameter)
 {
     flexcan_mb_t data_result;
     int lenght;

     printf("IO RECEIVER TASK STARTED\n");

     // Start receiving data
     while(1)
     {
         if(flexcan_wait_receive(MB_RX_FROM_IO_SLAVE, &data_result)==false){
             printf("\nFLEXCAN Receive Outputs Error \n");
             _time_delay(1000);
         }

         lenght = ((data_result.cs) >> 16) & 0xF;
         if(lenght!=8){
             printf("\nReceived wronght lenght CANOPEN frame\n");
             continue;
         }

         updateInputs((_CanSlaveFrame*) data_result.data);


     }
 }


 void Can_RxActuators_Task(uint32_t parameter)
 {
     flexcan_mb_t data_result;
     int lenght;

     printf("ACTUATORS RECEIVER TASK STARTED\n");

     // Start receiving data
     while(1)
     {
         if(flexcan_wait_receive(MB_RX_FROM_ACTUATORS_SLAVE, &data_result)==false){
             printf("\nFLEXCAN Receive Outputs Error \n");
             _time_delay(1000);
         }

         lenght = ((data_result.cs) >> 16) & 0xF;
         if(lenght!=8){
             printf("\nReceived wronght lenght CANOPEN frame\n");
             continue;
         }

         if(data_result.data[0]>ACTUATORS_ARM_CMD_SECTION)
            actuatorsRxFromArm(data_result.data);
         else if(data_result.data[0]>ACTUATORS_TRX_CMD_SECTION)
            actuatorsRxFromTrx(data_result.data);
         else if(data_result.data[0]>ACTUATORS_LENZE_CMD_SECTION)
            actuatorsRxFromLenze(data_result.data);
         else  actuatorsRxFromActuators(data_result.data);

     }
 }

/* EOF */
