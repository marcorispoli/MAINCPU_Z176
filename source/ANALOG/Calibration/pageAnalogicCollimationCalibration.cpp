#include "../analog.h"
#include "pageCalibAnalogic.h"
#include "analog_calib.h"
#include "ui_analog_calib.h"
#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"

#include "../../audio.h"
#include "../../../lib/msgbox.h"

extern audio* pAudio;

//____________________________________________________________________
//  FUNZIONE CHIAMATA PERIODIAMENTE PER TESTARE LO STATO DI READY
//
bool AnalogCalibPageOpen::getCollimationCalibrationReady(unsigned char opt){

    //int ready_stat=0;
    if(currentFormat == _DEF_COLLI_CALIB_FORMAT_MIRROR)  {

        return false;
    }

    if(currentFormat == _DEF_COLLI_CALIB_FORMAT_FACTORY)  {

        return false;
    }

    if(colliChanged) return false;
    //ApplicationDatabase.setData(_DB_READY_STAT,ready_stat,opt);

    // if(ready_stat) return false;
    return true;
}

void AnalogCalibPageOpen::exitCollimationCalibration(void){
    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(collimationValueChanged(int,int)));
    disconnect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(collimationGuiNotify(unsigned char,unsigned char,QByteArray)));
    disconnect(ui->buttonLeftList,SIGNAL(released()),this,SLOT(onLeftList()));
    disconnect(ui->buttonRightList,SIGNAL(released()),this,SLOT(onRightList()));
    disconnect(ui->downloadFormat,SIGNAL(released()),this,SLOT(onDownloadFormat()));
    disconnect(ui->downloadMirror,SIGNAL(released()),this,SLOT(onDownloadMirror()));
    disconnect(ui->frontEdit,SIGNAL(selectionChanged()),this,SLOT(onFrontEdit()));
    disconnect(ui->backEdit,SIGNAL(selectionChanged()),this,SLOT(onBackEdit()));
    disconnect(ui->leftEdit,SIGNAL(selectionChanged()),this,SLOT(onLeftEdit()));
    disconnect(ui->rightEdit,SIGNAL(selectionChanged()),this,SLOT(onRightEdit()));
    disconnect(ui->mirrorEdit,SIGNAL(selectionChanged()),this,SLOT(onMirrorEdit()));

    // Elimina il calcolatore
    disconnect(pCalculator);
    pCalculator->deleteLater(); // importante !!!
    pCalculator = 0;
    collimation_calibration = false;
    if(!isMaster) return;

    // Salva la configurazione in uscita
    if(colliConfChanged) storeColliFormat();


    // Reimposta la collimazione in automatico ed effettua l'update
    pCollimatore->manualCollimation = false;
    pCollimatore->updateColli();

    // Resetta il filtro in automatico
    pCollimatore->manualFiltroCollimation = false;

}

void AnalogCalibPageOpen::initCollimationCalibration(void){

    collimation_calibration = true;

    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(collimationValueChanged(int,int)),Qt::UniqueConnection);
    connect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(collimationGuiNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
    connect(ui->buttonLeftList,SIGNAL(released()),this,SLOT(onLeftList()),Qt::UniqueConnection);
    connect(ui->buttonRightList,SIGNAL(released()),this,SLOT(onRightList()),Qt::UniqueConnection);
    connect(ui->downloadFormat,SIGNAL(released()),this,SLOT(onDownloadFormat()),Qt::UniqueConnection);
    connect(ui->downloadMirror,SIGNAL(released()),this,SLOT(onDownloadMirror()),Qt::UniqueConnection);
    connect(ui->frontEdit,SIGNAL(selectionChanged()),this,SLOT(onFrontEdit()),Qt::UniqueConnection);
    connect(ui->backEdit,SIGNAL(selectionChanged()),this,SLOT(onBackEdit()),Qt::UniqueConnection);
    connect(ui->leftEdit,SIGNAL(selectionChanged()),this,SLOT(onLeftEdit()),Qt::UniqueConnection);
    connect(ui->rightEdit,SIGNAL(selectionChanged()),this,SLOT(onRightEdit()),Qt::UniqueConnection);
    connect(ui->mirrorEdit,SIGNAL(selectionChanged()),this,SLOT(onMirrorEdit()),Qt::UniqueConnection);

    pCalculator = new numericPad(rotview,view, parent);
    connect(pCalculator,SIGNAL(calcSgn(bool)),this,SLOT(colliCalculatorSlot(bool)));

    // Impostazione geometria del frame di lavoro
    ui->collimationPanelFrame->show();
    ui->collimationPanelFrame->setGeometry(160,60,516,361);
    ui->frameCollimation->setGeometry(10,75,496,281);
    ui->frameMirror->setGeometry(10,75,496,281);

    ui->pcConnectedFrame->hide();
    ui->intestazione->setText(QString("COLLIMATION PANEL"));
    ui->warningPC->hide();
    currentFormat =0;
    if(!isMaster) return;

    initColliData();

    ApplicationDatabase.setData(_DB_COLLI_CALIB_SEL_PAD,(int) currentFormat, DBase::_DB_FORCE_SGN);
    selectCollimationPattern(currentFormat, DBase::_DB_FORCE_SGN);
    colliChanged = false;
    colliConfChanged = false;

    // Impostazione collimazione Manuale
    pCollimatore->manualCollimation = true;
    pCollimatore->manualF = formatCurrentData[currentFormat].F;
    pCollimatore->manualB = formatCurrentData[currentFormat].B;
    pCollimatore->manualL = formatCurrentData[currentFormat].L;
    pCollimatore->manualR = formatCurrentData[currentFormat].R;
    pCollimatore->manualColliUpdate();

    // Impostazione filtro
    pCollimatore->manualFiltroCollimation = true;
    pCollimatore->manualFilter = pConfig->analogCnf.primo_filtro;
    pCollimatore->manualSetFiltro();


    // Imposta le condizioni di ready
    getCollimationCalibrationReady(DBase::_DB_FORCE_SGN);

    pSysLog->log("SERVICE PANEL: ANALOGIC COLLIMATION CALIBRATION");


}



void AnalogCalibPageOpen::collimationValueChanged(int index,int opt)
{
    int val;

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
        if(!ApplicationDatabase.getDataI(_DB_XRAYPUSH_READY)){          
            return;
        }
        pToConsole->activationXrayPush();
        startCollimationCalibrationXraySequence();
        break;


    case _DB_COLLI_CALIB_SEL_PAD: // Shift della lesta dei formati e relativa selezione
        currentFormat = ApplicationDatabase.getDataI(index);
        ui->compressorList->setText(formatList.at(currentFormat));
        ui->collimationPanelFrame->setFocus();
        // Se c'è selezionato il MIRROR non deve passare di qui

        if(currentFormat == _DEF_COLLI_CALIB_FORMAT_MIRROR)  {
            ui->mirrorEdit->setText(QString("%1").arg(ApplicationDatabase.getDataI(_DB_COLLI_CALIB_MIRROR)));
            ui->frameMirror->show();
            ui->frameCollimation->hide();
            ui->downloadMirror->show();
            ui->downloadFormat->hide();
            return;
        }else  if(currentFormat == _DEF_COLLI_CALIB_FORMAT_FACTORY)  {
            ui->frameMirror->hide();
            ui->frameCollimation->hide();
            ui->downloadMirror->hide();
            ui->downloadFormat->show();
            return;
        }

        // Finestra di selezione formato
        ui->frameMirror->hide();
        ui->frameCollimation->show();
        ui->downloadMirror->hide();
        ui->downloadFormat->show();

        if(isMaster) selectCollimationPattern(currentFormat,0);
        break;

    case _DB_COLLI_CALIB_FRONT: // Valore della lama frontale
        val = ApplicationDatabase.getDataI(index);
        ui->frontEdit->setText(QString("%1").arg(val));
        if(!isMaster) return;
        colliChanged = true;
        break;

    case _DB_COLLI_CALIB_BACK: // Valore della lama posteriore
        val = ApplicationDatabase.getDataI(index);
        ui->backEdit->setText(QString("%1").arg(val));
        if(!isMaster) return;
        colliChanged = true;
        break;

    case _DB_COLLI_CALIB_LEFT: // Valore della lama sinistra
        val = ApplicationDatabase.getDataI(index);
        ui->leftEdit->setText(QString("%1").arg(val));
        if(!isMaster) return;
        colliChanged = true;
        break;

    case _DB_COLLI_CALIB_RIGHT: // Valore della lama destra
        val = ApplicationDatabase.getDataI(index);
        ui->rightEdit->setText(QString("%1").arg(val));
        if(!isMaster) return;
        colliChanged = true;
        break;
    case _DB_COLLI_CALIB_MIRROR:
        val = ApplicationDatabase.getDataI(index);
        ui->mirrorEdit->setText(QString("%1").arg(val));
        if(!isMaster) return;
        break;

    case _DB_COLLI_DOWNLOAD:
        if(!isMaster) return;
        downloadColliFormat();
        break;
    case _DB_COLLI_FACTORY_DOWNLOAD:
        if(!isMaster) return;
        downloadFactoryColliFormat();
        break;
    case _DB_COLLI_MIRROR_DOWNLOAD:
        if(!isMaster) return;
        downloadColliMirror();
        break;
    }

}


// Attivazione funzione raggi per calibrazione detector
void AnalogCalibPageOpen::startCollimationCalibrationXraySequence(void){
    unsigned char data[18];


    // Impostazione dati di esposizione
    unsigned char errcode = pGeneratore->validateAnalogData(ANALOG_TECH_MODE_MANUAL,true, false);
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

    // Sempre alta Velocita (se presente)
    data[7]=0;
    if(pGeneratore->SWA) data[7]|=1;
    if(pGeneratore->SWB) data[7]|=2;

    // Alta velocità
    data[7]|=4;
    pGeneratore->starterHS = TRUE;

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

void AnalogCalibPageOpen::collimationGuiNotify(unsigned char id, unsigned char mcccode, QByteArray data)
{

    switch(mcccode)
    {

    case MCC_XRAY_ANALOG_MANUAL:
        stopAttesaDati();
        if(data.at(0)){

            // Esito negativo
            PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_RAGGI, data.at(0),TRUE); // Self resetting

        }else{

        }

        // Rilascio Pulsante raggi
        ApplicationDatabase.setData(_DB_XRAY_SYM,(unsigned char) 0, DBase::_DB_FORCE_SGN);
        io->setXrayLamp(false);

        break;
    }
}


// ___________________________________________________________________________________________
// Lista compressori


void AnalogCalibPageOpen::onRightList(void){
    int val = ApplicationDatabase.getDataI(_DB_COLLI_CALIB_SEL_PAD);
    val++; if(val>=formatList.size()) val=0;
    ApplicationDatabase.setData(_DB_COLLI_CALIB_SEL_PAD,val);

}
void AnalogCalibPageOpen::onLeftList(void){
    int val = ApplicationDatabase.getDataI(_DB_COLLI_CALIB_SEL_PAD);
    val--; if(val<0) val=formatList.size()-1;
    ApplicationDatabase.setData(_DB_COLLI_CALIB_SEL_PAD,val);
}

void AnalogCalibPageOpen::selectCollimationPattern(int val, int opt){
    if(!isMaster) return;
    if(formatList.size()<=val) return ;


    // Se si seleziona il reference, il reference viene aggiornato con i dati
    // del formato associato al reference
    if(val == _DEF_COLLI_CALIB_FORMAT_REFERENCE){
        formatCurrentData[val] = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_REF_ITEM];
    }


    // Impostazione dei campi lame
    ApplicationDatabase.setData(_DB_COLLI_CALIB_FRONT,(int) formatCurrentData[val].F,opt);
    ApplicationDatabase.setData(_DB_COLLI_CALIB_BACK,(int) formatCurrentData[val].B,opt);
    ApplicationDatabase.setData(_DB_COLLI_CALIB_LEFT,(int) formatCurrentData[val].L,opt);
    ApplicationDatabase.setData(_DB_COLLI_CALIB_RIGHT,(int) formatCurrentData[val].R,opt);

    // Comunque al cambio pattern occorre richiedere un downoad per fare raggi
    colliChanged = true;

    // Selezione del fuoco corrente
    if(val == _DEF_COLLI_CALIB_FORMAT_REFERENCE){
        if(_DEF_COLLI_CALIB_FORMAT_REF_ITEM == _DEF_COLLI_CALIB_FORMAT_MAG){
            pGeneratore->setkV((float) KV_COLLIMATION_SMALL);
            pGeneratore->setmAs((float) mAs_COLLIMATION_SMALL);
            pGeneratore->setFuoco((Generatore::_FuocoSize_Enum) Generatore::FUOCO_SMALL);
            pGeneratore->updateFuoco();
        }else{
            pGeneratore->setkV((float) KV_COLLIMATION_LARGE);
            pGeneratore->setmAs((float) mAs_COLLIMATION_LARGE);
            pGeneratore->setFuoco((Generatore::_FuocoSize_Enum) Generatore::FUOCO_LARGE);
            pGeneratore->updateFuoco();
        }

    }else{
        if(val == _DEF_COLLI_CALIB_FORMAT_MAG){
            pGeneratore->setkV((float) KV_COLLIMATION_SMALL);
            pGeneratore->setmAs((float) mAs_COLLIMATION_SMALL);
            pGeneratore->setFuoco((Generatore::_FuocoSize_Enum) Generatore::FUOCO_SMALL);
            pGeneratore->updateFuoco();
        }else{
            pGeneratore->setkV((float) KV_COLLIMATION_LARGE);
            pGeneratore->setmAs((float) mAs_COLLIMATION_LARGE);
            pGeneratore->setFuoco((Generatore::_FuocoSize_Enum) Generatore::FUOCO_LARGE);
            pGeneratore->updateFuoco();
        }
    }
}


// Imposta tutto come da Factory!
void AnalogCalibPageOpen::downloadFactoryColliFormat(void){
        for(int i=0; i<formatList.size(); i++){
            formatCurrentData[i] = formatFactory[i];
            formatInitData[i] = formatFactory[i];
        }
        selectCollimationPattern(currentFormat,0);
        colliChanged = false;
        colliConfChanged = true;

        // Mirror factory
        if(pCollimatore->colli_model == _COLLI_TYPE_ASSY_01){
            pCollimatore->colliConf.mirrorSteps_ASSY_01 = _FACTORY_MIRROR_STEPS_ASSY01;
            ApplicationDatabase.setData(_DB_COLLI_CALIB_MIRROR, (int) _FACTORY_MIRROR_STEPS_ASSY01, DBase::_DB_NO_CHG_SGN);
        }else{
            pCollimatore->colliConf.mirrorSteps_ASSY_02 = _FACTORY_MIRROR_STEPS_ASSY02;
            ApplicationDatabase.setData(_DB_COLLI_CALIB_MIRROR, (int) _FACTORY_MIRROR_STEPS_ASSY02, DBase::_DB_NO_CHG_SGN);
        }
}

/* Scrive tutti i formati relativi alle collimazioni dei pad
 * Se il parametro è zero aggiorna il solo Formato selezionato.
 * Se il parametro è 1, allora aggiorna tutti i formati applicando
 * le sole differenze rispetto al valore iniziale
 * Se uno dei campi
*/
void AnalogCalibPageOpen::downloadColliFormat(void){
_formatStr modFormat;
_formatStr diffFormat;


    // Impostazione dei campi lame
    modFormat.F = ApplicationDatabase.getDataI(_DB_COLLI_CALIB_FRONT);
    modFormat.B = ApplicationDatabase.getDataI(_DB_COLLI_CALIB_BACK);
    modFormat.L = ApplicationDatabase.getDataI(_DB_COLLI_CALIB_LEFT);
    modFormat.R = ApplicationDatabase.getDataI(_DB_COLLI_CALIB_RIGHT);

    // formatData = collimazioni iniziali prima di tutte le modifiche
    diffFormat.F = modFormat.F - formatInitData[currentFormat].F;
    diffFormat.B = modFormat.B - formatInitData[currentFormat].B;
    diffFormat.L = modFormat.L - formatInitData[currentFormat].L;
    diffFormat.R = modFormat.R - formatInitData[currentFormat].R;


    // I dati del reference riguardano TUTTI I PAD
    if(currentFormat == _DEF_COLLI_CALIB_FORMAT_REFERENCE){

        // Le modifiche per INGRANDIMENTO E BIOPSIA SI APPLICANO SOLO ALLA LAMA FRONTALE
        // Dato che le lame sono già al massimo possibile
        for(int i=0; i<formatList.size(); i++){
            if(i==currentFormat){
                formatCurrentData[i] = modFormat;
            }else {

                if((i ==_COLLI_FORMAT_BIOPSY) || (i ==_COLLI_FORMAT_MAGNIFIER)){
                    formatCurrentData[i].F = formatInitData[i].F + diffFormat.F;
                    if(formatCurrentData[i].F<0) formatCurrentData[i].F=0;
                    if(formatCurrentData[i].F>255) formatCurrentData[i].F=255;
                }else{
                    // Si applicano le differenze
                    formatCurrentData[i].F = formatInitData[i].F + diffFormat.F;
                    if(formatCurrentData[i].F<0) formatCurrentData[i].F=0;
                    if(formatCurrentData[i].F>255) formatCurrentData[i].F=255;

                    formatCurrentData[i].B = formatInitData[i].B + diffFormat.B;
                    if(formatCurrentData[i].B<0) formatCurrentData[i].B=0;
                    if(formatCurrentData[i].B>255) formatCurrentData[i].B=255;

                    formatCurrentData[i].L = formatInitData[i].L + diffFormat.L;
                    if(formatCurrentData[i].L<0) formatCurrentData[i].L=0;
                    if(formatCurrentData[i].L>255) formatCurrentData[i].L=255;

                    formatCurrentData[i].R = formatInitData[i].R + diffFormat.R;
                    if(formatCurrentData[i].R<0) formatCurrentData[i].R=0;
                    if(formatCurrentData[i].R>255) formatCurrentData[i].R=255;
                }
            }
        }

    }else{
        // Per i singoli formati le modifiche si applicano solo al dato formato
        formatCurrentData[currentFormat] = modFormat;
    }

    colliChanged = false;
    colliConfChanged = true;

    // Aggiorna la collimazione
    pCollimatore->manualF = modFormat.F;
    pCollimatore->manualB = modFormat.B;
    pCollimatore->manualL = modFormat.L;
    pCollimatore->manualR = modFormat.R;
    pCollimatore->manualColliUpdate();

}

/*
 *  Porta lo specchio in Home, modifica la posizione degli step in campo
 *  e rimette in campo con la luce del collimatore lo specchio
 */
void AnalogCalibPageOpen::downloadColliMirror(void){
    pCollimatore->setToggleMirrorLamp(ApplicationDatabase.getDataI(_DB_COLLI_CALIB_MIRROR));

}


void AnalogCalibPageOpen::onDownloadMirror(void){
    ApplicationDatabase.setData(_DB_COLLI_MIRROR_DOWNLOAD,(int) 0, DBase::_DB_FORCE_SGN|DBase::_DB_ONLY_MASTER_ACTION);
}

void AnalogCalibPageOpen::onDownloadFormat(void){

    if(currentFormat == _DEF_COLLI_CALIB_FORMAT_FACTORY)  {
        onFactoryPushButton();
        return;
    }

    ApplicationDatabase.setData(_DB_COLLI_DOWNLOAD,(int) 0, DBase::_DB_FORCE_SGN|DBase::_DB_ONLY_MASTER_ACTION);
}

void AnalogCalibPageOpen::onFactoryOkSignal(void){
    ApplicationDatabase.setData(_DB_COLLI_FACTORY_DOWNLOAD,(int) 0, DBase::_DB_FORCE_SGN|DBase::_DB_ONLY_MASTER_ACTION);

}

void AnalogCalibPageOpen::onFactoryPushButton(void){

    connect(pWarningBox,SIGNAL(buttonOkSgn(void)),this,SLOT(onFactoryOkSignal(void)),Qt::UniqueConnection);
    pWarningBox->activate(QString("COLLIMATION FACTORY RESET"),QString("PRESS OK TO RESET ALL FORMATS"),msgBox::_BUTTON_CANC|msgBox::_BUTTON_OK);

    // Rimuove il fuoco dal bottone (per eliminare il riquadro tratteggiato che sta male)
    ui->collimationPanelFrame->setFocus();
}

void AnalogCalibPageOpen::onFrontEdit(void){

    dataField=QString("%1").arg((int) ApplicationDatabase.getDataI(_DB_COLLI_CALIB_FRONT));
    pCalculator->activate(&dataField, QString("FRONT BLADE SETUP"), 1);

    // Sposta il focus per evitare il cursore
    ui->collimationPanelFrame->setFocus();
}


void AnalogCalibPageOpen::onBackEdit(void){

    dataField=QString("%1").arg((int) ApplicationDatabase.getDataI(_DB_COLLI_CALIB_BACK));
    pCalculator->activate(&dataField, QString("BACK BLADE SETUP"), 2);

    // Sposta il focus per evitare il cursore
    ui->collimationPanelFrame->setFocus();
}



void AnalogCalibPageOpen::onLeftEdit(void){

    dataField=QString("%1").arg((int) ApplicationDatabase.getDataI(_DB_COLLI_CALIB_LEFT));
    pCalculator->activate(&dataField, QString("LEFT BLADE SETUP"), 3);

    // Sposta il focus per evitare il cursore
    ui->collimationPanelFrame->setFocus();
}


void AnalogCalibPageOpen::onRightEdit(void){

    dataField=QString("%1").arg((int) ApplicationDatabase.getDataI(_DB_COLLI_CALIB_RIGHT));
    pCalculator->activate(&dataField, QString("RIGHT BLADE SETUP"), 4);

    // Sposta il focus per evitare il cursore
    ui->collimationPanelFrame->setFocus();
}

void AnalogCalibPageOpen::onMirrorEdit(void){

    dataField=QString("%1").arg((int) ApplicationDatabase.getDataI(_DB_COLLI_CALIB_MIRROR));
    pCalculator->activate(&dataField, QString("MIRROR POSITION SETUP"), 5);

    // Sposta il focus per evitare il cursore
    ui->collimationPanelFrame->setFocus();
}

void AnalogCalibPageOpen::colliCalculatorSlot(bool stat){
    if(!stat) return;

    int val = dataField.toInt();

    if(pCalculator->activation_code == 1) {
        if(val<0) val=0;
        else if(val>255) val = 255;
        ApplicationDatabase.setData(_DB_COLLI_CALIB_FRONT, (int) val);
    }else if(pCalculator->activation_code == 2) {
        if(val<0) val=0;
        else if(val>200) val = 200;
        ApplicationDatabase.setData(_DB_COLLI_CALIB_BACK, (int) val);
    }else if(pCalculator->activation_code == 3) {
        if(val<0) val=0;
        else if(val>230) val = 230;
        ApplicationDatabase.setData(_DB_COLLI_CALIB_LEFT, (int) val);
    }else if(pCalculator->activation_code == 4) {
        if(val<0) val=0;
        else if(val>230) val = 230;
        ApplicationDatabase.setData(_DB_COLLI_CALIB_RIGHT, (int) val);
    }else if(pCalculator->activation_code == 5) {
        if(val<0) val=0;
        else if(val>2000) val = 2000;
        ApplicationDatabase.setData(_DB_COLLI_CALIB_MIRROR, (int) val);
    }
}




void AnalogCalibPageOpen::storeColliFormat(){

    int cindex;

    // Tutti i formati 24x30:
    cindex=pCollimatore->getColli2DIndex(PAD_24x30);
    pCollimatore->colliConf.colli2D[cindex].F = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_24x30].F;
    pCollimatore->colliConf.colli2D[cindex].B = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_24x30].B;
    pCollimatore->colliConf.colli2D[cindex].L = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_24x30].L;
    pCollimatore->colliConf.colli2D[cindex].R = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_24x30].R;

    // Tutti i formati 18x24
    cindex=pCollimatore->getColli2DIndex(PAD_18x24);
    pCollimatore->colliConf.colli2D[cindex].F = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].F;
    pCollimatore->colliConf.colli2D[cindex].B = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].B;
    pCollimatore->colliConf.colli2D[cindex].L = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].L;
    pCollimatore->colliConf.colli2D[cindex].R = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].R;
    // Clona la collimazione Custom con quella 18x24
    pCollimatore->customL = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].L;
    pCollimatore->customR = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].R;
    pCollimatore->customF = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].F;
    pCollimatore->customB = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].B;

    cindex=pCollimatore->getColli2DIndex(PAD_D75_CNT);
    pCollimatore->colliConf.colli2D[cindex].F = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].F;
    pCollimatore->colliConf.colli2D[cindex].B = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].B;
    pCollimatore->colliConf.colli2D[cindex].L = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].L;
    pCollimatore->colliConf.colli2D[cindex].R = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].R;

    cindex=pCollimatore->getColli2DIndex(PAD_BIOP_2D);
    pCollimatore->colliConf.colli2D[cindex].F = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].F;
    pCollimatore->colliConf.colli2D[cindex].B = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].B;
    pCollimatore->colliConf.colli2D[cindex].L = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].L;
    pCollimatore->colliConf.colli2D[cindex].R = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].R;

    cindex=pCollimatore->getColli2DIndex(PAD_10x24);
    pCollimatore->colliConf.colli2D[cindex].F = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].F;
    pCollimatore->colliConf.colli2D[cindex].B = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].B;
    pCollimatore->colliConf.colli2D[cindex].L = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].L;
    pCollimatore->colliConf.colli2D[cindex].R = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24].R;


    // Tutti i formati per biopsia
    cindex=pCollimatore->getColli2DIndex(PAD_BIOP_3D);
    pCollimatore->colliConf.colli2D[cindex].F = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_BIOP].F;
    pCollimatore->colliConf.colli2D[cindex].B = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_BIOP].B;
    pCollimatore->colliConf.colli2D[cindex].L = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_BIOP].L;
    pCollimatore->colliConf.colli2D[cindex].R = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_BIOP].R;


    // Tutti i formati per ingrandimento
    cindex=pCollimatore->getColli2DIndex(PAD_9x21);
    pCollimatore->colliConf.colli2D[cindex].F = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_MAG].F;
    pCollimatore->colliConf.colli2D[cindex].B = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_MAG].B;
    pCollimatore->colliConf.colli2D[cindex].L = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_MAG].L;
    pCollimatore->colliConf.colli2D[cindex].R = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_MAG].R;

    cindex=pCollimatore->getColli2DIndex(PAD_D75_MAG);
    pCollimatore->colliConf.colli2D[cindex].F = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_MAG].F;
    pCollimatore->colliConf.colli2D[cindex].B = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_MAG].B;
    pCollimatore->colliConf.colli2D[cindex].L = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_MAG].L;
    pCollimatore->colliConf.colli2D[cindex].R = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_MAG].R;

    if(pCollimatore->colli_model == _COLLI_TYPE_ASSY_01){
        pCollimatore->colliConf.mirrorSteps_ASSY_01 = ApplicationDatabase.getDataI(_DB_COLLI_CALIB_MIRROR);
    }else{
        pCollimatore->colliConf.mirrorSteps_ASSY_02 = ApplicationDatabase.getDataI(_DB_COLLI_CALIB_MIRROR);
    }

    pCollimatore->storeConfigFile();

}

void AnalogCalibPageOpen::initColliData(void){

    // Carica il contenuto della collimazione iniziale di riferimento
    // Presa da campioni prestabiliti
    _formatStr formato;
    int cindex=pCollimatore->getColli2DIndex(PAD_24x30);
    formato.L = pCollimatore->colliConf.colli2D[cindex].L;
    formato.R = pCollimatore->colliConf.colli2D[cindex].R;
    formato.B = pCollimatore->colliConf.colli2D[cindex].B;
    formato.F = pCollimatore->colliConf.colli2D[cindex].F;
    formatInitData[_DEF_COLLI_CALIB_FORMAT_24x30]= formato;
    formatCurrentData[_DEF_COLLI_CALIB_FORMAT_24x30]= formato;

    cindex=pCollimatore->getColli2DIndex(PAD_18x24);
    formato.L = pCollimatore->colliConf.colli2D[cindex].L;
    formato.R = pCollimatore->colliConf.colli2D[cindex].R;
    formato.B = pCollimatore->colliConf.colli2D[cindex].B;
    formato.F = pCollimatore->colliConf.colli2D[cindex].F;
    formatInitData[_DEF_COLLI_CALIB_FORMAT_18x24]= formato;
    formatCurrentData[_DEF_COLLI_CALIB_FORMAT_18x24]= formato;

    cindex=pCollimatore->getColli2DIndex(PAD_BIOP_3D);
    formato.L = pCollimatore->colliConf.colli2D[cindex].L;
    formato.R = pCollimatore->colliConf.colli2D[cindex].R;
    formato.B = pCollimatore->colliConf.colli2D[cindex].B;
    formato.F = pCollimatore->colliConf.colli2D[cindex].F;
    formatInitData[_DEF_COLLI_CALIB_FORMAT_BIOP]= formato;
    formatCurrentData[_DEF_COLLI_CALIB_FORMAT_BIOP]= formato;

    cindex=pCollimatore->getColli2DIndex(PAD_D75_MAG);
    formato.L = pCollimatore->colliConf.colli2D[cindex].L;
    formato.R = pCollimatore->colliConf.colli2D[cindex].R;
    formato.B = pCollimatore->colliConf.colli2D[cindex].B;
    formato.F = pCollimatore->colliConf.colli2D[cindex].F;
    formatInitData[_DEF_COLLI_CALIB_FORMAT_MAG]= formato;
    formatCurrentData[_DEF_COLLI_CALIB_FORMAT_MAG]= formato;

    // Inizializzazione formato di riferimetno
    formatInitData[_DEF_COLLI_CALIB_FORMAT_REFERENCE] = formatInitData[_DEF_COLLI_CALIB_FORMAT_REF_ITEM];
    formatCurrentData[_DEF_COLLI_CALIB_FORMAT_REFERENCE] = formatCurrentData[_DEF_COLLI_CALIB_FORMAT_REF_ITEM];

    // Inizializazione mirror steps
    if(pCollimatore->colli_model == _COLLI_TYPE_ASSY_01){
        ApplicationDatabase.setData(_DB_COLLI_CALIB_MIRROR, (int) pCollimatore->colliConf.mirrorSteps_ASSY_01, DBase::_DB_NO_CHG_SGN);
    }else{
        ApplicationDatabase.setData(_DB_COLLI_CALIB_MIRROR, (int) pCollimatore->colliConf.mirrorSteps_ASSY_02, DBase::_DB_NO_CHG_SGN);
    }

}
