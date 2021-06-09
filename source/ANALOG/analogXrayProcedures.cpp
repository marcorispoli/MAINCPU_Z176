#include "pageOpenAnalogic.h"
#include "analog.h"
#include "ui_analog.h"
#include "../application.h"
#include "../appinclude.h"
#include "../globvar.h"

void AnalogPageOpen::xrayReleasePushButton(void){
    // In ogni caso
}

/*
 */
// FUNZIONE DI CHIAMATA ESPOSIZIONE
void AnalogPageOpen::startXraySequence(void){

    Xprofile = pGeneratore->pAECprofiles->getCurrentProfileSymName();
    ApplicationDatabase.setData(_DB_XDMAS,(int) 0);
    ApplicationDatabase.setData(_DB_XDKV,(int) 0);
    ApplicationDatabase.setData(_DB_X_UDOSE, QString("AGD: ----"));
    saveOptions();

    if(ApplicationDatabase.getDataI(_DB_TECH_MODE) == ANALOG_TECH_MODE_MANUAL) xrayManualSequence();
    else if(ApplicationDatabase.getDataI(_DB_TECH_MODE) == ANALOG_TECH_MODE_SEMI) xraySemiAutoSequence();
    else xrayFullAutoSequence();

}

void AnalogPageOpen::xrayErrorInCommand(unsigned char code){
    if(!code) return;

    // Spegne il simbolo raggi in corso
    ApplicationDatabase.setData(_DB_XRAY_SYM,(unsigned char) 0, DBase::_DB_FORCE_SGN);

    // Attiva codice di errore auto ripristinante
    PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_RAGGI,code,true);

    return;
}

void AnalogPageOpen::xrayManualSequence(void){
    unsigned char data[18];

    // Impostazione kV
    pGeneratore->setkV((float) ApplicationDatabase.getDataI(_DB_DKV)/10);
    pGeneratore->setmAs((float) ApplicationDatabase.getDataI(_DB_DMAS)/10);

    XspessoreSeno = ApplicationDatabase.getDataI(_DB_SPESSORE);
    XselectedkV = pGeneratore->selectedKv;
    XselectedDmAs = pGeneratore->selectedDmAs;
    XselectedIa = pGeneratore->selectedIn;
    XselectedFuoco = pGeneratore->selectedFSize;
    XselectedFiltro = pCollimatore->getFiltroStat();
    Xpre_selectedDmAs = 0; // No preimpulso
    Xpre_selectedkV = 0; // No preimpulso
    XThick = ApplicationDatabase.getDataI(_DB_SPESSORE);
    XForce = ApplicationDatabase.getDataI(_DB_FORZA);


    // Impostazione dati di esposizione
    unsigned char errcode = pGeneratore->validateAnalogData(ANALOG_TECH_MODE_MANUAL, false, false);
    if(errcode){
        xrayErrorInCommand(errcode);
        return;
    }

    // Comunque effettua il refresh dello starter
    pGeneratore->refreshStarter();

    // Update del fuoco per sicurezza
    setCurrentFuoco();

    data[0] =  (unsigned char) (pGeneratore->selectedVdac&0x00FF);
    data[1] =  (unsigned char) (pGeneratore->selectedVdac>>8);
    data[2] =  (unsigned char) (pGeneratore->selectedIdac&0x00FF);
    data[3] =  (unsigned char) (pGeneratore->selectedIdac>>8);
    data[4] =  (unsigned char) (pGeneratore->selectedmAsDac&0x00FF);
    data[5] =  (unsigned char) (pGeneratore->selectedmAsDac>>8);
    data[6] =  pGeneratore->timeoutExp;
    data[7]=0;
    if(pGeneratore->SWA) data[7]|=1;
    if(pGeneratore->SWB) data[7]|=2;

    // Gestione dello Starter:
    if(pGeneratore->starterHS) data[7]|=4;        // Alta VelocitÃ

    data[8] =  0; // Tensione Griglia da aggiungere
    data[9] =  pGeneratore->maxV;
    data[10] = pGeneratore->minV;
    data[11] = pGeneratore->maxI;
    data[12] = pGeneratore->minI;

    // Gestione dello sblocco del compressore: NON IN BIOPSIA e non in combo
    if(pBiopsy->connected) data[13] = 0;
    else if(pConfig->userCnf.enableSblocco) data[13] = 1;
    else data[13]=0;

    // Aggiungo i valori nominali inviati al driver
    data[14] = (unsigned char) ((unsigned int) (pGeneratore->selectedKv * 10) & 0x00FF);
    data[15] = (unsigned char) ((unsigned int) (pGeneratore->selectedKv * 10) >> 8);
    data[16] = (unsigned char) ((unsigned int) (pGeneratore->selectedIn * 10) & 0x00FF);
    data[17] = (unsigned char) ((unsigned int) (pGeneratore->selectedIn * 10) >> 8);

    // Prova ad inviare il comando
    if(pConsole->pGuiMcc->sendFrame(MCC_XRAY_ANALOG_MANUAL,1,data,sizeof(data))==FALSE)
    {
        xrayErrorInCommand(ERROR_MCC_COMMAND);
        return;
    }

    ApplicationDatabase.setData(_DB_XRAY_SYM,(unsigned char) 1, DBase::_DB_FORCE_SGN);
    io->setXrayLamp(true);


}

void AnalogPageOpen::xraySemiAutoSequence(void){
    xrayFullAutoSequence();
}

void AnalogPageOpen::xrayFullAutoSequence(void){

    unsigned char data[25];
    int tpre_ms;

    XThick = ApplicationDatabase.getDataI(_DB_SPESSORE);
    XForce = ApplicationDatabase.getDataI(_DB_FORZA);

    // Impostazione kV pre impulso sulla base del fuoco attualmente selezionato
    if(ApplicationDatabase.getDataI(_DB_CURRENT_FUOCO) == Generatore::FUOCO_LARGE){
        pGeneratore->setkV((float) KV_PRE_FG_GRID);
        Xpre_selectedkV = KV_PRE_FG_GRID;
        tpre_ms = 10;
    }else{
        pGeneratore->setkV((float) KV_PRE_FP_NO_GRID);
        Xpre_selectedkV = KV_PRE_FP_NO_GRID;
        tpre_ms = 6;
    }

    // Impostazione mAs per il pre impulso
    pGeneratore->setmAs((float) mAs_PRE);

    // Impostazione dati di esposizione in funzione della modalità attuale
    unsigned char errcode = pGeneratore->validateAnalogData(ANALOG_TECH_MODE_AUTO, false, true);
    if(errcode){
        xrayErrorInCommand(errcode);
        return;
    }

    // Si precalcolano i Dmas con un valore fisso che dipende dal fuoco:
    // Con il fuoco grande sono 10ms mentre fuoco piccolo sono 6ms
    Xpre_selectedDmAs = pGeneratore->selectedIn * tpre_ms * 10 / 1000 ;

    // Refresh dello starter
    pGeneratore->refreshStarter();

    XspessoreSeno = ApplicationDatabase.getDataI(_DB_SPESSORE);
    XselectedFuoco = pGeneratore->selectedFSize;
    XselectedFiltro = pConfig->analogCnf.selected_filtro;


    data[0] =  (unsigned char) (pGeneratore->selectedVdac&0x00FF);
    data[1] =  (unsigned char) (pGeneratore->selectedVdac>>8);
    data[2] =  (unsigned char) (pGeneratore->selectedIdac&0x00FF);
    data[3] =  (unsigned char) (pGeneratore->selectedIdac>>8);
    data[4] =  (unsigned char) (pGeneratore->selectedmAsDac&0x00FF);
    data[5] =  (unsigned char) (pGeneratore->selectedmAsDac>>8);
    data[6] =  10 | 0x80; // 100ms massimi per ogni impulso: usa il timer corto da 10ms

    // Switch Generatore + Alta Velocita
    data[7]=0;
    if(pGeneratore->SWA) data[7]|=1;
    if(pGeneratore->SWB) data[7]|=2;
    if(pGeneratore->starterHS) data[7]|=4;

    // Tensione Griglia
    data[8] =  0;

    // Diagnostica Tensione / Corrente anodica
    data[9] =  pGeneratore->maxV;
    data[10] = pGeneratore->minV;
    data[11] = pGeneratore->maxI;
    data[12] = pGeneratore->minI;


    // Gestione dello sblocco del compressore: NON IN BIOPSIA e non in combo
    if(pBiopsy->connected) data[13] = 0;
    else if(pConfig->userCnf.enableSblocco) data[13] = 1;
    else data[13]=0;

    // Aggiungo i valori nominali inviati al driver
    data[14] = (unsigned char) ((unsigned int) (pGeneratore->selectedKv * 10) & 0x00FF);
    data[15] = (unsigned char) ((unsigned int) (pGeneratore->selectedKv * 10) >> 8);
    data[16] = (unsigned char) ((unsigned int) (pGeneratore->selectedIn * 10) & 0x00FF);
    data[17] = (unsigned char) ((unsigned int) (pGeneratore->selectedIn * 10) >> 8);

    data[18] = 0; // Riservato per gli impulsi dell'esposimetro
    data[19] = 0;
    data[20] = 0; // 0=pre-pulse, 1 = Pulse
    data[21] = 0; // Nessun errore

    // Prova ad inviare il comando
    if(pConsole->pGuiMcc->sendFrame(MCC_XRAY_ANALOG_AUTO,1,data,21)==FALSE)
    {
        xrayErrorInCommand(ERROR_MCC_COMMAND);
        return;
    }

    ApplicationDatabase.setData(_DB_XRAY_SYM,(unsigned char) 1, DBase::_DB_FORCE_SGN);
    io->setXrayLamp(true);

}


void AnalogPageOpen::guiNotify(unsigned char id, unsigned char mcccode, QByteArray rxdata)
{
    float ldmas;
    float ldose;
    float pre_ldose;
    float uG;

    int offset;
    unsigned char data[25];
    int   rxplog;
    int   rxfiltro;
    float rxkV;
    int   rxdmas;
    int   rxpulses;
    int   errcode;
    int   campi;

    QString logstring;
    QString fuoco_string;
    QString exposure_string;
    QString pfilter_string;
    QString Pfilter_string;
    QString field_string;


    switch(mcccode)
    {

    case MCC_XRAY_ANALOG_MANUAL:
    case MCC_XRAY_ANALOG_AUTO:

        stopAttesaDati();

        ldmas = ((float) (rxdata.at(1) + 256 * rxdata.at(2)));

        // Associazione offset sul piano di compressione per il calcolo dell'entrance dose
        if(!pPotter->isMagnifier()) offset = 0;
        else offset = ApplicationDatabase.getDataI(_DB_MAG_OFFSET);

        // Calcolo dose pre impulso + impulso
        pre_ldose = pGeneratore->pDose->getDoseUg(XspessoreSeno,offset, Xpre_selectedDmAs,Xpre_selectedkV*10,XselectedFiltro); // Contributo pre impulso se presente
        ldose = pGeneratore->pDose->getDoseUg(XspessoreSeno,offset, ldmas,XselectedkV*10,XselectedFiltro); // Contributo impulso se presente

        ApplicationDatabase.setData(_DB_XDMAS,(int) ldmas);
        ApplicationDatabase.setData(_DB_XDKV,(int) (XselectedkV * 10) );

        // Aggiornamento campi dose
        uG =ldose+pre_ldose;
        cumulativeXdose += uG;

        if(uG==0) ApplicationDatabase.setData(_DB_X_UDOSE, QString("AGD: ----"));
        else if(pConfig->analogCnf.doseFormat == 'u'){
            ApplicationDatabase.setData(_DB_X_UDOSE, QString("AGD: %1 (uG)").arg(QString::number(uG,'f',1)));
        }else if(pConfig->analogCnf.doseFormat == 'm'){
            ApplicationDatabase.setData(_DB_X_UDOSE, QString("AGD: %1 (mG)").arg(QString::number(uG/1000,'f',2)));
        }else{
            ApplicationDatabase.setData(_DB_X_UDOSE, QString("AGD: %1 (Zv)").arg(QString::number(uG*50/1000,'f',2)));
        }


        // Aggiornamento delle statistiche solo con generazione
        if((rxdata.at(0)==RXOK)||(rxdata.at(0)<LAST_ERROR_WITH_PREP)){
            pGeneratore->notifyStatisticData(pGeneratore->selectedKv, ldmas/10, true);
        }

        if(pBiopsy->connected){
            logstring = "BIOPSY EXPOSURE: ";
        }else{
            if(ApplicationDatabase.getDataI(_DB_TECH_MODE) == ANALOG_TECH_MODE_SEMI) logstring = "(1P) EXPOSURE: ";
            else if(ApplicationDatabase.getDataI(_DB_TECH_MODE) == ANALOG_TECH_MODE_AUTO) logstring = "(0P) EXPOSURE: ";
            else logstring = "MANUAL EXPOSURE: ";
        }

        if(rxdata.at(0)){
            PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_RAGGI, rxdata.at(0),TRUE); // Self resetting
            logstring += "ABORTED!! ";
        }else{
            logstring += "COMPLETED. ";
        }

        // kV and mAs
        if(ApplicationDatabase.getDataI(_DB_TECH_MODE) != ANALOG_TECH_MODE_MANUAL){
            logstring += "PROFILE:" + Xprofile + ", ";
            logstring += "pre-kV:" + QString("%1").arg(Xpre_selectedkV) + ", ";
            logstring += "pre-mAs:" + QString("%1").arg((float)Xpre_selectedDmAs/10) + ", ";
        }
        logstring += "kV:" + QString("%1").arg(XselectedkV) + ", ";
        logstring += "mAs:" + QString("%1").arg(ldmas/10) + ", ";
        logstring += "Dose(uG):" + QString("%1").arg(ldose+pre_ldose) + ", ";
        logstring += "TotalDose(uG):" + QString("%1").arg(cumulativeXdose) + ", ";


        // Compressor
        logstring += "THICK:" + QString("%1").arg(XThick) + ", ";
        logstring += "FORCE:" + QString("%1").arg(XForce) + ", ";

        // Filter
        if(XselectedFiltro == Collimatore::FILTRO_Mo)   logstring += "FILTER:Mo, ";
        else logstring += "FILTER:Rh, ";

        // Focus
        if(pGeneratore->selectedFSize == Generatore::FUOCO_LARGE) logstring += "FOCUS: L, ";
        else fuoco_string = "FOCUS: S, ";

        // Angolo braccio
        logstring += "ARM:" + QString("%1").arg(getArm()) + "°, ";

        if(ApplicationDatabase.getDataI(_DB_TECH_MODE) != ANALOG_TECH_MODE_MANUAL){
            campi = ApplicationDatabase.getDataI(_DB_CAMPI);
            if(campi==ANALOG_AECFIELD_FRONT) field_string = "DET:FRONT, ";
            else if(campi==ANALOG_AECFIELD_CENTER)  field_string = "DET:CENTER, ";
            else  field_string = "DET:BACK, ";
            logstring += "Pl:" + QString("%1").arg(XPlog) + ", ";
            logstring += "Rd:" + QString("%1").arg(XRad) ;
        }


        // Rilascio Pulsante raggi
        ApplicationDatabase.setData(_DB_XRAY_SYM,(unsigned char) 0, DBase::_DB_FORCE_SGN);
        io->setXrayLamp(false);

        // Reimposta se necessario il filtro selezionato (in caso di AEC con cambio filtro)
        emit queuedExecution(QUEUED_SELECTED_FILTER,0,""); // Impostazione Filtro
        emit queuedExecution(QUEUED_LOG_FLUSH,0,logstring); // Impostazione Filtro

        break;


     // Messaggio per richiedere i dati di curva zero dall'esposimetro
     case MCC_XRAY_ANALOG_REQ_AEC_PULSE:

            // Richiede i dati dell'AEC
            rxplog = rxdata[0] + 256 * rxdata[1];
            XPlog = rxplog;
            XRad =  rxdata[2] + 256*rxdata[3];

            // rxrad  = rxdata[2] + 256*rxdata[3];

            // Preleva i dati per la nuova esposizione
            errcode = pGeneratore->pAECprofiles->getAecData(rxplog,ApplicationDatabase.getDataI(_DB_FILTER_MODE), XselectedFiltro, ApplicationDatabase.getDataI(_DB_OD),ApplicationDatabase.getDataI(_DB_TECHNIC),pGeneratore->selectedAnodo, pGeneratore->selectedFSize, &rxfiltro,&rxkV,&rxdmas,&rxpulses);
            if(errcode < 0){
                // In caso di errore, comunica il codice di errore con cui il driver deve uscire
                stopAttesaDati();
                data[20] = 1;
                if(errcode == -4 )  data[21] =  ESPOSIMETRO_BREAST_DENSE;
                else  data[21] =  ESPOSIMETRO_INVALID_AEC_DATA;
                pConsole->pGuiMcc->sendFrame(MCC_XRAY_ANALOG_AUTO,1,data,22);
                return;
            }

            // Se la modalità è SEMI-AUTO, i kV sono selezionati manualmente.
            if(ApplicationDatabase.getDataI(_DB_TECH_MODE) == ANALOG_TECH_MODE_SEMI){
                    rxkV = (float) ApplicationDatabase.getDataI(_DB_DKV) / 10.0;
                    pGeneratore->setkV(rxkV);
                    rxdmas = pGeneratore->getMaxDMas(rxkV,pGeneratore->selectedAnodo, pGeneratore->selectedFSize);
            }else   pGeneratore->setkV(rxkV);
            pGeneratore->setmAs((float) rxdmas/10);

            logstring = QString("AEC-KV:%1 mAs:%2").arg(rxkV).arg((float) rxdmas/10);

            // Seleziona il filtro opportunamente
            pCollimatore->setFiltro((Collimatore::_FilterCmd_Enum) rxfiltro, true);
            XselectedFiltro = rxfiltro;

            // Impostazione dati di esposizione           
            errcode = pGeneratore->validateAnalogData(ANALOG_TECH_MODE_AUTO, false, false);
            if(errcode){
                logstring += QString(" ERR:%1").arg(errcode);
                emit queuedExecution(QUEUED_LOG_FLUSH,0,logstring);
                stopAttesaDati();
                xrayErrorInCommand(errcode);
                return;
            }

            XselectedkV = pGeneratore->selectedKv;
            XselectedDmAs = pGeneratore->selectedDmAs;
            XselectedIa = pGeneratore->selectedIn;


            data[0] =  (unsigned char) (pGeneratore->selectedVdac&0x00FF);
            data[1] =  (unsigned char) (pGeneratore->selectedVdac>>8);
            data[2] =  (unsigned char) (pGeneratore->selectedIdac&0x00FF);
            data[3] =  (unsigned char) (pGeneratore->selectedIdac>>8);
            data[4] =  (unsigned char) (pGeneratore->selectedmAsDac&0x00FF);
            data[5] =  (unsigned char) (pGeneratore->selectedmAsDac>>8);
            data[6] = pGeneratore->timeoutExp;

            // Switch Generatore + Alta Velocita
            data[7]=0;
            if(pGeneratore->SWA) data[7]|=1;
            if(pGeneratore->SWB) data[7]|=2;
            if(pGeneratore->starterHS) data[7]|=4;

            // Codice di errore
            data[8] =  0;

            // Diagnostica Tensione / Corrente anodica
            data[9] =  pGeneratore->maxV;
            data[10] = pGeneratore->minV;
            data[11] = pGeneratore->maxI;
            data[12] = pGeneratore->minI;

            // Gestione dello sblocco del compressore: NON IN BIOPSIA e non in combo
            if(pBiopsy->connected) data[13] = 0;
            else if(pConfig->userCnf.enableSblocco) data[13] = 1;
            else data[13]=0;

            // Aggiungo i valori nominali inviati al driver
            data[14] = (unsigned char) ((unsigned int) (pGeneratore->selectedKv * 10) & 0x00FF);
            data[15] = (unsigned char) ((unsigned int) (pGeneratore->selectedKv * 10) >> 8);
            data[16] = (unsigned char) ((unsigned int) (pGeneratore->selectedIn * 10) & 0x00FF);
            data[17] = (unsigned char) ((unsigned int) (pGeneratore->selectedIn * 10) >> 8);

            data[18] = (unsigned char) (rxpulses);
            data[19] = (unsigned char) (rxpulses>>8);
            data[20] = 1; // 0=pre-pulse, 1 = Pulse
            data[21] = 0; // Nessun errore da conmunicare

            // Prova ad inviare il comando
            if(pConsole->pGuiMcc->sendFrame(MCC_XRAY_ANALOG_AUTO,1,data,22)==FALSE)
            {
                xrayErrorInCommand(ERROR_MCC_COMMAND);
                return;

            }

            break;
    default:
            return;
        break;
    }
}
