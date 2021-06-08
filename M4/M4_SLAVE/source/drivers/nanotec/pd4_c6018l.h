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
#ifndef _PD4_C6018L_H_
#define _PD4_C6018L_H_

// CiA Driver Function interfaces
#define CiA402_SwitchOnDisabled_To_ReadyToSwitchOn  Pd4CiA402ShutdownCommand
#define CiA402_GetCurrentStatus                     getPd4CiA402Status
#define CiA402_ReadyToSwitchOn_To_SwitchedOn        Pd4CiA402SwitchOnCommand
#define CiA402_To_SwitchOnDisabled                  Pd4CiA402DisableVoltageCommand
#define CiA402_SwitchedOn_To_ReadyToSwitchOn        Pd4CiA402ShutdownCommand
#define CiA402_SwitchedOn_To_OperationEnabled       Pd4CiA402EnableOperationCommand
#define CiA402_OperationEnabled_To_ReadyToSwitchOn  Pd4CiA402ShutdownCommand
#define CiA402_OperationEnabled_To_SwitchedOn       Pd4CiA402DisableOperationCommand
#define CiA402_SetOperatingOD                       Pd4CiA402SetOperatingOD
#define CiA402_Fault_To_SwitchOnDisabled            Pd4CiA402DisableVoltageCommand

//_____ OBJECT DICTIONARY ______________________________________
#define PD4_MAX_SPEED           0x2032, 0x00, CANOPEN_SDO_TX_4BYTE  // rot/min

#define PD4_MAX_CURRENT         0x2031, 0x00, CANOPEN_SDO_TX_4BYTE  // mA
#define PD4_NOM_CURRENT         0x203B, 0x01, CANOPEN_SDO_TX_4BYTE  // mA
#define PD4_MAX_TIME_CURRENT    0x203B, 0x02, CANOPEN_SDO_TX_4BYTE  // ms

//_____ POSITIONING ______________________________________

#define PD4_POSITION_OPTION_CODE    0x60F2, 0x00, CANOPEN_SDO_TX_2BYTE  // options in positionning
#define PD4_MIN_POSITION            0x607D, 0x01, CANOPEN_SDO_TX_4BYTE  // user unit
#define PD4_MAX_POSITION            0x607D, 0x02, CANOPEN_SDO_TX_4BYTE  // user unit
#define PD4_MAX_POSITION_SPEED      0x6081, 0x00, CANOPEN_SDO_TX_4BYTE  // user unit
#define PD4_END_POSITION_SPEED      0x6082, 0x00, CANOPEN_SDO_TX_4BYTE  // user unit
#define PD4_POSITION_ACCELL         0x6083, 0x00, CANOPEN_SDO_TX_4BYTE  // user unit
#define PD4_POSITION_DECELL         0x6084, 0x00, CANOPEN_SDO_TX_4BYTE  // user unit
#define PD4_POSITION_QS_DECELL      0x6085, 0x00, CANOPEN_SDO_TX_4BYTE  // user unit
#define PD4_POSITION_JERKLIM        0x6086, 0x00, CANOPEN_SDO_TX_2BYTE  // user unit
#define PD4_MAX_POSITION_ACCELL     0x60C5, 0x00, CANOPEN_SDO_TX_4BYTE  // user unit
#define PD4_MAX_POSITION_DECELL     0x60C6, 0x00, CANOPEN_SDO_TX_4BYTE  // user unit

#define POSITION_SETTING_CTRL_INIT  0x0370, 0x0020
#define POSITION_TRIGGER_SETTING_CTRL_INIT  0x0370, 0x0000
#define POSITION_SETTING_START      0x0370, 0x0030

#endif
/* EOF */
