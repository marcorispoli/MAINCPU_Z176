// <TAG1>  CALIBPOT
// calibpot  calibpot
// calibPotUI  CalibpotUI
// Calibpot  Calibpot

// _PG_SERVICE_CALIB_LENZE_POT  _PG_SERVICE_CALIB_LENZE_POT
// _PG_SERVICE_CALIB_MENU  _PG_SERVICE_CALIB_MENU


#include "calibconsole.h"
#include "ui_calibconsole.h"


#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"


#define UI_PAGINA _PG_SERVICE_CALIB_CONSOLE
#define EXIT_PAGINA _PG_SERVICE_CALIB_MENU
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000


#define _CAL_CONSOLE_FASE   _DB_SERVICE1_INT

#define _CAL_CONSOLE_X_HOME _DB_SERVICE2_INT
#define _CAL_CONSOLE_Y_HOME _DB_SERVICE3_INT
#define _CAL_CONSOLE_X_REF  _DB_SERVICE4_INT
#define _CAL_CONSOLE_Y_REF  _DB_SERVICE5_INT
#define _CAL_CONSOLE_JX     _DB_SERVICE6_INT
#define _CAL_CONSOLE_JY     _DB_SERVICE7_INT

// Configuration file
#define _CAL_CONSOLE_FANTOCCIO_DX   _DB_SERVICE8_INT
#define _CAL_CONSOLE_FANTOCCIO_DY   _DB_SERVICE9_INT
#define _CAL_CONSOLE_X_OFFSET       _DB_SERVICE10_INT
#define _CAL_CONSOLE_Y_OFFSET       _DB_SERVICE11_INT
#define _CAL_CONSOLE_Z_OFFSET       _DB_SERVICE12_INT
#define _CAL_CONSOLE_F_OFFSET       _DB_SERVICE13_INT

// Attivazioni
#define _CAL_CONSOLE_X_MOVE       _DB_SERVICE14_INT
#define _CAL_CONSOLE_Y_MOVE       _DB_SERVICE15_INT
#define _CAL_CONSOLE_Z_MOVE       _DB_SERVICE16_INT

#define _CAL_CONSOLE_STATUS_LOOP    _DB_SERVICE17_INT

// Bottoni
#define _CAL_CONSOLE_MOVEXYZ_BUTTON     _DB_SERVICE20_INT
#define _CAL_CONSOLE_MOVE_HOME_BUTTON   _DB_SERVICE21_INT
#define _CAL_CONSOLE_TEST_LOOP_BUTTON   _DB_SERVICE22_INT
#define _CAL_CONSOLE_EXIT_BUTTON        _DB_SERVICE23_INT
#define _CAL_CONSOLE_MOVE_BUTTONS       _DB_SERVICE24_INT
#define _CAL_CONSOLE_LOOP_BUTTONS       _DB_SERVICE25_INT
// Labels
#define _CAL_CONSOLE_X_DIF          _DB_SERVICE1_STR
#define _CAL_CONSOLE_Y_DIF          _DB_SERVICE2_STR



calibconsole::calibconsole(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::consoleUI)
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

    connect(ui->confirmExitButton,SIGNAL(released()),this,SLOT(onConfirmExitButton()),Qt::UniqueConnection);
    connect(ui->storeButton,SIGNAL(released()),this,SLOT(onStoreButton()),Qt::UniqueConnection);
    connect(ui->cancelButton,SIGNAL(released()),this,SLOT(onCancelButton()),Qt::UniqueConnection);

    connect(ui->openMoveButton,SIGNAL(released()),this,SLOT(onOpenMoveButton()),Qt::UniqueConnection);
    connect(ui->moveXYZ,SIGNAL(released()),this,SLOT(onMoveXYZButton()),Qt::UniqueConnection);
    connect(ui->cancelMove,SIGNAL(released()),this,SLOT(onCancelMoveButton()),Qt::UniqueConnection);
    connect(ui->homeButton,SIGNAL(released()),this,SLOT(onHomeButton()),Qt::UniqueConnection);
    connect(ui->loopButton,SIGNAL(released()),this,SLOT(onLoopButton()),Qt::UniqueConnection);
    connect(ui->startLoop,SIGNAL(released()),this,SLOT(onStartLoop()),Qt::UniqueConnection);
    connect(ui->stopLoop,SIGNAL(released()),this,SLOT(onStopLoop()),Qt::UniqueConnection);


    connect(ui->DX_Setup,SIGNAL(selectionChanged()),this,SLOT(onDxSetup()),Qt::UniqueConnection);
    connect(ui->DY_Setup,SIGNAL(selectionChanged()),this,SLOT(onDySetup()),Qt::UniqueConnection);

    connect(ui->X_Offset,SIGNAL(selectionChanged()),this,SLOT(onXOffset()),Qt::UniqueConnection);
    connect(ui->Y_Offset,SIGNAL(selectionChanged()),this,SLOT(onYOffset()),Qt::UniqueConnection);
    connect(ui->Z_Offset,SIGNAL(selectionChanged()),this,SLOT(onZOffset()),Qt::UniqueConnection);
    connect(ui->F_Offset,SIGNAL(selectionChanged()),this,SLOT(onFOffset()),Qt::UniqueConnection);

    connect(ui->X_Move,SIGNAL(selectionChanged()),this,SLOT(onXMove()),Qt::UniqueConnection);
    connect(ui->Y_Move,SIGNAL(selectionChanged()),this,SLOT(onYMove()),Qt::UniqueConnection);
    connect(ui->Z_Move,SIGNAL(selectionChanged()),this,SLOT(onZMove()),Qt::UniqueConnection);
    connect(ui->X_loop,SIGNAL(selectionChanged()),this,SLOT(onXloop()),Qt::UniqueConnection);
    connect(ui->Y_loop,SIGNAL(selectionChanged()),this,SLOT(onYloop()),Qt::UniqueConnection);
    connect(ui->Z_loop,SIGNAL(selectionChanged()),this,SLOT(onZloop()),Qt::UniqueConnection);

    buttonSelector = scene->addPixmap(QPixmap("://Service_Calib/Service_Calib/selezione_pulsante_console.png"));
    buttonSelector->setPos(266,295); // 266,322,388,462,526
    readerSelector = scene->addPixmap(QPixmap("://Sym/Sym/puntatore_console.png"));
    readerSelector->setPos(531,97); // 531,97  237,199

    timerDisable = 0;
    loopTimer =0;
}

calibconsole::~calibconsole()
{
    delete ui;
}

// Funzione agganciata ai sistemi di menu custom
void calibconsole::changePage(int pg,  int opt)
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


// Alla pressione del pulsante di uscita il master deve valutare se occorre salvare o no i dati eventualemente
// modificati.
void calibconsole::onExitButton(void)
{
    ApplicationDatabase.setData(_CAL_CONSOLE_EXIT_BUTTON, (int) 0,DBase::_DB_FORCE_SGN);

}
void calibconsole::onConfirmExitButton(void)
{
    ApplicationDatabase.setData(_CAL_CONSOLE_EXIT_BUTTON, (int) 1,DBase::_DB_FORCE_SGN);

}
void calibconsole::onStoreButton(void)
{
    ApplicationDatabase.setData(_CAL_CONSOLE_EXIT_BUTTON, (int) 2,DBase::_DB_FORCE_SGN);

}
void calibconsole::onCancelButton(void)
{
    ApplicationDatabase.setData(_CAL_CONSOLE_EXIT_BUTTON, (int) 3,DBase::_DB_FORCE_SGN);

}

void calibconsole::onOpenMoveButton(void)
{
    ApplicationDatabase.setData(_CAL_CONSOLE_MOVE_BUTTONS, (int) 1,DBase::_DB_FORCE_SGN);

}
void calibconsole::onMoveXYZButton(void)
{
    ApplicationDatabase.setData(_CAL_CONSOLE_MOVE_BUTTONS, (int) 2,DBase::_DB_FORCE_SGN);

}
void calibconsole::onCancelMoveButton(void)
{
    ApplicationDatabase.setData(_CAL_CONSOLE_MOVE_BUTTONS, (int) 3,DBase::_DB_FORCE_SGN);

}

void calibconsole::onHomeButton(void)
{
    ApplicationDatabase.setData(_CAL_CONSOLE_MOVE_BUTTONS, (int) 4,DBase::_DB_FORCE_SGN);

}


void calibconsole::onLoopButton(void)
{
    ApplicationDatabase.setData(_CAL_CONSOLE_LOOP_BUTTONS, (int) 1,DBase::_DB_FORCE_SGN);

}
void calibconsole::onStartLoop(void)
{
    ApplicationDatabase.setData(_CAL_CONSOLE_LOOP_BUTTONS, (int) 2,DBase::_DB_FORCE_SGN);

}
void calibconsole::onStopLoop(void)
{
    ApplicationDatabase.setData(_CAL_CONSOLE_LOOP_BUTTONS, (int) 3,DBase::_DB_FORCE_SGN);

}


void calibconsole::onExitPageEvent(void){
    if(loopTimer){
        killTimer(loopTimer);
        loopTimer=0;
    }
    GWindowRoot.setNewPage(EXIT_PAGINA,GWindowRoot.curPage,0);
}

void calibconsole::onDxSetup(void){
    dataField = QString("");
    pCalculator->activate(&dataField, QString("PHANTOM WIDTH (dmm)"), 1);

}
void calibconsole::onDySetup(void){
    dataField = QString("");
    pCalculator->activate(&dataField, QString("PHANTOM HEIGHT (dmm)"), 2);

}

void calibconsole::onXOffset(void){
    dataField = QString("");
    pCalculator->activate(&dataField, QString("LESION X OFFSET (dmm)"), 3);

}
void calibconsole::onYOffset(void){
    dataField = QString("");
    pCalculator->activate(&dataField, QString("LESION Y OFFSET (dmm)"), 4);

}
void calibconsole::onZOffset(void){
    dataField = QString("");
    pCalculator->activate(&dataField, QString("LESION Z OFFSET (dmm)"), 5);

}
void calibconsole::onFOffset(void){
    dataField = QString("");
    pCalculator->activate(&dataField, QString("FIBER TO TOP NEEDLE HOLDER (mm)"), 6);

}
void calibconsole::onXMove(void){
    dataField = QString("");
    pCalculator->activate(&dataField, QString("BIOPSY X ACTIVATION TARGET (dmm)"), 7);

}
void calibconsole::onYMove(void){
    dataField = QString("");
    pCalculator->activate(&dataField, QString("BIOPSY Y ACTIVATION TARGET (dmm)"), 8);

}
void calibconsole::onZMove(void){
    dataField = QString("");
    pCalculator->activate(&dataField, QString("BIOPSY Z ACTIVATION TARGET (dmm)"), 9);

}
void calibconsole::onXloop(void){
    dataField = QString("");
    pCalculator->activate(&dataField, QString("LOOP X ACTIVATION TARGET (dmm)"), 10);

}
void calibconsole::onYloop(void){
    dataField = QString("");
    pCalculator->activate(&dataField, QString("LOOP Y ACTIVATION TARGET (dmm)"), 11);

}
void calibconsole::onZloop(void){
    dataField = QString("");
    pCalculator->activate(&dataField, QString("LOOP Z ACTIVATION TARGET (dmm)"), 12);

}
void calibconsole::initPage(void){

    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::QueuedConnection);
    pCalculator = new numericPad(rotview,view, parent);
    connect(pCalculator,SIGNAL(calcSgn(bool)),this,SLOT(calculatorSlot(bool)));
    Xhome = 0;
    Yhome = 0;
    loopTimer =0 ;

    ApplicationDatabase.setData(_CAL_CONSOLE_FASE, (int) 0,DBase::_DB_NO_CHG_SGN | DBase::_DB_NO_ECHO); // Nessuna misura è in corso
    ApplicationDatabase.setData(_CAL_CONSOLE_X_HOME, (int) 0,DBase::_DB_NO_CHG_SGN | DBase::_DB_NO_ECHO); // Nessuna misura è in corso
    ApplicationDatabase.setData(_CAL_CONSOLE_Y_HOME, (int) 0,DBase::_DB_NO_CHG_SGN | DBase::_DB_NO_ECHO); // Nessuna misura è in corso
    ApplicationDatabase.setData(_CAL_CONSOLE_X_REF, (int) 0,DBase::_DB_NO_CHG_SGN | DBase::_DB_NO_ECHO); // Nessuna misura è in corso
    ApplicationDatabase.setData(_CAL_CONSOLE_Y_REF, (int) 0,DBase::_DB_NO_CHG_SGN | DBase::_DB_NO_ECHO); // Nessuna misura è in corso
    ApplicationDatabase.setData(_CAL_CONSOLE_X_DIF, "",DBase::_DB_NO_CHG_SGN | DBase::_DB_NO_ECHO); // Nessuna misura è in corso
    ApplicationDatabase.setData(_CAL_CONSOLE_Y_DIF, "",DBase::_DB_NO_CHG_SGN | DBase::_DB_NO_ECHO); // Nessuna misura è in corso

    ApplicationDatabase.setData(_CAL_CONSOLE_X_MOVE, 0,DBase::_DB_NO_CHG_SGN | DBase::_DB_NO_ECHO);
    ApplicationDatabase.setData(_CAL_CONSOLE_Y_MOVE, 0,DBase::_DB_NO_CHG_SGN | DBase::_DB_NO_ECHO);
    ApplicationDatabase.setData(_CAL_CONSOLE_Z_MOVE, 0,DBase::_DB_NO_CHG_SGN | DBase::_DB_NO_ECHO);

    ApplicationDatabase.setData(_CAL_CONSOLE_JX, 0,DBase::_DB_NO_CHG_SGN | DBase::_DB_NO_ECHO);
    ApplicationDatabase.setData(_CAL_CONSOLE_JY, 0,DBase::_DB_NO_CHG_SGN | DBase::_DB_NO_ECHO);


    if(isMaster){
        // Salvataggio dati di calibrazione di ingresso
        config = pBiopsy->config;

        ApplicationDatabase.setData(_CAL_CONSOLE_FANTOCCIO_DX, (int) config.dmm_DXReader,DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_CAL_CONSOLE_FANTOCCIO_DY, (int) config.dmm_DYReader,DBase::_DB_FORCE_SGN);

        ApplicationDatabase.setData(_CAL_CONSOLE_X_OFFSET, (int) config.offsetX,DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_CAL_CONSOLE_Y_OFFSET, (int) config.offsetY,DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_CAL_CONSOLE_Z_OFFSET, (int) config.offsetZ,DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_CAL_CONSOLE_F_OFFSET, (int) config.offsetFibra,DBase::_DB_FORCE_SGN);

    }

    refreshStatus();
    showCalibFrame();
}

// Operazioni da compiere all'uscita dalla pagina
void calibconsole::exitPage(void){
unsigned char data[2];

    if(timerDisable) killTimer (timerDisable);
    timerDisable=0;
    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));

    disconnect(pCalculator);
    pCalculator->deleteLater(); // importante !!!
    pCalculator = 0;

    if(loopTimer){
        killTimer(loopTimer);
        loopTimer =0;
    }
}

void calibconsole::timerEvent(QTimerEvent* ev)
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

    if(ev->timerId()==loopTimer)
    {
        if(pBiopsy->movingCommand != _BIOPSY_MOVING_COMPLETED) return;
/*        if(loopNtime){
            loopNtime--;
            ApplicationDatabase.setData(_CAL_CONSOLE_STATUS_LOOP,(int) loopNtime);
            return;
        }
        loopNtime = 10;
*/
        if(loopState){
           pBiopsy->moveHome();
        }else{
            loopNtime++;
            ApplicationDatabase.setData(_CAL_CONSOLE_STATUS_LOOP,(int) loopNtime);
            pBiopsy->moveXYZ(ApplicationDatabase.getDataI(_CAL_CONSOLE_X_MOVE),ApplicationDatabase.getDataI(_CAL_CONSOLE_Y_MOVE),ApplicationDatabase.getDataI(_CAL_CONSOLE_Z_MOVE));
        }
        loopState = !loopState;

    }
}

bool calibconsole::Calibrate(void){

    // Controllo dei valori
    if((Xref - Xhome) < 300) return false;
    if((Yref - Yhome) < 100) return false;

    // Calcolo del coefficiente di calibrazione
    config.readerKX = (float) config.dmm_DXReader / (float) (Xref - Xhome);
    config.readerKY = (float) config.dmm_DYReader / (float) (Yref - Yhome);

    QString sVal = QString("DX:%1 [Kx:%2]").arg((float) (Xref-Xhome) * config.readerKX ).arg(config.readerKX);
    ApplicationDatabase.setData(_CAL_CONSOLE_X_DIF, sVal);
    sVal = QString("DY:%1 [Ky:%2]").arg((float) (Yref-Yhome) * config.readerKY ).arg(config.readerKY);
    ApplicationDatabase.setData(_CAL_CONSOLE_Y_DIF, sVal);
    return true;

}

// FUNZIONE DI AGGIORNAMENTO CAMPI VALORE CONNESSO AI CAMPI DEL DATABASE
void calibconsole::valueChanged(int index,int opt)
{
    int iVal, iVal2;
    QString sVal;

    switch(index){
    case _DB_BIOP_CONSOLE_BUTTON:
        updateConsoleButtonShadow();
        if(!isMaster) return;
        updateConsoleButtonFunction();
        break;

    case _CAL_CONSOLE_FASE: // Cambio modalità tra Home e reference
        updateFase(ApplicationDatabase.getDataI(index));
        break;

    case _CAL_CONSOLE_X_HOME:
        iVal = ApplicationDatabase.getDataI(index);
        ui->labelXHome->setText(QString("X:%1").arg(iVal));
        ui->labelXHome->show();
        break;
    case _CAL_CONSOLE_Y_HOME:
        iVal = ApplicationDatabase.getDataI(index);
        ui->labelYHome->setText(QString("Y:%1").arg(iVal));
        ui->labelYHome->show();
        break;
    case _CAL_CONSOLE_X_REF:
        iVal = ApplicationDatabase.getDataI(index);
        ui->labelXRef->setText(QString("X:%1").arg(iVal));
        ui->labelXRef->show();
        break;
    case _CAL_CONSOLE_Y_REF:
        iVal = ApplicationDatabase.getDataI(index);
        ui->labelYRef->setText(QString("Y:%1").arg(iVal));
        ui->labelYRef->show();
        break;

    case _CAL_CONSOLE_X_DIF:
        ui->labelDX->setText(ApplicationDatabase.getDataS(index));
        ui->labelDX->show();
        break;
    case _CAL_CONSOLE_Y_DIF:
        ui->labelDY->setText(ApplicationDatabase.getDataS(index));
        ui->labelDY->show();
        break;

    case _CAL_CONSOLE_X_MOVE:
        ui->X_Move->setText(QString("X: %1 (dmm)").arg(ApplicationDatabase.getDataI(index)));
        ui->X_loop->setText(QString("X: %1 (dmm)").arg(ApplicationDatabase.getDataI(index)));
        break;
    case _CAL_CONSOLE_Y_MOVE:
        ui->Y_Move->setText(QString("Y: %1 (dmm)").arg(ApplicationDatabase.getDataI(index)));
        ui->Y_loop->setText(QString("Y: %1 (dmm)").arg(ApplicationDatabase.getDataI(index)));
        break;
    case _CAL_CONSOLE_Z_MOVE:
        ui->Z_Move->setText(QString("Z: %1 (dmm)").arg(ApplicationDatabase.getDataI(index)));
        ui->Z_loop->setText(QString("Z: %1 (dmm)").arg(ApplicationDatabase.getDataI(index)));
        break;


    case _CAL_CONSOLE_FANTOCCIO_DX:
        sVal = QString("DX: %1 (dmm)").arg(ApplicationDatabase.getDataI(index));
        ui->DX_Setup->setText(sVal);
        if(isMaster){
            config.dmm_DXReader = ApplicationDatabase.getDataI(index);
            ApplicationDatabase.setData(_CAL_CONSOLE_FASE, 0);
        }

        break;

    case _CAL_CONSOLE_FANTOCCIO_DY:
        sVal = QString("DY: %1 (dmm)").arg(ApplicationDatabase.getDataI(index));
        ui->DY_Setup->setText(sVal);
        if(isMaster){
            config.dmm_DYReader = ApplicationDatabase.getDataI(index);
            ApplicationDatabase.setData(_CAL_CONSOLE_FASE, 0);
        }
        break;

    case _CAL_CONSOLE_X_OFFSET:
        sVal = QString("X: %1 (dmm)").arg(ApplicationDatabase.getDataI(index));
        ui->X_Offset->setText(sVal);
        if(isMaster)   config.offsetX = ApplicationDatabase.getDataI(index);
        break;
    case _CAL_CONSOLE_Y_OFFSET:
        sVal = QString("Y: %1 (dmm)").arg(ApplicationDatabase.getDataI(index));
        ui->Y_Offset->setText(sVal);
        if(isMaster)   config.offsetY = ApplicationDatabase.getDataI(index);
        break;
    case _CAL_CONSOLE_Z_OFFSET:
        sVal = QString("Z: %1 (dmm)").arg(ApplicationDatabase.getDataI(index));
        ui->Z_Offset->setText(sVal);
        if(isMaster)   config.offsetZ = ApplicationDatabase.getDataI(index);
        break;
    case _CAL_CONSOLE_F_OFFSET:
        sVal = QString("F: %1 (mm)").arg(ApplicationDatabase.getDataI(index));
        ui->F_Offset->setText(sVal);
        if(isMaster)   config.offsetFibra = ApplicationDatabase.getDataI(index);
        break;

    case _DB_BIOP_ZLIMIT:
    case _CAL_CONSOLE_JX:
    case _CAL_CONSOLE_JY:
    case _DB_BIOP_UNLOCK_BUTTON:
    case _DB_ACCESSORIO:
    case _DB_BIOP_X:
    case _DB_BIOP_Y:
    case _DB_BIOP_Z:
    case _DB_BIOP_HOLDER:
        refreshStatus();
        break;



    case _CAL_CONSOLE_EXIT_BUTTON:
        iVal = ApplicationDatabase.getDataI(index);
        if(iVal == 3){
            ApplicationDatabase.setData(_CAL_CONSOLE_FASE, 0);
            showCalibFrame();
        }else showConfigChangeFrame();
        if(!isMaster) return;


        if(iVal==0){ // EXIT PAGE BUTTON
            if(!checkConfigChange()) onExitPageEvent(); // Se la configurazione non è cambiata esce
            return;
        }else if(iVal==1){ // CONFIRM EXIT WITHOUT SAVE
            onExitPageEvent();
            return;
        }else if(iVal==2){ // STORE AND EXIT
            pBiopsy->config = config;
            pBiopsy->storeConfig();
            pBiopsy->updateConfig();
            onExitPageEvent();
            return;
        }
        break;

    case _CAL_CONSOLE_STATUS_LOOP:
        sVal = QString("TEST RUNNING - CURRENT CYCLE:%1 ").arg(ApplicationDatabase.getDataI(index));
        ui->statusLoop->setText(sVal);
        ui->statusLoop->show();
        break;

    case _CAL_CONSOLE_MOVE_BUTTONS:
        iVal = ApplicationDatabase.getDataI(index);

        // Apertura sotto menu
        if(iVal == 1){
            showMoveFrame();
            return;
        }

        if(!isMaster){
            showCalibFrame();
            return;
        }

        // Solo Master
        if(iVal==2) pBiopsy->moveXYZ(ApplicationDatabase.getDataI(_CAL_CONSOLE_X_MOVE),ApplicationDatabase.getDataI(_CAL_CONSOLE_Y_MOVE),ApplicationDatabase.getDataI(_CAL_CONSOLE_Z_MOVE));
        else if(iVal==4) pBiopsy->moveHome();
        ApplicationDatabase.setData(_CAL_CONSOLE_FASE, 0);
        showCalibFrame();

        break;

    case _CAL_CONSOLE_LOOP_BUTTONS:
        iVal = ApplicationDatabase.getDataI(index);

        // Apertura sotto menu
        if(iVal == 1){
            showLoopFrame();
            return;
        }else if(iVal == 3){
            showCalibFrame();
            if(isMaster){
                if(loopTimer){
                    killTimer(loopTimer);
                    loopTimer=0;
                }
                ApplicationDatabase.setData(_CAL_CONSOLE_FASE, 0);
            }
            return;
        }
        if(!isMaster) return;

        // Inizia il ciclo
        loopState = true;
        loopNtime = 1;
        ApplicationDatabase.setData(_CAL_CONSOLE_STATUS_LOOP,(int) loopNtime);
        pBiopsy->moveXYZ(ApplicationDatabase.getDataI(_CAL_CONSOLE_X_MOVE),ApplicationDatabase.getDataI(_CAL_CONSOLE_Y_MOVE),ApplicationDatabase.getDataI(_CAL_CONSOLE_Z_MOVE));
        loopTimer = startTimer(1000);
        break;
    }
}


/*
 *  Apertura dei Frames
 */

void calibconsole::showLoopFrame(void){
    ui->frameLoop->show();
    ui->frameLoop->setGeometry(130,45,556,351);
    ui->statusLoop->hide();
    ui->X_loop->setText(QString("X: %1 (dmm)").arg(ApplicationDatabase.getDataI(_CAL_CONSOLE_X_MOVE)));
    ui->Y_loop->setText(QString("Y: %1 (dmm)").arg(ApplicationDatabase.getDataI(_CAL_CONSOLE_Y_MOVE)));
    ui->Z_loop->setText(QString("Z: %1 (dmm)").arg(ApplicationDatabase.getDataI(_CAL_CONSOLE_Z_MOVE)));

    ui->frameMoveXYZ->hide();
    ui->calibFrame->hide();
    readerSelector->hide();
    buttonSelector->hide();
    ui->frameDataChanged->hide();
}


void calibconsole::showConfigChangeFrame(void){
    ui->frameDataChanged->show();
    ui->frameDataChanged->setGeometry(120,65,546,346);

    ui->frameMoveXYZ->hide();
    ui->calibFrame->hide();
    ui->frameLoop->hide();
    readerSelector->hide();
    buttonSelector->hide();

}
void calibconsole::showMoveFrame(void){
    ui->frameMoveXYZ->show();
    ui->frameMoveXYZ->setGeometry(250,15,306,446);
    ui->X_Move->setText(QString("X: %1 (dmm)").arg(ApplicationDatabase.getDataI(_CAL_CONSOLE_X_MOVE)));
    ui->Y_Move->setText(QString("Y: %1 (dmm)").arg(ApplicationDatabase.getDataI(_CAL_CONSOLE_Y_MOVE)));
    ui->Z_Move->setText(QString("Z: %1 (dmm)").arg(ApplicationDatabase.getDataI(_CAL_CONSOLE_Z_MOVE)));

    ui->frameDataChanged->hide();
    ui->calibFrame->hide();
    ui->frameLoop->hide();
    readerSelector->hide();
    buttonSelector->hide();

}
void calibconsole::showCalibFrame(void){
    ui->calibFrame->show();
    ui->calibFrame->setGeometry(0,0,800,480);
    updateFase(ApplicationDatabase.getDataI(_CAL_CONSOLE_FASE));
    updateConsoleButtonShadow();

    ui->frameDataChanged->hide();
    ui->frameMoveXYZ->hide();
    ui->frameLoop->hide();

}

void calibconsole::updateFase(int fase){

    if(fase==0){

        ui->labelXRef->hide();
        ui->labelYRef->hide();
        ui->labelXHome->hide();
        ui->labelYHome->hide();
        ui->labelDX->hide();
        ui->labelDY->hide();
        readerSelector->hide();

    }else if(fase==1){ // Home
        readerSelector->setPos(531,97); // 531,97  237,199
        readerSelector->show();

    }else if(fase==2){ // reference
        readerSelector->setPos(237,199); // 531,97  237,199
    }
}

void calibconsole::updateConsoleButtonShadow(void){
    int iVal = ApplicationDatabase.getDataI(_DB_BIOP_CONSOLE_BUTTON);
    iVal &=0x1F;
    if(iVal){
        buttonSelector->show();
        if(iVal & 0x01)  buttonSelector->setPos(266,295); // 266,322,388,462,526
        else if(iVal & 0x02)  buttonSelector->setPos(322,295); // 266,322,388,462,526
        else if(iVal & 0x04)  buttonSelector->setPos(388,295); // 266,322,388,462,526
        else if(iVal & 0x08)  buttonSelector->setPos(462,295); // 266,322,388,462,526
        else if(iVal & 0x10)  buttonSelector->setPos(526,295); // 266,322,388,462,526
        else buttonSelector->hide();
    }else buttonSelector->hide();
}

void calibconsole::updateConsoleButtonFunction(void){
    int iVal, iVal2;

    iVal = ApplicationDatabase.getDataI(_DB_BIOP_CONSOLE_BUTTON) & 0x1F;
    if(!iVal) return;

    // Aggiornamento Stato di lettura del puntatore tra Home e Reference
    if(iVal & _BP_BIOP_PUSH_BACK){
        iVal2 = ApplicationDatabase.getDataI(_CAL_CONSOLE_FASE);
        iVal2 ++;
        if(iVal2 > 2) iVal2=0;
        ApplicationDatabase.setData(_CAL_CONSOLE_FASE, iVal2);
    }else  if(iVal & _BP_BIOP_PUSH_SEQ){

        ApplicationDatabase.setData(_CAL_CONSOLE_JX, (int)( (pBiopsy->dmmJX-Xhome) * config.readerKX));
        ApplicationDatabase.setData(_CAL_CONSOLE_JY, (int)( (pBiopsy->dmmJY-Yhome) * config.readerKY));
        iVal2 = ApplicationDatabase.getDataI(_CAL_CONSOLE_FASE);
        if(iVal2==1){
            // Acquisizione Home
            Xhome = pBiopsy->dmmJX;
            Yhome = pBiopsy->dmmJY;
            ApplicationDatabase.setData(_CAL_CONSOLE_X_HOME, (int)Xhome);
            ApplicationDatabase.setData(_CAL_CONSOLE_Y_HOME, (int) Yhome);


        }else if(iVal2==2){
            // Acquisizione Reference
            Xref = pBiopsy->dmmJX;
            Yref = pBiopsy->dmmJY;
            ApplicationDatabase.setData(_CAL_CONSOLE_X_REF, (int) Xref);
            ApplicationDatabase.setData(_CAL_CONSOLE_Y_REF, (int) Yref);
            Calibrate();
        }
    }

}

void calibconsole::calculatorSlot(bool state){
    if(state==false) return;

    // Controllo sul valore acquisito
    if(dataField=="") return;
    int val = dataField.toInt();

    switch(pCalculator->activation_code){
    case 1:
        ApplicationDatabase.setData(_CAL_CONSOLE_FANTOCCIO_DX,val);
        break;
    case 2:
        ApplicationDatabase.setData(_CAL_CONSOLE_FANTOCCIO_DY,val);
        break;
    case 3:
        ApplicationDatabase.setData(_CAL_CONSOLE_X_OFFSET,val);
        break;
    case 4:
        ApplicationDatabase.setData(_CAL_CONSOLE_Y_OFFSET,val);
        break;
    case 5:
        ApplicationDatabase.setData(_CAL_CONSOLE_Z_OFFSET,val);
        break;
    case 6:
        ApplicationDatabase.setData(_CAL_CONSOLE_F_OFFSET,val);
        break;
    case 7:
        if(val>520) val = 520;
        if(val<0) val =0;
        ApplicationDatabase.setData(_CAL_CONSOLE_X_MOVE,val);
        break;
    case 8:
        if(val>470) val = 470;
        if(val<0) val =0;
        ApplicationDatabase.setData(_CAL_CONSOLE_Y_MOVE,val);
        break;
    case 9:
        if(val>ApplicationDatabase.getDataI(_DB_BIOP_ZLIMIT)) val = ApplicationDatabase.getDataI(_DB_BIOP_ZLIMIT);
        if(val<0) val =0;
        ApplicationDatabase.setData(_CAL_CONSOLE_Z_MOVE,val);
        break;

    case 10:
        if(val>520) val = 520;
        if(val<0) val =0;
        ApplicationDatabase.setData(_CAL_CONSOLE_X_MOVE,val);
        break;
    case 11:
        if(val>470) val = 470;
        if(val<0) val =0;
        ApplicationDatabase.setData(_CAL_CONSOLE_Y_MOVE,val);
        break;
    case 12:
        if(val>ApplicationDatabase.getDataI(_DB_BIOP_ZLIMIT)) val = ApplicationDatabase.getDataI(_DB_BIOP_ZLIMIT);
        if(val<0) val =0;
        ApplicationDatabase.setData(_CAL_CONSOLE_Z_MOVE,val);
        break;
    }


}



bool calibconsole::checkConfigChange(void){
unsigned char* p1 =  (unsigned char*) &config;
unsigned char* p2 =  (unsigned char*) &pBiopsy->config;

    for(int i=0; i<sizeof(biopsyConf_Str); i++){
        if(p1[i] != p2[i]) return true;
    }
    return false;
}

void calibconsole::refreshStatus(void){

    if(ApplicationDatabase.getDataU(_DB_ACCESSORIO)!=BIOPSY_DEVICE)    ui->statusLabel->setText("DEVICE NOT CONNECTED");
    else if(ApplicationDatabase.getDataU(_DB_BIOP_UNLOCK_BUTTON)) ui->statusLabel->setText("UNLOCK BUTTON ACTIVATED!");
    else{
        ui->statusLabel->setText(QString("HOLDER:[%1] - CURSOR (dmm):[X:%2,Y:%3,Z:%4, ZLIM:%5] - JOY (dmm):[X:%6,Y:%7]")\
                                 .arg(ApplicationDatabase.getDataI(_DB_BIOP_HOLDER))\
                                 .arg(ApplicationDatabase.getDataI(_DB_BIOP_X))\
                                 .arg(ApplicationDatabase.getDataI(_DB_BIOP_Y))\
                                 .arg(ApplicationDatabase.getDataI(_DB_BIOP_Z))\
                                 .arg(ApplicationDatabase.getDataI(_DB_BIOP_ZLIMIT))\
                                 .arg(ApplicationDatabase.getDataI(_CAL_CONSOLE_JX))\
                                 .arg(ApplicationDatabase.getDataI(_CAL_CONSOLE_JY))\
                                 );
    }
    return;
}
