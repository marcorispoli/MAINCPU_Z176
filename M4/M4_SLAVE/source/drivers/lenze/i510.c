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
#define _i510_C_
#include "dbt_m4.h"
#include "i510.h"

// return the current internal CiA402 status
bool getI510NetworkStatus(CANOPEN_CONTEXT_DEC, _i510_Status_t* pStat){
    _canopen_ObjectDictionary_t od= {i510_STATUSWORD_OD};

    // Verifies the current status
    if(canopenReadSDO(&od, CANOPEN_CONTEXT)==false) return false;

    switch(od.val){
    case CANOPEN_NETWORK_INIZIALIZATION_VAL:
    case CANOPEN_NETWORK_RESETNODE_VAL :
    case CANOPEN_NETWORK_RESETCOMM_VAL:
        pStat->networkStat = CANOPEN_NETWORK_INITIALIZATION;
        if(pStat->networkStat!=pStat->memNetworkStat) printf("LENZE NETWORK STATUS: INIT\n");
        break;
    case CANOPEN_NETWORK_STOPPED_VAL:
        pStat->networkStat = CANOPEN_NETWORK_STOPPED;
        if(pStat->networkStat!=pStat->memNetworkStat) printf("LENZE NETWORK STATUS: STOPPED\n");
        break;
    case CANOPEN_NETWORK_OPERATIONAL_VAL:
        pStat->networkStat = CANOPEN_NETWORK_OPERATIONAL;
        if(pStat->networkStat!=pStat->memNetworkStat) printf("LENZE NETWORK STATUS: OPERATIONAL\n");
        break;
    case CANOPEN_NETWORK_PRE_OPERATIONAL_VAL:
        pStat->networkStat = CANOPEN_NETWORK_PRE_OPERATIONAL;
        if(pStat->networkStat!=pStat->memNetworkStat) printf("LENZE NETWORK STATUS: PRE OPERATIONAL\n");
        break;
    default:
        pStat->networkStat = CANOPEN_NETWORK_UNDEFINED;
        if(pStat->networkStat!=pStat->memNetworkStat) printf("LENZE NETWORK STATUS: UNDEFINED\n");
        break;
    }

    if(pStat->networkStat!=pStat->memNetworkStat){
        pStat->statChanged=true;
        pStat->memNetworkStat = pStat->networkStat;
    }
    else
        pStat->statChanged=false;

    return true;

}

bool setI510PresetSpeed(CANOPEN_CONTEXT_DEC, unsigned char preset){
    _canopen_ObjectDictionary_t od= {i510_2860_01_OD};
    od.val =  preset;

    return canopenWriteSDO(&od, CANOPEN_CONTEXT);

}

// Imposta il contenuto di un setpoint
bool setI510WriteSDO(CANOPEN_CONTEXT_DEC,_OD_TEMPLATE, uint32_t data){
    _canopen_ObjectDictionary_t od= {_OD_TEMPLATE_VAL};
    od.val =  data;

    return canopenWriteSDO(&od, CANOPEN_CONTEXT);
}

// Legge il registro errori del lenze
uint16_t getI510ErrorRegister(CANOPEN_CONTEXT_DEC){
    _canopen_ObjectDictionary_t od= {i510_603F_00_OD};

    // Verifies the current status
    if(canopenReadSDO(&od, CANOPEN_CONTEXT)==true) {
        return od.val;
    }else return 0;

}


int getI510BusVoltage(CANOPEN_CONTEXT_DEC){
    _canopen_ObjectDictionary_t od= {i510_2D87_00_OD};

    // Verifies the current status
    if(canopenReadSDO(&od, CANOPEN_CONTEXT)==true) {
        return od.val;
    }else return -1;
}

char* i510ErrorString(unsigned short value){
    switch(value){
    case 0x2250: return "CiA: continuous overcurrent (inside the device) Fault";
    case 0x3222: return "DC-bus voltage too low for switch-on Warning ";
    case 0x3221: return "DC bus undervoltage warning Warning -";
    case 0x3220: return "DC bus undervoltage Trouble -";
    case 0x3211: return "DC bus overvoltage warning Warning -";
    case 0x3210: return "DC bus overvoltage Fault -";
    case 0x3180: return "Operation at UPS active Warning -";
    case 0x3120: return "Mains phase fault Fault -";
    case 0x2388: return "SLPSM stall detection active Trouble -";
    case 0x2387: return "Imax: Clamp responded too often Fault -";
    case 0x2383: return "I*t warning Warning -";
    case 0x2382: return "I*t error Fault 0x2D40:005 (P135.05)";
    case 0x2350: return "CiA: i²*t overload (thermal state) Fault 0x2D4B:003 (P308.03)";
    case 0x2340: return "CiA: -Short circuit (inside the device) Fault ";
    case 0x2320: return "CiA:-Short circuit/earth leakage (internal) Fault ";
    case 0x618A: return "Internal fan warning Warning -";
    case 0x5380: return "OEM hardware incompatible Fault -";
    case 0x5180: return "24-V supply overload Warning -";
    case 0x5112: return "24 V supply critical Warning -";
    case 0x4285: return "Power section overtemperature warning Warning -";
    case 0x4281: return "Heatsink fan warning Warning -";
    case 0x4280: return "Thermal sensor heatsink error Fault -";
    case 0x4210: return "PU: overtemperature fault Fault -";
    case 0x62B1: return "NetWordIN1 configuration incorrect Trouble";
    case 0x62A2: return "Network: user fault 2 Fault ";
    case 0x62A1: return "Network: user fault 1 Fault ";
    case 0x62A0: return "AC Drive: user fault Fault";
    case 0x6291: return "Number of maximum permissible faults exceeded Fault";
    case 0x6290: return "Reversal warning Warning";
    case 0x6282: return "User-defined fault 2 Fault";
    case 0x6281: return "User-defined fault 1 Fault";
    case 0x6280: return "Trigger/functions connected incorrectly Trouble ";
    case 0x7689: return "Memory module: invalid OEM data Warning";
    case 0x7686: return "Internal communication error Fault";
    case 0x7684: return "Data not completely saved before switch-off Warning";
    case 0x7682: return "Memory module: invalid user data Fault";
    case 0x7681: return "No memory module Fault";
    case 0x7680: return "Memory module is full Warning";
    case 0x7180: return "Motor overcurrent Fault 0x2D46:002 (P353.02)";
    case 0x7121: return "Pole position identification fault Fault 0x2C60";
    case 0x70A2: return "Analog output 2 fault Warning";
    case 0x70A1: return "Analog output 1 fault Warning ";
    case 0x7082: return "Error of analog input 2 Fault 0x2637:010 (P431.10)";
    case 0x7081: return "Error of analog input 1 Fault 0x2636:010 (P430.10)";
    case 0x7080: return "Monitoring of connection level (Low/High) Fault";
    case 0x63A3: return "Power section unknown Fault";
    case 0x63A2: return "PU: load error ID tag Fault";
    case 0x63A1: return "CU: load error ID tag Fault";
    case 0x8187: return "CAN: heartbeat time-out consumer 4 Fault 0x2857:008";
    case 0x8186: return "CAN: heartbeat time-out consumer 3 Fault 0x2857:007";
    case 0x8185: return "CAN: heartbeat time-out consumer 2 Fault 0x2857:006";
    case 0x8184: return "CAN: heartbeat time-out consumer 1 Fault 0x2857:005";
    case 0x8183: return "CAN: warning Warning 0x2857:011";
    case 0x8182: return "CAN: bus off Trouble 0x2857:010";
    case 0x8115: return "Time-out (PZÜ) No response 0x2552:004 (P595.04)";
    case 0x7697: return "Changed parameters lost Fault ";
    case 0x7696: return "EPM data: unknown parameter found Info";
    case 0x7695: return "Invalid configuration of parameter change-over Warning";
    case 0x7694: return "EPM data: new PU size detected Fault";
    case 0x7693: return "EPM data: PU size incompatible Fault";
    case 0x7692: return "EPM data: new firmware type detected Fault";
    case 0x7691: return "EPM data: firmware type incompatible Fault";
    case 0x7690: return "EPM firmware version incompatible Fault";
    case 0x768A: return "Memory module: wrong type Fault";
    case 0xFF0C: return "Motor phase failure phase W No response 0x2D45:001 (P310.01)";
    case 0xFF0B: return "Motor phase failure phase V No response 0x2D45:001 (P310.01)";
    case 0xFF0A: return "Phase U motor phase failure No response 0x2D45:001 (P310.01)";
    case 0xFF09: return "Motor phase missing No response 0x2D45:001 (P310.01)";
    case 0xFF06: return "Motor overspeed Fault 0x2D44:002 (P350.02)";
    case 0xFF05: return "Safe Torque Off error Fault";
    case 0x9080: return "Keypad removed Fault";
    case 0x8311: return "Torque limit reached No response 0x2D67:001 (P329.01)";
    case 0x8293: return "CAN: RPDO3 time-out Fault 0x2857:003";
    case 0x8292: return "CAN: RPDO2 time-out Fault 0x2857:002";
    case 0x8291: return "CAN: RPDO1 time-out Fault 0x2857:001";
    case 0x81A2: return "Modbus: incorrect request by master Warning";
    case 0x81A1: return "Modbus: network time-out Fault 0x2858:001 (P515.01)";
    case 0xFF85: return "eypad full control active Warning";
    case 0xFF56: return "Maximum motor frequency reached Warning";
    case 0xFF37: return "Automatic start disabled Fault ";
    case 0xFF19: return "Motor parameter identification error Fault";
    default: return "Undefined error";
    }
}



/* EOF */
