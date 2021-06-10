#include "calibzerosetting.h"
#include "ui_calibzerosetting.h"

#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"
#include "../../../lib/msgbox.h"

#define UI_PAGINA _PG_SERVICE_CALIB_ZEROSETTING
#define EXIT_PAGINA _PG_SERVICE_CALIB_MENU
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000



calibzerosetting::calibzerosetting(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::calibzerosetting)
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

    // _____________________________________________________________________________________________________
    connect(ui->manualModeButton,SIGNAL(released()),this,SLOT(onManualModeButton()),Qt::UniqueConnection);
    connect(ui->trxZeroButton,SIGNAL(released()),this,SLOT(onTrxZero()),Qt::UniqueConnection);

    connect(ui->resetGonioButton,SIGNAL(released()),this,SLOT(onGonioZero()),Qt::UniqueConnection);


    ui->attesaFrame->setGeometry(0,0,800,480);
    ui->attesaFrame->hide();
    timerDisable = 0;
}

calibzerosetting::~calibzerosetting()
{
    delete ui;
}


// Funzione agganciata ai sistemi di menu custom
void calibzerosetting::changePage(int pg,int opt)
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
        // Esce dalla pagina mcorrente
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
void calibzerosetting::onExitButton(void)
{
    GWindowRoot.setNewPage(EXIT_PAGINA,GWindowRoot.curPage,0);
}

// Operazioni da compiere all'ingresso della pagina
void calibzerosetting::initPage(void){


    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);

    if(isMaster){
        connect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(guiNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
        pSysLog->log("SERVICE PANEL: ZERO SETTING");
    }

    timerDisable = startTimer(500);

    ui->attesaFrame->hide();
    // Inizializzazione del modo
    rotButtonsMode = CALIB_ZERO_MANUAL_ACTIVATION_ARM_STANDARD ;
    ApplicationDatabase.setData(_DB_SERVICE5_INT,(int) CALIB_ZERO_MANUAL_ACTIVATION_ARM_STANDARD,DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
    ui->labelManualButtons->setText("");
    updateManualMode();

    // Inizializzazione campo azione bottoni
    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 0,DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
    ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) 0,DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
    ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) 0,DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);


}

// Operazioni da compiere all'uscita dalla pagina
void calibzerosetting::exitPage(void){

    if(timerDisable) killTimer(timerDisable);
    timerDisable=0;
    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
    if(isMaster){
        disconnect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(guiNotify(unsigned char,unsigned char,QByteArray)));
    }

    rotButtonsMode = CALIB_ZERO_MANUAL_ACTIVATION_ARM_STANDARD ;
    updateManualMode();
}


void calibzerosetting::timerEvent(QTimerEvent* ev)
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
}

// FUNZIONE DI AGGIORNAMENTO CAMPI VALORE CONNESSO AI CAMPI DEL DATABASE
void calibzerosetting::valueChanged(int index,int opt)
{

    switch(index)
    {
    case _DB_SERVICE5_INT: // Activation mode
        rotButtonsMode = ApplicationDatabase.getDataI(_DB_SERVICE5_INT);
        if(rotButtonsMode==CALIB_ZERO_MANUAL_ACTIVATION_ARM_STANDARD) {
            ui->labelManualButtons->setText("");
        }else if(rotButtonsMode==CALIB_ZERO_MANUAL_ACTIVATION_ARM_CALIB){
            ui->labelManualButtons->setText("ARM");
        }else{
            ui->labelManualButtons->setText("TRX");
        }
        updateManualMode();
        break;
    case _DB_SERVICE1_INT: // TRX Zero Setting
        if(ApplicationDatabase.getDataI(index)==0) return;
        activateTrxZeroSetting();
        break;

    case _DB_SERVICE3_INT: // Gonio Reset
        if(ApplicationDatabase.getDataI(index)==0) return;
        activateGonioZeroSetting();
        break;

    case _DB_SERVICE4_INT: // Attesa
        if(ApplicationDatabase.getDataI(index)==0) ui->attesaFrame->hide();
        else ui->attesaFrame->show();
        break;
    }
}

//________________________________________________________________________
void calibzerosetting::onManualModeButton(void)
{
    ui->frame->setFocus();

    rotButtonsMode = ApplicationDatabase.getDataI(_DB_SERVICE5_INT);

    // Toggle fra le modalità
    if(rotButtonsMode==CALIB_ZERO_MANUAL_ACTIVATION_ARM_STANDARD) {
        rotButtonsMode=CALIB_ZERO_MANUAL_ACTIVATION_ARM_CALIB;
    }else if(rotButtonsMode==CALIB_ZERO_MANUAL_ACTIVATION_ARM_CALIB){
        rotButtonsMode=CALIB_ZERO_MANUAL_ACTIVATION_TRX_CALIB;
    }else{
        rotButtonsMode=CALIB_ZERO_MANUAL_ACTIVATION_ARM_STANDARD;
    }
    ApplicationDatabase.setData(_DB_SERVICE5_INT,(int) rotButtonsMode);

}


void calibzerosetting::onTrxZero(void)
{
    ui->frame->setFocus();
    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 1);

}


void calibzerosetting::onGonioZero(void)
{
    ui->frame->setFocus();
    ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) 1);

}

void calibzerosetting::updateManualMode(void){
    unsigned char buffer[2];
    if(!isMaster) return;

    buffer[0] = rotButtonsMode;
    pConsole->pGuiMcc->sendFrame(MCC_CALIB_ZERO,1,buffer, sizeof(buffer));
}


void calibzerosetting::activateTrxZeroSetting(void){
    unsigned char buffer[2];
    ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) 1);
    if(!isMaster) return;

    buffer[0] = CALIB_ZERO_ACTIVATE_TRX_ZERO_SETTING;
    pConsole->pGuiMcc->sendFrame(MCC_CALIB_ZERO,1,buffer, sizeof(buffer));
}



void calibzerosetting::activateGonioZeroSetting(void){
    unsigned char buffer[2];
    ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) 1);
    if(!isMaster) return;

    buffer[0] = CALIB_ZERO_ACTIVATE_GONIO_ZERO_SETTING;
    pConsole->pGuiMcc->sendFrame(MCC_CALIB_ZERO,1,buffer, sizeof(buffer));
}

void calibzerosetting::guiNotify(unsigned char id,unsigned char cmd, QByteArray buffer){
    if(id!=1) return;
    if(cmd==MCC_CALIB_ZERO){
        // buffer[0] == comando
        // buffer[1] == esito (255= comando in esecuzione, 0=fallito, 1=eseguito)
        switch(buffer[0]){
            case CALIB_ZERO_ACTIVATE_TRX_ZERO_SETTING:
                ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 0);
            break;

            case CALIB_ZERO_ACTIVATE_GONIO_ZERO_SETTING:
                ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) 0);
                ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) 0);
            break;
        }
    }else if(cmd==MCC_CMD_TRX){
        //buffer[0] = data[1]; // Codice esito proveniente dai drivers..
        //buffer[1] = data[2]; // sub codice in caso di errore da fault
        //mccGuiNotify(generalConfiguration.trxExecution.id,MCC_CMD_TRX,buffer,2);
        ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) 0);

    }
}
