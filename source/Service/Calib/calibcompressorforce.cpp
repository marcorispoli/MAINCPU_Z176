#include "calibcompressorforce.h"
#include "ui_calibcompressorforce.h"


#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"
#include "../../../lib/msgbox.h"

#define UI_PAGINA _PG_SERVICE_CALIB_FORCE
#define EXIT_PAGINA _PG_SERVICE_CALIB_MENU
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000

CalibCompressorForce::CalibCompressorForce(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CalibCompressorForce)
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
    connect(ui->startButton,SIGNAL(released()),this,SLOT(onStartButton()),Qt::UniqueConnection);
    connect(ui->calibButton,SIGNAL(released()),this,SLOT(onCalibButton()),Qt::UniqueConnection);
    connect(ui->storeButton,SIGNAL(released()),this,SLOT(onStoreButton()),Qt::UniqueConnection);

    connect(ui->calibPadCancelButton,SIGNAL(released()),this,SLOT(onCalibForceCancelButton()),Qt::UniqueConnection);

    connect(ui->calibComprEditF1,SIGNAL(selectionChanged()),this,SLOT(onCalibComprEditF1()),Qt::UniqueConnection);
    connect(ui->calibComprEditF2,SIGNAL(selectionChanged()),this,SLOT(onCalibComprEditF2()),Qt::UniqueConnection);

    ui->calibComprFrameInit->setGeometry(QRect(120,65,456,446));
    ui->calibComprFrameF1->setGeometry(QRect(270,90,296,296));
    ui->calibComprFrameF2->setGeometry(QRect(270,90,296,296));
    ui->calibForceFrameCalibrate->setGeometry(QRect(210,90,316,291));
    ui->calibForceFrameStore->setGeometry(QRect(210,90,316,291));

    timerDisable = 0;
}

CalibCompressorForce::~CalibCompressorForce()
{
    delete ui;
}

// Funzione agganciata ai sistemi di menu custom
void CalibCompressorForce::changePage(int pg, int opt)
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
void CalibCompressorForce::onExitButton(void)
{


    GWindowRoot.setNewPage(EXIT_PAGINA,GWindowRoot.curPage,0);
}

void CalibCompressorForce::initPage(void){

    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
    pCalculator = new numericPad(rotview,view, parent);
    connect(pCalculator,SIGNAL(calcSgn(bool)),this,SLOT(calculatorSlot(bool)));
    connect(pWarningBox,SIGNAL(buttonCancSgn(void)),this,SLOT(warningBoxSignal(void)),Qt::UniqueConnection);

    // Init delle variabili di stato
    ui->calibratedForce->setText(QString("---"));
    ui->rawForce->setText(QString("---"));

    calibrated = false;
    timerDisable = startTimer(500);

    if(isMaster){
        // Attivazione modo calibrazione compressore su M4
        connect(pConsole,SIGNAL(mccPcb215Notify(unsigned char,unsigned char,QByteArray)),this,SLOT(pcb215Notify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
        unsigned char data = 1;
        pConsole->pGuiMcc->sendFrame(MCC_CMD_PCB215_CALIB,1,&data,1);
        ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 0, DBase::_DB_FORCE_SGN); // Init
        pSysLog->log("SERVICE PANEL: CALIB COMPRESSOR FORCE");
    }
}

// Operazioni da compiere all'uscita dalla pagina
void CalibCompressorForce::exitPage(void){
    if(timerDisable) killTimer(timerDisable);
    timerDisable=0;

    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
    disconnect(pCalculator);
    pCalculator->deleteLater(); // importante !!!
    pCalculator = 0;

    if(isMaster){
        unsigned char data = 0;
        pConsole->pGuiMcc->sendFrame(MCC_CMD_PCB215_CALIB,1,&data,1);
        disconnect(pConsole,SIGNAL(mccPcb215Notify(unsigned char,unsigned char,QByteArray)),this,SLOT(pcb215Notify(unsigned char,unsigned char,QByteArray)));
    }

    disconnect(pWarningBox,SIGNAL(buttonCancSgn(void)),this,SLOT(warningBoxSignal(void)));

}

void CalibCompressorForce::timerEvent(QTimerEvent* ev)
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
void CalibCompressorForce::valueChanged(int index,int opt)
{

    switch(index)
    {
    case _DB_SERVICE1_INT: // seqindex
        seqindex = ApplicationDatabase.getDataI(index);
        if(seqindex==255) return;

        hideAll();
        if(seqindex==0){
           // RESET CALIBRATION
           ui->calibComprFrameInit->show();
           pWarningBox->externalHide();
           ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) 500,DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
           ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) 500,DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
           ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) 500,DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
           ApplicationDatabase.setData(_DB_SERVICE5_INT,(int) 500,DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
           ApplicationDatabase.setData(_DB_SERVICE1_STR,QString(""),DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);

           ui->calibComprEditF1->setText(QString(""));
           ui->calibComprEditF2->setText(QString(""));

        } else if(seqindex==1){
            rawOffset = rawForce;
            ui->calibComprFrameF1->show();
        }else if(seqindex==2){
            rawF1Force = rawForce;
            ui->calibComprFrameF2->show();
        }else if(seqindex==3){
            rawF2Force = rawForce;
            ui->calibForceFrameCalibrate->show();
        }else if(seqindex==4){
            if(isMaster) calibrate();
        }else if(seqindex==5){
            ui->calibForceFrameStore->show();
        }else if(seqindex==6){
            if(isMaster) store();
        }
        break;

    case _DB_SERVICE2_INT: // F1 MEASURED FORCE        
        ui->calibComprEditF1->setText(QString("%1").arg(ApplicationDatabase.getDataI(_DB_SERVICE2_INT)));
        break;

    case _DB_SERVICE3_INT: // F2 MEASURED FORCE
        ui->calibComprEditF2->setText(QString("%1").arg(ApplicationDatabase.getDataI(_DB_SERVICE3_INT)));
        break;

    case _DB_SERVICE4_INT: // RAW FORCE
        ui->rawForce->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        break;

    case _DB_SERVICE5_INT: // CALIBRATED FORCE
        ui->calibratedForce->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        break;

    case _DB_SERVICE1_STR: // Attivazione stringa di errore
        // La Warning box si disconnette automaticamente alla pressione di uno dei tasti..
        connect(pWarningBox,SIGNAL(buttonCancSgn(void)),this,SLOT(warningBoxSignal(void)),Qt::UniqueConnection); // Ogni volta va connesso..
        pWarningBox->activate(QString("FORCE CALIBRATION MESSAGE"),ApplicationDatabase.getDataS(index),msgBox::_BUTTON_CANC);
        break;
    }
}


// Reazione alla pressione del pulsante di uscita
void CalibCompressorForce::onStartButton(void)
{
    if(timerDisable) return;
    seqindex=1;
    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) seqindex);
    timerDisable = startTimer(500);

}

void CalibCompressorForce::onCalibButton(void)
{
    if(timerDisable) return;
    seqindex=4;
    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) seqindex);
    timerDisable = startTimer(500);

}

void CalibCompressorForce::onStoreButton(void)
{
    if(timerDisable) return;
    seqindex=6;
    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) seqindex);
    timerDisable = startTimer(500);

}

// Reazione alla pressione del pulsante di uscita
void CalibCompressorForce::onCalibForceCancelButton(void)
{
    // Reset calibrazione
    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 0);

}
void CalibCompressorForce::onCalibComprEditF1(void){

    dataField="";
    pCalculator->activate(&dataField, QString("FORCE 1 MEASURE"), CALIB_FORCE_F1);
}

void CalibCompressorForce::onCalibComprEditF2(void){
    dataField="";
    pCalculator->activate(&dataField, QString("FORCE 2 MEASURE"), CALIB_FORCE_F2);
}

void CalibCompressorForce::hideAll(void){

    ui->calibComprFrameInit->hide();
    ui->calibComprFrameF1->hide();
    ui->calibComprFrameF2->hide();
    ui->calibForceFrameCalibrate->hide();
    ui->calibForceFrameStore->hide();

}


void CalibCompressorForce::calibrate(void){
    QString errorStr;
    bool error = false;

    if(!isMaster) return;

    f1Force = ApplicationDatabase.getDataI(_DB_SERVICE2_INT);
    f2Force = ApplicationDatabase.getDataI(_DB_SERVICE3_INT);


    // CONTROLLI SUI VALORI ACQUISITI
    if((rawOffset>40)||(rawOffset<25)){
         errorStr=QString("INVALID COMPRESSOR OFFSET!\nThe expected offset should be from 25 to 40. Check the force sensor setup.");
         error=true;
    }

    if(f1Force <= 40){
        errorStr=QString("INVALID DATA!\n The force shall exceed 40N.");
        error =true;
    }

    if(rawF1Force<=rawOffset){
        errorStr=QString("INVALID DATA F0 RAW VALUE!\nThe detected force is unchanged. Check the sensor tuning.");
        error = true;
    }

    if(f2Force <= 180){
        errorStr=QString("INVALID DATA!\n The force shall exced 180N.");
        error=true;
    }


    int k0 = f1Force * 256 / (rawF1Force-rawOffset);
    int forza = (rawF2Force-rawOffset) * k0 / 256; // Forza caso curva 1
    int k1 = (f2Force - f1Force) * 256 / (forza-f1Force);

    // Scrive nella configurazione copia, il risultato della calibrazione
    F0 = (unsigned short) rawOffset;
    KF0 = (unsigned short) k0;
    F1 = (unsigned short) f1Force;
    KF1 = (unsigned short) k1;

    if(KF0 > 1000){
        errorStr=QString("INVALID DATA, KF0 TOO LARGE!\nCheck the sensor or\ntry to increase the F0 calibration point.\nF0=%1, KF0=%2\nRAWF1=%3, F1=%4\nKF1=%5, RAWF2=%6 F2=%7").arg(rawOffset).arg(KF0).arg(rawF1Force).arg(F1).arg(KF1).arg(rawF2Force).arg(f2Force);
        error=true;
    }

    if(KF1 > 255){
        errorStr=QString("INVALID DATA, KF1 TOO LARGE!Try to reduce the F0 calibration point:\n(40/50N instead of 60/70N)\nF0=%1, KF0=%2\nRAWF1=%3, F1=%4\nKF1=%5, RAWF2=%6 F2=%7").arg(rawOffset).arg(KF0).arg(rawF1Force).arg(F1).arg(KF1).arg(rawF2Force).arg(f2Force);
        error=true;
    }

    if(error){
        ApplicationDatabase.setData(_DB_SERVICE1_STR,errorStr);
        return;
    }


    calibrated = true;

    // Invio al driver dei dati di calibrazioni per renderli attuali
    unsigned char data[6];

    // Invio dati di calibrazione al dispositivo
    data[0] = 3;
    data[1] = F0;
    data[2] = (unsigned char) KF0;
    data[3] = (unsigned char) (KF0>>8);
    data[4] = F1;
    data[5] = KF1;
    pConsole->pGuiMcc->sendFrame(MCC_CMD_PCB215_CALIB,1,data,6);


    // Incrementa la fase corrente
    seqindex++;
    ApplicationDatabase.setData(_DB_SERVICE1_INT,seqindex);
    return ;

}

void CalibCompressorForce::store(void){
    pCompressore->config.F0 = F0;
    pCompressore->config.KF0 = KF0;
    pCompressore->config.F1 = F1;
    pCompressore->config.KF1 = KF1;

    // Salva i dati di calibrazione
    pCompressore->storeConfigFile();
    calibrated =  false;
    ApplicationDatabase.setData(_DB_SERVICE1_STR,QString("CALIBRATION SUCCESSFULLY COMPLETED!"));

}
// Riceve la pressione del pulsante OK/CANC
void CalibCompressorForce::calculatorSlot(bool state){

    if(state==false) return;
    if(pCalculator->activation_code == CALIB_FORCE_F1) ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) dataField.toInt());
    else ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) dataField.toInt());

    seqindex++;
    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) seqindex);

}

// Notifica dati dal driver della PCB215
void CalibCompressorForce::pcb215Notify(unsigned char id, unsigned char notifyCode, QByteArray buffer)
{

    if(id!=1) return;
    if(notifyCode!=PCB215_NOTIFY_CALIB_DATA) return;

    // Legge la posizione RAW
    rawForce = buffer.at(4);
    calibratedForce = buffer.at(6);
    ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) rawForce);
    ApplicationDatabase.setData(_DB_SERVICE5_INT,(int) calibratedForce);

    return;
}

// La Warning Box tutte le volte resetta tutta la sequenza
void CalibCompressorForce::warningBoxSignal(void){
    // Reset calibrazione
    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 0);
}
