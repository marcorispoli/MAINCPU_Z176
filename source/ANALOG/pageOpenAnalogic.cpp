#include "pageOpenAnalogic.h"
#include "pannelloComandi.h"
#include "analog.h"
#include "ui_analog.h"
#include "../application.h"
#include "../appinclude.h"
#include "../globvar.h"
#include "../audio.h"
#include "../systemlog.h"
extern systemLog* pSysLog;

extern audio* pAudio;

int gradi=0;
int timerAngolo;


#define UI_PAGINA _PG_OPEN_STUDY_ANALOG
#define EXIT_PAGINA _PG_OPEN_STUDY_ANALOG
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000




AnalogPageOpen::AnalogPageOpen(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::analogUI)
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



    // Crea pannello comandi nella rispettiva view
    commandPanel = new pannelloComandi(ui->comandi);
    compressionPanel = new pannelloCompressione(ui->forza);
    thicknessPanel = new pannelloSpessore(ui->spessore);
    kvPanel = new pannelloKv(ui->kv);
    masPanel = new pannelloMas(ui->mAs);
    optionPanel = new pannelloOpzioni(ui->options);
    projPanel = new pannelloProiezioni(ui->proiezioni);
    colliPanel = new pannelloColli(ui->manualColliPanel,parent);
    biopsyPanel = new pannelloBiopsia(ui->biopsia);


    // Ready not ready
    currentAngolo = 0;


    if(isMaster) ui->sblocco->setGeometry(715,335,81,81);
    else ui->sblocco->setGeometry(4,335,81,81);


    if(isMaster) ui->manColliFrame->setGeometry(615,335,96,76);
    else ui->manColliFrame->setGeometry(89,335,96,76);

    if(isMaster) ui->pad->setGeometry(10,200,141,81);
    else ui->pad->setGeometry(649,200,141,81);

    if(isMaster) ui->unlock_pad->setGeometry(35,205,101,76);
    else ui->unlock_pad->setGeometry(664,205,101,76);

    if(isMaster) ui->unlock_compressor->setGeometry(35,205,101,76);
    else ui->unlock_compressor->setGeometry(664,205,101,76);

    if(isMaster) ui->alarm_frame->setGeometry(685,65,101,96);
    else ui->alarm_frame->setGeometry(14,65,101,96);
    ui->alarm_frame->hide();

    if(isMaster) ui->frameTubeTemp->setGeometry(665,195,121,106);
    else   ui->frameTubeTemp->setGeometry(14,195,121,106);
    ui->frameTubeTempAlarm->hide();

    ui->frameDetNotCalibrated->setGeometry(175,0,71,91);
    ui->frameProfileNotCalibrated->setGeometry(175,0,71,61);

    ui->frameInvalidPotter->setGeometry(710,5,91,71);
    ui->frameInvalidPad->setGeometry(710,5,91,71);
    ui->frameMissingCassette->setGeometry(630,0,76,76);
    ui->frameExposedCassette->setGeometry(630,0,76,76);
    ui->frameNotExposedCassette->setGeometry(630,0,76,76);

    ui->frameOpenDoor->setGeometry(570,0,66,76);


    if(isMaster) ui->frameMissingCompression->setGeometry(100,280,46,41);
    else    ui->frameMissingCompression->setGeometry(735,280,46,41);

    //if(isMaster) view->setGeometry(20,285,126,126);
    //else view->setGeometry(655,285,126,126);

    ui->frameInvalidPotter->hide();
    ui->frameInvalidPad->hide();
    ui->frameMissingCassette->hide();
    ui->frameExposedCassette->hide();
    ui->frameNotExposedCassette->hide();
    ui->frameOpenDoor->hide();
    ui->frameMissingCompression->hide();
    ui->frameDetNotCalibrated->hide();
    ui->frameProfileNotCalibrated->hide();

    /*
    if(isMaster) ui->info_frame->setGeometry(690,195,101,96);
    else ui->info_frame->setGeometry(9,195,101,96);
    ui->info_frame->hide();
    */

    timerDisable=0;
    timerReady = 0; // Polling per monitariio Ready/not ready

    ui->frameAttesaDati->setGeometry(0,0,800,640);
    ui->frameAttesaDati->hide();
    data_ready=true;
    timer_attesa_dati = 0;

    // Solo il Master effettuerà tale comando
    if(isMaster) connect(this, SIGNAL( queuedExecution(int, int, QString )), this, SLOT(queuedExecutionSlot(int, int,QString )),Qt::QueuedConnection);

}

AnalogPageOpen::~AnalogPageOpen()
{
    delete ui;
}

// Funzione agganciata ai sistemi di menu custom
void AnalogPageOpen::changePage(int pg, int opt)
{
    if(UI_PAGINA==pg)
    {

        // Attivazione pagina
        if(GWindowRoot.curPageVisible== TRUE){
            // Disabilitazione allarmi di sistema
            paginaAllarmi->alarm_enable = true;

            // Disabilita il pulsante di uscita per un certo tempo
            EXIT_BUTTON->hide();
            changePageTimer = startTimer(DISABLE_EXIT_TMO);
            view->show();

            // DBase::_DB_INIT_PAGE
            if(opt & DBase::_DB_INIT_PAGE) initPage();
            else if(isMaster){
                if(pBiopsy->connected) biopsyPanel->manageAfterAlarmActions();
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
        if(opt & DBase::_DB_EXIT_PAGE) exitPage();
    }

}

// Reazione alla pressione del pulsante di uscita
void AnalogPageOpen::onExitButton(void)
{

    //Consentito solo con il pannello comandi aperto
    if((commandPanel->isOpen()) ||(biopsyPanel->isOpen()))   closePageRequest();// ApplicationDatabase.setData(_DB_CLOSE_STUDY_INT,(int) 1,DBase::_DB_FORCE_SGN);
}


//_____________________________________________________________________________________________________________________
// Imposta il range dei kV selezionabili
// Funzione eseguibile solo dal Master
// La funzine imposta il Database: _DB_MIN_MAX_KV;
// Opt rappresenta l'opzione da inviare al Database
//_____________________________________________________________________________________________________________________
void AnalogPageOpen::setKvRange(unsigned char opt){
    if(!isMaster) return;
    if(pGeneratore->pAECprofiles->getCurrentProfilePtr()==null) return;
    int min, max;

    if(pConfig->analogCnf.tech_mode == ANALOG_TECH_MODE_MANUAL){
        // I kV possono essere impostati da 20 a 35/40kV a seconda dei limiti del tubo in oggetto
        min = _MIN_KV * 10;
        max = pGeneratore->getMaxKv() * 10;

    }else if(pConfig->analogCnf.tech_mode == ANALOG_TECH_MODE_SEMI){
        if(pGeneratore->pAECprofiles->getCurrentProfilePtr()->plateType == ANALOG_PLATE_FILM){
            min = pConfig->analogCnf.minKvFilm * 10;
            max = pConfig->analogCnf.maxKvFilm * 10;
        }else{
            min = pConfig->analogCnf.minKvCR * 10;
            max = pConfig->analogCnf.maxKvCR * 10;
        }
    }else{
        // Automatismo: non definito
        min=0;
        max=0;
    }

    ApplicationDatabase.setData(_DB_MIN_MAX_DKV,(int) ((max<<16)|min),opt);

    return;
}

/*
 *  Aggiornamento icona allarmi/info
 */
void AnalogPageOpen::updateAlarmField(void){


}

// Ingresso pagina da cambio Pagina generale
void AnalogPageOpen::initPage(void){

    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
    connect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)), this,SLOT(buttonActivationNotify(int,bool,int)),Qt::UniqueConnection);
    connect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(guiNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
    connect(ui->alarm_butt,SIGNAL(released()),this,SLOT(onAlarmButt()),Qt::UniqueConnection);
    gradi=0;
    int angolo = getArm();
    rotView(angolo);
    thicknessPanel->init();     // Inizializzazione pannello di visualizzazione Spessore seno
    compressionPanel->init();   // Inizializzazione pannello di visualizzazione Forza di compressione
    setPad();                   // Visualizza icona del PAD rilevato
    //updateAlarmField();         // Aggiornamento icona di allarme / info
    commandPanel->xrayPixActivation(false); // Spegnimento pixamap raggi in corso
    biopsyPanel->xrayPixActivation(false); // Spegnimento pixamap raggi in corso
    if(timerDisable==0)  timerDisable=startTimer(500);
    stopAttesaDati();

    if(ApplicationDatabase.getDataU(_DB_NALLARMI_ATTIVI)) ui->alarm_frame->show();
    else ui->alarm_frame->hide();

    // Imposta l'icona di collimazione automatica
    ui->manColliFrame->setStyleSheet("border-image: url(:/Sym/Sym/AutoColliSym.png);background-image: url(:/transparent.png);");


    // Aggiornamento comune a Maste e Slave_______________________________________________________________________________________________________________________
    connect(ui->sblocco_compressore,SIGNAL(released()),this,SLOT(onSblocco_compressore()),Qt::UniqueConnection);
    connect(ui->manualColliButton,SIGNAL(released()),this,SLOT(onManualColliButt()),Qt::UniqueConnection);

    if(!isMaster) return;
    emit queuedExecution(QUEUED_INIT_PAGE,0,"");

}
void AnalogPageOpen::initializeBiopsyPage(void){
    if(!isMaster) return;

    // Impostazione Tipologia di Rotazione
    ApplicationDatabase.setData(_DB_ROT_MODE, (int) pConfig->sys.armMotor, DBase::_DB_FORCE_SGN |DBase::_DB_NO_CHG_SGN);

    // Impostazione del Workflow
    ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_INIT,DBase::_DB_FORCE_SGN |DBase::_DB_NO_CHG_SGN);

    // Impostazione dati per collimazione manuale
    ApplicationDatabase.setData(_DB_ANALOG_MANUAL_COLLI_STAT, (int) _AN_COLLI_AUTO,DBase::_DB_FORCE_SGN |DBase::_DB_NO_CHG_SGN);
    ApplicationDatabase.setData(_DB_ANALOG_CUSTOM_COLLI_LEFT, (int) pCollimatore->customL,DBase::_DB_FORCE_SGN |DBase::_DB_NO_CHG_SGN);
    ApplicationDatabase.setData(_DB_ANALOG_CUSTOM_COLLI_RIGHT, (int) pCollimatore->customR,DBase::_DB_FORCE_SGN |DBase::_DB_NO_CHG_SGN);
    ApplicationDatabase.setData(_DB_ANALOG_CUSTOM_COLLI_BACK, (int) pCollimatore->customB,DBase::_DB_FORCE_SGN |DBase::_DB_NO_CHG_SGN);
    ApplicationDatabase.setData(_DB_ANALOG_CUSTOM_COLLI_FRONT, (int) pCollimatore->customF,DBase::_DB_FORCE_SGN |DBase::_DB_NO_CHG_SGN);

    ApplicationDatabase.setData(_DB_INFO_ALARM_MODE, (int) 0,DBase::_DB_FORCE_SGN |DBase::_DB_NO_CHG_SGN);

    ApplicationDatabase.setData(_DB_COMPRESSOR_UNLOCK, (unsigned char) 0,DBase::_DB_FORCE_SGN); // Compressione bloccata in biopsia
    ApplicationDatabase.setData(_DB_XRAY_SYM, (unsigned char) 0,DBase::_DB_FORCE_SGN);

    // Inizializzazione dei flags di pannello
    int flags = 0;
    if(pConfig->sys.armMotor) flags = _DB_ARM_WITH_MOTOR;
    ApplicationDatabase.setData(_DB_ANALOG_FLAGS,(int) flags ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);

    // Impostazione fuoco corrente
    emit queuedExecution(QUEUED_SELECTED_FUOCO,0,""); // Impostazione Anodo/Fuoco

    // Imposta tecnica manuale
    ApplicationDatabase.setData(_DB_TECH_MODE,(int) ANALOG_TECH_MODE_MANUAL ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);

    // Imposta i range difunzionamento e corregge i valori impostati se necessario
    ApplicationDatabase.setData(_DB_DKV,(int) pConfig->analogCnf.selectedDkV ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
    pGeneratore->setkV(pConfig->analogCnf.selectedDkV/10); // Necessario per poter impostare i limiti sui mAs
    ApplicationDatabase.setData(_DB_DMAS,(int) pConfig->analogCnf.selectedDmas,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
    optionPanel->setExpositionRange(DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);

    // Impostazione Primo filtro come filtro corrente
    ApplicationDatabase.setData(_DB_PRIMO_FILTRO,(int) pConfig->analogCnf.primo_filtro ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
    ApplicationDatabase.setData(_DB_SECONDO_FILTRO,(int) pConfig->analogCnf.secondo_filtro ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
    ApplicationDatabase.setData(_DB_FILTER_MODE,(int) ANALOG_FILTRO_FISSO ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
    ApplicationDatabase.setData(_DB_SELECTED_FILTER,(int) pConfig->analogCnf.primo_filtro ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
    emit queuedExecution(QUEUED_SELECTED_FILTER,0,""); // Impostazione Filtro

    // Imposta il collimatore in automatico e aggiorna la collimazione se necessario
    pCollimatore->manualCollimation = false;
    emit queuedExecution(QUEUED_UPDATE_COLLI,0,""); // Impostazione Collimazione automatica


    // Azzera la diagnostica sul ready/no ready
    ApplicationDatabase.setData(_DB_INFO_ALARM_MODE, (int) 0, DBase::_DB_FORCE_SGN);

    // Inizializza lunghezza Ago
    ApplicationDatabase.setData(_DB_BIOP_AGO, (int) 0, DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);

    // Apertura differita dei pannelli per consentire al database di aggiornarsi correttamente
    ApplicationDatabase.setData(_DB_CHANGE_PANNELLO,(int) PANNELLO_COMANDI,DBase::_DB_FORCE_SGN);

    // Attivazione Timer ready
    if(!timerReady) timerReady = startTimer(1000);

}
void AnalogPageOpen::initializeStandardPage(void){
    if(!isMaster) return;


    // Impostazione Tipologia di Rotazione
    ApplicationDatabase.setData(_DB_ROT_MODE, (int) pConfig->sys.armMotor, DBase::_DB_FORCE_SGN |DBase::_DB_NO_CHG_SGN);


    // Impostazione dati per collimazione manuale
    ApplicationDatabase.setData(_DB_ANALOG_MANUAL_COLLI_STAT, (int) _AN_COLLI_AUTO,DBase::_DB_FORCE_SGN |DBase::_DB_NO_CHG_SGN);
    ApplicationDatabase.setData(_DB_ANALOG_CUSTOM_COLLI_LEFT, (int) pCollimatore->customL,DBase::_DB_FORCE_SGN |DBase::_DB_NO_CHG_SGN);
    ApplicationDatabase.setData(_DB_ANALOG_CUSTOM_COLLI_RIGHT, (int) pCollimatore->customR,DBase::_DB_FORCE_SGN |DBase::_DB_NO_CHG_SGN);
    ApplicationDatabase.setData(_DB_ANALOG_CUSTOM_COLLI_BACK, (int) pCollimatore->customB,DBase::_DB_FORCE_SGN |DBase::_DB_NO_CHG_SGN);
    ApplicationDatabase.setData(_DB_ANALOG_CUSTOM_COLLI_FRONT, (int) pCollimatore->customF,DBase::_DB_FORCE_SGN |DBase::_DB_NO_CHG_SGN);

    ApplicationDatabase.setData(_DB_INFO_ALARM_MODE, (int) 0,DBase::_DB_FORCE_SGN |DBase::_DB_NO_CHG_SGN);

    ApplicationDatabase.setData(_DB_COMPRESSOR_UNLOCK, (unsigned char) pConfig->userCnf.enableSblocco,DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_XRAY_SYM, (unsigned char) 0,DBase::_DB_FORCE_SGN);

    // Inizializzazione dei flags di pannello
    int flags = 0;
    if(pConfig->sys.armMotor) flags = _DB_ARM_WITH_MOTOR;    
    ApplicationDatabase.setData(_DB_ANALOG_FLAGS,(int) flags ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);


    // Impostazione vettori proiezioni (solo Master)
    projectionsAngoli[PROJ_LCC] = pConfig->analogCnf.LCC;
    projectionsAngoli[PROJ_LMLO]= pConfig->analogCnf.LMLO;
    projectionsAngoli[PROJ_LML]= pConfig->analogCnf.LML;
    projectionsAngoli[PROJ_LISO]= pConfig->analogCnf.LISO;
    projectionsAngoli[PROJ_LFB]= pConfig->analogCnf.LFB;
    projectionsAngoli[PROJ_LSIO]= pConfig->analogCnf.LSIO;
    projectionsAngoli[PROJ_LLM]= pConfig->analogCnf.LLM;
    projectionsAngoli[PROJ_LLMO]= pConfig->analogCnf.LLMO;
    projectionsAngoli[PROJ_RCC] = pConfig->analogCnf.RCC;
    projectionsAngoli[PROJ_RMLO]= pConfig->analogCnf.RMLO;
    projectionsAngoli[PROJ_RML]= pConfig->analogCnf.RML;
    projectionsAngoli[PROJ_RISO]= pConfig->analogCnf.RISO;
    projectionsAngoli[PROJ_RFB]= pConfig->analogCnf.RFB;
    projectionsAngoli[PROJ_RSIO]= pConfig->analogCnf.RSIO;
    projectionsAngoli[PROJ_RLM]= pConfig->analogCnf.RLM;
    projectionsAngoli[PROJ_RLMO]= pConfig->analogCnf.RLMO;

    // Inizializza la lateralità e la vista correnti
    currentProjection = PROJ_UNDEF;
    ApplicationDatabase.setData(_DB_SELECTED_PROJECTION, (int) PROJ_UNDEF,DBase::_DB_FORCE_SGN |DBase::_DB_NO_CHG_SGN);
    currentBreast = BREAST_R;
    ApplicationDatabase.setData(_DB_CURRENT_LAT, (int) currentBreast,DBase::_DB_FORCE_SGN |DBase::_DB_NO_CHG_SGN);

    // ____________    Acquisizione del profilo AEC corrente (se presente)
    AEC::profileCnf_Str* profilePtr = pGeneratore->pAECprofiles->getCurrentProfilePtr();    
    emit queuedExecution(QUEUED_SELECTED_FUOCO,0,""); // Impostazione Anodo/Fuoco


    // ____________    Dati del profilo corrente se esiste
    if(profilePtr==null){
        ApplicationDatabase.setData(_DB_NUMERO_PROFILI,(int) 0);
        pConfig->analogCnf.tech_mode=ANALOG_TECH_MODE_MANUAL;

        // Modalità manuale imposta o selezionata
        ApplicationDatabase.setData(_DB_PROFILE_INDEX,(int) -1 ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_PROFILE_NAME,"---" ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_PLATE_TYPE,(int) ANALOG_PLATE_UNDEF ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_TECHNIC, 255 ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_OD,(int) 255 ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_TECH_MODE,(int) ANALOG_TECH_MODE_MANUAL ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);

    } else {
        ApplicationDatabase.setData(_DB_NUMERO_PROFILI,(int) pGeneratore->pAECprofiles->getNumProfiles());
        ApplicationDatabase.setData(_DB_PROFILE_INDEX,(int) pConfig->analogCnf.current_profile ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_PROFILE_NAME,profilePtr->symbolicName ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_PLATE_TYPE,(int) profilePtr->plateType ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        if(profilePtr->plateType == ANALOG_PLATE_FILM){
            ApplicationDatabase.setData(_DB_TECHNIC,(int) profilePtr->technic ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
            ApplicationDatabase.setData(_DB_OD,(int) profilePtr->odindex ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        }else{
            ApplicationDatabase.setData(_DB_TECHNIC,(int) ANALOG_TECH_PROFILE_LD ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
            ApplicationDatabase.setData(_DB_OD,(int) 5 ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        }
    }


    // Impostazione della tecnica corrente
    ApplicationDatabase.setData(_DB_TECH_MODE,(int) pConfig->analogCnf.tech_mode ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
    ApplicationDatabase.setData(_DB_DKV,(int) pConfig->analogCnf.selectedDkV ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
    pGeneratore->setkV(pConfig->analogCnf.selectedDkV/10); // Necessario per poter impostare i limiti sui mAs
    ApplicationDatabase.setData(_DB_DMAS,(int) pConfig->analogCnf.selectedDmas,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);


    // Imposta i range difunzionamento e corregge i valori impostati se necessario
    optionPanel->setExpositionRange(DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);


    // Imposta quale campi dell'esposimetro sono attualmente selezionati
    ApplicationDatabase.setData(_DB_CAMPI,(int) pConfig->analogCnf.aec_field ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
    pPotter->setDetectorField(pConfig->analogCnf.aec_field );

    // Impostazione del filtro corrente:
    // Se la modalità è automatica, il filtro selezionato è sempre il primo.
    // Se la modalità è manuale allora vale l'ultimo filtro selezionato e viene disabilitao il modo filtro automatico
    ApplicationDatabase.setData(_DB_PRIMO_FILTRO,(int) pConfig->analogCnf.primo_filtro ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
    ApplicationDatabase.setData(_DB_SECONDO_FILTRO,(int) pConfig->analogCnf.secondo_filtro ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
    if(pConfig->analogCnf.tech_mode == ANALOG_TECH_MODE_MANUAL) pConfig->analogCnf.auto_filtro_mode = false;

    if(pConfig->analogCnf.auto_filtro_mode){
        ApplicationDatabase.setData(_DB_FILTER_MODE,(int) ANALOG_FILTRO_AUTO ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        pConfig->analogCnf.selected_filtro = pConfig->analogCnf.primo_filtro;
    }else ApplicationDatabase.setData(_DB_FILTER_MODE,(int) ANALOG_FILTRO_FISSO ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
    ApplicationDatabase.setData(_DB_SELECTED_FILTER,(int) pConfig->analogCnf.selected_filtro ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
    emit queuedExecution(QUEUED_SELECTED_FILTER,0,""); // Impostazione Filtro


    // Imposta il collimatore in automatico e aggiorna la collimazione se necessario
    pCollimatore->manualCollimation = false;
    emit queuedExecution(QUEUED_UPDATE_COLLI,0,""); // Impostazione Collimazione automatica

    // Azzera la diagnostica sul ready/no ready
    ApplicationDatabase.setData(_DB_INFO_ALARM_MODE, (int) 0, DBase::_DB_FORCE_SGN);

    // Apertura differita dei pannelli per consentire al database di aggiornarsi correttamente
    ApplicationDatabase.setData(_DB_CHANGE_PANNELLO,(int) PANNELLO_COMANDI, DBase::_DB_FORCE_SGN);

    if(!timerReady) timerReady = startTimer(1000);
    return;
}


// triggera l'evento al database
void AnalogPageOpen::onSblocco_compressore(void){
    if(timerDisable) return;
    timerDisable=startTimer(500);

    // Con la Biopsia connessa non deve sbloccare!
    if(ApplicationDatabase.getDataU(_DB_ACCESSORIO) == BIOPSY_DEVICE) return;

    unsigned char stato = ApplicationDatabase.getDataU(_DB_COMPRESSOR_UNLOCK);
    if(stato) ApplicationDatabase.setData(_DB_COMPRESSOR_UNLOCK, (unsigned char) 0);
    else ApplicationDatabase.setData(_DB_COMPRESSOR_UNLOCK, (unsigned char) 1);

    ui->ServicePanelMenuFrame->setFocus();

}

// Pulsante di apertura pannello di selezione della modalità di collimazione
void AnalogPageOpen::onManualColliButt(void){
    if(timerDisable) return;
    timerDisable=startTimer(500);

    ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_COMANDI_COLLI_SELECTION ,DBase::_DB_FORCE_SGN);    
    ui->ServicePanelMenuFrame->setFocus();

}



void AnalogPageOpen::onAlarmButt(void){
    if(timerDisable) return;
    timerDisable=startTimer(500);
    ui->ServicePanelMenuFrame->setFocus();

    GWindow::setPage(_PG_ALARM,GWindowRoot.curPage,0);


}

void AnalogPageOpen::setSbloccoCompressore(void){
     unsigned char stato = ApplicationDatabase.getDataU(_DB_COMPRESSOR_UNLOCK);

     if(stato){
         ui->sblocco->setStyleSheet("background-image: url(:/paginaOperativaAnalogica/paginaOperativaAnalogica/sblocco.png);");
     }else{
         ui->sblocco->setStyleSheet("background-image: url(:/paginaOperativaAnalogica/paginaOperativaAnalogica/blocco.png);");
     }

     if((isMaster)&&(pConfig->userCnf.enableSblocco != stato)){
         pConfig->userCnf.enableSblocco = stato;
         pConfig->saveUserCfg();
     }

     ui->ServicePanelMenuFrame->setFocus();
}


// FUNZIONE CHIAMATA DALL'HANDLER DEL CAMBIO PAGINA
void AnalogPageOpen::exitPage(void){

   // disconnect(pagina_language,SIGNAL(changeLanguageSgn()), this,SLOT(languageChanged()));
    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
    disconnect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)), this,SLOT(buttonActivationNotify(int,bool,int)));
    disconnect(ui->sblocco_compressore,SIGNAL(released()),this,SLOT(onSblocco_compressore()));
    disconnect(ui->manualColliButton,SIGNAL(released()),this,SLOT(onManualColliButt()));
    disconnect(ui->alarm_butt,SIGNAL(released()),this,SLOT(onAlarmButt()));
    //disconnect(ui->info_butt,SIGNAL(released()),this,SLOT(onAlarmButt()));
    disconnect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(guiNotify(unsigned char,unsigned char,QByteArray)));

    // Visualizzazione pannelli
    kvPanel->exit();
    masPanel->exit();
    projPanel->exit();
    commandPanel->exit();
    optionPanel->exit();
    colliPanel->exit();
    biopsyPanel->exit();

    if(!isMaster) return;
    if(timerReady) killTimer(timerReady);
    timerReady = 0;
    ApplicationDatabase.setData(_DB_STUDY_STAT, (unsigned char) _CLOSED_STUDY_STATUS,DBase::_DB_FORCE_SGN);

    return;
}

void AnalogPageOpen::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId()==changePageTimer)
    {
        killTimer(changePageTimer);
        // Abilita il pulsante di uscita
        EXIT_BUTTON->show();
        return;
    }

    if(ev->timerId()==timerDisable)
    {
        killTimer(timerDisable);
        timerDisable=0;
        return;
    }

    if(isMaster){
        if(ev->timerId()==timerReady)
        {
            if(pBiopsy->connected) verifyBiopsyReady();
            else verifyStandardReady();
            return;
        }

    }

    if(ev->timerId()==timer_attesa_dati)
    {
        killTimer(timer_attesa_dati);
        ui->frameAttesaDati->hide();
        timer_attesa_dati=0;
        return;
    }
}

void AnalogPageOpen::setTempCuffia(int temp)
{
    QString str;
    float hu;
    int cuffiaViewMode=2;



    // Estrazione status cuffia
    int cuffiaStat = (temp >> 8)& 0x00FF;
    temp = temp &0x00FF;


    switch(cuffiaViewMode)
    {
    case 2:
        str = QString("T-SHIELD: %1 ").arg(temp);
        str.append(QString::fromUtf8("Â°"));
    break;
    case 0: // HU%
        hu = (float) 100 * 0.9 * (temp - 25) / 40;
        if(hu<0) hu=0;
        str = QString("T-SHIELD: %1 HU\%").arg(hu);
        break;
    case 1: // HU
        hu = (float) 550 * 0.9 * (temp - 25) / 40;
        if(hu<0) hu=0;
        str = QString("T-SHIELD: %1 kHU").arg(hu);
        str.setNum((int) hu);
        str.append(QString(" kHU"));
        break;
    }

    ui->tempShield->setText(str);

}


void AnalogPageOpen::setHuAnode(int khu)
{
    QString str;
    float hu;
    int cuffiaViewMode=0;

    // Estrazione status cuffia
    int huStat = (khu >> 12)& 0x00FF;
    khu = khu & 0x0FFF;

    switch(cuffiaViewMode)
    {
    case 2:
    case 0: // HU%
        hu = (float) khu * 100 / 300;
        if(hu<0) hu=0;
        str = QString("T-ANODE: %1 HU\%").arg(hu);
        break;
    case 1: // HU
        hu = (float) khu ;
        if(hu<0) hu=0;
        str = QString("T-ANODE: %1 kHU").arg(hu);
        break;
    }
    ui->tempAnode->setText(str);


}

void AnalogPageOpen::valueChanged(int index,int opt)
{
    int angolo;
    int flags;

    if((isMaster)&&(opt&DBase::_DB_ONLY_SLAVE_ACTION)) return;
    if((!isMaster)&&(opt&DBase::_DB_ONLY_MASTER_ACTION)) return;


    switch(index){
    case _DB_DANGOLO:
        angolo = getArm();
        commandPanel->setArm(angolo);
        biopsyPanel->setArm(angolo);
        rotView(angolo);  // Corregge la visualizzazione di tutto il pannello conseguentemente
        if(!isMaster) return;

        // Questa sezione viene eseguita solo nel caso in cui la rotazione sia Motorizzata e non ci sia la biopsia connessa
        if(!pConfig->sys.armMotor) return;
        if(pBiopsy->connected) return;
        if(ApplicationDatabase.getDataI(_DB_SELECTED_PROJECTION)==PROJ_UNDEF) return;
        flags = ApplicationDatabase.getDataI(_DB_ANALOG_FLAGS);

        if(getCurrentProjection(angolo) != ApplicationDatabase.getDataI(_DB_SELECTED_PROJECTION)){
            flags|= _DB_WRONG_PROJECTION;

        }else flags &=~ _DB_WRONG_PROJECTION;

        ApplicationDatabase.setData(_DB_ANALOG_FLAGS,(int) flags);
        break;

    case _DB_T_CUFFIA:
        setTempCuffia(ApplicationDatabase.getDataI(index));
        break;

    case _DB_HU_ANODE:
        setHuAnode(ApplicationDatabase.getDataI(index));
        break;

    case _DB_CHANGE_PANNELLO:

        changePanel(ApplicationDatabase.getDataI(index));
        break;

    case _DB_CALLBACKS:
        // if(!isMaster) return;
        manageCallbacks(ApplicationDatabase.getDataI(index));
        break;

    case _DB_CURRENT_LAT:        
        currentBreast = ApplicationDatabase.getDataI(index);
        if(projPanel->isOpen()) projPanel->setLat(ApplicationDatabase.getDataI(index));
        break;


    case _DB_COMPRESSOR_PAD_CODE:
        setPad();
        break;

    case _DB_ACCESSORIO:

        if(!isMaster) return;
        emit queuedExecution(QUEUED_INIT_PAGE,0,"");

        break;
    case _DB_COMPRESSOR_UNLOCK:
            setSbloccoCompressore();
        break;
    case _DB_ANALOG_MANUAL_COLLI_STAT:
        if(ApplicationDatabase.getDataI(index)==_AN_COLLI_AUTO) {
            ui->manColliFrame->setStyleSheet("border-image: url(:/Sym/Sym/AutoColliSym.png);background-image: url(:/transparent.png);");
        }else{
            ui->manColliFrame->setStyleSheet("border-image: url(:/Sym/Sym/ManualColliSym.png);background-image: url(:/transparent.png);");
        }
        break;
    case _DB_NALLARMI_ATTIVI:
        if(ApplicationDatabase.getDataU(index)==0){
            ui->alarm_frame->hide();
        }else{
            ui->alarm_frame->show();
        }
        break;
    case _DB_INFO_ALARM_MODE:
        setInfoReadyFields(ApplicationDatabase.getDataI(index));
        break;

    case _DB_XRAY_PUSH_BUTTON:
        if(!isMaster) return;

        if(!ApplicationDatabase.getDataU(_DB_XRAY_PUSH_BUTTON)){
            // Rilascio Pulsante raggi
            ApplicationDatabase.setData(_DB_XRAY_SYM,(unsigned char) 0, DBase::_DB_FORCE_SGN);
            io->setXrayLamp(false);
            return;
        }

        // Segnale di attivazione pulsante raggi
        if(!ApplicationDatabase.getDataU(_DB_READY_EXPOSURE)){
            flags = ApplicationDatabase.getDataI(_DB_INFO_ALARM_MODE);

            // Allarmi in corso
            if( flags & 0x400) pAudio->playAudio(AUDIO_NOT_READY_SYSTEM_ALARM);
            else{

                // Tube temp alarm
                if( flags & 0x800) pAudio->playAudio(AUDIO_NOT_READY_TUBE_TEMP_ALARM);

                // Gruppo Invalid Potter
                if( flags & 0x1) pAudio->playAudio(AUDIO_NOT_READY_INVALID_POTTER);
                else  if( flags & 0x2) pAudio->playAudio(AUDIO_NOT_READY_INVALID_COMPRESSOR);
                else  if( flags & 0x4) pAudio->playAudio(AUDIO_NOT_READY_APPLY_COMPRESSION);

                // Gruppo Cassette
                if( flags & 0x10)  pAudio->playAudio(AUDIO_NOT_READY_FOR_EXPOSURE_MISSING_CASSETTE);
                else  if( flags & 0x20)  pAudio->playAudio(AUDIO_NOT_READY_FOR_EXPOSURE_CASSETTE_ALREADY_EXPOSED);

                // Gruppo OpenDoor
                if( flags & 0x100) pAudio->playAudio(AUDIO_NOT_READY_OPEN_DOOR);

                // Detector
                if( flags & 0x200) pAudio->playAudio(AUDIO_NOT_READY_DETECTOR_NOT_CALIBRATED);
            }

            return;
        }
        emit queuedExecution(QUEUED_START_XRAY_SEQ,0,""); // Attivazione procedura raggi
        break;


    case _DB_ANOPER_STOP_ATTESA_DATI:
        data_ready = true;
        ui->frameAttesaDati->hide();
        if(timer_attesa_dati) killTimer(timer_attesa_dati);
        timer_attesa_dati=0;
        break;

    case _DB_XRAY_SYM:
        if(ApplicationDatabase.getDataU(_DB_XRAY_SYM)){
            data_ready = false;
            commandPanel->xrayPixActivation(true);
            biopsyPanel->xrayPixActivation(true);
        }else{
            if(!data_ready) startAttesaDati(5000);
            commandPanel->xrayPixActivation(false);
            biopsyPanel->xrayPixActivation(false);
        }
        break;


    case _DB_STUDY_STAT:// Richiesta di chiusura studio da protocollo console
        if(ApplicationDatabase.getDataU(index)) return;
        // Chiude le info
        PageAlarms::activateNewAlarm(_DB_ALLARME_INFO_STAT,0);
        pConfig->selectMainPage();
        break;



    case _DB_CLOSE_STUDY_INT:       // Richiesta di chiusura studio eseguita da pulsante di uscita
       if(!isMaster) return;

       // Chiude le info
       PageAlarms::activateNewAlarm(_DB_ALLARME_INFO_STAT,0);
       pConfig->selectMainPage();
       break;

    }

}



/*
 *  VERIFICA LE CONDIZIONI PER AUTORIZZARE LA PRESSIONE DEL PULSANTE RAGGI

GRUPPO ICONA INVALID POTTER
not_ready_bits|=1;   // Invalid potter
not_ready_bits|=2;   // Invalid Pad
not_ready_bits|=4;   // Not compressed

GRUPPO ICONA CASSETTA
not_ready_bits|=10;   // Missing Cassette
not_ready_bits|=20;   // Exposed Cassette
not_ready_bits|=40;   // Not Exposed Cassette

GRUPPO ICON DOOR
not_ready_bits|=100;  // Open Door

GRUPPO ICONA DETECTOR CALIBRATION
not_ready_bits|=200;  // Detector not calibrated
not_ready_bits|=400;  // Profilo non valido per la combinazione filtro/fuoco

not_ready_bits|=1000;  // Allarme attivo
not_ready_bits|=2000;  // TEmperatura cuffia

 */

void AnalogPageOpen::verifyStandardReady(void){

    int flags = ApplicationDatabase.getDataI(_DB_ANALOG_FLAGS);
    int not_ready_bits=0;

    // Gruppo Invalid Potter
    if(!pPotter->isValid()){
         not_ready_bits|=1;   // Invalid potter
    }else if(!pCompressore->isValidPad()) {
        not_ready_bits|=2;   // Invalid Pad
    }else if(!pCompressore->isCompressed()) {
        not_ready_bits|=4;   // Not compressed
    }

    // Gruppo cassetta
    if(!pPotter->getCassettePresence()){
            not_ready_bits|=0x10;   // Missing Cassette
    } else if(pPotter->getCassetteExposed()){
            not_ready_bits|=0x20;   // Exposed Cassette
    }

    // Gruppo OpenDoor
    if(ApplicationDatabase.getDataU(_DB_CLOSED_DOOR)==0){ // Verifica che la porta dello studio sia chiusa
        not_ready_bits|=0x100;   // Open Door
    }

    // Gruppo calibrazioni
    if(pConfig->analogCnf.tech_mode!=ANALOG_TECH_MODE_MANUAL){
        if((pConfig->analogCnf.calib_f1 == 0)||(pConfig->analogCnf.calib_f2 == 0)||(pConfig->analogCnf.calib_f3 == 0)) {
            not_ready_bits|=0x200;   // Detector not calibrated
        }else{
            // Verifica della coerenza del profilo secondo filtro e fuoco selezionato
            if(pConfig->analogCnf.auto_filtro_mode) // IN Auto mode entrambi i filtri devono essere calibrati!!
            {
               if(!pGeneratore->pAECprofiles->isCurrentProfileCalibrated(pConfig->analogCnf.primo_filtro, pGeneratore->selectedFSize))  not_ready_bits|=0x400;
               else if(!pGeneratore->pAECprofiles->isCurrentProfileCalibrated(pConfig->analogCnf.secondo_filtro, pGeneratore->selectedFSize))  not_ready_bits|=0x400;
            }else{
                // Si controlla solo il filtro selezionato
                if(!pGeneratore->pAECprofiles->isCurrentProfileCalibrated(pConfig->analogCnf.selected_filtro, pGeneratore->selectedFSize))  not_ready_bits|=0x400;
            }

        }
    }

    // Allarme attivo
    if(ApplicationDatabase.getDataU(_DB_NALLARMI_ATTIVI))  not_ready_bits|=0x1000;

    // Temperatura tubo
    if((pCollimatore->alrCuffia) || (pCollimatore->alrSensCuffia)) not_ready_bits|=0x2000;

    // Aggiorna iconografia
    ApplicationDatabase.setData(_DB_INFO_ALARM_MODE,(int) not_ready_bits);

    // READY
    if(!not_ready_bits){
        flags |=_DB_ANFLG_EXP_READY;
        ApplicationDatabase.setData(_DB_ANALOG_FLAGS, (int) flags);
        if(!ApplicationDatabase.getDataU(_DB_READY_EXPOSURE)) pAudio->playAudio(AUDIO_READY_FOR_EXPOSURE  );
        ApplicationDatabase.setData(_DB_READY_EXPOSURE,(unsigned char) 1);
    }else {
        flags &=~_DB_ANFLG_EXP_READY;
        ApplicationDatabase.setData(_DB_ANALOG_FLAGS, (int) flags); // NOT READY
        ApplicationDatabase.setData(_DB_READY_EXPOSURE,(unsigned char) 0);

    }

}

void AnalogPageOpen::verifyBiopsyReady(void){
    int not_ready_bits=0;
    int flags = ApplicationDatabase.getDataI(_DB_ANALOG_FLAGS);

    // Allarme attivo
    if(ApplicationDatabase.getDataU(_DB_NALLARMI_ATTIVI))  not_ready_bits|=0x1000;

    // Temperatura tubo
    if((pCollimatore->alrCuffia) || (pCollimatore->alrSensCuffia)) not_ready_bits|=0x2000;

    // Gruppo OpenDoor
    if(ApplicationDatabase.getDataU(_DB_CLOSED_DOOR)==0){ // Verifica che la porta dello studio sia chiusa
        not_ready_bits|=0x100;   // Open Door
    }

    // Gruppo Compressione sempre
    if(!pCompressore->isValidPad()) {
        not_ready_bits|=2;   // Invalid Pad
    }else if(!pCompressore->isCompressed()) {
        not_ready_bits|=4;   // Not compressed
    }

    // NOT READY SEMPRE
    if((biopsyPanel->workflow != _BIOPSY_SHOT_RIGHT) && (biopsyPanel->workflow != _BIOPSY_SHOT_LEFT)&& (biopsyPanel->workflow != _BIOPSY_CHECK_POSITION)){
        flags &=~_DB_ANFLG_EXP_READY;
        ApplicationDatabase.setData(_DB_ANALOG_FLAGS, (int) flags); // NOT READY
        ApplicationDatabase.setData(_DB_READY_EXPOSURE,(unsigned char) 0);
        ApplicationDatabase.setData(_DB_INFO_ALARM_MODE,(int) not_ready_bits);// Aggiorna iconografia
        return;

    }


    ApplicationDatabase.setData(_DB_INFO_ALARM_MODE,(int) not_ready_bits);

    // READY
    if(!not_ready_bits){
        flags |=_DB_ANFLG_EXP_READY;
        ApplicationDatabase.setData(_DB_ANALOG_FLAGS, (int) flags);
        // if(!ApplicationDatabase.getDataU(_DB_READY_EXPOSURE)) pAudio->playAudio(AUDIO_READY_FOR_EXPOSURE  );
        ApplicationDatabase.setData(_DB_READY_EXPOSURE,(unsigned char) 1);
    }else {
        flags &=~_DB_ANFLG_EXP_READY;
        ApplicationDatabase.setData(_DB_ANALOG_FLAGS, (int) flags); // NOT READY
        ApplicationDatabase.setData(_DB_READY_EXPOSURE,(unsigned char) 0);

    }

}

void AnalogPageOpen::setPad(void){

    if(isMaster) ui->unlock_pad->setGeometry(35,205,101,76);
    else ui->unlock_pad->setGeometry(664,205,101,76);
    if(isMaster) ui->unlock_compressor->setGeometry(35,205,101,76);
    else ui->unlock_compressor->setGeometry(664,205,101,76);

    ui->unlock_pad->hide();
    ui->unlock_compressor->hide();
    ui->pad->hide();

    unsigned char padcode = ApplicationDatabase.getDataU(_DB_COMPRESSOR_PAD_CODE);
    if(padcode < PAD_ENUM_SIZE){
        ui->pad->show();
        ui->padname->setText(ApplicationDatabase.getDataS(_DB_COMPRESSOR_PAD));
    }else if(padcode==PAD_UNLOCKED) ui->unlock_compressor->show();
    else if(padcode==PAD_UNMOUNTED) ui->unlock_pad->show();
}

void AnalogPageOpen::setCurrentFuoco(void){
    if(!isMaster) return;
    unsigned char accessorio = ApplicationDatabase.getDataU(_DB_ACCESSORIO);

    // Condizini in operativo: il fuoco è schiavo dell'accessorio
    if(accessorio == POTTER_MAGNIFIER) pGeneratore->setFuocoPiccolo();
    else if((accessorio == POTTER_2D)||(accessorio == POTTER_TOMO)) pGeneratore->setFuocoGrande();
    pGeneratore->updateFuoco(); // Questo invia al driver il comando di attivazione

    ApplicationDatabase.setData(_DB_ANODE_NAME,pGeneratore->selectedAnodo);
    ApplicationDatabase.setData(_DB_CURRENT_FUOCO,(int) pGeneratore->selectedFSize);
}

// RESTITUISCE UN ANGOLO DEFINITO IN GRADI INTERI
int AnalogPageOpen::getArm(void){


    // Angolo in decimi di grado
    int angolo = ApplicationDatabase.getDataI(_DB_DANGOLO);
    int intero = angolo/10;
    int decimale = angolo-intero*10;

    if(decimale>5) return intero+1;
    else return intero;

}

void AnalogPageOpen::rotView(int ang){
    int angolo;

    if((ang>-10) && (ang<10)) angolo = 0;
    else if((ang>-50) && (ang<-40)) angolo = -45;
    else if((ang>-95) && (ang<-85)) angolo = -90;
    else if((ang>40) && (ang<50)) angolo = 45;
    else if((ang>85) && (ang<95)) angolo = 90;
    else if((ang>-140) && (ang<-130)) angolo = -135;
    else if((ang>130) && (ang<140)) angolo = 135;
    else if((ang>175) && (ang<185)) angolo = 180;
    else if((ang>-185) && (ang<-175)) angolo = -180;
    else angolo = 15 * (ang / 15) ;


    int new_angolo = currentAngolo - angolo;

    // Impostazione angolo grafica
    ui->comandi->rotate(new_angolo);
    ui->spessore->rotate(new_angolo);
    ui->forza->rotate(new_angolo);
    ui->kv->rotate(new_angolo);
    ui->proiezioni->rotate(new_angolo);
    ui->mAs->rotate(new_angolo);
    ui->options->rotate(new_angolo);
    ui->manualColliPanel->rotate(new_angolo);
    ui->biopsia->rotate(new_angolo);

    currentAngolo=angolo;
    ui->comandi->update();

}

/*_________________________________________________________________________________________
 *  Apre un nuovo pannello comandi: ricevuto sia da Master che da slave
 *  Le inizializzazioni possono essere fatte solo dal Master
 ________________________________________________________________________________________*/
void AnalogPageOpen::changePanel(int panel){


    // Visualizzazione pannelli
    kvPanel->exit();
    masPanel->exit();
    projPanel->exit();
    commandPanel->exit();
    optionPanel->exit();
    colliPanel->exit();
    biopsyPanel->exit();

    switch(panel){
    case PANNELLO_PROIEZIONI:
        projPanel->open();
        break;
    case PANNELLO_OPZIONI:
        optionPanel->open();
        break;
    case PANNELLO_COMANDI:
        if(ApplicationDatabase.getDataU(_DB_ACCESSORIO) == BIOPSY_DEVICE) biopsyPanel->open();
        else commandPanel->open();
        break;
    case PANNELLO_KV:
        kvPanel->open();
        break;
    case PANNELLO_MAS:
        masPanel->open();
        break;
    case PANNELLO_COLLI:
        colliPanel->open();
        break;

    }

}

/*_________________________________________________________________________________________
 *  Effettua una ricerca per determinare la vista più vicina all'angolo corrente
 *  Durante la ricerca non viene cambiato il seno corrente e quindi la ricerca viene
 *  effettuata solo tra le viste relative alla lateralità corrente
 *  La funzione restituisce la proiezione più vicina
 ________________________________________________________________________________________*/
int AnalogPageOpen::getCurrentProjection(int angolo){

    int distanza;
    int projection=currentProjection;
    int ofs;


    distanza = abs(projectionsAngoli[projection]-angolo);


    if(currentBreast==BREAST_L) ofs=PROJ_LCC;
    else ofs = PROJ_RCC;

    for(int j=0; j<8;j++){
        if(abs(angolo -  projectionsAngoli[j+ofs]) < 5){
            // Entro un angolo di 5 gradi è di sicuro
            projection = j+ofs;

            break;
        }else if(abs(angolo -  projectionsAngoli[j+ofs]) < (distanza-5)){
            // Entro un angolo di 5 gradi è di sicuro
            projection = j+ofs;
            distanza = abs(angolo -  projectionsAngoli[j+ofs]);

        }
    }



    return projection;

}


/*_________________________________________________________________________________________
 * Gestione callbacks propagate fino al Master (solo Master esegue!!!!!)
 ________________________________________________________________________________________*/
void AnalogPageOpen::manageCallbacks(int opt){

    switch(opt){

    //______________________________________________________________________________________________________
    // ATTIVAZIONE PANNELLI
    //______________________________________________________________________________________________________
    case CALLBACK_COMANDI_PROJ_SELECTION: // Apre il pannello di selezione proiezioni

        if(ApplicationDatabase.getDataI(_DB_FORZA)){
            if(isMaster) PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_ARM,ARM_DISABLED_ERROR,true);
            return;
        }
        if(ApplicationDatabase.getDataU(_DB_DEAD_MEN)){
            if(isMaster) PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_ARM,ARM_DISABLED_ERROR,true);
            return;
        }
        if(!(ApplicationDatabase.getDataU(_DB_ENABLE_MOVIMENTI) & ACTUATOR_STATUS_ENABLE_ROT_FLAG)){
            if(isMaster) PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_ARM,ARM_DISABLED_ERROR,true);
            return;
        }


        // Questi allarmi riguardano la sicurezza del braccio
        //if(ApplicationDatabase.getDataI(_DB_ALLARMI_ALR_PAD)) return;
        if(ApplicationDatabase.getDataI(_DB_ALLARMI_POWERDOWN)) return;
        if(ApplicationDatabase.getDataI(_DB_ALLARMI_ALR_ARM)) return;
        if(ApplicationDatabase.getDataI(_DB_ALLARMI_ALR_TRX)) return;
        if(ApplicationDatabase.getDataI(_DB_ALLARMI_ALR_LENZE)) return;

        if(!(ApplicationDatabase.getDataI(_DB_ANALOG_FLAGS) & _DB_ARM_WITH_MOTOR) ) return; // Rotazione automatica non abilitata
        changePanel(PANNELLO_PROIEZIONI);
        break;

    case CALLBACK_COMANDI_KV_SELECTION:        
        changePanel(PANNELLO_KV);
        break;

    case CALLBACK_COMANDI_MAS_SELECTION:
        changePanel(PANNELLO_MAS);
        break;
    case CALLBACK_COMANDI_CAMPI_SELECTION:
        break;
    case CALLBACK_COMANDI_COLLI_SELECTION:
        changePanel(PANNELLO_COLLI);
        break;
    case CALLBACK_COMANDI_OPTION_SELECTION:

        changePanel(PANNELLO_OPZIONI);
        break;

        //______________________________________________________________________________________________________
        // DISATTIVAZIONE PANNELLI
        //______________________________________________________________________________________________________
        case CALLBACK_OPTIONEXIT_SELECTION:           
            changePanel(PANNELLO_COMANDI);
            break;
        case CALLBACK_BIOPSIAEXIT_SELECTION:
            changePanel(PANNELLO_COMANDI);
        break;

        case CALLBACK_KVEXIT_SELECTION:
            // Ricalcola i mAs nel range dei kV selezionati
            if(isMaster){
                // Ricalcola i range ora relativi ai nuovi kV (se necessario
                pGeneratore->setkV((float) ApplicationDatabase.getDataI(_DB_DKV)/10);
                pConfig->analogCnf.selectedDkV = ApplicationDatabase.getDataI(_DB_DKV);
                optionPanel->analog_conf_changed = true;
                optionPanel->setExpositionRange(DBase::_DB_NO_CHG_SGN);
            }

            changePanel(PANNELLO_COMANDI);
            break;

        case CALLBACK_COLLIEXIT_SELECTION:
            changePanel(PANNELLO_COMANDI);

            // Imposta la collimazione in uscita dal pannello
            if(isMaster) setCurrentCollimation();


        break;

        case CALLBACK_MASEXIT_SELECTION:
            if(isMaster){                
                pConfig->analogCnf.selectedDmas = ApplicationDatabase.getDataI(_DB_DMAS);
                optionPanel->analog_conf_changed = true;
            }
            changePanel(PANNELLO_COMANDI);
            break;

        case CALLBACK_PROJEXIT_SELECTION:
            changePanel(PANNELLO_COMANDI);
            if(isMaster)   activateProjection();
            break;


    }
}

// Invia il comando di movimento Braccio alla posizione dettata dalla proiezione
void  AnalogPageOpen::activateProjection(void) {


    unsigned char index = ApplicationDatabase.getDataI(_DB_SELECTED_PROJECTION);
    if(index>=PROJ_UNDEF) return;

    unsigned char data[3];

    int angolo =  projectionsAngoli[index];
    int angolo_corrente =  ApplicationDatabase.getDataI(_DB_DANGOLO) / 10;

    // Se c'è la biopsia il massimo angolo attivabile è 90°
    if((pBiopsy->connected) && (abs(angolo)>90)){
        // TO BE DONE: MESSAGGIO DI ERRORE PER ANGOLO FUORI RANGE
        return;
    }

    // Per i movimenti manuali il comando non deve avere effetti ulteriori
    if(!pConfig->sys.armMotor) return;

    // SE si trova in un angolo tra +-2 il target allora non si muove
    if((angolo >= angolo_corrente-2) && (angolo <= angolo_corrente+2)) return;

    // Se si trova in compressione non può muoversi
    if(pCompressore->isCompressed()) return;

    // Impostazione Parametro in gradi
    data[0] =(unsigned char) (angolo&0xFF);
    data[1] =(unsigned char) (angolo>>8);

    // Invio comando
    if(pConsole->pGuiMcc->sendFrame(MCC_CMD_ARM,1,data, 2)==FALSE)
    {
        qDebug() << "CONSOLE <SetArm>: ERRORE COMANDO MCC";
        PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_SOFT,ERROR_MCC,TRUE); // Self resetting
    }

    return ;
}

int  AnalogPageOpen::getMinKv(void) {
    return (ApplicationDatabase.getDataI(_DB_MIN_MAX_DKV)&&0xFFFF);
}

int  AnalogPageOpen::getMaxKv(void) {
    return (ApplicationDatabase.getDataI(_DB_MIN_MAX_DKV)>>16);
}

void  AnalogPageOpen::saveOptions(void) {
    if(!isMaster) return;
    if((optionPanel->analog_conf_changed)||(commandPanel->config_changed))  pConfig->saveAnalogConfig();
    if(optionPanel->profile_conf_changed) pGeneratore->pAECprofiles->saveSelectedProfile();
    optionPanel->analog_conf_changed = false;
    optionPanel->profile_conf_changed = false;
    commandPanel->config_changed = false;


}

void AnalogPageOpen::startAttesaDati(int time){
    timer_attesa_dati = startTimer(time);
    ui->frameAttesaDati->show();
}
void AnalogPageOpen::stopAttesaDati(void){
    if(!isMaster) return;
    ApplicationDatabase.setData(_DB_ANOPER_STOP_ATTESA_DATI,(int)0, DBase::_DB_FORCE_SGN);
}

// Richiesta di apertura pagina operativa
bool AnalogPageOpen::openPageRequest(void){
    if(pConfig->testConfigError(true,true)) return false; // Non apre la pagina operativa con errori di configurazione
    if(ApplicationDatabase.getDataU(_DB_NALLARMI_ATTIVI)) return false; //Nessun allarme ammesso
    ApplicationDatabase.setData(_DB_EXPOSURE_MODE,(unsigned char) _EXPOSURE_MODE_OPERATING_MODE);
    ApplicationDatabase.setData(_DB_STUDY_STAT,(unsigned char) _OPEN_STUDY_ANALOG);
    biopsyPanel->workflow = _BIOPSY_NO_STATUS;; // inizializza lo stato della biopsia
    cumulativeXdose = 0; // Azzera la dose accumulata nelo studio corrente
    pSysLog->log("OPEN STUDY __________________________________________________________________");
    pConfig->selectOperatingPage();
    return true;
}

bool AnalogPageOpen::closePageRequest(void){
    ApplicationDatabase.setData(_DB_CLOSE_STUDY_INT,(int) 1,DBase::_DB_FORCE_SGN);
    pSysLog->log("CLOSE STUDY __________________________________________________________________");
    return true;
}

 void AnalogPageOpen::queuedExecutionSlot(int code, int val, QString str){
     switch(code){
     case QUEUED_SELECTED_FILTER: // Seleziona filtro corrente
         // Impostazione filtro pre impulso
         if(pCollimatore->getFiltroStat() != (Collimatore::_FilterCmd_Enum) pConfig->analogCnf.selected_filtro){
             pCollimatore->setFiltro((Collimatore::_FilterCmd_Enum) pConfig->analogCnf.selected_filtro, true);
         }
         break;
     case QUEUED_SELECTED_FUOCO:
         setCurrentFuoco();
         break;
     case QUEUED_UPDATE_COLLI:
         pCollimatore->updateColli();
         break;
     case QUEUED_START_XRAY_SEQ:
         startXraySequence();
         break;
     case QUEUED_LOG_FLUSH:
         pSysLog->log(str);
         pSysLog->flush();
         break;
     case QUEUED_LOG:
         pSysLog->log(str);
         break;
     case QUEUED_INIT_PAGE:
         // Con la Biopsia il campio pagina viene gestito a parte
         if(pBiopsy->connected) initializeBiopsyPage();
         else initializeStandardPage();
         break;
     }
 }

 /*
  * Questa funzione aggiorna la collimazione corrente
  * in  funzione della modalità attiva
  */
void  AnalogPageOpen::setCurrentCollimation(void){
     bool storeColli ;
     int cindex;
     int modo = ApplicationDatabase.getDataI(_DB_ANALOG_MANUAL_COLLI_STAT);
     switch(modo){
     case _AN_COLLI_AUTO:
         if(pCollimatore->manualCollimation){
             pCollimatore->manualCollimation = false;
             pCollimatore->updateColli();
         }
         break;

     case _AN_COLLI_MAN_24x30:
         pCollimatore->manualCollimation = true;
         cindex=pCollimatore->getColli2DIndex(PAD_24x30);
         pCollimatore->manualR = pCollimatore->colliConf.colli2D[cindex].R;
         pCollimatore->manualL = pCollimatore->colliConf.colli2D[cindex].L;
         pCollimatore->manualF = pCollimatore->colliConf.colli2D[cindex].F;
         pCollimatore->manualB = pCollimatore->colliConf.colli2D[cindex].B;
         pCollimatore->manualColliUpdate();

         break;

     case _AN_COLLI_MAN_18x24:
         pCollimatore->manualCollimation = true;
         cindex=pCollimatore->getColli2DIndex(PAD_18x24);
         pCollimatore->manualR = pCollimatore->colliConf.colli2D[cindex].R;
         pCollimatore->manualL = pCollimatore->colliConf.colli2D[cindex].L;
         pCollimatore->manualF = pCollimatore->colliConf.colli2D[cindex].F;
         pCollimatore->manualB = pCollimatore->colliConf.colli2D[cindex].B;
         pCollimatore->manualColliUpdate();

         break;
     case _AN_COLLI_MAN_BIOPSY:
         pCollimatore->manualCollimation = true;
         cindex=pCollimatore->getColli2DIndex(PAD_BIOP_3D);
         pCollimatore->manualR = pCollimatore->colliConf.colli2D[cindex].R;
         pCollimatore->manualL = pCollimatore->colliConf.colli2D[cindex].L;
         pCollimatore->manualF = pCollimatore->colliConf.colli2D[cindex].F;
         pCollimatore->manualB = pCollimatore->colliConf.colli2D[cindex].B;
         pCollimatore->manualColliUpdate();

         break;
     case _AN_COLLI_MAN_MAGNIFIER:
         pCollimatore->manualCollimation = true;
         cindex=pCollimatore->getColli2DIndex(PAD_D75_MAG);
         pCollimatore->manualR = pCollimatore->colliConf.colli2D[cindex].R;
         pCollimatore->manualL = pCollimatore->colliConf.colli2D[cindex].L;
         pCollimatore->manualF = pCollimatore->colliConf.colli2D[cindex].F;
         pCollimatore->manualB = pCollimatore->colliConf.colli2D[cindex].B;
         pCollimatore->manualColliUpdate();
         break;
     case _AN_COLLI_MAN_CUSTOM:
         pCollimatore->manualCollimation = true;

         if(pCollimatore->customL != ApplicationDatabase.getDataI(_DB_ANALOG_CUSTOM_COLLI_LEFT))         storeColli = true;
         else if(pCollimatore->customR != ApplicationDatabase.getDataI(_DB_ANALOG_CUSTOM_COLLI_RIGHT))   storeColli = true;
         else if(pCollimatore->customB != ApplicationDatabase.getDataI(_DB_ANALOG_CUSTOM_COLLI_BACK))    storeColli = true;
         else if(pCollimatore->customF != ApplicationDatabase.getDataI(_DB_ANALOG_CUSTOM_COLLI_FRONT))   storeColli = true;
         else storeColli = false;

         pCollimatore->manualL = pCollimatore->customL = ApplicationDatabase.getDataI(_DB_ANALOG_CUSTOM_COLLI_LEFT);
         pCollimatore->manualR = pCollimatore->customR = ApplicationDatabase.getDataI(_DB_ANALOG_CUSTOM_COLLI_RIGHT);
         pCollimatore->manualF = pCollimatore->customF = ApplicationDatabase.getDataI(_DB_ANALOG_CUSTOM_COLLI_FRONT);
         pCollimatore->manualB = pCollimatore->customB = ApplicationDatabase.getDataI(_DB_ANALOG_CUSTOM_COLLI_BACK);
         pCollimatore->manualColliUpdate();


         if(storeColli) pCollimatore->storeConfigFile();
         break;
     }
 }

// Aggiorna lo stato delle icone di Ready/Not ready
void  AnalogPageOpen::setInfoReadyFields(int flags){

    ui->frameInvalidPotter->hide();
    ui->frameInvalidPad->hide();
    ui->frameMissingCassette->hide();
    ui->frameExposedCassette->hide();
    ui->frameNotExposedCassette->hide();
    ui->frameOpenDoor->hide();
    ui->frameMissingCompression->hide();
    ui->frameDetNotCalibrated->hide();
    ui->frameProfileNotCalibrated->hide();
    ui->frameTubeTempAlarm->hide();

    // Gruppo Invalid Potter
    if( flags & 0x1) ui->frameInvalidPotter->show();
    else  if( flags & 0x2) ui->frameInvalidPad->show();
    else  if( flags & 0x4) ui->frameMissingCompression->show();

    // Gruppo Cassette
    if( flags & 0x10) ui->frameMissingCassette->show();
    else  if( flags & 0x20) ui->frameExposedCassette->show();
    else  if( flags & 0x40) ui->frameNotExposedCassette->show();

    // Gruppo OpenDoor
    if( flags & 0x100) ui->frameOpenDoor->show();

    // Detector
    if( flags & 0x200) ui->frameDetNotCalibrated->show();
    else if ( flags & 0x400) ui->frameProfileNotCalibrated->show();

    // Tube temp alarm
    if( flags & 0x2000) ui->frameTubeTempAlarm->show();

}
