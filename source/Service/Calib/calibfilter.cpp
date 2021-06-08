#include "calibfilter.h"
#include "ui_calibFilter.h"


#include "../../application.h"
#include "../../appinclude.h"
#include "../../globvar.h"


#define UI_PAGINA _PG_SERVICE_CALIB_FILTER
#define EXIT_PAGINA _PG_SERVICE_CALIB_MENU
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000


/*
 *
    colliConf.filterType[0]=FILTRO_Rh;
    colliConf.filterType[1]=FILTRO_Ag;
    colliConf.filterType[2]=FILTRO_Al;
    colliConf.filterType[3]=FILTRO_Cu;

    colliConf.filterPos[0] = 39;
    colliConf.filterPos[1] = 97;
    colliConf.filterPos[2] = 163;
    colliConf.filterPos[3] = 224;
 *
 */

calibfilter::calibfilter(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::calibFilterUI)
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

    connect(ui->button1,SIGNAL(released()),this,SLOT(onButton1()),Qt::UniqueConnection);
    connect(ui->button2,SIGNAL(released()),this,SLOT(onButton2()),Qt::UniqueConnection);
    connect(ui->button3,SIGNAL(released()),this,SLOT(onButton3()),Qt::UniqueConnection);
    connect(ui->button4,SIGNAL(released()),this,SLOT(onButton4()),Qt::UniqueConnection);
    connect(ui->adjustRight,SIGNAL(released()),this,SLOT(onAdjustRight()),Qt::UniqueConnection);
    connect(ui->adjustLeft,SIGNAL(released()),this,SLOT(onAdjustLeft()),Qt::UniqueConnection);
    connect(ui->storeButton,SIGNAL(released()),this,SLOT(onStoreButton()),Qt::UniqueConnection);
    connect(ui->playTest,SIGNAL(released()),this,SLOT(onTestButton()),Qt::UniqueConnection);
    connect(ui->stopTest,SIGNAL(released()),this,SLOT(onStopButton()),Qt::UniqueConnection);

    timerDisable = 0;
    timer_test = 0;

    // Sequenza di test ciclico
    sequenza.clear();
    sequenza.append((char) 0);
    sequenza.append(1);
    sequenza.append((char)0);
    sequenza.append(2);
    sequenza.append((char)0);
    sequenza.append(3);

    sequenza.append(1);
    sequenza.append((char)0);
    sequenza.append(1);
    sequenza.append(2);
    sequenza.append(1);
    sequenza.append(3);

    sequenza.append(2);
    sequenza.append((char)0);
    sequenza.append(2);
    sequenza.append(1);
    sequenza.append(2);
    sequenza.append(3);

    sequenza.append((char)0);
    sequenza.append(3);
    sequenza.append(1);
    sequenza.append(3);
    sequenza.append(2);

}

calibfilter::~calibfilter()
{
    delete ui;
}

// Funzione agganciata ai sistemi di menu custom
void calibfilter::changePage(int pg,int opt)
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
void calibfilter::onExitButton(void)
{


    GWindowRoot.setNewPage(EXIT_PAGINA,GWindowRoot.curPage,0);
}


void calibfilter::initPage(void){

    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);

    ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) 255, DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO); // Inizializza segnale per bottone di selezione
    ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) 1000, DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO); // Inizializza segnale per bottone di selezione
    ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) 0, DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO); // Inizializza segnale per bottone di selezione
    ApplicationDatabase.setData(_DB_SERVICE5_INT,(int) 0, DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO); // Inizializza segnale per bottone di selezione
    ApplicationDatabase.setData(_DB_SERVICE6_INT,(int) 0, DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO); // Inizializza segnale per bottone di selezione
    ApplicationDatabase.setData(_DB_SERVICE7_INT,(int) 0); // Inizializza errori
    ui->frameFilterButton->show();
    ui->frameStore->show();
    ui->testLabel->hide();
    ui->frameErrors->hide();
    errors = 0;
    ui->adjustLeft->hide();
    ui->adjustRight->hide();

    if(isMaster){
        connect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(guiNotifySlot(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);


        for(int i=0; i<4; i++){
            posizioneIniziale[i]=pCollimatore->colliConf.filterPos[i];

            if(pCollimatore->colliConf.filterType[i]==Collimatore::FILTRO_Rh){
                fname[i]="Rh";
            }
            else if(pCollimatore->colliConf.filterType[i]==Collimatore::FILTRO_Ag){
                fname[i]="Ag";
            }
            else if(pCollimatore->colliConf.filterType[i]==Collimatore::FILTRO_Al) {
                fname[i]="Al";
            }
            else if(pCollimatore->colliConf.filterType[i]==Collimatore::FILTRO_Cu) {
                fname[i]="Cu";
            }
            else if(pCollimatore->colliConf.filterType[i]==Collimatore::FILTRO_Mo) {
                fname[i]="Mo";
            }
            else {
                fname[i]="";
            }

            pSysLog->log("SERVICE PANEL: CALIB FILTER");
        }

        ApplicationDatabase.setData(_DB_SERVICE1_STR,fname[0], DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_SERVICE2_STR,fname[1], DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_SERVICE3_STR,fname[2], DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_SERVICE4_STR,fname[3], DBase::_DB_FORCE_SGN);

        indiceCorrente = -1;
        // Inizializza quale è la posizione attuale del filtro
        ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 255, DBase::_DB_FORCE_SGN);
        ApplicationDatabase.setData(_DB_SERVICE5_STR,"-", DBase::_DB_FORCE_SGN);

        // Lancia la selezione del filtro iniziale a posizione 0
        pCollimatore->manualFiltroCollimation=TRUE;
        pCollimatore->manualSetFiltro(0);

    }

}

// Operazioni da compiere all'uscita dalla pagina
void calibfilter::exitPage(void){

    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
    if(timerDisable) killTimer(timerDisable);
    if(timer_test) killTimer(timer_test);
    timerDisable = 0;
    timer_test = 0;

    if(isMaster){
        disconnect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(guiNotifySlot(unsigned char,unsigned char,QByteArray)));

        // Ripristino valori (se non sono stati salvati!
        pCollimatore->colliConf.filterPos[0] = posizioneIniziale[0];
        pCollimatore->colliConf.filterPos[1] = posizioneIniziale[1];
        pCollimatore->colliConf.filterPos[2] = posizioneIniziale[2];
        pCollimatore->colliConf.filterPos[3] = posizioneIniziale[3];

    }

}

void calibfilter::timerEvent(QTimerEvent* ev)
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

    if(ev->timerId()==timer_test)
    {
        killTimer(timer_test);
        pCollimatore->manualSetFiltro(sequenza.at(indice_sequenza));
        indice_sequenza++;
        if(indice_sequenza==sequenza.size()) indice_sequenza = 0;
        timer_test = startTimer(5000);
    }
}

// FUNZIONE DI AGGIORNAMENTO CAMPI VALORE CONNESSO AI CAMPI DEL DATABASE
#define STYLESHEET1 QString("border-image:url(:/Service_Calib/Service_Calib/Filtro1.png);background-image:url(:/transparent.png);")
#define STYLESHEET2 QString("border-image:url(:/Service_Calib/Service_Calib/Filtro2.png);background-image:url(:/transparent.png);")
#define STYLESHEET3 QString("border-image:url(:/Service_Calib/Service_Calib/Filtro3.png);background-image:url(:/transparent.png);")
#define STYLESHEET4 QString("border-image:url(:/Service_Calib/Service_Calib/Filtro4.png);background-image:url(:/transparent.png);")
#define STYLESHEETNULL QString("border-image:url(:/Service_Calib/Service_Calib/FiltroNull.png);background-image:url(:/transparent.png);")
void calibfilter::valueChanged(int index,int opt)
{
    int i,ii;

    switch(index){
        case _DB_SERVICE1_STR:
            ui->button1->setText(ApplicationDatabase.getDataS(index));

        break;
        case _DB_SERVICE2_STR:
            ui->button2->setText(ApplicationDatabase.getDataS(index));

        break;
        case _DB_SERVICE3_STR:
            ui->button3->setText(ApplicationDatabase.getDataS(index));

        break;
        case _DB_SERVICE4_STR:
            ui->button4->setText(ApplicationDatabase.getDataS(index));

        break;

        case _DB_SERVICE1_INT: // Indice filtro selezionato
            i = ApplicationDatabase.getDataI(index); // Preleva il numero del filtro selezionato (0:3)
            if(i==0) ui->rotofilterFrame->setStyleSheet( STYLESHEET1);
            else if(i==1) ui->rotofilterFrame->setStyleSheet(STYLESHEET2);
            else if(i==2) ui->rotofilterFrame->setStyleSheet(STYLESHEET3);
            else if(i==3) ui->rotofilterFrame->setStyleSheet(STYLESHEET4);
            else{
                ui->rotofilterFrame->setStyleSheet(STYLESHEETNULL);
                // Disattiva i pulsanti per aggiustare la posizione
                ui->adjustLeft->hide();
                ui->adjustRight->hide();
                return;
            }

            // Riattiva i pulsanti, ma solo se non c'è il test in corso
            if(ApplicationDatabase.getDataI(_DB_SERVICE6_INT)!=1){
                ui->adjustLeft->show();
                ui->adjustRight->show();
            }
        break;

        case _DB_SERVICE5_STR: // Materiale impostato
            ui->materialeLabel->setText(ApplicationDatabase.getDataS(index));

        break;

        case _DB_SERVICE2_INT: // Pulsanti di selezione filtro            
            if(!isMaster) return;
            if(fname[ApplicationDatabase.getDataI(index)]=="") return;
            pCollimatore->manualSetFiltro(ApplicationDatabase.getDataI(index));

        break;
        case _DB_SERVICE3_INT:  // Posizione fisica filtro
            ui->positionLabel->setText(QString("[%1]").arg(ApplicationDatabase.getDataI(index)));
        break;

        case _DB_SERVICE4_INT:  // ADJUST LEFT = 1, RIGHT = 2
            i = ApplicationDatabase.getDataI(_DB_SERVICE1_INT);
            ii = ApplicationDatabase.getDataI(index);            
            if(i>3) return;

            if(isMaster){
                if(ii==1){
                    // LEFT: aumento di 1
                    if(pCollimatore->colliConf.filterPos[i]<255)  pCollimatore->colliConf.filterPos[i]++;
                }else{
                    // RIGHT: diminuisce di 1
                    if(pCollimatore->colliConf.filterPos[i]>0)  pCollimatore->colliConf.filterPos[i]--;
                }

                // Comando
                indiceCorrente = i; // Serve per riportare il filtro nella posizione originale
                if(i==3) i=2;
                else if(i==0) i=1;
                else if(i==1) i=2;
                else i=3;

                // Muove il filtro in posizione differente per poi riportarlo nella posizione corretta
                pCollimatore->manualSetFiltro(i);
            }


        break;
    case _DB_SERVICE5_INT:  // Store button        
        pWarningBox->activate(QString("CALIB FILTER"), QString("STORING DATA"),0);
        pWarningBox->setTimeout(3000);

        if(isMaster){
            posizioneIniziale[0] = pCollimatore->colliConf.filterPos[0];
            posizioneIniziale[1] = pCollimatore->colliConf.filterPos[1];
            posizioneIniziale[2] = pCollimatore->colliConf.filterPos[2];
            posizioneIniziale[3] = pCollimatore->colliConf.filterPos[3];
            pCollimatore->storeConfigFile();
            return;
        }
    break;

    case _DB_SERVICE6_INT: // Pulsanti di play stop
        i = ApplicationDatabase.getDataI(_DB_SERVICE6_INT);

        if(i==1){
            ui->frameFilterButton->hide();
            ui->frameStore->hide();
            ui->testLabel->show();
            ui->adjustLeft->hide();
            ui->adjustRight->hide();

            if(!isMaster) return;
            if(timer_test) return; // Già partito il test
            // PLAY TEST
            indice_sequenza = 0;
            timer_test = startTimer(0);
            ApplicationDatabase.setData(_DB_SERVICE7_INT,(int) 0);

        }else{
            // STOP TEST
            ui->frameFilterButton->show();
            ui->frameStore->show();
            ui->testLabel->hide();

            if(!isMaster) return;
            if(timer_test==0) return;
            killTimer(timer_test);
            timer_test=0;
        }
        break;
    case _DB_SERVICE7_INT:
        i = ApplicationDatabase.getDataI(index);
        if(i==0) ui->frameErrors->hide();
        else {
         ui->frameErrors->show();
         ui->errorLabel->setText(QString("%1").arg(i));
        }

    break;
    }
}



void calibfilter::guiNotifySlot(unsigned char id, unsigned char mcccode, QByteArray buffer)
{
    int i;

    if(!id) return;
    switch(mcccode)
    {

    case MCC_SET_FILTRO:
        if(buffer.at(0)==0)
        {
            // Errore di collimazione
            i = ApplicationDatabase.getDataI(_DB_SERVICE7_INT);
            i++;
            ApplicationDatabase.setData(_DB_SERVICE7_INT, i);
            ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) 255); // Indice filtro
            ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) buffer.at(2)); // Posizione fisica filtro
            ApplicationDatabase.setData(_DB_SERVICE5_STR,""); // Tag assegnato al filtro


        }else
        {
            if(buffer.at(1)>3) return;

            if(indiceCorrente>=0){
                pCollimatore->manualSetFiltro(indiceCorrente);
                indiceCorrente=-1;
                return;
            }

            // Nel caso di successo, il target del filtro selezionato viene aggiornato
            //pCollimatore->colliConf.filterPos[buffer.at(1)] = buffer.at(2);

            ApplicationDatabase.setData(_DB_SERVICE1_INT,(int) buffer.at(1)); // Indice filtro
            ApplicationDatabase.setData(_DB_SERVICE3_INT,(int) buffer.at(2)); // Posizione fisica filtro
            ApplicationDatabase.setData(_DB_SERVICE5_STR,pCollimatore->getFiltroTag(pCollimatore->colliConf.filterType[buffer.at(1)])); // Tag assegnato al filtro


        }
    break;


    }

    return;
}


void calibfilter::onButton1(void){
    ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) 0);
}
void calibfilter::onButton2(void){
    ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) 1);
}
void calibfilter::onButton3(void){
    ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) 2);
}
void calibfilter::onButton4(void){
    ApplicationDatabase.setData(_DB_SERVICE2_INT,(int) 3);
}

void calibfilter::onAdjustLeft(void){
    ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) 1, DBase::_DB_FORCE_SGN);
}
void calibfilter::onAdjustRight(void){
    ApplicationDatabase.setData(_DB_SERVICE4_INT,(int) 2, DBase::_DB_FORCE_SGN);
}

void calibfilter::onStoreButton(void){
    ApplicationDatabase.setData(_DB_SERVICE5_INT,(int) 1, DBase::_DB_FORCE_SGN);
}

void calibfilter::onTestButton(void){
    ApplicationDatabase.setData(_DB_SERVICE6_INT,(int) 1);
}
void calibfilter::onStopButton(void){
    ApplicationDatabase.setData(_DB_SERVICE6_INT,(int) 2);
}

