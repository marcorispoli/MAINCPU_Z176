
#include "pottertool.h"
#include "ui_potter.h"


#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"


#define UI_PAGINA _PG_SERVICE_TOOLS_POTTER
#define EXIT_PAGINA _PG_SERVICE_TOOLS_MENU
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000

pottertool::pottertool(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::potterToolUI)
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

pottertool::~pottertool()
{
    delete ui;
}

// Funzione agganciata ai sistemi di menu custom
void pottertool::changePage(int pg,  int opt)
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
void pottertool::onExitButton(void)
{

    GWindowRoot.setNewPage(EXIT_PAGINA,GWindowRoot.curPage,0);
}

void pottertool::initPage(void){


    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
    pCalculator = new numericPad(rotview,view, parent);
    connect(pCalculator,SIGNAL(calcSgn(bool)),this,SLOT(calculatorSlot(bool)));
    connect(ui->gridCycles,SIGNAL(selectionChanged()),this,SLOT(onGridCycles()),Qt::UniqueConnection);
    connect(ui->startGrid,SIGNAL(released()),this,SLOT(onStartGrid()),Qt::UniqueConnection);

    // Inizializzazioni
    if(isMaster){
        ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 10,DBase::_DB_FORCE_SGN); // Inputs/Outputs
    }

}

// Operazioni da compiere all'uscita dalla pagina
void pottertool::exitPage(void){


    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
    disconnect(pCalculator,SIGNAL(calcSgn(bool)),this,SLOT(calculatorSlot(bool)));
    disconnect(ui->gridCycles,SIGNAL(selectionChanged()),this,SLOT(onGridCycles()));
    disconnect(ui->startGrid,SIGNAL(released()),this,SLOT(onStartGrid()));

    disconnect(pCalculator);
    pCalculator->deleteLater(); // importante !!!
    pCalculator = 0;

    
}

void pottertool::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId()==changePageTimer)
    {
        killTimer(changePageTimer);
        // Abilita il pulsante di uscita
        EXIT_BUTTON->show();
    }else if(ev->timerId()==timerDisable)
    {
        killTimer(timerDisable);
        timerDisable=0;
    }

}

// FUNZIONE DI AGGIORNAMENTO CAMPI VALORE CONNESSO AI CAMPI DEL DATABASE
void pottertool::valueChanged(int index,int opt)
{
    unsigned int i;

    switch(index){
    case _DB_SERVICE1_INT: // Cicli di test
        ui->gridCycles->setText(QString("%1").arg(ApplicationDatabase.getDataI(index)));
        break;

    case _DB_SERVICE2_INT: // StartGrid
        if(!isMaster) return;
        pPotter->startTestGrid(ApplicationDatabase.getDataI(_DB_SERVICE1_INT));
        break;


    }
}


void pottertool::onGridCycles(void){
    if(timerDisable) return;
    timerDisable = startTimer(500);

    dataField = QString("%1").arg(ApplicationDatabase.getDataI(_DB_SERVICE1_INT));
    pCalculator->activate(&dataField, QString("SET GRID CYCLES"), 0);
}

void pottertool::calculatorSlot(bool state){
    if(state==false) return;

    // Controllo sul valore acquisito
    if(dataField=="") return;
    int val = dataField.toInt();
    if(val<1) return;
    if(val>255) return;
    ApplicationDatabase.setData(_DB_SERVICE1_INT,val);

}

void pottertool::onStartGrid(void){
    if(timerDisable) return;
    timerDisable = startTimer(500);

    ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) 1,DBase::_DB_FORCE_SGN);

}



