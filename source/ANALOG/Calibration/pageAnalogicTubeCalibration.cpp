#include "../analog.h"
#include "pageCalibAnalogic.h"
#include "analog_calib.h"
#include "ui_analog_calib.h"
#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"

// SetAnalogTubeOffset()
// SetAnalogFineRaggiCalibKv



//____________________________________________________________________
//  FUNZIONE CHIAMATA PERIODIAMENTE PER TESTARE LO STATO DI READY
//
bool AnalogCalibPageOpen::getTubeCalibrationReady(unsigned char opt){

    int ready_stat=0;

    if(!pc_data_valid) ready_stat|=1;                                                               // Missing data
    if(!ApplicationDatabase.getDataU(_DB_READY_EXPOSURE)) ready_stat|=2;                            // PC not READY
    if(!ApplicationDatabase.getDataU(_DB_AWS_CONNECTION)) ready_stat|=4;                            // PC DISCONNECTED
    ApplicationDatabase.setData(_DB_CALIB_TUBE_READY_STAT,ready_stat,opt);
    if(ready_stat) return false;
    return true;
}

void AnalogCalibPageOpen::exitTubeCalibration(void){
    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(tubeValueChanged(int,int)));
    disconnect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(tubeGuiNotify(unsigned char,unsigned char,QByteArray)));   
    tube_calibration = false;
}

void AnalogCalibPageOpen::initTubeCalibration(void){

    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(tubeValueChanged(int,int)),Qt::UniqueConnection);
    connect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(tubeGuiNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);   

    // Impostazione geometria del frame di lavoro
    ui->tubeCalibFrame->setGeometry(189,79,461,336);
    ui->largeFocusFrame->setGeometry(95,15,41,26);
    ui->smallFocusFrame->setGeometry(95,15,41,26);

    ui->pcConnectedFrame->show();
    ui->tubeCalibFrame->show();

    // Set intestazione pagina
    if(ApplicationDatabase.getDataU(_DB_EXPOSURE_MODE) == _EXPOSURE_MODE_CALIB_MODE_KERMA)
        ui->intestazione->setText(QString("KERMA CALIBRATION PANEL"));
    else
        ui->intestazione->setText(QString("TUBE CALIBRATION PANEL"));

    // Stato di attesa dati   
    pc_data_valid = false;
    tube_calibration = true;

    if(isMaster){
        pSysLog->log("SERVICE PANEL: ANALOGIC TUBE CALIBRATION");

    }
    setTubeData();

}

void AnalogCalibPageOpen::setTubeData(void){
    if(!tube_calibration) return;
    if(!isMaster) return;

    // Aggiorna il Tubo correntemente selezionato
    selectTube(pConfig->userCnf.tubeFileName);

    if(!pc_data_valid) {
        // Inizializza il pannello senza i dati validi
        ApplicationDatabase.setData(_DB_CALIB_TUBE_FOCUS,(int) Generatore::FUOCO_SZ_ND,DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_CALIB_TUBE_FILTER,(int) Collimatore::FILTRO_ND,DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_CALIB_TUBE_KV,(int) 0,DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_CALIB_TUBE_VDAC,(int) 0,DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_CALIB_TUBE_IA,(int) 0,DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_CALIB_TUBE_IDAC,(int) 0,DBase::_DB_FORCE_SGN);

        ApplicationDatabase.setData(_DB_CALIB_TUBE_DMAS,(int) 0,DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_CALIB_TUBE_DKVR,(int) 0,DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_CALIB_TUBE_IAR,(int) 0,DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_CALIB_TUBE_DMASR,(int) 0,DBase::_DB_FORCE_SGN);


        // Impostazione collimazione aperta
        pCollimatore->manualCollimation = true;
        pCollimatore->manualF = 0;
        pCollimatore->manualB = 0;
        pCollimatore->manualL = 0;
        pCollimatore->manualR = 0;
        pCollimatore->manualColliUpdate();
        return;
    }
    // Il PC ha assegnato i dati di lavoro

    if(ApplicationDatabase.getDataU(_DB_EXPOSURE_MODE) == _EXPOSURE_MODE_CALIB_MODE_KERMA){
        pGeneratore->setFuoco(Generatore::FUOCO_LARGE);
        pGeneratore->setkV(pc_selected_kV);
        pGeneratore->setmAs(pc_selected_mAs);
        pGeneratore->validateAnalogData(ANALOG_TECH_MODE_MANUAL, true, false);

        pc_selected_vdac = pGeneratore->selectedVdac;
        pc_selected_Idac = pGeneratore->selectedIdac;
        pc_selected_Ia = pGeneratore->selectedIn;

    }

    // Impostazione del fuoco
    ApplicationDatabase.setData(_DB_CALIB_TUBE_FOCUS, (int) pc_selected_fuoco, DBase::_DB_FORCE_SGN);
    pGeneratore->setFuoco((Generatore::_FuocoSize_Enum) pc_selected_fuoco);
    pGeneratore->updateFuoco();

    // Filtro selezionato
    ApplicationDatabase.setData(_DB_CALIB_TUBE_FILTER,(int) pc_selected_filtro,DBase::_DB_FORCE_SGN);
    pCollimatore->manualFiltroCollimation = true;
    pCollimatore->manualFilter = pc_selected_filtro;
    pCollimatore->manualSetFiltro();

    // Impostazione kV
    ApplicationDatabase.setData(_DB_CALIB_TUBE_KV,(int) pc_selected_kV,DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_CALIB_TUBE_VDAC,(int) pc_selected_vdac,DBase::_DB_FORCE_SGN);

    // Impostazione Anodica
    ApplicationDatabase.setData(_DB_CALIB_TUBE_IA,(int) pc_selected_Ia,DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_CALIB_TUBE_IDAC,(int) pc_selected_Idac,DBase::_DB_FORCE_SGN);

    // Impostazione mAs
    ApplicationDatabase.setData(_DB_CALIB_TUBE_DMAS,(int) pc_selected_mAs*10,DBase::_DB_FORCE_SGN);

    // Imposta le condizioni di ready
    getProfileCalibrationReady(DBase::_DB_FORCE_SGN);
    return;

}

void AnalogCalibPageOpen::tubeValueChanged(int index,int opt)
{
     if((isMaster)&&(opt&DBase::_DB_ONLY_SLAVE_ACTION)) return;
    if((!isMaster)&&(opt&DBase::_DB_ONLY_MASTER_ACTION)) return;

    switch(index){

    case _DB_T_CUFFIA:
        if(!isMaster) return;
        pToConsole->notifyTubeTemp(ApplicationDatabase.getDataI(_DB_HU_ANODE), ApplicationDatabase.getDataI(_DB_T_CUFFIA));
        break;
    case _DB_HU_ANODE:
        if(!isMaster) return;
        pToConsole->notifyTubeTemp(ApplicationDatabase.getDataI(_DB_HU_ANODE), ApplicationDatabase.getDataI(_DB_T_CUFFIA));
        break;
    case _DB_XRAY_PUSH_BUTTON:
        if(!isMaster) return;

        if(!ApplicationDatabase.getDataU(_DB_XRAY_PUSH_BUTTON)){
            // Rilascio Pulsante raggi
            ApplicationDatabase.setData(_DB_XRAY_SYM,(unsigned char) 0, DBase::_DB_FORCE_SGN);
            io->setXrayLamp(false);
            return;
        }
        if(!pc_data_valid) return;

        // Segnale di attivazione pulsante raggi
        if(!ApplicationDatabase.getDataI(_DB_XRAYPUSH_READY)) return;
        pToConsole->activationXrayPush();
        startTubeCalibrationXraySequence();

        break;


    case _DB_AWS_CONNECTION:

        break;

    case _DB_CALIB_TUBE_NAME:
        ui->tubeName->setText(ApplicationDatabase.getDataS(index));
        if(ApplicationDatabase.getDataS(index).contains("TEMPLATE")) ui->readOnlyTubeFrame->show();
        else ui->readOnlyTubeFrame->hide();
        break;
    case _DB_CALIB_TUBE_FOCUS:
        if(ApplicationDatabase.getDataI(index)==Generatore::FUOCO_LARGE){
            ui->largeFocusFrame->show();
            ui->smallFocusFrame->hide();
        }else if(ApplicationDatabase.getDataI(index)==Generatore::FUOCO_SMALL){
            ui->largeFocusFrame->hide();
            ui->smallFocusFrame->show();
        }else{
            ui->largeFocusFrame->hide();
            ui->smallFocusFrame->hide();
        }

        break;
    case _DB_CALIB_TUBE_FILTER:
        if(ApplicationDatabase.getDataI(index)==Collimatore::FILTRO_ND){
            ui->tubeFilter->setText("---");
        }else if(ApplicationDatabase.getDataI(index)==Collimatore::FILTRO_Mo){
            ui->tubeFilter->setText("Mo");
        }else if(ApplicationDatabase.getDataI(index)==Collimatore::FILTRO_Rh){
            ui->tubeFilter->setText("Rh");
        }
        break;

    case _DB_CALIB_TUBE_KV:
        if(ApplicationDatabase.getDataI(index)==0){
            ui->tubekV->setText(QString("---"));
        }else {
            ui->tubekV->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        }
        break;

    case _DB_CALIB_TUBE_VDAC:
        if(ApplicationDatabase.getDataI(index)==0){
            ui->tubeVdac->setText(QString("---"));
        }else {
            ui->tubeVdac->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        }
        break;
    case _DB_CALIB_TUBE_IA:
        if(ApplicationDatabase.getDataI(index)==0){
            ui->tubeIa->setText(QString("---"));
        }else {
            ui->tubeIa->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        }
        break;
    case _DB_CALIB_TUBE_IDAC:
        if(ApplicationDatabase.getDataI(index)==0){
            ui->tubeIdac->setText(QString("---"));
        }else {
            ui->tubeIdac->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        }
        break;

    case _DB_CALIB_TUBE_DMAS:
        if(ApplicationDatabase.getDataI(index)==0){
            ui->tubemAs->setText(QString("---"));
        }else {
            ui->tubemAs->setText(QString("%1").arg(((float) ApplicationDatabase.getDataI(index))/10));
        }
        break;

    case _DB_CALIB_TUBE_DKVR:
        if(ApplicationDatabase.getDataI(index)==0){
            ui->kVRead->setText(QString("---"));
        }else {
            ui->kVRead->setText(QString("%1").arg(((float) ApplicationDatabase.getDataI(index))/10));
        }
        break;
    case _DB_CALIB_TUBE_IAR:
        if(ApplicationDatabase.getDataI(index)==0){
            ui->IaRead->setText(QString("---"));
        }else {
            ui->IaRead->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        }
        break;
    case _DB_CALIB_TUBE_DMASR:
        if(ApplicationDatabase.getDataI(index)==0){
            ui->mAsRead->setText(QString("---"));
        }else {
            ui->mAsRead->setText(QString("%1").arg(((float) ApplicationDatabase.getDataI(index))/10));
        }
        break;

    case _DB_CALIB_TUBE_READY_STAT:


        if(ApplicationDatabase.getDataI(index) & 0x4)  ui->warningPC->show();
        else ui->warningPC->hide();

        break;

    }

}


// Attivazione pre impulso di una sequenza di calibrazione del profilo
void AnalogCalibPageOpen::startTubeCalibrationXraySequence(void){

    unsigned char data[25];

    // Comunque effettua il refresh dello starter
    pGeneratore->refreshStarter();


    if(ApplicationDatabase.getDataU(_DB_EXPOSURE_MODE) == _EXPOSURE_MODE_CALIB_MODE_KERMA){

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
        data[7]|=4;        // Alta VelocitÃ
        pGeneratore->starterHS = true;

        data[8] =  0; // Tensione Griglia da aggiungere
        data[9] =  pGeneratore->maxV;
        data[10] = pGeneratore->minV;
        data[11] = pGeneratore->maxI;
        data[12] = pGeneratore->minI;

    }else{
        data[0] =  (unsigned char) (pc_selected_vdac&0x00FF);
        data[1] =  (unsigned char) (pc_selected_vdac>>8);
        data[2] =  (unsigned char) (pc_selected_Idac&0x00FF);
        data[3] =  (unsigned char) (pc_selected_Idac>>8);
        data[4] =  (unsigned char) ((pc_selected_mAs*50)&0x00FF);
        data[5] =  (unsigned char) ((pc_selected_mAs*50)>>8);
        data[6] =  40 | 0x80; // 200ms massimi per ogni impulso: usa il timer corto da 10ms
        data[6] =  pGeneratore->timeoutExp;


        // Switch Generatore + Alta Velocita
        unsigned char SWA = pGeneratore->tube[pc_selected_kV-_MIN_KV].vRef.SWA;
        unsigned char SWB = pGeneratore->tube[pc_selected_kV-_MIN_KV].vRef.SWB;
        data[7]=0;
        if(SWA) data[7]|=1;
        if(SWB) data[7]|=2;

        data[7]|=4; // Starter HS always ON
        pGeneratore->starterHS = true;

        // Tensione Griglia
        data[8] =  0;

        // Diagnostica Tensione / Corrente anodica (determinato dal breve Timeout utilizzato)
        data[9] =  255;
        data[10] = 0;
        data[11] = 255;
        data[12] = 0;

    }

    // No Sblocco compressore dopo esposizione
    data[13] = 0;

    // Aggiungo i valori nominali inviati al driver
    data[14] = (unsigned char) ((unsigned int) (pc_selected_kV * 10) & 0x00FF);
    data[15] = (unsigned char) ((unsigned int) (pc_selected_kV * 10) >> 8);
    data[16] = (unsigned char) ((unsigned int) (pc_selected_Ia * 10) & 0x00FF);
    data[17] = (unsigned char) ((unsigned int) (pc_selected_Ia * 10) >> 8);

    data[18] = 0; // Riservato per gli impulsi dell'esposimetro
    data[19] = 0;
    data[20] = 0; // 0=pre-pulse, 1 = Pulse

    // Prova ad inviare il comando
    if(pConsole->pGuiMcc->sendFrame(MCC_XRAY_ANALOG_CALIB_TUBE,1,data,21)==FALSE)
    {
        xrayErrorInCommand(ERROR_MCC_COMMAND);
        return;

    }


    ApplicationDatabase.setData(_DB_XRAY_SYM,(unsigned char) 1, DBase::_DB_FORCE_SGN);
    io->setXrayLamp(true);

}


void AnalogCalibPageOpen::tubeGuiNotify(unsigned char id, unsigned char mcccode, QByteArray data)
{
    int tpls,dmas,imean;
    unsigned char raw_vmean;
    float dvmean;

    switch(mcccode)
    {

    // Questa funzione viene utilizzata solo in caso di comunicazione di errori
    case MCC_XRAY_ANALOG_CALIB_TUBE: // Fine raggi
        stopAttesaDati();

        if(data.at(0)){
            // Esito negativo
            PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_RAGGI, data.at(0),TRUE); // Self resetting
            pToConsole->fineRaggiAnalogCalibTube(data.at(0),0,0,0);
        }else{

            // Acquisizione risultati dal buffer
            dmas = (int) (data.at(1)+256*data.at(2));
            raw_vmean = data.at(3);
            dvmean = (float) (data.at(4) + 256 * data.at(5));
            tpls = (int) (data.at(6)+256*data.at(7));

            // Calcolo corrente media sulla durata dell'impulso
            if(tpls!=0) imean = dmas * 100 / tpls;
            else imean=0;

            // Aggiornamento dell'interfaccia con i dati dell'esposizione
            ApplicationDatabase.setData(_DB_CALIB_TUBE_DKVR, (int) (dvmean));
            ApplicationDatabase.setData(_DB_CALIB_TUBE_IAR, (int) (imean));
            ApplicationDatabase.setData(_DB_CALIB_TUBE_DMASR, (int) (dmas));

            // Comunica i dati al software Ultra:
            pToConsole->fineRaggiAnalogCalibTube(0,raw_vmean, (int) (dvmean), imean*10);
        }

        // Rilascio Pulsante raggi
        ApplicationDatabase.setData(_DB_XRAY_SYM,(unsigned char) 0, DBase::_DB_FORCE_SGN);
        io->setXrayLamp(false);

    break;


    }

}

void AnalogCalibPageOpen::selectTube(QString tube_name){
    ApplicationDatabase.setData(_DB_CALIB_TUBE_NAME,tube_name,0);
}
