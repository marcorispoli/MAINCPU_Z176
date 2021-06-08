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
#define _PD4_C_
#include "dbt_m4.h"
#include "pd4.h"

// return the current internal CiA402 status
bool getPd4CiA402Status(CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat){
    _canopen_ObjectDictionary_t od= {OD_6041_00};

    // Verifies the current status
    if(canopenReadSDO(&od, CANOPEN_CONTEXT)==false) return false;


    if((od.val&CiA402MASK(CiA402Mask_NotReadyToSwitchOn))==CiA402VAL(CiA402Mask_NotReadyToSwitchOn))
        pStat->Cia402Stat = CiA402_NotReadyToSwitchOn;
    else if((od.val&CiA402MASK(CiA402Mask_SwitchOnDisabled))==CiA402VAL(CiA402Mask_SwitchOnDisabled))
        pStat->Cia402Stat = CiA402_SwitchOnDisabled;
    else if((od.val&CiA402MASK(CiA402Mask_ReadyToSwitchOn))==CiA402VAL(CiA402Mask_ReadyToSwitchOn))
        pStat->Cia402Stat = CiA402_ReadyToSwitchOn;
    else if((od.val&CiA402MASK(CiA402Mask_SwitchedOn))==CiA402VAL(CiA402Mask_SwitchedOn))
        pStat->Cia402Stat = CiA402_SwitchedOn;
    else if((od.val&CiA402MASK(CiA402Mask_OperationEnabled))==CiA402VAL(CiA402Mask_OperationEnabled))
        pStat->Cia402Stat = CiA402_OperationEnabled;
    else if((od.val&CiA402MASK(CiA402Mask_QuickStopActive))==CiA402VAL(CiA402Mask_QuickStopActive))
        pStat->Cia402Stat = CiA402_QuickStopActive;
    else if((od.val&CiA402MASK(CiA402Mask_FaultReactionActive))==CiA402VAL(CiA402Mask_FaultReactionActive))
        pStat->Cia402Stat = CiA402_FaultReactionActive;
    else if((od.val&CiA402MASK(CiA402Mask_Fault))==CiA402VAL(CiA402Mask_Fault))
        pStat->Cia402Stat = CiA402_Fault;
    else pStat->statChanged=CiA402_UndefinedStat;

    if(pStat->Cia402Stat!=pStat->memCia402Stat){
        pStat->statChanged=true;
        pStat->memCia402Stat = pStat->Cia402Stat;
    }
    else
        pStat->statChanged=false;

    return true;

}

bool Pd4CiA402ShutdownCommand(CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat){
    _canopen_ObjectDictionary_t ctrlword= {OD_6040_00};
    if(canopenReadSDO(&ctrlword, CANOPEN_CONTEXT)==false) return false;

    // Setta i soli bit per il cambio stato
    ctrlword.val &=~ CiA402MASK(PD4_SHUTDOWN);
    ctrlword.val |= CiA402VAL(PD4_SHUTDOWN);

    return canopenWriteSDO(&ctrlword, CANOPEN_CONTEXT);

}

bool Pd4CiA402SwitchOnCommand(CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat){
    _canopen_ObjectDictionary_t ctrlword= {OD_6040_00};
    if(canopenReadSDO(&ctrlword, CANOPEN_CONTEXT)==false) return false;

    // Setta i soli bit per il cambio stato
    ctrlword.val &=~ CiA402MASK(PD4_SWITCHON);
    ctrlword.val |= CiA402VAL(PD4_SWITCHON);

    return canopenWriteSDO(&ctrlword, CANOPEN_CONTEXT);

}

bool Pd4CiA402DisableVoltageCommand(CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat){
    _canopen_ObjectDictionary_t ctrlword= {OD_6040_00};
    if(canopenReadSDO(&ctrlword, CANOPEN_CONTEXT)==false) return false;

    // Setta i soli bit per il cambio stato
    ctrlword.val &=~ CiA402MASK(PD4_DISVOLTAGE);
    ctrlword.val |= CiA402VAL(PD4_DISVOLTAGE);

    return canopenWriteSDO(&ctrlword, CANOPEN_CONTEXT);

}


bool Pd4CiA402EnableOperationCommand(uint32_t ctrlmask, uint32_t ctrlval, CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat){
    _canopen_ObjectDictionary_t ctrlword= {OD_6040_00};
    if(canopenReadSDO(&ctrlword, CANOPEN_CONTEXT)==false) return false;

    // Setta i soli bit per il cambio stato
    ctrlword.val &=~ CiA402MASK(PD4_ENABLEOP);
    ctrlword.val |= CiA402VAL(PD4_ENABLEOP);

    // Initialization values
    ctrlword.val &= ~ctrlmask;
    ctrlword.val |= ctrlval;

    return canopenWriteSDO(&ctrlword, CANOPEN_CONTEXT);

}

bool Pd4CiA402DisableOperationCommand(uint32_t ctrlmask, uint32_t ctrlval, CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat){
    _canopen_ObjectDictionary_t ctrlword= {OD_6040_00};
    if(canopenReadSDO(&ctrlword, CANOPEN_CONTEXT)==false) return false;

    // Setta i soli bit per il cambio stato
    ctrlword.val &=~ CiA402MASK(PD4_DISABLEOP);
    ctrlword.val |= CiA402VAL(PD4_DISABLEOP);

    // Initialization values
    ctrlword.val &= ~ctrlmask;
    ctrlword.val |= ctrlval;

    return canopenWriteSDO(&ctrlword, CANOPEN_CONTEXT);

}



bool Pd4CiA402SetControlOD(uint32_t mask, uint32_t val, CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat){
    _canopen_ObjectDictionary_t ctrlword= {OD_6040_00};
    if(canopenReadSDO(&ctrlword, CANOPEN_CONTEXT)==false) return false;


    ctrlword.val &= ~mask;
    ctrlword.val |= val;


    return canopenWriteSDO(&ctrlword, CANOPEN_CONTEXT);

}

bool Pd4CiA402SetOperatingOD(uint8_t val, CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat){
    _canopen_ObjectDictionary_t od= {OD_6060_00};
    od.val = val;
    return canopenWriteSDO(&od, CANOPEN_CONTEXT);

}

bool Pd4CiA402FaultReset(CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat){
    _canopen_ObjectDictionary_t od= {OD_6040_00};

    if(canopenReadSDO(&od, CANOPEN_CONTEXT)==false) return false;

    // Set
    od.val |= 0x80;
    if(canopenWriteSDO(&od, CANOPEN_CONTEXT)==false) return false;
    _time_delay(10);
    od.val &=~ 0x80;
    if(canopenWriteSDO(&od, CANOPEN_CONTEXT)==false) return false;
    _time_delay(10);

    return true;
}

bool Pd4CiA402QuickStop(CANOPEN_CONTEXT_DEC, _PD4_Status_t* pStat){
    _canopen_ObjectDictionary_t od= {OD_6040_00};

    int attempt=100;

    while(attempt--){
        od.val=0x0002; // QUICK STOP COMMAND
        if(canopenWriteSDO(&od, CANOPEN_CONTEXT)==true) return true;
        _time_delay(10);
    }

    printf("FALLITO TENTATIVO DI QUICK STOP\n");
    return false;
}

// Upload a set of dictionary object. Repeat a writing at least 10 times then return false.
// Every Writing is done every @delay (ms)
// The list SHALL ends with the element {0,0,0}
bool canopenUploadObjectDictionaryList(const _canopen_ObjectDictionary_t* pDictionary, int delay, CANOPEN_CONTEXT_DEC){
    int index=0, error=0;
    if(pDictionary==null) return false;

    while((pDictionary[index].index)||(pDictionary[index].sbidx)||(pDictionary[index].type)){
        // printf("SCRITTURA DI: 0x%x:0x%x\n",pDictionary[index].index,pDictionary[index].sbidx);
        if(canopenWriteSDO((_canopen_ObjectDictionary_t*) &pDictionary[index], CANOPEN_CONTEXT)==true){
            error=0;
            index++;
        }else error++;

        // In case of exceeded errors, returns false
        if(error>10) return false;
        _time_delay(delay);
    }

    return true;
}


bool uploadOjectDictionary(unsigned short ID, const char* DEVICE, const _canopen_ObjectDictionary_t* pDictionary,CANOPEN_CONTEXT_DEC){
    _canopen_ObjectDictionary_t user_param_od={OD_2700_03};
    _canopen_ObjectDictionary_t user_param_save_od={OD_2700_01};
    _canopen_ObjectDictionary_t od_save_od={OD_1010_01};
    bool retcmd;


    printf("%s: UPLOAD OBJECT DICTIONARY\n",DEVICE);

    // Upload of the Object Dictionary general parameters to configure the motor device
    while(canopenUploadObjectDictionaryList(pDictionary,10,CANOPEN_CONTEXT)==false){
        printf("%s: ERROR IN UPLOADING THE GENERAL OBJECT DICTIONARY ITEMS\n",DEVICE);
        _time_delay(500);
    }

    // Legge il contenuto dello User Parameter
    if(canopenReadSDO(&user_param_od, CANOPEN_CONTEXT)==false){
        printf("%s: ERROR IN READING THE USER PARAM REGISTER\n",DEVICE);
        return false;
    }

    if((user_param_od.val)==ID){
        printf("%s: UPLOAD PARAMETRI GENERALI, COMPLETATO\n",DEVICE);
        return true;
    }

    printf("%s: SAVING OBJECT DICTIONARY ..\n",DEVICE);

    user_param_od.val = ID;
    while(canopenWriteSDO(&user_param_od, CANOPEN_CONTEXT)==false) {
        _time_delay(100);
        printf("%s: ATTEMPT TO WRITE THE ID REGISTER\n",DEVICE);
        return false;
    }

    // Salva il contenuto della user ID
    user_param_save_od.val = 1;
    if(canopenWriteSDO(&user_param_save_od, CANOPEN_CONTEXT)==false) {
        printf("%s: ERROR IN SAVING THE USER PARAM REGISTER\n",DEVICE);
        return false;
    }

    od_save_od.val = OD_SAVE_CODE;
    while(canopenWriteSDO(&od_save_od, CANOPEN_CONTEXT)==false) {
        printf("%s: ERROR IN SAVING THE USER PARAM REGISTER\n",DEVICE);
        _time_delay(100);
    }

    // Attende il completamento ..
    _time_delay(1000);
    while(1){
        retcmd = canopenReadSDO(&od_save_od, CANOPEN_CONTEXT);
        if(retcmd==false){
            _time_delay(100);
            continue;
        }
        if(od_save_od.val==1) break;
    }
    printf("%s: OBJECT DICTIONARY UPLOADED AND SAVED\n",DEVICE);

    return true;
}

bool caricamentoNanojProgram(bool forceFlash, const char* DEVICE, const unsigned char* vmmFile, uint32_t vmmSize, CANOPEN_CONTEXT_DEC){
    _canopen_ObjectDictionary_t user_param_od_nanoj={OD_2700_02};
    _canopen_ObjectDictionary_t user_param_save_od={OD_2700_01};
    _canopen_ObjectDictionary_t vmm_ctrl_od={OD_1F51_02};
    _canopen_ObjectDictionary_t vmm_data_od={OD_1F50_02};
    _canopen_ObjectDictionary_t vmm_status_od={OD_1F57_02};
    bool retcmd;

    // Verifica il caricamento del codice NANOJ e del salvataggio dei parametri in generale
    unsigned char vmmchk = 0;
    for(unsigned int i=0; i<vmmSize;i++) vmmchk+=vmmFile[i];

    // Legge il contenuto dello User Parameter
    if(canopenReadSDO(&user_param_od_nanoj, CANOPEN_CONTEXT)==false){
        printf("%s: ERROR IN READING THE USER PARAM REGISTER\n",DEVICE);
        return false;
    }

    if((!forceFlash)&&(user_param_od_nanoj.val==vmmchk)) return true; // Già aggiornata
    printf("%s: UPDATE NANOJ FLASH PROGRAM EXECUTING...\n",DEVICE);


    // Cancellazione flash nanoj
    printf("%s: ERASING FLASH ... \n",DEVICE);
    vmm_ctrl_od.val = VMM_DELETE;
    if(canopenWriteSDO(&vmm_ctrl_od, CANOPEN_CONTEXT)==false) {
        printf("%s: ERROR IN ERASING THE FLASH\n",DEVICE);
        return false;
    }

    // Effettuazione reset per completare l'operazione
    printf("%s: RESET NODE ... \n",DEVICE);
    canopenResetNode(2000, CANOPEN_CONTEXT);

    // Attende che il nodo sia operativo
    while(canopenReadSDO(&vmm_status_od, CANOPEN_CONTEXT)==false) _time_delay(500);

    // Inizializza flash per salvataggio di tutti i blocchi
    vmm_ctrl_od.val = VMM_INIT;
    if(canopenWriteSDO(&vmm_ctrl_od, CANOPEN_CONTEXT)==false) {
        printf("%s: ERROR IN INIT THE FLASH\n",DEVICE);
        return false;
    }

    // Scrive x blocchi di 1024 byte nella ram di appoggio
    int index=0;
    int size;

    while(1){
        size = vmmSize - (index * 1024);
        if(size==0) break;
        if(size>1024) size = 1024;

        printf("%s: DOWNLOAD BLOCK %d... \n",DEVICE, index);
        if(canopenWriteDomainSDO(&vmm_data_od, CANOPEN_CONTEXT,&(vmmFile[index*1024]), size)==false) {
            printf("%s: ERROR IN DOWNLOADING FLASH DATA BLOCK\n",DEVICE);
            return false;
        }

        // Scrive in flash il blocco dati
        printf("%s: WRITE BLOCK ... \n",DEVICE);
        vmm_ctrl_od.val = VMM_WRITE;
        if(canopenWriteSDO(&vmm_ctrl_od, CANOPEN_CONTEXT)==false) {
            printf("%s: ERROR IN WRITING THE FLASH\n",DEVICE);
            return false;
        }
        // Attende il completamento ..
        _time_delay(100);
        while(1){
            retcmd = canopenReadSDO(&vmm_ctrl_od, CANOPEN_CONTEXT);
            if(retcmd==false){
                _time_delay(100);
                continue;
            }
            if(vmm_ctrl_od.val==0) break;
        }
        // Legge lo status per verificare il corretto salvataggio
        if(canopenReadSDO(&vmm_status_od, CANOPEN_CONTEXT)==false){
            printf("%s: ERROR IN READING THE FLASH STATUS REGISTER\n",DEVICE);
            return false;
        }
        if(vmm_status_od.val!=0){
            printf("%s: ERROR IN THE WRITING COMMAND: %x\n",DEVICE,vmm_status_od.val);
            return false;
        }

        if(size<1024) break;
        index++;
    }

    printf("%s: NANO-J PROGRAM SUCCESSFULLY DOWNLOADED\n",DEVICE);

    // Salva il checksum in modo che alla ripartenza successiva non ripeta la scrittura della flash
    user_param_od_nanoj.val = vmmchk;
    while(canopenWriteSDO(&user_param_od_nanoj, CANOPEN_CONTEXT)==false) {
        _time_delay(100);
        printf("%s: ATTEMPT TO WRITE THE ID REGISTER\n",DEVICE);
        return false;
    }

    // Salva il contenuto della user ID
    user_param_save_od.val = 1;
    if(canopenWriteSDO(&user_param_save_od, CANOPEN_CONTEXT)==false) {
        printf("%s: ERROR IN SAVING THE USER PARAM REGISTER\n",DEVICE);
        return false;
    }

    // Operazione completata con successo
    printf("%s: UPLOAD NANOJ PROGRAM COMPLETED\n",DEVICE);
    return true;

}
/* EOF */
