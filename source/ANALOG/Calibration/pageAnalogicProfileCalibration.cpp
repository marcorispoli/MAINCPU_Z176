#include "../analog.h"
#include "pageCalibAnalogic.h"
//#include "pannelloComandi.h"
#include "analog_calib.h"
#include "ui_analog_calib.h"
#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"


// SELEZIONARE UN PROFILO DI RIFERIMENTO: ERRORE IN CASO DI MANCANZA DI PROFILO..
// SELEZIONARE CAMPO (DA COMANDO DI INGRESSO)
// GESTIONE ERRORE DURANTE CALCOLO AEC
// GESTIONE ERRORI DURANTE SEQUENZA

//____________________________________________________________________
//  FUNZIONE CHIAMATA PERIODIAMENTE PER TESTARE LO STATO DI READY
//
bool AnalogCalibPageOpen::getProfileCalibrationReady(unsigned char opt){

    int ready_stat=0;

    if(!pc_data_valid) return false;

    if((pc_selected_fuoco==Generatore::FUOCO_SMALL)&&(pPotter->getPotId()!=POTTER_MAGNIFIER)) ready_stat|=1;    // Wrong Potter
    if((pc_selected_fuoco==Generatore::FUOCO_LARGE)&&(pPotter->getPotId()!=POTTER_2D)) ready_stat|=1;           // Wrong Potter

    if(!pPotter->getCassettePresence()) ready_stat|=2;                                              // Missing Cassetta
    if(!ApplicationDatabase.getDataU(_DB_READY_EXPOSURE)) ready_stat|=4;                            // PC not READY
    if(pc_selected_pmmi==0) ready_stat|=8;                                                          // PC non ha selezionato i pmmi
    if(!pCompressore->isValidPad()) ready_stat|=0x10;                                               // Compressore non riconosciuto
    if(abs(pc_selected_pmmi-ApplicationDatabase.getDataI(_DB_SPESSORE))>10) ready_stat|=0x20;       // Spessore non compatibile con PMMI


    ApplicationDatabase.setData(_DB_CALIB_PROFILE_READY_STAT,ready_stat,opt);
    if(ready_stat) return false;
    return true;
}

void AnalogCalibPageOpen::exitProfileCalibration(void){
    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(profileValueChanged(int,int)));
    disconnect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(profileGuiNotify(unsigned char,unsigned char,QByteArray)));
    profile_calibration = false;
}

void AnalogCalibPageOpen::initProfileCalibration(void){

    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(profileValueChanged(int,int)),Qt::UniqueConnection);
    connect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(profileGuiNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

    // Impostazione geometria del frame di lavoro
    ui->calibProfileFrame->setGeometry(170,55,471,366);
    ui->exposureFrame->setGeometry(25,195,421,166);
    ui->dataFrame->setGeometry(25,10,421,176);


    ui->pcConnectedFrame->show();
    ui->calibProfileFrame->show();

    // Set intestazione pagina
    ui->intestazione->setText(QString("PROFILE CALIBRATION PANEL"));   

    // Stato di attesa dati
    ui->waitingProfileLabel->show();
    ui->exposureFrame->hide();
    ui->dataFrame->hide();
    pc_data_valid = false;

    profile_calibration = true;

    // Se i dati sono già validi accede direttamente al pannello dati
    // Questo potrebbe accadere a seguito di un'uscita di pagina dopo un errore
    if(pc_data_valid){
        setProfileData();
    }

    if(isMaster)  pSysLog->log("SERVICE PANEL: ANALOGIC PROFILE CALIBRATION");
}

void AnalogCalibPageOpen::setProfileData(void){
    if(!profile_calibration) return;
    if(!isMaster) return;

    // Selezione profilo in base all'indice
    if(pGeneratore->pAECprofiles->selectProfile(pc_selected_profile_index)!=null){
        ApplicationDatabase.setData(_DB_CALIB_PROFILE_NAME, pGeneratore->pAECprofiles->getCurrentProfilePtr()->symbolicName, DBase::_DB_FORCE_SGN);
    }else{
        ApplicationDatabase.setData(_DB_CALIB_PROFILE_NAME, "INVALID!!!", DBase::_DB_FORCE_SGN);
        pc_selected_profile_index=-1;
        PageAlarms::activateNewAlarm(_DB_ALLARMI_ANALOGICA, ERROR_NO_AEC_PROFILE,TRUE);
    }

    // Impostazione dati Pre Impulso
    if(pc_selected_fuoco==Generatore::FUOCO_LARGE){
        ApplicationDatabase.setData(_DB_CALIB_PROFILE_KVPRE, (int)  KV_PRE_FG_GRID , DBase::_DB_FORCE_SGN);
    }else{
        ApplicationDatabase.setData(_DB_CALIB_PROFILE_KVPRE, (int)  KV_PRE_FP_NO_GRID , DBase::_DB_FORCE_SGN);
    }
    ApplicationDatabase.setData(_DB_CALIB_PROFILE_DMAS_PRE, (int)  mAs_PRE*10 , DBase::_DB_FORCE_SGN);

    ApplicationDatabase.setData(_DB_CALIB_PROFILE_DKV, (int)  0, DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_CALIB_PROFILE_DMAS, (int)  0, DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_CALIB_PROFILE_PULSE, (int)  0, DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_CALIB_PROFILE_DOSE, (int)  0, DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_CALIB_PROFILE_PMMI, (int)  pc_selected_pmmi, DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_CALIB_PROFILE_PLOG, (int)  0, DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_CALIB_PROFILE_RAD, (int)  0, DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_CALIB_PROFILE_OFFSET, (int)  0, DBase::_DB_FORCE_SGN);


    // Impostazione collimazione aperta
    pCollimatore->manualCollimation = true;
    pCollimatore->manualF = 0;
    pCollimatore->manualB = 0;
    pCollimatore->manualL = 0;
    pCollimatore->manualR = 0;
    if(!pCollimatore->manualColliUpdate()) {
        PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_COLLI, COLLI_UPDATE_FALLITO,TRUE);
        colli_ok=false;
    }else colli_ok=true;

    // Impostazione filtro: il pre impulso deve comunque essere fatto con il Molibdeno (primo filtro)
    pCollimatore->manualFiltroCollimation = true;
    pCollimatore->manualFilter = pConfig->analogCnf.primo_filtro; //pc_selected_filtro;
    if(!pCollimatore->manualSetFiltro()){
        filter_ok=false;
        PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_COLLI, COLLI_FILTRO_FALLITO,TRUE);
    }else filter_ok=true;
    ApplicationDatabase.setData(_DB_CALIB_PROFILE_FILTRO, (int) pc_selected_filtro , DBase::_DB_FORCE_SGN);

    // Impostazione fuoco sulla base del potter presente
    // ApplicationDatabase.setData(_DB_ACCESSORIO, (unsigned char) data.at(0),0);
    if(pc_selected_fuoco==Generatore::FUOCO_SMALL){
        pGeneratore->setFuoco(Generatore::FUOCO_SMALL);
        ApplicationDatabase.setData(_DB_CALIB_PROFILE_PC_POTTER, (int)  1, DBase::_DB_FORCE_SGN);
    }else{
        // Impostazione Fuoco grande
        pGeneratore->setFuoco(Generatore::FUOCO_LARGE);
        ApplicationDatabase.setData(_DB_CALIB_PROFILE_PC_POTTER, (int)  0, DBase::_DB_FORCE_SGN);
    }
    if(!pGeneratore->updateFuoco()){
        PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_GEN, GEN_SET_FUOCO,TRUE);
        focus_ok=false;
    }else focus_ok=true;

    // Impostazione campo esposimetro
    ApplicationDatabase.setData(_DB_CALIB_PROFILE_CAMPO, (int)  pc_selected_field, DBase::_DB_FORCE_SGN);
    if(!pPotter->setDetectorField(pc_selected_field)){
        PageAlarms::activateNewAlarm(_DB_ALLARMI_ANALOGICA, ERROR_SETTING_DET_FIELD,TRUE);
        field_ok=false;
    }else{
        field_ok=true;
    }


    // Verifica se è possibile abilitare i dati
    if((pc_selected_profile_index==-1)||(!field_ok)||(!focus_ok)||(!filter_ok)||(!colli_ok)){
        pc_data_valid = false;
        ApplicationDatabase.setData(_DB_CALIB_PROFILE_WINDOW,(int) 0, DBase::_DB_FORCE_SGN);
        return;
    }

    pc_data_valid = true;
    ApplicationDatabase.setData(_DB_CALIB_PROFILE_WINDOW,(int) 1, DBase::_DB_FORCE_SGN);

    // Imposta le condizioni di ready
    getProfileCalibrationReady(DBase::_DB_FORCE_SGN);
    return;

}

void AnalogCalibPageOpen::profileValueChanged(int index,int opt)
{
    if((isMaster)&&(opt&DBase::_DB_ONLY_SLAVE_ACTION)) return;
    if((!isMaster)&&(opt&DBase::_DB_ONLY_MASTER_ACTION)) return;

    switch(index){
    case _DB_CALIB_PROFILE_WINDOW: // Impostazione della grafica corrente
        if(ApplicationDatabase.getDataI(index)==1){
            pc_data_valid = true;
            ui->waitingProfileLabel->hide();
            ui->exposureFrame->show();
            ui->dataFrame->show();
        }else{
            pc_data_valid = false;
            ui->waitingProfileLabel->show();
            ui->exposureFrame->hide();
            ui->dataFrame->hide();
        }
        break;
    case _DB_CALIB_PROFILE_NAME:
        ui->profileName->setText(QString("PROFILE NAME: %1").arg(ApplicationDatabase.getDataS(index)));
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

        ApplicationDatabase.setData(_DB_CALIB_PROFILE_DKV, (int)  0, DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_CALIB_PROFILE_DMAS, (int)  0, DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_CALIB_PROFILE_PULSE, (int)  0, DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_CALIB_PROFILE_DOSE, (int)  0, DBase::_DB_FORCE_SGN);

        ApplicationDatabase.setData(_DB_CALIB_PROFILE_PLOG, (int)  0, DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_CALIB_PROFILE_RAD, (int)  0, DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_CALIB_PROFILE_OFFSET, (int)  0, DBase::_DB_FORCE_SGN);

        // Segnale di attivazione pulsante raggi
        if(!ApplicationDatabase.getDataI(_DB_XRAYPUSH_READY)) return;
        pToConsole->activationXrayPush();
        startProfileCalibrationXraySequence();

        break;

    case _DB_CALIB_PROFILE_CAMPO: // Impostazione del campo detector in utilizzo

        switch(ApplicationDatabase.getDataI(index)){
        case ANALOG_AECFIELD_FRONT:
            ui->profileFieldFrame->setStyleSheet(QString("border-image: url(:/Sym/Sym/campi_front.png); background-color: rgb(255, 255, 255,0);"));
            break;
        case ANALOG_AECFIELD_CENTER:
            ui->profileFieldFrame->setStyleSheet(QString("border-image: url(:/Sym/Sym/campi_center.png); background-color: rgb(255, 255, 255,0);"));
            break;
        case ANALOG_AECFIELD_BACK:
            ui->profileFieldFrame->setStyleSheet(QString("border-image: url(:/Sym/Sym/campi_back.png); background-color: rgb(255, 255, 255,0);"));
            break;
        default:
            return;
        }

        break;



    case _DB_CALIB_PROFILE_RAD:
        if(ApplicationDatabase.getDataI(index)) ui->profileRad->setText(QString("Rad:%1").arg(ApplicationDatabase.getDataI(index)));
        else ui->profileRad->setText(QString("Rad:---"));
        break;

    case _DB_CALIB_PROFILE_PLOG:
        if(ApplicationDatabase.getDataI(index)) ui->profilePlog->setText(QString("Plog:%1").arg(ApplicationDatabase.getDataI(index)));
        else ui->profilePlog->setText(QString("Plog:---"));
        break;
    case _DB_CALIB_PROFILE_OFFSET:
        if(ApplicationDatabase.getDataI(index)) ui->profileOffset->setText(QString("Ofs:%1").arg((float) ApplicationDatabase.getDataI(index)/25));
        else ui->profileOffset->setText(QString("Ofs:---"));
        break;

    case _DB_CALIB_PROFILE_KVPRE:
        if(ApplicationDatabase.getDataI(index)) ui->profilekVPre->setText(QString("kV:%1").arg(ApplicationDatabase.getDataI(index)));
        else ui->profilekVPre->setText(QString("kV:---"));
        break;
    case _DB_CALIB_PROFILE_DKV:
        if(ApplicationDatabase.getDataI(index)) ui->profileKv->setText(QString("kV:%1").arg((float) ApplicationDatabase.getDataI(index)/10));
        else ui->profileKv->setText(QString("kV:---"));
        break;
    case _DB_CALIB_PROFILE_DMAS_PRE:
        if(ApplicationDatabase.getDataI(index)) ui->profilemAsPre->setText(QString("mAs:%1").arg((float) (ApplicationDatabase.getDataI(index))/10));
        else ui->profilemAsPre->setText(QString("mAs:---"));
        break;
    case _DB_CALIB_PROFILE_DMAS:
        if(ApplicationDatabase.getDataI(index)) ui->profilemAs->setText(QString("mAs:%1").arg((float) (ApplicationDatabase.getDataI(index))/10));
        else ui->profilemAs->setText(QString("mAs:---"));
        break;
    case _DB_CALIB_PROFILE_PULSE:
        if(ApplicationDatabase.getDataI(index)) ui->profilePulse->setText(QString("Pulse:%1").arg(ApplicationDatabase.getDataI(index)));
        else ui->profilePulse->setText(QString("Pulse:---"));
        break;
    case _DB_CALIB_PROFILE_DOSE:
        if(ApplicationDatabase.getDataI(index)) ui->profileDose->setText(QString("Dose:%1").arg((float) (ApplicationDatabase.getDataI(index))/10));
        else ui->profileDose->setText(QString("Dose:---"));
        break;
    case _DB_CALIB_PROFILE_FILTRO:
        if(ApplicationDatabase.getDataI(index)==Collimatore::FILTRO_Rh) ui->profileFilter->setText(QString("Filter:Rh"));
        else ui->profileFilter->setText(QString("Filter:Mo"));
        break;
    case _DB_CALIB_PROFILE_PMMI:
        if(ApplicationDatabase.getDataI(index)==45){
            ui->frameProfilePhantom->setStyleSheet(QString("border-image: url(:/Sym/Sym/PMMI45.png);background-color: rgb(0, 85, 255,0);"));
        }else{
            ui->frameProfilePhantom->setStyleSheet(QString("border-image: url(:/Sym/Sym/PMMI%1.png);background-color: rgb(0, 85, 255,0);").arg(ApplicationDatabase.getDataI(index)/10));
        }

        break;

    case _DB_CALIB_PROFILE_PC_POTTER:
        if(ApplicationDatabase.getDataI(index)==1)
            ui->frameProfilePotter->setStyleSheet(QString("border-image: url(:/Sym/Sym/MAGNIFIER_CASSETTE.png);background-color: rgb(0, 85, 255,0);"));
        else
            ui->frameProfilePotter->setStyleSheet(QString("border-image: url(:/Sym/Sym/POTTER-BUCKY_CASSETTE.png);background-color: rgb(0, 85, 255,0);"));
        break;
    case _DB_AWS_CONNECTION:

        break;

    case _DB_SPESSORE:
        ui->profileThick->setText(QString("thick:%1").arg(ApplicationDatabase.getDataI(index)));
        break;
    case _DB_FORZA:
        ui->profileForce->setText(QString("force:%1").arg(ApplicationDatabase.getDataI(index)));
        break;

    case _DB_CALIB_PROFILE_READY_STAT:

        if(ApplicationDatabase.getDataI(index) & 0x1)  ui->warningPotterProfile->show();
        else ui->warningPotterProfile->hide();

        if(ApplicationDatabase.getDataI(index) & 0x2)  ui->warningCassetteProfile->show();
        else ui->warningCassetteProfile->hide();

        if(ApplicationDatabase.getDataI(index) & 0x4)  ui->warningPC->show();
        else ui->warningPC->hide();

        if(ApplicationDatabase.getDataI(index) & 0x8){
            ui->warningPhantomProfile->show();
            ui->profilePhantomLabel->setText("PHANTOM NOT SELECTED!");
            ui->profilePhantomLabel->show();
        }else {
            if(!(ApplicationDatabase.getDataI(index) & 0x1)){
                if(ApplicationDatabase.getDataI(index) & 0x10){
                    // Compressore non identificato
                     ui->warningPhantomProfile->show();
                     ui->profilePhantomLabel->setText("COMPRESSOR UNDEFINED!");
                     ui->profilePhantomLabel->show();
                }else if(ApplicationDatabase.getDataI(index) & 0x20){
                    // Spessore non corrispondente
                     ui->warningPhantomProfile->show();
                     ui->profilePhantomLabel->setText("UNMATCHING THICKNESS!");
                     ui->profilePhantomLabel->show();
                }else{
                    // Tutto OK
                    ui->profilePhantomLabel->hide();
                    ui->warningPhantomProfile->hide();
                }
            }else{
                // Non c'è il potter, quindi non si può identificare il compressore
                ui->profilePhantomLabel->hide();
                ui->warningPhantomProfile->hide();
            }
        }


        break;

    }

}


// Attivazione pre impulso di una sequenza di calibrazione del profilo
void AnalogCalibPageOpen::startProfileCalibrationXraySequence(void){
    unsigned char data[25];

    // Impostazione kV
    pGeneratore->setkV((float) ApplicationDatabase.getDataI(_DB_CALIB_PROFILE_KVPRE));
    pGeneratore->setmAs((float) mAs_PRE);

    // Impostazione dati di esposizione
    unsigned char errcode = pGeneratore->validateAnalogData(ANALOG_TECH_MODE_MANUAL, true, true);
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
    data[6] =  10 | 0x80; // 100ms massimi per ogni impulso: usa il timer corto da 10ms

    // Switch Generatore + Alta Velocita
    data[7]=0;
    if(pGeneratore->SWA) data[7]|=1;
    if(pGeneratore->SWB) data[7]|=2;
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

    data[18] = 0; // Riservato per gli impulsi dell'esposimetro
    data[19] = 0;
    data[20] = 0; // 0=pre-pulse, 1 = Pulse

    // Prova ad inviare il comando
    if(pConsole->pGuiMcc->sendFrame(MCC_XRAY_ANALOG_CALIB_PROFILE,1,data,21)==FALSE)
    {
        xrayErrorInCommand(ERROR_MCC_COMMAND);
        return;

    }


    ApplicationDatabase.setData(_DB_XRAY_SYM,(unsigned char) 1, DBase::_DB_FORCE_SGN);
    io->setXrayLamp(true);

}


void AnalogCalibPageOpen::profileGuiNotify(unsigned char id, unsigned char mcccode, QByteArray rxdata)
{
    unsigned char data[25];
    QByteArray qdata;

    switch(mcccode)
    {

    /*
        data[0]=RXOK;
        data[1]=(unsigned char) ((Param->dmAs_released)&0xFF);
        data[2]=(unsigned char) ((Param->dmAs_released>>8)&0xFF);
        data[3]=(unsigned char) ((Param->pulses_released)&0xFF);
        data[4]=(unsigned char) ((Param->pulses_released>>8)&0xFF);
    */
    case MCC_XRAY_ANALOG_CALIB_PROFILE: // Fine raggi
        stopAttesaDati();

        qdata.append(rxdata.at(0));      // risultato

        if(rxdata.at(0)){
            // Esito negativo
            PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_RAGGI, rxdata.at(0),TRUE); // Self resetting
            ApplicationDatabase.setData(_DB_CALIB_PROFILE_DMAS,(int) 0  );
            ApplicationDatabase.setData(_DB_CALIB_PROFILE_PULSE,(int) 0  );
            ApplicationDatabase.setData(_DB_CALIB_PROFILE_DKV,(int) pGeneratore->selectedKv*10 );

            qdata.append(( char) 0); // dma
            qdata.append(( char) 0);
            qdata.append(( char) 0); // pulses
            qdata.append(( char) 0);
            qdata.append(( char) (pGeneratore->selectedKv*10));
            qdata.append(( char) (((int)(pGeneratore->selectedKv*10))>>8));
            qdata.append(( char) profile_rxplog); // plog
            qdata.append(( char) (profile_rxplog >> 8));


        }else{
            // Assegna i dati ricevuti
            ApplicationDatabase.setData(_DB_CALIB_PROFILE_DMAS,(int) ((int) rxdata[1]+ 256 * (int) rxdata[2])  );
            ApplicationDatabase.setData(_DB_CALIB_PROFILE_PULSE,(int) ((int) rxdata[3]+ 256 * (int) rxdata[4])  );
            ApplicationDatabase.setData(_DB_CALIB_PROFILE_DKV,(int) (pGeneratore->selectedKv*10) );            
            qdata.append(rxdata.at(1)); // dma
            qdata.append(rxdata.at(2));
            qdata.append(rxdata.at(3)); // pulses
            qdata.append(rxdata.at(4));
            qdata.append(( char) (pGeneratore->selectedKv*10));
            qdata.append(( char) (((int)(pGeneratore->selectedKv*10))>>8));
            qdata.append(( char) profile_rxplog); // plog
            qdata.append(( char) (profile_rxplog >> 8));


        }

        // Rilascio Pulsante raggi
        ApplicationDatabase.setData(_DB_XRAY_SYM,(unsigned char) 0, DBase::_DB_FORCE_SGN);
        io->setXrayLamp(false);

        // Notifica a PC se necessario
        if(!ApplicationDatabase.getDataU(_DB_AWS_CONNECTION)) break;
        pToConsole->fineRaggiAnalogCalibProfile(qdata);

    break;

    // Messaggio per richiedere i dati di curva zero dall'esposimetro
    case MCC_XRAY_ANALOG_REQ_AEC_PULSE:

        stopAttesaDati();

        // Richiede i dati dell'AEC
        profile_rxplog = rxdata[0] + 256*rxdata[1];
        profile_rxrad = rxdata[2] + 256*rxdata[3];
        int rad25 = rxdata[6] + 256*rxdata[7];


        ApplicationDatabase.setData(_DB_CALIB_PROFILE_PLOG, (int)  profile_rxplog, DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_CALIB_PROFILE_RAD, (int)  profile_rxrad, DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_CALIB_PROFILE_OFFSET, (int)  rad25, DBase::_DB_FORCE_SGN);


        // Preleva i dati per la nuova esposizione
        int error = pGeneratore->pAECprofiles->getAecData(profile_rxplog,ANALOG_FILTRO_FISSO, pc_selected_filtro, 5,ANALOG_TECH_PROFILE_STD,pGeneratore->selectedAnodo, pGeneratore->selectedFSize, &profile_rxfiltro,&profile_rxkV,&profile_rxdmas,&profile_rxpulses);
        if(error<0){
            xrayErrorInCommand(ESPOSIMETRO_INVALID_AEC_DATA);
            PRINT(QString("PROFILE ERROR AEC:%1 ").arg(error));
            return;
        }

        if(pc_selected_pulses) profile_rxpulses = pc_selected_pulses; // bypass da PC sugli impulsi



        pGeneratore->setkV(profile_rxkV);
        pGeneratore->setmAs((float) profile_rxdmas/10);
        pCollimatore->manualFilter = pc_selected_filtro;
        if(!pCollimatore->manualSetFiltro()){
          // TBD
        }

        // Impostazione dati di esposizione
        unsigned char errcode = pGeneratore->validateAnalogData(ANALOG_TECH_MODE_MANUAL, true, false);
        if(errcode){
            xrayErrorInCommand(errcode);
            return;
        }

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

        data[18] = (unsigned char) (profile_rxpulses);
        data[19] = (unsigned char) (profile_rxpulses>>8);
        data[20] = 1; // 0=pre-pulse, 1 = Pulse

        // Prova ad inviare il comando
        if(pConsole->pGuiMcc->sendFrame(MCC_XRAY_ANALOG_CALIB_PROFILE,1,data,21)==FALSE)
        {
            xrayErrorInCommand(ERROR_MCC_COMMAND);
            return;

        }

        break;
    }
}
