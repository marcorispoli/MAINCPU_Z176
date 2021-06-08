#include "startuppage.h"
#include "ui_startuppage.h"

#include "application.h"
#include "appinclude.h"
#include "globvar.h"

#define UI_PAGINA _PG_STARTUP
#define EXIT_PAGINA _PG_MAIN
#define EXIT_BUTTON ui->exitButton
#define DISABLE_EXIT_TMO    1000

// DEFINIZIONE DEGLI STATI DI STARTUP
#define _STARTUP_INIT                               1
#define _STARTUP_WAIT_HARDWARE_CONFIGURATION        2
#define _STARTUP_WAIT_DRIVER_CONNECTION             3
#define _STARTUP_WAIT_DRIVER_CONFIGURATION          4
#define _STARTUP_COMPLETED                          5


#define PCB244_CONNECTED        0x1
#define PCB269_CONNECTED        0x2
#define PCB249U1_CONNECTED      0x4
#define PCB249U2_CONNECTED      0x8
#define PCB240_CONNECTED        0x10
#define PCB190_CONNECTED        0x20
#define ARM_CONNECTED           0x40
#define TRX_CONNECTED           0x80
#define LENZE_CONNECTED         0x100
#define DRIVERS_CONNECTED_MASK  0x1FE // PCB244 può non esser presente..
#define DRIVERS_CONNECTED       0x200
#define DRIVERS_CONFIGURED      0x400
#define TERMINALS_CONNECTED     0x800
#define CAN_CONNECTED           0x1000

static QGraphicsPixmapItem* TcPix;
static QGraphicsPixmapItem* pcb269Pix;
static QGraphicsPixmapItem* pcb249Pix;
static QGraphicsPixmapItem* pcb190Pix;
static QGraphicsPixmapItem* pcb240Pix;
static QGraphicsPixmapItem* trxPix;
static QGraphicsPixmapItem* armPix;
static QGraphicsPixmapItem* lenzePix;
static QGraphicsPixmapItem* configPix;
static int timerUpdate=0;

StartupPage::StartupPage(int rotview, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StartupPage)
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

    TcPix = scene->addPixmap(QPixmap("://StartupPage/StartupPage/startup_tc.png"));
    TcPix->setPos(49,179);
    TcPix->hide();
    pcb269Pix = scene->addPixmap(QPixmap("://StartupPage/StartupPage/startup_pcb269.png"));
    pcb269Pix->setPos(276,128);
    pcb269Pix->hide();
    pcb249Pix = scene->addPixmap(QPixmap("://StartupPage/StartupPage/startup_collimatore.png"));
    pcb249Pix->setPos(269,195);
    pcb249Pix->hide();
    pcb190Pix = scene->addPixmap(QPixmap("://StartupPage/StartupPage/startup_pcb190.png"));
    pcb190Pix->setPos(275,274);
    pcb190Pix->hide();
    pcb240Pix = scene->addPixmap(QPixmap("://StartupPage/StartupPage/startup_pcb240.png"));
    pcb240Pix->setPos(367,115);
    pcb240Pix->hide();
    trxPix = scene->addPixmap(QPixmap("://StartupPage/StartupPage/startup_trx.png"));
    trxPix->setPos(390,170);
    trxPix->hide();
    armPix = scene->addPixmap(QPixmap("://StartupPage/StartupPage/startup_arm.png"));
    armPix->setPos(390,226);
    armPix->hide();
    lenzePix = scene->addPixmap(QPixmap("://StartupPage/StartupPage/startup_lenze.png"));
    lenzePix->setPos(399,281);
    lenzePix->hide();
    configPix = scene->addPixmap(QPixmap("://StartupPage/StartupPage/startup_config.png"));
    configPix->setPos(557,170);
    configPix->hide();

    hardwareConfigured = false;
    deviceConnected = false;
    deviceConfigured = false;

    // Avvisa lo slave che si trova in fase di startup:
    // Lo Slave pu cosi effettuare tutte le inizializzazioni del caso
    if(isMaster) syncToSlave=startTimer(1000);

    connect(&GWindowRoot,SIGNAL(changePage(int,int)), this,SLOT(changePage(int,int)),Qt::UniqueConnection);
    connect(EXIT_BUTTON,SIGNAL(released()),this,SLOT(onExitButton()),Qt::UniqueConnection);

}

StartupPage::~StartupPage()
{
    delete ui;
}

void StartupPage::initWindow(void){

   connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
   ApplicationDatabase.setData(_DB_CONNETCTED_DRIVER,(int) 0,DBase::_DB_NO_ECHO | DBase::_DB_FORCE_SGN);

   if(isMaster){

       // Aggancia handler per gestione notifiche
       connect(pConsole,SIGNAL(mccConfigNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(startupNotifySlot(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

       // Imposta la fase corrente
       ApplicationDatabase.setData(_DB_STARTUP_FASE, ((unsigned char) _STARTUP_INIT));
       if(timerUpdate==0) timerUpdate =startTimer(1000); // Update system status and sequencer

    }

}


// Funzione agganciata ai sistemi di menu custom
void StartupPage::changePage(int pg, int opt)
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

            initWindow();
         }
        else view->hide();
        return;
    }
    else if(GWindowRoot.curPage==UI_PAGINA)
    {
        // Disattivazione pagina
        paginaAllarmi->alarm_enable = true;
        view->hide();

        if(isMaster){
            if(timerUpdate) killTimer(timerUpdate);
            timerUpdate=0;

            if(syncToSlave) killTimer(syncToSlave);
            syncToSlave=0;

            disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
            disconnect(pConsole,SIGNAL(mccConfigNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(startupNotifySlot(unsigned char,unsigned char,QByteArray)));

        }
    }

}

// Reazione alla pressione del pulsante di uscita
// Finisce lo startup senza successo
void StartupPage::onExitButton(void)
{
     GWindowRoot.setNewPage(_PG_MAIN_ANALOG,GWindowRoot.curPage,0);


}


void StartupPage::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId()==changePageTimer)
    {
        killTimer(changePageTimer);
        // Abilita il pulsante di uscita
        EXIT_BUTTON->show();
    }

    if(!isMaster) return;

    // Effettua un polling verso lo slave per le inizializzazioni
    if(ev->timerId()==syncToSlave)
    {
        pConfig->syncToSlave();
        return;
    }

    if(ev->timerId()==timerUpdate)
    {
        int connStatus = ApplicationDatabase.getDataI(_DB_CONNETCTED_DRIVER);
        unsigned char startupFase = ApplicationDatabase.getDataU(_DB_STARTUP_FASE);


        switch(startupFase){
        case _STARTUP_INIT:
#ifdef __NO_SLAVE_STUB
            pConfig->slaveDataInitialized=true;
#endif
            // Attende che lo slave sia stato inizializzato
            if(!pConfig->slaveDataInitialized) break;

            // Se il file di configurazione di sistema non c'è
            // viene aperta la pagina di configurazione
            if(!pConfig->sysConfigured){
                GWindowRoot.setNewPage(_PG_SYSTEM_SETUP,GWindowRoot.curPage,DBase::_DB_INIT_PAGE);
                return;
            }


            connStatus |= TERMINALS_CONNECTED;
            ApplicationDatabase.setData(_DB_SERVICE1_STR, QString(""),DBase::_DB_FORCE_SGN);
            ApplicationDatabase.setData(_DB_CONNETCTED_DRIVER, connStatus);
            ApplicationDatabase.setData(_DB_PACKAGE_ID,pConfig->swConf.rvPackage);

            // Qui inizia la sequenza di startup della macchina
            ApplicationDatabase.setData(_DB_STARTUP_FASE,(unsigned char)  _STARTUP_WAIT_HARDWARE_CONFIGURATION);
            break;

        case _STARTUP_WAIT_HARDWARE_CONFIGURATION:
            if(hardwareConfigured) ApplicationDatabase.setData(_DB_STARTUP_FASE, (unsigned char) _STARTUP_WAIT_DRIVER_CONNECTION);
            else {
                // Invio configurazione Hardware
                pConfig->updateSystemCfg();
            }

            break;

        case _STARTUP_WAIT_DRIVER_CONNECTION:            
            // Inizia ad inviare i GET STATUS fino ad ottenere la completa connessione dei dispositivi
            if(deviceConnected){

                // Check revisioni: finisce lo startup se il package non è conforme
                if(pConfig->checkPackage()== false) {
                    pConfig->startupCompleted = false;
                    ApplicationDatabase.setData(_DB_STARTUP_FASE, 0);

                    GWindowRoot.setNewPage(_PG_MAIN_ANALOG,GWindowRoot.curPage,0);
                    PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_SOFT,ERROR_REVISIONS);
                    return;
                }

                // Se le revisioni sono corrette allora prosegue con l'aggiornamento della configurazione
                // Inizia il processo di download della configurazione
                pConfig->updateAllDrivers();
                ApplicationDatabase.setData(_DB_STARTUP_FASE, (unsigned char) _STARTUP_WAIT_DRIVER_CONFIGURATION);

            }else{
                // Durante l'attesa richiede lo stato corrente delle connessioni
                pConfig->sendMccConfigCommand(CONFIG_STATUS);
            }
            break;

        case _STARTUP_WAIT_DRIVER_CONFIGURATION:

            // Attende che il processo di download delle configurazioni sia completato
            if(deviceConfigured){
                ApplicationDatabase.setData(_DB_STARTUP_FASE, (unsigned char) _STARTUP_COMPLETED);
            }
            break;

        case _STARTUP_COMPLETED:

            // Flag di Startup avvenuto con successo
            pConfig->startupCompleted = true; // Attenzione, lasciarlo dopo onExiButton() che di suo scrive false
            ApplicationDatabase.setData(_DB_STARTUP_FASE, 0);            
            // Aggiorna RTC per i terminali
            pConfig->updateDate();
            GWindowRoot.setNewPage(_PG_MAIN_ANALOG,GWindowRoot.curPage,0);

            // Impostazioni generali al completamento dello startup
            pCollimatore->filtroCmd = Collimatore::FILTRO_Rh;
            pCollimatore->setFiltro();

            break;
        }
    }

}


// FUNZIONE DI AGGIORNAMENTO CAMPI VALORE CONNESSO AI CAMPI DEL DATABASE
void StartupPage::valueChanged(int index,int opt)
{
    unsigned char startupFase;
    int connStatus;

    if(opt&DBase::_DB_NO_ACTION) return;

    switch(index)
    {

    case _DB_CONNETCTED_DRIVER:
        connStatus = ApplicationDatabase.getDataI(_DB_CONNETCTED_DRIVER);
        if(connStatus==0) ApplicationDatabase.setData(_DB_SERVICE1_STR, QString("WARNING: Missing Master Slave Ethernet connection!"),DBase::_DB_NO_ECHO);

        if(connStatus & TERMINALS_CONNECTED) TcPix->show();
        if(connStatus & PCB269_CONNECTED) pcb269Pix->show();
        if(connStatus & PCB240_CONNECTED) pcb240Pix->show();
        if(connStatus & PCB190_CONNECTED) pcb190Pix->show();
        if((connStatus & PCB249U1_CONNECTED)&&(connStatus & PCB249U2_CONNECTED)) pcb249Pix->show();
        if(connStatus & ARM_CONNECTED) armPix->show();
        if(connStatus & TRX_CONNECTED) trxPix->show();
        if(connStatus & LENZE_CONNECTED) lenzePix->show();
        break;

    case _DB_STARTUP_FASE:
        startupFase = ApplicationDatabase.getDataU(_DB_STARTUP_FASE);


        if(startupFase == _STARTUP_INIT ){
            ui->PackageIdLabel->setText("--");
            TcPix->hide();
            pcb249Pix->hide();
            pcb190Pix->hide();
            pcb240Pix->hide();
            trxPix->hide();
            armPix->hide();
            lenzePix->hide();
            configPix->hide();
        }else if(startupFase == _STARTUP_WAIT_DRIVER_CONFIGURATION) configPix->show();

        break;

    case _DB_PACKAGE_ID:
        ui->PackageIdLabel->setText(ApplicationDatabase.getDataS(_DB_PACKAGE_ID));
        break;
    case _DB_SERVICE1_STR: // Error label
        ui->startupErrorLabel->setText(ApplicationDatabase.getDataS(index));
        break;

    case _DB_REQ_POWEROFF:
        if(!isMaster) return;
        if(ApplicationDatabase.getDataU(index)) pConfig->activatePowerOff();
        break;
    }

}

// Handler di gestione dei messaggi provenienti dal GUI drivers
void StartupPage::startupNotifySlot(unsigned char id, unsigned char mcccode, QByteArray buffer)
{
    static int connStatusBack=0;
    static unsigned char errors_back=0;
    int connStatus;
    unsigned char errors;

    switch(mcccode){

    case CONFIG_STATUS:
        connStatus = ApplicationDatabase.getDataI(_DB_CONNETCTED_DRIVER);
        if(buffer[0]) deviceConnected=true;
        if(deviceConnected){
            pConfig->rvGuiSlave=QString("%1").arg(ApplicationDatabase.getDataS(_DB_SLAVE_GUI_REVISION));
            pConfig->rvM4Master=QString("%1.%2").arg((int)buffer[1]).arg((int)buffer[2]);
            pConfig->rv269=QString("%1.%2").arg((int)buffer[3]).arg((int)buffer[4]);
            pConfig->rv249U1=QString("%1.%2").arg((int)buffer[5]).arg((int)buffer[6]);
            pConfig->rv249U2=QString("%1.%2").arg((int)buffer[7]).arg((int)buffer[8]);
            pCollimatore->colli_model = buffer[24];
            pConfig->rv190=QString("%1.%2").arg((int)buffer[9]).arg((int)buffer[10]);

            // Il buffer usato è in comune dato che o c'è una scheda o c'è l'altra
            pConfig->rv244=QString("%1.%2").arg((int)buffer[11]).arg((int)buffer[12]);
            pConfig->rv244A=QString("%1.%2").arg((int)buffer[11]).arg((int)buffer[12]);

            pConfig->rvM4Slave=QString("%1.%2").arg((int)buffer[13]).arg((int)buffer[14]);
            pConfig->rv240=QString("%1.%2").arg((int)buffer[15]).arg((int)buffer[16]);
        }
        if((buffer[3]!=0)||(buffer[4]!=0)) connStatus|=PCB269_CONNECTED;    // Compressore connesso
        if((buffer[5]!=0)||(buffer[6]!=0)) connStatus|=PCB249U1_CONNECTED;  // Collimatore U1 connected
        if((buffer[7]!=0)||(buffer[8]!=0)) connStatus|=PCB249U2_CONNECTED;  // Collimatore U2 connected
        if((buffer[9]!=0)||(buffer[10]!=0)) connStatus|=PCB190_CONNECTED;   // Collimatore U2 connected
        if(buffer[19]!=0) connStatus|=LENZE_CONNECTED;                      // Lenze connected
        if(buffer[17]) connStatus|=ARM_CONNECTED;                        // Arm connected
        if(buffer[18]) connStatus|=TRX_CONNECTED;                        // Trx connected
        if(buffer[20]) connStatus|=PCB240_CONNECTED;  // Collimatore U2 connected
        if(buffer[22]) connStatus|=CAN_CONNECTED;
        errors=buffer[23];

        // RTC
        pConfig->rtc_present = buffer[25];
        pConfig->weekday = buffer[26];
        pConfig->year = buffer[27]+buffer[28]*256;
        pConfig->month = buffer[29];
        pConfig->day = buffer[30];
        pConfig->hour = buffer[31];
        pConfig->min = buffer[32];
        pConfig->sec = buffer[33];


        ApplicationDatabase.setData(_DB_CONNETCTED_DRIVER,connStatus);

        // Test di verifica possibili anomalie sulla base della sequenza di attivazione in corso
        if((connStatusBack!=connStatus)||(errors_back!=errors)){
            connStatusBack=connStatus;
            errors_back=errors;

            if((connStatus & CAN_CONNECTED)==0){
                ApplicationDatabase.setData(_DB_SERVICE1_STR, QString("CAN BUS ERROR!\nCheck the cable integrity, the Termination resistors and the Master and Slave can connectors!!"),DBase::_DB_FORCE_SGN);

            }else if((connStatus & (PCB269_CONNECTED|PCB249U1_CONNECTED|PCB249U2_CONNECTED|PCB190_CONNECTED))== 0){
                ApplicationDatabase.setData(_DB_SERVICE1_STR, QString("WARNING: device Bus disconnected from the PCB240 board!"),DBase::_DB_FORCE_SGN);

            }else if(((connStatus & PCB190_CONNECTED) == 0) && (errors&0x1)){
                ApplicationDatabase.setData(_DB_SERVICE1_STR, QString("WARNING: Emergency Push Button may be activated!!"),DBase::_DB_FORCE_SGN);

            }else if((connStatus&(PCB190_CONNECTED|LENZE_CONNECTED|PCB269_CONNECTED | PCB249U1_CONNECTED | PCB249U2_CONNECTED)) == (LENZE_CONNECTED|PCB269_CONNECTED | PCB249U1_CONNECTED | PCB249U2_CONNECTED) ){
                ApplicationDatabase.setData(_DB_SERVICE1_STR, QString("WARNING: The PCB190 board is not working!!"),DBase::_DB_FORCE_SGN);

            }else if((connStatus&(PCB244_CONNECTED|PCB190_CONNECTED|LENZE_CONNECTED|PCB269_CONNECTED | PCB249U1_CONNECTED | PCB249U2_CONNECTED)) == (PCB244_CONNECTED|PCB190_CONNECTED|LENZE_CONNECTED| PCB249U1_CONNECTED | PCB249U2_CONNECTED) ){
                ApplicationDatabase.setData(_DB_SERVICE1_STR, QString("WARNING: The PCB269 board is not working!!"),DBase::_DB_FORCE_SGN);

            }else if((connStatus&(PCB190_CONNECTED|LENZE_CONNECTED|PCB269_CONNECTED | PCB249U1_CONNECTED | PCB249U2_CONNECTED)) == (PCB190_CONNECTED|LENZE_CONNECTED| PCB249U1_CONNECTED | PCB249U2_CONNECTED) ){
                ApplicationDatabase.setData(_DB_SERVICE1_STR, QString("WARNING: The PCB269 board is not working\nor the Device Bus cable link from PCB249 to PCB269 could be disconnected!!"),DBase::_DB_FORCE_SGN);

            }else if((connStatus&(PCB190_CONNECTED|LENZE_CONNECTED|PCB269_CONNECTED | PCB249U1_CONNECTED | PCB249U2_CONNECTED)) == (PCB190_CONNECTED|LENZE_CONNECTED|PCB269_CONNECTED)){
                ApplicationDatabase.setData(_DB_SERVICE1_STR, QString("WARNING: The PCB249 doesn't work!!"),DBase::_DB_FORCE_SGN);

            }else if((connStatus&(PCB190_CONNECTED|LENZE_CONNECTED|PCB269_CONNECTED | PCB249U1_CONNECTED | PCB249U2_CONNECTED)) == (PCB190_CONNECTED|PCB269_CONNECTED|LENZE_CONNECTED| PCB249U2_CONNECTED) ){
                ApplicationDatabase.setData(_DB_SERVICE1_STR, QString("WARNING: The PCB249U1 doesn't communicate!!"),DBase::_DB_FORCE_SGN);

            }else if((connStatus&(PCB190_CONNECTED|LENZE_CONNECTED|PCB269_CONNECTED | PCB249U1_CONNECTED | PCB249U2_CONNECTED)) == (PCB190_CONNECTED|LENZE_CONNECTED|PCB269_CONNECTED | PCB249U1_CONNECTED) ){
                ApplicationDatabase.setData(_DB_SERVICE1_STR, QString("WARNING: The PCB249U2 doesn't communicate!!"),DBase::_DB_FORCE_SGN);

            }else if((connStatus&(PCB190_CONNECTED|LENZE_CONNECTED|PCB269_CONNECTED | PCB249U1_CONNECTED | PCB249U2_CONNECTED)) == (PCB190_CONNECTED|LENZE_CONNECTED)){
                ApplicationDatabase.setData(_DB_SERVICE1_STR, QString("WARNING: The Device Bus cable link from PCB190 to PCB249 could be disconnected\nor it could be damaged. Check the cable integrity!!"),DBase::_DB_FORCE_SGN);

            }else{
                ApplicationDatabase.setData(_DB_SERVICE1_STR, QString(""),DBase::_DB_FORCE_SGN);
            }
        }
        break;

        case CONFIG_GANTRY:
            // Feedback di ricezione corretta della configurazione Hardware
            hardwareConfigured = true;

            // Riceve anche l'info sullo stato del can bus
            if(buffer[0]) connStatus|=CAN_CONNECTED;


        break;

        case CONFIG_COMPLETED:
            // Configurazione dei dispositivi completata
            deviceConfigured=true;
        break;
    }


}
