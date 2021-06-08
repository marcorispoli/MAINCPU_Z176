
#include "invertertool.h"
#include "ui_inverter.h"


#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"


#define UI_PAGINA _PG_SERVICE_TOOLS_INVERTER
#define EXIT_PAGINA _PG_SERVICE_TOOLS_MENU
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000

invertertool::invertertool(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::inverterUI)
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
    timerRequest =0;

}

invertertool::~invertertool()
{
    delete ui;
}

// Funzione agganciata ai sistemi di menu custom
void invertertool::changePage(int pg,int opt)
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
void invertertool::onExitButton(void)
{

    GWindowRoot.setNewPage(EXIT_PAGINA,GWindowRoot.curPage,0);
}

void invertertool::onHSButton(void)
{
    ApplicationDatabase.setData(_DB_SERVICE15_INT,(int) 1);

}
void invertertool::onLSButton(void)
{
    ApplicationDatabase.setData(_DB_SERVICE15_INT,(int) 2);

}
void invertertool::onStopButton(void)
{
    ApplicationDatabase.setData(_DB_SERVICE15_INT,(int) 3);

}
void invertertool::initPage(void){


    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
    connect(ui->buttonHS,SIGNAL(released()),this,SLOT(onHSButton()),Qt::UniqueConnection);
    connect(ui->buttonLS,SIGNAL(released()),this,SLOT(onLSButton()),Qt::UniqueConnection);
    connect(ui->buttonStop,SIGNAL(released()),this,SLOT(onStopButton()),Qt::UniqueConnection);

    ui->anodeHU->setText(QString("ANODE HU:%1(%)").arg(ApplicationDatabase.getDataI(_DB_HU_ANODE)* 100 / 300));
    if(ApplicationDatabase.getDataU(_DB_SYSTEM_CONFIGURATION)&_ARCH_HIGH_SPEED_STARTER){
        ui->labelARmodel->setText(QString("MODEL:HIGH SPEED"));
        ui->buttonHS->show();
    }else{
        ui->labelARmodel->setText(QString("MODEL:LOW SPEED"));
        ui->buttonHS->hide();
    }


    // Inizializzazioni competono allo Slave
    if(isMaster){
        ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) -1,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) -1,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) -1,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) -1,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_SERVICE5_INT,(int) -1,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_SERVICE6_INT,(int) -1,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_SERVICE7_INT,(int) -1,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_SERVICE8_INT,(int) -1,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_SERVICE9_INT,(int) -1,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_SERVICE10_INT,(int) -1,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_SERVICE11_INT,(int) -1,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_SERVICE12_INT,(int) -1,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_SERVICE13_INT,(int) -1,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_SERVICE14_INT,(int) -1,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_SERVICE15_INT, (int) 0, DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_SERVICE1_STR, pConfig->userCnf.tubeFileName, DBase::_DB_FORCE_SGN);
        //connect(pConsole,SIGNAL(mccPcb190Notify(unsigned char,unsigned char,QByteArray)),this,SLOT(masterNotifySlot(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

        timerRequest = startTimer(1000);

    }

}

// Operazioni da compiere all'uscita dalla pagina
void invertertool::exitPage(void){


    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
    disconnect(ui->buttonHS,SIGNAL(released()),this,SLOT(onHSButton()));
    disconnect(ui->buttonLS,SIGNAL(released()),this,SLOT(onLSButton()));
    disconnect(ui->buttonStop,SIGNAL(released()),this,SLOT(onStopButton()));

    if(isMaster){
        //disconnect(pConsole,SIGNAL(mccPcb190Notify(unsigned char,unsigned char,QByteArray)),this,SLOT(masterNotifySlot(unsigned char,unsigned char,QByteArray)));

        if(timerRequest){
            killTimer(timerRequest);
            timerRequest = 0;
        }
    }
    
}

void invertertool::timerEvent(QTimerEvent* ev)
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

    if(ev->timerId()==timerRequest)
    {

        mainPowerUpdate();
       // pConfig->pSlaveMcc->sendFrame(MCC_GET_ARM_INPUTS,1,data,1); // attiva la modalità di calibrazione potenziometro

    }
}

// FUNZIONE DI AGGIORNAMENTO CAMPI VALORE CONNESSO AI CAMPI DEL DATABASE

void invertertool::valueChanged(int index,int opt)
{
    if((isMaster)&&(opt&DBase::_DB_ONLY_SLAVE_ACTION)) return;
    if((!isMaster)&&(opt&DBase::_DB_ONLY_MASTER_ACTION)) return;

    switch(index){

    case   _DB_SERVICE1_STR:
        ui->labelTube->setText(QString("SELECTED TUBE: %1").arg(ApplicationDatabase.getDataS(_DB_SERVICE1_STR)));
        break;
    case _DB_SERVICE1_INT:
        ui->label560->setText(QString("[+560V]:%1(V)").arg(ApplicationDatabase.getDataI(_DB_SERVICE1_INT)));// Tensione di BUS
        break;
    case _DB_SERVICE2_INT:
        ui->labelT->setText(QString("T° AMP:%1(°)").arg(ApplicationDatabase.getDataI(_DB_SERVICE2_INT)));// Temperatura amplificatore
        break;
    case _DB_SERVICE3_INT:
        ui->labelTubeTemp->setText(QString("TUBE TEMP:%1(°)").arg(ApplicationDatabase.getDataI(_DB_SERVICE3_INT))); // Temperatura cuffia
        break;
    case _DB_SERVICE4_INT:
        ui->labelFilament->setText(QString("CURRENT:%1(mA)").arg(ApplicationDatabase.getDataI(_DB_SERVICE4_INT)));// Corrente di filamento
        break;
    case _DB_SERVICE5_INT:
       ui->mAsTest->setText(QString("mAs TEST:%1").arg(ApplicationDatabase.getDataI(_DB_SERVICE5_INT))); // mAs di test in IDLE
       break;

    case _DB_SERVICE6_INT:
       ui->iTest->setText(QString("I TEST:%1(mA)").arg(ApplicationDatabase.getDataI(_DB_SERVICE6_INT))); // mAs di test in IDLE
       break;

    case _DB_SERVICE7_INT:
        if(ApplicationDatabase.getDataI(_DB_SERVICE7_INT)==1){
            ui->faultFrame->show();
            ui->ground->show();
            ui->r16->hide();
        }else if(ApplicationDatabase.getDataI(_DB_SERVICE7_INT)==2){
            ui->faultFrame->show();
            ui->ground->hide();
            ui->r16->show();
        }else{
            ui->faultFrame->hide();
        }
       break;
    case _DB_SERVICE8_INT:
        ui->label33->setText(QString("[+33V]:%1(V)").arg(ApplicationDatabase.getDataI(_DB_SERVICE8_INT)));
        break;

    case _DB_SERVICE9_INT:
        ui->labelm33->setText(QString("[-33V]:%1(V)").arg(ApplicationDatabase.getDataI(_DB_SERVICE9_INT)));
        break;

    case _DB_SERVICE10_INT:
        ui->label12->setText(QString("[+12V]:%1(V)").arg(ApplicationDatabase.getDataI(_DB_SERVICE10_INT)));
        break;

    case _DB_SERVICE11_INT:
        ui->labelm12->setText(QString("[-12V]:%1(V)").arg(ApplicationDatabase.getDataI(_DB_SERVICE11_INT)));
        break;

    case _DB_SERVICE12_INT:
        ui->label15->setText(QString("[+15V]:%1(V)").arg(ApplicationDatabase.getDataI(_DB_SERVICE12_INT)));
        break;
    case _DB_SERVICE13_INT:
        ui->label15ext->setText(QString("[+15Vext]:%1(V)").arg(ApplicationDatabase.getDataI(_DB_SERVICE13_INT)));
        break;
    case _DB_SERVICE14_INT:
        if(ApplicationDatabase.getDataI(index)==0) ui->labelFocus->setText("FOCUS: OFF");
        else if(ApplicationDatabase.getDataI(index)==1) ui->labelFocus->setText("FOCUS: LARGE");
        else if(ApplicationDatabase.getDataI(index)==2) ui->labelFocus->setText("FOCUS: SMALL");
        else ui->labelFocus->setText("FOCUS: ND");
        break;

    case _DB_SERVICE15_INT:
        if(!isMaster) return;
        if( ApplicationDatabase.getDataI(_DB_SERVICE15_INT)==1){ // HS
            pGeneratore->setStarter(2);
        }else if( ApplicationDatabase.getDataI(_DB_SERVICE15_INT)==2){ // LS
            pGeneratore->setStarter(1);
        }else{ // STOP
            pGeneratore->setStarter(0);
        }
        ApplicationDatabase.setData(_DB_SERVICE15_INT, (int) 0, DBase::_DB_NO_CHG_SGN);
        break;


    case _DB_HU_ANODE:
        ui->anodeHU->setText(QString("ANODE HU:%1(%)").arg(ApplicationDatabase.getDataI(index)* 100 / 300));
        break;
    }

}
void invertertool::mainPowerUpdate(void){


    // Monitoraggio HV
    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) pGeneratore->hvval);  // Tensione di BUS



    // Temperatura amplificatore
    ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) pGeneratore->tempAmplFil);    // Temperatura amplificatore

    // Temperatura cuffia
    int tCuffia = ApplicationDatabase.getDataI(_DB_T_CUFFIA) & 0x00FF;
    ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) tCuffia);        // Temperatura cuffia


    // Corrente di filamento
    float iFil = (float) pGeneratore->iFilamento;
    if(iFil<100) iFil = 0;
    ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) iFil);    // Corrente di filamento

    // mAs in IDLE dovuti alla corrente anodica di Test
    ApplicationDatabase.setData(_DB_SERVICE5_INT,(int) pGeneratore->mAsTest); // mAs di test in IDLE

    // Corrente anodica di Test in IDLE
    ApplicationDatabase.setData(_DB_SERVICE6_INT,(int) pGeneratore->anodicaTest); // mAs di test in IDLE


    // Verifiche diagnostiche
    if(pGeneratore->faultGnd){
        ApplicationDatabase.setData(_DB_SERVICE7_INT,(int) 1); // FAULT GROUND
    }else if(pGeneratore->faultR16){
        ApplicationDatabase.setData(_DB_SERVICE7_INT,(int) 2); // FAULT R16
    }else{
        ApplicationDatabase.setData(_DB_SERVICE7_INT,(int) 0); // No FAULT
    }


    // Tensioni scheda PCB190
    ApplicationDatabase.setData(_DB_SERVICE8_INT,(int)pGeneratore->v32);
    ApplicationDatabase.setData(_DB_SERVICE9_INT,(int)pGeneratore->vm32);
    ApplicationDatabase.setData(_DB_SERVICE10_INT,(int)pGeneratore->v12);
    ApplicationDatabase.setData(_DB_SERVICE11_INT,(int)pGeneratore->vm12);
    ApplicationDatabase.setData(_DB_SERVICE12_INT,(int)pGeneratore->v15);
    ApplicationDatabase.setData(_DB_SERVICE13_INT,(int)pGeneratore->v15ext);

    // FUOCO
    if(!(pGeneratore->flags0 & 8)){
        ApplicationDatabase.setData(_DB_SERVICE14_INT,(int) 0); // Fuoco OFF
    }else{
        if(pGeneratore->selectedFSize == Generatore::FUOCO_LARGE) {
            ApplicationDatabase.setData(_DB_SERVICE14_INT,(int) 1); // Large
        }else if(pGeneratore->selectedFSize == Generatore::FUOCO_SMALL) {
            ApplicationDatabase.setData(_DB_SERVICE14_INT,(int) 2); // Small
        }else{
            ApplicationDatabase.setData(_DB_SERVICE14_INT,(int) 3); // ND
        }
    }

}

