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

void main_task(uint32_t initial_data);
void devices_task(uint32_t initial_data);
void io_task(uint32_t initial_data);
void ioSetInputs(_MccFrame_Str* mcc_cmd);
void MainGetCfg(void);

void startupDigitalGantry(void);
void startupAnalogGantry(void);
extern void vmTask(uint32_t initial_data);

const TASK_TEMPLATE_STRUCT  MQX_template_list[] = 
{ 
   /* Task Index,                       Function,               Stack,  Priority,                       Name,           Attributes,             Param,  Time Slice */
   { _IDTASK(MAIN_TASK),                main_task,              1500,   _PRIOTASK(MAIN_TASK),           "main_task",    MQX_AUTO_START_TASK,    0,      0 },
   { _IDTASK(SER422_DRIVER),            ser422_driver,          1500,   _PRIOTASK(SER422_DRIVER),       "ser422_driver",0,                      0,      0 },
   { _IDTASK(PCB215),                   pcb215_driver,          1500,   _PRIOTASK(PCB215),              "pcb215_driver",0,                      0,      0 },
   { _IDTASK(PCB190),                   pcb190_driver,          1500,   _PRIOTASK(PCB190),              "pcb190_driver",0,                      0,      0 },
   { _IDTASK(PCB244_A),                 pcb244_A_driver,        1500,   _PRIOTASK(PCB244_A),            "pcb244_A_driver",0,                      0,      0 },
   { _IDTASK(BIOPSY),                   BIOPSY_driver,          1500,   _PRIOTASK(BIOPSY),              "BIOPSY_driver",0,                      0,      0 },
   { _IDTASK(PCB249U1),                 pcb249U1_driver,        1500,   _PRIOTASK(PCB249U1),            "pcb249U1_driver",0,                    0,      0 },
   { _IDTASK(PCB249U2),                 pcb249U2_driver,        1500,   _PRIOTASK(PCB249U2),            "pcb249U2_driver",0,                    0,      0 },
   { _IDTASK(GUI_INTERFACE),            gui_interface_task,     1500,   _PRIOTASK(GUI_INTERFACE),       "gui_task",     0,                      0,      0 },
   { _IDTASK(ANALOG_RX_TASK ),          analog_rx_task,         1500,   _PRIOTASK(ANALOG_RX_TASK ),     "analog_rx_task",  0,                      0,      0 },
   { _IDTASK(CANTX_TASK),               Can_Tx_Task,            1500,   _PRIOTASK(CANTX_TASK),          "CAN TX task",           0, 0, 0},
   { _IDTASK(CANRX_TASK),               Can_Rx_Task,            1500,   _PRIOTASK(CANRX_TASK),          "CAN RX task",           0, 0, 0},
   { _IDTASK(CANACTUATORS_TASK),        Can_RxActuators_Task,   1500,   _PRIOTASK(CANACTUATORS_TASK),   "CAN RX Actuators task", 0, 0, 0},
   { _IDTASK(CANRXERR_TASK),            Can_RxErrors_Task,      1500,   _PRIOTASK(CANRXERR_TASK),        "CAN RX ERR task",           0, 0, 0},
   { _IDTASK(VMUSIC3_TASK),             vmTask,                 1500,   _PRIOTASK(VMUSIC3_TASK),         "VMusic3 task",           0, 0, 0},
   { 0 }
};

void default_isr(){
    printf("UNANDLED INSTRUCTION OR POINTER\n");
}

void main_task(uint32_t initial_data)
{
  int i;
  _task_id tid;
  _SystemOutputs_Str output, mask;    
 _DeviceAppRegister_Str        ConfList;
 char* pDate;

 printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",REVISIONE);
 printf("MASTER PROCESS REVISION WITH CANOPEN:--- %s --- STARTING.....\n",REVISIONE);
 printf("\n\n\n\n\n",REVISIONE);

 generalConfiguration.revisioni.m4_master.maj = REVMAJ;
 generalConfiguration.revisioni.m4_master.min = REVMIN;
 generalConfiguration.gantryConfigurationReceived = false;
 generalConfiguration.canConnected = false;
 generalConfiguration.deviceConnected = false;
 generalConfiguration.slaveDeviceConnected = false;
 generalConfiguration.deviceConfigured = false;

 generalConfiguration.lenzeConnectedStatus = false;
 generalConfiguration.armConnectedStatus = false;
 generalConfiguration.trxConnectedStatus = false;
 generalConfiguration.candevice_error_startup = 0 ;

 generalConfiguration.deviceStarted = FALSE;  // Definisce lo stato di Startup dei drivers
 generalConfiguration.deviceConfigOk = FALSE; // Dichiara che la configurazione è giunta da A5
 generalConfiguration.loaderOn = false;
 generalConfiguration.demoMode = FALSE;
 generalConfiguration.trxExecution.run = false;
 generalConfiguration.trxExecution.id = 0;
 generalConfiguration.trxExecution.test = false;
 generalConfiguration.trxExecution.completed = true;
 generalConfiguration.trxExecution.idle = true;
 generalConfiguration.manual_mode_activation=_MANUAL_ACTIVATION_ARM_STANDARD;

 generalConfiguration.armExecution.run = false;
 generalConfiguration.armExecution.lenze_run = false;
 generalConfiguration.armExecution.id = 0;
 generalConfiguration.armExecution.test = false;
 generalConfiguration.armExecution.completed = true;
 generalConfiguration.armCfg.direction_memory = MEM_ARM_DIR_UNDEF;


 generalConfiguration.isInCompression = false;

 generalConfiguration.enableAudio = false;
 generalConfiguration.volumeAudio = 0;


 mcc_count = 0;
 
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
   
  // Creazione della struttura per gli Eventi generici
   if( _lwevent_create(&Ev1, 0) != MQX_OK)
   {
     printf("MAIN: IMPOSSIBILE CREARE STRUTTURA EVENTI Ev1!!\n");
     _mqx_exit(-1);
   }
   _lwevent_clear(&Ev1,0xFFFFFFFF);

  // Creazione della struttura per gli Eventi generici
   if( _lwevent_create(&Ev2, 0) != MQX_OK)
   {
     printf("MAIN: IMPOSSIBILE CREARE STRUTTURA EVENTI Ev2!!\n");
     _mqx_exit(-1);
   }
   _lwevent_clear(&Ev2,0xFFFFFFFF);

   // Creazione della struttura per gli Eventi delle sequenze
   if( _lwevent_create(&SeqEventi, 0) != MQX_OK)
   {
     printf("MAIN: IMPOSSIBILE CREARE STRUTTURA SEQUENZE EVENTI!!\n");
     _mqx_exit(-1);
   }  
   _lwevent_clear(&SeqEventi,0xFFFFFFFF);
 
     // Init Inputs e Outputs
   for(i=0; i<sizeof(_SystemOutputs_Str); i++)
     ((unsigned char*)&SystemOutputs)[i] = 0;
   for(i=0; i<sizeof(_SystemInputs_Str); i++)
     ((unsigned char*)&SystemInputs)[i] = 0;
 
#ifndef __NO_CANBUS
   // Task Gestione IO tramite CAN BUS
   canOpenMasterInit();
#endif
  
   // Partenza driver seriale
   tid= _task_create(0,_IDTASK(SER422_DRIVER),0);
   _EVWAIT_ANY(_EV0_COM_STARTED);


   // Apertura Interfaccia GUI: il sistema attende di ricevere la configurazione del Gantry prima di proseguire
   // inizia a inviare la configurazione
   tid= _task_create(0,_IDTASK(GUI_INTERFACE),0);
   _time_delay(100);
   while(!generalConfiguration.gantryConfigurationReceived) _time_delay(100);

   // Attivazione RTC e acquisizione ora corrente
   generalConfiguration.rtc_present = false;
   if(rtcInit()!=0) {
       printf("RTC DEVICE NOT DETECTED!\n");
   }else{
       if ((pDate = rtcGetDate()) == NULL){
           printf("RTC UNABLE TO READ DATE AND TIME!\n");
       }else{
           generalConfiguration.weekday = rtcGet(RTC_WEEKDAY);
           generalConfiguration.year = rtcGet(RTC_YEAR);
           generalConfiguration.month = rtcGet(RTC_MONTH);
           generalConfiguration.day = rtcGet(RTC_DAY);

           generalConfiguration.hour = rtcGet(RTC_HOURS);
           generalConfiguration.min = rtcGet(RTC_MINUTES);
           generalConfiguration.sec = rtcGet(RTC_SECONDS);

           generalConfiguration.rtc_present = true;
           printf("RTC DATE: %s\n", pDate);

      }
   }





#ifdef __NO_CANBUS
   printf("CAN BUS DISABLED IN THIS COMPILATION\n");
#else
   // Attivazione selettiva dei processi in relazione alla struttura della configurazione della macchina
   // Attivazione processi su Terminale Slave
   bool lenze=true;
   bool trx=generalConfiguration.gantryCfg.trxMotor;;
   bool arm= generalConfiguration.gantryCfg.armMotor;
   actuatorsStartProcess(lenze,trx,arm);

#endif

    startupAnalogGantry();
   _task_block();

}


void startupAnalogGantry(void){
    _task_id tid;
    printf("STARTUP GANTRY ANALOG MODEL\n");    

    // Partenza sequenze raggi nella versione digitale
   tid= _task_create(0,_IDTASK(ANALOG_RX_TASK),0);

   //   Apertura driver PCB244: il driver NON deve attendere la connessione
   tid = _task_create(0,_IDTASK(PCB244_A),(uint32_t) NULL);

   // Partenza processo gestione biopsia (opzionale)
   tid = _task_create(0,_IDTASK(BIOPSY),(uint32_t) NULL);

   // Crea il driver PCB215
   tid = _task_create(0,_IDTASK(PCB215),(uint32_t) NULL);

   //   Apertura driver PCB190
   tid = _task_create(0,_IDTASK(PCB190),(uint32_t) NULL);

   //   Apertura driver PCB249U1
   tid = _task_create(0,_IDTASK(PCB249U1),(uint32_t) NULL);

   //   Apertura driver PCB249U2
   tid = _task_create(0,_IDTASK(PCB249U2),(uint32_t) NULL);

   // Attende che i dispositivi locali siano connessi e che le revisioni siano arrivate
   printf("ATTESA CONNESSIONE PER I SEGUENTI PROCESSI: PCB249U1, PCB249U2, PCB190, PCB269\n");
   while(1){
       actuatorsGetStatus();
       if(_EVWAIT_TALL(_MOR4(_EV1_PCB249U2_CONNECTED,_EV1_PCB249U1_CONNECTED,_EV1_PCB190_CONNECTED,_EV1_PCB215_CONNECTED),100)==true) break;
   }

   // Autorizza il completamento del refresh registri sui vari moduli ed attende il completamento prima di autorizzare la configurazione
   printf("ATTESA REFRESH REGISTRI PERIFERICHE ..\n");
   _EVSET(_EV1_UPDATE_REGISTERS);

   // Attesa completamento dello startup di tutti i processi
   while(1){
       actuatorsGetStatus();
       if((_EVWAIT_TALL(_MOR4(_EV2_PCB249U1_STARTUP_OK,_EV2_PCB249U2_STARTUP_OK,_EV2_PCB190_STARTUP_OK,_EV2_PCB215_STARTUP_OK),100)==true) && (generalConfiguration.slaveDeviceConnected)) break;
       _time_delay(100);
   }
   printf("CONNESSIONE DI TUTTI I DISPOSITIVI COMPLETATA\n");

   // Segnalazione a GUI che può ricevere la configurazione dei dispositivi
   generalConfiguration.deviceConnected = 1;

   // Main si sospende. Non ha altro da fare
   printf("MAIN: STARTUP COMPLETATO. ATTESA CONFIGURAZIONE DISPOSITIVI\n");

   _EVWAIT_ALL(_EV1_DEV_CONFIG_OK);
    printf("MAIN: CONFIGURAZIONE COMPLETATA\n");

    generalConfiguration.deviceConfigured = true;

    // Se la configurazione prevede VMUSIC allora attiva la thread di gestione
    generalConfiguration.audioInitiated = false;
    if(generalConfiguration.enableAudio) tid = _task_create(0,_IDTASK(VMUSIC3_TASK),(uint32_t) NULL);

    // Fine thread main
   _task_block();
}


// Stampa della configurazione hardware
void mainPrintHardwareConfig(void){
    printf("HARDWARE CONFIGURATION:\n");
    printf("ANALOG MODEL\n");

    if(generalConfiguration.gantryCfg.armMotor) printf("ARM WITH MOTORIZATION\n");
    else printf("ARM WITH BRAKE\n");

    if(generalConfiguration.gantryCfg.trxMotor) printf("TILT WITH MOTORIZATION\n");
    else printf("NO TILT\n");

    if(generalConfiguration.gantryCfg.highSpeedStarter) printf("IAE HIGH SPEED STARTER\n");
    else printf("INTERNAL LOW SPEED STARTER\n");

}


/* EOF */
