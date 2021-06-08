// system  system
// systemUI  systemUI

// _PG_SYSTEM_SETUP  codice pagina
// _PG_MAIN  codice pagina exit

#include "system.h"
#include "ui_system.h"


#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"


#define UI_PAGINA _PG_SYSTEM_SETUP
#define EXIT_PAGINA _PG_MAIN
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000

systemPage::systemPage(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::systemUI)
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


    timerDisable = 0;
}

systemPage::~systemPage()
{
    delete ui;
}

// Funzione agganciata ai sistemi di menu custom
void systemPage::changePage(int pg,  int opt)
{
    if(UI_PAGINA==pg)
    {

        // Attivazione pagina
        if(GWindowRoot.curPageVisible== TRUE){
            // Disabilitazione allarmi di sistema
            paginaAllarmi->alarm_enable = false;

            // Disabilita il pulsante di uscita per un certo tempo
            // EXIT_BUTTON->hide();
            changePageTimer = startTimer(DISABLE_EXIT_TMO);
            view->show();
            if(opt & DBase::_DB_INIT_PAGE) initFactoryPage();
            else initServicePage();

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
void systemPage::onExitButton(void)
{
    ApplicationDatabase.setData(_DB_SYS_EXIT,(int)1 ,DBase::_DB_FORCE_SGN);

    //GWindowRoot.setNewPage(_PG_MAIN_ANALOG,GWindowRoot.curPage,0);
}

void systemPage::initFactoryPage(void){

    serviceMode = false;
    ui->frameAnodeStarter->show();
    ui->frameRot->show();
    ui->frameTilt->show();
    ui->frameClock->show();
    clockChg=false;

    pCalculator = new numericPad(rotview,view, parent);
    connect(pCalculator,SIGNAL(calcSgn(bool)),this,SLOT(clockSlot(bool)));

    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
    connect(EXIT_BUTTON,SIGNAL(released()),this,SLOT(onExitButton()),Qt::UniqueConnection);


    //______________________________________________________________________________________________________
    connect(ui->buttonLeftStarter,SIGNAL(released()),this,SLOT(onLeftStarter()),Qt::UniqueConnection);
    connect(ui->buttonRightStarter,SIGNAL(released()),this,SLOT(onRightStarter()),Qt::UniqueConnection);
    connect(ui->buttonRightRotation,SIGNAL(released()),this,SLOT(onRightRotation()),Qt::UniqueConnection);
    connect(ui->buttonLeftRotation,SIGNAL(released()),this,SLOT(onLeftRotation()),Qt::UniqueConnection);
    connect(ui->buttonRightTilt,SIGNAL(released()),this,SLOT(onRightTilt()),Qt::UniqueConnection);
    connect(ui->buttonLeftTilt,SIGNAL(released()),this,SLOT(onLeftTilt()),Qt::UniqueConnection);

    connect(ui->Yedit,SIGNAL(selectionChanged()),this,SLOT(onYedit()),Qt::UniqueConnection);
    connect(ui->Medit,SIGNAL(selectionChanged()),this,SLOT(onMedit()),Qt::UniqueConnection);
    connect(ui->Dedit,SIGNAL(selectionChanged()),this,SLOT(onDedit()),Qt::UniqueConnection);

    connect(ui->hedit,SIGNAL(selectionChanged()),this,SLOT(onhedit()),Qt::UniqueConnection);
    connect(ui->medit,SIGNAL(selectionChanged()),this,SLOT(onmedit()),Qt::UniqueConnection);
    connect(ui->sedit,SIGNAL(selectionChanged()),this,SLOT(onsedit()),Qt::UniqueConnection);

    if(!isMaster) return;

    ApplicationDatabase.setData(_DB_SYS_HS,(int) pConfig->sys.highSpeedStarter,DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_SYS_ARM,(int) pConfig->sys.armMotor,DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_SYS_TRX,(int) pConfig->sys.trxMotor,DBase::_DB_FORCE_SGN);


    ApplicationDatabase.setData(_DB_SYS_DATA_Y,(int) pConfig->year,DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_SYS_DATA_M,(int) pConfig->month,DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_SYS_DATA_D,(int) pConfig->day,DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_SYS_DATA_h,(int) pConfig->hour,DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_SYS_DATA_m,(int) pConfig->min,DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_SYS_DATA_s,(int) pConfig->sec,DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_SYS_CLOCK_CHG,(int) 0,DBase::_DB_FORCE_SGN);



}
void systemPage::initServicePage(void){

    // Disabilita i frame di architettura
    ui->frameAnodeStarter->hide();
    ui->frameRot->hide();
    ui->frameTilt->hide();
    ui->frameClock->show();
    serviceMode = true;
    clockChg=false;

    pCalculator = new numericPad(rotview,view, parent);
    connect(pCalculator,SIGNAL(calcSgn(bool)),this,SLOT(clockSlot(bool)));

    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
    connect(EXIT_BUTTON,SIGNAL(released()),this,SLOT(onExitButton()),Qt::UniqueConnection);

    connect(ui->Yedit,SIGNAL(selectionChanged()),this,SLOT(onYedit()),Qt::UniqueConnection);
    connect(ui->Medit,SIGNAL(selectionChanged()),this,SLOT(onMedit()),Qt::UniqueConnection);
    connect(ui->Dedit,SIGNAL(selectionChanged()),this,SLOT(onDedit()),Qt::UniqueConnection);

    connect(ui->hedit,SIGNAL(selectionChanged()),this,SLOT(onhedit()),Qt::UniqueConnection);
    connect(ui->medit,SIGNAL(selectionChanged()),this,SLOT(onmedit()),Qt::UniqueConnection);
    connect(ui->sedit,SIGNAL(selectionChanged()),this,SLOT(onsedit()),Qt::UniqueConnection);

    if(!isMaster) return;

    ApplicationDatabase.setData(_DB_SYS_DATA_Y,(int) pConfig->year,DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_SYS_DATA_M,(int) pConfig->month,DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_SYS_DATA_D,(int) pConfig->day,DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_SYS_DATA_h,(int) pConfig->hour,DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_SYS_DATA_m,(int) pConfig->min,DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_SYS_DATA_s,(int) pConfig->sec,DBase::_DB_FORCE_SGN);

}

// Operazioni da compiere all'uscita dalla pagina
void systemPage::exitPage(void){    
    if(timerDisable) killTimer(timerDisable);
    timerDisable = 0;

    // Elimina il calcolatore
    disconnect(pCalculator);
    pCalculator->deleteLater(); // importante !!!
    pCalculator = 0;


    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
    disconnect(EXIT_BUTTON,SIGNAL(released()),this,SLOT(onExitButton()));
    disconnect(ui->buttonLeftStarter,SIGNAL(released()),this,SLOT(onLeftStarter()));
    disconnect(ui->buttonRightStarter,SIGNAL(released()),this,SLOT(onRightStarter()));
    disconnect(ui->buttonRightRotation,SIGNAL(released()),this,SLOT(onRightRotation()));
    disconnect(ui->buttonLeftRotation,SIGNAL(released()),this,SLOT(onLeftRotation()));
    disconnect(ui->buttonRightTilt,SIGNAL(released()),this,SLOT(onRightTilt()));
    disconnect(ui->buttonLeftTilt,SIGNAL(released()),this,SLOT(onLeftTilt()));
    disconnect(ui->Yedit,SIGNAL(selectionChanged()),this,SLOT(onYedit()));
    disconnect(ui->Medit,SIGNAL(selectionChanged()),this,SLOT(onMedit()));
    disconnect(ui->Dedit,SIGNAL(selectionChanged()),this,SLOT(onDedit()));
    disconnect(ui->hedit,SIGNAL(selectionChanged()),this,SLOT(onhedit()));
    disconnect(ui->medit,SIGNAL(selectionChanged()),this,SLOT(onmedit()));
    disconnect(ui->sedit,SIGNAL(selectionChanged()),this,SLOT(onsedit()));


}

// Eseguita solo da Master!!!
void systemPage::exitFunction(void){
    if(clockChg){
        // Aggiorna l'orologio di sistema
        pConfig->year = ApplicationDatabase.getDataI(_DB_SYS_DATA_Y);
        pConfig->month = ApplicationDatabase.getDataI(_DB_SYS_DATA_M);
        pConfig->day = ApplicationDatabase.getDataI(_DB_SYS_DATA_D);
        pConfig->hour = ApplicationDatabase.getDataI(_DB_SYS_DATA_h);
        pConfig->min = ApplicationDatabase.getDataI(_DB_SYS_DATA_m);
        pConfig->sec = ApplicationDatabase.getDataI(_DB_SYS_DATA_s);
        pConfig->updateDate();
        pConfig->updateRTC();
    }

    if(serviceMode){

        GWindowRoot.setNewPage(_PG_SERVICE_MENU,GWindowRoot.curPage,DBase::_DB_INIT_PAGE);

    }else{

        // Salvataggio della configurazione e reboot della macchina
        pConfig->sys.highSpeedStarter = ApplicationDatabase.getDataI(_DB_SYS_HS);
        pConfig->sys.armMotor = ApplicationDatabase.getDataI(_DB_SYS_ARM);
        pConfig->sys.trxMotor = ApplicationDatabase.getDataI(_DB_SYS_TRX);
        pConfig->saveSysCfg();
        pConfig->executeReboot();
    }
}

void systemPage::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId()==changePageTimer)
    {
        killTimer(changePageTimer);
        // Abilita il pulsante di uscita
        //EXIT_BUTTON->show();
    }

    if(ev->timerId()==timerDisable)
    {
        killTimer(timerDisable);
        timerDisable=0;
    }
}

// FUNZIONE DI AGGIORNAMENTO CAMPI VALORE CONNESSO AI CAMPI DEL DATABASE
void systemPage::valueChanged(int index,int opt)
{
    switch(index)
    {
    case _DB_SYS_HS:
        ui->starter->setText(getStarterStr(ApplicationDatabase.getDataI(index)));
    break;
    case _DB_SYS_ARM:
        ui->rotation->setText(getRotationStr(ApplicationDatabase.getDataI(index)));
    break;  
    case _DB_SYS_TRX:
        ui->tilt->setText(getTiltStr(ApplicationDatabase.getDataI(index)));
    break;

    case _DB_SYS_DATA_Y:
       ui->Yedit->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
       clockChg = true;
    break;
    case _DB_SYS_DATA_M:
       ui->Medit->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        clockChg = true;
    break;
    case _DB_SYS_DATA_D:
       ui->Dedit->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        clockChg = true;
    break;
    case _DB_SYS_DATA_h:
       ui->hedit->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        clockChg = true;
    break;
    case _DB_SYS_DATA_m:
       ui->medit->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        clockChg = true;
    break;
    case _DB_SYS_DATA_s:
       ui->sedit->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        clockChg = true;
    break;
    case _DB_SYS_CLOCK_CHG:
        clockChg = false;
        break;
    case _DB_SYS_EXIT:
        if(!isMaster) return;
        exitFunction();
        break;

    }

}


void systemPage::onLeftStarter(void){
    int val = ApplicationDatabase.getDataI(_DB_SYS_HS);
    if(val) val = 0;
    else val = 1;
    ApplicationDatabase.setData(_DB_SYS_HS,val);
}
void systemPage::onRightStarter(void){
    onLeftStarter();
}


void systemPage::onLeftRotation(void){
    int val = ApplicationDatabase.getDataI(_DB_SYS_ARM);
    if(val) val = 0;
    else val = 1;
    ApplicationDatabase.setData(_DB_SYS_ARM,val);
}

void systemPage::onRightRotation(void){
    onLeftRotation();
}

void systemPage::onLeftTilt(void){
    int val = ApplicationDatabase.getDataI(_DB_SYS_TRX);
    if(val) val = 0;
    else val = 1;
    ApplicationDatabase.setData(_DB_SYS_TRX,val);
}

void systemPage::onRightTilt(void){
    onLeftTilt();
}


QString systemPage::getStarterStr(unsigned char val){
    if(val) return QString("HIGH SPEED STARTER");
    else return QString("LOW SPEED STARTER");
}

QString systemPage::getRotationStr(unsigned char val){
    if(val) return QString("ROTATION WITH MOTOR");
    else return QString("MANUAL ROTATION");
}

QString systemPage::getTiltStr(unsigned char val){
    if(val) return QString("TILT WITH MOTOR");
    else return QString("NO TILT");
}



void systemPage::onYedit(void){

    dataField=QString("%1").arg((int) ApplicationDatabase.getDataI(_DB_SYS_DATA_Y));
    pCalculator->activate(&dataField, QString("SET DATA: YEAR "), 1);

    ui->frame->setFocus();// Sposta il focus per evitare il cursore
}
void systemPage::onMedit(void){

    dataField=QString("%1").arg((int) ApplicationDatabase.getDataI(_DB_SYS_DATA_M));
    pCalculator->activate(&dataField, QString("SET DATA: MONTH "), 2);

    ui->frame->setFocus();// Sposta il focus per evitare il cursore
}
void systemPage::onDedit(void){

    dataField=QString("%1").arg((int) ApplicationDatabase.getDataI(_DB_SYS_DATA_D));
    pCalculator->activate(&dataField, QString("SET DATA: DAY "), 3);

    ui->frame->setFocus();// Sposta il focus per evitare il cursore
}
void systemPage::onhedit(void){

    dataField=QString("%1").arg((int) ApplicationDatabase.getDataI(_DB_SYS_DATA_h));
    pCalculator->activate(&dataField, QString("SET DATA: HOUR "), 4);

    ui->frame->setFocus();// Sposta il focus per evitare il cursore
}

void systemPage::onmedit(void){

    dataField=QString("%1").arg((int) ApplicationDatabase.getDataI(_DB_SYS_DATA_m));
    pCalculator->activate(&dataField, QString("SET DATA: MINUTES "), 5);

    ui->frame->setFocus();// Sposta il focus per evitare il cursore
}
void systemPage::onsedit(void){

    dataField=QString("%1").arg((int) ApplicationDatabase.getDataI(_DB_SYS_DATA_s));
    pCalculator->activate(&dataField, QString("SET DATA: SECONDS "), 6);

    ui->frame->setFocus();// Sposta il focus per evitare il cursore
}


void systemPage::clockSlot(bool stat){
    if(!stat) return;

    int val = dataField.toInt();

    if(pCalculator->activation_code == 1) { // Anno in 4 cifre
        if(val<0) val = 0;
        else if(val<100) val = 2000+val;


        if(val<2021) val = 2021;
        else if(val>2050) val = 2050;
        ApplicationDatabase.setData(_DB_SYS_DATA_Y,(int) val);

    }else if(pCalculator->activation_code == 2) { // Mese
        if(val<1) val=1;
        else if(val>12) val = 12;
        ApplicationDatabase.setData(_DB_SYS_DATA_M,(int) val);

    }else if(pCalculator->activation_code == 3) { // Giorno
        if(val<1) val=1;
        else if(val>31) val = 31;
        ApplicationDatabase.setData(_DB_SYS_DATA_D,(int) val);

    }else if(pCalculator->activation_code == 4) { // Ora
        if(val<0) val=0;
        else if(val>24) val = 24;
        ApplicationDatabase.setData(_DB_SYS_DATA_h,(int) val);
    }else if(pCalculator->activation_code == 5) { // Minuti
        if(val<0) val=0;
        else if(val>60) val = 60;
        ApplicationDatabase.setData(_DB_SYS_DATA_m,(int) val);
    }else if(pCalculator->activation_code == 6) { // secondi
        if(val<0) val=0;
        else if(val>60) val = 60;
        ApplicationDatabase.setData(_DB_SYS_DATA_s,(int) val);
    }
}
