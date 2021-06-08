
#include "armtool.h"
#include "ui_arm.h"


#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"


#define UI_PAGINA _PG_SERVICE_TOOLS_ARM
#define EXIT_PAGINA _PG_SERVICE_TOOLS_MENU
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000

armtool::armtool(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::armToolUI)
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

armtool::~armtool()
{
    delete ui;
}

// Funzione agganciata ai sistemi di menu custom
void armtool::changePage(int pg, int opt)
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
void armtool::onExitButton(void)
{

    GWindowRoot.setNewPage(EXIT_PAGINA,GWindowRoot.curPage,0);
}

void armtool::initPage(void){


    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);

    // Inizializzazioni competono allo Slave
    if(!isMaster){
        ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 0,DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) 0,DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) 0,DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) 0,DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_SERVICE5_INT,(int) -1,DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_SERVICE6_INT,(int) -1,DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_SERVICE7_INT,(int) -1,DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_SERVICE1_STR,"",DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_SERVICE2_STR,"",DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_SERVICE3_STR,"",DBase::_DB_FORCE_SGN);

        connect(pConfig,SIGNAL(mccSlaveNotifySgn(unsigned char,unsigned char,QByteArray)),this,SLOT(slaveNotifySlot(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

        timerRequest = startTimer(1000);

    }

}

// Operazioni da compiere all'uscita dalla pagina
void armtool::exitPage(void){


    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));

    if(!isMaster){
        disconnect(pConfig,SIGNAL(mccSlaveNotifySgn(unsigned char,unsigned char,QByteArray)),this,SLOT(slaveNotifySlot(unsigned char,unsigned char,QByteArray)));

        if(timerRequest){
            killTimer(timerRequest);
            timerRequest = 0;
        }
    }
    
}

void armtool::timerEvent(QTimerEvent* ev)
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

        unsigned char data[0];
        pConfig->pSlaveMcc->sendFrame(MCC_GET_ARM_INPUTS,1,data,1); // attiva la modalità di calibrazione potenziometro

    }
}

// FUNZIONE DI AGGIORNAMENTO CAMPI VALORE CONNESSO AI CAMPI DEL DATABASE
void armtool::valueChanged(int index,int opt)
{
    switch(index){
    case _DB_SERVICE1_INT:
        if(ApplicationDatabase.getDataI(index)) ui->in1->show();
        else ui->in1->hide();
        break;
    case _DB_SERVICE2_INT:
        if(ApplicationDatabase.getDataI(index)) ui->in2->show();
        else ui->in2->hide();
        break;
    case _DB_SERVICE3_INT:
        if(ApplicationDatabase.getDataI(index)) ui->in3->show();
        else ui->in3->hide();
        break;
    case _DB_SERVICE4_INT:
        if(ApplicationDatabase.getDataI(index)) ui->in4->show();
        else ui->in4->hide();
        break;
    case _DB_SERVICE5_INT:  // VBus (mV)
        if(ApplicationDatabase.getDataI(index)==-1) ui->vpower->setText("---(mV)");
        else ui->vpower->setText(QString("%1(mV)").arg(ApplicationDatabase.getDataI(index)));
       break;

    case _DB_SERVICE6_INT: // VLOGIC mV
        if(ApplicationDatabase.getDataI(index)==-1) ui->vlogic->setText("---");
        else if(ApplicationDatabase.getDataI(index)) ui->vlogic->setText(QString("ACTIVE"));
        else ui->vlogic->setText(QString("NOT ACTIVE"));
       break;

    case _DB_SERVICE7_INT: // Temp dC°
        if(ApplicationDatabase.getDataI(index)==-1) ui->temp->setText("---(°C)");
        else ui->temp->setText(QString("%1(°C)").arg(ApplicationDatabase.getDataI(index)/10));
       break;


    case _DB_SERVICE1_STR: // Err Class
        ui->errClass->setText(ApplicationDatabase.getDataS(index));
       break;
    case _DB_SERVICE2_STR: // Err Subclass
        ui->errSubclass->setText(ApplicationDatabase.getDataS(index));
       break;
    case _DB_SERVICE3_STR: // Err Code
        ui->errCode->setText(ApplicationDatabase.getDataS(index));
       break;

    }

}

// Slot di ricezione Notifiche MCC dal modulo Slave
void armtool::slaveNotifySlot(unsigned char id,unsigned char mcc_code,QByteArray data)
{
    unsigned long val;

    if(mcc_code==MCC_GET_ARM_INPUTS){
        // Dati relativi al potenziometro
        //val = data[0] + data[1]*256 + data[2]*256*256 + data[3]*256*256*256;

        // Verifica Inputs:
        if(data[0]&1) ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 1);
        else ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 0);

        if(data[0]&2) ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) 1);
        else ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) 0);

        if(data[0]&4) ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) 1);
        else ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) 0);

        if(data[0]&8) ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) 1);
        else ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) 0);

        // VBUS
        val = data[4] + data[5]*256 + data[6]*256*256 + data[7]*256*256*256;
        ApplicationDatabase.setData(_DB_SERVICE5_INT,(int) (val));

        // VLOGIC
        //val = data[8] + data[9]*256 + data[10]*256*256 + data[11]*256*256*256;
        ApplicationDatabase.setData(_DB_SERVICE6_INT,(int) data[8] );

        // TEMP in d°C
        val = data[12] + data[13]*256 + data[14]*256*256 + data[15]*256*256*256;
        ApplicationDatabase.setData(_DB_SERVICE7_INT,(int) (val));

        if(data[18]==0){
            ApplicationDatabase.setData(_DB_SERVICE1_STR,"No Errors");
            ApplicationDatabase.setData(_DB_SERVICE2_STR,"");
            ApplicationDatabase.setData(_DB_SERVICE3_STR,"");
        }else{
            ApplicationDatabase.setData(_DB_SERVICE1_STR, pConfig->getNanotecErrorClass(data[18]));
            ApplicationDatabase.setData(_DB_SERVICE2_STR, pConfig->getNanotecErrorSubClass(data[19]));
            ApplicationDatabase.setData(_DB_SERVICE3_STR, pConfig->getNanotecErrorCode(data[16] + data[17]*256));
        }
    }


}


