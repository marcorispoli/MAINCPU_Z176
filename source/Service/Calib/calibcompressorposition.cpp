#include "calibcompressorposition.h"
#include "ui_calibcompressorposition.h"


#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"
#include "../../../lib/msgbox.h"

#define UI_PAGINA _PG_SERVICE_CALIB_POSITION
#define EXIT_PAGINA _PG_SERVICE_CALIB_MENU
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000

calibCompressorPosition::calibCompressorPosition(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::calibCompressorPosition)
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

    connect(ui->startButton,SIGNAL(released()),this,SLOT(onStartButton()),Qt::UniqueConnection);
    connect(ui->calibButton,SIGNAL(released()),this,SLOT(onCalibButton()),Qt::UniqueConnection);
    connect(ui->storeButton,SIGNAL(released()),this,SLOT(onStoreButton()),Qt::UniqueConnection);
    connect(ui->calibPadCancelButton,SIGNAL(released()),this,SLOT(onCalibPadCancelButton()),Qt::UniqueConnection);
    connect(ui->calibPadEditUp,SIGNAL(selectionChanged()),this,SLOT(onCalibPadEditUp()),Qt::UniqueConnection);
    connect(ui->calibPadEditDwn,SIGNAL(selectionChanged()),this,SLOT(onCalibPadEditDwn()),Qt::UniqueConnection);

    ui->calibPadFrameInit->setGeometry(QRect(130,65,466,396));
    ui->calibPadFrameUp->setGeometry(QRect(130,65,466,396));
    ui->calibPadFrameDwn->setGeometry(QRect(110,5,491,421));
    ui->calibPadFrameCalibrate->setGeometry(QRect(210,90,316,291));
    ui->calibPadFrameStore->setGeometry(QRect(210,90,316,291));

    timerDisable = 0;
}

calibCompressorPosition::~calibCompressorPosition()
{
    delete ui;
}

// Funzione agganciata ai sistemi di menu custom
void calibCompressorPosition::changePage(int pg,  int opt)
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
void calibCompressorPosition::onExitButton(void)
{


    GWindowRoot.setNewPage(EXIT_PAGINA,GWindowRoot.curPage,0);
}

void calibCompressorPosition::initPage(void){

    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
    pCalculator = new numericPad(rotview,view, parent);
    connect(pCalculator,SIGNAL(calcSgn(bool)),this,SLOT(calculatorSlot(bool)));

    connect(pWarningBox,SIGNAL(buttonCancSgn(void)),this,SLOT(warningBoxSignal(void)),Qt::UniqueConnection);

    if(isMaster){
        // Attivazione modo calibrazione compressore su M4
        connect(pConsole,SIGNAL(mccPcb215Notify(unsigned char,unsigned char,QByteArray)),this,SLOT(pcb215Notify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
        unsigned char data = 1;
        pConsole->pGuiMcc->sendFrame(MCC_CMD_PCB215_CALIB,1,&data,1);
        ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 0,DBase::_DB_FORCE_SGN); // Init
        pSysLog->log("SERVICE PANEL: CALIB COMPRESSOR POSITION");
    }

    calibrated = false;
    timerDisable = startTimer(500);


}

// Operazioni da compiere all'uscita dalla pagina
void calibCompressorPosition::exitPage(void){

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

void calibCompressorPosition::timerEvent(QTimerEvent* ev)
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
void calibCompressorPosition::valueChanged(int index,int opt)
{

    switch(index)
    {
    case _DB_SERVICE1_INT: // seqindex
        seqindex = ApplicationDatabase.getDataI(index);
        if(seqindex==255) return;

        hideAll();
        if(seqindex==0){
           // RESET CALIBRATION
           calibrated=false;
           ui->calibPadFrameInit->show();
           upperPosition=0;
           lowerPosition=0;
           rawUpperPosition=0;
           rawLowerPosition=0;
           ui->calibPadFrameInit->show();
           pWarningBox->externalHide();
           ui->calibPadEditDwn->setText(QString(""));
           ui->calibPadEditUp->setText(QString(""));
           ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) 0,DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
           ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) 0,DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
           ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) 0,DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
           ApplicationDatabase.setData(_DB_SERVICE5_INT,(int) 0,DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);


        } else if(seqindex==1) ui->calibPadFrameUp->show();
        else if(seqindex==2){
            // Salva il campione della posizione alta
            rawUpperPosition = rawPosition;
            ui->calibPadFrameDwn->show();
        }else if(seqindex==3){
            // Salva il campione della posizione bassa
            rawLowerPosition = rawPosition;
            ui->calibPadFrameCalibrate->show();
        }else if(seqindex==4){
            if(isMaster) calibrate();
        }else if(seqindex==5){
            ui->calibPadFrameStore->show();
        }else if(seqindex==6){
            if(isMaster) store();
        }
        break;
    case _DB_SERVICE2_INT: // UPPER POSITION
        upperPosition = ApplicationDatabase.getDataI(index);
        ui->calibPadEditUp->setText(QString("%1").arg(upperPosition));
        break;
    case _DB_SERVICE3_INT: // LOWER POSITION
        lowerPosition = ApplicationDatabase.getDataI(index);
        ui->calibPadEditDwn->setText(QString("%1").arg(lowerPosition));
        break;

    case _DB_SERVICE4_INT: // RAW POSITION
        rawPosition = ApplicationDatabase.getDataI(index);
        ui->rawPosition->setText(QString("%1").arg(rawPosition));
        break;

    case _DB_SERVICE5_INT: // CALIBRATED POSITION
        calibratedPosition = ApplicationDatabase.getDataI(index);
        ui->calibratedPosition->setText(QString("%1").arg(calibratedPosition));
        break;

    case _DB_SERVICE1_STR: // Attivazione stringa di errore
        // La Warning box si disconnette automaticamente alla pressione di uno dei tasti..
        connect(pWarningBox,SIGNAL(buttonCancSgn(void)),this,SLOT(warningBoxSignal(void)),Qt::UniqueConnection); // Ogni volta va connesso..
        pWarningBox->activate(QString("PAD CALIBRATION ERROR"),ApplicationDatabase.getDataS(index),msgBox::_BUTTON_CANC);
        ApplicationDatabase.setData(index,QString(""),DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
        break;
    }
}


// Reazione alla pressione del pulsante di uscita

void calibCompressorPosition::onCalibButton(void)
{
    if(timerDisable) return;
    seqindex=4;
    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) seqindex);
    timerDisable = startTimer(500);

}

void calibCompressorPosition::onStoreButton(void)
{
    if(timerDisable) return;
    seqindex=6;
    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) seqindex);
    timerDisable = startTimer(500);

}

void calibCompressorPosition::onStartButton(void)
{
    if(timerDisable) return;
    seqindex++;
    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) seqindex);
    timerDisable = startTimer(500);
}

// Reazione alla pressione del pulsante di uscita
void calibCompressorPosition::onCalibPadCancelButton(void)
{
    // Reset calibrazione
    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 0);

}
void calibCompressorPosition::onCalibPadEditUp(void){
    dataField = QString("");
    pCalculator->activate(&dataField, QString("UPPER POSITION MEASURE"), CALIB_POS_UP_FIELD);
}

void calibCompressorPosition::onCalibPadEditDwn(void){
    pCalculator->activate(&dataField, QString("LOWER POSITION MEASURE"), CALIB_POS_DWN_FIELD);
}

void calibCompressorPosition::hideAll(void){
    ui->calibPadFrameInit->hide();
    ui->calibPadFrameUp->hide();
    ui->calibPadFrameDwn->hide();
    ui->calibPadFrameCalibrate->hide();
    ui->calibPadFrameStore->hide();
}



void calibCompressorPosition::calibrate(void){
    QString errorStr;
    bool error = false;

    if(!isMaster) return;

    // Controlli preliminari
    if(rawLowerPosition < 10){
        errorStr = QString("CALIBRATION FAILED!\nInvalid RAW data acquisition. Check the PAD integrity");
        error=true;
    }else if(rawUpperPosition-rawLowerPosition < 50){
        errorStr = QString("CALIBRATION FAILED!\nInvalid RAW data acquisition. Check the PAD integrity");
        error=true;
    }else  if(upperPosition - lowerPosition < 100){
        errorStr = QString("CALIBRATION FAILED!\nInvalid Measured data acquisition. The minimum required difference from UP and LOW position is 100mm.");
        error=true;
    }

    if(error){
        ApplicationDatabase.setData(_DB_SERVICE1_STR,errorStr);
        return;
    }

    int calibPosK = (int) ( 256 * ((float) (upperPosition - lowerPosition) / (float) (rawUpperPosition-rawLowerPosition)));
    if(calibPosK>255)
    {
        errorStr = QString("CALIBRATION FAILED!\nInvalid K coefficient. Check the PAD integrity.");
        ApplicationDatabase.setData(_DB_SERVICE1_STR,errorStr);
        return;
    }

    int calibPosOfs = (calibPosK * rawUpperPosition) / 256 - upperPosition ;
    if(calibPosOfs<0)
    {
        errorStr = QString("CALIBRATION FAILED!\nInvalid Offset coefficient. Check the PAD integrity.");
        ApplicationDatabase.setData(_DB_SERVICE1_STR,errorStr);
        return;
    }

    this->calibPosOfs = calibPosOfs;
    this->calibPosK = calibPosK;
    calibrated = true;

    // Invio al driver dei dati di calibrazioni per renderli attuali
    unsigned char data[4];
    data[0] = 2; // Comando di invio calibrazione della nacchera
    data[1] = (unsigned char)  calibPosOfs ;
    data[2] = (unsigned char) (calibPosOfs >> 8);
    data[3] = (unsigned char)  calibPosK;
    pConsole->pGuiMcc->sendFrame(MCC_CMD_PCB215_CALIB,1,data,4);

    // Incrementa la fase corrente
    seqindex++;
    ApplicationDatabase.setData(_DB_SERVICE1_INT,seqindex);
    return ;

}

void calibCompressorPosition::store(void){
    pCompressore->config.calibPosOfs = this->calibPosOfs;
    pCompressore->config.calibPosK = this->calibPosK;

    // Salva i dati di calibrazione
    pCompressore->storeConfigFile();
    ApplicationDatabase.setData(_DB_SERVICE1_STR,QString("CALIBRATION SUCCESSFULLY COMPLETED!"));

}
// Riceve la pressione del pulsante OK/CANC
void calibCompressorPosition::calculatorSlot(bool state){

    if(state==false) return;
    if(pCalculator->activation_code == CALIB_POS_UP_FIELD) ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) dataField.toInt());
    else ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) dataField.toInt());

    // Procede con gli steps
    seqindex++;
    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) seqindex);
}

// Notifica dati dal driver della PCB215
void calibCompressorPosition::pcb215Notify(unsigned char id, unsigned char notifyCode, QByteArray buffer)
{

    if(id!=1) return;
    if(notifyCode!=PCB215_NOTIFY_CALIB_DATA) return;

    // Legge la posizione RAW
    rawPosition = buffer.at(2)+buffer.at(3)*256;
    calibratedPosition = buffer.at(7)+buffer.at(8)*256;
    ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) rawPosition);
    ApplicationDatabase.setData(_DB_SERVICE5_INT,(int) calibratedPosition);
    return;
}

void calibCompressorPosition::warningBoxSignal(void){
    // Reset calibrazione
    ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 0);
}
