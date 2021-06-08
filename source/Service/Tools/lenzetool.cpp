
#include "lenzetool.h"
#include "ui_lenze.h"


#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"


#define UI_PAGINA _PG_SERVICE_TOOLS_LENZE
#define EXIT_PAGINA _PG_SERVICE_TOOLS_MENU
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000

lenzetool::lenzetool(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::lenzeToolUI)
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

lenzetool::~lenzetool()
{
    delete ui;
}

// Funzione agganciata ai sistemi di menu custom
void lenzetool::changePage(int pg,  int opt)
{
    if(UI_PAGINA==pg)
    {

        // Attivazione pagina
        if(GWindowRoot.curPageVisible== TRUE){
            // Disabilitazione alllenzei di sistema
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
void lenzetool::onExitButton(void)
{

    GWindowRoot.setNewPage(EXIT_PAGINA,GWindowRoot.curPage,0);
}

void lenzetool::initPage(void){


    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);

    // Inizializzazioni competono allo Slave
    if(!isMaster){
        ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 0,DBase::_DB_FORCE_SGN); // Inputs/Outputs
        ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) 0,DBase::_DB_FORCE_SGN); // Analog1 - Analog 2
        ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) 0,DBase::_DB_FORCE_SGN); // Thl
        ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) 0,DBase::_DB_FORCE_SGN); // Thh
        ApplicationDatabase.setData(_DB_SERVICE5_INT,(int) 0,DBase::_DB_FORCE_SGN); // VBUS
        ApplicationDatabase.setData(_DB_SERVICE6_INT,(int) 0,DBase::_DB_FORCE_SGN); // Temp
        ApplicationDatabase.setData(_DB_SERVICE1_STR,"",DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_SERVICE2_STR,"",DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_SERVICE3_STR,"",DBase::_DB_FORCE_SGN);

        connect(pConfig,SIGNAL(mccSlaveNotifySgn(unsigned char,unsigned char,QByteArray)),this,SLOT(slaveNotifySlot(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

        timerRequest = startTimer(1000);

    }

}

// Operazioni da compiere all'uscita dalla pagina
void lenzetool::exitPage(void){


    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));

    if(!isMaster){
        disconnect(pConfig,SIGNAL(mccSlaveNotifySgn(unsigned char,unsigned char,QByteArray)),this,SLOT(slaveNotifySlot(unsigned char,unsigned char,QByteArray)));

        if(timerRequest){
            killTimer(timerRequest);
            timerRequest = 0;
        }
    }
    
}

void lenzetool::timerEvent(QTimerEvent* ev)
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
        pConfig->pSlaveMcc->sendFrame(MCC_GET_LENZE_INPUTS,1,data,1); // attiva la modalità di calibrazione potenziometro

    }
}

// FUNZIONE DI AGGIORNAMENTO CAMPI VALORE CONNESSO AI CAMPI DEL DATABASE
void lenzetool::valueChanged(int index,int opt)
{
    unsigned int i;

    switch(index){
    case _DB_SERVICE1_INT: // Input - Output
        i=ApplicationDatabase.getDataI(index);

        if(i&0x1) ui->di1->show();
        else ui->di1->hide();
        if(i&0x2) ui->di2->show();
        else ui->di2->hide();
        if(i&0x4) ui->di3->show();
        else ui->di3->hide();
        if(i&0x8) ui->di4->show();
        else ui->di4->hide();
        if(i&0x10) ui->di5->show();
        else ui->di5->hide();
        if(i&0x20) ui->sto->show();
        else ui->sto->hide();
        if(i&0x40) ui->do1->show();
        else ui->do1->hide();

        break;

    case _DB_SERVICE2_INT: // An1 An2
        i=ApplicationDatabase.getDataI(index);
        ui->an1->setText(QString("%1").arg((unsigned char) i));
        ui->an2->setText(QString("%1").arg((unsigned char) (i>>8)));
        break;

    case _DB_SERVICE3_INT: // THL
        i=ApplicationDatabase.getDataI(index);
        ui->thl->setText(QString("%1").arg(i));
        break;

    case _DB_SERVICE4_INT: // THH
        i=ApplicationDatabase.getDataI(index);
        ui->thh->setText(QString("%1").arg( i));
        break;

    case _DB_SERVICE5_INT:  // VBus
        ui->vbus->setText(QString("%1 (V)").arg(ApplicationDatabase.getDataI(index)));
       break;

    case _DB_SERVICE6_INT: // Temp dC°
        ui->temp->setText(QString("%1(°C)").arg(ApplicationDatabase.getDataI(index)/10));
       break;


    case _DB_SERVICE1_STR: // Err Diagnostic
        ui->errDiagnostic->setText(ApplicationDatabase.getDataS(index));
       break;
    case _DB_SERVICE2_STR: // Err Internal
        ui->errInternal->setText(ApplicationDatabase.getDataS(index));
       break;

    }

}

// Slot di ricezione Notifiche MCC dal modulo Slave
void lenzetool::slaveNotifySlot(unsigned char id,unsigned char mcc_code,QByteArray data)
{
    unsigned int i;

    if(mcc_code==MCC_GET_LENZE_INPUTS){

        // Inputs: o1,sto,i5,i4,i3,i2,i1
        ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) data[0]);

        // An1 + An2
        i = (int) data[1] + (int) data[2] * 256;
        ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) i);

        // Thl
        i = (int) data[3] + (int) data[4] * 256;
        ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) i);

        // Thh
        i = (int) data[5] + (int) data[6] * 256;
        ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) i);

        // VBus AC
        ApplicationDatabase.setData(_DB_SERVICE5_INT,(int) data[7] + (int) data[8]*256);

        // TEMP in °C
        ApplicationDatabase.setData(_DB_SERVICE6_INT,(int) data[9] + (int) data[10]*256);

        if((data[11]==0)&&(data[12]==0)&&(data[13]==0))
        {
            ApplicationDatabase.setData(_DB_SERVICE1_STR,"No Errors");
            ApplicationDatabase.setData(_DB_SERVICE2_STR,"");
        }else{
            ApplicationDatabase.setData(_DB_SERVICE1_STR, pConfig->getI550DiagnosticErrorStr(data[11])); // Diagnostic error
            ApplicationDatabase.setData(_DB_SERVICE2_STR, pConfig->getI550ErrorString((int) data[12]+256*data[13])); // Internal
        }
    }


}




