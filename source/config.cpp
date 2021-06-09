#define _CONFIG_CPP
#include "application.h"
#include "appinclude.h"
#include "globvar.h"
#include "systemlog.h"
extern systemLog* pSysLog;

#include "ANALOG/Calibration/pageCalibAnalogic.h"
extern AnalogCalibPageOpen* paginaCalibAnalogic;

#define MAX_BLOCCO_FTP 1000
#define TRX_FILE_CFG    "/resource/config/trxCfg.cnf"
#define TOMO_FILE_CFG   "/resource/config/tomoCfg_%1.cnf"
#define ARM_FILE_CFG    "/resource/config/armCfg.cnf"
#define LENZE_FILE_CFG  "/resource/config/lenzeCfg.cnf"
#define USERCFG         "/resource/config/user.cnf"
#define SYSCFG          "/resource/config/sysCfg.cnf"

// FTP_ERR codici di errore
#define ERR_CREATING_FTP_DESTINATION_FILE   1
#define ERR_FTP_NOT_ENABLED                 2
#define ERR_FTP_WRONG_CRC                   3



Config::Config(bool master, QObject *parent) :
    QObject(parent)
{
    systemInitialized = false;
    slaveDataInitialized = false;
    if(master) configMaster();
    else configSlave();
}

// Attiva tutte le connessioni del processo
void Config::activateMasterConnections(){

    // Socket per segnali asincroni
    configMasterTcp = new TcpIpClient();
    configMasterTcp->Start(QHostAddress(_SLAVE_IP),_CONFIG_SLAVE_IN_PORT);

    QObject::connect(this,SIGNAL(configMasterTx(QByteArray)), configMasterTcp,SLOT(txData(QByteArray)),Qt::QueuedConnection);
    QObject::connect(configMasterTcp,SIGNAL(rxData(QByteArray)),this,SLOT(configMasterRxHandler(QByteArray)),Qt::QueuedConnection);

    // Aggancia handler per gestione notifiche
    connect(pConsole,SIGNAL(mccConfigNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(configNotifySlot(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

}

void Config::activateSlaveConnections(){

    // Client per comunicazione a basso livello con la stazione AWS
    awsTcp = new TcpIpClient();
    QObject::connect(awsTcp,SIGNAL(clientConnection(bool)),this,SLOT(awsConnectionHandler(bool)),Qt::UniqueConnection);
    QObject::connect(this,SIGNAL(awsTxHandler(QByteArray)), awsTcp,SLOT(txData(QByteArray)),Qt::UniqueConnection);
    awsTcp->Start(QHostAddress(_CONSOLE_IP),_AWS_OUT_PORT);



    // Apertura server di comunicazione con il configuratore Master
    // secondo il protocollo della console (testuale)
    configSlaveSocketTcp = new TcpIpServer();
    if(configSlaveSocketTcp->Start(_CONFIG_SLAVE_IN_PORT)<0) exit(-1);

    QObject::connect(configSlaveSocketTcp,SIGNAL(rxData(QByteArray)),this,SLOT(configSlaveRxHandler(QByteArray)),Qt::QueuedConnection);
    QObject::connect(this,SIGNAL(configSlaveTx(QByteArray)), configSlaveSocketTcp,SLOT(txData(QByteArray)),Qt::QueuedConnection);

    // Apertura canali in trasmissione e ricezione per M4 SLAVE
    pSlaveMcc = new mccCom(_DEF_APP_SLAVE_TO_M4_SLAVE,FALSE);  // Comunicazione Comandi  Per M4 SLAVE
    pSlaveRxMcc = new mccSlaveCom(_DEF_M4_SLAVE_TO_APP_SLAVE,TRUE);// Comunicazione Comandi  Da M4 MASTER

    // Notifiche da M4 SLAVE
    connect(this,SIGNAL(mccSlaveNotifySgn(unsigned char,unsigned char,QByteArray)),this,SLOT(slaveNotifySlot(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

}

// Inizializzazione configuratore Master
void Config::configMaster(void)
{
     sysConfigured=false;             // Architettura di sistema correttamente configurata
     userConfigured=false;            // File di configurazione user correttamente aperto
     packageConfigured=false;          // File package correttamente caricato
     SN_Configured=false;             // Serial Number configurato

     startupCompleted=false ;         // Fase di startup completata
     generator_configured=false;      // Il Generatore correttamente configurato
     compressor_configured=false;     // Compressore configurato
     collimator_configured=false;     // Configurazione collimatore acquisita

     analog_configured = false;           // File di configurazione analogico
     aec_configured=false;            // AEC configurato
     kerma_mo_configured=false;       // Kerma data per Mo configurato;
     kerma_rh_configured=false;       // Kerma data per Rh configurato;
     kerma_cg_configured=false;       // Parametri CG configurati

    this->isMaster = TRUE;
    rvGuiMaster = QString("%1.%2").arg(APPREVMAJ).arg(APPREVMIN);

    // Apre il file di configurazione di systema
    sysConfigured = openSysCfg();
    userConfigured = openUserCfg();

    // Apertura file firmware package
    packageConfigured = openPackageCfg(FIRMWARE_FILE, &this->swConf);

    rvGuiSlave.clear();
    rvM4Master.clear();
    rvM4Slave.clear();
    rv269.clear();
    rv240.clear();
    rv249U1.clear();
    rv249U2.clear();
    rv190.clear();
    rv244.clear();
    rv244A.clear();
    revisionOK = FALSE;


    // Caricamento files di configurazione per dispositivi su CAN
    readTrxConfig();
    readArmConfig();
    readLenzeConfig();
    readLogo();
    analog_configured = openAnalogConfig();

    // Reset timers
    timerInstall = 0;
    singleConfigUpdate = FALSE;

    startupCompleted = false;
    timerReboot = 0;
    timerPoweroff = 0;

    year = 2021;
    month=1;
    day=1;
    hour=9;
    min=0;
    sec=0;
}


bool Config::testConfigError(bool msgon, bool forceshow){
    int error=0;

    if(!sysConfigured){
        error = ERROR_SYS_CONFIG;
    }else if(!userConfigured){
        error = ERROR_USER_CONFIG;
    }else if(!packageConfigured){
        error = ERROR_PACKAGE_CONFIG;
    }else if(!startupCompleted){
        error = ERROR_STARTUP;
    }else if(!SN_Configured){
        error = ERROR_SN_CONFIG;
    }else if(!generator_configured){
        error = ERROR_CONF_GENERATOR;
    }else if(!compressor_configured){
        error = ERROR_CONF_COMPRESSOR;
    }else if(!collimator_configured){
        error = COLLI_WRONG_CONFIG;
    }else if(!pConfig->analog_configured){
        error =  ERROR_ANALOG_CONFIG;
    }

    if(!error){
        PageAlarms::activateNewAlarm(_DB_ALLARMI_SYSCONF,0);
        return false;
    }

    // Il sistema Ë in errore
    if(msgon){
        if(forceshow) PageAlarms::reopenExistingAlarm(_DB_ALLARMI_SYSCONF,error,FALSE);
        else PageAlarms::activateNewAlarm(_DB_ALLARMI_SYSCONF,error);
    }

    return true;

}

// Funzione necessaria per consentire al confugaratore esterno di operare
void Config::readLogo(void){
    // Verifica se nella HOME esiste un logo da aggiornare
    QString filename = QString("/home/user/logo.png");
    QFile file(filename);
    if (file.exists())
    {
        // Sposta il file di configurazione nell'area risorse
        // Effettua un sync
        QString command = QString("mv /home/user/logo.png /resource/config/logo.png");
        system(command.toStdString().c_str());

        // Effettua un sync
        command = QString("sync");
        system(command.toStdString().c_str());
    }
    return;
}

// Inizializzazione configuratore Slave
void Config::configSlave(void)
{
    this->isMaster = FALSE;
    slaveDataInitialized = false;
    rvGuiSlave = QString("%1.%2").arg(APPREVMAJ).arg(APPREVMIN);



    // Disabilita il trasferimento Master Slave FTP
    ftpEnabled = false;

    if(QDateTime::currentDateTime().date().year()>=2018){
        systemTimeUpdated = true;
    }

    timerPoweroff = 0;
    timerXrayPush = 0; // Diagnostica pulsante raggi

    readLogo();

}

// Funzione per entrare nella pagina Operativa del sistema.
void Config::selectOperatingPage(){
    if(!isMaster) return;

    if(ApplicationDatabase.getDataU(_DB_EXPOSURE_MODE) == _EXPOSURE_MODE_OPERATING_MODE){
        GWindowRoot.setNewPage(_PG_OPEN_STUDY_ANALOG,GWindowRoot.curPage,DBase::_DB_INIT_PAGE);
    }else{
        GWindowRoot.setNewPage(_PG_CALIB_ANALOG,GWindowRoot.curPage,DBase::_DB_INIT_PAGE);
    }
}

// Funzione per uscire dalla pagina Operativa del sistema.
void Config::selectMainPage(){
    if(!isMaster) return;

    // Riattiva l'allarme di compressione a studio chiuso solo se non si trova in compressione
    if(!pCompressore->isCompressed()) pCompressore->enable_compressione_closed_study=false;

    GWindowRoot.setNewPage(_PG_MAIN_ANALOG,GWindowRoot.curPage,DBase::_DB_EXIT_PAGE);
}


// SE IL FIL EDI CONFIGURAZIONE NON VIENE TROVATO O RISULTASSE
// ILLEGGIBILE ALLORA IL SISTEMA APRIRA' LA FINESTRA DI FACTORY INIT
#define SYS_ITEMS 3
bool Config::openSysCfg(void)
{
    QList<QString> dati;
    unsigned char nItems = 0;

    // Inizializzazione    
    sys.armMotor    =   true;                   // Presenza rotazione motorizzata/freno
    sys.trxMotor    =   true;                   // Presenza pendolazione
    sys.highSpeedStarter = true;                // Presenza starter alta velocit‡

    // Verifica se esiste il file: se non esiste lo crea dai defaults
    QFile file(QString(SYSCFG).toAscii());
    if(!file.exists()){
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }


    while(1)
    {

        dati = getNextArrayFields(&file);
        if(dati.isEmpty()) break;

        // Se il dato non corretto non lo considera
        if(dati.size()!=2) continue;

        // Sblocco/Blocco compressore
        if(dati.at(0)=="ARMMOT"){
            nItems++;
            if(dati.at(1).toInt()==1)      sys.armMotor=true;
            else sys.armMotor=false;
        }else  if(dati.at(0)=="TILTMOT"){
            nItems++;
            if(dati.at(1).toInt()==1)      sys.trxMotor=true;
            else sys.trxMotor=false;
        }else if(dati.at(0)=="HS_STARTER"){
            nItems++;
            if(dati.at(1)=="1") sys.highSpeedStarter = true; // IAE
            else sys.highSpeedStarter = false; // Starter Low Speed interno
        }

    }

    file.close();

    // In caso di mancanza di items (nuovi items aggiunti)
    if(nItems < SYS_ITEMS){
        return false;
    }

    return TRUE;
}

/*
 *  Funzione per il salvataggio del file di configurazione
 */
bool Config::saveSysCfg(void)
{
    QString frame;
    QString filename =  QString(SYSCFG);

    // Apre il file in scrittura
    QFile file(filename.toAscii());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return FALSE;
    }

    if(sys.armMotor) frame = QString("<ARMMOT,1>        //  Arm with motor\n");
    else frame = QString("<ARMMOT,0>        //  Arm with brake\n");
    file.write(frame.toAscii().data());

    if(sys.trxMotor) frame = QString("<TILTMOT,1>        //  Tilt with motor\n");
    else frame = QString("<TILTMOT,0>        //  No Tilt present\n");
    file.write(frame.toAscii().data());

    // Ozione uso starter alta velocit‡
    if(sys.highSpeedStarter) file.write("<HS_STARTER,1>    // opzione utilizzo hs starter\n");
    else file.write("<HS_STARTER,0>    // opzione utilizzo hs starter\n");

    pSysLog->log("CONFIG: SYSTEM CONFIGURATION FILE");

    file.close();
    file.flush();

    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());

    return TRUE;
}

/*
 *  Modifica file di configurazione delle rotazioni.
 *  Se console_id!=0 allora il comando richiede una risposta asincrona
 *
 */
bool Config::setTomoSpeedMode(QString tomoModeStr, int console_id)
{
    unsigned char tomoMode = trxConfig.tomo_mode;

    // Verifica la modalit√  corrente 1F/2F e salva eventualmente la nuova modalit√ 
    if(tomoModeStr == "4F"){
        trxConfig.tomo_mode = _TOMO_MODE_4F;
    }else if(tomoModeStr == "3F"){
        trxConfig.tomo_mode = _TOMO_MODE_3F;
    }else if(tomoModeStr == "2F"){
        trxConfig.tomo_mode = _TOMO_MODE_2F;
    }else if(tomoModeStr == "1F"){
        trxConfig.tomo_mode = _TOMO_MODE_1F;
    }else return false;

    // Se la configurazione cambia, occorre ricaricare i nuovi dati e aggiornare i drivers
    if(tomoMode!=trxConfig.tomo_mode){
        if(!saveTrxConfig()) return false; // Fallito il salvataggio
        if(!readTomoConfig(getTomoFilename())) return false; // Fallita la rilettura dei dati Tomo
        updateTrxDriver();
    }

    return true;
}



void Config::setRotazioniCfgSlot(void){
    if(console_id){
        disconnect(this, SIGNAL(configUpdatedSgn()), this, SLOT(setRotazioniCfgSlot()));
        protoConsole cmd(console_id,UNICODE_FORMAT);
        pToConsole->sendNotificheTcp(cmd.ackToQByteArray(0)); // OK
    }
    return;
}

bool Config::openUserCfg(void)
{
    QString filename;
    QList<QString> dati;

    // Gruppo Generali
    userCnf.enableSblocco =true;
    userCnf.enableCheckAccessorio = false;      // Non usato
    userCnf.enableComboCheckAccessorio = false; // Non usato
    userCnf.enable2DCheckAccessorio = false;    // Non usato
    userCnf.enable3DCheckAccessorio = false;   // Non usato
    userCnf.enablePbCheckAccessorio = false;   // Non usato

    // Gruppo Starter
    if(sys.highSpeedStarter) {
        userCnf.starter_off_after_exposure = false;
        userCnf.starter_brake = false;
        userCnf.starterTimeout = 300;
    }else{
        userCnf.starter_off_after_exposure = true;
        userCnf.starter_brake = false;
        userCnf.starterTimeout = 300;
    }

    // Gruppo Diagnosi
    userCnf.enableHVMonitor = true;
    userCnf.enableTestGnd = true;
    userCnf.correnteAnodicaTest = 30;
    userCnf.enableIFil = true;
    userCnf.correnteFilamento=2020;
    userCnf.enableTestMasmetro = true;
    userCnf.tempCuffiaAlr = 52;     // Set allarme cuffia
    userCnf.tempCuffiaAlrOff = 45;  // Reset Allarme cuffia


    // Gruppo Speciali
    userCnf.demoMode = false;   
    userCnf.deadman = false;
    userCnf.tubeFileName="TEMPLATE_XM12L40";
    userCnf.languageInterface = "ENG";
    userCnf.SN = "";
    userCnf.ServicePassword = "271070";
    userCnf.audioEnable =false;
    userCnf.volumeAudio = 0; // Massimo volume

    // Apre automaticamente il file user
    filename =  QString(USERCFG);
    QFile file2(filename.toAscii());
    if (!file2.open(QIODevice::ReadOnly | QIODevice::Text))  return saveUserCfg();

    while(1)
    {

        dati = getNextArrayFields(&file2);
        if(dati.isEmpty()) break;

        // Se il dato non √® corretto non lo considera
        if(dati.size()!=2) continue;

        // Sblocco/Blocco compressore
        if(dati.at(0)=="TUBE"){

            userCnf.tubeFileName=dati.at(1);
        }else  if(dati.at(0)=="SBLOCCO_COMPRESSORE"){

            // Legge la configurazione dello sblocco compressore            
            if(dati.at(1)=="1"){
                userCnf.enableSblocco =true;
                ApplicationDatabase.setData(_DB_COMPRESSOR_UNLOCK,(unsigned char) 1,0);
            }
            else{
                userCnf.enableSblocco = false;
                ApplicationDatabase.setData(_DB_COMPRESSOR_UNLOCK,(unsigned char) 0,0);
            }

        }else if(dati.at(0)=="TEMPCUFFIA_ON"){
            userCnf.tempCuffiaAlr = dati.at(1).toInt();
        }else if(dati.at(0)=="TEMPCUFFIA_OFF"){
            userCnf.tempCuffiaAlrOff = dati.at(1).toInt();
        }else if(dati.at(0)=="STARTER_TIMEOUT"){
            userCnf.starterTimeout = dati.at(1).toInt();
        }else if(dati.at(0)=="STARTER_OFF"){
            if(dati.at(1)=="1") userCnf.starter_off_after_exposure = true;
            else userCnf.starter_off_after_exposure = false;
        }else if(dati.at(0)=="STARTER_BRK"){
            if(dati.at(1)=="1") userCnf.starter_brake = true;
            else userCnf.starter_brake = false;
        }else if(dati.at(0)=="ACCESSORIO_PAZIENTE"){
            if(dati.at(1)=="1") userCnf.enableCheckAccessorio = TRUE;
            else userCnf.enableCheckAccessorio = FALSE;
        }else if(dati.at(0)=="PCB190_GNDTEST_ENABLE"){
            if(dati.at(1)=="1") userCnf.enableTestGnd = TRUE;
            else userCnf.enableTestGnd = FALSE;
        }else if(dati.at(0)=="PCB190_ITEST"){
            userCnf.correnteAnodicaTest = dati.at(1).toInt();
        }else if(dati.at(0)=="PCB190_IFIL"){
            userCnf.correnteFilamento = dati.at(1).toInt();
        }else if(dati.at(0)=="PCB190_MAS_ENA"){
            if(dati.at(1)=="1") userCnf.enableTestMasmetro = TRUE;
            else userCnf.enableTestMasmetro = FALSE;
        }else if(dati.at(0)=="PCB190_IFIL_ENA"){
            if(dati.at(1)=="1") userCnf.enableIFil = TRUE;
            else userCnf.enableIFil = FALSE;
        }else if(dati.at(0)=="PCB190_HV"){
            if(dati.at(1)=="1") userCnf.enableHVMonitor = TRUE;
            else userCnf.enableHVMonitor = FALSE;
        }else if(dati.at(0)=="DEAD_MAN"){
            if(dati.at(1)=="1") {
                userCnf.deadman = TRUE;
            } else{
                userCnf.deadman = FALSE;
            }
        }else if(dati.at(0)=="DEMO"){
            if(dati.at(1)=="1") {
                userCnf.demoMode = TRUE;
            } else{
                userCnf.demoMode = FALSE;
            }
        }else if(dati.at(0)=="LANGUAGE"){
            userCnf.languageInterface = dati.at(1);
        }else if(dati.at(0)=="SN"){
            userCnf.SN = dati.at(1);
            if(userCnf.SN !="") SN_Configured=true;
            else SN_Configured=false;
        }else if(dati.at(0)=="PASSWD"){
            userCnf.ServicePassword = dati.at(1);
        }else if(dati.at(0)=="AUDIO"){
             if(dati.at(1).toInt()) userCnf.audioEnable=true;
             else userCnf.audioEnable=false;
        }else if(dati.at(0)=="VAUDIO"){
            userCnf.volumeAudio=dati.at(1).toInt();
        }


    }

    // Con lo Starter si forza il parametro relativo allo stop
    if(sys.highSpeedStarter) {
        userCnf.starter_off_after_exposure = false;
        userCnf.starter_brake = false;
    }
    file2.close();
    return TRUE;
}

bool Config::saveUserCfg(void)
{
    QString frame;
    QString filename =  QString(USERCFG);

    // Apre il file in scrittura
    QFile file(filename.toAscii());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  return FALSE;


    // GRUPPO GENERALI -------------------------------------------------------------------------------------------
    // Sblocco compressore
    if(userCnf.enableSblocco) file.write("<SBLOCCO_COMPRESSORE,1>    // Impostazione sblocco compressore\n");
    else file.write("<SBLOCCO_COMPRESSORE,0>    // Impostazione sblocco compressore\n");


    if(userCnf.starter_off_after_exposure) file.write("<STARTER_OFF,1>    // opzione spegnimento starter dopo esposizione\n");
    else file.write("<STARTER_OFF,0>    // opzione spegnimento starter dopo esposizione\n");

    if(userCnf.starter_brake) file.write("<STARTER_BRK,1>    // opzione spegnimento starter con frenatura\n");
    else file.write("<STARTER_BRK,0>    // opzione spegnimento starter con frenatura\n");


    // Timer spegnimento starter
    frame = QString("<STARTER_TIMEOUT,%1>  //  Tempo di corsa starter\n").arg(userCnf.starterTimeout);
    file.write(frame.toAscii().data());

    // Ozione utilizzo accessorio
    if(userCnf.enableCheckAccessorio) file.write("<ACCESSORIO_PAZIENTE,1>    // opzione verifica accessorio di protezione\n");
    else file.write("<ACCESSORIO_PAZIENTE,0>    // opzione verifica accessorio di protezione\n");   

    // GRUPPO DIAGNOSTICA ----------------------------------------------------------------------------------------------
    if(userCnf.enableTestGnd) file.write("<PCB190_GNDTEST_ENABLE,1>    // opzione verifica accessorio di protezione\n");
    else file.write("<PCB190_GNDTEST_ENABLE,0>    // opzione verifica accessorio di protezione\n");

    frame = QString("<PCB190_ITEST,%1>  //  I test PCB190\n").arg(userCnf.correnteAnodicaTest);
    file.write(frame.toAscii().data());

    frame = QString("<PCB190_IFIL,%1>  //  I filamento di riscaldamento\n").arg(userCnf.correnteFilamento);
    file.write(frame.toAscii().data());

    if(userCnf.enableTestMasmetro) file.write("<PCB190_MAS_ENA,1>    // Abilitazione monitoraggio mAsmetro\n");
    else file.write("<PCB190_MAS_ENA,0>    // Abilitazione monitoraggio mAsmetro\n");

    if(userCnf.enableIFil) file.write("<PCB190_IFIL_ENA,1>    //  Abilitazione monitoraggio corrente di filamento\n");
    else file.write("<PCB190_IFIL_ENA,0>    // Abilitazione monitoraggio corrente di filamento\n");

    if(userCnf.enableHVMonitor) file.write("<PCB190_HV,1>    //  Abilitazione monitoraggio livelli HV\n");
    else file.write("<PCB190_HV,0>    // Abilitazione monitoraggio livelli HV\n");

    frame = QString("<TEMPCUFFIA_ON,%1>  //  Temperatura allarme cuffia\n").arg(userCnf.tempCuffiaAlr);
    file.write(frame.toAscii().data());

    frame = QString("<TEMPCUFFIA_OFF,%1>  //  Temperatura allarme cuffia reset\n").arg(userCnf.tempCuffiaAlrOff);
    file.write(frame.toAscii().data());

    // GRUPPO SPECIALI --------------------------------------------------------------------------------------------------------
    if(userCnf.demoMode) file.write("<DEMO,1>    //  Attivazione/Disattivazione modalit√  demo\n");
    else file.write("<DEMO,0>    //  Attivazione/Disattivazione modalit√  demo\n");

    if(userCnf.deadman) file.write("<DEAD_MAN,1>    //  Attivazione/Disattivazione dead-man\n");
    else file.write("<DEAD_MAN,0>    //  Attivazione/Disattivazione dead-man\n");

    if(userCnf.audioEnable) frame = QString("<AUDIO,1>          //  Audio messages\n");
    else frame = QString("<AUDIO,0>          //  Audio messages\n");
    file.write(frame.toAscii().data());

    frame = QString("<VAUDIO,%1>  //  volume audio\n").arg(userCnf.volumeAudio);
    file.write(frame.toAscii().data());

    frame = QString("<TUBE,%1>          //  XRAY-Tube Model\n").arg(userCnf.tubeFileName);
    file.write(frame.toAscii().data());

    frame = QString("<LANGUAGE,%1>  //  Linguaggio interfaccia\n").arg(userCnf.languageInterface);
    file.write(frame.toAscii().data());

    frame = QString("<SN,%1>  //  Serial Number\n").arg(userCnf.SN);
    file.write(frame.toAscii().data());

    frame = QString("<PASSWD,%1>  //  Serial Number\n").arg(userCnf.ServicePassword);
    file.write(frame.toAscii().data());

    file.close();
    file.flush();

    pSysLog->log("CONFIG: USER CONFIGURATION FILE");

    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());

    return TRUE;
}

bool Config::openPackageCfg(QString filename, firmwareCfg_Str* swConf)
{
    QList<QString> dati;

    swConf->rvGuiMaster.clear();
    swConf->rvGuiSlave.clear();
    swConf->rvM4Master.clear();
    swConf->rvM4Slave.clear();
    swConf->rv269.clear();
    swConf->rv240.clear();
    swConf->rv249U1.clear();
    swConf->rv249U2.clear();
    swConf->rv190.clear();
    swConf->rv244.clear();
    swConf->rv244A.clear();
    swConf->installedPackageName.clear();
    swConf->rvPackage.clear();
    swConf->rvLanguage.clear();


    QFile file(filename.toAscii());
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) return FALSE;


    while(1)
    {

        dati = getNextArrayFields(&file);

        if(dati.isEmpty()) break;

        // Se il dato non √® corretto non lo considera
        if(dati.size()!=2) continue;

        if(dati.at(0)=="MASTER")            swConf->rvGuiMaster = dati.at(1);
        else if(dati.at(0)=="SLAVE")        swConf->rvGuiSlave = dati.at(1);
        else if(dati.at(0)=="M4_MASTER")    swConf->rvM4Master = dati.at(1);
        else if(dati.at(0)=="M4_SLAVE")     swConf->rvM4Slave = dati.at(1);
        else if(dati.at(0)=="PCB269")       swConf->rv269 = dati.at(1);
        else if(dati.at(0)=="PCB240")       swConf->rv240 = dati.at(1);
        else if(dati.at(0)=="PCB249U1")     swConf->rv249U1 = dati.at(1);
        else if(dati.at(0)=="PCB249U2")     swConf->rv249U2 = dati.at(1);
        else if(dati.at(0)=="PCB190")       swConf->rv190 = dati.at(1);
        else if(dati.at(0)=="PCB244A")      swConf->rv244A = dati.at(1);
        else if(dati.at(0)=="PACKAGE")      swConf->rvPackage = dati.at(1);
        else if(dati.at(0)=="FILENAME")     swConf->installedPackageName = dati.at(1);
        else if(dati.at(0)=="LANGUAGE")     swConf->rvLanguage = dati.at(1);

    }

    file.close();
    return TRUE;

}

bool Config::savePackageCfg(QString filename, firmwareCfg_Str sw)
{
    QString data;


    QFile file( filename.toAscii());
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) return FALSE;

    data = QString("<PACKAGE,%1>  //  Revisione Package corrente\n").arg(sw.rvPackage);
    file.write(data.toAscii().data());

    data = QString("<FILENAME,%1>  //  file package istallato\n").arg(sw.installedPackageName);
    file.write(data.toAscii().data());

    data = QString("<MASTER,%1>  //  Revisione GUI Master\n").arg(sw.rvGuiMaster);
    file.write(data.toAscii().data());
    data = QString("<SLAVE,%1>  //  Revisione GUI Slave\n").arg(sw.rvGuiSlave);
    file.write(data.toAscii().data());

    data = QString("<M4_MASTER,%1>  //  Revisione M4 Master\n").arg(sw.rvM4Master);
    file.write(data.toAscii().data());
    data = QString("<M4_SLAVE,%1>  //  Revisione M4 Slave\n").arg(sw.rvM4Slave);
    file.write(data.toAscii().data());

    data = QString("<LANGUAGE,%1>  //  language package istallato\n").arg(sw.rvLanguage);
    file.write(data.toAscii().data());


    data = QString("<PCB269,%1>  //  Revisione PCB269\n").arg(sw.rv269);
    file.write(data.toAscii().data());

    data = QString("<PCB190,%1>  //  Revisione PCB190\n").arg(sw.rv190);
    file.write(data.toAscii().data());

    data = QString("<PCB240,%1>  //  Revisione PCB240\n").arg(sw.rv240);
    file.write(data.toAscii().data());

    data = QString("<PCB249U1,%1>  //  Revisione PCB249U1\n").arg(sw.rv249U1);
    file.write(data.toAscii().data());

    data = QString("<PCB249U2,%1>  //  Revisione PCB249U2\n").arg(sw.rv249U2);
    file.write(data.toAscii().data());


    data = QString("<PCB244A,%1>  //  Revisione PCB244-A\n").arg(sw.rv244A);
    file.write(data.toAscii().data());

    file.close();
    file.flush();

    pSysLog->log("CONFIG: PACKAGE CONFIGURATION FILE");

    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());

    return TRUE;

}


// Gestione file di configurazione per la macchina analogica
#define _ANALOG_CONFIG_ID   1
bool Config::openAnalogConfig(void){
QList<QString> dati;

    // Impostazioni di default
    analogCnf.doseFormat='m';

    analogCnf.selectedDkV=250;
    analogCnf.selectedDmas=500;
    analogCnf.selected_filtro = analogCnf.primo_filtro;
    analogCnf.auto_filtro_mode =false;
    analogCnf.current_profile = 0;
    analogCnf.aec_field = ANALOG_AECFIELD_FRONT;
    analogCnf.tech_mode  = ANALOG_TECH_MODE_MANUAL;


    // Campi file di configurazione analog.cnf
    analogCnf.primo_filtro = Collimatore::FILTRO_Mo;
    analogCnf.secondo_filtro = Collimatore::FILTRO_ND;

    analogCnf.DKV_HC = -2;
    analogCnf.DKV_LD = +2;

    analogCnf.minKvFilm = 23;
    analogCnf.maxKvFilm = 32;
    analogCnf.minKvCR = 26;
    analogCnf.maxKvCR = 31;

    analogCnf.calib_f1 = 0;     // Valore di riferimento per verifica sul campo (campo 1)
    analogCnf.calib_f2 = 0;     // Valore di riferimento per verifica sul campo (campo 2)
    analogCnf.calib_f3 = 0;     // Valore di riferimento per verifica sul campo (campo 3)
    analogCnf.calib_margine = 10; // Margine di accettabilita per la verifica dei campi

    analogCnf.LCC = 0;
    analogCnf.LMLO = 45;
    analogCnf.LML =90;
    analogCnf.LISO=135;
    analogCnf.LFB=180;
    analogCnf.LSIO=-45;
    analogCnf.LLM=-90;
    analogCnf.LLMO=-135;

    analogCnf.RCC=0;
    analogCnf.RMLO=-45;
    analogCnf.RML=-90;
    analogCnf.RISO=-135;
    analogCnf.RFB=180;
    analogCnf.RSIO=45;
    analogCnf.RLM=90;
    analogCnf.RLMO=135;



    // Lettura file di configurazione
    QFile file("/resource/config/analog.cnf");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return saveAnalogConfig();
    }

    // Lettura file di configurazione
    while(1)
    {
        dati = getNextArrayFields(&file);
        if(dati.isEmpty()) break;

        if(dati.size()==2){
            if(dati.at(0)=="DOSE"){
                analogCnf.doseFormat = dati.at(1).toAscii().at(0);
            }else if(dati.at(0)=="DKV"){
                analogCnf.selectedDkV = dati.at(1).toInt();
            }else  if(dati.at(0)=="DMAS"){
                analogCnf.selectedDmas = dati.at(1).toInt();
            }else if(dati.at(0)=="PRIMO_FILTRO"){
                if(dati.at(1)=="Rh") analogCnf.primo_filtro = Collimatore::FILTRO_Rh;
                else analogCnf.primo_filtro = Collimatore::FILTRO_Mo;
            }else if(dati.at(0)=="SECONDO_FILTRO"){
                if(dati.at(1)=="Rh") analogCnf.secondo_filtro = Collimatore::FILTRO_Rh;
                else if(dati.at(1)=="Mo") analogCnf.secondo_filtro = Collimatore::FILTRO_Mo;
                else analogCnf.secondo_filtro = Collimatore::FILTRO_ND;
            }else if(dati.at(0)=="SELECTED_FILTRO"){
                if(dati.at(1)=="Rh") analogCnf.selected_filtro = Collimatore::FILTRO_Rh;
                else analogCnf.selected_filtro = Collimatore::FILTRO_Mo;
            }else if(dati.at(0)=="AUTO_FILTRO"){
                if(dati.at(1).toInt()) analogCnf.auto_filtro_mode =true;
                else analogCnf.auto_filtro_mode =false;               
            }else if(dati.at(0)=="AEC_FIELD"){
                if(dati.at(1)=="FRONT") analogCnf.aec_field = ANALOG_AECFIELD_FRONT;
                else if(dati.at(1)=="BACK") analogCnf.aec_field = ANALOG_AECFIELD_BACK;
                else analogCnf.aec_field = ANALOG_AECFIELD_CENTER;
            }else if(dati.at(0)=="TECH_MODE"){
                if(dati.at(1)=="MANUAL") analogCnf.tech_mode  = ANALOG_TECH_MODE_MANUAL;
                else if(dati.at(1)=="SEMI") analogCnf.tech_mode  = ANALOG_TECH_MODE_SEMI;
                else analogCnf.tech_mode  = ANALOG_TECH_MODE_AUTO;
            }else if(dati.at(0)=="PROFILE"){
                analogCnf.current_profile = dati.at(1).toInt();
            }else if(dati.at(0)=="DKVHC"){
                analogCnf.DKV_HC = dati.at(1).toInt();
            }else if(dati.at(0)=="DKVLD"){
                analogCnf.DKV_LD = dati.at(1).toInt();
            }else if(dati.at(0)=="F1"){
                analogCnf.calib_f1 = dati.at(1).toInt();
            }else if(dati.at(0)=="F2"){
                analogCnf.calib_f2 = dati.at(1).toInt();
            }else if(dati.at(0)=="F3"){
                analogCnf.calib_f3 = dati.at(1).toInt();
            }else if(dati.at(0)=="MARGINE_F"){
                analogCnf.calib_margine = dati.at(1).toInt();
            }else if(dati.at(0)=="LCC"){
                analogCnf.LCC = dati.at(1).toInt();
            }else if(dati.at(0)=="LMLO"){
                analogCnf.LMLO = dati.at(1).toInt();
            }else if(dati.at(0)=="LML"){
                analogCnf.LML = dati.at(1).toInt();
            }else if(dati.at(0)=="LISO"){
                analogCnf.LISO = dati.at(1).toInt();
            }else if(dati.at(0)=="LFB"){
                analogCnf.LFB = dati.at(1).toInt();
            }else if(dati.at(0)=="LSIO"){
                analogCnf.LSIO = dati.at(1).toInt();
            }else if(dati.at(0)=="LLM"){
                analogCnf.LLM = dati.at(1).toInt();
            }else if(dati.at(0)=="LLMO"){
                analogCnf.LLMO = dati.at(1).toInt();
            }else if(dati.at(0)=="RCC"){
                analogCnf.RCC = dati.at(1).toInt();
            }else if(dati.at(0)=="RMLO"){
                analogCnf.RMLO = dati.at(1).toInt();
            }else if(dati.at(0)=="RML"){
                analogCnf.RML = dati.at(1).toInt();
            }else if(dati.at(0)=="RISO"){
                analogCnf.RISO = dati.at(1).toInt();
            }else if(dati.at(0)=="RFB"){
                analogCnf.RFB = dati.at(1).toInt();
            }else if(dati.at(0)=="RSIO"){
                analogCnf.RSIO = dati.at(1).toInt();
            }else if(dati.at(0)=="RLM"){
                analogCnf.RLM = dati.at(1).toInt();
            }else if(dati.at(0)=="RLMO"){
                analogCnf.RLMO = dati.at(1).toInt();
            }

        }

        if(dati.size()==3){
            if(dati.at(0)=="FILMKV"){
                analogCnf.minKvFilm = dati.at(1).toInt();
                analogCnf.maxKvFilm = dati.at(2).toInt();
                if(analogCnf.minKvFilm < 20) analogCnf.minKvFilm = 20;
                if(analogCnf.minKvFilm > 35) analogCnf.minKvFilm = 35;
                if(analogCnf.maxKvFilm < 20) analogCnf.maxKvFilm = 20;
                if(analogCnf.maxKvFilm > 35) analogCnf.maxKvFilm = 35;
            }else if(dati.at(0)=="CRKV"){
                analogCnf.minKvCR = dati.at(1).toInt();
                analogCnf.maxKvCR = dati.at(2).toInt();
                if(analogCnf.minKvCR < 20) analogCnf.minKvCR = 20;
                if(analogCnf.minKvCR > 35) analogCnf.minKvCR = 35;
                if(analogCnf.maxKvCR < 20) analogCnf.maxKvCR = 20;
                if(analogCnf.maxKvCR > 35) analogCnf.maxKvCR = 35;
            }
        }
    }
    file.close();

    // Applicazione di regole di consistenza
    if(analogCnf.primo_filtro == Collimatore::FILTRO_ND)
        analogCnf.primo_filtro = Collimatore::FILTRO_Mo;

    if(analogCnf.secondo_filtro == analogCnf.primo_filtro)
        analogCnf.secondo_filtro = Collimatore::FILTRO_ND;

    if(analogCnf.secondo_filtro == Collimatore::FILTRO_ND){
        analogCnf.auto_filtro_mode = false;
        analogCnf.selected_filtro = analogCnf.primo_filtro;
    }


    return true;

}

bool Config::saveAnalogConfig(void){
QString frame;
QFile   file("/resource/config/analog.cnf");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() <<"IMPOSSIBILE SALVARE IL FILE:" << QString("/resource/config/analog.cnf");
        return FALSE;
    }


    frame = QString("<DOSE,%1>  \n").arg(analogCnf.doseFormat);
    file.write(frame.toAscii().data());

    frame = QString("<DKV,%1>  \n").arg(analogCnf.selectedDkV);
    file.write(frame.toAscii().data());

    frame = QString("<DMAS,%1>  \n").arg(analogCnf.selectedDmas);
    file.write(frame.toAscii().data());

    if(analogCnf.tech_mode==ANALOG_TECH_MODE_MANUAL){
        frame = QString("<TECH_MODE,MANUAL>  \n");
    }else if(analogCnf.tech_mode==ANALOG_TECH_MODE_SEMI){
        frame = QString("<TECH_MODE,SEMI>  \n");
    }else {
         frame = QString("<TECH_MODE,AUTO>  \n");
    }
    file.write(frame.toAscii().data());

    if(analogCnf.selected_filtro == Collimatore::FILTRO_Rh){
        frame = QString("<SELECTED_FILTRO,Rh>  \n");
    }else {
        frame = QString("<SELECTED_FILTRO,Mo>  \n");
    }
    file.write(frame.toAscii().data());

    if(analogCnf.auto_filtro_mode){
        frame = QString("<AUTO_FILTRO,1>  \n");
    }else {
         frame = QString("<AUTO_FILTRO,0>  \n");
    }
    file.write(frame.toAscii().data());

    if(analogCnf.aec_field==ANALOG_AECFIELD_FRONT){
        frame = QString("<AEC_FIELD,FRONT>  \n");
    }else if(analogCnf.aec_field==ANALOG_AECFIELD_BACK){
        frame = QString("<AEC_FIELD,BACK>  \n");
    }else {
         frame = QString("<AEC_FIELD,MIDDLE>  \n");
    }
    file.write(frame.toAscii().data());

    if(analogCnf.current_profile >= MAX_ANALOG_PROFILES) analogCnf.current_profile = 0;
    frame = QString("<PROFILE,%1>  \n").arg(analogCnf.current_profile);
    file.write(frame.toAscii().data());


    if(analogCnf.primo_filtro == Collimatore::FILTRO_Rh){
        frame = QString("<PRIMO_FILTRO,Rh>  \n");
    }else {
        frame = QString("<PRIMO_FILTRO,Mo>  \n");
    }
    file.write(frame.toAscii().data());

    if(analogCnf.secondo_filtro == Collimatore::FILTRO_Rh){
        frame = QString("<SECONDO_FILTRO,Rh>  \n");
    }else if(analogCnf.secondo_filtro == Collimatore::FILTRO_Mo){
        frame = QString("<SECONDO_FILTRO,Mo>  \n");
    }else {
        frame = QString("<SECONDO_FILTRO,ND>  \n");
    }
    file.write(frame.toAscii().data());


    frame = QString("<DKVHC,%1>  \n").arg(analogCnf.DKV_HC);
    file.write(frame.toAscii().data());
    frame = QString("<DKVLD,%1>  \n").arg(analogCnf.DKV_LD);
    file.write(frame.toAscii().data());

    frame = QString("<FILMKV,%1,%2>  \n").arg(analogCnf.minKvFilm).arg(analogCnf.maxKvFilm);
    file.write(frame.toAscii().data());

    frame = QString("<CRKV,%1,%2>  \n").arg(analogCnf.minKvCR).arg(analogCnf.maxKvCR);
    file.write(frame.toAscii().data());

    frame = QString("<F1,%1>  \n").arg(analogCnf.calib_f1);
    file.write(frame.toAscii().data());
    frame = QString("<F2,%1>  \n").arg(analogCnf.calib_f2);
    file.write(frame.toAscii().data());
    frame = QString("<F3,%1>  \n").arg(analogCnf.calib_f3);
    file.write(frame.toAscii().data());
    frame = QString("<MARGINE_F,%1>  \n").arg(analogCnf.calib_margine);
    file.write(frame.toAscii().data());

    frame = QString("<LCC,%1>  \n").arg(analogCnf.LCC);
    file.write(frame.toAscii().data());

    frame = QString("<LMLO,%1>  \n").arg(analogCnf.LMLO);
    file.write(frame.toAscii().data());

    frame = QString("<LML,%1>  \n").arg(analogCnf.LML);
    file.write(frame.toAscii().data());

    frame = QString("<LISO,%1>  \n").arg(analogCnf.LISO);
    file.write(frame.toAscii().data());

    frame = QString("<LFB,%1>  \n").arg(analogCnf.LFB);
    file.write(frame.toAscii().data());

    frame = QString("<LSIO,%1>  \n").arg(analogCnf.LSIO);
    file.write(frame.toAscii().data());

    frame = QString("<LLM,%1>  \n").arg(analogCnf.LLM);
    file.write(frame.toAscii().data());

    frame = QString("<LLMO,%1>  \n").arg(analogCnf.LLMO);
    file.write(frame.toAscii().data());

    frame = QString("<LLMO,%1>  \n").arg(analogCnf.LLMO);
    file.write(frame.toAscii().data());



    frame = QString("<RCC,%1>  \n").arg(analogCnf.RCC);
    file.write(frame.toAscii().data());

    frame = QString("<RMLO,%1>  \n").arg(analogCnf.RMLO);
    file.write(frame.toAscii().data());

    frame = QString("<RML,%1>  \n").arg(analogCnf.RML);
    file.write(frame.toAscii().data());

    frame = QString("<RISO,%1>  \n").arg(analogCnf.RISO);
    file.write(frame.toAscii().data());

    frame = QString("<RFB,%1>  \n").arg(analogCnf.RFB);
    file.write(frame.toAscii().data());

    frame = QString("<RSIO,%1>  \n").arg(analogCnf.RSIO);
    file.write(frame.toAscii().data());

    frame = QString("<RLM,%1>  \n").arg(analogCnf.RLM);
    file.write(frame.toAscii().data());

    frame = QString("<RLMO,%1>  \n").arg(analogCnf.RLMO);
    file.write(frame.toAscii().data());

    frame = QString("<RLMO,%1>  \n").arg(analogCnf.RLMO);
    file.write(frame.toAscii().data());


    file.close();
    file.flush();


    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());

    return TRUE;
}

#ifdef __CAMPIONAMENTO_ANGOLO
// Salva il contenuto del potenziometro a 0 degree
bool Config::saveZeroPotCfg(unsigned short pot)
{
    QString data;

    QFile file( "/resource/config/zeropot.cnf");
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) return FALSE;

    rotCfg.zeropot= pot;

    data = QString("<VAL,%1>\n").arg(pot);
    file.write(data.toAscii().data());

    file.close();
    file.flush();

    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());

    return TRUE;

}
#endif

#ifdef __CAMPIONAMENTO_ANGOLO
// Restituisce TRUE se il file esisteva altrementi restituisce FALSE
// e i parametri saranno quelli di default
bool Config::readZeroPot(void)
{
    rotCfg.zeropot=515; // Per casi nuovi aggiornamenti

    // Apre il file
    QString filename = QString("/resource/config/zeropot.cnf");
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  return TRUE;

    QList<QString> dati;

    // Valori di default: Hotfix 11C
    while(1)
    {
        dati = getNextArrayFields(&file);
        if(dati.isEmpty()) break;
        // Se il dato non √® corretto non lo considera
        if(dati.size()!=2) continue;

        if(dati.at(0)=="VAL")  rotCfg.zeropot=dati.at(1).toInt();

    }

    file.close();
    return TRUE;
}
#endif

QByteArray Config::getLine(QFile* fp)
{
    int i;
    QByteArray dato;


    while(!fp->atEnd())
    {
        dato.clear();
        QByteArray frame = fp->readLine();

        // Inizio sequenza: carattere '<'
        for(i=0; i<frame.size(); i++)
        {
            // Elimina gli spazi vuoti
            if(frame.at(i)=='<') break;
        }
        if(i==frame.size()) continue;
        i++; // i contiene il primo carattere valido

        // Costruzione dato fino a  '>'
        for(;i<frame.size(); i++)
        {
            if(frame.at(i)=='>') return dato;
            dato.append(frame.at(i));
        }
        dato.clear();
        return dato; // Errore
    }

    dato.clear();
    return dato; // Errore
}

bool Config::getNextArrayLine(QFile* fp, unsigned char* data, int len)
{
    int i,j;
    QByteArray dato;


    while(!fp->atEnd())
    {
        j=0;
        dato.clear();
        QByteArray frame = fp->readLine();

        // Inizio sequenza: carattere '<'
        for(i=0; i<frame.size(); i++)
        {
            // Elimina gli spazi vuoti
            if(frame.at(i)=='<') break;
        }
        if(i==frame.size()) continue;
        i++; // i contiene il primo carattere valido

        // Costruzione dato fino a  '>' o a ','
        for(;i<frame.size(); i++)
        {
            if(frame.at(i)=='>')
            {
                // legge ultimo dato
                if(dato.size()==0) return FALSE;
                data[j]=(unsigned char) dato.toShort();
                j++;
                if(j!=len) return FALSE;
                return TRUE;
            }
            if(frame.at(i)==',')
            {
                // aggiorna array
                if(dato.size()==0) return FALSE;
                data[j]=(unsigned char) dato.toShort();                
                j++;
                if(j>len) return FALSE;
                dato.clear();
                continue;
            }

            // Trattasi di altro carattere del numero
            dato.append(frame.at(i));
        }

        return FALSE; // Errore
    }

    return FALSE; // Errore

}

bool Config::getNextArrayLine(QFile* fp, int* data, int len)
{
    int i,j;
    QByteArray dato;


    while(!fp->atEnd())
    {
        j=0;
        dato.clear();
        QByteArray frame = fp->readLine();

        // Inizio sequenza: carattere '<'
        for(i=0; i<frame.size(); i++)
        {
            // Elimina gli spazi vuoti
            if(frame.at(i)=='<') break;
        }
        if(i==frame.size()) continue;
        i++; // i contiene il primo carattere valido

        // Costruzione dato fino a  '>' o a ','
        for(;i<frame.size(); i++)
        {
            if(frame.at(i)=='>')
            {
                // legge ultimo dato
                if(dato.size()==0) return FALSE;
                data[j]=(int) dato.toInt();
                j++;
                if(j!=len) return FALSE;
                return TRUE;
            }
            if(frame.at(i)==',')
            {
                // aggiorna array
                if(dato.size()==0) return FALSE;
                data[j]=(int) dato.toInt();                
                j++;
                if(j>len) return FALSE;
                dato.clear();
                continue;
            }

            // Trattasi di altro carattere del numero
            dato.append(frame.at(i));
        }

        return FALSE; // Errore
    }

    return FALSE; // Errore

}


bool Config::getNextLine(QFile* fp, unsigned char* data)
{
    QByteArray dato = Config::getLine(fp);

    if(dato.size()==0) return FALSE;
    int val = dato.toInt();
    if(val>255) return FALSE;

    *data = (unsigned char) val;
    return TRUE;
}

bool Config::getNextLine(QFile* fp, signed char* data)
{
    QByteArray dato = getLine(fp);

    if(dato.size()==0) return FALSE;
    int val = dato.toInt();
    if(val>127) return FALSE;
    if(val<-128) return FALSE;

    *data = (signed char) (val&0xFF);
    return TRUE;
}

bool Config::getNextLine(QFile* fp, unsigned short* data)
{
    QByteArray dato = getLine(fp);

    if(dato.size()==0) return FALSE;
    long val = dato.toLong();
    if(val>0xFFFF) return FALSE;

    *data = (unsigned short) val;
    return TRUE;
}

bool Config::getNextLine(QFile* fp, signed short* data)
{
    QByteArray dato = getLine(fp);

    if(dato.size()==0) return FALSE;
    long val = dato.toLong();
    if(val>32767) return FALSE;
    if(val<-32768) return FALSE;

    *data = (signed short) (val&0xFFFF);
    return TRUE;
}

bool Config::getNextLine(QFile* fp, signed int* data)
{
    QByteArray dato = getLine(fp);

    if(dato.size()==0) return FALSE;
    *data = (signed int) dato.toInt();
    return TRUE;
}

bool Config::getNextLine(QFile* fp, unsigned int* data)
{
    QByteArray dato = getLine(fp);

    if(dato.size()==0) return FALSE;
    *data = (unsigned int) dato.toUInt();
    return TRUE;
}

bool Config::getNextLine(QFile* fp, QByteArray* data)
{
    QByteArray dato = getLine(fp);

    if(dato.size()==0) return FALSE;
    *data = dato;
    return TRUE;
}

bool Config::getNextLine(QFile* fp, QString* data)
{
    QByteArray dato = getLine(fp);

    if(dato.size()==0) return FALSE;
    *data = QString(dato);
    return TRUE;
}

QString Config::removeSpaces(QString frame){
    QString result;
    int i,j;

    result.clear();
    if(frame.isEmpty()) return result;


    for(i=0;i<frame.size();i++){
        if(frame.toAscii().at(i)!=' ') break;
    }
    if(i==frame.size()) return result;
    for(j=frame.size()-1;j>i;j--){
        if(frame.toAscii().at(j)!=' ') break;
    }
    if(j==i) return result;

    for(;i<=j;i++) result.append(frame.at(i));
    return result;

}

QList<QString> Config::getNextArrayFields(QFile* fp)
{
    QList<QString> lista;
    QString item;
    int i;

    QByteArray frame = getNextValidLine(fp) ;
    lista.clear();
    frame.replace(" ","");
    while(frame.size())
    {
        i = frame.indexOf(",");
        if(i==-1)
        {
            lista.append(frame); // Elimina gli spazi ..
            return lista;
        }

        item = frame.left(i);        
        lista.append(item);
        if(i==frame.size()) break;
        frame = frame.right(frame.size()-i-1);
    }
    return lista;
}
/*
 *
 *
 */

// Lettura file di configurazione pendolazione
// DATI TOMOGRAFIA:
// NARROW:          1.5∞, 11 shots
// INTERMEDIATE:    2∞, 13 shots
// WIDE:            2∞, 19 shots
// Calcolo anticipo: Anticipo = Velocit‡^2 /(2*Acc)
// Velocit‡ = Angolo/Ts
// Accelerazione e velocit‡ sono espresse in centesimi di grado/secondo e centesimi/secondoq
// Posizioni sono espresse in centesimi di grado
bool Config::readTrxConfig(void)
{
    // ________________________ CONTESTO MOVIMENTO 2D/BIOPSIA ________
    trxConfig.context2D.speed = 400;
    trxConfig.context2D.accell= 400;
    trxConfig.context2D.decell= 400;
    trxConfig.angolo_biopsia = 1500;

    // ________________________ AZZERAMENTO __________________________
    trxConfig.zero_setting.offset = 0;
    trxConfig.zero_setting.speed_approach = 200;
    trxConfig.zero_setting.speed_reverse = 100;
    trxConfig.zero_setting.accell = 50;
    trxConfig.zero_setting.speed_manual_approach = 10; // 0.1∞ secondo

    // ________________________ Modalit‡ Tomo  __________________________
    trxConfig.tomo_mode = _TOMO_MODE_2F; // Modalit‡ TOMO di default


    // Apre il file se esiste
    QString filename = QString(TRX_FILE_CFG);
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {

        // Crea il file di default
        saveTrxConfig();
        return readTomoConfig(getTomoFilename());
    }

    QList<QString> dati;
    while(1)
    {
        dati = getNextArrayFields(&file);
        if(dati.isEmpty()) break;
        if(dati.size()!=2) continue;

        if(dati.at(0)=="TMODE")             trxConfig.tomo_mode=dati.at(1).toInt();
        else if(dati.at(0)=="2DSPEED")      trxConfig.context2D.speed=dati.at(1).toInt();
        else if(dati.at(0)=="2DACC")        trxConfig.context2D.accell=dati.at(1).toInt();
        else if(dati.at(0)=="2DDEC")        trxConfig.context2D.decell=dati.at(1).toInt();
        else if(dati.at(0)=="BPOS")         trxConfig.angolo_biopsia=dati.at(1).toInt();

        else if(dati.at(0)=="ZOFS")         trxConfig.zero_setting.offset=dati.at(1).toInt();
        else if(dati.at(0)=="ZAPP")         trxConfig.zero_setting.speed_approach=dati.at(1).toInt();
        else if(dati.at(0)=="ZREV")         trxConfig.zero_setting.speed_reverse=dati.at(1).toInt();
        else if(dati.at(0)=="ZACC")         trxConfig.zero_setting.accell=dati.at(1).toInt();
        else if(dati.at(0)=="MZAPP")        trxConfig.zero_setting.speed_manual_approach=dati.at(1).toInt();

    }


    file.close();

    // Lettura file di configurazione Tomo relativo alla modalit‡ selezionata
    return readTomoConfig(getTomoFilename());
}

// Restituisce il nome del File di configurazione della Tomo relativo
// alla modalit‡ impostata
QString Config::getTomoFilename(void){
    // Lettura file di configurazione Tomo relativo alla modalit‡ selezionata
    if(trxConfig.tomo_mode==_TOMO_MODE_4F)
        return QString(TOMO_FILE_CFG).arg("4F");
    else if(trxConfig.tomo_mode==_TOMO_MODE_3F)
        return QString(TOMO_FILE_CFG).arg("3F");
    else if(trxConfig.tomo_mode==_TOMO_MODE_2F)
        return QString(TOMO_FILE_CFG).arg("2F");
    else
        return QString(TOMO_FILE_CFG).arg("1F");
}

bool Config::saveTrxConfig(void)
{
    QString filename = QString(TRX_FILE_CFG);
    QFile   file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() <<"IMPOSSIBILE SALVARE IL FILE:" << filename;
        return FALSE;
    }

    QString frame;

    frame = QString("<TMODE,%1>  \n").arg(trxConfig.tomo_mode);
    file.write(frame.toAscii().data());

    frame = QString("<BPOS,%1>  \n").arg(trxConfig.angolo_biopsia);
    file.write(frame.toAscii().data());

    frame = QString("<2DSPEED,%1>  \n").arg(trxConfig.context2D.speed);
    file.write(frame.toAscii().data());
    frame = QString("<2DACC,%1>  \n").arg(trxConfig.context2D.accell);
    file.write(frame.toAscii().data());
    frame = QString("<2DDEC,%1>  \n").arg(trxConfig.context2D.decell);
    file.write(frame.toAscii().data());


    frame = QString("<ZOFS,%1>   \n").arg(trxConfig.zero_setting.offset);
    file.write(frame.toAscii().data());

    frame = QString("<ZAPP,%1>   \n").arg(trxConfig.zero_setting.speed_approach);
    file.write(frame.toAscii().data());
    frame = QString("<ZREV,%1>   \n").arg(trxConfig.zero_setting.speed_reverse);
    file.write(frame.toAscii().data());
    frame = QString("<ZACC,%1>   \n").arg(trxConfig.zero_setting.accell);
    file.write(frame.toAscii().data());
    frame = QString("<MZAPP,%1>   \n").arg(trxConfig.zero_setting.speed_manual_approach);
    file.write(frame.toAscii().data());


    file.close();
    file.flush();


    pSysLog->log("CONFIG: TRX CONFIGURATION FILE");

    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());

    return TRUE;
}

//_________________________________________________________________________________________
//  READ TOMO CONFIG
bool Config::readTomoConfig(QString filename)
{

    //________________________________________________________________
    // Ts = 500ms;
    trxConfig.tomo.w.speed = 400; // Velocit‡ = 2∞/ 0.5
    trxConfig.tomo.w.accell= 400; // Anticipo = 4 * 4 / (2 * 4) = 2
    trxConfig.tomo.w.home_position = 1800 + 200;
    trxConfig.tomo.w.decell= trxConfig.tomo.w.accell;
    trxConfig.tomo.w.end_position = - trxConfig.tomo.w.home_position;
    trxConfig.tomo.w.samples = 19;
    trxConfig.tomo.w.pre_samples = 1;
    trxConfig.tomo.w.skip_samples = 0;

    trxConfig.tomo.i.speed = 400; // Velocit‡ = 2∞/ 0.5
    trxConfig.tomo.i.accell= 400; // Anticipo = 4 * 4 / (2 * 4) = 2
    trxConfig.tomo.i.home_position = 1200 + 200;
    trxConfig.tomo.i.decell= trxConfig.tomo.i.accell;
    trxConfig.tomo.i.end_position = -trxConfig.tomo.i.home_position;
    trxConfig.tomo.i.samples = 13;
    trxConfig.tomo.i.pre_samples = 1;
    trxConfig.tomo.i.skip_samples = 0;

    trxConfig.tomo.n.speed = 300; // Velocit‡ = 1.5∞/ 0.5
    trxConfig.tomo.n.accell= 300; // Anticipo = 3 * 3 / (2*3) = 1.5
    trxConfig.tomo.n.home_position = 750 + 150;
    trxConfig.tomo.n.decell= trxConfig.tomo.n.accell;
    trxConfig.tomo.n.end_position = -trxConfig.tomo.n.home_position;
    trxConfig.tomo.n.samples = 11;
    trxConfig.tomo.n.pre_samples = 1;
    trxConfig.tomo.n.skip_samples = 0;

    // Apre il file se esiste
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // Crea il file di default
        return saveTomoConfig(filename);
    }

    QList<QString> dati;
    while(1)
    {
        dati = getNextArrayFields(&file);
        if(dati.isEmpty()) break;
        if(dati.size()!=2) continue;

        if(dati.at(0)=="HOME_W")            trxConfig.tomo.w.home_position=dati.at(1).toInt();
        else if(dati.at(0)=="END_W")        trxConfig.tomo.w.end_position=dati.at(1).toInt();
        else if(dati.at(0)=="SPEED_W")      trxConfig.tomo.w.speed=dati.at(1).toInt();
        else if(dati.at(0)=="ACC_W")        trxConfig.tomo.w.accell=dati.at(1).toInt();
        else if(dati.at(0)=="DEC_W")        trxConfig.tomo.w.decell=dati.at(1).toInt();
        else if(dati.at(0)=="SMP_W")        trxConfig.tomo.w.samples=dati.at(1).toInt();
        else if(dati.at(0)=="PRESMP_W")     trxConfig.tomo.w.pre_samples=dati.at(1).toInt();
        else if(dati.at(0)=="SKSMP_W")      trxConfig.tomo.w.skip_samples=dati.at(1).toInt();

        if(dati.at(0)=="HOME_I")            trxConfig.tomo.i.home_position=dati.at(1).toInt();
        else if(dati.at(0)=="END_I")        trxConfig.tomo.i.end_position=dati.at(1).toInt();
        else if(dati.at(0)=="SPEED_I")      trxConfig.tomo.i.speed=dati.at(1).toInt();
        else if(dati.at(0)=="ACC_I")        trxConfig.tomo.i.accell=dati.at(1).toInt();
        else if(dati.at(0)=="DEC_I")        trxConfig.tomo.i.decell=dati.at(1).toInt();
        else if(dati.at(0)=="SMP_I")       trxConfig.tomo.i.samples=dati.at(1).toInt();
        else if(dati.at(0)=="PRESMP_I")    trxConfig.tomo.i.pre_samples=dati.at(1).toInt();
        else if(dati.at(0)=="SKSMP_I")      trxConfig.tomo.i.skip_samples=dati.at(1).toInt();

        if(dati.at(0)=="HOME_N")            trxConfig.tomo.n.home_position=dati.at(1).toInt();
        else if(dati.at(0)=="END_N")        trxConfig.tomo.n.end_position=dati.at(1).toInt();
        else if(dati.at(0)=="SPEED_N")      trxConfig.tomo.n.speed=dati.at(1).toInt();
        else if(dati.at(0)=="ACC_N")        trxConfig.tomo.n.accell=dati.at(1).toInt();
        else if(dati.at(0)=="DEC_N")        trxConfig.tomo.n.decell=dati.at(1).toInt();
        else if(dati.at(0)=="SMP_N")        trxConfig.tomo.n.samples=dati.at(1).toInt();
        else if(dati.at(0)=="PRESMP_N")     trxConfig.tomo.n.pre_samples=dati.at(1).toInt();
        else if(dati.at(0)=="SKSMP_N")      trxConfig.tomo.n.skip_samples=dati.at(1).toInt();


    }


    file.close();
    return TRUE;

}

bool Config::saveTomoConfig(QString filename)
{

    QFile   file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() <<"IMPOSSIBILE SALVARE IL FILE:" << filename;
        return FALSE;
    }

    QString frame;

    frame = QString("<HOME_W,%1>  \n").arg(trxConfig.tomo.w.home_position);
    file.write(frame.toAscii().data());
    frame = QString("<END_W,%1>   \n").arg(trxConfig.tomo.w.end_position);
    file.write(frame.toAscii().data());
    frame = QString("<SPEED_W,%1>   \n").arg(trxConfig.tomo.w.speed);
    file.write(frame.toAscii().data());
    frame = QString("<ACC_W,%1>   \n").arg(trxConfig.tomo.w.accell);
    file.write(frame.toAscii().data());
    frame = QString("<DEC_W,%1>   \n").arg(trxConfig.tomo.w.decell);
    file.write(frame.toAscii().data());
    frame = QString("<SMP_W,%1>   \n").arg(trxConfig.tomo.w.samples);
    file.write(frame.toAscii().data());
    frame = QString("<PRESMP_W,%1>   \n").arg(trxConfig.tomo.w.pre_samples);
    file.write(frame.toAscii().data());
    frame = QString("<SKSMP_W,%1>   \n").arg(trxConfig.tomo.w.skip_samples);
    file.write(frame.toAscii().data());

    frame = QString("<HOME_I,%1>  \n").arg(trxConfig.tomo.i.home_position);
    file.write(frame.toAscii().data());
    frame = QString("<END_I,%1>   \n").arg(trxConfig.tomo.i.end_position);
    file.write(frame.toAscii().data());
    frame = QString("<SPEED_I,%1>   \n").arg(trxConfig.tomo.i.speed);
    file.write(frame.toAscii().data());
    frame = QString("<ACC_I,%1>   \n").arg(trxConfig.tomo.i.accell);
    file.write(frame.toAscii().data());
    frame = QString("<DEC_I,%1>   \n").arg(trxConfig.tomo.i.decell);
    file.write(frame.toAscii().data());
    frame = QString("<SMP_I,%1>   \n").arg(trxConfig.tomo.i.samples);
    file.write(frame.toAscii().data());
    frame = QString("<PRESMP_I,%1>   \n").arg(trxConfig.tomo.i.pre_samples);
    file.write(frame.toAscii().data());
    frame = QString("<SKSMP_I,%1>   \n").arg(trxConfig.tomo.i.skip_samples);
    file.write(frame.toAscii().data());

    frame = QString("<HOME_N,%1>  \n").arg(trxConfig.tomo.n.home_position);
    file.write(frame.toAscii().data());
    frame = QString("<END_N,%1>   \n").arg(trxConfig.tomo.n.end_position);
    file.write(frame.toAscii().data());
    frame = QString("<SPEED_N,%1>   \n").arg(trxConfig.tomo.n.speed);
    file.write(frame.toAscii().data());
    frame = QString("<ACC_N,%1>   \n").arg(trxConfig.tomo.n.accell);
    file.write(frame.toAscii().data());
    frame = QString("<DEC_N,%1>   \n").arg(trxConfig.tomo.n.decell);
    file.write(frame.toAscii().data());
    frame = QString("<SMP_N,%1>   \n").arg(trxConfig.tomo.n.samples);
    file.write(frame.toAscii().data());
    frame = QString("<PRESMP_N,%1>   \n").arg(trxConfig.tomo.n.pre_samples);
    file.write(frame.toAscii().data());
    frame = QString("<SKSMP_N,%1>   \n").arg(trxConfig.tomo.n.skip_samples);
    file.write(frame.toAscii().data());


    file.close();
    file.flush();

    pSysLog->log("CONFIG: TOMO CONFIGURATION FILE");

    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());

    return TRUE;
}

//_________________________________________________________________________________________








// Lettura file di configurazione pendolazione
bool Config::readArmConfig(void)
{
    armConfig.speed = 200;
    armConfig.accell = 100;
    armConfig.decell = 100;
    armConfig.manual_speed = 100;
    armConfig.manual_accell = 50;
    armConfig.manual_decell = 200;
    armConfig.direction_memory = MEM_ARM_DIR_UNDEF;


    // Apre il file se esiste
    QString filename = QString(ARM_FILE_CFG);
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // Crea il file di default
        return saveArmConfig();
    }

    QList<QString> dati;
    while(1)
    {
        dati = getNextArrayFields(&file);
        if(dati.isEmpty()) break;
        if(dati.size()!=2) continue;

        else if(dati.at(0)=="SPEED") armConfig.speed=dati.at(1).toInt();
        else if(dati.at(0)=="ACC")   armConfig.accell=dati.at(1).toInt();
        else if(dati.at(0)=="DEC")   armConfig.decell=dati.at(1).toInt();
        else if(dati.at(0)=="MSPEED") armConfig.manual_speed=dati.at(1).toInt();
        else if(dati.at(0)=="MACC")   armConfig.manual_accell=dati.at(1).toInt();
        else if(dati.at(0)=="MDEC")   armConfig.manual_decell=dati.at(1).toInt();
        else if(dati.at(0)=="MEMDIR")   armConfig.direction_memory=dati.at(1).toInt();

    }

    file.close();
    return TRUE;

}

bool Config::saveArmConfig(void)
{
    QString filename = QString(ARM_FILE_CFG);
    QFile   file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() <<"IMPOSSIBILE SALVARE IL FILE:" << filename;
        return FALSE;
    }

    QString frame;


    frame = QString("<SPEED,%1>  \n").arg(armConfig.speed);
    file.write(frame.toAscii().data());

    frame = QString("<ACC,%1>  \n").arg(armConfig.accell);
    file.write(frame.toAscii().data());

    frame = QString("<DEC,%1>  \n").arg(armConfig.decell);
    file.write(frame.toAscii().data());

    frame = QString("<MSPEED,%1>  \n").arg(armConfig.manual_speed);
    file.write(frame.toAscii().data());

    frame = QString("<MACC,%1>  \n").arg(armConfig.manual_accell);
    file.write(frame.toAscii().data());

    frame = QString("<MDEC,%1>  \n").arg(armConfig.manual_decell);
    file.write(frame.toAscii().data());

    frame = QString("<MEMDIR,%1>  \n").arg(armConfig.direction_memory);
    file.write(frame.toAscii().data());

    file.close();
    file.flush();

    pSysLog->log("CONFIG: ARM CONFIGURATION FILE");
    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());

    return TRUE;
}

// Lettura file di configurazione pendolazione
bool Config::readLenzeConfig(void)
{
    lenzeConfig.calibrated = 0;
    lenzeConfig.min_lenze_position = 15;  // %
    lenzeConfig.max_lenze_position = 85;  // %
    lenzeConfig.manual_speed =  50;       // 50Hz
    lenzeConfig.automatic_speed =20;      // 20Hz

    // Apre il file se esiste
    QString filename = QString(LENZE_FILE_CFG);
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // Crea il file di default
        return saveLenzeConfig();
    }

    QList<QString> dati;
    while(1)
    {
        dati = getNextArrayFields(&file);
        if(dati.isEmpty()) break;
        if(dati.size()!=2) continue;

        if(dati.at(0)=="MINPOS")  lenzeConfig.min_lenze_position=dati.at(1).toInt();
        else if(dati.at(0)=="MAXPOS")  lenzeConfig.max_lenze_position=dati.at(1).toInt();
        else if(dati.at(0)=="MANSPEED")  lenzeConfig.manual_speed=dati.at(1).toInt();
        else if(dati.at(0)=="AUTOSPEED")  lenzeConfig.automatic_speed=dati.at(1).toInt();
        else if(dati.at(0)=="CALIBRATED") lenzeConfig.calibrated=dati.at(1).toInt();
    }

    file.close();
    return TRUE;

}

bool Config::saveLenzeConfig(void)
{
    QString filename = QString(LENZE_FILE_CFG);
    QFile   file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() <<"IMPOSSIBILE SALVARE IL FILE:" << filename;
        return FALSE;
    }

    QString frame;


    frame = QString("<MINPOS,%1>        \n").arg(lenzeConfig.min_lenze_position);
    file.write(frame.toAscii().data());

    frame = QString("<MAXPOS,%1>        \n").arg(lenzeConfig.max_lenze_position);
    file.write(frame.toAscii().data());

    frame = QString("<MANSPEED,%1>      \n").arg(lenzeConfig.manual_speed);
    file.write(frame.toAscii().data());

    frame = QString("<AUTOSPEED,%1>     \n").arg(lenzeConfig.automatic_speed);
    file.write(frame.toAscii().data());

    frame = QString("<CALIBRATED,%1>    \n").arg(lenzeConfig.calibrated);
    file.write(frame.toAscii().data());


    file.close();
    file.flush();

    pSysLog->log("CONFIG: LENZE CONFIGURATION FILE");

    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());

    return TRUE;
}



// PROTOCOLLO SUL COMANDO DI CONFIGURAZIONE:
// mcc_cmd.buffer[0] = Dispositivo da configurare
// mcc_cmd.buffer[1] = Codice Blocco dati (se utile, caso per caso)
// mcc_cmd.buffer[2:len-3] = Blocco dati da inviare
bool Config::sendMccConfigCommand(unsigned char cmd){
    unsigned char buffer[_MCC_DIM];
    unsigned char buflen=2;
    buffer[0] = cmd;
    buffer[1] = 0; // Numero di blocco dati (se necessario si pu√≤ spezzare l'intero buffer in pi√π blocchi)
    unsigned char* pData = &buffer[2]; // Per i blocchi di dati

    // Specificare il contenuto del buffer solo per i comandi che richiedono un buffer!
    switch(cmd){
        case CONFIG_PCB190:
            memcpy(pData, (unsigned char*) &pGeneratore->genCnf.pcb190, sizeof(pcb190Conf_Str));
            buflen += sizeof(pcb190Conf_Str);
        break;
        case CONFIG_PCB269:
            memcpy(pData, (unsigned char*) (&pCompressore->config), sizeof(compressoreCnf_Str));
            buflen += sizeof(compressoreCnf_Str);
        break;
        case CONFIG_PCB249U1_1:
            // Invia il blocco 0. Sulle risposte verranno inviati i blocchi successivi.
            memcpy(pData, (unsigned char*) (&pCollimatore->colliConf.colliTomoW.tomoLeftBladeP), COLLI_DYNAMIC_SAMPLES);
            buflen += COLLI_DYNAMIC_SAMPLES;
        break;
        case CONFIG_PCB249U1_2:
            // Invia il blocco 0. Sulle risposte verranno inviati i blocchi successivi.
            memcpy(pData, (unsigned char*) (&pCollimatore->colliConf.colliTomoW.tomoLeftBladeN), COLLI_DYNAMIC_SAMPLES);
            buflen += COLLI_DYNAMIC_SAMPLES;
        break;
        case CONFIG_PCB249U1_3:
            // Invia il blocco 0. Sulle risposte verranno inviati i blocchi successivi.
            memcpy(pData, (unsigned char*) (&pCollimatore->colliConf.colliTomoW.tomoRightBladeP), COLLI_DYNAMIC_SAMPLES);
            buflen += COLLI_DYNAMIC_SAMPLES;
        break;
        case CONFIG_PCB249U1_4:
            // Invia il blocco 0. Sulle risposte verranno inviati i blocchi successivi.
            memcpy(pData, (unsigned char*) (&pCollimatore->colliConf.colliTomoW.tomoRightBladeN), COLLI_DYNAMIC_SAMPLES);
            buflen += COLLI_DYNAMIC_SAMPLES;
        break;
        case CONFIG_PCB249U1_5:
            // Invia il blocco 0. Sulle risposte verranno inviati i blocchi successivi.
            memcpy(pData, (unsigned char*) (&pCollimatore->colliConf.colliTomoW.tomoBackTrapP), COLLI_DYNAMIC_SAMPLES);
            buflen += COLLI_DYNAMIC_SAMPLES;
        break;
        case CONFIG_PCB249U1_6:
            // Invia il blocco 0. Sulle risposte verranno inviati i blocchi successivi.
            memcpy(pData, (unsigned char*) (&pCollimatore->colliConf.colliTomoW.tomoBackTrapN), COLLI_DYNAMIC_SAMPLES);
            buflen += COLLI_DYNAMIC_SAMPLES;
        break;
        case CONFIG_PCB249U1_7:
            // Invia il blocco 0. Sulle risposte verranno inviati i blocchi successivi.
            pData[0] = pCollimatore->colliConf.colliTomoW.tomoFront;
            pData[1] = pCollimatore->colliConf.colliTomoW.tomoBack;
            pData[2] = pConfig->userCnf.tempCuffiaAlr;
            pData[3] = pConfig->userCnf.tempCuffiaAlrOff;
            buflen +=4;
        break;
        case CONFIG_PCB249U2:

            // Invia il blocco 0. Sulle risposte verranno inviati i blocchi successivi.
            pData[0] = pCollimatore->colliConf.filterPos[0];
            pData[1] = pCollimatore->colliConf.filterPos[1];
            pData[2] = pCollimatore->colliConf.filterPos[2];
            pData[3] = pCollimatore->colliConf.filterPos[3];
            pData[4] = pCollimatore->colliConf.filterTomo[0]; // Hotfix 11C
            pData[5] = pCollimatore->colliConf.filterTomo[1];
            pData[6] = pCollimatore->colliConf.filterTomo[2];
            pData[7] = pCollimatore->colliConf.filterTomo[3];
            if(pCollimatore->colli_model == _COLLI_TYPE_ASSY_01){
                pData[8] = (unsigned char) pCollimatore->colliConf.mirrorSteps_ASSY_01;
                pData[9] = (unsigned char) (pCollimatore->colliConf.mirrorSteps_ASSY_01>>8);
            }else{
                pData[8] = (unsigned char) pCollimatore->colliConf.mirrorSteps_ASSY_02;
                pData[9] = (unsigned char) (pCollimatore->colliConf.mirrorSteps_ASSY_02>>8);
            }
            buflen += 10;
        break;
        case CONFIG_BIOPSY:
            /*
                buffer[0]: offsetFibra
                buffer[1]: offsetPad
                buffer[2]: margine risalita compressore
                buffer[3]: margine posizionamento
            */
            pData[0] = pBiopsy->config.offsetFibra;
            pData[1] = pBiopsy->config.offsetPad;
            pData[2] = pBiopsy->config.margineRisalita;
            pData[3] = pBiopsy->config.marginePosizionamento;
            buflen += 4;
        break;
        case CONFIG_PCB244:           
            pData[0] = 0;
            buflen += 1;
        break;
        case CONFIG_ARM:
            memcpy(pData, (unsigned char*) &armConfig, sizeof(armConfig_Str));
            buflen += sizeof(armConfig_Str);
        break;
        case CONFIG_TRX:
           memcpy(pData, (unsigned char*) &trxConfig, sizeof(trxConfig_Str));
           buflen += sizeof(trxConfig_Str);
        break;
        case CONFIG_LENZE:
            memcpy(pData, (unsigned char*) &lenzeConfig, sizeof(lenzeConfig_Str));
            buflen += sizeof(lenzeConfig_Str);
        break;

        case CONFIG_GANTRY:
            memcpy(pData, (unsigned char*) &sys, sizeof(sys));
            buflen += sizeof(sys);
        break;

        case CONFIG_GENERAL:
            if(userCnf.demoMode) pData[0] = 1;
            else pData[0] = 0;
            if(userCnf.audioEnable){
                pData[1] = 1;
                pData[2] = userCnf.volumeAudio;
            }else {
                pData[1] = 0;
                pData[2] = 0;
            }

            buflen += 3;
        break;

        default:
        break;
    }

    return pConsole->pGuiMcc->sendFrame(MCC_CONFIG,1,buffer, buflen);
}


void Config::timerEvent(QTimerEvent* ev)
{

    // Start Install
    if(ev->timerId()==timerInstall){
        killTimer(timerInstall);
        sysUpdate(swConf.installedPackageName);
        return;
    }else if(ev->timerId()==timerReboot){
        rebootSlot();
        return;
    }else if(ev->timerId()==timerPoweroff){
        powerOffSlot();
        return;
    }else if(ev->timerId()==timerXrayPush){
        killTimer(timerXrayPush);
        timerXrayPush=0;

        if(ApplicationDatabase.getDataU(_DB_READY_EXPOSURE)){
            timerXrayPush=startTimer(90000);
        }else  PageAlarms::activateNewAlarm(_DB_ALLARME_XRAY_PUSH,XRAY_PUSH_TIMEOUT);

        return;
    }

}

/* Nuova istallazione del package */
void Config::onInstallPackage(void)
{

    // Verifica se esite il package da istallare
    QString stringa1 = QString("INSTALLING PACKAGE:%1").arg(swConf.rvPackage);
    QString stringa =  QString("Verify PACKAGE file ...");
    pWarningBox->activate(stringa1,stringa,100,0);
    pWarningBox->setTextAlignment(Qt::AlignLeft);

    connect(this,SIGNAL(sysUpdateCompletedSgn(bool,QString)),this,SLOT(localInstallMonitorCompleted(bool,QString)),Qt::QueuedConnection);
    timerInstall = startTimer(0);
    return;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 *  CERCA LA PROSSIMA RIGA DI CONFIGURAZIONE VALIDA
 *
 *
 */
QByteArray Config::getNextValidLine(QFile* fp)
{
    QByteArray risultato, frame;
    int i;

    // Scansione righe del file
    while(!fp->atEnd())
    {
        frame = fp->readLine(); // Lettura riga
        risultato.clear();

        // Se primo caratter ==  # salta la linea
        if(frame.at(0)=='#') continue;

        // Ricerca carattere '<'
        for(i=0; i<frame.size(); i++)  if(frame.at(i)=='<') break;
        if(i==frame.size()) continue; // Nessun carattere trovato, passa a nuova riga
        i++; // i contiene il primo carattere valido

        // Costruzione dato fino a  '>' o a ','
        for(;i<frame.size(); i++)
        {
            if(frame.at(i)=='>') break;
            else risultato.append(frame.at(i));
        }
        if(i==frame.size()) continue; // Nessun carattere trovato, passa a nuova riga
        if(risultato.size()==0) continue;
        // Risultato contiene la riga valida
        return risultato;
    }

    // Nessuna nuova riga trovata
    risultato.clear();
    return risultato;
}

QByteArray Config::getNextParam(QByteArray* dato, int* index, bool last)
{
    QByteArray risultato;
    int i;

    risultato.clear();
    i=(*index);

    // Legge primo dato fino alla virgola
    risultato.clear();

    for(;i<dato->size();i++)
    {
        if(dato->at(i)==',') break;
        risultato.append(dato->at(i));
    }
    (*index)=i;

    if(last) return risultato;

    if(i==dato->size())
    {
        risultato.clear();
        return risultato;
    }
    i++;
    (*index)=i;
    return risultato;

}

QByteArray Config::getNextTag(QByteArray* dato, int* index, bool last)
{
    QByteArray risultato;
    int i;

    risultato.clear();
    i=(*index);

    // Legge primo dato fino a '"'
    risultato.clear();

    for(;i<dato->size();i++) if(dato->at(i)=='"') break;
    if(i==dato->size()) return risultato;
    i++;
    for(;i<dato->size();i++)
    {
        if(dato->at(i)==',')
        {
            risultato.clear();
            return risultato;
        }
        if(dato->at(i)=='"') break;
        risultato.append(dato->at(i));
    }
    if(dato->at(i)!='"')
    {
        risultato.clear();
        return risultato;
    }

    (*index)=i;
    if(last) return risultato;
    i++;

    // Cerca il primo dato dopo la virgola
    for(;i<dato->size();i++)
    {
        if(dato->at(i)==',')
        {
            (*index)=++i;
            return risultato;
        }
    }

    risultato.clear();
    return risultato;
}

// Array generico di caratteri
QByteArray Config::getNextArrayLine(QFile* fp)
{
    int i,j;
    QByteArray dato;
    QByteArray risultato;

    while(!fp->atEnd())
    {
        j=0;
        dato.clear();
        QByteArray frame = fp->readLine();

        // Inizio sequenza: carattere '<'
        for(i=0; i<frame.size(); i++)
        {
            // Elimina gli spazi vuoti
            if(frame.at(i)=='<') break;
        }
        if(i==frame.size()) continue;
        i++; // i contiene il primo carattere valido

        // Costruzione dato fino a  '>' o a ','
        for(;i<frame.size(); i++)
        {
            if(frame.at(i)=='>')
            {
                // legge ultimo dato
                if(dato.size()==0) risultato.clear(); // Errore
                else risultato.append(dato.toShort());
                return risultato;
            }
            if(frame.at(i)==',')
            {
                // aggiorna array
                if(dato.size()==0)
                {
                    risultato.clear(); // Errore
                    return risultato;
                }
                else risultato.append(dato.toShort());
                dato.clear();
                continue;
            }

            // Trattasi di altro carattere del numero
            dato.append(frame.at(i));
        }
        risultato.clear(); // Errore
        return risultato;
    }

    risultato.clear(); // Errore
    return risultato;

}

// Scrive il file destinazione con i dati del file sorgente aggiungendo i commenti
// Questa funzione deve essere utilizzata solo per cami obbligatori e non opzionali
// Per i file descritti con i TAG opzionali non √® possibile utilizzare la funzione
void Config::writeNextArrayLine(QFile* filedest, QFile* filesrc,int* array, int len)
{
    int i;
    QByteArray dato;
    QByteArray commento;


    while(!filesrc->atEnd())
    {
        dato.clear();
        commento.clear();
        QByteArray frame = filesrc->readLine();

        // Inizio sequenza: carattere '<'
        for(i=0; i<frame.size(); i++)
        {
            // Elimina gli spazi vuoti
            if(frame.at(i)=='<') break;
        }
        if(i==frame.size())
        {
            // LInea non valida: copia il contenuto in destinazione
            filedest->write(frame);
            continue;
        }
        i++; // i contiene il primo carattere valido

        // Costruzione dato fino a  '>' o a ','
        for(;i<frame.size(); i++)
        {
            if(frame.at(i)=='>')
            {
                // Copia il commento
                commento.append(frame.right(frame.size()-i-1));
                goto scrive_dato;
            }
        }
        if(i==frame.size())
        {
            // LInea non valida: copia il contenuto in destinazione
            filedest->write(frame);
            continue;
        }
    }
    return ;

    scrive_dato:
    // Qui si pu√≤ scrivere il nuovo dato ed il commento a seguire
    dato.clear();
    dato.append('<');
    for(i=0; i<len; i++)
    {
        dato.append(QString("%1").arg(array[i]));
        if(i<len-1) dato.append(',');
    }
    dato.append('>');
    dato.append(commento);

    filedest->write(dato);
    return;
}

/*
 *  Questa funzione effettua esclusivamente la scrittura di una stringa
 *  di parametri all'interno degli apici <>. Questa funzione deve essere
 *  utilizzata per la scrittura di parametri non sequenziali
 */
void Config::writeNextStringLine(QFile* filedest,QString stringa)
{
    filedest->write(stringa.prepend("<").append(">\n").toAscii());
}

/*
 *  Questo file legge tutte le prossime righe fino a trovare il TAG richiesto
 *  Serve per allineare un file dopo la scrittura di parametri non sequenziali
 */
bool Config::alignFileWithTag(QFile* file,QString tag)
{
    while(!file->atEnd())
    {
        QByteArray frame = file->readLine();
        if(frame.contains(tag.toAscii())) return TRUE;
    }
    return FALSE;

}

/*
 *  Copia nel file destinazione tutti i commenti del file sorgente fino al primo campo valido
 *
 */
QByteArray Config::alignFileWithValidField(QFile* filedest, QFile* filesrc)
{    
    QByteArray frame;


    while(!filesrc->atEnd())
    {
        frame = filesrc->readLine();
        if((frame.contains("<")) && (frame.contains(">"))) return frame;
        filedest->write(frame);
    }

    // Fine file
    frame.clear();
    return frame;
}

/* Scrive il file destinazione con i dati del file sorgente
 *
 *  La funzione cerca la prossima riga con un dato valido nel formato <dato>
 *  e sostituisce dato con stringa. La funziona copia dal file sorgente tutte
 *  le linee non formattate nel file destinazione, compreso i caratteri di commento
 *  a seguito del campo formattato.
 *
 *  ATTENZIONE: L'uso di questa funzione deve tenere in conto che il file sorgente
 *  e il file destinazione devono essere allineabili rispetto al campo dati
 *  da modificare. Se il file in oggetto presenta dei campi opzionali occorrer√ 
 *  utilizzare l'apposita funzione di allineamento a TAG.
 */
void Config::writeNextStringLine(QFile* filedest, QFile* filesrc,QString stringa)
{
    int i;
    QByteArray dato;
    QByteArray commento;


    while(!filesrc->atEnd())
    {
        dato.clear();
        commento.clear();
        QByteArray frame = filesrc->readLine();

        // Inizio sequenza: carattere '<'
        for(i=0; i<frame.size(); i++)
        {
            // Elimina gli spazi vuoti
            if(frame.at(i)=='<') break;
        }
        if(i==frame.size())
        {
            // LInea non valida: copia il contenuto in destinazione
            filedest->write(frame);
            continue;
        }
        i++;
        // i contiene carattere dopo <

        // Costruzione dato fino a  '>'
        for(;i<frame.size(); i++)
        {
            if(frame.at(i)=='>')
            {
                // Crea il dato valido
                stringa.prepend("<");
                stringa.append(">");
                stringa.append(frame.right(frame.size()-i-1));
                filedest->write(stringa.toAscii());
                return;
            }
        }
        if(i==frame.size())
        {
            // LInea non valida: copia il contenuto in destinazione
            filedest->write(frame);
            continue;
        }
    }
    return ;


}

// Aggiunge tutti i registri del database che possono richiedere un aggiornamento verso lo slave
// Aggiungere solo quelli che sono impostati alla partenza e che rischiano di non essere sincronizzati
void Config::masterUpdateDatabase(void){
    ApplicationDatabase.setData(_DB_AWS_CONNECTION, ApplicationDatabase.getDataU(_DB_AWS_CONNECTION),DBase::_DB_FORCE_SGN|DBase::_DB_ONLY_MASTER_ACTION);
}

/////////////////////// HANDLER DI RICEZIONE RISPOSTE DA SLAVE /////////////////////////////////////////////////////////
void Config::configMasterRxHandler(QByteArray frame)
{
    static int nBlocchi=0;
    protoConsole protocollo(&frame);
    if(protocollo.isValid==FALSE) return;
    QString comando = protocollo.comando;


    if(comando==FTP_BLOCK){
        protoConsole blocco(1,false);

        // Verifica se il trasferimento √® completo
        if(ftpFile.atEnd()){
            // Invia il comando di fine transazione a Slave
            emit configMasterTx( blocco.cmdToQByteArray(FTP_END));
            ftpFile.close();
            return;
        }

        nBlocchi++;
        if((nBlocchi%70)==0)
            pWarningBox->setSuffix(QString("TRANSFER TO SLAVE: %1%").arg(nBlocchi*100/7000));

        // Invia il blocco su protocollo FTP
        emit configMasterTx( blocco.createFtpArray(ftpFile.read(MAX_BLOCCO_FTP)));
        return;

    }else if(comando==FTP_ERR){
        // Errore: viene notificato
        ftpFile.close();
        emit ftpSgn((int) protocollo.parametri[0].toInt());
        return;
    }else if(comando==FTP_END){


        emit ftpSgn((int) 0); // Tutto OK
        return;
    }else if(comando==SLAVE_EXTRACT_ARCHIVE){
        slaveUpdated();
        return;
    }else if(comando==SLAVE_EXECUTE_BACKUP){
        // Lo Slave ha terminato l'esecuzione del comando di backup.
        // Se l'ID!=0 allora risponde alla consolle che aveva riciesto il servizio di backup
        if(protocollo.parametri.size()==0) return;
        if(protocollo.parametri[0].toInt()==0) return;

        // Risponde alla consolle che attende l'esito dell'operazione
        pToConsole->endCommandAck(protocollo.parametri[0].toInt(),0);
    }else if(comando==SLAVE_EXECUTE_RESTORE){
        // Lo Slave ha terminato l'esecuzione del comando di restore.
        // Se l'ID!=0 allora risponde alla consolle che aveva riciesto il servizio di backup
        if(protocollo.parametri.size()==0) return;
        if(protocollo.parametri[0].toInt()==0) return;

        // Risponde alla consolle che attende l'esito dell'operazione
        pToConsole->endCommandAck(protocollo.parametri[0].toInt(),0);
    }else if(comando==SLAVE_EXECUTE_CLEAN){
        // Lo Slave ha terminato l'esecuzione del comando di restore.
        // Se l'ID!=0 allora risponde alla consolle che aveva riciesto il servizio di backup
        if(protocollo.parametri.size()==0) return;
        if(protocollo.parametri[0].toInt()==0) return;

        // Risponde alla consolle che attende l'esito dell'operazione
        pToConsole->endCommandAck(protocollo.parametri[0].toInt(),0);
    }else if(comando==SLAVE_EXECUTE_SHELL){

        QByteArray comando = "";
        for(int i=0; i<protocollo.parametri.size(); i++){
            comando.append(protocollo.parametri[i]).append(" ");
        }
        emit sgnRemoteShell(comando);
    }else if(comando==SYNC_TO_SLAVE){
        slaveDataInitialized = true;

        // Impostazione database dipendente da Master
        masterUpdateDatabase();
    }
    return;
}

void Config::slaveExtractArchive(void){
    protoConsole blocco(1,false);
    emit configMasterTx( blocco.cmdToQByteArray(SLAVE_EXTRACT_ARCHIVE));
    return;
}

// Inizia il trasferimento
bool Config::fileTransfer(QString origine, QString destinazione){

    // Apertura file
    ftpFile.setFileName(origine);
    if(ftpFile.exists()==false) return false;
    if (!ftpFile.open(QIODevice::ReadOnly)) return false;

    connect(this,SIGNAL(ftpSgn(int)),this,SLOT(ftpPrintMsg(int)),Qt::UniqueConnection);
    protoConsole blocco(1,false);
    blocco.addParam(destinazione);
    emit configMasterTx( blocco.cmdToQByteArray(FTP_START));
    return true;
}

void Config::ftpPrintMsg(int val){

    switch(val){
    //case 0: qDebug() << "FTP: TRASFERIMENTO COMPLETATO\n"; break;
    case 1: qDebug() << "FTP ERROR: IMPOSSIBILE CREARE FILE DESTINAZIONE\n"; break;
    case 2: qDebug() << "FTP ERROR: PROTOCOLLO NON ABILITATO\n"; break;
    case 3: qDebug() << "FTP ERROR: CRC NON CORRETTO\n"; break;
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                  SEZIONE RELATIVA AL CONFIGURATORE PER IL TERMINALE SLAVE


void Config::configSlaveRxHandler(QByteArray frame)
{
    protoConsole protocollo(&frame);
    protoConsole answ(protocollo.id,false);

    // PROTOCOLLO FTP RICONOSCIUTO DURANTE UNA TRANSAZIONE FTP
    if(protocollo.isFtpProtocol==TRUE)
    {
        // Non era attiva la modalit√  FTP
        if(ftpEnabled==false)
        {
            answ.addParam(QString("%1").arg(ERR_FTP_NOT_ENABLED));
            emit configSlaveTx(answ.cmdToQByteArray(FTP_ERR));
            return;
        }
        if(protocollo.errorCrc==true)
        {
            answ.addParam(QString("%1").arg(ERR_FTP_WRONG_CRC));
            emit configSlaveTx(answ.cmdToQByteArray(FTP_ERR));
            ftpFile.close();
            ftpEnabled=false;
            return;
        }

        // Scrive il file
        ftpFile.write(protocollo.ftp);
        emit configSlaveTx(answ.cmdToQByteArray(FTP_BLOCK)); // Feedback a Master

        return;

    }

    // Sezione protocollo ascii
    if(protocollo.isValid==FALSE) return;

    QString comando = protocollo.comando;
    unsigned char data[2];

    if(comando==MASTER_EXIT_PROGRAM)
    {
        exit(0);
    }else if(comando==FTP_START)
    {
        // Crea il file che deve essere
        this->pendingFileTransfer = protocollo.parametri[0];
        ftpFile.setFileName(protocollo.parametri[0]);
        if (!ftpFile.open(QIODevice::WriteOnly))
        {
            // Errore creazione file
            answ.addParam(QString("%1").arg(ERR_CREATING_FTP_DESTINATION_FILE));
            emit configSlaveTx(answ.cmdToQByteArray(FTP_ERR));
            ftpEnabled = false;
            return;
        }

        ftpEnabled = true;
        emit configSlaveTx(answ.cmdToQByteArray(FTP_BLOCK)); // Feedback OK per inizio FTP
        return;

    }else if(comando==FTP_END){

        // Termine della transazione FTP
        ftpEnabled = false;
        ftpFile.close();
        QString command = QString("sync");
        system(command.toStdString().c_str());

        emit configSlaveTx(answ.cmdToQByteArray(FTP_END)); // Ack
        return;

    }else if(comando==SLAVE_EXTRACT_ARCHIVE){

        // Preparazione per l'istallazione
        QString command = QString("mkdir %1").arg(INSTALL_DIR);
        system(command.toStdString().c_str());
        command = QString("rm %1/*").arg(INSTALL_DIR);
        system(command.toStdString().c_str());
        command = QString("mv %1 %2/").arg(SLAVE_TAR_PACKAGE).arg(INSTALL_DIR);
        system(command.toStdString().c_str());

        // Effettua il sync per consolidare i files
        command = QString("sync");
        system(command.toStdString().c_str());

        // OK
        emit configSlaveTx(answ.cmdToQByteArray(SLAVE_EXTRACT_ARCHIVE)); // Feedback OK per estrazione archivio su slave
    }else if(comando==SLAVE_EXECUTE_BACKUP){
        if(protocollo.parametri.size()!=2) return;

        // Lo Slave riceve su par0 l'ID e su par1 il nome del file
        sysBackup(FALSE,protocollo.parametri[1],0);

        // Risponde al master
        answ.addParam(protocollo.parametri[0]);
        answ.addParam(protocollo.parametri[1]);
        emit configSlaveTx(answ.cmdToQByteArray(SLAVE_EXECUTE_BACKUP));
    }else if(comando==SLAVE_EXECUTE_RESTORE){
        if(protocollo.parametri.size()!=2) return;

        // Lo Slave riceve su par0 l'ID e su par1 il nome del file
        sysRestore(FALSE,protocollo.parametri[1],0);

        // Risponde al master
        answ.addParam(protocollo.parametri[0]);
        answ.addParam(protocollo.parametri[1]);
        emit configSlaveTx(answ.cmdToQByteArray(SLAVE_EXECUTE_RESTORE));
    }else if(comando==SLAVE_EXECUTE_CLEAN){
        if(protocollo.parametri.size()!=1) return;

        // Lo Slave riceve su par0 l'ID e su par1 il nome del file
        userDirClean(FALSE,0);

        // Risponde al master
        answ.addParam(protocollo.parametri[0]);
        emit configSlaveTx(answ.cmdToQByteArray(SLAVE_EXECUTE_CLEAN));
    }else if(comando==SLAVE_EXECUTE_REBOOT){

        rebootSlot();

    }else if(comando==SLAVE_EXECUTE_PC_POWEROFF){


        if(protocollo.parametri.size()!=1) data[0]=20;
        else data[0] = (unsigned char) protocollo.parametri[0].toInt();

        awsPowerOff(); // Comando al client a basso livello del PC, se connesso

        if(pSlaveMcc->sendFrame(MCC_POWER_OFF,1,data,1)==false){
            PRINT("FALLITO MCC POWER OFF!");
        }

        // Attiva il timer per il reboot locale
        timerPoweroff = startTimer(5000);
        PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_SOFT,WARNIN_SPEGNIMENTO);

    }else if(comando==SLAVE_EXECUTE_POWEROFF){

        if(protocollo.parametri.size()!=1) data[0]=20;
        else data[0] = (unsigned char) protocollo.parametri[0].toInt();

        if(pSlaveMcc->sendFrame(MCC_POWER_OFF,1,data,1)==false){
            PRINT("FALLITO MCC POWER OFF!");
        }

        // Attiva il timer per il reboot locale
        timerPoweroff = startTimer(5000);
        PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_SOFT,WARNIN_SPEGNIMENTO);

    }else if(comando==SLAVE_EXECUTE_UPDATE_IDE){

        QString command = QString("/monta.sh");
        system(command.toStdString().c_str());
        command = QString("cp /mnt/nfs/m4_slave.bin /");
        system(command.toStdString().c_str());
        command = QString("cp /mnt/target/DBTController /");
        system(command.toStdString().c_str());
        command = QString("sync");
        system(command.toStdString().c_str());

        // Reboot della macchina
        command = QString("reboot -f");
        system(command.toStdString().c_str());
    }else if(comando==SLAVE_EXECUTE_UPDATE_GUI){

        QString command = QString("/monta.sh");
        system(command.toStdString().c_str());
        command = QString("cp /mnt/target/DBTController /");
        system(command.toStdString().c_str());
        command = QString("sync");
        system(command.toStdString().c_str());

        // Reboot della macchina
        command = QString("reboot -f");
        system(command.toStdString().c_str());
    }else if(comando==SLAVE_EXECUTE_SHELL){
        QByteArray comando = "";
        for(int i=0; i<protocollo.parametri.size(); i++){
            comando.append(protocollo.parametri[i]).append(" ");
        }

        QByteArray risposta = QByteArray(SLAVE_EXECUTE_SHELL).append(" ").append(pConfig->executeShell(answ.unformatData(comando)));
        emit configSlaveTx(answ.cmdToQByteArray(risposta));
    }else if(comando==SYNC_TO_SLAVE){
        slaveInitialization();
        if(slaveDataInitialized)  emit configSlaveTx(answ.cmdToQByteArray(SYNC_TO_SLAVE));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                  SEZIONE RELATIVA ALLA RICEZIONE DA MCC M4 SLAVE SU GUI SLAVE
void mccSlaveCom::mccRxHandler(_MccFrame_Str mccframe)
{
 int ciclo;
 QByteArray bufdata;

 switch(mccframe.cmd)
  {
    case MCC_GUI_NOTIFY: // Notifica da M4 SLAVE

     bufdata.clear();
     for(ciclo=0; ciclo<mccframe.buffer[1]; ciclo++ )
         bufdata.append(mccframe.buffer[2+ciclo]);
     pConfig->emitMccSlaveNotify(mccframe.id, mccframe.buffer[0],bufdata);
     break;

 default:
     break;

 }
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
 * Comando generaledi attivazione aggiornamento software
 * di tutta la macchina.
 * @Param package   identifica il package da istallare
 * @Param errstr    identifica il puntatore ad una stringa errore in caso di fallimento
 * RETURN
 *  true -> Aggiornamento attivato con successo;
 *  false -> Aggiornamento fallito (vedi errstr)
 *
 *  Al termine dell'aggiornamento verr√  generata una signal
 *  a cui il chiamante dovr√  essersi connesso per rilevare
 *  il completqamento dell'aggiornamento.
 *  La signal in oggetto √®:
 *      void sysUpdateCompletedSgn(bool esito, QString errstr);
 *      - esito = risultato dell'aggiornamento
 *      - errstr = stringa di errore in caso di esito negativo
 *
 */
// TO BE DONE: visualizzazione stato aggiornamento
void Config::sysUpdate(QString package){
    QString command;
    QString pkgname = package;
    master_updated = false;
    slave_updated=false;
    firmware_updated = false;
    updErrStr.clear();
    updError = false;


    // Prima di procedere effettua un sync
    // Scompatta il file contenente i firmwares
    command = QString("sync");
    system(command.toStdString().c_str());

    // Aggiunge la Home
    package.prepend(FILE_HOME);

    // Verifica il package da estrarre
    QFile file(package.toStdString().c_str());
    if(file.exists()==false){
        emit sysUpdateCompletedSgn(false,QString("Loader: ERR %1, File:%2").arg(PACKAGE_INESISTENTE).arg(package));
        return;
    }

    // Estrazione del package in HOME
    command = QString("tar -C %1 -xf %2").arg(FILE_HOME).arg(package);
    system(command.toStdString().c_str());


    // Verifica se esiste un pacchetto di aggiornamento versioni
    file.setFileName(FILE_INDICE);
    if(file.exists()==false){
        emit sysUpdateCompletedSgn(false,QString("Loader: ERR %1, File:%2").arg(FILE_INDICE_INESISTENTE).arg(FILE_INDICE));
        return;
    }


    // Carica il nuovo file indice
    firmwareCfg_Str swNew;
    if(openPackageCfg(FILE_INDICE, &swNew) == false){
        emit sysUpdateCompletedSgn(false,QString("Loader: ERR %1, File:%2").arg(FORMATO_INDICE_ERRATO).arg(FILE_INDICE));
        return;
    }


    // Istallazione aggiornamento MASTER se presente
    file.setFileName(MASTER_TAR_PACKAGE);
    if(file.exists()==true){

        // Preparazione per l'istallazione
        QString command = QString("mkdir %1").arg(INSTALL_DIR);
        system(command.toStdString().c_str());
        command = QString("rm %1/*").arg(INSTALL_DIR);
        system(command.toStdString().c_str());
        master_updated = true;

        // Invia il pacchetto allo SLAVE
        connect(this,SIGNAL(ftpSgn(int)),this,SLOT(ftpSlavePackageCompleted(int)),Qt::UniqueConnection);
        fileTransfer(SLAVE_TAR_PACKAGE,SLAVE_TAR_PACKAGE);

    }else{
        slave_updated = true;
        master_updated = true;
    }

    // Aggiorna il contenuto del package
    swNew.installedPackageName = pkgname; // Salva il nome del package per essere utilizzato successivamente
    swConf = swNew;
    savePackageCfg(FIRMWARE_FILE,swConf);

    // Scompatta il file contenente i firmwares
    command = QString("tar -C %1 -xf %2").arg(FILE_HOME).arg(PACKAGE_FIRMWARE);
    system(command.toStdString().c_str());

    connect(pLoader,SIGNAL(loaderCompletedSgn(bool,QString)), this, SLOT(loaderCompletedSlot(bool, QString)),Qt::UniqueConnection);
    pLoader->firmwareUpdate();


    // Se il sistema ha completato l'aggiornamento viene lanciata la signal di termine
    if(isSystemUpdated())
    {
        emit sysUpdateCompletedSgn(!updError, updErrStr);
    }



}

// Visualizzazione stato aggiornamento
void Config::localInstallMonitorCompleted(bool esito, QString errstr){
    QString stringa;

    // Effettua il sync per consolidare i files
    QString command = QString("sync");
    system(command.toStdString().c_str());

    disconnect(this,SIGNAL(sysUpdateCompletedSgn(bool,QString)),this,SLOT(localInstallMonitorCompleted(bool,QString)));

    QString stringa1 = QString("INSTALL PROCESS, PACKAGE:%1").arg(swConf.installedPackageName);
    if(esito)   stringa =  QString("INSTALLATION COMPLETED SUCCESSFULLY\nRESTART THE SYSTEM TO COMPLETE INSTALLATION");
    else {
        stringa = QString("INSTALLATION ERROR:\n");
        stringa.append(errstr);
    }

    connect(pWarningBox,SIGNAL(buttonOkSgn(void)),this,SLOT(onOkCompletedInstallPackage(void)),Qt::QueuedConnection);
    pWarningBox->activate(stringa1,stringa,100,msgBox::_BUTTON_OK);
    pWarningBox->setTextAlignment(Qt::AlignLeft);
    return;
}

// Alla pressione del pulsante OK al termine del processo, il sistema effettua il reboot
void Config::onOkCompletedInstallPackage(void){
    //executeReboot();
}


/**
 * @brief Config::sysBackup
 * @param isMaster
 * @param filename
 * @param id    indica l'ID del messaggio della console se il comando viene dalla consolle
 */
void Config::sysBackup(bool isMaster,QString filename, int id){
QString command;

    if(isMaster){

        // Comando di archiviazione per il master
        command = QString("tar -cf %1master_%2.tar resource DBTController m4_master.bin dbtm").arg(FILE_HOME).arg(filename);
        system(command.toStdString().c_str());

//        command = QString("sync");
//        system(command.toStdString().c_str());

        // Invia comando a slave
        protoConsole frame(1,false);
        frame.addParam(QString("%1").arg(id));
        frame.addParam(filename);
        emit configMasterTx( frame.cmdToQByteArray(SLAVE_EXECUTE_BACKUP));
    }else{
        // Comando di archiviazione per slave
        command = QString("tar -cf %1slave_%2.tar resource DBTController m4_slave.bin dbts").arg(FILE_HOME).arg(filename);
        system(command.toStdString().c_str());

  //      command = QString("sync");
  //      system(command.toStdString().c_str());

    }

}


void Config::userDirClean(bool isMaster, int id){
QString command;

    if(isMaster){

        command = QString("rm %1*.*").arg(FILE_HOME);
        system(command.toStdString().c_str());

        command = QString("rm %1*").arg(FILE_HOME);
        system(command.toStdString().c_str());

//        command = QString("sync");
//        system(command.toStdString().c_str());

        // Invia comando a slave
        protoConsole frame(1,false);
        frame.addParam(QString("%1").arg(id));
        emit configMasterTx( frame.cmdToQByteArray(SLAVE_EXECUTE_CLEAN));
    }else{

        command = QString("rm %1*.*").arg(FILE_HOME);
        system(command.toStdString().c_str());

        command = QString("rm %1*").arg(FILE_HOME);
        system(command.toStdString().c_str());

//        command = QString("sync");
//        system(command.toStdString().c_str());

    }

}
void Config::sysRestore(bool isMaster,QString filename, int id){
    QString command;

    if(isMaster){
        // Comando di archiviazione per il master
        command = QString("tar -C / -xf %1master_%2.tar").arg(FILE_HOME).arg(filename);
        system(command.toStdString().c_str());

        // Cancellazione del file dalla home
        command = QString("rm %1master_%2.tar").arg(FILE_HOME).arg(filename);
        system(command.toStdString().c_str());

        // Invia comando a slave
        protoConsole frame(1,false);
        frame.addParam(QString("%1").arg(id));
        frame.addParam(filename);
        emit configMasterTx( frame.cmdToQByteArray(SLAVE_EXECUTE_RESTORE));
    }else{
        // Comando di archiviazione per slave
        command = QString("tar -C / -xf %1slave_%2.tar").arg(FILE_HOME).arg(filename);
        system(command.toStdString().c_str());

        // Cancellazione del file dalla home
        command = QString("rm %1slave_%2.tar").arg(FILE_HOME).arg(filename);
        system(command.toStdString().c_str());

    }


}


// Slot associato alla signal di fine operazioni da parte del loader
void Config::loaderCompletedSlot(bool esito, QString errstr){

    firmware_updated = true;
    if(esito==false){
        // Nel caso sia stato rilevato qualche errore, viene aggiunta la stringa.
        updError = true;
        updErrStr.append("\n").append(errstr);
    }

    // Se il sistema ha completato l'aggiornamento viene lanciata la signal di termine
    if(isSystemUpdated()) emit sysUpdateCompletedSgn(!updError, updErrStr);
}

// Slot associato alla fine dell'FTP verso slave del package
void Config::ftpSlavePackageCompleted(int esito){

    // Errore
    QString errstr;

    switch(esito){
        case 0:  slaveExtractArchive(); return; break;
        case 1: errstr = QString("Loader: ERR %1, Unable to create file").arg(FILE_TRANSFER_ERR);break;
        case 2: errstr = QString("Loader: ERR %1, Master to Slave transfer disabled").arg(FILE_TRANSFER_ERR); break;
        case 3: errstr = QString("Loader: ERR %1, Invalid checksum detected").arg(FILE_TRANSFER_ERR);break;
    }

    updErrStr.append("\n").append(errstr);
    updError = true;
    slave_updated = true;

    if(isSystemUpdated()) emit sysUpdateCompletedSgn(!updError, updErrStr);
    return;
}

// Feedback da Slave che ha completato l'estrazione del package
void Config::slaveUpdated(void){
    slave_updated = true;
    if(isSystemUpdated()) emit sysUpdateCompletedSgn(!updError, updErrStr);
    return;
}

// Slot di ricezione Notifiche MCC dal modulo Slave
void Config::slaveNotifySlot(unsigned char id,unsigned char mcc_code,QByteArray data)
{
    int vbat;
    // Questa parte √® la risposta ad una interrogazione. Al momento
    // l'interrogazione non viene effettuata perch√® richiede un nuovo codice di errore
    // e un atento incastro con il loader. Si rimanda alla prossima PDV
    if(mcc_code==MCC_POWER_OFF){

        //powerOffSlot();

    }else if(mcc_code==MCC_SLAVE_ERRORS){
        if(data.at(0)==SLAVE_ERROR_LIFT_PEDALS) PageAlarms::activateNewAlarm(_DB_ALLARME_LIFT_PUSH,LIFT_INVALID_PUSH);
        else PageAlarms::activateNewAlarm(_DB_ALLARME_LIFT_PUSH,SLAVE_ERROR_NONE);
    }else if(mcc_code==MCC_LENZE_ERRORS){
        /*
        buffer[0]= event_code;
        buffer[1]= (unsigned char) event_data;
        buffer[2]= (unsigned char) (event_data>>8);
        buffer[3]= (unsigned char) (event_data>>16);
        buffer[4]= (unsigned char) (event_data>>24);
        */
        PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_LENZE,data.at(0));

    }else if(mcc_code==MCC_POWER_MANAGEMENT){


        // Assegna lo stato corrente della potenza, ad uso generale
        ApplicationDatabase.setData(_DB_POWER_STAT,data.at(PWRMANAGEMENT_STAT),0);

        switch((int) data.at(PWRMANAGEMENT_STAT)){
        case PWRMANAGEMENT_STAT_OK:
            PageAlarms::activateNewAlarm(_DB_ALLARMI_POWERDOWN,0); // reset alarm            
            ApplicationDatabase.setData(_DB_ACVOLT,(int) (data.at(PWRMANAGEMENT_VLENZE_L)+256*data.at(PWRMANAGEMENT_VLENZE_H)),0);

            // Viene visualizzato il valore medio della percentuale della carica delle due batterie
            vbat = (data.at(PWRMANAGEMENT_VBAT1) + data.at(PWRMANAGEMENT_VBAT2))/2;
            ApplicationDatabase.setData(_DB_BATTCHARGE,(unsigned char) vbat,0);

        break;
        case PWRMANAGEMENT_STAT_POWERDOWN:
            PageAlarms::activateNewAlarm(_DB_ALLARMI_POWERDOWN,ERROR_POWER_DOWN_MAINS);
            ApplicationDatabase.setData(_DB_ACVOLT,0,0);
        break;
        case PWRMANAGEMENT_STAT_EMERGENCY:
            PageAlarms::activateNewAlarm(_DB_ALLARMI_POWERDOWN,ERROR_POWER_DOWN_EMERGENCY_BUTTON);
        break;
        case PWRMANAGEMENT_STAT_FUSE_TF155:
            PageAlarms::activateNewAlarm(_DB_ALLARMI_POWERDOWN,ERROR_POWER_DOWN_TF155_FUSE);
            ApplicationDatabase.setData(_DB_ACVOLT,(int) (data.at(PWRMANAGEMENT_VLENZE_L)+256*data.at(PWRMANAGEMENT_VLENZE_H)),0);
        break;
        case PWRMANAGEMENT_STAT_BLITERS_ON:
            PageAlarms::activateNewAlarm(_DB_ALLARMI_POWERDOWN,ERROR_POWER_DOWN_WARNING_BLITERS_ON);
            ApplicationDatabase.setData(_DB_ACVOLT,(int) (data.at(PWRMANAGEMENT_VLENZE_L)+256*data.at(PWRMANAGEMENT_VLENZE_H)),0);
        break;

        }

        // Stato actuators
        ApplicationDatabase.setData(_DB_ENABLE_MOVIMENTI,(unsigned char) data.at(PWRMANAGEMENT_ACTUATOR_STATUS),0);

        for(int i=0; i<sizeof(_SystemInputs_Str); i++) ((unsigned char*) &io->inputs)[i] = (unsigned char) data.at(PWRMANAGEMENT_ACTUATOR_IO+i);
        for(int i=0; i<sizeof(_SystemOutputs_Str); i++) ((unsigned char*) &io->outputs)[i] = (unsigned char) data.at(PWRMANAGEMENT_ACTUATOR_IO+sizeof(_SystemInputs_Str)+i);

        // Diagnostica pulsante raggi
        if(io->inputs.CPU_XRAYPUSH_FAULT){
            // Fault segnalato dalla PCB240 su controllo del pulsante bloccato all'accensione!
            PageAlarms::activateNewAlarm(_DB_ALLARME_XRAY_PUSH,XRAY_PUSH_TIMEOUT);
            if(timerXrayPush){
                killTimer(timerXrayPush);
                timerXrayPush=0;
            }
        }else{
            // Impostazione stato del pulsante raggi
            if(io->inputs.CPU_XRAY_REQ){
                if(timerXrayPush==0) timerXrayPush = startTimer(90000); // 1 minuto se Ë stato abilitato
                ApplicationDatabase.setData(_DB_XRAY_PUSH_BUTTON,(unsigned char) 1,0);
            }else{
                if(timerXrayPush){
                    killTimer(timerXrayPush);
                    timerXrayPush=0;
                }
                PageAlarms::activateNewAlarm(_DB_ALLARME_XRAY_PUSH,0);
                ApplicationDatabase.setData(_DB_XRAY_PUSH_BUTTON,(unsigned char) 0,0);
            }
        }

        if(io->inputs.CPU_CLOSED_DOOR) ApplicationDatabase.setData(_DB_CLOSED_DOOR,(unsigned char) 1,0);
        else ApplicationDatabase.setData(_DB_CLOSED_DOOR,(unsigned char) 0,0);

        // Diagnostica pedali lift
        if(io->inputs.CPU_LENZ_PED_FAULT)PageAlarms::activateNewAlarm(_DB_ALLARME_LIFT_PUSH,LIFT_INVALID_PUSH);
        else  PageAlarms::activateNewAlarm(_DB_ALLARME_LIFT_PUSH,0);

        // Diagnostica pedali arm
        if(io->inputs.CPU_ARM_PED_FAULT) PageAlarms::activateNewAlarm(_DB_ALLARME_ARM_PUSH,ARM_PUSH_TIMEOUT);
        else  PageAlarms::activateNewAlarm(_DB_ALLARME_ARM_PUSH,0);

        // Richiesta spegnimento sistema
        if(io->inputs.CPU_REQ_POWER_OFF) ApplicationDatabase.setData(_DB_REQ_POWEROFF,(unsigned char) 1,0);
        else  ApplicationDatabase.setData(_DB_REQ_POWEROFF,(unsigned char) 0,0);



    }


}

void Config::executeCmdFile(QString file){
    return;
}

// Controlla che il package sia contenuto nella directory home
bool Config::packageExist(){

    QString pkgfile = swConf.installedPackageName;
    if(pkgfile.isEmpty()) return false;
    pkgfile.prepend(FILE_HOME);

    // Verifica se c'√® un package da estrarre
    QFile file(pkgfile.toStdString().c_str());
    return file.exists();
}

/* Questa funzione attiva il timer per l'esecuzione del REBOOT DI SISTEMA */
void Config::executeReboot(){
    QString stringa1 = QString("SYSTEM REBOOT");
    QString stringa =  QString("The System will reboot in 3 seconds ...");
    pWarningBox->activate(stringa1,stringa,100,0);
    pWarningBox->setTextAlignment(Qt::AlignLeft);


    // Invia comando di reboot allo slave
    protoConsole frame(1,false);
    emit configMasterTx( frame.cmdToQByteArray(SLAVE_EXECUTE_REBOOT));

    // Attiva il timer per il reboot locale
    timerReboot = startTimer(3000);
    return;
}


void Config::activatePowerOff(void)
{
    if(!isMaster) return;

    // Con il Software AWS connesso e lo studio aperto (ma non in test) non si deve spegnere il PC brutalmente
    // ma si chiede il permesso
    if((ApplicationDatabase.getDataU(_DB_STUDY_STAT)!=_CLOSED_STUDY_STATUS) && (ApplicationDatabase.getDataS(_DB_CALIB_SYM)=="")){
        pToConsole->notifyRequestPowerOff();
        return;
    }

    // Se il PC Ë presente, chiede lo spegnimento a basso livello
    if(ApplicationDatabase.getDataU(_DB_PCAWS_CONNECTION)){
        masterRequestAwsPowerOff(); // Richiede lo spegnimento del PC a basso livello
        timerPoweroff = startTimer(5000);
        return;
    }

    // Se AWS Ë spenta allora si richiede uno spegnimento del solo stativo
    executePoweroff(10);

}

/* Questa funzione attiva il timer per l'esecuzione del REBOOT DI SISTEMA */
void Config::executePoweroff(unsigned char secondi){

    // Invia comando di reboot allo slave
    protoConsole frame(1,false);
    frame.addParam(QString("%1").arg(secondi));
    emit configMasterTx( frame.cmdToQByteArray(SLAVE_EXECUTE_POWEROFF));

    // Attiva il timer per il reboot locale
    timerPoweroff = startTimer(5000);
    return;
}


bool Config::executeUpdateIde(){

    // Invia comando di reboot allo slave
    protoConsole frame(1,false);
    emit configMasterTx( frame.cmdToQByteArray(SLAVE_EXECUTE_UPDATE_IDE));

    QString command = QString("/monta.sh");
    system(command.toStdString().c_str());
    command = QString("cp /mnt/nfs/m4_master.bin /");
    system(command.toStdString().c_str());
    command = QString("cp /mnt/nfs/m4_slave.bin /");
    system(command.toStdString().c_str());
    command = QString("cp /mnt/target/DBTController /");
    system(command.toStdString().c_str());
    command = QString("sync");
    system(command.toStdString().c_str());

    // Attiva il timer per il eboot locale
    timerReboot = startTimer(2000);
    return true;
}

bool Config::executeUpdateGui(){

    // Invia comando di reboot allo slave
    protoConsole frame(1,false);
    emit configMasterTx( frame.cmdToQByteArray(SLAVE_EXECUTE_UPDATE_GUI));

    QString command = QString("/monta.sh");
    system(command.toStdString().c_str());
    command = QString("cp /mnt/target/DBTController /");
    system(command.toStdString().c_str());
    command = QString("sync");
    system(command.toStdString().c_str());

    // Attiva il timer per il eboot locale
    timerReboot = startTimer(2000);
    return true;
}
QByteArray Config::executeShell(QByteArray data){

    QString command = QString("(") + QString(data) + QString(")") + QString(" > ppp");
    system(command.toStdString().c_str());

    QFile file("ppp");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QByteArray("ERROR\n");
    }

    QByteArray buffer = "";
    while(!file.atEnd())
    {
        buffer.append(file.readLine()).append('\r');
    }
    file.close();

    return buffer;
}

void Config::executeSlaveShell(QByteArray data){

    protoConsole frame(1,false);

    QByteArray comando = QByteArray(SLAVE_EXECUTE_SHELL).append(" ").append(data);

    emit configMasterTx( frame.formatToQByteArray(comando));

}


/* Restituisce TRUE se si trova gi√  nello stato richiesto */
bool Config::setDemoMode(bool demo){
    if(userCnf.demoMode == demo) return true;

    // Imposta il nuovo stato
    userCnf.demoMode = demo;
    this->saveUserCfg();

    // Comanda il reboot del sistema
    this->executeReboot();
    return false;
}

/* Questa funzione attiva il timer per l'esecuzione del REBOOT DI SISTEMA */
void Config::setToolsData(int tipo, QList<int> data){

    protoConsole frame(tipo,false);

    for(int i = 0; i< data.size(); i++)
        frame.addParam(QString("%1").arg(data[i]));

    emit configMasterTx( frame.cmdToQByteArray(SLAVE_TOOLS_DATA));
    return;
}
/* Questa funzione attiva il timer per l'esecuzione del REBOOT DI SISTEMA */
void Config::setToolsData(int tipo){

    protoConsole frame(tipo,false);
    emit configMasterTx( frame.cmdToQByteArray(SLAVE_TOOLS_DATA));
    return;
}

/* Questa funzione effettua un polling sync verso lo slave */
void Config::syncToSlave(void){

    protoConsole frame(1,false);
    emit configMasterTx( frame.cmdToQByteArray(SYNC_TO_SLAVE));

}
// _____________________________________________________________________________________________
// FUNZIONI DI CONFIGURAZIONE DRIVER

void Config::updateAllDrivers(void){
    singleConfigUpdate = false;
    sendMccConfigCommand(CONFIG_GENERAL);
    return ;
}

void Config::updateSystemCfg(void){
    singleConfigUpdate = true;
    sendMccConfigCommand(CONFIG_GANTRY);
    return ;
}

void Config::updateGeneral(void){
    singleConfigUpdate = true;
    sendMccConfigCommand(CONFIG_GENERAL);
    return ;
}

void Config::updatePCB249U1(void){
    singleConfigUpdate = true;
    pcb249U1UpdateRequired = false;
    sendMccConfigCommand(CONFIG_PCB249U1_1);
    return ;
}

void Config::updateBiopsy(void){
    singleConfigUpdate = true;
    sendMccConfigCommand(CONFIG_BIOPSY);
    return ;
}

void Config::updatePCB244(void){
    singleConfigUpdate = true;
    sendMccConfigCommand(CONFIG_PCB244);
    return ;
}
void Config::updatePCB249U2(void){
    singleConfigUpdate = true;
    sendMccConfigCommand(CONFIG_PCB249U2);
    return ;
}

void Config::updatePCB269(void){
    singleConfigUpdate = true;
    sendMccConfigCommand(CONFIG_PCB269);
    return ;
}

void Config::updatePCB190(void){
    singleConfigUpdate = true;
    sendMccConfigCommand(CONFIG_PCB190);
    return ;
}

void Config::updateLenzeDriver(void){
    singleConfigUpdate = true;
    sendMccConfigCommand(CONFIG_LENZE);
    return ;
}

void Config::updateArmDriver(void){
    singleConfigUpdate = true;
    sendMccConfigCommand(CONFIG_ARM);
    return ;
}

void Config::updateTrxDriver(void){
    singleConfigUpdate = true;
    sendMccConfigCommand(CONFIG_TRX);
    return ;
}


/**
 *
 * Handler di gestione delle fasi di upgrade della configurazione
 *
 */
void Config::configNotifySlot(unsigned char id, unsigned char mcccode, QByteArray buffer)
{

    switch(mcccode){
    case CONFIG_GENERAL:
         sendMccConfigCommand(CONFIG_PCB269);
        break;
    case CONFIG_PCB269:
        if(singleConfigUpdate) {
            singleConfigUpdate = false;
            emit configUpdatedSgn();
            return ;
        }
        sendMccConfigCommand(CONFIG_PCB190);
        break;
    case CONFIG_PCB190:
        if(singleConfigUpdate) {
            singleConfigUpdate = false;
            emit configUpdatedSgn();
            return ;
        }
        sendMccConfigCommand(CONFIG_PCB249U1_1);
        break;

    case CONFIG_PCB249U1_1: // ___________________ BLOCCO AGGIORNAMENTO PCB249 U1
        sendMccConfigCommand(CONFIG_PCB249U1_2);
        break;

    case CONFIG_PCB249U1_2:
        sendMccConfigCommand(CONFIG_PCB249U1_3);
        break;

    case CONFIG_PCB249U1_3:
        sendMccConfigCommand(CONFIG_PCB249U1_4);
        break;

    case CONFIG_PCB249U1_4:
        sendMccConfigCommand(CONFIG_PCB249U1_5);
        break;

    case CONFIG_PCB249U1_5:
        sendMccConfigCommand(CONFIG_PCB249U1_6);
        break;
    case CONFIG_PCB249U1_6:
        sendMccConfigCommand(CONFIG_PCB249U1_7);
        break;
    case CONFIG_PCB249U1_7: // __________________________________________________
        if(singleConfigUpdate) {
            singleConfigUpdate = false;
            emit configUpdatedSgn();
            return ;
        }
        sendMccConfigCommand(CONFIG_PCB249U2);
        break;
    case CONFIG_PCB249U2:
        if(singleConfigUpdate) {
            singleConfigUpdate = false;
            emit configUpdatedSgn();
            return ;
        }
        sendMccConfigCommand(CONFIG_PCB244);
        break;
    case CONFIG_PCB244:
        if(singleConfigUpdate) {
            singleConfigUpdate = false;
            emit configUpdatedSgn();
            return ;
        }
        sendMccConfigCommand(CONFIG_BIOPSY);
        break;

    case CONFIG_BIOPSY:
        if(singleConfigUpdate) {
            singleConfigUpdate = false;
            emit configUpdatedSgn();
            return ;
        }
        sendMccConfigCommand(CONFIG_ARM);
        break;


    case CONFIG_ARM:
        if(singleConfigUpdate) {
            singleConfigUpdate = false;
            emit configUpdatedSgn();
            return ;
        }
        sendMccConfigCommand(CONFIG_TRX);
        break;

    case CONFIG_TRX:
        if(singleConfigUpdate) {
            singleConfigUpdate = false;
            emit configUpdatedSgn();
            return ;
        }
        sendMccConfigCommand(CONFIG_LENZE);
        break;

   case CONFIG_LENZE:
        if(singleConfigUpdate) {
            singleConfigUpdate = false;
            emit configUpdatedSgn();
            return ;
        }
        sendMccConfigCommand(CONFIG_COMPLETED);
        break;

    case CONFIG_COMPLETED:
        emit configUpdatedSgn();
        break;
    }

}



/*
 *  FUNZIONE PER IL CONTROLLO DEL PACKAGE
 *
 *
 */
bool Config::checkPackage(void)
{

    // Verifica delle revisioni compatibili con la versione corrente
    revisionError.clear();
    bool result=true;

    revisionError.append(QString("MASTER [%1]: -> %2").arg(swConf.rvGuiMaster).arg(pConfig->rvGuiMaster));
    if((!swConf.rvGuiMaster.isEmpty())&&(swConf.rvGuiMaster!=pConfig->rvGuiMaster)){
        result = false;
        revisionError.append(QString(" NOK!\n\r"));
    }else revisionError.append(QString(" OK\n\r"));

    revisionError.append(QString("SLAVE [%1]: -> %2").arg(swConf.rvGuiSlave).arg(pConfig->rvGuiSlave));
    if((!swConf.rvGuiSlave.isEmpty())&&(swConf.rvGuiSlave!=pConfig->rvGuiSlave)){
        result = false;
        revisionError.append(QString(" NOK!\n\r"));
    }else revisionError.append(QString(" OK\n\r"));

    // Applicazioni M4
    revisionError.append(QString("DRIVER MASTER [%1]: -> %2").arg(swConf.rvM4Master).arg(pConfig->rvM4Master));
    if((!swConf.rvM4Master.isEmpty())&&(swConf.rvM4Master!=pConfig->rvM4Master)){
        result = false;
        revisionError.append(QString(" NOK!\n\r"));
    }else revisionError.append(QString(" OK\n\r"));

    revisionError.append(QString("DRIVER SLAVE [%1]: -> %2").arg(swConf.rvM4Slave).arg(pConfig->rvM4Slave));
    if((!swConf.rvM4Slave.isEmpty())&&(swConf.rvM4Slave!=pConfig->rvM4Slave)){
        result = false;
        revisionError.append(QString(" NOK!\n\r"));
    }else revisionError.append(QString(" OK\n\r"));

    revisionError.append(QString("FW269 [%1]: -> %2").arg(swConf.rv269).arg(rv269));
    if(swConf.rv269!=rv269){
        result = false;
        revisionError.append(QString(" NOK!\n\r"));
    }else revisionError.append(QString(" OK\n\r"));


    // Discrimina il modello montato. Per compatibilit‡ con il passato9
    // i collimatori in ASSY 01 (MODELLO A) rimarranno fermi alla revisione 2.4 (U1) 1.4 (U2)
    // mentre i nuovi collimatori (ASSY 02) andranno avfanti a partire dalle revisioni 3.1 (U1) 2.1 (U2)
    // Il tipo di colllimatore viene impostato tramite parametro (pConfig->sys.collimator_type)
    QString rv249U1selected, rv249U2selected;
    if(pCollimatore->colli_model == _COLLI_TYPE_ASSY_01){
        rv249U1selected = "2.5";
        rv249U2selected = "1.4";
    }else{
        rv249U1selected = swConf.rv249U1;
        rv249U2selected = swConf.rv249U2;
    }

    revisionError.append(QString("FW249U1 [%1]: -> %2").arg(rv249U1selected).arg(rv249U1));
    if(rv249U1selected!=rv249U1){
        result = false;
        revisionError.append(QString(" NOK!\n\r"));
    }else revisionError.append(QString(" OK\n\r"));

    revisionError.append(QString("FW249U2 [%1]: -> %2").arg(rv249U2selected).arg(rv249U2));
    if(rv249U2selected!=rv249U2){
        result = false;
        revisionError.append(QString(" NOK!\n\r"));
    }else revisionError.append(QString(" OK\n\r"));



    revisionError.append(QString("FW190A [%1]: -> %2").arg(swConf.rv190).arg(rv190));
    if(swConf.rv190!=rv190){
        result = false;
        revisionError.append(QString(" NOK!\n\r"));
    }else revisionError.append(QString(" OK\n\r"));

    revisionError.append(QString("FW240DMD [%1]: -> %2").arg(swConf.rv240).arg(rv240));
    if(swConf.rv240!=rv240){
        result = false;
        revisionError.append(QString(" NOK!\n\r"));
    }else revisionError.append(QString(" OK\n\r"));
#ifdef __FORCE_DIGITAL
    // Non controlla la PCB244
#else
    // Esposimero
    revisionError.append(QString("FW244A [%1]: -> %2").arg(swConf.rv244A).arg(rv244A));
    if(swConf.rv244A!=rv244A){
        result = false;
        revisionError.append(QString(" NOK!\n\r"));
    }else revisionError.append(QString(" OK\n\r"));

#endif
    // Verifica se tutto Ë conforme
    PRINT(revisionError);
    PRINT("\n");

    if(!result){
        if(packageExist()) ApplicationDatabase.setData(_DB_SERVICE1_STR,"PACKAGE_PRESENT");
        else ApplicationDatabase.setData(_DB_SERVICE1_STR,"");
        ApplicationDatabase.setData(_DB_REVISION_ERROR_STRING,revisionError);
    }
    return result;

}

// Converte da decimi di grado a grado pi˘ vicino
int Config::convertDangolo(int data){

    // Converte da decimi di grado a grado pi˘ vicino
    float fangolo = (float) data / 10;
    float angolo = (float) ((int) fangolo);
    if((fangolo-angolo)>0.5) angolo=angolo+1;
    return (int) angolo;

}

////////////////////////////////////////////////////////////////////////////////
// COMANDI PER COMUNICAZIONE A BASSO LIVELLO CON IL PC
void Config::awsConnectionHandler(bool stat)
{
    awsConnected = stat;

    if(awsConnected) ApplicationDatabase.setData(_DB_PCAWS_CONNECTION,(unsigned char) 1);
    else ApplicationDatabase.setData(_DB_PCAWS_CONNECTION,(unsigned char) 0);

    return;
}

// Funzione per comandare lo spegnimento del PC
void Config::awsPowerOff(void){
    if(awsConnected==false) return;

    protoConsole cmd(1,UNICODE_FORMAT);
    emit awsTxHandler(cmd.cmdToQByteArray(QString("SetPowerOff")));


    return;
}



void Config::masterRequestAwsPowerOff(void){

    // Invia comando di reboot allo slave
    protoConsole frame(1,false);
    emit configMasterTx( frame.cmdToQByteArray(SLAVE_EXECUTE_PC_POWEROFF));
}

//________________________________________________________________________________________
//                      SLAVE DATA INITIALIZATION
//________________________________________________________________________________________
void Config::slaveInitialization(void){

    if(!systemInitialized) return;

    // Upgrade dello stato della connessione con la AWS

    if(awsConnected) ApplicationDatabase.setData(_DB_PCAWS_CONNECTION,(unsigned char) 1,DBase::_DB_FORCE_SGN);
    else ApplicationDatabase.setData(_DB_PCAWS_CONNECTION,(unsigned char) 0,DBase::_DB_FORCE_SGN);

    // Impostazione della configurazione
    ApplicationDatabase.setData(_DB_SLAVE_GUI_REVISION,pConfig->rvGuiSlave,DBase::_DB_FORCE_SGN);

    // Inizializzazione dei registri dipendenti dallo Slave

    slaveDataInitialized = true;
    return;
}

QString Config::getNanotecErrorClass(unsigned char classe){

    switch(classe){
    case 0x1: return NANOTEC_ERRCLASS_0x1;
    case 0x2: return NANOTEC_ERRCLASS_0x2;
    case 0x4: return NANOTEC_ERRCLASS_0x4;
    case 0x8: return NANOTEC_ERRCLASS_0x8;
    case 0x10: return NANOTEC_ERRCLASS_0x10;
    case 0x20: return NANOTEC_ERRCLASS_0x20;
    case 0x40: return NANOTEC_ERRCLASS_0x40;
    case 0x80: return NANOTEC_ERRCLASS_0x80;
    default: return "";
    }
}

QString Config::getNanotecErrorSubClass(unsigned char subclass){

    switch(subclass){
    case 0: return NANOTEC_ERRNUM_0;
    case 1: return NANOTEC_ERRNUM_1;
    case 2: return NANOTEC_ERRNUM_2;
    case 3: return NANOTEC_ERRNUM_3;
    case 4: return NANOTEC_ERRNUM_4;
    case 5: return NANOTEC_ERRNUM_5;
    case 6: return NANOTEC_ERRNUM_6;
    case 7: return NANOTEC_ERRNUM_7;
    case 8: return NANOTEC_ERRNUM_8;
    case 9: return NANOTEC_ERRNUM_9;
    case 10: return NANOTEC_ERRNUM_10;
    case 11: return NANOTEC_ERRNUM_11;
    case 12: return NANOTEC_ERRNUM_12;
    case 13: return NANOTEC_ERRNUM_13;
    case 14: return NANOTEC_ERRNUM_14;
    case 15: return NANOTEC_ERRNUM_15;
    case 16: return NANOTEC_ERRNUM_16;
    case 17: return NANOTEC_ERRNUM_17;
    case 18: return NANOTEC_ERRNUM_18;
    case 19: return NANOTEC_ERRNUM_19;
    case 20: return NANOTEC_ERRNUM_20;
    case 21: return NANOTEC_ERRNUM_21;
    case 22: return NANOTEC_ERRNUM_22;
    case 23: return NANOTEC_ERRNUM_23;
    case 24: return NANOTEC_ERRNUM_24;
    case 25: return NANOTEC_ERRNUM_25;
    case 26: return NANOTEC_ERRNUM_26;
    case 27: return NANOTEC_ERRNUM_27;
    case 28: return NANOTEC_ERRNUM_28;

    case 251: return NANOTEC_ERRNUM_251;
    case 252: return NANOTEC_ERRNUM_252;
    case 253: return NANOTEC_ERRNUM_253;
    case 254: return NANOTEC_ERRNUM_254;
    case 255: return NANOTEC_ERRNUM_255;
    }

    return "";
}

QString Config::getNanotecErrorCode(unsigned short code){

    switch(code){

    // Codici interni
    case 0x0001: return NANOTEC_ERRCODE_0x0001;
    case 0x0002: return NANOTEC_ERRCODE_0x0002;
    case 0x0003: return NANOTEC_ERRCODE_0x0003;
    case 0x0004: return NANOTEC_ERRCODE_0x0004;
    case 0x0005: return NANOTEC_ERRCODE_0x0005;
    case 0x0006: return NANOTEC_ERRCODE_0x0006;
    case 0x0007: return NANOTEC_ERRCODE_0x0007;
    case 0x0008: return NANOTEC_ERRCODE_0x0008;

    // Codici Nanotec
    case 0x1000: return NANOTEC_ERRCODE_0x1000;
    case 0x2300: return NANOTEC_ERRCODE_0x2300;
    case 0x3100: return NANOTEC_ERRCODE_0x3100;
    case 0x4200: return NANOTEC_ERRCODE_0x4200;
    case 0x6010: return NANOTEC_ERRCODE_0x6010;
    case 0x6100: return NANOTEC_ERRCODE_0x6320;
    case 0x6320: return NANOTEC_ERRCODE_0x7121;
    case 0x7305: return NANOTEC_ERRCODE_0x7305;
    case 0x7600: return NANOTEC_ERRCODE_0x7600;
    case 0x8000: return NANOTEC_ERRCODE_0x8000;
    case 0x8130: return NANOTEC_ERRCODE_0x8130;
    case 0x8200: return NANOTEC_ERRCODE_0x8200;
    case 0x8210: return NANOTEC_ERRCODE_0x8210;
    case 0x8220: return NANOTEC_ERRCODE_0x8220;
    case 0x8611: return NANOTEC_ERRCODE_0x8611;
    case 0x8612: return NANOTEC_ERRCODE_0x8612;
    case 0x9000: return NANOTEC_ERRCODE_0x9000;
    }

    return "";

}

QString Config::getI550ErrorString(unsigned short value){
    switch(value){
    case 0: return "";
    case 0x2250: return "CiA: continuous overcurrent (inside the device) Fault";
    case 0x3222: return "DC-bus voltage too low for switch-on Warning ";
    case 0x3221: return "DC bus undervoltage warning Warning -";
    case 0x3220: return "DC bus undervoltage Trouble -";
    case 0x3211: return "DC bus overvoltage warning Warning -";
    case 0x3210: return "DC bus overvoltage Fault -";
    case 0x3180: return "Operation at UPS active Warning -";
    case 0x3120: return "Mains phase fault Fault -";
    case 0x2388: return "SLPSM stall detection active Trouble -";
    case 0x2387: return "Imax: Clamp responded too often Fault -";
    case 0x2383: return "I*t warning Warning -";
    case 0x2382: return "I*t error Fault 0x2D40:005 (P135.05)";
    case 0x2350: return "CiA: i≤*t overload (thermal state) Fault 0x2D4B:003 (P308.03)";
    case 0x2340: return "CiA: -Short circuit (inside the device) Fault ";
    case 0x2320: return "CiA:-Short circuit/earth leakage (internal) Fault ";
    case 0x618A: return "Internal fan warning Warning -";
    case 0x5380: return "OEM hardware incompatible Fault -";
    case 0x5180: return "24-V supply overload Warning -";
    case 0x5112: return "24 V supply critical Warning -";
    case 0x4285: return "Power section overtemperature warning Warning -";
    case 0x4281: return "Heatsink fan warning Warning -";
    case 0x4280: return "Thermal sensor heatsink error Fault -";
    case 0x4210: return "PU: overtemperature fault Fault -";
    case 0x62B1: return "NetWordIN1 configuration incorrect Trouble";
    case 0x62A2: return "Network: user fault 2 Fault ";
    case 0x62A1: return "Network: user fault 1 Fault ";
    case 0x62A0: return "AC Drive: user fault Fault";
    case 0x6291: return "Number of maximum permissible faults exceeded Fault";
    case 0x6290: return "Reversal warning Warning";
    case 0x6282: return "User-defined fault 2 Fault";
    case 0x6281: return "User-defined fault 1 Fault";
    case 0x6280: return "Trigger/functions connected incorrectly Trouble ";
    case 0x7689: return "Memory module: invalid OEM data Warning";
    case 0x7686: return "Internal communication error Fault";
    case 0x7684: return "Data not completely saved before switch-off Warning";
    case 0x7682: return "Memory module: invalid user data Fault";
    case 0x7681: return "No memory module Fault";
    case 0x7680: return "Memory module is full Warning";
    case 0x7180: return "Motor overcurrent Fault 0x2D46:002 (P353.02)";
    case 0x7121: return "Pole position identification fault Fault 0x2C60";
    case 0x70A2: return "Analog output 2 fault Warning";
    case 0x70A1: return "Analog output 1 fault Warning ";
    case 0x7082: return "Error of analog input 2 Fault 0x2637:010 (P431.10)";
    case 0x7081: return "Error of analog input 1 Fault 0x2636:010 (P430.10)";
    case 0x7080: return "Monitoring of connection level (Low/High) Fault";
    case 0x63A3: return "Power section unknown Fault";
    case 0x63A2: return "PU: load error ID tag Fault";
    case 0x63A1: return "CU: load error ID tag Fault";
    case 0x8187: return "CAN: heartbeat time-out consumer 4 Fault 0x2857:008";
    case 0x8186: return "CAN: heartbeat time-out consumer 3 Fault 0x2857:007";
    case 0x8185: return "CAN: heartbeat time-out consumer 2 Fault 0x2857:006";
    case 0x8184: return "CAN: heartbeat time-out consumer 1 Fault 0x2857:005";
    case 0x8183: return "CAN: warning Warning 0x2857:011";
    case 0x8182: return "CAN: bus off Trouble 0x2857:010";
    case 0x8115: return "Time-out (PZ‹) No response 0x2552:004 (P595.04)";
    case 0x7697: return "Changed parameters lost Fault ";
    case 0x7696: return "EPM data: unknown parameter found Info";
    case 0x7695: return "Invalid configuration of parameter change-over Warning";
    case 0x7694: return "EPM data: new PU size detected Fault";
    case 0x7693: return "EPM data: PU size incompatible Fault";
    case 0x7692: return "EPM data: new firmware type detected Fault";
    case 0x7691: return "EPM data: firmware type incompatible Fault";
    case 0x7690: return "EPM firmware version incompatible Fault";
    case 0x768A: return "Memory module: wrong type Fault";
    case 0xFF0C: return "Motor phase failure phase W No response 0x2D45:001 (P310.01)";
    case 0xFF0B: return "Motor phase failure phase V No response 0x2D45:001 (P310.01)";
    case 0xFF0A: return "Phase U motor phase failure No response 0x2D45:001 (P310.01)";
    case 0xFF09: return "Motor phase missing No response 0x2D45:001 (P310.01)";
    case 0xFF06: return "Motor overspeed Fault 0x2D44:002 (P350.02)";
    case 0xFF05: return "Safe Torque Off error Fault";
    case 0x9080: return "Keypad removed Fault";
    case 0x8311: return "Torque limit reached No response 0x2D67:001 (P329.01)";
    case 0x8293: return "CAN: RPDO3 time-out Fault 0x2857:003";
    case 0x8292: return "CAN: RPDO2 time-out Fault 0x2857:002";
    case 0x8291: return "CAN: RPDO1 time-out Fault 0x2857:001";
    case 0x81A2: return "Modbus: incorrect request by master Warning";
    case 0x81A1: return "Modbus: network time-out Fault 0x2858:001 (P515.01)";
    case 0xFF85: return "eypad full control active Warning";
    case 0xFF56: return "Maximum motor frequency reached Warning";
    case 0xFF37: return "Automatic start disabled Fault ";
    case 0xFF19: return "Motor parameter identification error Fault";
    default: return "Undefined error";
    }
}

QString Config::getI550DiagnosticErrorStr(unsigned char code){
    switch(code){
    case 0: return "";
    case LENZE_ANALOG_CONNECTION_ERROR: return "Analog Input1 seems not connected to Analog Input2!";
    case LENZE_ANALOG_INPUT_ERROR: return "Analog Input1 or Analog Input2 disconnected or damaged!";
    case LENZE_DROP_ARM: return "Drop C-Arm switch detected active!";
    case LENZE_DEVICE_ERROR: return "Lenze driver internal error!";
    case LENZE_LOWER_ERROR: return "Wrong low pot threshold!";
    case LENZE_UPPER_ERROR: return "Wrong high pot threshold!";
    default: return "Undefined error!";
    }
}

void Config::powerOffSlot(void){
    if(isMaster) pSysLog->flush();

    QString command = QString("sync");
    system(command.toStdString().c_str());

    command = QString("poweroff ");
    system(command.toStdString().c_str());
    return;
}

void Config::rebootSlot(void){
    if(isMaster) pSysLog->flush();

    QString command = QString("sync");
    system(command.toStdString().c_str());

    command = QString("reboot ");
    system(command.toStdString().c_str());
    return;
}

void Config::updateDate(void){
    if(!isMaster) {
        return;
    }

    QString Y,M,D,h,m,s,command;

    Y = QString("%1").arg(year);
    if(month<10) M=QString("0%1").arg(month);
    else M=QString("%1").arg(month);
    if(day<10) D=QString("0%1").arg(day);
    else D=QString("%1").arg(day);

    if(hour<10) h=QString("0%1").arg(hour);
    else h=QString("%1").arg(hour);
    if(min<10) m=QString("0%1").arg(min);
    else m=QString("%1").arg(min);
    if(sec<10) s=QString("0%1").arg(sec);
    else s=QString("%1").arg(sec);


    echoDisplay.echoDate(D,M,Y,h,m,s,DBase::_DB_NO_OPT);

    command = QString("date -u %1.%2.%3-%4:%5:%6").arg(Y).arg(M).arg(D).arg(h).arg(m).arg(s);
    PRINT(command);
    system(command.toStdString().c_str());

    command = QString("hwclock -w");
    system(command.toStdString().c_str());

    systemTimeUpdated = TRUE;

    // Con la ricezione della data Ë possibile inizializzare il dispositivo
    // per la misura della quantit‡ di calore accumulata nell'Anodo
    pGeneratore->initAnodeHU();

    return;
}

void Config::updateRTC(void){
    if(!isMaster) {
        return;
    }

    unsigned char buffer[10];

    buffer[0] = 1; // Comando SET
    buffer[1] = _RTC_MON; // Viene sempre inizializzato a lunedi
    buffer[2] = year & 0xFF; // Anno
    buffer[3] = (year >> 8) & 0xFF; // Anno
    buffer[4] = month; // Mese
    buffer[5] = day; // Giorno
    buffer[6] = hour; // Ora
    buffer[7] = min; // Minuti
    buffer[8] = sec; // Secondi
    pConsole->pGuiMcc->sendFrame(MCC_RTC_COMMANDS,1,buffer,9);

}
