#include "calibmenu.h"
#include "ui_calibmenu.h"

#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"

#include "../../ANALOG/Calibration/pageCalibAnalogic.h"
extern AnalogCalibPageOpen* paginaCalibAnalogic;

#define UI_PAGINA _PG_SERVICE_CALIB_MENU
#define EXIT_PAGINA _PG_SERVICE_MENU
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000

CalibMenu::CalibMenu(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CalibMenuObject)
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

    // IMPOSTAZIONI STANDARD _______________________________________________________________________________
    connect(&GWindowRoot,SIGNAL(changePage(int,int)), this,SLOT(changePage(int,int)),Qt::UniqueConnection);
    connect(EXIT_BUTTON,SIGNAL(released()),this,SLOT(onExitButton()),Qt::UniqueConnection);

    //______________________________________________________________________________________________________


    connect(ui->StarterCalibButton,SIGNAL(released()),this,SLOT(onStarterCalibButton()),Qt::UniqueConnection);
    connect(ui->HVCalibButton,SIGNAL(released()),this,SLOT(onHVCalibButton()),Qt::UniqueConnection);
    connect(ui->PositionCalibButton,SIGNAL(released()),this,SLOT(onPositionCalibButton()),Qt::UniqueConnection);
    connect(ui->CompressionCalibButton,SIGNAL(released()),this,SLOT(onForceCalibButton()),Qt::UniqueConnection);
    connect(ui->TiltCalibButton,SIGNAL(released()),this,SLOT(onTiltCalibButton()),Qt::UniqueConnection);
    connect(ui->FilterCalibButton,SIGNAL(released()),this,SLOT(onFilterCalibButton()),Qt::UniqueConnection);
    connect(ui->lenzePotCalibButton,SIGNAL(released()),this,SLOT(onLenzePotCalibButton()),Qt::UniqueConnection);
    connect(ui->detectorButton,SIGNAL(released()),this,SLOT(onDetectorCalibButton()),Qt::UniqueConnection);
    connect(ui->ColliCalibButton,SIGNAL(released()),this,SLOT(onColliCalibButton()),Qt::UniqueConnection);
    connect(ui->consoleButton,SIGNAL(released()),this,SLOT(onConsoleButton()),Qt::UniqueConnection);


}

CalibMenu::~CalibMenu()
{
    delete ui;
}

// Funzione agganciata ai sistemi di menu custom
void CalibMenu::changePage(int pg,  int opt)
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
        exitPage();
    }


}

// Reazione alla pressione del pulsante di uscita
void CalibMenu::onExitButton(void)
{

    GWindowRoot.setNewPage(EXIT_PAGINA,GWindowRoot.curPage,0);
}

void CalibMenu::initPage(void){

    unsigned char architettura =  ApplicationDatabase.getDataU(_DB_SYSTEM_CONFIGURATION);

    // Calibrazione Starter Interno solo se configurato
    if(architettura & _ARCH_HIGH_SPEED_STARTER) ui->StarterCalibButton->hide();
    else ui->StarterCalibButton->show();

    // Calibrazione Console solo se Biopsia connessa
    ui->labelConsole->show();
    ui->consoleButton->show();
    ui->frameDetector->show();

    return;
}

void CalibMenu::exitPage(void){


    return;
}

void CalibMenu::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId()==changePageTimer)
    {
        killTimer(changePageTimer);
        // Abilita il pulsante di uscita
        EXIT_BUTTON->show();
    }
}



//____________________________________________________________________________________________________________________________________
// Reazione alla pressione del pulsante di uscita
void CalibMenu::onTiltCalibButton(void)
{
        GWindowRoot.setNewPage(_PG_SERVICE_CALIB_ZEROSETTING,GWindowRoot.curPage,0);
}

void CalibMenu::onFilterCalibButton(void)
{

        GWindowRoot.setNewPage(_PG_SERVICE_CALIB_FILTER,GWindowRoot.curPage,0);
}

void CalibMenu::onForceCalibButton(void)
{
        GWindowRoot.setNewPage(_PG_SERVICE_CALIB_FORCE,GWindowRoot.curPage,0);
}

void CalibMenu::onPositionCalibButton(void)
{
        GWindowRoot.setNewPage(_PG_SERVICE_CALIB_POSITION,GWindowRoot.curPage,0);
}

void CalibMenu::onHVCalibButton(void)
{        
        GWindowRoot.setNewPage(_PG_SERVICE_CALIB_POWER,GWindowRoot.curPage,0);
}


void CalibMenu::onStarterCalibButton(void)
{
    // Solo se configurato
    GWindowRoot.setNewPage(_PG_SERVICE_CALIB_STARTER,GWindowRoot.curPage,0);

}

void CalibMenu::onLenzePotCalibButton(void)
{

        GWindowRoot.setNewPage(_PG_SERVICE_CALIB_LENZE_POT,GWindowRoot.curPage,0);
}

void CalibMenu::onDetectorCalibButton(void)
{
    paginaCalibAnalogic->openPageRequest(_EXPOSURE_MODE_CALIB_MODE_EXPOSIMETER);
}

void CalibMenu::onColliCalibButton(void)
{
    paginaCalibAnalogic->openPageRequest(_EXPOSURE_MODE_ANALOG_COLLIMATION_EXPOSURE);

}

void CalibMenu::onConsoleButton(void)
{
    GWindowRoot.setNewPage(_PG_SERVICE_CALIB_CONSOLE,GWindowRoot.curPage,0);

}
