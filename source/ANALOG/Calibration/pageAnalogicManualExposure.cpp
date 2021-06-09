#include "../analog.h"
#include "pageCalibAnalogic.h"
#include "analog_calib.h"
#include "ui_analog_calib.h"
#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"



//____________________________________________________________________
//  FUNZIONE CHIAMATA PERIODIAMENTE PER TESTARE LO STATO DI READY
//
bool AnalogCalibPageOpen::getManualExposureReady(unsigned char opt){
    if(ApplicationDatabase.getDataI(_DB_INVALID_FRAME)) return false;
    return true;
}

void AnalogCalibPageOpen::exitManualExposure(void){
    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(manualExposureValueChanged(int,int)));
    disconnect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(manualExposureGuiNotify(unsigned char,unsigned char,QByteArray)));


    disconnect(ui->manualFieldSelection,SIGNAL(released()),this,SLOT(onManualNextCampiButton()));
    disconnect(ui->manualFocus,SIGNAL(released()),this,SLOT(onManualFuoco()));
    disconnect(ui->manualFilter,SIGNAL(selectionChanged()),this,SLOT(onManualFilter()));

    // Elimina il calcolatore
    disconnect(pCalculator);
    pCalculator->deleteLater(); // importante !!!
    pCalculator = 0;

    manual_exposure = false;
}

void AnalogCalibPageOpen::updateExposureData(int opt){
    if(!isMaster) return;

    int errcode = pGeneratore->validateAnalogData(ANALOG_TECH_MODE_MANUAL,true, false);
    if(errcode==0){
        ApplicationDatabase.setData(_DB_INVALID_FRAME, (int)  0, opt);
        ApplicationDatabase.setData(_DB_MANUAL_VDAC, (int)  pGeneratore->selectedVdac, opt);
        ApplicationDatabase.setData(_DB_MANUAL_IDAC, (int)  pGeneratore->selectedIdac, opt);
        ApplicationDatabase.setData(_DB_MANUAL_IN, (int)  pGeneratore->selectedIn, opt);

    } else{
        ApplicationDatabase.setData(_DB_INVALID_FRAME, (int)  1, opt);

        // Errore impostazione valori
        ApplicationDatabase.setData(_DB_MANUAL_VDAC, (int)  0, opt);
        ApplicationDatabase.setData(_DB_MANUAL_IDAC, (int)  0, opt);
        ApplicationDatabase.setData(_DB_MANUAL_IN, (int)  0, opt);
    }

    // Impostazione limiti relativi a kV e mAs sulla base dei KV, MAS  FUOCO e TUBO
    int maxKV  = pGeneratore->max_selectable_kv;
    int maxMas = pGeneratore->getMaxDMas();

    ApplicationDatabase.setData(_DB_MANUAL_MAX_KV, (int)  maxKV, opt);
    ApplicationDatabase.setData(_DB_MANUAL_MAX_DMAS, (int)  maxMas, opt);

}

void AnalogCalibPageOpen::initManualExposure(void){

    manual_exposure = true;

    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(manualExposureValueChanged(int,int)),Qt::UniqueConnection);
    connect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(manualExposureGuiNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

    connect(ui->manualFieldSelection,SIGNAL(released()),this,SLOT(onManualNextCampiButton()),Qt::UniqueConnection);
    connect(ui->manualFocus,SIGNAL(released()),this,SLOT(onManualFuoco()),Qt::UniqueConnection);
    connect(ui->manualFilter,SIGNAL(selectionChanged()),this,SLOT(onManualFilter()),Qt::UniqueConnection);
    connect(ui->manualKV,SIGNAL(selectionChanged()),this,SLOT(onManualKV()),Qt::UniqueConnection);
    connect(ui->manualMAS,SIGNAL(selectionChanged()),this,SLOT(onManualMAS()),Qt::UniqueConnection);

    pCalculator = new numericPad(rotview,view, parent);
    connect(pCalculator,SIGNAL(calcSgn(bool)),this,SLOT(manualCalculatorSlot(bool)));

    // Impostazione geometria del frame di lavoro
    ui->manualPanelFrame->show();
    ui->manualPanelFrame->setGeometry(160,60,626,351);

    ui->intestazione->setText(QString("MANUAL EXPOSURE PANEL"));
    ui->warningPC->hide();

    if(ApplicationDatabase.getDataU(_DB_AWS_CONNECTION)){
        ui->pcConnectedFrame->show();    
    }else{
        ui->pcConnectedFrame->hide();    
    }

    if(!isMaster) return;

    ApplicationDatabase.setData(_DB_MANUAL_PRIMO_FILTRO, (int)  pConfig->analogCnf.primo_filtro, DBase::_DB_FORCE_SGN | DBase::_DB_NO_ACTION);
    ApplicationDatabase.setData(_DB_MANUAL_SECONDO_FILTRO, (int)  pConfig->analogCnf.secondo_filtro, DBase::_DB_FORCE_SGN | DBase::_DB_NO_ACTION);

    // Imposta le condizioni di ready
    getManualExposureReady(DBase::_DB_FORCE_SGN);


    ApplicationDatabase.setData(_DB_MANUAL_CAMPO, (int)  _ANALOG_DETECTOR_FRONT_FIELD, DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_MANUAL_KV, (int)  25, DBase::_DB_FORCE_SGN | DBase::_DB_NO_ACTION);
    ApplicationDatabase.setData(_DB_MANUAL_MAS, (int)  50, DBase::_DB_FORCE_SGN| DBase::_DB_NO_ACTION);
    ApplicationDatabase.setData(_DB_MANUAL_FOCUS, (int)  Generatore::FUOCO_LARGE, DBase::_DB_FORCE_SGN| DBase::_DB_NO_ACTION);
    ApplicationDatabase.setData(_DB_MANUAL_FILTER, (int)  pConfig->analogCnf.primo_filtro, DBase::_DB_FORCE_SGN);

    ApplicationDatabase.setData(_DB_MANUAL_RAD, (int)  0, DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_MANUAL_PLOG, (int)  0, DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_MANUAL_PULSE, (int)  0, DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_MANUAL_KVREAD, (int)  0, DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_MANUAL_MASREAD, (int)  0, DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_MANUAL_IAREAD, (int)  0, DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_MANUAL_PULSE_TIME, (int)  0, DBase::_DB_FORCE_SGN);

    ApplicationDatabase.setData(_DB_MANUAL_KERMA, (int)  0, DBase::_DB_FORCE_SGN);



    // Impostazione Fuoco grande
    pGeneratore->setkV(25);
    pGeneratore->setmAs(50);
    updateExposureData(DBase::_DB_FORCE_SGN|DBase::_DB_NO_ACTION);

    // Impostazione collimazione aperta
    pCollimatore->manualCollimation = true;
    pCollimatore->manualF = 0;
    pCollimatore->manualB = 0;
    pCollimatore->manualL = 0;
    pCollimatore->manualR = 0;
    pCollimatore->manualColliUpdate();

    // Impostazione filtro
    pCollimatore->manualFiltroCollimation = true;
    pCollimatore->manualFilter = pConfig->analogCnf.primo_filtro;
    pCollimatore->manualSetFiltro();



}



void AnalogCalibPageOpen::manualExposureValueChanged(int index,int opt)
{


    if((isMaster)&&(opt&DBase::_DB_ONLY_SLAVE_ACTION)) return;
    if((!isMaster)&&(opt&DBase::_DB_ONLY_MASTER_ACTION)) return;

    switch(index){

    case _DB_XRAY_PUSH_BUTTON:

        if(!isMaster) return;

        if(!ApplicationDatabase.getDataU(_DB_XRAY_PUSH_BUTTON)){
            // Rilascio Pulsante raggi
            ApplicationDatabase.setData(_DB_XRAY_SYM,(unsigned char) 0, DBase::_DB_FORCE_SGN);
            io->setXrayLamp(false);
            return;
        }


        // Segnale di attivazione pulsante raggi
        if(!ApplicationDatabase.getDataI(_DB_XRAYPUSH_READY)) return;
        pToConsole->activationXrayPush();
        startManualExposureXraySequence();
        break;


    case _DB_INVALID_FRAME:
        if(ApplicationDatabase.getDataI(index))  ui->manualInvalidFrame->show();
        else  ui->manualInvalidFrame->hide();
        break;

    case _DB_MANUAL_MAX_KV:
        if(!ApplicationDatabase.getDataI(index)) ui->manualMaxKV->setText(QString("kV max: 20"));
        else ui->manualMaxKV->setText(QString("kV max: %1").arg(ApplicationDatabase.getDataI(index)));
        break;

    case _DB_MANUAL_MAX_DMAS:
        if(!ApplicationDatabase.getDataI(index)) ui->manualMaxMAS->setText(QString("mAs max: 1"));
        else ui->manualMaxMAS->setText(QString("mAs max: %1").arg( ApplicationDatabase.getDataI(index)/10));
        break;

    case _DB_MANUAL_VDAC:
        if(!ApplicationDatabase.getDataI(index)) ui->manualKVdac->setText(QString("----"));
        else ui->manualKVdac->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        break;
    case _DB_MANUAL_IDAC:
        if(!ApplicationDatabase.getDataI(index)) ui->manualIdac->setText(QString("----"));
        else ui->manualIdac->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        break;
    case _DB_MANUAL_IN:
        if(!ApplicationDatabase.getDataI(index)) ui->manualIaN->setText(QString("----"));
        else ui->manualIaN->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        break;

    case _DB_MANUAL_CAMPO: // Impostazione del campo detector in utilizzo

        switch(ApplicationDatabase.getDataI(index)){
        case ANALOG_AECFIELD_FRONT:
            ui->manualFieldSelection->setStyleSheet(QString("border-image: url(:/Sym/Sym/campi_front.png); background-color: rgb(255, 255, 255,0);"));
            break;
        case ANALOG_AECFIELD_CENTER:
            ui->manualFieldSelection->setStyleSheet(QString("border-image: url(:/Sym/Sym/campi_center.png); background-color: rgb(255, 255, 255,0);"));
            break;
        case ANALOG_AECFIELD_BACK:
            ui->manualFieldSelection->setStyleSheet(QString("border-image: url(:/Sym/Sym/campi_back.png); background-color: rgb(255, 255, 255,0);"));
            break;
        default:
            return;
        }

        // Imposta sull'esposimetro il campo effettivo
        if(isMaster) pPotter->setDetectorField(ApplicationDatabase.getDataI(index));
        break;

    case _DB_MANUAL_KV:
        if(!ApplicationDatabase.getDataI(index)) ui->manualKV->setText(QString("----"));
        else ui->manualKV->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));

        if((isMaster) && (!(opt & DBase::_DB_NO_ACTION))){
            pGeneratore->setkV((float) ApplicationDatabase.getDataI(index));
            updateExposureData(0);

            // Corregge i mAs se dovessere avere un limite inferiore rispetto ai nuovi kV
            if(ApplicationDatabase.getDataI(_DB_MANUAL_MAS) > (ApplicationDatabase.getDataI(_DB_MANUAL_MAX_DMAS)/10)){
                ApplicationDatabase.setData(_DB_MANUAL_MAS, (int) (ApplicationDatabase.getDataI(_DB_MANUAL_MAX_DMAS)/10),DBase::_DB_NO_ACTION);
            }
        }

        break;
    case _DB_MANUAL_MAS:
        if(!ApplicationDatabase.getDataI(index)) ui->manualMAS->setText(QString("----"));
        else ui->manualMAS->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        if((isMaster) && (!(opt & DBase::_DB_NO_ACTION))){
            pGeneratore->setmAs(ApplicationDatabase.getDataI(index));
            updateExposureData(0);
        }
        break;
    case _DB_MANUAL_FOCUS:
        if(ApplicationDatabase.getDataI(index) == Generatore::FUOCO_LARGE)
            ui->manualFocus->setStyleSheet(QString("border-image: url(:/Sym/Sym/LargeFocus.png); "));
        else
            ui->manualFocus->setStyleSheet(QString("border-image: url(:/Sym/Sym/SmallFocus.png); "));

        if((isMaster) && (!(opt & DBase::_DB_NO_ACTION))){
            pGeneratore->setFuoco((Generatore::_FuocoSize_Enum) ApplicationDatabase.getDataI(index));
            pGeneratore->updateFuoco();
            updateExposureData(0);
        }
        break;

    case _DB_MANUAL_FILTER:
        if(ApplicationDatabase.getDataI(index) == Collimatore::FILTRO_Mo) ui->manualFilter->setText(QString("Mo"));
        else ui->manualFilter->setText(QString("Rh"));

        if((isMaster) && (!(opt & DBase::_DB_NO_ACTION))){
            pCollimatore->manualFilter = ApplicationDatabase.getDataI(index);
            pCollimatore->manualSetFiltro();
        }
        break;

    case _DB_MANUAL_RAD:
        if(!ApplicationDatabase.getDataI(index)) ui->manualRad->setText(QString("----"));
        else ui->manualRad->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        break;
    case _DB_MANUAL_PLOG:
        if(!ApplicationDatabase.getDataI(index)) ui->manualPlog->setText(QString("----"));
        else ui->manualPlog->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        break;
    case _DB_MANUAL_PULSE:
        if(!ApplicationDatabase.getDataI(index)) ui->manualPulse->setText(QString("----"));
        else ui->manualPulse->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        break;
    case _DB_MANUAL_KVREAD:        
        if(!ApplicationDatabase.getDataI(index)) ui->manualKVREAD->setText(QString("----"));
        else ui->manualKVREAD->setText(QString("%1").arg((float) ApplicationDatabase.getDataI(index)/10));
        break;
    case _DB_MANUAL_MASREAD:
        if(!ApplicationDatabase.getDataI(index)) ui->manualMASRead->setText(QString("----"));
        else ui->manualMASRead->setText(QString("%1").arg((float) ApplicationDatabase.getDataI(index)/10));
        break;
    case _DB_MANUAL_IAREAD:
        if(!ApplicationDatabase.getDataI(index)) ui->manualIaRead->setText(QString("----"));
        else ui->manualIaRead->setText(QString("%1").arg((float) ApplicationDatabase.getDataI(index)/10));
        break;
    case _DB_MANUAL_PULSE_TIME:
        if(!ApplicationDatabase.getDataI(index)) ui->manualPulseTime->setText(QString("----"));
        else ui->manualPulseTime->setText(QString("%1").arg((int) ApplicationDatabase.getDataI(index)));
        break;

    case _DB_MANUAL_KERMA:
        if(ApplicationDatabase.getDataI(index)<0) ui->manualKerma->setText(QString("MISSING PAD!"));
        else if(ApplicationDatabase.getDataI(index)==0) ui->manualKerma->setText(QString("----"));
        else ui->manualKerma->setText(QString("%1 (uG)").arg((int) ApplicationDatabase.getDataI(index)));
        break;

    case _DB_AWS_CONNECTION:
        if(ApplicationDatabase.getDataU(_DB_AWS_CONNECTION)){
            ui->pcConnectedFrame->show();            
        }else{
            ui->pcConnectedFrame->hide();           
        }

        break;
    case _DB_MANUAL_READY_STAT:

        break;

    }

}


void AnalogCalibPageOpen::onManualNextCampiButton(void){
    int ncampi = ApplicationDatabase.getDataI(_DB_MANUAL_CAMPO);
    ncampi++;
    if(ncampi>2) ncampi=0;
    ApplicationDatabase.setData(_DB_MANUAL_CAMPO, (int) ncampi);

}

void AnalogCalibPageOpen::onManualFuoco(void){
    int fuoco = ApplicationDatabase.getDataI(_DB_MANUAL_FOCUS);
    if(fuoco==Generatore::FUOCO_LARGE) ApplicationDatabase.setData(_DB_MANUAL_FOCUS, (int) Generatore::FUOCO_SMALL);
    else ApplicationDatabase.setData(_DB_MANUAL_FOCUS, (int) Generatore::FUOCO_LARGE);


}

// Azione sulla pressione del campo Filtro
void AnalogCalibPageOpen::onManualFilter(void){
    int primo_filtro = ApplicationDatabase.getDataI(_DB_MANUAL_PRIMO_FILTRO);
    int secondo_filtro = ApplicationDatabase.getDataI(_DB_MANUAL_SECONDO_FILTRO);
    int filtro = ApplicationDatabase.getDataI(_DB_MANUAL_FILTER);

    // Se è configurato un solo filtro allora non effettua il cambio
    if(secondo_filtro == Collimatore::FILTRO_ND) return;
    if(secondo_filtro == primo_filtro) return;

    if(filtro == secondo_filtro) ApplicationDatabase.setData(_DB_MANUAL_FILTER, (int) primo_filtro);
    else ApplicationDatabase.setData(_DB_MANUAL_FILTER, (int) secondo_filtro);

    // Sposta il focus per evitare il cursore
    ui->manualPanelFrame->setFocus();
}

// Azione sulla pressione del campo Filtro
void AnalogCalibPageOpen::onManualKV(void){

    dataField="";
    pCalculator->activate(&dataField, QString("MANUAL KV SETTING"), 0);

    // Sposta il focus per evitare il cursore
    ui->manualPanelFrame->setFocus();
}

// Azione sulla pressione del campo Filtro
void AnalogCalibPageOpen::onManualMAS(void){

    dataField="";
    pCalculator->activate(&dataField, QString("MANUAL MAS SETTING"), 1);

    // Sposta il focus per evitare il cursore
    ui->manualPanelFrame->setFocus();
}

void AnalogCalibPageOpen::manualCalculatorSlot(bool stat){
    if(!stat) return;

    int maxKV = ApplicationDatabase.getDataI(_DB_MANUAL_MAX_KV);
    int maxDMAS = ApplicationDatabase.getDataI(_DB_MANUAL_MAX_DMAS);

    PRINT(QString("MAX KV:%1").arg((int) maxKV));
    PRINT(QString("MAX DMAS:%1").arg((int) maxDMAS));

    if(pCalculator->activation_code == 0) {
        int kv = dataField.toInt();
        if(kv > maxKV) kv = maxKV;
        else if(kv<20) kv = 20;
        ApplicationDatabase.setData(_DB_MANUAL_KV, kv);
    } else {
        // Impostazione mAs
        int dmas = dataField.toInt() * 10;
        if(dmas > maxDMAS) dmas = maxDMAS;
        else if(dmas<10) maxDMAS = 10;
        ApplicationDatabase.setData(_DB_MANUAL_MAS, dmas/10);
    }

}


// Attivazione raggi in modalità Manuale.
// I dati di esposizione sono già tutti stati caricati
void AnalogCalibPageOpen::startManualExposureXraySequence(void){
    unsigned char data[18];


    // Impostazione dati di esposizione
    unsigned char errcode = pGeneratore->validateAnalogData(ANALOG_TECH_MODE_MANUAL, true, false);
    if(errcode){
        xrayErrorInCommand(errcode);
        return;
    }

    // Comunque effettua il refresh dello starter
    pGeneratore->refreshStarter();

    data[0] =  (unsigned char) (pGeneratore->selectedVdac&0x00FF);
    data[1] =  (unsigned char) (pGeneratore->selectedVdac>>8);
    data[2] =  (unsigned char) (pGeneratore->selectedIdac&0x00FF);
    data[3] =  (unsigned char) (pGeneratore->selectedIdac>>8);
    data[4] =  (unsigned char) (pGeneratore->selectedmAsDac&0x00FF);
    data[5] =  (unsigned char) (pGeneratore->selectedmAsDac>>8);
    data[6] =  pGeneratore->timeoutExp;

    // In manuale sempre alta Velocita (se presente)
    data[7]=0;
    if(pGeneratore->SWA) data[7]|=1;
    if(pGeneratore->SWB) data[7]|=2;

    // Alta velocità
    data[7]|=4;
    pGeneratore->starterHS = true;

    // Tensione Griglia
    data[8] =  0;

    // Diagnostica Tensione / Corrente anodica
    data[9] =  pGeneratore->maxV;
    data[10] = pGeneratore->minV;
    data[11] = pGeneratore->maxI;
    data[12] = pGeneratore->minI;

    // No Sblocco compressore dopo esposizione
    data[13] = 0;

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


void AnalogCalibPageOpen::manualExposureGuiNotify(unsigned char id, unsigned char mcccode, QByteArray data)
{

    switch(mcccode)
    {

    case MCC_XRAY_ANALOG_MANUAL:
        stopAttesaDati();
        if(data.at(0)){

            // Esito negativo
            PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_RAGGI, data.at(0),TRUE); // Self resetting

            ApplicationDatabase.setData(_DB_MANUAL_MASREAD,(int) 0,0);
            ApplicationDatabase.setData(_DB_MANUAL_PULSE,(int) 0,0);
            ApplicationDatabase.setData(_DB_MANUAL_PLOG,(int) 0,0);
            ApplicationDatabase.setData(_DB_MANUAL_RAD,(int) 0,0);
            ApplicationDatabase.setData(_DB_MANUAL_IAREAD,(int) 0,0);
            ApplicationDatabase.setData(_DB_MANUAL_KVREAD,(int) 0);
            ApplicationDatabase.setData(_DB_MANUAL_PULSE_TIME,(int) 0);
            ApplicationDatabase.setData(_DB_MANUAL_KERMA,(int) 0);

        }else{
            int dvmean = ((int) data[13]+ 256 * (int) data[14]);
            int dmas = ((int) data[1]+ 256 * (int) data[2]);
            int tpulse = ((int) data[17]+ 256 * (int) data[18]);
            float dimean = dmas * 1000 / tpulse;

            // Calcolo del KERMA al livello di 58mm dal piano del potter
            float ugKermaNominale = pGeneratore->pDose->getConvertedUgKerma(ApplicationDatabase.getDataI(_DB_MANUAL_KV)*10,ApplicationDatabase.getDataI(_DB_MANUAL_MAS)*10, 58, ApplicationDatabase.getDataI(_DB_MANUAL_FILTER));

            ApplicationDatabase.setData(_DB_MANUAL_KVREAD,(int) (dvmean),0);
            ApplicationDatabase.setData(_DB_MANUAL_PULSE_TIME,(int) (tpulse),0);
            ApplicationDatabase.setData(_DB_MANUAL_IAREAD,(int) dimean,0);
            ApplicationDatabase.setData(_DB_MANUAL_MASREAD,(int) dmas);
            ApplicationDatabase.setData(_DB_MANUAL_PULSE,(int) ((int) data[3]+ 256 * (int) data[4]) );
            ApplicationDatabase.setData(_DB_MANUAL_PLOG,(int) ((int) data[5]+ 256 * (int) data[6])  );
            ApplicationDatabase.setData(_DB_MANUAL_RAD,(int) ((int) data[7]+ 256 * (int) data[8])  );

            if(!pCompressore->isValidPad()) ApplicationDatabase.setData(_DB_MANUAL_KERMA,(int) -1);
            else ApplicationDatabase.setData(_DB_MANUAL_KERMA,(int) ugKermaNominale);
        }

        // Rilascio Pulsante raggi
        ApplicationDatabase.setData(_DB_XRAY_SYM,(unsigned char) 0, DBase::_DB_FORCE_SGN);
        io->setXrayLamp(false);

        // Notifica a PC se necessario
        if(!ApplicationDatabase.getDataU(_DB_AWS_CONNECTION)) break;
        pToConsole->fineRaggiAnalogCalibDetector(data);

        break;

    }
}

