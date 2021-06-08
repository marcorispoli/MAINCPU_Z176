/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 1989-2008 ARC International
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale MQX RTOS License
* distributed with this Material.
* See the MQX_RTOS_LICENSE file distributed for more details.
*
* Brief License Summary:
* This software is provided in source form for you to use free of charge,
* but it is not open source software. You are allowed to use this software
* but you cannot redistribute it or derivative works of it in source form.
* The software may be used only in connection with a product containing
* a Freescale microprocessor, microcontroller, or digital signal processor.
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains the source for the rs485 example program.
*
*
*END************************************************************************/
#define _MAIN_C_
#include "dbt_m4.h"


static void mcc_get_rev(void);


void main_task(uint32_t initial_data);
void devices_task(uint32_t initial_data);

const TASK_TEMPLATE_STRUCT  MQX_template_list[] = 
{ 
   /* Task Index,                       Function,               Stack,  Priority,                       Name,           Attributes,             Param,  Time Slice */
   { _IDTASK(MAIN_TASK),                main_task,              1500,   _PRIOTASK(MAIN_TASK),           "main_task",    MQX_AUTO_START_TASK,    0,      0 },
   { _IDTASK(FAULT_TASK),               fault_task,             1500,   _PRIOTASK(FAULT_TASK),          "fault_task",   0,                      0,      0 },
   { _IDTASK(CPU_TO_PCB240_DRIVER),     pcb240_driver,          1500,   _PRIOTASK(CPU_TO_PCB240_DRIVER),"pcb240_task",  0,                      0,      0 },
   { _IDTASK(CANRXERR_TASK),            Can_RxErrors_Task,      1500,   _PRIOTASK(CANRXERR_TASK),       "CAN RXERR task",  0,                      0,      0 },
   { _IDTASK(INPUTS_TASK),              Inputs_To_Master_Task,  1500,   _PRIOTASK(INPUTS_TASK),          "Inputs to Master",  0,                      0,      0 },
   { _IDTASK(OUTPUTS_TASK),             Outputs_From_Master_Task, 1500,   _PRIOTASK(OUTPUTS_TASK),          "Output to Master",  0,                      0,      0 },
   { _IDTASK(TRX_TX_TASK),              trx_tx_Task,            1500,   _PRIOTASK(TRX_TX_TASK),         "trx_tx_task",  0,                      0,      0 },
   { _IDTASK(ARM_TX_TASK),              arm_tx_Task,            1500,   _PRIOTASK(ARM_TX_TASK),         "arm_tx_task",  0,                      0,      0 },
   { _IDTASK(LENZE_TX_TASK),            lenze_tx_Task,          1500,   _PRIOTASK(LENZE_TX_TASK),       "lenze_tx_task",  0,                    0,      0 },
   { _IDTASK(DEVICE_STARTUP_TASK),      device_startup_Task,    1500,   _PRIOTASK(DEVICE_STARTUP_TASK), "device_startup_task",  0,                    0,      0 },
   { _IDTASK(ACTUATORS_RX_FROM_MASTER), actuators_rx_master,   1500,   _PRIOTASK(ACTUATORS_RX_FROM_MASTER),  "actuators_rx_master",  0,             0,      0 },
   { _IDTASK(ACTUATORS_RX_FROM_DEVICES),actuators_rx_devices,  1500,   _PRIOTASK(ACTUATORS_RX_FROM_DEVICES),  "actuators_rx_devices",  0,             0,      0 },
   { 0 }
};


_MccFrame_Str mcc_cmd;  // Comandi ricevuti da GUI
MCC_MEM_SIZE mcc_len;   // Dimensione messaggio ricevuto

void main_task(uint32_t initial_data)
{
  int i;
  _task_id pcb240_id;
  _task_id fault_id;
  uint32_t u32val;
  uint16_t u16val;


  generalConfiguration.lenzeDriver=false;
  generalConfiguration.trxDriver=false;
  generalConfiguration.armDriver=false;
  generalConfiguration.deviceStarted=false;
  generalConfiguration.deviceConnected = false;
  generalConfiguration.lenzeCalibPot = false;


  // Inizializzazione flags generali
  isPowerdown = false;

 printf("SLAVE PROCESS REVISION WITH CANOPEN:--- %s --- STARTING.....\n",REVISIONE);
 
  // Creazione Nodo 0 MCC
  switch(mcc_initialize(0))
  {
    case MCC_ERR_DEV:
      printf ("M4: IMPOSSIBILE CREARE IL NODO 0");
      _task_block();
    break;

    case MCC_SUCCESS:
      printf ("M4: NODO 0 CREATO\n");
     break;
  }

  // Inizializzazione delle mutex
  if (_mutex_init(&input_mutex, NULL) != MQX_OK)
  {
    printf("Initializing print mutex failed.\n");
    _mqx_exit(-1);
  }
  if (_mutex_init(&output_mutex, NULL) != MQX_OK)
  {
    printf("Initializing print mutex failed.\n");
    _mqx_exit(-1);
  }

  // Creazione della struttura per gli Eventi generici
   if( _lwevent_create(&Ev0, 0) != MQX_OK)
   {
     printf("MAIN: IMPOSSIBILE CREARE STRUTTURA EVENTI!!\n");
     _mqx_exit(-1);
   }
   _lwevent_clear(&Ev0,0xFFFFFFFF);
   
   // Init Inputs e Outputs
   for(i=0; i<sizeof(_SystemOutputs_Str); i++)
     ((unsigned char*)&SystemOutputs)[i] = 0;
   for(i=0; i<sizeof(_SystemInputs_Str); i++)
     ((unsigned char*)&SystemInputs)[i] = 0;
 
   /////////////////////////////////////////////////////////////////////////////
   // Task Gestione Faults
   fault_id = _task_create(0,_IDTASK(FAULT_TASK),0);
   _time_delay(100); // Attende che tutti i processi siano partiti

   /////////////////////////////////////////////////////////////////////////////
   // Task Gestione PCB240
   pcb240_id = _task_create(0,_IDTASK(CPU_TO_PCB240_DRIVER),0);
   _time_delay(100); // Attende che tutti i processi siano partiti
  
   /////////////////////////////////////////////////////////////////////////////
   // CAN INTERFACE
   canOpenSlaveInit(); // Inizializza le strutture relative alla comunicazione CAN
   
   // Reception from MASTER
   _task_id created_task = _task_create(0, _IDTASK(CANRXERR_TASK), 0);
   if (created_task == MQX_NULL_TASK_ID)
   {
       printf("\nRx task: task creation failed.");
       _mqx_exit(-1);
   }

   // Reception from MASTER
   created_task = _task_create(0, _IDTASK(OUTPUTS_TASK), 0);
   if (created_task == MQX_NULL_TASK_ID)
   {
       printf("\nRx task: task creation failed.");
       _mqx_exit(-1);
   }

   // Transmission to MASTER
   created_task = _task_create(0, _IDTASK(INPUTS_TASK), 0);
   if (created_task == MQX_NULL_TASK_ID)
   {
       printf("\nTx task: task creation failed.");
       _mqx_exit(-1);
   }

   _task_id id = _task_create(0,_IDTASK(ACTUATORS_RX_FROM_MASTER),0);
   id = _task_create(0,_IDTASK(ACTUATORS_RX_FROM_DEVICES),0);

  
  // Creazione Endpoint per ricezione comandi da GUI SLAVE
  MCC_ENDPOINT  ep={_DEF_APP_SLAVE_TO_M4_SLAVE};
  if(mccOpenPort(&ep)==FALSE)
  {
    printf ("GUI INTERFACE: MEMORIA NON DISPONIBILE PER END POINT\n");
    _mqx_exit(-1);
  }


  // Attende di ricevere dalla PCB240 la revisione corrente, prima di
  // mettersi in attesa dei messaggi dalla Gui..
  // Attesa comandi da Interfaccia Core A5 SLAVE
  // Richiesta revisione corrente a PCB240

  if(pcb240UpdateRevision()==FALSE)
  {
    // In caso di fallimento restituisce il solo codice di M4
    printf("RICHIESTA REVISIONE FW A PCB240 FALLITA!\n");
  }

  unsigned char buffer[20];
  while(1)
  {
    if(mccRxFrame(&ep, &mcc_cmd))
    {
      
      switch(mcc_cmd.cmd)
      {
      case MCC_SLAVE_GET_REV:        
        mcc_get_rev();
      break;

      case MCC_POWER_OFF:
          printf("RICHIESTO POWER-OFF:%d\n",(int) mcc_cmd.buffer[0]);
          if(pcb240PowerOffCommand( mcc_cmd.buffer[0])){
              mccGuiNotify(1,mcc_cmd.cmd,(unsigned char*) null ,0);
          }
          break;
       case MCC_CALIB_LENZE:
          if(mcc_cmd.buffer[0]==1)   generalConfiguration.lenzeCalibPot = true;
          else generalConfiguration.lenzeCalibPot = false;

          // Aggiorna il potenziometro verso terminale SLAVE
          buffer[0] = lenzeGetStatus()->analog1;   // ANALOG 1 (\millesimi)
          buffer[1] = (unsigned char) (lenzeGetStatus()->analog1>>8);
          buffer[2] = (unsigned char) lenzeGetStatus()->analog2;    // ANALOG 2 (\millesimi)
          buffer[3] = (unsigned char) (lenzeGetStatus()->analog2>>8);
          mccGuiNotify(1,MCC_CALIB_LENZE,buffer,4);

          if(generalConfiguration.lenzeCalibPot) printf("ATTIVATO CALIBRAZIONE POTENZIOMETRO\n");
          else printf("DISATTIVATO CALIBRAZIONE POTENZIOMETRO\n");
          break;

      case MCC_GET_TRX_INPUTS:

          // Lettura Inputs
          u32val = getTrxInputs();
          buffer[0] = (unsigned char) u32val;
          buffer[1] = (unsigned char) (u32val>>8);
          buffer[2] = (unsigned char) (u32val>>16);
          buffer[3] = (unsigned char) (u32val>>24);

          // Lettura tensione di Bus
          u32val = getTrxVbus();
          printf("VBUS=%d\n",u32val);
          buffer[4] = (unsigned char) u32val;
          buffer[5] = (unsigned char) (u32val>>8);
          buffer[6] = (unsigned char) (u32val>>16);
          buffer[7] = (unsigned char) (u32val>>24);

          // Lettura tensione di Logica
          u32val = getTrxVlogic();
          printf("VLOGIC=%d\n",u32val);
          buffer[8] = (unsigned char) u32val;
          buffer[9] = (unsigned char) (u32val>>8);
          buffer[10] = (unsigned char) (u32val>>16);
          buffer[11] = (unsigned char) (u32val>>24);

          u32val = getTrxTemp();
          printf("TEMP=%d\n",u32val);
          buffer[12] = (unsigned char) u32val;
          buffer[13] = (unsigned char) (u32val>>8);
          buffer[14] = (unsigned char) (u32val>>16);
          buffer[15] = (unsigned char) (u32val>>24);

          u32val = getTrxFault();
          printf("FAULT=%x\n",u32val);
          buffer[16] = (unsigned char) u32val;
          buffer[17] = (unsigned char) (u32val>>8);
          buffer[18] = (unsigned char) (u32val>>16);
          buffer[19] = (unsigned char) (u32val>>24);
          mccGuiNotify(1,MCC_GET_TRX_INPUTS,buffer,20);
          break;

      case MCC_GET_ARM_INPUTS:

          // Lettura Inputs
          u32val = getArmInputs();
          buffer[0] = (unsigned char) u32val;
          buffer[1] = (unsigned char) (u32val>>8);
          buffer[2] = (unsigned char) (u32val>>16);
          buffer[3] = (unsigned char) (u32val>>24);

          // Lettura tensione di Bus
          u32val = getArmVbus();
          printf("VBUS=%d\n",u32val);
          buffer[4] = (unsigned char) u32val;
          buffer[5] = (unsigned char) (u32val>>8);
          buffer[6] = (unsigned char) (u32val>>16);
          buffer[7] = (unsigned char) (u32val>>24);

          // Lettura tensione di Logica
          u32val = getArmVlogic();
          printf("VLOGIC=%d\n",u32val);
          buffer[8] = (unsigned char) u32val;
          buffer[9] = (unsigned char) (u32val>>8);
          buffer[10] = (unsigned char) (u32val>>16);
          buffer[11] = (unsigned char) (u32val>>24);

          u32val = getArmTemp();
          printf("TEMP=%d\n",u32val);
          buffer[12] = (unsigned char) u32val;
          buffer[13] = (unsigned char) (u32val>>8);
          buffer[14] = (unsigned char) (u32val>>16);
          buffer[15] = (unsigned char) (u32val>>24);

          u32val = getArmFault();
          printf("FAULT=%x\n",u32val);
          buffer[16] = (unsigned char) u32val;
          buffer[17] = (unsigned char) (u32val>>8);
          buffer[18] = (unsigned char) (u32val>>16);
          buffer[19] = (unsigned char) (u32val>>24);
          mccGuiNotify(1,MCC_GET_ARM_INPUTS,buffer,20);
          break;

      case MCC_GET_LENZE_INPUTS:

          // Lettura Inputs: // o1,sto,i5,i4,i3,i2,i1
          buffer[0] = (unsigned char) lenzeGetIO();
          printf("LENZE IO:%x\n", buffer[0]);

          buffer[1] = (unsigned char) lenzeGetAn1();
          buffer[2] = (unsigned char) lenzeGetAn2();

          u32val = lenzeGetLowThreshold();
          buffer[3] = (unsigned char) u32val;
          buffer[4] = (unsigned char) (u32val>>8);

          u32val = lenzeGetHighThreshold();
          buffer[5] = (unsigned char) u32val;
          buffer[6] = (unsigned char) (u32val>>8);

          u32val = lenzeGetVBUS();
          buffer[7] = (unsigned char) u32val;
          buffer[8] = (unsigned char) (u32val>>8);

          // Temperatura
          u32val = lenzeGetTemp();
          buffer[9] = (unsigned char) u32val;
          buffer[10] = (unsigned char) (u32val>>8);

          buffer[11] = lenzeGetDiagnosticErrors();

          u16val = lenzeGetInternalErrors();
          buffer[12] = (unsigned char) u16val;
          buffer[13] = (unsigned char) (u16val>>8);

          mccGuiNotify(1,MCC_GET_LENZE_INPUTS,buffer,14);
          break;

      default:
        printf("MCC NON DECODIFICATO:%d\n", mcc_cmd.cmd);
        break;
      }
    }
  }
 

}

/* Richiesta revisione software corrente
    PARAMETRI: nessuno
    
return:
Se PCB240 non risponde allora solo 2 byte: 
  data[0]= MAJ M4 MASTER
  data[1]= MIN M4 MASTER

Se PCB240 risponde allora sono 4 byte: 
  data[0]= MAJ M4 MASTER
  data[1]= MIN M4 MASTER
  data[2]= MAJ PCB240
  data[3]= MIN PCB240

*/
void mcc_get_rev(void)
{
  unsigned char data[10];
  data[0] = REVMAJ;
  data[1] = REVMIN;
  printf("RICEVUTO COMANDO RICHIESTA REVISIONE\n");
  
  // Richiesta revisione corrente a PCB240
  if(pcb240UpdateRevision()==FALSE) 
  {
    // In caso di fallimento restituisce il solo codice di M4 
    printf("RICHIESTA REVISIONE FW A PCB240 FALLITA!\n");
    mccGuiNotify(1,mcc_cmd.cmd,data,2);
    return;
  }
  data[2] = revPCB240.maj;
  data[3] = revPCB240.min;
  
  mccGuiNotify(1,mcc_cmd.cmd,data,4);
  return;
}




void show_mqx_error(int code){
    switch(code){
    case MQX_EINVAL:
        printf("MQX EINVAL CODE\n");
        break;
    case MQX_INVALID_COMPONENT_BASE:
        printf("MQX_INVALID_COMPONENT_BASE\n");
        break;
    case MQX_OUT_OF_MEMORY:
        printf("MQX_OUT_OF_MEMORY\n");
        break;
    default:
        printf("MQX ERROR UNDEFINED\n");
        break;
    }


}

void mainStartProcesses(void){
    if(generalConfiguration.deviceStarted) return;
    generalConfiguration.deviceStarted = true;

    _task_id  created_task = _task_create(0, _IDTASK( DEVICE_STARTUP_TASK), 0);
    if (created_task == MQX_NULL_TASK_ID)
    {
        printf("\nIMPOSSIBLE ATTIVARE THREAD DI STARTUP PROCESSI. QUIT PROGRAM\n");
        _mqx_exit(-1);
    }
    return;
}

//______________________________________________________________________________
// Thread di lancio dei dispositivi
void device_startup_Task(uint32_t parameter)
{
    _task_id  created_task;

    // IL driver LENZE è il primo a partire
    if(generalConfiguration.lenzeDriver){
        printf("ATTIVAZIONE LENZE DRIVER\n");
        created_task = _task_create(0, _IDTASK( LENZE_TX_TASK), 0);
        if (created_task == MQX_NULL_TASK_ID)
        {
            printf("\nIMPOSSIBLE TO START LENZE DRIVER. QUIT PROGRAM\n");
            _mqx_exit(-1);
        }

        while(!generalConfiguration.lenzeConnected) _time_delay(100);
        printf("LENZE DRIVER CONNECTED!\n");
    }

    // Segue il driver TRX
    if(generalConfiguration.trxDriver){
        printf("ATTIVAZIONE TRX DRIVER\n");
        created_task = _task_create(0, _IDTASK( TRX_TX_TASK), 0);
        if (created_task == MQX_NULL_TASK_ID)
        {
            printf("\nIMPOSSIBLE TO START TRX DRIVER. QUIT PROGRAM\n");
            _mqx_exit(-1);
        }
        while(!generalConfiguration.trxConnected) _time_delay(100);
        printf("TRX DRIVER CONNECTED!\n");
    }

    // Driver  ARM
   if(generalConfiguration.armDriver){
       printf("ATTIVAZIONE ARM DRIVER\n");
       created_task = _task_create(0, _IDTASK( ARM_TX_TASK), 0);
       if (created_task == MQX_NULL_TASK_ID)
       {
           printf("\nIMPOSSIBLE TO START ARM DRIVER. QUIT PROGRAM\n");
           _mqx_exit(-1);
       }

       while(!generalConfiguration.armConnected) _time_delay(100);
       printf("ARM DRIVER CONNECTED!\n");

   }


    // Attende che tutti i dispositivi dell'architettura hardware siano in fine connessi
    // Prima di dare l'ok dello Slave
    if(generalConfiguration.pcb240connected)
        generalConfiguration.deviceConnected = true;


    // Fine della thread di startup
   _task_block();
}

/* EOF */
