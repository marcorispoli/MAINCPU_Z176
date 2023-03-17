
#include "calibParking.h"
#include "ui_calibParking.h"


#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"


#define UI_PAGINA _PG_SERVICE_CALIB_PARKING
#define EXIT_PAGINA _PG_SERVICE_CALIB_MENU
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000


#define _PARK_BUTTON                    _DB_SERVICE1_INT
    #define _PARK_BUTTON_ACTIVATE   1
    #define _PARK_BUTTON_STORE      2
    #define _PARK_BUTTON_EXIT       3
    #define _PARK_BUTTON_ARM_0      4
    #define _PARK_BUTTON_ARM_180    5

#define _PARK_POT                       _DB_SERVICE2_INT
#define _PARK_STATUS                    _DB_SERVICE3_INT
#define _PARK_ROT_ENA                   _DB_SERVICE4_INT
calibParking::calibParking(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::calibParkUI)
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



    ui->frameStoring->setGeometry(230,80,366,331);

    connect(ui->activateButton,SIGNAL(released()),this,SLOT(onActivateButton()),Qt::UniqueConnection);
    connect(ui->storeButton,SIGNAL(released()),this,SLOT(onStoreButton()),Qt::UniqueConnection);
    connect(ui->arm0Button,SIGNAL(released()),this,SLOT(onArm0Button()),Qt::UniqueConnection);
    connect(ui->arm180Button,SIGNAL(released()),this,SLOT(onArm180Button()),Qt::UniqueConnection);
    timerDisable = 0;
    timerCommand = 0;
}

calibParking::~calibParking()
{
    delete ui;
}

// Funzione agganciata ai sistemi di menu custom
void calibParking::changePage(int pg,  int opt)
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
        if(timerStoring){
            killTimer(timerStoring);
            timerStoring = 0;
        }

        exitPage();

    }

}


// Reazione alla pressione del pulsante di uscita
void calibParking::onExitButton(void)
{
    ApplicationDatabase.setData(_PARK_BUTTON,(int) _PARK_BUTTON_EXIT, DBase::_DB_FORCE_SGN);
}

void calibParking::initPage(void){
    ui->frameStore->hide();
    ui->armFrame->hide();
    ui->activateButton->show();
    ui->frameStoring->hide();
    timerStoring = 0;

    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
    if(!isMaster){
        connect(pConfig,SIGNAL(mccSlaveNotifySgn(unsigned char,unsigned char,QByteArray)),this,SLOT(slaveNotifySlot(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

    }

    if(isMaster){
        ApplicationDatabase.setData(_PARK_POT, 0);
        ApplicationDatabase.setData(_PARK_STATUS,0);
        ApplicationDatabase.setData(_PARK_ROT_ENA, 0);

    }


}

// Operazioni da compiere all'uscita dalla pagina
void calibParking::exitPage(void){
unsigned char data[2];

    if(timerDisable) killTimer (timerDisable);
    timerDisable=0;

    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));

    if(!isMaster){
        // Solo lo Slave deve inizializzare la parte che segue
        //data[0] = 0;
        //pConfig->pSlaveMcc->sendFrame(MCC_CALIB_LENZE,1,data,1); // disattiva la modalità di calibrazione potenziometro
        disconnect(pConfig,SIGNAL(mccSlaveNotifySgn(unsigned char,unsigned char,QByteArray)),this,SLOT(slaveNotifySlot(unsigned char,unsigned char,QByteArray)));
        if(timerCommand)  killTimer(timerCommand);
        timerCommand = 0;

    }

    if(isMaster){

    }


}

void calibParking::timerEvent(QTimerEvent* ev)
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
    if(ev->timerId()==timerStoring)
    {
        killTimer(timerStoring);
        timerStoring=0;
        ui->frameStoring->hide();
    }


    if(!isMaster){
        if(ev->timerId()==timerCommand)
        {
            requestLenzePot();
        }
    }
}

// FUNZIONE DI AGGIORNAMENTO CAMPI VALORE CONNESSO AI CAMPI DEL DATABASE
void calibParking::valueChanged(int index,int opt)
{
    switch(index){
    case _PARK_BUTTON:
        if(ApplicationDatabase.getDataI(index) == _PARK_BUTTON_ACTIVATE){
            if(!isMaster){
                requestStartCalib();
            }
        }else   if(ApplicationDatabase.getDataI(index) == _PARK_BUTTON_EXIT){
            if(!isMaster){
                requestStopCalib();
            }
        }else   if(ApplicationDatabase.getDataI(index) == _PARK_BUTTON_ARM_0){
            if(isMaster){
                activateRot(0);
            }
        }else if(ApplicationDatabase.getDataI(index) == _PARK_BUTTON_ARM_180){
            if(isMaster){
                activateRot(180);
            }
        }else if(ApplicationDatabase.getDataI(index) == _PARK_BUTTON_STORE){

            if(!timerStoring){
                timerStoring = startTimer(3000);
                ui->frameStoring->show();
            }else return;

            if(isMaster){
                pConfig->lenzeConfig.parkingTarget = potenziometro;
                pConfig->lenzeConfig.calibratedParkingTarget = true;
                pConfig->saveLenzeConfig();
                pConfig->updateLenzeDriver();
            }
        }

        break;
    case _PARK_POT:
        potenziometro = ApplicationDatabase.getDataI(index);
        ui->potLabel->setText(QString("%1").arg(potenziometro));
        break;

    case _PARK_STATUS:
        if(ApplicationDatabase.getDataI(index) == 1){
            ui->frameStore->show();
             ui->activateButton->hide();
        }else{
            ui->frameStore->hide();
            ui->activateButton->show();
        }

        break;
    case _PARK_ROT_ENA:
        if( !(ApplicationDatabase.getDataU(_DB_SYSTEM_CONFIGURATION)&_ARCH_ARM_MOTOR)){
            ui->armFrame->hide();
        }else{
            if(ApplicationDatabase.getDataI(index) == 1){
                ui->armFrame->show();
            }else ui->armFrame->hide();
        }
        break;
    }
}

void calibParking::onActivateButton(void){
    ApplicationDatabase.setData(_PARK_BUTTON,(int) _PARK_BUTTON_ACTIVATE, DBase::_DB_FORCE_SGN);
}

void calibParking::onStoreButton(void){
    ApplicationDatabase.setData(_PARK_BUTTON,(int) _PARK_BUTTON_STORE, DBase::_DB_FORCE_SGN);
}

void calibParking::onArm0Button(void){
    ApplicationDatabase.setData(_PARK_BUTTON,(int) _PARK_BUTTON_ARM_0, DBase::_DB_FORCE_SGN);
}

void calibParking::onArm180Button(void){
    ApplicationDatabase.setData(_PARK_BUTTON,(int) _PARK_BUTTON_ARM_180, DBase::_DB_FORCE_SGN);
}


// Slot di ricezione Notifiche MCC dal modulo Slave
/*
 *  buffer[0] = COMANDO
 *  buffer[1] = Stato flag di parking mode
 *  buffer[3,2] = potenziometro
 *  buffer[4] = 1 se potenziometro > posizione di sicurezza
 */
void calibParking::slaveNotifySlot(unsigned char id,unsigned char mcc_code,QByteArray data)
{

    if(mcc_code==MCC_PARKING_MODE_COMMANDS){
        if(data.size() !=5 ) return;

        switch(data.at(0)){
            case MCC_PARKING_MODE_COMMANDS_START_CALIBRATION:
                if(data.at(1)) {
                    // Modalità attiva: inizia a polare il potenziometro
                    if(!timerCommand)  timerCommand = startTimer(500);
                }
                // Aggiornamento potenziometro
                ApplicationDatabase.setData(_PARK_POT, (int) ((int) data.at(2) + 256 * (int) data.at(3)));
                ApplicationDatabase.setData(_PARK_STATUS, (int) data.at(1));
                ApplicationDatabase.setData(_PARK_ROT_ENA, (int) data.at(4));

            break;
            case MCC_PARKING_MODE_COMMANDS_STOP_CALIBRATION:
                if(data.at(1)==0)   GWindowRoot.setNewPage(EXIT_PAGINA,GWindowRoot.curPage,0);

                // Aggiornamento potenziometro
                ApplicationDatabase.setData(_PARK_POT, (int) ((int) data.at(2) + 256 * (int) data.at(3)));
                ApplicationDatabase.setData(_PARK_STATUS, (int) data.at(1));
                ApplicationDatabase.setData(_PARK_ROT_ENA, (int) data.at(4));
            break;

            case MCC_PARKING_MODE_COMMANDS_GET_POT:

                // Aggiornamento potenziometro
                ApplicationDatabase.setData(_PARK_POT, (int) ((int) data.at(2) + 256 * (int) data.at(3)));
                ApplicationDatabase.setData(_PARK_STATUS, (int) data.at(1));
                ApplicationDatabase.setData(_PARK_ROT_ENA, (int) data.at(4));

            break;
        }

    }


}

void calibParking::requestLenzePot(void){
    unsigned char buffer[4];

    buffer[0] = MCC_PARKING_MODE_COMMANDS_GET_POT;
    pConfig->pSlaveMcc->sendFrame(MCC_PARKING_MODE_COMMANDS,1,buffer,sizeof(buffer));
}

void calibParking::requestStartCalib(void){
    unsigned char buffer[4];

    buffer[0] = MCC_PARKING_MODE_COMMANDS_START_CALIBRATION;
    pConfig->pSlaveMcc->sendFrame(MCC_PARKING_MODE_COMMANDS,1,buffer,sizeof(buffer));
}

void calibParking::requestStopCalib(void){
    unsigned char buffer[4];

    buffer[0] = MCC_PARKING_MODE_COMMANDS_STOP_CALIBRATION;
    pConfig->pSlaveMcc->sendFrame(MCC_PARKING_MODE_COMMANDS,1,buffer,sizeof(buffer));
}

void calibParking::activateRot(int angolo)
{

    if(angolo > 180) angolo = 180;
    else if(angolo<-180) angolo = -180;

    // Impostazione Parametro
    unsigned char buffer[2];
    buffer[0] =(unsigned char) (angolo&0xFF);
    buffer[1] =(unsigned char) (angolo>>8);

    pConsole->pGuiMcc->sendFrame(MCC_CMD_ARM,0,buffer, 2);
}
