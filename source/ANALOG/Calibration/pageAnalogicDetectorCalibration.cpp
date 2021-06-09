#include "../analog.h"
#include "pageCalibAnalogic.h"
//#include "pannelloComandi.h"
#include "analog_calib.h"
#include "ui_analog_calib.h"
#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"

#include "../../audio.h"

extern audio* pAudio;

//____________________________________________________________________
//  FUNZIONE CHIAMATA PERIODIAMENTE PER TESTARE LO STATO DI READY
//
bool AnalogCalibPageOpen::getDetectorCalibrationReady(unsigned char opt){

    int ready_stat=0;

    if(pPotter->getPotId()!=POTTER_2D) ready_stat|=0x1; // Potter Not correct or not present
    if(pPotter->getCassettePresence()) ready_stat|=0x2; // Cassetta presente
    if(pCompressore->isValidPad())     ready_stat|=0x4; // Compressore presente

    // In caso di presenza PC il reay è condizionato dal ready del PC
    if((ApplicationDatabase.getDataU(_DB_AWS_CONNECTION)) && (!ApplicationDatabase.getDataU(_DB_READY_EXPOSURE)))
        ready_stat|=0x8;   // PC NOT READY

    ApplicationDatabase.setData(_DB_READY_STAT,ready_stat,opt);

    if(ready_stat) return false;
    return true;
}

void AnalogCalibPageOpen::exitDetectorCalibration(void){
    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(detectorValueChanged(int,int)));
    disconnect(ui->nextButton,SIGNAL(released()),this,SLOT(onNextCampiButton()));
    disconnect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(detectorGuiNotify(unsigned char,unsigned char,QByteArray)));
    detector_calibration = false;
}

void AnalogCalibPageOpen::initDetectorCalibration(void){

    detector_calibration = true;

    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(detectorValueChanged(int,int)),Qt::UniqueConnection);
    connect(ui->nextButton,SIGNAL(released()),this,SLOT(onNextCampiButton()),Qt::UniqueConnection);
    connect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(detectorGuiNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

    // Impostazione geometria del frame di lavoro
    ui->calibDetectorFrame->show();
    ui->calibDetectorFrame->setGeometry(170,70,471,346);
    ui->frameWarningPotter->setGeometry(250,10,186,171);
    ui->frameWarningCompressor->setGeometry(250,60,186,76);

    ui->frameWarningPotter->show();
    ui->frameWarningCompressor->setGeometry(250,60,186,76);
    ui->frameWarningCompressor->hide();

    if(ApplicationDatabase.getDataU(_DB_AWS_CONNECTION)){
        ui->pcConnectedFrame->show();
        ui->manualSelectFrame->hide();
        ui->refLabel->show();
        ui->intestazione->setText(QString("DETECTOR CALIBRATION PANEL"));
        ui->framePhantom->setStyleSheet("border-image: url(:/Sym/Sym/RMMI-155.png);");

    }else{
        ui->pcConnectedFrame->hide();
        ui->manualSelectFrame->show();
        ui->refLabel->show();
        ui->intestazione->setText(QString("DETECTOR VERIFY PANEL"));
        ui->framePhantom->setStyleSheet("border-image: url(:/Sym/Sym/PMMI7.png);");
        ui->warningPC->hide();
    }

    if(!isMaster) return;

    pSysLog->log("SERVICE PANEL: ANALOGIC DETECTOR CALIBRATION");

    // Imposta le condizioni di ready
    getDetectorCalibrationReady(DBase::_DB_FORCE_SGN);


    if(ApplicationDatabase.getDataU(_DB_AWS_CONNECTION)){
        // IMpostazione tolleranza di accettabilità per il reference (solo bordo macchina)
        ApplicationDatabase.setData(_DB_REF_TOL, (int) rmmi_toll, DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_DETECTOR_REF_RAD, (int)  rmmi_reference, DBase::_DB_FORCE_SGN);
    }else{
        // IMpostazione tolleranza di accettabilità per il reference (solo bordo macchina)
        ApplicationDatabase.setData(_DB_REF_TOL, (int)  pConfig->analogCnf.calib_margine, DBase::_DB_FORCE_SGN);

        // Impostazione corrente del campo dell'esposimetro e del relativo riferimento
        ApplicationDatabase.setData(_DB_DETECTOR_CALIB_CAMPO, (int)  pConfig->analogCnf.aec_field, DBase::_DB_FORCE_SGN);

    }


    // Inizializzazione dei campi risultato
    ApplicationDatabase.setData(_DB_DETECTOR_RAD, (int)  0, DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_DETECTOR_PLOG, (int)  0, DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_DETECTOR_OFSET, (int)  0, DBase::_DB_FORCE_SGN);


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

    // Impostazione Fuoco grande
    pGeneratore->setFuoco(Generatore::FUOCO_LARGE);
    pGeneratore->updateFuoco();

}



void AnalogCalibPageOpen::detectorValueChanged(int index,int opt)
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

        // Annulla i dati
        ApplicationDatabase.setData(_DB_DETECTOR_PLOG, (int) 0);
        ApplicationDatabase.setData(_DB_DETECTOR_RAD, (int) 0);

        // Segnale di attivazione pulsante raggi
        if(!ApplicationDatabase.getDataI(_DB_XRAYPUSH_READY)){
            // Invio comando vocale relativo alla condizione di not ready

            if(ApplicationDatabase.getDataI(_DB_READY_STAT) & 0x1){
               pAudio->playAudio(AUDIO_NOT_READY_UNDETECTED_POTTER);

            }else if(ApplicationDatabase.getDataI(_DB_READY_STAT) & 0x2){
               pAudio->playAudio(AUDIO_NOT_READY_FOR_EXPOSURE_REMOVE_CASSETTE);

            }else if(ApplicationDatabase.getDataI(_DB_READY_STAT) & 0x4){
               pAudio->playAudio(AUDIO_NOT_READY_REMOVE_COMPRESSOR);

            }else if(ApplicationDatabase.getDataI(_DB_READY_STAT) & 0x8){
                pAudio->playAudio(AUDIO_NOT_READY_PC_NOT_READY);
            }

            return;
        }
        pToConsole->activationXrayPush();
        startDetectorCalibrationXraySequence();


        break;

    case _DB_DETECTOR_CALIB_CAMPO: // Impostazione del campo detector in utilizzo

        // Reset del contenuto dei campi risultato
        ApplicationDatabase.setData(_DB_DETECTOR_PLOG, (int) 0, DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_DETECTOR_RAD, (int) 0, DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_DETECTOR_OFSET, (int) 0, DBase::_DB_NO_CHG_SGN);

        ui->plogLabel->setText("PLOG:---");
        ui->radLabel->setText(QString("RAD:---"));
        ui->ofsRadLabel->setText(QString("OFS:---"));
        ui->resultFrame->setStyleSheet("background-image: url(:/transparent.png);background-color: rgb(255, 170, 127,90)");

        switch(ApplicationDatabase.getDataI(index)){
        case ANALOG_AECFIELD_FRONT:
            ui->outDetectorFrame->setStyleSheet(QString("border-image: url(:/Sym/Sym/campi_front.png); background-color: rgb(255, 255, 255,0);"));
            ui->nextButton->setStyleSheet(QString("background-image: url(:/transparent.png); border-image: url(:/Sym/Sym/campi_front.png);"));
            if((isMaster)&&(!ApplicationDatabase.getDataU(_DB_AWS_CONNECTION))) ApplicationDatabase.setData(_DB_DETECTOR_REF_RAD, (int)  pConfig->analogCnf.calib_f1,DBase::_DB_FORCE_SGN);
            break;
        case ANALOG_AECFIELD_CENTER:
            ui->outDetectorFrame->setStyleSheet(QString("border-image: url(:/Sym/Sym/campi_center.png); background-color: rgb(255, 255, 255,0);"));
            ui->nextButton->setStyleSheet(QString("background-image: url(:/transparent.png); border-image: url(:/Sym/Sym/campi_center.png);"));
            if((isMaster)&&(!ApplicationDatabase.getDataU(_DB_AWS_CONNECTION))) ApplicationDatabase.setData(_DB_DETECTOR_REF_RAD, (int)  pConfig->analogCnf.calib_f2,DBase::_DB_FORCE_SGN);
            break;
        case ANALOG_AECFIELD_BACK:
            ui->outDetectorFrame->setStyleSheet(QString("border-image: url(:/Sym/Sym/campi_back.png); background-color: rgb(255, 255, 255,0);"));
            ui->nextButton->setStyleSheet(QString("background-image: url(:/transparent.png); border-image: url(:/Sym/Sym/campi_back.png);"));
            if((isMaster)&&(!ApplicationDatabase.getDataU(_DB_AWS_CONNECTION))) ApplicationDatabase.setData(_DB_DETECTOR_REF_RAD, (int)  pConfig->analogCnf.calib_f3,DBase::_DB_FORCE_SGN);
            break;
        default:
            return;
        }

        // Imposta sull'esposimetro il campo effettivo
        if(isMaster) pPotter->setDetectorField(ApplicationDatabase.getDataI(index));
        break;

    case _DB_DETECTOR_REF_RAD: // Aggiornamento del campo REF RAD
        if(!ApplicationDatabase.getDataI(_DB_DETECTOR_REF_RAD)) ui->refLabel->setText(QString("REF:---"));
        else ui->refLabel->setText(QString("REF:%1").arg(ApplicationDatabase.getDataI(_DB_DETECTOR_REF_RAD)));
        break;

    case _DB_DETECTOR_RAD: // Aggiornamento RAD ottenuto dall'ultima esposizione
        if(ApplicationDatabase.getDataI(_DB_DETECTOR_RAD)) ui->radLabel->setText(QString("RAD:%1").arg(ApplicationDatabase.getDataI(_DB_DETECTOR_RAD)));
        else ui->radLabel->setText(QString("RAD:---"));

        // Controllo sul riferimento ma solo se il riferimento è diverso da 0
        if(!ApplicationDatabase.getDataI(_DB_DETECTOR_REF_RAD)){
            // Sempre fuori range poichè il sistema non è stato calibrato!!
            ui->resultFrame->setStyleSheet("background-image: url(:/transparent.png);background-color: rgb(255, 170, 127,90)");
        }else if(
                (ApplicationDatabase.getDataI(_DB_DETECTOR_RAD)>ApplicationDatabase.getDataI(_DB_DETECTOR_REF_RAD)+ApplicationDatabase.getDataI(_DB_REF_TOL)) ||
                (ApplicationDatabase.getDataI(_DB_DETECTOR_RAD)<ApplicationDatabase.getDataI(_DB_DETECTOR_REF_RAD)-ApplicationDatabase.getDataI(_DB_REF_TOL)) )  {
            // Fuori range
            ui->resultFrame->setStyleSheet("background-image: url(:/transparent.png);background-color: rgb(255, 170, 127,90)");
        }else{
            // Accettabile
            ui->resultFrame->setStyleSheet("background-image: url(:/transparent.png);background-color: rgb(54, 113, 6,90)");
        }
        break;

    case _DB_DETECTOR_PLOG:
        if(ApplicationDatabase.getDataI(_DB_DETECTOR_PLOG)) ui->plogLabel->setText(QString("PLOG:%1").arg(ApplicationDatabase.getDataI(_DB_DETECTOR_PLOG)));
        else ui->plogLabel->setText(QString("PLOG:---"));
        break;
    case _DB_DETECTOR_OFSET: // Il valore ricevuto è la somma di quattro campionamenti sul canale 25x
        if(ApplicationDatabase.getDataI(_DB_DETECTOR_OFSET))
            ui->ofsRadLabel->setText(QString("OFS:%1(R) %2(mV)").arg((float) ApplicationDatabase.getDataI(_DB_DETECTOR_OFSET)/4).arg((int)((float) ApplicationDatabase.getDataI(_DB_DETECTOR_OFSET)*1000*10)/(4*1024)));
        else ui->ofsRadLabel->setText(QString("OFS:---"));
        break;

    case _DB_AWS_CONNECTION:
        if(ApplicationDatabase.getDataU(_DB_AWS_CONNECTION)){
            ui->pcConnectedFrame->show();
            ui->manualSelectFrame->hide();
            ui->refLabel->show();
            ui->framePhantom->setStyleSheet("border-image: url(:/Sym/Sym/RMMI-155.png);");
            // Il frame del risultato è sempre in modalità accettabile
            ui->resultFrame->setStyleSheet("background-image: url(:/transparent.png);background-color: rgb(54, 113, 6,90)");
        }else{
            ui->pcConnectedFrame->hide();
            ui->manualSelectFrame->show();
            ui->refLabel->show();
            ui->framePhantom->setStyleSheet("border-image: url(:/Sym/Sym/PMMI7.png);");
        }

        if(!isMaster) return;
        if(ApplicationDatabase.getDataU(_DB_AWS_CONNECTION)){
            // IMpostazione tolleranza di accettabilità per il reference (solo bordo macchina)
            ApplicationDatabase.setData(_DB_REF_TOL, (int) rmmi_toll, DBase::_DB_FORCE_SGN);
            ApplicationDatabase.setData(_DB_DETECTOR_REF_RAD, (int)  rmmi_reference, DBase::_DB_FORCE_SGN);
        }else{
            // IMpostazione tolleranza di accettabilità per il reference (solo bordo macchina)
            ApplicationDatabase.setData(_DB_REF_TOL, (int)  pConfig->analogCnf.calib_margine, DBase::_DB_FORCE_SGN);

            // Impostazione corrente del campo dell'esposimetro e del relativo riferimento
            ApplicationDatabase.setData(_DB_DETECTOR_CALIB_CAMPO, (int)  pConfig->analogCnf.aec_field, DBase::_DB_FORCE_SGN);
        }

        break;

    case _DB_READY_STAT:

        // Warning PC not ready
        if(ApplicationDatabase.getDataI(index) & 0x8)  ui->warningPC->show();
        else ui->warningPC->hide();

        // Warning Potter non presente
        if(ApplicationDatabase.getDataI(index) & 0x1){
            ui->frameWarningPotter->show();
            ui->frameWarningCompressor->hide();
            ui->warningPotter->show();
            ui->warningCassette->hide();
            return;
        }

        // Warning cassetta presente
        if(ApplicationDatabase.getDataI(index) & 0x2){
            ui->frameWarningPotter->show();
            ui->frameWarningCompressor->hide();
            ui->warningPotter->hide();
            ui->warningCassette->show();
            return;
        }

        // Warning compressore presente
        if(ApplicationDatabase.getDataI(index) & 0x4){
            ui->frameWarningPotter->hide();
            ui->frameWarningCompressor->show();
            return;
        }

        // Nessun allarme
        ui->frameWarningPotter->show();
        ui->frameWarningCompressor->hide();
        ui->warningPotter->hide();
        ui->warningCassette->hide();

        break;

    }

}


void AnalogCalibPageOpen::onNextCampiButton(void){
    int ncampi = ApplicationDatabase.getDataI(_DB_DETECTOR_CALIB_CAMPO);
    ncampi++;
    if(ncampi>2) ncampi=0;
    ApplicationDatabase.setData(_DB_DETECTOR_CALIB_CAMPO, (int) ncampi);

}



// Attivazione funzione raggi per calibrazione detector
void AnalogCalibPageOpen::startDetectorCalibrationXraySequence(void){
    unsigned char data[18];

    // Impostazione kV
    pGeneratore->setkV((float) KV_PRE_FG_GRID);
    pGeneratore->setmAs((float) 50);

    // Impostazione dati di esposizione
    unsigned char errcode = pGeneratore->validateAnalogData(ANALOG_TECH_MODE_MANUAL,true, true);
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
    data[6] =  50 ; // 5s massimi per ogni impulso

    // Switch Generatore + Alta Velocita
    data[7]=0;
    if(pGeneratore->SWA) data[7]|=1;
    if(pGeneratore->SWB) data[7]|=2;

    // Starter HS sempre attivo
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
    if(pConsole->pGuiMcc->sendFrame(MCC_XRAY_ANALOG_CALIB_PRE,1,data,sizeof(data))==FALSE)
    {
        xrayErrorInCommand(ERROR_MCC_COMMAND);
        return;

    }


    ApplicationDatabase.setData(_DB_XRAY_SYM,(unsigned char) 1, DBase::_DB_FORCE_SGN);
    io->setXrayLamp(true);

}

void AnalogCalibPageOpen::detectorGuiNotify(unsigned char id, unsigned char mcccode, QByteArray data)
{

    switch(mcccode)
    {

    case MCC_XRAY_ANALOG_CALIB_PRE:
        stopAttesaDati();
        if(data.at(0)){
            // Esito negativo
            PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_RAGGI, data.at(0),TRUE); // Self resetting

        }else{
            // Assegna i dati ricevuti
            ApplicationDatabase.setData(_DB_DETECTOR_DMAS,(int) ((int) data[1]+ 256 * (int) data[2])  );
            ApplicationDatabase.setData(_DB_DETECTOR_PLOG,(int) ((int) data[3]+ 256 * (int) data[4])  );
            ApplicationDatabase.setData(_DB_DETECTOR_RAD,(int) ((int) data[5]+ 256 * (int) data[6])  );
            ApplicationDatabase.setData(_DB_DETECTOR_OFSET,(int) ((int) data[9]+ 256 * (int) data[10])  );
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
