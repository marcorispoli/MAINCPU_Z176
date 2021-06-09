// CALIBSTARTER  CALIBSTARTER
// calibstarter  calibstarter
// calibStarterUI  calibStarterUI
// calibStarter  calibStarter

// _PG_SERVICE_CALIB_STARTER  _PG_SERVICE_CALIB_STARTER
// _PG_SERVICE_CALIB_MENU  _PG_SERVICE_CALIB_MENU


#include "calibstarter.h"
#include "ui_calibStarter.h"


#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"


#define UI_PAGINA _PG_SERVICE_CALIB_STARTER
#define EXIT_PAGINA _PG_SERVICE_CALIB_MENU
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000

calibstarter::calibstarter(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::calibStarterUI)
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

    this->parent = parent;
    this->rotview = rotview;

    // IMPOSTAZIONI STANDARD _______________________________________________________________________________
    connect(&GWindowRoot,SIGNAL(changePage(int,int)), this,SLOT(changePage(int,int)),Qt::UniqueConnection);
    connect(EXIT_BUTTON,SIGNAL(released()),this,SLOT(onExitButton()),Qt::UniqueConnection);

    //______________________________________________________________________________________________________


    timerDisable = 0;
    timerStored =0;
}

calibstarter::~calibstarter()
{
    delete ui;
}

// Funzione agganciata ai sistemi di menu custom
void calibstarter::changePage(int pg,  int opt)
{
    if(UI_PAGINA==pg)
    {

        // Attivazione pagina
        if(GWindowRoot.curPageVisible== TRUE){
            // Disabilitazione allarmi di sistema
            paginaAllarmi->alarm_enable = false;

            // Disabilita il pulsante di uscita per un certo tempo
            EXIT_BUTTON->hide();
            changePageTimer = startTimer(DISABLE_EXIT_TMO);
            view->show();
            initPage();

        }
        else view->hide();
        return;
    }
    else if(GWindowRoot.curPage==UI_PAGINA)
    {        
        // Disattivazione pagina
        paginaAllarmi->alarm_enable = true;
        view->hide();
        if(timerDisable){
            killTimer(timerDisable);
            timerDisable = 0;
        }
        exitPage();

    }

}


// Reazione alla pressione del pulsante di uscita
void calibstarter::onExitButton(void)
{
    if(ApplicationDatabase.getDataI(_DB_SERVICE7_INT)){ // Se lo store è attivo alora viene richiesto di salvare
        ApplicationDatabase.setData(_DB_SERVICE14_INT,1,DBase::_DB_FORCE_SGN); // Richiesta conferma
    }else
        GWindowRoot.setNewPage(EXIT_PAGINA,GWindowRoot.curPage,0);
}

/*
    genCnf.pcb190.cal_main_run = 0;
    genCnf.pcb190.cal_shift_run = 0;
    genCnf.pcb190.cal_main_keep = 0;
    genCnf.pcb190.cal_shift_keep = 0;
    genCnf.pcb190.cal_main_off = 0;
    genCnf.pcb190.cal_shift_off = 0;
    genCnf.pcb190.cal_main_brk = 0;
    genCnf.pcb190.cal_shift_brk = 0;

    genCnf.pcb190.cal_max_main_off = 255;
    genCnf.pcb190.cal_max_shift_off = 255;
    genCnf.pcb190.cal_max_main_run = 255;
    genCnf.pcb190.cal_max_shift_run = 255;
    genCnf.pcb190.cal_max_main_keep = 255;
    genCnf.pcb190.cal_max_shift_keep = 255;
    genCnf.pcb190.cal_max_main_brk = 255;
    genCnf.pcb190.cal_max_shift_brk = 255;


    genCnf.pcb190.cal_min_main_run = 0;
    genCnf.pcb190.cal_min_shift_run = 0;
    genCnf.pcb190.cal_min_main_keep = 0;
    genCnf.pcb190.cal_min_shift_keep = 0;
    genCnf.pcb190.cal_min_main_brk = 0;
    genCnf.pcb190.cal_min_shift_brk = 0;
  */
void calibstarter::initPage(void){


    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
    connect(ui->startButton,SIGNAL(released()),this,SLOT(onStartButton()),Qt::UniqueConnection);
    connect(ui->storeButton,SIGNAL(released()),this,SLOT(onStoreButton()),Qt::UniqueConnection);
    connect(ui->afterExposure,SIGNAL(released()),this,SLOT(onAfterExposure()),Qt::UniqueConnection);
    connect(ui->brakes,SIGNAL(released()),this,SLOT(onBrakes()),Qt::UniqueConnection);

    // Inizializzazione delle variabili
    if(isMaster){
        ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) -1,DBase::_DB_FORCE_SGN); // Corrente Main Run
        ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) -1,DBase::_DB_FORCE_SGN); // Corrente Main Keep
        ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) -1,DBase::_DB_FORCE_SGN); // Corrente Main Off
        ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) -1,DBase::_DB_FORCE_SGN); // Corrente Shift Run
        ApplicationDatabase.setData(_DB_SERVICE5_INT,(int) -1,DBase::_DB_FORCE_SGN); // Corrente Shift Keep
        ApplicationDatabase.setData(_DB_SERVICE6_INT,(int) -1,DBase::_DB_FORCE_SGN); // Corrente Shift Off
        ApplicationDatabase.setData(_DB_SERVICE7_INT,(int) 0,DBase::_DB_FORCE_SGN);  // ABILITAZIONE PULSANTE STORE
        ApplicationDatabase.setData(_DB_SERVICE8_INT,(int) 0,DBase::_DB_FORCE_SGN);  // VISUALIZZAZIONE WARNING
        ApplicationDatabase.setData(_DB_SERVICE9_INT,(int) 0,DBase::_DB_FORCE_SGN);  // VISUALIZZAZIONE DATA STORED

        // Configurazione per lo spegnimento dello starter
        if(pConfig->userCnf.starter_off_after_exposure) ApplicationDatabase.setData(_DB_SERVICE12_INT,(int) 0,DBase::_DB_FORCE_SGN);
        else ApplicationDatabase.setData(_DB_SERVICE12_INT,(int) 1,DBase::_DB_FORCE_SGN);

        if(pConfig->userCnf.starter_brake) ApplicationDatabase.setData(_DB_SERVICE13_INT,(int) 1,DBase::_DB_FORCE_SGN);
        else ApplicationDatabase.setData(_DB_SERVICE13_INT,(int) 0,DBase::_DB_FORCE_SGN);

        connect(pConsole,SIGNAL(mccServiceNotify(unsigned char,unsigned char, QByteArray)), this,SLOT(onServiceNotify(unsigned char,unsigned char, QByteArray)),Qt::UniqueConnection);
        pSysLog->log("SERVICE PANEL: CALIB STARTER");
    }

    ApplicationDatabase.setData(_DB_SERVICE10_INT,(int)0,DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO); // Pulsante Start
    ApplicationDatabase.setData(_DB_SERVICE11_INT,(int)0,DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO); // Pulsante Store

    timerDisable = startTimer(1000);
}

// Operazioni da compiere all'uscita dalla pagina
void calibstarter::exitPage(void){
    if(timerDisable) killTimer(timerDisable);
    if(timerStored) killTimer(timerStored);
    timerDisable=0;
    timerStored=0;

    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
    disconnect(ui->startButton,SIGNAL(released()),this,SLOT(onStartButton()));
    disconnect(ui->storeButton,SIGNAL(released()),this,SLOT(onStoreButton()));
    disconnect(ui->afterExposure,SIGNAL(released()),this,SLOT(onAfterExposure()));
    disconnect(ui->brakes,SIGNAL(released()),this,SLOT(onBrakes()));

    if(isMaster){
        disconnect(pConsole,SIGNAL(mccServiceNotify(unsigned char,unsigned char, QByteArray)), this,SLOT(onServiceNotify(unsigned char,unsigned char, QByteArray)));

        // Controllo se sono cambiati i parametri di uso dellko starter
        if(     ((pConfig->userCnf.starter_off_after_exposure)==((bool) ApplicationDatabase.getDataI(_DB_SERVICE12_INT))) ||
                ((pConfig->userCnf.starter_brake)!=((bool) ApplicationDatabase.getDataI(_DB_SERVICE13_INT)))    ){
            pConfig->userCnf.starter_off_after_exposure = !((bool) ApplicationDatabase.getDataI(_DB_SERVICE12_INT));
            pConfig->userCnf.starter_brake = ((bool) ApplicationDatabase.getDataI(_DB_SERVICE13_INT));
            // Aggiunta configurazione dello starter
            pConfig->saveUserCfg();
        }

        pGeneratore->genCnf.pcb190.starter_off_after_exposure = pConfig->userCnf.starter_off_after_exposure;
        pGeneratore->genCnf.pcb190.starter_off_with_brake = pConfig->userCnf.starter_brake;
        pConfig->updatePCB190();
    }

}

void calibstarter::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId()==changePageTimer)
    {
        killTimer(changePageTimer);
        // Abilita il pulsante di uscita
        EXIT_BUTTON->show();
    }

    if(ev->timerId()==timerDisable)
    {
        killTimer(timerDisable);
        timerDisable=0;
    }

    if(ev->timerId()==timerStored)
    {
        killTimer(timerStored);
        timerStored=0;
        ui->msgstored->hide();
    }


}

// FUNZIONE DI AGGIORNAMENTO CAMPI VALORE CONNESSO AI CAMPI DEL DATABASE
void calibstarter::valueChanged(int index,int opt)
{
    int val = ApplicationDatabase.getDataI(index);

    switch(index){
    case _DB_SERVICE1_INT:
        if(val==-1) ui->mainRunCurrent->setText("--.-");
        else ui->mainRunCurrent->setText(QString("%1").arg(val));
        break;
    case _DB_SERVICE2_INT:
        if(val==-1) ui->mainKeepCurrent->setText("--.-");
        else ui->mainKeepCurrent->setText(QString("%1").arg(val));
        break;

    case _DB_SERVICE4_INT:
        if(val==-1) ui->shiftRunCurrent->setText("--.-");
        else ui->shiftRunCurrent->setText(QString("%1").arg(val));
        break;
    case _DB_SERVICE5_INT:
        if(val==-1) ui->shiftKeepCurrent->setText("--.-");
        else ui->shiftKeepCurrent->setText(QString("%1").arg(val));
        break;
    case _DB_SERVICE7_INT:
        if(ApplicationDatabase.getDataI(index))  ui->storeButton->show();
        else  ui->storeButton->hide();
        break;
    case _DB_SERVICE8_INT:
        if(ApplicationDatabase.getDataI(index))  ui->msglabel->show();
        else  ui->msglabel->hide();
        break;
    case _DB_SERVICE9_INT:
        if(ApplicationDatabase.getDataI(index)){
            ui->msgstored->show();
            if(!timerStored) timerStored = startTimer(3000);
        }else  ui->msgstored->hide();
        break;

    case _DB_SERVICE10_INT:
        if(!isMaster) return;
        if(ApplicationDatabase.getDataI(index)==0) return;
        activateTest();
    break;

    case _DB_SERVICE11_INT:
        if(ApplicationDatabase.getDataI(index)==0) return;
        ApplicationDatabase.setData(_DB_SERVICE7_INT,(int) 0); // Disabilita il pulsante di store immediatamente
        if(isMaster) storeData();
    break;

    case _DB_SERVICE12_INT:
        if(ApplicationDatabase.getDataI(index))  ui->afterExposure->setStyleSheet("background-image:url(:/transparent.png);border-image:url(:/Pulsanti/Pulsanti/but_on.png);");
        else ui->afterExposure->setStyleSheet("background-image:url(:/transparent.png);border-image:url(:/Pulsanti/Pulsanti/but_off.png);");
        break;
    case _DB_SERVICE13_INT:
        if(ApplicationDatabase.getDataI(index))  ui->brakes->setStyleSheet("background-image:url(:/transparent.png);border-image:url(:/Pulsanti/Pulsanti/but_on.png);");
        else ui->brakes->setStyleSheet("background-image:url(:/transparent.png);border-image:url(:/Pulsanti/Pulsanti/but_off.png);");
        break;
    case _DB_SERVICE14_INT:
        if(!isMaster) return;
        connect(pWarningBox,SIGNAL(buttonCancSgn(void)),this,SLOT(noStoreSlot(void)),Qt::UniqueConnection); // Ogni volta va connesso..
        connect(pWarningBox,SIGNAL(buttonOkSgn()),this,SLOT(storeSlot(void)),Qt::UniqueConnection); // Ogni volta va connesso..
        pWarningBox->activate("EXIT BUTTON WARNING","DO YOU WANT TO STORE\nTHE LAST CALIBRATION?",msgBox::_BUTTON_CANC|msgBox::_BUTTON_OK);
        break;
    }
}
void calibstarter::noStoreSlot(void){
    GWindowRoot.setNewPage(EXIT_PAGINA,GWindowRoot.curPage,0);
}

void calibstarter::storeSlot(void){
    storeData();
    GWindowRoot.setNewPage(EXIT_PAGINA,GWindowRoot.curPage,0);
}

void calibstarter::onStartButton(void){
    if(timerDisable) return;
    timerDisable = startTimer(1000);
    ApplicationDatabase.setData(_DB_SERVICE10_INT,(int) 1,DBase::_DB_FORCE_SGN);
}

void calibstarter::onStoreButton(void){
    if(timerDisable) return;
    timerDisable = startTimer(1000);
    ApplicationDatabase.setData(_DB_SERVICE11_INT,(int) 1,DBase::_DB_FORCE_SGN);
}


void calibstarter::onAfterExposure(void){
    if(timerDisable) return;
    timerDisable = startTimer(1000);

    if(ApplicationDatabase.getDataI(_DB_SERVICE12_INT))
        ApplicationDatabase.setData(_DB_SERVICE12_INT,(int) 0);
    else ApplicationDatabase.setData(_DB_SERVICE12_INT,(int) 1);
}

void calibstarter::onBrakes(void){
    if(timerDisable) return;
    timerDisable = startTimer(1000);
    if(ApplicationDatabase.getDataI(_DB_SERVICE13_INT))
        ApplicationDatabase.setData(_DB_SERVICE13_INT,(int) 0);
    else ApplicationDatabase.setData(_DB_SERVICE13_INT,(int) 1);
}


void calibstarter::activateTest(void){
    QByteArray buf;
    buf.append((unsigned char) SRV_TEST_LS_STARTER); // Comando di servizio

    // Invia il comando per fermare il movimento
    if(pConsole->pGuiMcc->sendFrame(MCC_SERVICE,0,(unsigned char*) buf.data(), buf.size())==false) return;

    pWarningBox->activate("LOW SPEED STARTER TEST","TEST ACTIVATED.\nWAIT ABOUT 10 SECONDS...");
    pWarningBox->setTimeout(11000);

}

void calibstarter::storeData(void){


    // Valori di corrente misurati durante l'esposizione
    pGeneratore->genCnf.pcb190.cal_main_run = main_run;
    pGeneratore->genCnf.pcb190.cal_shift_run=shift_run;
    pGeneratore->genCnf.pcb190.cal_main_keep=main_keep;
    pGeneratore->genCnf.pcb190.cal_shift_keep=shift_keep;
    pGeneratore->genCnf.pcb190.cal_main_off=main_off;
    pGeneratore->genCnf.pcb190.cal_shift_off=shift_off;

    pGeneratore->genCnf.pcb190.cal_max_main_off=cal_max_main_off;
    pGeneratore->genCnf.pcb190.cal_max_shift_off=cal_max_shift_off;
    pGeneratore->genCnf.pcb190.cal_max_main_run=cal_max_main_run;
    pGeneratore->genCnf.pcb190.cal_max_shift_run=cal_max_shift_run;
    pGeneratore->genCnf.pcb190.cal_max_main_keep=cal_max_main_keep;
    pGeneratore->genCnf.pcb190.cal_max_shift_keep=cal_max_shift_keep;
    pGeneratore->genCnf.pcb190.cal_max_main_brk=255;
    pGeneratore->genCnf.pcb190.cal_max_shift_brk=255;

    pGeneratore->genCnf.pcb190.cal_min_main_run=cal_min_main_run;
    pGeneratore->genCnf.pcb190.cal_min_shift_run=cal_min_shift_run;
    pGeneratore->genCnf.pcb190.cal_min_main_keep=cal_min_main_keep;
    pGeneratore->genCnf.pcb190.cal_min_shift_keep=cal_min_shift_keep;
    pGeneratore->genCnf.pcb190.cal_min_main_brk=0;
    pGeneratore->genCnf.pcb190.cal_min_shift_brk=0;

    pGeneratore->saveStarterFile();
    ApplicationDatabase.setData(_DB_SERVICE9_INT,(int) 1,DBase::_DB_FORCE_SGN);

}
/*
 *
         buffer[0] = _DEVREGL(RG190_AR_MAIN_IRUN,PCB190_CONTEST);
         buffer[1] = _DEVREGL(RG190_AR_SHIFT_IRUN,PCB190_CONTEST);
         buffer[2] = _DEVREGL(RG190_AR_MAIN_IKEEP,PCB190_CONTEST);
         buffer[3] = _DEVREGL(RG190_AR_SHIFT_IKEEP,PCB190_CONTEST);
         buffer[4] = _DEVREGL(RG190_AR_MAIN_IOFF,PCB190_CONTEST);
         buffer[5] = _DEVREGL(RG190_AR_SHIFT_IOFF,PCB190_CONTEST);

        ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) -1,DBase::_DB_FORCE_SGN); // Corrente Main Run
        ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) -1,DBase::_DB_FORCE_SGN); // Corrente Main Keep
        ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) -1,DBase::_DB_FORCE_SGN); // Corrente Main Off
        ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) -1,DBase::_DB_FORCE_SGN); // Corrente Shift Run
        ApplicationDatabase.setData(_DB_SERVICE5_INT,(int) -1,DBase::_DB_FORCE_SGN); // Corrente Shift Keep
        ApplicationDatabase.setData(_DB_SERVICE6_INT,(int) -1,DBase::_DB_FORCE_SGN); // Corrente Shift Off

*/
void calibstarter::onServiceNotify(unsigned char id, unsigned char mcccode, QByteArray buffer)
{
    int min_keep;
    if(mcccode != SRV_TEST_LS_STARTER) return;

    main_run = buffer[0];
    shift_run = buffer[1];
    main_keep = buffer[2];
    shift_keep = buffer[3];
    main_off = buffer[4];
    shift_off = buffer[5];



    int mainrun_ma = buffer[0] * 25 * 1000 / 255; // Converte in mA
    ApplicationDatabase.setData(_DB_SERVICE1_INT,mainrun_ma); // Corrente Main Run

    int shiftrun_ma = buffer[1] * 25 * 1000/ 255; // Converte in mA
    ApplicationDatabase.setData(_DB_SERVICE4_INT,shiftrun_ma); // Corrente Shift Run

    int mainkeep_ma = buffer[2] * 25 * 1000/ 255; // Converte in mA
    ApplicationDatabase.setData(_DB_SERVICE2_INT,mainkeep_ma); // Corrente Main Keep

    int shiftkeep_ma = buffer[3] * 25 * 1000/ 255; // Converte in mA
    ApplicationDatabase.setData(_DB_SERVICE5_INT,shiftkeep_ma); // Corrente Shift Keep

    // Condizioni di accettazione dei risultati: dipende se il parametro di abilitazione keep è on/off
    if(ApplicationDatabase.getDataI(_DB_SERVICE12_INT)) min_keep = 3500;
    else min_keep = 0;

    // Condizioni di accettazione dei risultati
    if((mainrun_ma < 8000) || (shiftrun_ma < 5000) || (mainkeep_ma < min_keep)){
        ApplicationDatabase.setData(_DB_SERVICE7_INT,(int) 0,DBase::_DB_FORCE_SGN);  // ABILITAZIONE PULSANTE STORE
        ApplicationDatabase.setData(_DB_SERVICE8_INT,(int) 1,DBase::_DB_FORCE_SGN);  // VISUALIZZAZIONE WARNING
    }else{
        ApplicationDatabase.setData(_DB_SERVICE7_INT,(int) 1,DBase::_DB_FORCE_SGN);  // ABILITAZIONE PULSANTE STORE
        ApplicationDatabase.setData(_DB_SERVICE8_INT,(int) 0,DBase::_DB_FORCE_SGN);  // VISUALIZZAZIONE WARNING
    }

    // Viene disabilitato per sicurezza
    cal_max_main_off = 255 ;
    cal_max_shift_off = 255;

    // Viene disabilitato il valore massimo di run
    cal_max_main_run = 255;
    cal_max_shift_run = 255;

    // Viene disabilitato il valore massimo di keep
    cal_max_main_keep = 255;
    cal_max_shift_keep = 255;

    // Viene impostato il controllo sulla corrente di picco minima di RUN su Main
    cal_min_main_run = (main_run + main_off) / 2;

    // Viene impostato il controllo sulla corrente di picco minima di RUN su Shift
    cal_min_shift_run = (shift_run + shift_off) / 2;

    // Viene impostato il controllo sulla corrente di picco minima  di KEEP su Main
    cal_min_main_keep = (main_keep + main_off) / 2;

    // Sullo shift nessun controllo poichè l'offset supera il valore letto
    cal_min_shift_keep = 0;

    return;
}
