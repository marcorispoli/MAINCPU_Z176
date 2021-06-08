// calibpower  calibpower
// calibPowerUI  calibPowerUI
// calibPower  calibPower
// _PG_SERVICE_CALIB_POWER  _PG_SERVICE_CALIB_POWER
// _PG_SERVICE_CALIB_MENU  _PG_SERVICE_CALIB_MENU


#include "calibpower.h"
#include "ui_calibPower.h"


#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"


#define UI_PAGINA _PG_SERVICE_CALIB_POWER
#define EXIT_PAGINA _PG_SERVICE_CALIB_MENU
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000

calibpower::calibpower(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::calibPowerUI)
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
    timer_update = 0;
}

calibpower::~calibpower()
{
    delete ui;
}

// Funzione agganciata ai sistemi di menu custom
void calibpower::changePage(int pg, int opt)
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
void calibpower::onExitButton(void)
{

    GWindowRoot.setNewPage(EXIT_PAGINA,GWindowRoot.curPage,0);
}

void calibpower::initPage(void){


    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
    connect(ui->valueEdit,SIGNAL(selectionChanged()),this,SLOT(onValueEdit()),Qt::UniqueConnection);
    pCalculator = new numericPad(rotview,view, parent);
    connect(pCalculator,SIGNAL(calcSgn(bool)),this,SLOT(calculatorSlot(bool)));
    // connect(ui->storeButton,SIGNAL(released()),this,SLOT(onStoreButton()),Qt::UniqueConnection);


    int vac   = ApplicationDatabase.getDataI(_DB_ACVOLT);
    ApplicationDatabase.setData(_DB_ACVOLT,(int) 10000,DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO); // Forza l'agiornamento
    ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) 10000,DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO); // Campo HV


    // Inizializzazione ui
    if(isMaster){
        ApplicationDatabase.setData(_DB_ACVOLT,vac);
        if(pGeneratore->hv_calibrated){
            ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) pGeneratore->genCnf.pcb190.HV_VPRIMARIO,DBase::_DB_FORCE_SGN);
            ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) pGeneratore->genCnf.pcb190.HV_VAC,DBase::_DB_FORCE_SGN);
        }else{
            ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 0,DBase::_DB_FORCE_SGN);// Primario
            ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) 0,DBase::_DB_FORCE_SGN);// Value
        }

        if(timer_update) killTimer(timer_update);
        timer_update = startTimer(1000);
        pSysLog->log("SERVICE PANEL: CALIB POWER");
    }


}

void calibpower::exitPage(void){

    if(timer_update) killTimer(timer_update);
    if(timerDisable) killTimer(timerDisable);
    timer_update=0;
    timerDisable=0;

    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
    disconnect(ui->valueEdit,SIGNAL(selectionChanged()),this,SLOT(onValueEdit()));
    pCalculator->deleteLater(); // importante !!!
    pCalculator = 0;
}

void calibpower::timerEvent(QTimerEvent* ev)
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
    }else if(ev->timerId() == timer_update){
        if(isMaster){
            // Calcola il valore della tensione
            ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) pGeneratore->hvval);
        }
    }
}

// FUNZIONE DI AGGIORNAMENTO CAMPI VALORE CONNESSO AI CAMPI DEL DATABASE
void calibpower::valueChanged(int index,int opt)
{
    int i,vlenze,vstarter,grid,low,vmain;
    float n2_n1;

    switch(index){
    case _DB_ACVOLT:
        // Si intende il lenze collegato a 220VAC, lo starter a 210VAC, la potenza a 400VAC
        vlenze   = ApplicationDatabase.getDataI(_DB_ACVOLT) / 10;
        vstarter = ApplicationDatabase.getDataI(_DB_ACVOLT)*210/2200;
        grid = ApplicationDatabase.getDataI(_DB_ACVOLT)*70/2200;
        low = ApplicationDatabase.getDataI(_DB_ACVOLT)*190/2200;


        // Inizializzazione ui
        ui->starterVoltage->setText(QString("%1(V)").arg(vstarter));
        ui->lenzeVoltage->setText(QString("%1(V)").arg(vlenze));
        ui->gridVoltage->setText(QString("%1(V)").arg(grid));
        ui->lowSideVoltage->setText(QString("%1(V)").arg(low));
        break;

    case _DB_SERVICE1_INT: // Campo Primario
        i = ApplicationDatabase.getDataI(index);
        if(i) ui->primarySetup->setText(QString("[%1]").arg(i));
        else ui->primarySetup->setText(QString("[---]").arg(i));
        break;

    case _DB_SERVICE2_INT: // Visualizzazione campo Value
        vmain = ApplicationDatabase.getDataI(index);        
        if(vmain)   ui->valueEdit->setText(QString("%1(V)").arg(vmain));
        else        ui->valueEdit->setText(QString("---(V)"));
        break;

    case _DB_SERVICE4_INT: // Campo HV
        ui->powerVoltage->setText(QString("%1(Vdc)").arg(ApplicationDatabase.getDataI(index)));
        break;

    case _DB_SERVICE5_INT: // Richiesta di verifica e store: solo Master
        if(!isMaster) return;
        vmain = ApplicationDatabase.getDataI(index);
        if(!vmain) return;

        // Calcolo del possibile switch
        vlenze   = ApplicationDatabase.getDataI(_DB_ACVOLT) / 10;
        n2_n1 = (float) vlenze / (float) vmain;
        if(n2_n1>1.5) ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 115);
        else if(n2_n1>0.98) ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 220);
        else if(n2_n1>0.936) ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 230);
        else ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 240);

        // Effettua lo Store automaticamente
        onStoreButton();
        break;

    case _DB_SERVICE1_STR: // Casella messaggi
        pWarningBox->activate("STORE COMMAND", ApplicationDatabase.getDataS(index));
        pWarningBox->setTimeout(3000);
        break;
    }

}


void calibpower::onValueEdit(void){
    pCalculator->activate(&calcData, QString("MEASURED AC VOLTAGE (RMS)"), 0);
}

// Riceve la pressione del pulsante OK/CANC
void calibpower::calculatorSlot(bool state){

    if(state==false) return;

    // Verifica che il dato sia coerente
    if(calcData.toInt() > 250){
        pWarningBox->activate("AC VOLTAGE SELECTION","INVALID VOLTAGE SELECTED!",msgBox::_BUTTON_CANC);
        ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) 0); // Cancella la scritta
        return;
    }

    ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) calcData.toInt(),DBase::_DB_FORCE_SGN); // Imposta il valore di lettura della rete
    ApplicationDatabase.setData(_DB_SERVICE5_INT,(int) calcData.toInt(),DBase::_DB_FORCE_SGN); // Richiede calcoli al Master


}

void calibpower::onStoreButton(void){
    int vpower,vmain,vprimario;


    //ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) 1,0); // Store

    if(!isMaster) return;

    if(pGeneratore->hvraw ==0) {
        // Messaggio di errore
        ApplicationDatabase.setData(_DB_SERVICE1_STR,"GENERATOR HV READ VOLTAGE NOT AVAILABLE",DBase::_DB_FORCE_SGN);
        return;
    }

    vprimario = ApplicationDatabase.getDataI(_DB_SERVICE1_INT);
    vmain = ApplicationDatabase.getDataI(_DB_SERVICE2_INT);
    vpower   = (ApplicationDatabase.getDataI(_DB_ACVOLT)*257)/1000; // (VDC) = VLENZE * 400 * 1.414 / (220*1000)

    pGeneratore->genCnf.pcb190.HV_CONVERTION = vpower * 1000 / pGeneratore->hvraw;
    pGeneratore->genCnf.pcb190.HV_VAC = vmain;
    pGeneratore->genCnf.pcb190.HV_VPRIMARIO = vprimario;
    pGeneratore->hv_calibrated = true;
    pGeneratore->storeHVcalibFile();
    ApplicationDatabase.setData(_DB_VPRIMARIO, (int) pGeneratore->genCnf.pcb190.HV_VPRIMARIO);
    ApplicationDatabase.setData(_DB_SERVICE1_STR,"VOLTAGE CALIBRATION SUCCESSFULLY COMPLETED",DBase::_DB_FORCE_SGN);
}
