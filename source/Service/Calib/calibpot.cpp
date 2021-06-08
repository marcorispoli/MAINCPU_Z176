// <TAG1>  CALIBPOT
// calibpot  calibpot
// calibPotUI  CalibpotUI
// Calibpot  Calibpot

// _PG_SERVICE_CALIB_LENZE_POT  _PG_SERVICE_CALIB_LENZE_POT
// _PG_SERVICE_CALIB_MENU  _PG_SERVICE_CALIB_MENU


#include "calibpot.h"
#include "ui_calibpot.h"


#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"


#define UI_PAGINA _PG_SERVICE_CALIB_LENZE_POT
#define EXIT_PAGINA _PG_SERVICE_CALIB_MENU
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000

calibpot::calibpot(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::calibPotUI)
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


    ui->frame_init->setGeometry(QRect(635,205,161,206));  // Init
    ui->frame_step1->setGeometry(QRect(635,205,161,161)); // Set potenziometero 50%
    ui->frame_step2->setGeometry(QRect(635,205,161,161)); // Muovere il braccio fino al limite
    ui->frame_step3->setGeometry(QRect(635,205,161,161)); // Set potenziometro al 15%
    ui->frame_step4->setGeometry(QRect(635,205,161,161)); // Test and modify
    ui->frame_next->setGeometry(QRect(180,310,376,106)); // Test and modify
    ui->startButton->setGeometry(315,315,100,100);

    connect(ui->startButton,SIGNAL(released()),this,SLOT(onStartButton()),Qt::UniqueConnection);
    connect(ui->nextButton,SIGNAL(released()),this,SLOT(onNextButton()),Qt::UniqueConnection);
    connect(ui->prevButton,SIGNAL(released()),this,SLOT(onPrevButton()),Qt::UniqueConnection);
    connect(ui->storeButton,SIGNAL(released()),this,SLOT(onStoreButton()),Qt::UniqueConnection);
    connect(ui->calibPositionLow,SIGNAL(selectionChanged()),this,SLOT(onCalibPositionLow()),Qt::UniqueConnection);
    connect(ui->calibPositionHigh,SIGNAL(selectionChanged()),this,SLOT(onCalibPositionHigh()),Qt::UniqueConnection);

    timerDisable = 0;
}

calibpot::~calibpot()
{
    delete ui;
}

// Funzione agganciata ai sistemi di menu custom
void calibpot::changePage(int pg,  int opt)
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
void calibpot::onExitButton(void)
{

    GWindowRoot.setNewPage(EXIT_PAGINA,GWindowRoot.curPage,0);
}

void calibpot::initPage(void){
    unsigned char data[2];



    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);

    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 1000, DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 0);

    ApplicationDatabase.setData(_DB_SERVICE2_INT,2000, DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
    ApplicationDatabase.setData(_DB_SERVICE3_INT,2000, DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
    ApplicationDatabase.setData(_DB_SERVICE4_INT,2000, DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
    ApplicationDatabase.setData(_DB_SERVICE5_INT,2000, DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
    ApplicationDatabase.setData(_DB_SERVICE6_INT,2000, DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO); // Store

    if(!isMaster){
        connect(pConfig,SIGNAL(mccSlaveNotifySgn(unsigned char,unsigned char,QByteArray)),this,SLOT(slaveNotifySlot(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

        // Solo lo Slave deve inizializzare la parte che segue
        data[0] = 1;
        pConfig->pSlaveMcc->sendFrame(MCC_CALIB_LENZE,1,data,1); // attiva la modalità di calibrazione potenziometro
    }

    if(isMaster){
        ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) pConfig->lenzeConfig.min_lenze_position);
        ApplicationDatabase.setData(_DB_SERVICE5_INT,(int) pConfig->lenzeConfig.max_lenze_position);
        pSysLog->log("SERVICE PANEL: CALIB UP/DOWN POSITION");
    }

    pCalculator = new numericPad(rotview,view, parent);
    connect(pCalculator,SIGNAL(calcSgn(bool)),this,SLOT(calculatorSlot(bool)));

}

// Operazioni da compiere all'uscita dalla pagina
void calibpot::exitPage(void){
unsigned char data[2];

    if(timerDisable) killTimer (timerDisable);
    timerDisable=0;
    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));

    if(!isMaster){
        // Solo lo Slave deve inizializzare la parte che segue
        data[0] = 0;
        pConfig->pSlaveMcc->sendFrame(MCC_CALIB_LENZE,1,data,1); // disattiva la modalità di calibrazione potenziometro
        disconnect(pConfig,SIGNAL(mccSlaveNotifySgn(unsigned char,unsigned char,QByteArray)),this,SLOT(slaveNotifySlot(unsigned char,unsigned char,QByteArray)));

    }

    disconnect(pCalculator);
    pCalculator->deleteLater(); // importante !!!
    pCalculator = 0;
}

void calibpot::timerEvent(QTimerEvent* ev)
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
void calibpot::valueChanged(int index,int opt)
{
    switch(index){
    case _DB_SERVICE1_INT:
        ui->frame_init->hide();
        ui->frame_step1->hide();
        ui->frame_step2->hide();
        ui->frame_step3->hide();
        ui->frame_step4->hide();
        ui->frame_next->hide();
        ui->startButton->hide();

        switch(ApplicationDatabase.getDataI(index)){
        case 0: ui->frame_init->show();ui->startButton->show();break;
        case 1: ui->frame_step1->show();  ui->frame_next->show(); break;
        case 2: ui->frame_step2->show(); ui->frame_next->show(); break;
        case 3: ui->frame_step3->show(); ui->frame_next->show(); break;
        case 4: ui->frame_step4->show(); ui->frame_next->show(); break;
        }

        break;
    case _DB_SERVICE2_INT: // Potenziometro 1
        ui->an1Label->setText(QString("%1\%").arg(QString::number((float) ApplicationDatabase.getDataI(index)/10,'f',1)));
        break;
    case _DB_SERVICE3_INT: // Potenziometro 2
        ui->an2Label->setText(QString("%1\%").arg(QString::number((float) ApplicationDatabase.getDataI(index)/10,'f',1)));
        break;
    case _DB_SERVICE4_INT: // Soglia Bassa
        ui->calibPositionLow->setText(QString("%1\%").arg(ApplicationDatabase.getDataI(index)));
        break;
    case _DB_SERVICE5_INT: // Soglia Alta
        ui->calibPositionHigh->setText(QString("%1\%").arg(ApplicationDatabase.getDataI(index)));
        break;
    case _DB_SERVICE6_INT:
        ApplicationDatabase.setData(_DB_SERVICE6_INT,(int) 0, DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
        if(!isMaster) return;

        pWarningBox->activate(QString("CALIB LENZE"), QString("Thresholds stored!\nThe new thresholds are now operating"));
        pWarningBox->setTimeout(3000);

        pConfig->lenzeConfig.min_lenze_position = ApplicationDatabase.getDataI(_DB_SERVICE4_INT);
        pConfig->lenzeConfig.max_lenze_position = ApplicationDatabase.getDataI(_DB_SERVICE5_INT);
        pConfig->saveLenzeConfig();
        pConfig->updateLenzeDriver();
        break;
    }
}

void calibpot::onStartButton(void){
    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 1);
}
void calibpot::onPrevButton(void){
    int step = ApplicationDatabase.getDataI(_DB_SERVICE1_INT);
    if(step == 0) step = 4;
    else step--;

    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) step);
}
void calibpot::onNextButton(void){
    int step = ApplicationDatabase.getDataI(_DB_SERVICE1_INT)+1;
    if(step == 5) step =0;
    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) step);
}


void calibpot::onStoreButton(void){
    ApplicationDatabase.setData(_DB_SERVICE6_INT,(int) 1); // Store
}

// Slot di ricezione Notifiche MCC dal modulo Slave
void calibpot::slaveNotifySlot(unsigned char id,unsigned char mcc_code,QByteArray data)
{

    if(mcc_code==MCC_CALIB_LENZE){
        // Dati relativi al potenziometro
        int analog1 = data[0] + data[1]*256;
        int analog2 = data[2] + data[3]*256;
        ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) analog1);
        ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) analog2);

    }


}

void calibpot::onCalibPositionLow(void){
    dataField = QString("%1").arg(ApplicationDatabase.getDataI(_DB_SERVICE4_INT));
    pCalculator->activate(&dataField, QString("SET LOWER THRESHOLD [0:30])"), CALIB_POT_LOW_THRESHOLD);
}
void calibpot::onCalibPositionHigh(void){
    dataField = QString("%1").arg(ApplicationDatabase.getDataI(_DB_SERVICE5_INT));
    pCalculator->activate(&dataField, QString("SET UPPER THRESHOLD [70:100]"), CALIB_POT_HIGH_THRESHOLD);
}

void calibpot::calculatorSlot(bool state){
    if(state==false) return;

    // Controllo sul valore acquisito
    if(dataField=="") return;
    int val = dataField.toInt();
    if(val<0) return;
    if(val>100) return;

    if(pCalculator->activation_code == CALIB_POT_LOW_THRESHOLD){
        if(val>30) return;
        ApplicationDatabase.setData(_DB_SERVICE4_INT,val);
    } else{
        if(val<70) return;
        ApplicationDatabase.setData(_DB_SERVICE5_INT,val);
    }
}

