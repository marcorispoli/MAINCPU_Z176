#include "servicepanelmenu.h"
#include "ui_servicepanelmenu.h"
#include "../application.h"
#include "../appinclude.h"
#include "../globvar.h"

#define UI_PAGINA _PG_SERVICE_MENU
#define EXIT_PAGINA _PG_MAIN
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000

ServicePanelMenu::ServicePanelMenu(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServicePanelMenu)
{
    ui->setupUi(this);
    parentWidget=parent;
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


    connect(ui->ToolsMenuButton,SIGNAL(released()),this,SLOT(onToolsMenuButton()),Qt::UniqueConnection);
    connect(ui->CalibMenuButton,SIGNAL(released()),this,SLOT(onCalibMenuButton()),Qt::UniqueConnection);
    connect(ui->PackagePanelButton,SIGNAL(released()),this,SLOT(onPackagePanelButton()),Qt::UniqueConnection);

    pCalculator = new numericPad(rotview,view, parent);


}

ServicePanelMenu::~ServicePanelMenu()
{
    delete ui;
}

void ServicePanelMenu::initPage(void){
    // Disabilitazione allarmi di sistema
    paginaAllarmi->alarm_enable = false;

    // Disabilita il pulsante di uscita per un certo tempo
    EXIT_BUTTON->hide();
    changePageTimer = startTimer(DISABLE_EXIT_TMO);
    view->show();
}
void ServicePanelMenu::passwordSlot(bool result){

    if(pCalculator->activation_code == PASSWORD){

        if(!result) {
            ApplicationDatabase.setData(_DB_SERVICE_PASSWORD,"",DBase::_DB_FORCE_SGN);
            return;
        }
        ApplicationDatabase.setData(_DB_SERVICE_PASSWORD,calcData,DBase::_DB_FORCE_SGN);
        return;
    }

}

// Funzione agganciata ai sistemi di menu custom
void ServicePanelMenu::changePage(int pg, int opt)
{
    if(UI_PAGINA==pg)
    {

        // Attivazione pagina
        if(GWindowRoot.curPageVisible== TRUE){
            connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);

            if(ApplicationDatabase.getDataS(_DB_SERVICE_PASSWORD)==ApplicationDatabase.getDataS(_DB_PASSWORD)){
                initPage();
            }else{
                connect(pCalculator,SIGNAL(calcSgn(bool)),this,SLOT(passwordSlot(bool)),Qt::UniqueConnection);
                calcData = "";
                pCalculator->activate(&calcData, QString("PASSWORD REQUESTED"), PASSWORD);
                pCalculator->setCripto();
            }

        }
        else view->hide();
        return;
    }
    else if(GWindowRoot.curPage==UI_PAGINA)
    {
        // Disattivazione pagina
        paginaAllarmi->alarm_enable = true;
        view->hide();
        disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
        disconnect(pCalculator,SIGNAL(calcSgn(bool)),this,SLOT(passwordSlot(bool)));
    }

}

void ServicePanelMenu::valueChanged(int index,int opt){
    switch(index)
    {

    case _DB_SERVICE_PASSWORD:

        pCalculator->hide();
        disconnect(pCalculator,SIGNAL(calcSgn(bool)),this,SLOT(passwordSlot(bool)));
        if(ApplicationDatabase.getDataS(_DB_SERVICE_PASSWORD)==ApplicationDatabase.getDataS(_DB_PASSWORD)) initPage();
        else  if(isMaster) {onExitButton();}
        break;
    }
}

// Reazione alla pressione del pulsante di uscita
void ServicePanelMenu::onExitButton(void)
{   
    GWindowRoot.setNewPage(_PG_MAIN_ANALOG,GWindowRoot.curPage,0);
}

void ServicePanelMenu::timerEvent(QTimerEvent* ev)
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


void ServicePanelMenu::onToolsMenuButton(void)
{

        GWindowRoot.setNewPage(_PG_SERVICE_TOOLS_MENU,GWindowRoot.curPage,0);
}

void ServicePanelMenu::onCalibMenuButton(void)
{
        GWindowRoot.setNewPage(_PG_SERVICE_CALIB_MENU,GWindowRoot.curPage,0);
}

void ServicePanelMenu::onPackagePanelButton(void)
{

    GWindowRoot.setNewPage(_PG_SYSTEM_SETUP,GWindowRoot.curPage,0);
}
