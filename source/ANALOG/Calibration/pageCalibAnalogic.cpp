#include "pageCalibAnalogic.h"
//#include "pannelloComandi.h"
#include "analog_calib.h"
#include "ui_analog_calib.h"
#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"

#define UI_PAGINA _PG_CALIB_ANALOG
#define EXIT_PAGINA _PG_MAIN_ANALOG
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000

AnalogCalibPageOpen::AnalogCalibPageOpen(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::analog_calibUI)
{
    ui->setupUi(this);
    scene = new QGraphicsScene();
    view = new QGraphicsView(scene);
    proxy = scene->addWidget(this);

    view->setWindowFlags(Qt::FramelessWindowHint);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setFixedSize(800,480);    // Dimensione della vista
    scene->setSceneRect(0,0,800,480);
    view->rotate(rotview);       // Angolo di rotazione della vista corrente
    view->setAlignment(Qt::AlignRight);
    view->setScene(scene);

    this->rotview = rotview;
    this->parent = parent;

    // IMPOSTAZIONI STANDARD _______________________________________________________________________________
    connect(&GWindowRoot,SIGNAL(changePage(int,int)), this,SLOT(changePage(int,int)),Qt::UniqueConnection);
    connect(EXIT_BUTTON,SIGNAL(released()),this,SLOT(onExitButton()),Qt::UniqueConnection);

    //______________________________________________________________________________________________________



    // Crea pannello comandi nella rispettiva view
    // commandPanel = new pannelloComandi(ui->comandi);
    ui->alarm_frame->setGeometry(680,70,101,96);
    ui->ready->setGeometry(155,430,521,41);
    ui->not_ready->setGeometry(155,430,521,41);
    ui->frameAttesaDati->setGeometry(0,0,800,640);
    ui->frameAttesaDati->hide();
    data_ready=true;
    timer_attesa_dati = 0;

    ui->xrayFrame->hide();
    ui->alarm_frame->hide();

    // Inizializzazione necessaria per non lasciare il campo vuoto
    if(isMaster){
        pc_selected_filtro = pConfig->analogCnf.primo_filtro;
        pc_selected_pmmi = 20;
    }

    // Creazione della lista dei potter disponibili
    formatFactory.clear();
    formatList.clear();
    formatCurrentData.clear();
    formatInitData.clear();


    _formatStr factory;
    formatList.append(QString("REFERENCE (18x24)"));
    factory.F=_FACTORY_FRONT_LARGE_FOCUS;
    factory.B=_FACTORY_BACK_18x24;
    factory.L=_FACTORY_LEFT_18x24;
    factory.R=_FACTORY_RIGHT_18x24;
    formatFactory.append(factory);
    formatCurrentData.append(factory);
    formatInitData.append(factory);

    formatList.append(QString("FORMAT 24x30"));
    factory.F=_FACTORY_FRONT_LARGE_FOCUS;
    factory.B=_FACTORY_BACK_24x30;
    factory.L=_FACTORY_LEFT_24x30;
    factory.R=_FACTORY_RIGHT_24x30;
    formatFactory.append(factory);
    formatCurrentData.append(factory);
    formatInitData.append(factory);


    formatList.append(QString("FORMAT 18x24"));
    factory.F=_FACTORY_FRONT_LARGE_FOCUS;
    factory.B=_FACTORY_BACK_18x24;
    factory.L=_FACTORY_LEFT_18x24;
    factory.R=_FACTORY_RIGHT_18x24;
    formatFactory.append(factory);
    formatCurrentData.append(factory);
    formatInitData.append(factory);

    formatList.append(QString("FORMAT BIOPSY"));
    factory.F=_FACTORY_FRONT_LARGE_FOCUS;
    factory.B=_FACTORY_BACK_BIOPSY;
    factory.L=_FACTORY_LEFT_BIOPSY;
    factory.R=_FACTORY_RIGHT_BIOPSY;
    formatFactory.append(factory);
    formatCurrentData.append(factory);
    formatInitData.append(factory);

    formatList.append(QString("FORMAT MAGNIFIER"));
    factory.F=_FACTORY_FRONT_SMALL_FOCUS;
    factory.B=_FACTORY_BACK_MAGNIFIER;
    factory.L=_FACTORY_LEFT_MAGNIFIER;
    factory.R=_FACTORY_RIGHT_MAGNIFIER;
    formatFactory.append(factory);
    formatCurrentData.append(factory);
    formatInitData.append(factory);

    formatList.append(QString("MIRROR SETUP"));
    factory.F=0;
    factory.B=0;
    factory.L=0;
    factory.R=0;
    formatFactory.append(factory);
    formatCurrentData.append(factory);
    formatInitData.append(factory);

    formatList.append(QString("FACTORY RESET"));
    factory.F=0;
    factory.B=0;
    factory.L=0;
    factory.R=0;
    formatFactory.append(factory);
    formatCurrentData.append(factory);
    formatInitData.append(factory);

    collimation_calibration=false;
    detector_calibration=false;
    profile_calibration=false;
    tube_calibration=false;
    manual_exposure=false;

    timerDisable=0;
    timerReady = 0; // Polling per monitariio Ready/not ready
}

AnalogCalibPageOpen::~AnalogCalibPageOpen()
{
    delete ui;
}

// Funzione agganciata ai sistemi di menu custom
void AnalogCalibPageOpen::changePage(int pg, int opt)
{
    if(UI_PAGINA==pg)
    {

        // Attivazione pagina
        if(GWindowRoot.curPageVisible== TRUE){
            // Disabilitazione allarmi di sistema
            paginaAllarmi->alarm_enable = true;

            // Disabilita il pulsante di uscita per un certo tempo
            EXIT_BUTTON->hide();
            changePageTimer = startTimer(DISABLE_EXIT_TMO);
            view->show();

            // Verifica se si tratta di apertura pagina o semplice shift
            if(opt & DBase::_DB_INIT_PAGE) initPage();
        }
        else view->hide();
        return;
    }
    else if(GWindowRoot.curPage==UI_PAGINA)
    {
        // Disattivazione pagina
        paginaAllarmi->alarm_enable = true;
        view->hide();

        // Verifica se si tratta di uscita definitiva o apertura di pagina tipo allarme
        if(opt & DBase::_DB_EXIT_PAGE) exitPage();
    }

}

bool AnalogCalibPageOpen::openPageRequest(unsigned char page_code){

    ApplicationDatabase.setData(_DB_EXPOSURE_MODE,(unsigned char) page_code);
    ApplicationDatabase.setData(_DB_STUDY_STAT,(unsigned char) _OPEN_STUDY_ANALOG);
    GWindowRoot.setNewPage(_PG_CALIB_ANALOG,GWindowRoot.curPage,DBase::_DB_INIT_PAGE);
    return true;
}

// Reazione alla pressione del pulsante di uscita
void AnalogCalibPageOpen::onExitButton(void)
{
    // Chiude lo studio e forza l'uscita della pagina
    ApplicationDatabase.setData(_DB_STUDY_STAT,(unsigned char) _CLOSED_STUDY_STATUS,DBase::_DB_FORCE_SGN);
}



void AnalogCalibPageOpen::initPage(void){

    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
    connect(ui->alarm_butt,SIGNAL(released()),this,SLOT(onAlarmButt()),Qt::UniqueConnection);

    PRINT("INIZIALIZZAZIONE CALIB PAGE");
    if(ApplicationDatabase.getDataU(_DB_NALLARMI_ATTIVI)) ui->alarm_frame->show();
    else ui->alarm_frame->hide();

    // Disabilita tutti i frame di lavoro per poi attivare solo quello operativo
    ui->calibDetectorFrame->hide();
    ui->calibProfileFrame->hide();
    ui->tubeCalibFrame->hide();
    ui->manualPanelFrame->hide();
    ui->collimationPanelFrame->hide();
    stopAttesaDati();

    // Selezione della visualizzazione dello sfondo in funzione della calibrazione in corso
    switch(ApplicationDatabase.getDataU(_DB_EXPOSURE_MODE)){
    case _EXPOSURE_MODE_RX_SHOT_NODET_MODE:        
        break;
    case _EXPOSURE_MODE_CALIB_MODE_KV:
        initTubeCalibration();
        break;
    case _EXPOSURE_MODE_CALIB_MODE_IA:
        initTubeCalibration();
        break;
    case _EXPOSURE_MODE_CALIB_MODE_KERMA:
        initTubeCalibration();
        break;

    case _EXPOSURE_MODE_CALIB_MODE_EXPOSIMETER:
         initDetectorCalibration();
         break;
    case _EXPOSURE_MODE_CALIB_MODE_PROFILE:
         initProfileCalibration();
         break;
    case _EXPOSURE_MODE_ANALOG_MANUAL_EXPOSURE:
         initManualExposure();
         break;
    case _EXPOSURE_MODE_ANALOG_COLLIMATION_EXPOSURE:
         initCollimationCalibration();
         break;

    }

    if(timerDisable==0)  timerDisable=startTimer(500);

    // Vale per tutti al ritorno nella pagina
    if(!isMaster) return;

    ApplicationDatabase.setData(_DB_XRAYPUSH_READY, (int) 0,DBase::_DB_FORCE_SGN);      // Imposta il not Ready di default
    ApplicationDatabase.setData(_DB_XRAY_SYM,(unsigned char) 0,DBase::_DB_FORCE_SGN);   // Elimina il simbolo raggi
    if(!timerReady) timerReady = startTimer(500);
    return;
}

void AnalogCalibPageOpen::startAttesaDati(int time){
    timer_attesa_dati = startTimer(time);
    ui->frameAttesaDati->show();
}

void AnalogCalibPageOpen::stopAttesaDati(void){
    if(!isMaster) return;
    ApplicationDatabase.setData(_DB_STOP_ATTESA_DATI,(int)0, DBase::_DB_FORCE_SGN);
}

void AnalogCalibPageOpen::setAecField(int campo){

    // Dipende dalla funzione selezionata
    ApplicationDatabase.setData(_DB_DETECTOR_CALIB_CAMPO, (int) campo, DBase::_DB_FORCE_SGN);

}

void AnalogCalibPageOpen::onAlarmButt(void){
    if(timerDisable) return;
    timerDisable=startTimer(500);
    ui->calibFrame->setFocus();

    GWindow::setPage(_PG_ALARM,GWindowRoot.curPage,0);
}


// FUNZIONE CHIAMATA DALL'HANDLER DEL CAMBIO PAGINA
void AnalogCalibPageOpen::exitPage(void){
    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
    disconnect(ui->alarm_butt,SIGNAL(released()),this,SLOT(onAlarmButt()));


    // Fa uscire la pagina correntemente attiva
    if(detector_calibration)            exitDetectorCalibration();
    else if(profile_calibration)        exitProfileCalibration();
    else if(manual_exposure)            exitManualExposure();
    else if(tube_calibration)           exitTubeCalibration();
    else if(collimation_calibration)    exitCollimationCalibration();

    // Chiama l'uscita se master
    if(!isMaster) return;

    // Cancella il timer ready
    if(timerReady) killTimer(timerReady);
    timerReady=0;

    // Riporta la collimazione in automatico
    pCollimatore->manualCollimation = false;
    pCollimatore->manualFiltroCollimation = false;
    pCollimatore->updateColli();
    return;
}

void AnalogCalibPageOpen::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId()==changePageTimer)
    {
        killTimer(changePageTimer);
        // Abilita il pulsante di uscita
        EXIT_BUTTON->show();
        return;
    }

    if(ev->timerId()==timerDisable)
    {
        killTimer(timerDisable);
        timerDisable=0;
        return;
    }

    if(isMaster){
        if(ev->timerId()==timerReady)
        {
            verifyReady();
            return;
        }
    }

    if(ev->timerId()==timer_attesa_dati)
    {
        killTimer(timer_attesa_dati);
        ui->frameAttesaDati->hide();
        timer_attesa_dati=0;
        return;
    }



}

void AnalogCalibPageOpen::valueChanged(int index,int opt)
{
    if((isMaster)&&(opt&DBase::_DB_ONLY_SLAVE_ACTION)) return;
    if((!isMaster)&&(opt&DBase::_DB_ONLY_MASTER_ACTION)) return;

    switch(index){
    case _DB_ACCESSORIO:
        break;

    case _DB_COMPRESSOR_UNLOCK:
        break;

    case _DB_NALLARMI_ATTIVI:
        if(ApplicationDatabase.getDataU(_DB_NALLARMI_ATTIVI)) ui->alarm_frame->show();
        else ui->alarm_frame->hide();
        break;

    case _DB_XRAYPUSH_READY:

        if(ApplicationDatabase.getDataI(index) ){
            ui->ready->show();
            ui->not_ready->hide();
            if(isMaster) pToConsole->notifyReadyForExposure(true);
        }else{
            ui->ready->hide();
            ui->not_ready->show();
            if(isMaster) pToConsole->notifyReadyForExposure(false);
        }

        break;

    case _DB_XRAY_SYM:
        // Attivazione del simbolo raggi
        if(ApplicationDatabase.getDataU(_DB_XRAY_SYM)){
            ui->xrayFrame->show();
            data_ready = false;
        } else{
            if(!data_ready) startAttesaDati(5000);
            ui->xrayFrame->hide();
        }
        break;

    case _DB_STOP_ATTESA_DATI:
        data_ready = true;
        ui->frameAttesaDati->hide();
        if(timer_attesa_dati) killTimer(timer_attesa_dati);
        timer_attesa_dati=0;
        break;

    case _DB_STUDY_STAT:
        if(!isMaster) return;
        if(ApplicationDatabase.getDataU(index) == _CLOSED_STUDY_STATUS) pConfig->selectMainPage();
        break;
    }

}



/*
 *  VERIFICA LE CONDIZIONI PER AUTORIZZARE LA PRESSIONE DEL PULSANTE RAGGI


 */

void AnalogCalibPageOpen::verifyReady(void){
    if(!isMaster) return;

    // Non appena si preme il pulsante raggi il ready viene tolto
    if(ApplicationDatabase.getDataU(_DB_XRAY_PUSH_BUTTON)){
        ApplicationDatabase.setData(_DB_XRAYPUSH_READY,(int) 0);
        return;
    }

    bool ready = true;

    switch(ApplicationDatabase.getDataU(_DB_EXPOSURE_MODE)){
    case _EXPOSURE_MODE_CALIB_MODE_KERMA:
        if(!getTubeCalibrationReady(0)) ready = false;
        break;
    case _EXPOSURE_MODE_CALIB_MODE_KV:
        if(!getTubeCalibrationReady(0)) ready = false;
        break;
    case _EXPOSURE_MODE_CALIB_MODE_IA:
        if(!getTubeCalibrationReady(0)) ready = false;
        break;
    case _EXPOSURE_MODE_CALIB_MODE_EXPOSIMETER:
        if(!getDetectorCalibrationReady(0)) ready = false;
        break;
    case _EXPOSURE_MODE_CALIB_MODE_PROFILE:
        if(!getProfileCalibrationReady(0)) ready = false;
        break;
    case _EXPOSURE_MODE_ANALOG_MANUAL_EXPOSURE:
        if(!getManualExposureReady(0)) ready = false;
        break;
    case _EXPOSURE_MODE_ANALOG_COLLIMATION_EXPOSURE:
         if(!getCollimationCalibrationReady(0)) ready = false;
         break;

    default:
        ready=false;
        break;
    }


    // READY
    if(ready){        
        ApplicationDatabase.setData(_DB_XRAYPUSH_READY,(int) 1);

    }else {
        ApplicationDatabase.setData(_DB_XRAYPUSH_READY,(int) 0);
    }

}

void AnalogCalibPageOpen::xrayErrorInCommand(unsigned char code){
    if(!code) return;

    // Spegne il simbolo raggi in corso
    ApplicationDatabase.setData(_DB_XRAY_SYM,(unsigned char) 0, DBase::_DB_FORCE_SGN);

    // Attiva codice di errore auto ripristinante
    PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_RAGGI,code,true);

    return;
}
