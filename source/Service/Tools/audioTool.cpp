
#include "audioTool.h"
#include "ui_audio.h"


#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"
#include "../../audio.h"
extern audio* pAudio;

#define UI_PAGINA _PG_SERVICE_TOOLS_AUDIO
#define EXIT_PAGINA _PG_SERVICE_TOOLS_MENU
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000

audiotool::audiotool(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::audioToolUI)
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
    connect(ui->volMButton,SIGNAL(released()),this,SLOT(onVolMButton()),Qt::UniqueConnection);
    connect(ui->volPButton,SIGNAL(released()),this,SLOT(onVolPButton()),Qt::UniqueConnection);
    connect(ui->enabButton,SIGNAL(released()),this,SLOT(onEnabButton()),Qt::UniqueConnection);
    connect(ui->testButton,SIGNAL(released()),this,SLOT(onTestButton()),Qt::UniqueConnection);
    connect(ui->msgNum,SIGNAL(selectionChanged()),this,SLOT(onMsgNum()),Qt::UniqueConnection);


    //______________________________________________________________________________________________________


    timerDisable = 0;
    configChanged = false;

}

audiotool::~audiotool()
{
    delete ui;
}

// Funzione agganciata ai sistemi di menu custom
void audiotool::changePage(int pg, int opt)
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
void audiotool::onExitButton(void)
{
    GWindowRoot.setNewPage(EXIT_PAGINA,GWindowRoot.curPage,0);
}

void audiotool::initPage(void){


    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);

    pCalculator = new numericPad(rotview,view, parent);
    connect(pCalculator,SIGNAL(calcSgn(bool)),this,SLOT(calculatorSlot(bool)));

    // Inizializzazioni competono allo Slave
    if(!isMaster) return;

    // Impostazione dell'abilitazione user
    if(pConfig->userCnf.audioEnable) ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) 1,DBase::_DB_FORCE_SGN);
    else ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) 0,DBase::_DB_FORCE_SGN);

    // Impostazione della presenza del device AUDIO
    if(ApplicationDatabase.getDataU(_DB_AUDIO_PRESENT)) ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 1,DBase::_DB_FORCE_SGN);
    else ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 0,DBase::_DB_FORCE_SGN);

    // Impostazione Volume
    ApplicationDatabase.setData(_DB_SERVICE3_INT,(int)  pConfig->userCnf.volumeAudio ,DBase::_DB_FORCE_SGN);

    // Impostazione campo numero messaggio di test
    ApplicationDatabase.setData(_DB_SERVICE6_INT,(int) 1 ,DBase::_DB_FORCE_SGN);

    // Pre-impostazione dell'azione test message
    ApplicationDatabase.setData(_DB_SERVICE7_INT,(int) 0 ,DBase::_DB_FORCE_SGN);

    // Disabilito la mute se dovesse essere attiva
    pAudio->setMute(false);

    configChanged = false;
}

// Operazioni da compiere all'uscita dalla pagina
void audiotool::exitPage(void){

    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
    disconnect(ui->volMButton,SIGNAL(released()),this,SLOT(onVolMButton()));
    disconnect(ui->volPButton,SIGNAL(released()),this,SLOT(onVolPButton()));
    disconnect(ui->enabButton,SIGNAL(released()),this,SLOT(onEnabButton()));
    disconnect(ui->testButton,SIGNAL(released()),this,SLOT(onTestButton()));
    disconnect(ui->msgNum,SIGNAL(selectionChanged()),this,SLOT(onMsgNum()));

    disconnect(pCalculator);
    pCalculator->deleteLater(); // importante !!!
    pCalculator = 0;

    if(!isMaster) return;
    if(configChanged) pConfig->saveUserCfg();
}

void audiotool::timerEvent(QTimerEvent* ev)
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
void audiotool::valueChanged(int index,int opt)
{
    int val;

    switch(index){
    case _DB_SERVICE1_INT: // AUDIO PRESENT
        if(ApplicationDatabase.getDataI(index)==0){
            ui->notDetectedFrame->show();
            if(ApplicationDatabase.getDataI(_DB_SERVICE2_INT)) ui->notDetectedLabel->show();
        }else{
            ui->notDetectedFrame->hide();
            ui->notDetectedLabel->hide();
        }

        break;


    case _DB_SERVICE2_INT: // Pulsante Enable
        if(ApplicationDatabase.getDataI(index)==1){
            ui->enabButton->setStyleSheet("background-image:url(:/transparent.png);border-image:url(:/Pulsanti/Pulsanti/but_on.png);background-color: rgb(0, 0, 0,0);");
        }else{
            ui->enabButton->setStyleSheet("background-image:url(:/transparent.png);border-image:url(:/Pulsanti/Pulsanti/but_off.png);background-color: rgb(0, 0, 0,0);");
        }
        break;

    case _DB_SERVICE3_INT: // Volume corrente
        val = ApplicationDatabase.getDataI(index);
        ui->volume->setText(QString("%1").arg(10*(10-val)));
        if(isMaster) pConfig->userCnf.volumeAudio = val;
        break;

    case _DB_SERVICE4_INT: // Modifica volume
        if(!isMaster) return;
        if(ApplicationDatabase.getDataI(index)==1){
            // Incrementa il volume
            if(pConfig->userCnf.volumeAudio==0) return;
            else pConfig->userCnf.volumeAudio--;
        }else{
            // decrementa il volume
            if(pConfig->userCnf.volumeAudio>=10){
                pConfig->userCnf.volumeAudio = 10;
                return;
            } else pConfig->userCnf.volumeAudio++;
        }

        configChanged = true;
        ApplicationDatabase.setData(_DB_SERVICE3_INT,(int)  pConfig->userCnf.volumeAudio);
        break;

    case _DB_SERVICE5_INT: // Modifica pulsante enable
        if(!isMaster) return;
        configChanged = true;
        if(pConfig->userCnf.audioEnable){
            pConfig->userCnf.audioEnable = false;
            ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) 0);
        }else{
            pConfig->userCnf.audioEnable = true;
            ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) 1);
        }

       break;

    case _DB_SERVICE6_INT: // Campo numero messaggio da testare

        ui->msgNum->setText(QString("#%1").arg(ApplicationDatabase.getDataI(index)));

       break;

    case _DB_SERVICE7_INT: // Pulsante di Test
        if(!isMaster) return;
        if(ApplicationDatabase.getDataI(index)==0) return;
        pAudio->playAudio(ApplicationDatabase.getDataI(_DB_SERVICE6_INT));

       break;

    }

}

void audiotool::onVolMButton(void){
    if(timerDisable) return;
    timerDisable = startTimer(500);

    ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) 0,DBase::_DB_FORCE_SGN);

}

void audiotool::onVolPButton(void){
    if(timerDisable) return;
    timerDisable = startTimer(500);
    ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) 1,DBase::_DB_FORCE_SGN);

}
void audiotool::onEnabButton(void){
    if(timerDisable) return;
    timerDisable = startTimer(500);

    ApplicationDatabase.setData(_DB_SERVICE5_INT,(int) 1,DBase::_DB_FORCE_SGN);

}

void audiotool::onTestButton(void){
    if(timerDisable) return;
    timerDisable = startTimer(500);

    ApplicationDatabase.setData(_DB_SERVICE7_INT,(int) 1,DBase::_DB_FORCE_SGN);

}

void audiotool::onMsgNum(void){
    if(timerDisable) return;
    timerDisable = startTimer(500);

    dataField = QString("%1").arg(ApplicationDatabase.getDataI(_DB_SERVICE6_INT));
    pCalculator->activate(&dataField, QString("SET MESSAGE NUMBER"), 0);
}

void audiotool::calculatorSlot(bool state){
    if(state==false) return;

    // Controllo sul valore acquisito
    if(dataField=="") return;
    int val = dataField.toInt();
    if(val<1) return;
    if(val>100) return;
    ApplicationDatabase.setData(_DB_SERVICE6_INT,val);

}


