#include "application.h"
#include "appinclude.h"
#include "globvar.h"

#include "ANALOG/pageOpenAnalogic.h"
extern AnalogPageOpen* paginaOpenStudyAnalogic;


QHostAddress serverDebug::setIpAddress( int val){
    int size = TcpIpServer::hostAddress().toString().length();
    return QHostAddress(QString("%1.%2").arg(TcpIpServer::hostAddress().toString().left(size-2)).arg(val)) ;
}

bool serverDebug::isIp(int val){
    if(TcpIpServer::hostAddress().toString().right(1).toInt()==val) return true;
    return false;
}

void serverDebug::activateConnections(void) {
    QObject::connect(serviceTcp,SIGNAL(rxData(QByteArray)),this,SLOT(serviceRxHandler(QByteArray)),Qt::UniqueConnection);
    QObject::connect(serviceTcp,SIGNAL(serverConnection(bool)),this,SLOT(notificheConnectionHandler(bool)),Qt::UniqueConnection);
    serviceTcp->Start(_LOCAL_SERVICE_PORT);

}

serverDebug::serverDebug(void) :
    QObject(0)
{
    // Creazione del socket di comunicazione esterna con la Console
    serviceTcp = new TcpIpServer();

    lastValidFrame.clear();
    cmdGroup.clear();

    return;

}

void serverDebug::notificheConnectionHandler(bool stat)
{
    if(stat)
    {
        // Connessione automatica al servizio di log eventi
        serviceTcp->txData(QString("---------   ANALOG DEVICE: IRS INTERFACE, Rev %1.%2.%3      -------------\r\n").arg((int)IRS_MAJ).arg((int)IRS_MIN).arg((int)IRS_BETA).toAscii());
        serviceTcp->txData(QByteArray(">"));
    }else
    {
        disconnect(this);
        pGeneratore->manualMode = false;

    }
    return;
}

void serverDebug::serviceRxHandler(QByteArray data)
{

    // Carattere di annullamento famiglia
    if(data.contains("..")) cmdGroup.clear();

    // Cambio famiglia
    if(data.contains(':')==FALSE) data.prepend(cmdGroup);

    if(data.contains("collimatore:"))
    {
        cmdGroup="collimatore: ";
        handleCollimatore(data);
    }else if(data.contains("config:"))
    {
        cmdGroup="config: ";
        handleConfig(data);
    } else if(data.contains("shell:"))
    {
        cmdGroup="shell: ";
        handleShell(data);
    } else if(data.contains("rotazioni:"))
    {
        cmdGroup="rotazioni: ";
        handleRotazioni(data);
    }else if(data.contains("setPage:"))
    {
        handleSetPage(data);
    }
    else if(data.contains("generatore:"))
    {
        cmdGroup="generatore: ";
        handleGeneratore(data);
    }else if(data.contains("aws:"))
    {
        cmdGroup="aws: ";
        handleConsole(data);
    }else if(data.contains("system:"))
    {
        cmdGroup="system: ";
        handleSystem(data);
    }else if(data.contains("drivers:"))
    {
        cmdGroup="drivers: ";
        handleDrivers(data);
    }else if(data.contains("biopsy:"))
    {
        cmdGroup="biopsy: ";
        handleBiopsy(data);
    }else if(data.contains("loader:"))
    {
        cmdGroup="loader: ";
        handleLoader(data);
    }else if(data.contains("compressore:"))
    {
        cmdGroup="compressore: ";
        handleCompressore(data);
    }else if(data.contains("master:"))
    {
        cmdGroup="master: ";
        handleMasterShell(data);
    }else if(data.contains("slave:"))
    {
        cmdGroup="slave: ";
        handleSlaveShell(data);
    }else if(data.contains("potter:"))
    {
        cmdGroup="potter: ";
        handlePotter(data);
    }else if(data.contains("canopen:"))
    {
        cmdGroup="canopen: ";
        handleCanOpen(data);
    }else if(data.contains("debug:"))
    {
        cmdGroup="debug: ";
        handleDebug(data);
    }else if(data.contains("analog:"))
    {
        cmdGroup="analog: ";
        handleAnalog(data);
    }else if(data.contains("?")) // DEVE ESSERE L'ULTIMO
    {
        handleList();
    }



    lastValidFrame = data; // Salva il comando
    if(cmdGroup.isEmpty()) serviceTcp->txData(QByteArray(">"));
    else serviceTcp->txData(QByteArray(cmdGroup).append(">"));
    return;
}

void serverDebug::handleConfig(QByteArray data)
{
    QString stringa;
    unsigned char buffer[20];

    if(data.contains("?"))
    {
        serviceTcp->txData(QByteArray("----------------------------------------------------------------------------------\r\n"));
        serviceTcp->txData(QByteArray("enableAccessory   [ON/OFF]   ON=Enabled OFF=Disabled \r\n"));
        serviceTcp->txData(QByteArray("setSN             [n/.]       .=erase SN, n=gantry serial number (only digits)\r\n"));
        serviceTcp->txData(QByteArray("setPSW            [n]         n=Service panel password (only digits)\r\n"));
        serviceTcp->txData(QByteArray("enableStarterKeep [ON/OFF]   ON=Keeping enabled OFF=Keeping disabled\r\n"));
        serviceTcp->txData(QByteArray("setTubeTemp       [lval/hval] lval = reset Tube alarm temp, hval=Trip Tube alarm temp\r\n"));
        serviceTcp->txData(QByteArray("setLanguage       [ITA/ENG..] set the current language\r\n"));
        serviceTcp->txData(QByteArray("setRtc            Y:[2021/..] M:[01/12] D:[1/31] H:[00/23] m:[0/60] s:[0/60] \r\n"));
        serviceTcp->txData(QByteArray("resetGantry                   reset the factory gantry configuration\r\n"));
        serviceTcp->txData(QByteArray("resetKVread                   reset the KV read calibration file\r\n"));
        serviceTcp->txData(QByteArray("sysBackup         filename    backup master: nome = serial number\r\n"));
        serviceTcp->txData(QByteArray("sysRestore        filename    Restore da home\r\n"));
        serviceTcp->txData(QByteArray("----------------------------------------------------------------------------------\r\n"));
    }else if(data.contains("enableAccessory")){
                if(data.contains("ON")) {
                    pConfig->userCnf.enableCheckAccessorio = 1;
                    serviceTcp->txData(QByteArray("ACCESSORY TEST ENABLED!\n\r"));
                    pConfig->saveUserCfg();
                }else if(data.contains("OFF")){
                    pConfig->userCnf.enableCheckAccessorio = 0;
                    serviceTcp->txData(QByteArray("ACCESSORY TEST DISABLED!\n\r"));
                    pConfig->saveUserCfg();
                }else{
                    if(pConfig->userCnf.enableCheckAccessorio) serviceTcp->txData(QByteArray("ACCESSORY TEST ENABLED!\n\r"));
                    else serviceTcp->txData(QByteArray("ACCESSORY TEST DISABLED!\n\r"));
                }

     }else if(data.contains("setSN")){
        QList<QByteArray> parametri = getNextFieldsAfterTag(data, QString("setSN"));

        if(parametri.size() == 0) {
            stringa = QString("CURRENT SN: %1\r\n").arg(pConfig->userCnf.SN);
            serviceTcp->txData(stringa.toAscii());
            return;
        }
        if(parametri.size() != 1) {
            serviceTcp->txData(QString("Invalid serial number\n\r").toAscii());
            return;
        }
        if(parametri[0]=="."){
            pConfig->userCnf.SN ="";
            pConfig->SN_Configured = false;
            serviceTcp->txData(QByteArray("SERIAL NUMBER RESET!\r\n"));
        }else{
            pConfig->userCnf.SN = parametri[0];
            pConfig->SN_Configured = true;
            serviceTcp->txData(QByteArray("SERIAL NUMBER ASSIGNED!\r\n"));
        }

        pConfig->saveUserCfg();
        return;
    }else if(data.contains("setPSW")){
        QList<QByteArray> parametri = getNextFieldsAfterTag(data, QString("setPSW"));
        if(parametri.size() == 0) {
            stringa = QString("CURRENT PSW: %1\r\n").arg(pConfig->userCnf.ServicePassword);
            serviceTcp->txData(stringa.toAscii());
            return;
        }
        if(parametri.size() != 1) {
            serviceTcp->txData(QString("Invalid Password\n\r").toAscii());
            return;
        }
        pConfig->userCnf.ServicePassword = parametri[0];

        pConfig->saveUserCfg();
        pConfig->testConfigError(true, true);
        serviceTcp->txData(QByteArray("Password assigned!\r\n"));
        return;
    }else if(data.contains("enableStarterKeep")){
        if(data.contains("ON")) {
            pConfig->userCnf.starter_off_after_exposure = 0;
            serviceTcp->txData(QByteArray("STARTER KEEPING ENABLED!\n\r"));
            pConfig->saveUserCfg();
        }else if(data.contains("OFF")){
            pConfig->userCnf.starter_off_after_exposure = 1;
            serviceTcp->txData(QByteArray("STARTER KEEPING DISABLED!\n\r"));
            pConfig->saveUserCfg();
        }else{
            if(!pConfig->userCnf.starter_off_after_exposure) serviceTcp->txData(QByteArray("STARTER KEEPING ENABLED!\n\r"));
            else serviceTcp->txData(QByteArray("STARTER KEEPING DISABLED!\n\r"));
        }

    }else if(data.contains("setTubeTemp")){
        QList<QByteArray> parametri = getNextFieldsAfterTag(data, QString("setTubeTemp"));
        if(parametri.size() == 0 ) {
            stringa = QString("CURRENT PARAM: HIGH TEMP = %1, LOW TEMP=%2!\r\n").arg(pConfig->userCnf.tempCuffiaAlr).arg(pConfig->userCnf.tempCuffiaAlrOff);
            serviceTcp->txData(stringa.toAscii());
            return;
        }
        if(parametri.size() != 2) {
            serviceTcp->txData(QString("Invalid command: almost 2 parameters is requested\n\r").toAscii());
            return;
        }


        pConfig->userCnf.tempCuffiaAlr = parametri[1].toInt();     // Set allarme cuffia
        pConfig->userCnf.tempCuffiaAlrOff = parametri[0].toInt();  // Reset Allarme cuffia
        pConfig->saveUserCfg();
        stringa = QString("HIGH TEMP = %1, LOW TEMP=%2!\r\n").arg(pConfig->userCnf.tempCuffiaAlr).arg(pConfig->userCnf.tempCuffiaAlrOff);
        serviceTcp->txData(stringa.toAscii());
        return;
    }else if(data.contains("setLanguage")){
        handleSetLanguage(data);
    }else if(data.contains("resetGantry")){
        // Effettua un sync
        QString command = QString("rm /resource/config/sysCfg.cnf");
        system(command.toStdString().c_str());
        command = QString("sync");
        system(command.toStdString().c_str());

    }else if(data.contains("resetKVread")){
        // Effettua un sync
        QString command = QString("rm /resource/config/kvcalib.dat");
        system(command.toStdString().c_str());
        command = QString("sync");
        system(command.toStdString().c_str());

    }else if(data.contains("setRtc")){
        QList<QByteArray> parametri;
        parametri = getNextFieldsAfterTag(data, QString("setRtc"));
        if(parametri.size()==0){
            buffer[0] = 2; // GET RTC
            pConsole->pGuiMcc->sendFrame(MCC_RTC_COMMANDS,1,buffer,1);
            serviceTcp->txData(QByteArray("DONE \r\n"));
            return;
        }
        if(parametri.size()!=6) {
            serviceTcp->txData(QByteArray("MISSING PARAMETERS \r\n"));
            return;
        }

        buffer[0] = 1;

        buffer[1] = _RTC_MON;
        buffer[2] = parametri[1].toInt() & 0xFF; // Anno
        buffer[3] = (parametri[1].toInt() >> 8) & 0xFF; // Anno
        buffer[4] = parametri[2].toInt(); // Mese
        buffer[5] = parametri[3].toInt(); // Giorno
        buffer[6] = parametri[4].toInt(); // Ora
        buffer[7] = parametri[5].toInt(); // Minuti
        buffer[8] = parametri[6].toInt(); // Secondi

        pConsole->pGuiMcc->sendFrame(MCC_RTC_COMMANDS,1,buffer,9);
        serviceTcp->txData(QByteArray("DONE \r\n"));
        return;
    }else if(data.contains("sysBackup")){
        QList<QByteArray> parametri = getNextFieldsAfterTag(data, QString("sysBackup"));

        if(parametri.size() != 1) {
            stringa = QString("MISSING FILENAME\r\n");
            serviceTcp->txData(stringa.toAscii());
            return;
        }
        pConfig->sysBackup(TRUE,parametri[0],0);
        serviceTcp->txData(QByteArray("DONE \r\n"));
        return;
    }else if(data.contains("sysRestore")){
        QList<QByteArray> parametri = getNextFieldsAfterTag(data, QString("sysRestore"));

        if(parametri.size() != 1) {
            stringa = QString("MISSING FILENAME\r\n");
            serviceTcp->txData(stringa.toAscii());
            return;
        }
        pConfig->sysRestore(TRUE,parametri[0],0);
        serviceTcp->txData(QByteArray("DONE \r\n"));
        return;
    }

}

/*
 *  Funzione per la visualizzazione dei comandi a shell disponibili
 */
void serverDebug::handleList(void)
{
    serviceTcp->txData(QByteArray("-----------------------------------------------------\r\n"));
    serviceTcp->txData(QByteArray("system: -------- Comandi generali di sistema \r\n"));
    serviceTcp->txData(QByteArray("drivers:  ------ Comandi diretti ai drivers \r\n"));
    serviceTcp->txData(QByteArray("collimatore: --- Comandi di gestione del collimatore \r\n"));
    serviceTcp->txData(QByteArray("compressore: --- Comandi di gestione del compressore \r\n"));
    serviceTcp->txData(QByteArray("rotazioni: ----- Comandi di gestione delle rotazioni \r\n"));
    serviceTcp->txData(QByteArray("potter: -------- Comandi di gestione delle funzioni potter\r\n"));
    serviceTcp->txData(QByteArray("aws: ----------- Comandi relativi alla Console \r\n"));
    serviceTcp->txData(QByteArray("loader: -------- Comandi relativi al loader \r\n"));
    serviceTcp->txData(QByteArray("biopsy: -------- Comandi di gestione torretta \r\n"));
    serviceTcp->txData(QByteArray("generatore: ---- Comandi relativi al generatore \r\n"));
    serviceTcp->txData(QByteArray("master: -------- Comandi shell su Master \r\n"));
    serviceTcp->txData(QByteArray("slave: --------- Comandi shell su Slave \r\n"));
    serviceTcp->txData(QByteArray("canopen: ------- Comandi shell su Slave \r\n"));
    serviceTcp->txData(QByteArray("analog: -------- Comandi macchina Analogica \r\n"));
    serviceTcp->txData(QByteArray("debug: --------- Comandi per operazioni di debug \r\n"));
    serviceTcp->txData(QByteArray("------------------------------------------------------\r\n"));

}




void serverDebug::handleAnalog(QByteArray data)
{

    unsigned char buffer[10];
    if(data.contains("?"))
    {
        serviceTcp->txData(QByteArray("---------------------------------------------------------\r\n"));
        serviceTcp->txData(QByteArray("openStudy    Apre lo studio  \r\n"));
        serviceTcp->txData(QByteArray("closeStudy   Chiude lo studio  \r\n"));
        serviceTcp->txData(QByteArray("getRev       Chiede la revisione corrente  \r\n"));
        serviceTcp->txData(QByteArray("getRad       <x1,x5,x25>   \r\n"));
        serviceTcp->txData(QByteArray("setField     <FRONT,MIDDLE,BACK,OPEN>  \r\n"));
        serviceTcp->txData(QByteArray("getAccessory Chiede codice accessorio  \r\n"));
        serviceTcp->txData(QByteArray("getCassette  Chiede stato cassetta  \r\n"));
        serviceTcp->txData(QByteArray("setCassette   <B/R> B=Busy, R=Ready \r\n"));
        serviceTcp->txData(QByteArray("testGrid      n n=Cycles number \r\n"));
        serviceTcp->txData(QByteArray("setOfs        <val>    \r\n"));
        serviceTcp->txData(QByteArray("getOfs        Chiede il registro offset   \r\n"));
        serviceTcp->txData(QByteArray("autoOfs       Azzera automaticamente   \r\n"));
        serviceTcp->txData(QByteArray("initRtc       Inizializza RTC   \r\n"));
        serviceTcp->txData(QByteArray("setRtc  [MON/TUE/../SUN] [2021/..] [01/12] [1/31] [00/23] [0/60][0/60] \r\n"));
        serviceTcp->txData(QByteArray("getRtc       chiede data   \r\n"));
        serviceTcp->txData(QByteArray("---------------------------------------------------------\r\n"));
    }else if(data.contains("openStudy")){
        if(paginaOpenStudyAnalogic->openPageRequest()) serviceTcp->txData(QByteArray("DONE \r\n"));
        else serviceTcp->txData(QByteArray("ERROR! \r\n"));

    }else if(data.contains("closeStudy")){
        if(paginaOpenStudyAnalogic->closePageRequest()) serviceTcp->txData(QByteArray("DONE \r\n"));
        else serviceTcp->txData(QByteArray("ERROR! \r\n"));

    }else if(data.contains("setCassette")){
        QList<QByteArray> parametri;
        parametri = getNextFieldsAfterTag(data, QString("setCassette"));
        if(parametri.size()!=1) {
            serviceTcp->txData(QByteArray("MISSING PARAMETER \r\n"));
            return;
        }

        if(parametri[0]=="B") buffer[1]=1;
        else buffer[1]=0;
        buffer[0] = MCC_PCB244_A_SET_CASSETTE;
        pConsole->pGuiMcc->sendFrame(MCC_244_A_FUNCTIONS,1,buffer,2);
        serviceTcp->txData(QByteArray("DONE \r\n"));
        return;
    }else if(data.contains("testGrid")){
        QList<QByteArray> parametri;
        parametri = getNextFieldsAfterTag(data, QString("testGrid"));
        if(parametri.size()!=1) {
            serviceTcp->txData(QByteArray("MISSING CYCLES NUMBER \r\n"));
            return;
        }

        buffer[1]=parametri[0].toInt();
        if(parametri[0].toInt()>255)    buffer[1]=255;

        buffer[0] = MCC_PCB244_A_ACTIVATE_GRID;
        pConsole->pGuiMcc->sendFrame(MCC_244_A_FUNCTIONS,1,buffer,2);
        serviceTcp->txData(QByteArray("DONE \r\n"));
        return;
    }else if(data.contains("getRad")){
        QList<QByteArray> parametri;
        parametri = getNextFieldsAfterTag(data, QString("getRad"));
        if(parametri.size()!=1) {
            serviceTcp->txData(QByteArray("MISSING PARAMETER \r\n"));
            return;
        }


        if(parametri[0]=="x1"){
            buffer[0] = MCC_PCB244_A_GET_RADx1;
            connect(pConsole,SIGNAL(mccPcb244ANotifySgn(unsigned char,unsigned char,QByteArray)),this,SLOT(PCB244A_Notify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
            pConsole->pGuiMcc->sendFrame(MCC_244_A_FUNCTIONS,1,buffer,1);
        }else if(parametri[0]=="x5"){
            buffer[0] = MCC_PCB244_A_GET_RADx5;
            connect(pConsole,SIGNAL(mccPcb244ANotifySgn(unsigned char,unsigned char,QByteArray)),this,SLOT(PCB244A_Notify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
            pConsole->pGuiMcc->sendFrame(MCC_244_A_FUNCTIONS,1,buffer,1);

        }else if(parametri[0]=="x25"){
            buffer[0] = MCC_PCB244_A_GET_RADx25;
            connect(pConsole,SIGNAL(mccPcb244ANotifySgn(unsigned char,unsigned char,QByteArray)),this,SLOT(PCB244A_Notify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
            pConsole->pGuiMcc->sendFrame(MCC_244_A_FUNCTIONS,1,buffer,1);

        }else {
            serviceTcp->txData(QByteArray("INVALID PARAMETER \r\n"));
            return;
        }

        serviceTcp->txData(QByteArray("Wait .. \r\n"));
    }else if(data.contains("getRev")){
        buffer[0] = MCC_PCB244_A_GET_REV;
        connect(pConsole,SIGNAL(mccPcb244ANotifySgn(unsigned char,unsigned char,QByteArray)),this,SLOT(PCB244A_Notify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
        pConsole->pGuiMcc->sendFrame(MCC_244_A_FUNCTIONS,1,buffer,1);

    }else if(data.contains("getAccessory")){
        buffer[0] = MCC_PCB244_A_GET_ID;
        connect(pConsole,SIGNAL(mccPcb244ANotifySgn(unsigned char,unsigned char,QByteArray)),this,SLOT(PCB244A_Notify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
        pConsole->pGuiMcc->sendFrame(MCC_244_A_FUNCTIONS,1,buffer,1);

    }else if(data.contains("getCassette")){
        buffer[0] = MCC_PCB244_A_GET_CASSETTE;
        connect(pConsole,SIGNAL(mccPcb244ANotifySgn(unsigned char,unsigned char,QByteArray)),this,SLOT(PCB244A_Notify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
        pConsole->pGuiMcc->sendFrame(MCC_244_A_FUNCTIONS,1,buffer,1);

    }else if(data.contains("setField")){
        QList<QByteArray> parametri;
        parametri = getNextFieldsAfterTag(data, QString("setField"));
        if(parametri.size()!=1) {
            serviceTcp->txData(QByteArray("MISSING PARAMETER \r\n"));
            return;
        }

        buffer[0] = MCC_PCB244_A_SET_FIELD;
        if(parametri[0]=="FRONT")          buffer[1] = _ANALOG_DETECTOR_FRONT_FIELD;
        else if(parametri[0]=="MIDDLE")    buffer[1] = _ANALOG_DETECTOR_MIDDLE_FIELD;
        else if(parametri[0]=="BACK")      buffer[1] = _ANALOG_DETECTOR_BACK_FIELD;
        else                               buffer[1] = 255; // Apre tutti i canali

        connect(pConsole,SIGNAL(mccPcb244ANotifySgn(unsigned char,unsigned char,QByteArray)),this,SLOT(PCB244A_Notify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
        pConsole->pGuiMcc->sendFrame(MCC_244_A_FUNCTIONS,1,buffer,2);

    }else if(data.contains("setOfs")){
        QList<QByteArray> parametri;
        parametri = getNextFieldsAfterTag(data, QString("setOfs"));
        if(parametri.size()!=1) {
            serviceTcp->txData(QByteArray("MISSING PARAMETER \r\n"));
            return;
        }

        buffer[0] = MCC_PCB244_A_SET_OFFSET;
        buffer[1] = 1; // SET
        buffer[2] = (unsigned char) parametri[0].toInt();
        buffer[3] = (unsigned char)  (parametri[0].toInt() >>8);

        connect(pConsole,SIGNAL(mccPcb244ANotifySgn(unsigned char,unsigned char,QByteArray)),this,SLOT(PCB244A_Notify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
        pConsole->pGuiMcc->sendFrame(MCC_244_A_FUNCTIONS,1,buffer,4);

    }else if(data.contains("getOfs")){
        buffer[0] = MCC_PCB244_A_SET_OFFSET;
        buffer[1] = 0; // GET
        buffer[2] = 0;
        buffer[3] = 0;

        connect(pConsole,SIGNAL(mccPcb244ANotifySgn(unsigned char,unsigned char,QByteArray)),this,SLOT(PCB244A_Notify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
        pConsole->pGuiMcc->sendFrame(MCC_244_A_FUNCTIONS,1,buffer,4);
    }else if(data.contains("autoOfs")){

        buffer[0] = MCC_PCB244_A_SET_OFFSET;
        buffer[1] = 2; // AUTO OFFSET
        buffer[2] = 0;
        buffer[3] = 0;
        pConsole->pGuiMcc->sendFrame(MCC_244_A_FUNCTIONS,1,buffer,4);
        serviceTcp->txData(QByteArray("DONE \r\n"));
        return;
    }else if(data.contains("initRtc")){

        buffer[0] = 0;
        pConsole->pGuiMcc->sendFrame(MCC_RTC_COMMANDS,1,buffer,1);
        serviceTcp->txData(QByteArray("DONE \r\n"));
        return;
    }else if(data.contains("setRtc")){
        QList<QByteArray> parametri;
        parametri = getNextFieldsAfterTag(data, QString("setRtc"));
        if(parametri.size()!=7) {
            serviceTcp->txData(QByteArray("MISSING PARAMETER \r\n"));
            return;
        }

        buffer[0] = 1;

        if((parametri[0]=="LUN")||(parametri[0]=="MON")) buffer[1] = _RTC_MON;
        else if((parametri[0]=="MAR")||(parametri[0]=="TUE")) buffer[1] = _RTC_TUE;
        else if((parametri[0]=="MER")||(parametri[0]=="WED")) buffer[1] = _RTC_WED;
        else if((parametri[0]=="GIO")||(parametri[0]=="THU")) buffer[1] = _RTC_THU;
        else if((parametri[0]=="VEN")||(parametri[0]=="FRI")) buffer[1] = _RTC_FRI;
        else if((parametri[0]=="SAB")||(parametri[0]=="SAT")) buffer[1] = _RTC_SAT;
        else {
            serviceTcp->txData(QByteArray("WRONG DAY \r\n"));
            return;
        }

        buffer[2] = parametri[1].toInt() & 0xFF; // Anno
        buffer[3] = (parametri[1].toInt() >> 8) & 0xFF; // Anno
        buffer[4] = parametri[2].toInt(); // Mese
        buffer[5] = parametri[3].toInt(); // Giorno
        buffer[6] = parametri[4].toInt(); // Ora
        buffer[7] = parametri[5].toInt(); // Minuti
        buffer[8] = parametri[6].toInt(); // Secondi

        pConsole->pGuiMcc->sendFrame(MCC_RTC_COMMANDS,1,buffer,9);
        serviceTcp->txData(QByteArray("DONE \r\n"));
        return;
    }else if(data.contains("getRtc")){

        buffer[0] = 2;
        pConsole->pGuiMcc->sendFrame(MCC_RTC_COMMANDS,1,buffer,1);
        serviceTcp->txData(QByteArray("DONE \r\n"));
        return;
    }
}



void serverDebug::PCB244A_Notify(unsigned char id, unsigned char mcccode, QByteArray buffer)
{
    unsigned short uval;
    float val;
    float val1;


    //serviceTcp->txData(QByteArray("ARRIVATO\r\n"));
    disconnect(pConsole,SIGNAL(mccPcb244ANotifySgn(unsigned char,unsigned char,QByteArray)),this,SLOT(PCB244A_Notify(unsigned char,unsigned char,QByteArray)));
    if(buffer[0]!=0){
        serviceTcp->txData(QByteArray("COMMAND FAILED!\r\n"));
        return;
    }else{
        serviceTcp->txData(QByteArray("COMMAND:> "));
    }

    switch(mcccode){
    case MCC_PCB244_A_SET_AUTO_OFFSET:
        break;
    case MCC_PCB244_A_SET_OFFSET:
        if(buffer.size()<8) {
            serviceTcp->txData(QString("INVALID ANSWER FORMAT!\r\n").toAscii());
            break;
        }
        if(buffer[1]==0){
            // Read section
            uval = (buffer[2] + 256 * buffer[3]);
            val =  (float) (buffer[4] + 256 * buffer[5])/4;
            val1 =  (float) (buffer[6] + 256 * buffer[7])/4;
            serviceTcp->txData(QString("OFFSET=%1, VFreq_x1=%2(mV), VFreq_x5=%3(mV)\r\n").arg(uval).arg(val*5*1000*2/(1024)).arg(val1*5*1000/(4.9*1024)).toAscii());
        }else{
            // Write section
            uval = (buffer[2] + 256 * buffer[3]);
            val =  (float) (buffer[4] + 256 * buffer[5])/4;
            val1 =  (float) (buffer[6] + 256 * buffer[7])/4;
            serviceTcp->txData(QString("OFFSET=%1, VFreq_x1=%2(mV), VFreq_x5=%3(mV)\r\n").arg(uval).arg(val*5*1000*2/(1024)).arg(val1*5*1000/(4.9*1024)).toAscii());
        }

        break;
    case MCC_PCB244_A_GET_REV:
        if(buffer.size()<3) {
            serviceTcp->txData(QString("INVALID ANSWER FORMAT!\r\n").toAscii());
            break;
        }
        serviceTcp->txData(QString("REVISIONE PCB244A = %1.%2\r\n").arg((int)buffer[1]).arg((int)buffer[2]).toAscii());
        break;
    case MCC_PCB244_A_GET_RADx1:
        if(buffer.size()<3) {
            serviceTcp->txData(QString("INVALID ANSWER FORMAT!\r\n").toAscii());
            break;
        }
        val = ((float)buffer[1] + 256 * (float)buffer[2])/4;
        serviceTcp->txData(QString("RADx1=%1, VFreq=%2(mV)\r\n").arg(val).arg(val*5*1000*2/(1024)).toAscii());
        break;
    case MCC_PCB244_A_GET_RADx5:
        if(buffer.size()<3) {
            serviceTcp->txData(QString("INVALID ANSWER FORMAT!\r\n").toAscii());
            break;
        }
        val = ((float)buffer[1] + 256 * (float)buffer[2])/4;
        serviceTcp->txData(QString("RADx5=%1, VFreq=%2(mV)\r\n").arg(val).arg(val*5*1000/(4.9*1024)).toAscii());
        break;
    case MCC_PCB244_A_GET_RADx25:
        if(buffer.size()<3) {
            serviceTcp->txData(QString("INVALID ANSWER FORMAT!\r\n").toAscii());
            break;
        }
        val = ((float)buffer[1] + 256 * (float)buffer[2])/4;
        serviceTcp->txData(QString("RADx25=%1, VFreq=%2(mV)\r\n").arg(val).arg(val*5*1000/(4.9*4.9*1024)).toAscii());
        break;
    case MCC_PCB244_A_GET_CASSETTE:
        if(buffer.size()<3) {
            serviceTcp->txData(QString("INVALID ANSWER FORMAT!\r\n").toAscii());
            break;
        }
        if((buffer[1]) && (!buffer[2]))  serviceTcp->txData(QString("CASSETTA PRESENTE NON ESPOSTA\r\n").toAscii());
        else if((buffer[1]) && (buffer[2]))  serviceTcp->txData(QString("CASSETTA PRESENTE GIA ESPOSTA\r\n").toAscii());
        else     serviceTcp->txData(QString("MANCANZA CASSETTA\r\n").toAscii());
        break;
    case MCC_PCB244_A_GET_ID:
        if(buffer.size()<3) {
            serviceTcp->txData(QString("INVALID ANSWER FORMAT!\r\n").toAscii());
            break;
        }
        switch(buffer[1]){
        case POTTER_2D:
            if(buffer[2]==POTTER_DESCR_18x24)  serviceTcp->txData(QString("POTTER 18x24\r\n").toAscii());
            else serviceTcp->txData(QString("POTTER 24x30\r\n").toAscii());
            break;
        case POTTER_MAGNIFIER:
            serviceTcp->txData(QString("POTTER MAGNIFIER. MAG FACTOR = %1\r\n").arg((int) buffer[3]).toAscii());
            break;
        case POTTER_UNDEFINED:
            serviceTcp->txData(QString("POTTER UNDEFINED\r\n").toAscii());
            break;
        }

        break;
    }
}

//_____________________________________________________________________________________________________________________________        >>>>>> DEBUG GROUP
void serverDebug::handleDebug(QByteArray data)
{
    //int Generatore::getAecData(int plog, int modo_filtro, int odindex, int techmode, int* filtro,float* kV, int* dmAs, int* pulses){
    unsigned char buffer[10];
    if(data.contains("?"))
    {
        serviceTcp->txData(QByteArray("----------------------------------------------------------------------------------\r\n"));
        serviceTcp->txData(QByteArray("updateTS                                   Aggiorna M4+DBT\r\n"));
        serviceTcp->txData(QByteArray("updateGUI                                  Aggiorna solo DBT \r\n"));
        serviceTcp->txData(QByteArray("setCompressorNotify: <frame-gui-notify>    Simula ricezione Gui notify per compressore\r\n"));
        serviceTcp->txData(QByteArray("setActuatorEnableNotify: <frame-actuator-notify>      Simula ricezione actuator enable notify \r\n"));
        serviceTcp->txData(QByteArray("setAutoAlarm: classe, codice               Simula attivazione allarme automatico\r\n"));
        serviceTcp->txData(QByteArray("setAlarm: classe, codice                   Simula attivazione allarme non rirpistinabile\r\n"));
        serviceTcp->txData(QByteArray("setDatabaseI     code,val \r\n"));
        serviceTcp->txData(QByteArray("setDatabaseU     code,val \r\n"));
        serviceTcp->txData(QByteArray("setDatabaseS     code,val \r\n"));
        serviceTcp->txData(QByteArray("getDatabaseU     code \r\n"));
        serviceTcp->txData(QByteArray("testCmd          Esegue il comando di test \r\n"));
        serviceTcp->txData(QByteArray("freeze           Blocca ciclo automatico dei drivers\r\n"));
        serviceTcp->txData(QByteArray("run              Attiva ciclo automatico dei drivers\r\n"));
        serviceTcp->txData(QByteArray("setArmAng        Imposta l'angolo corrente\r\n"));
        serviceTcp->txData(QByteArray("generateAlarmList  \r\n"));
        serviceTcp->txData(QByteArray("getPulse  G/P, plog, A/F, odi, STD/LD/HC \r\n"));
        serviceTcp->txData(QByteArray("initAudio  \r\n"));
        serviceTcp->txData(QByteArray("playAudio  num vol\r\n"));
        serviceTcp->txData(QByteArray("mccTest  \r\n"));
        serviceTcp->txData(QByteArray("logFlush  \r\n"));
        serviceTcp->txData(QByteArray("----------------------------------------------------------------------------------\r\n"));
    }else if(data.contains("logFlush")){
        pSysLog->flush();
    }else if(data.contains("getPulse")){
        QList<QByteArray> parametri;
        parametri = getNextFieldsAfterTag(data, QString("getPulse"));
        if(parametri.size()!=5) return;

        if(parametri[0]=="G") pGeneratore->setFuocoGrande();
        else pGeneratore->setFuocoPiccolo();
        int plog= parametri[1].toInt();
        int af;
        if(parametri[2]=="A") af = ANALOG_FILTRO_AUTO;
        else af = ANALOG_FILTRO_FISSO;
        int odi = parametri[3].toInt();
        int tm;
        if(parametri[4]=="STD") tm = ANALOG_TECH_PROFILE_STD;
        else if(parametri[4]=="HC") tm = ANALOG_TECH_PROFILE_HC;
        else tm = ANALOG_TECH_PROFILE_LD;

        int filtro;
        float kvout;
        int dmasout;
        int pulses;
        int res = pGeneratore->pAECprofiles->getAecData(plog,af,4,odi,tm,pGeneratore->selectedAnodo, pGeneratore->selectedFSize, &filtro,&kvout,&dmasout,&pulses);
        QString val;
        if(res==0){
            QString flt;
            if(filtro==Collimatore::FILTRO_Mo) flt="Mo";
            else if(filtro==Collimatore::FILTRO_Rh) flt="Rh";
            else flt="ND";
             val = QString("OK: F=%1, KV=%2, DMAS=%3, P=%4\n\r").arg(flt).arg(kvout).arg(dmasout).arg(pulses);
        }else val = QString("NOK %1\n\r").arg(res);
        serviceTcp->txData(val.toAscii());

    } else if(data.contains("generateAlarmList"))
    {
        paginaAllarmi->exportMessageList();
        serviceTcp->txData(QByteArray("DONE\n\r"));
    } else if(data.contains("setCompressorNotify"))
    {
        handleSetCompressorNotify(data);
    }else if(data.contains("setActuatorEnableNotify"))
    {
        handleSetActuatorEnableNotify(data);
    }else if(data.contains("setAutoAlarm"))
    {
        handleSetAlarm(data,true);
    }else if(data.contains("setAlarm"))
    {
        handleSetAlarm(data,false);
    }else if(data.contains("updateTS")){
        if(pConfig->executeUpdateIde()==true){
          serviceTcp->txData(QByteArray("AGGIORNAMENTO IDE EFFETTUATO: SYSTEM REBOOT ...\n\r"));
        }
    }else if(data.contains("updateGUI")){
        if(pConfig->executeUpdateGui()==true){
          serviceTcp->txData(QByteArray("AGGIORNAMENTO GUI EFFETTUATO: SYSTEM REBOOT ...\n\r"));
        }
    }else if(data.contains("setDatabaseI")){
        handleSetDatabaseI(data);
   }else if(data.contains("setDatabaseU")){
       handleSetDatabaseU(data);
   }else if(data.contains("setDatabaseS")){
       handleSetDatabaseS(data);
   }else if(data.contains("getDatabaseU")){
        QList<QByteArray> parametri;
        parametri = getNextFieldsAfterTag(data, QString("getDatabaseU"));
        if(parametri.size()!=1) return;
        int indice = parametri[0].toInt();
        QString val = QString("DATABASE-%1 = %2\n\r").arg(indice).arg(ApplicationDatabase.getDataU(indice));
        serviceTcp->txData(val.toAscii());

    }else if(data.contains("setArmAng")){
        QList<QByteArray> parametri;
        parametri = getNextFieldsAfterTag(data, QString("setArmAng"));
        if(parametri.size()!=1) return;
        ApplicationDatabase.setData(_DB_DANGOLO, (int) parametri[0].toInt());
        serviceTcp->txData(QByteArray("OK\n\r"));

    }else if(data.contains("freeze")) handleDriverFreeze(TRUE);
    else if(data.contains("run")) handleDriverFreeze(FALSE);
    else if(data.contains("testCmd")){
        if(pConfig->startupCompleted) serviceTcp->txData(QByteArray("STARTUP OK\n\r"));
        else serviceTcp->txData(QByteArray("STARTUP NOK\n\r"));
   }else if(data.contains("mccTest")){
        QList<QByteArray> parametri;
        parametri = getNextFieldsAfterTag(data, QString("mccTest"));

        buffer[0] = parametri[0].toInt();
        if(parametri.size()==2) buffer[1] = parametri[1].toInt();
        else buffer[1]=0;

        pConsole->pGuiMcc->sendFrame(MCC_TEST,1,buffer,2);
        serviceTcp->txData(QByteArray("TEST COMMAND OK\n\r"));

   }else if(data.contains("initAudio")){
        buffer[0] = 0;
        buffer[1] = 0;
        pConsole->pGuiMcc->sendFrame(MCC_AUDIO,1,buffer,2);
        serviceTcp->txData(QByteArray("AUDIO INIT OK\n\r"));

    }else if(data.contains("playAudio")){
        QList<QByteArray> parametri;
        parametri = getNextFieldsAfterTag(data, QString("playAudio"));
        if(parametri.size()!=2){
            serviceTcp->txData(QByteArray("Missing message number\n\r"));
            return;
        }

        buffer[0] = 1; // Codice x riproduzione messaggi audio
        buffer[1] = (unsigned char) parametri[0].toInt(); // Codice messaggio
        buffer[2] = (unsigned char) parametri[1].toInt(); // Volume
        pConsole->pGuiMcc->sendFrame(MCC_AUDIO,1,buffer,3);
        serviceTcp->txData(QByteArray("PLAYING..\n\r"));
    }
}

void serverDebug::handleSetCompressorNotify(QByteArray data){
    QList<QByteArray> parametri;

#ifndef __COMPRESSOR_PCB215_NOTIFY_STUB
    serviceTcp->txData(QByteArray("COMPILAZIONE NON ABILITATA\n\r"));
    return;
#else

    parametri = getNextFieldsAfterTag(data, QString("setCompressorNotify"));
    if(parametri.size()!=7){
        serviceTcp->txData(QByteArray("FLAG0, FLAG1, FORZA, SPESSORE, PAD, TARGET, POSITION!\n\r"));
        return;
    }
    QByteArray buffer;
    buffer.append((unsigned char) parametri[0].toUShort());
    buffer.append((unsigned char) parametri[1].toUShort());
    buffer.append((unsigned char) (parametri[2].toUShort()&0x00FF));
    buffer.append((unsigned char) (parametri[2].toUShort()>>8));
    buffer.append((unsigned char) parametri[3].toUShort());
    buffer.append((unsigned char) parametri[4].toUShort());
    buffer.append((unsigned char) (parametri[5].toUShort()&0x00FF));
    buffer.append((unsigned char) (parametri[5].toUShort()>>8));


    pCompressore->pcb215Notify(0, PCB215_NOTIFY_COMPR_DATA, buffer);
    serviceTcp->txData(QByteArray("DONE!\n\r"));
#endif
}

void serverDebug::handleSetActuatorEnableNotify(QByteArray data){
    QList<QByteArray> parametri;

    parametri = getNextFieldsAfterTag(data, QString("setActuatorEnableNotify"));
    if(parametri.size()!=3){
        serviceTcp->txData(QByteArray("ROTENA, PENDENA, LIFTENA \n\r"));
        return;
    }
    QByteArray buffer;
    buffer.append((unsigned char) parametri[0].toUShort());
    buffer.append((unsigned char) parametri[1].toUShort());
    buffer.append((unsigned char) parametri[2].toUShort());

    pConsole->emitMccActuatorNotify(1, ACTUATOR_STATUS,buffer);
    serviceTcp->txData(QByteArray("DONE!\n\r"));

}

void serverDebug::handleSetAlarm(QByteArray data,bool selfreset)
{
    QList<QByteArray> parametri;
    int classe, codice;

    if(selfreset)
        parametri = getNextFieldsAfterTag(data, QString("setAutoAlarm"));
    else
        parametri = getNextFieldsAfterTag(data, QString("setAlarm"));


    if(parametri.size()==2){
        classe = parametri[0].toInt();
        codice = parametri[1].toInt();
    }else{
        serviceTcp->txData("PARAMETRI NON CORRETTI \n");
        return;
    }

    PageAlarms::debugActivateNewAlarm(classe, codice,selfreset);
}

void serverDebug::handleSetDatabaseI(QByteArray data){
    QList<QByteArray> parametri;
    parametri = getNextFieldsAfterTag(data, QString("setDatabaseI"));
    if(parametri.size()<2){
        serviceTcp->txData("ERRORI PARAMETRI \n");
        return;
    }

    ApplicationDatabase.setData(parametri[0].toInt(), (int) parametri[1].toInt(),0);

}
void serverDebug::handleSetDatabaseU(QByteArray data){
    QList<QByteArray> parametri;
    parametri = getNextFieldsAfterTag(data, QString("setDatabaseU"));
    if(parametri.size()<2){
        serviceTcp->txData("ERRORI PARAMETRI \n");
        return;
    }

    ApplicationDatabase.setData(parametri[0].toInt(), (unsigned char) parametri[1].toInt(),0);
}
void serverDebug::handleSetDatabaseS(QByteArray data){
    QList<QByteArray> parametri;
    parametri = getNextFieldsAfterTag(data, QString("setDatabaseS"));



    QString val;
    if(parametri.size()==1) val="";
    else{
        for(int ciclo=1;ciclo<parametri.size();ciclo++ ){
            val.append(parametri[ciclo]);
        }
    }
    ApplicationDatabase.setData(parametri[0].toInt(), val,0);
}

// COMANDA L'ATTIVAZIONE DEL MODO FREEZE (ON/OFF)
// data[0] = 1->ON, 0->OFF
// RISPOSTA:
// BUFFER[0]= RISULTATO
// BUFFER[1] = STATO ESEGUITO
void serverDebug::handleDriverFreeze(bool stat)
{
    QByteArray val;
    val.clear();

    if(stat) val.append((char) 1);
    else val.append((char) 0);

    if(mccService(1,SRV_FREEZE_DEVICE,val)== FALSE) serviceTcp->txData("MCC FALLITO");
    else connect(pConsole,SIGNAL(mccServiceNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(handleDriverFreezeNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

}

void serverDebug::handleDriverFreezeNotify(unsigned char id,unsigned char cmd, QByteArray data)
{
    if(cmd!=SRV_FREEZE_DEVICE) return;
    disconnect(pConsole,SIGNAL(mccServiceNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(handleDriverFreezeNotify(unsigned char,unsigned char,QByteArray)));
    if(data.at(0)==0) serviceTcp->txData("DRIVER FREEZE FALLITO");
    else
    {
        if(data.at(1)) serviceTcp->txData("DRIVER FREEZED ");
        else serviceTcp->txData("DRIVER RUN ");
    }
}
//_____________________________________________________________________________________________________________________________        >>>>>> COMPRESSORE GROUP

void serverDebug::handleCompressore(QByteArray data)
{
    if(data.contains("?"))
    {
        serviceTcp->txData(QByteArray("----------------------------------------------------------------------------------\r\n"));
        serviceTcp->txData(QByteArray("setCalibPos: <offset, klin>  modifica la calibrazione della nacchera \r\n"));
        serviceTcp->txData(QByteArray("getCalibPos:                 restituisce il valore corrente di calibrazione nacchera \r\n"));
        serviceTcp->txData(QByteArray("getPadList:                  restituisce una lista dei pad configurabili \r\n"));
        serviceTcp->txData(QByteArray("                             Un asterisco indica quali pad risultano configurati \r\n"));
        serviceTcp->txData(QByteArray("setCalibPad: <ofs,kF,peso>   modifica la calibrazione della nacchera \r\n"));
        serviceTcp->txData(QByteArray("getCalibPad:                 restituisce il valore corrente di calibrazione del pad corrente \r\n"));
        serviceTcp->txData(QByteArray("setThick: <val>              corregge il calcolo dello spessore alla compressione corrente \r\n"));
        serviceTcp->txData(QByteArray("setKF: <val>                 imposta il coefficiente di flessione per correggere lo spessore\r\n"));
        serviceTcp->txData(QByteArray("setWeight: <val>             imposta il peso in (N) \r\n"));
        serviceTcp->txData(QByteArray("STORE:                       Salva i dati nel file di configurazione \r\n"));
        serviceTcp->txData(QByteArray("getBattery:                  Legge il valore della tensione di batteria \r\n"));
        serviceTcp->txData(QByteArray("readPadConfig:               Legge il file padcalib.cnf e aggiorna i drivers \r\n"));
        serviceTcp->txData(QByteArray("storePadConfid:              Salva i dati di configurazione PAD nel file padclib.cnf \r\n"));
        serviceTcp->txData(QByteArray("calibThresholds: <val>       Calcola le soglie ideali per il riconoscimento PAD \r\n"));
        serviceTcp->txData(QByteArray("getTrolley                   Restituisce lo spessore anche non in compressione \r\n"));
        serviceTcp->txData(QByteArray("setCompressorRelease <val>   Imposta lo stato del rilascio dopo esposizione \r\n"));

        serviceTcp->txData(QByteArray("----------------------------------------------------------------------------------\r\n"));
    } else if(data.contains("getTrolley"))
    {
        // Invio comando
        unsigned char data=0;
        if(pConsole->pGuiMcc->sendFrame(MCC_GET_TROLLEY,1,&data, 1)==TRUE)
        {
            serviceTcp->txData(QByteArray("Comando in esecuzione..... \r\n"));
            connect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(handleGetTrolleyNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
        }
        return;

    }else if(data.contains("setCompressorRelease"))
    {
        handleSetCompressorRelease(data);
    } else if(data.contains("setThick"))
    {
        handleSetThick(data);
    }else if(data.contains("setKF"))
    {
        handleSetKF(data);
    }else if(data.contains("setWeight"))
    {
        handleSetPeso(data);
    }else if(data.contains("getCalibPad"))
    {
        handleGetCalibPad(data);
    }else if(data.contains("setCalibPad"))
    {
        handleSetCalibPad(data);
    }else if(data.contains("getCalibPos"))
    {
        handleGetCalibNacchera();
    }else if(data.contains("setCalibPos"))
    {
        handleSetCalibNacchera(data);
    }else if(data.contains("getPadList"))
    {
        handleGetPadList();
    }else if(data.contains("STORE"))
    {
        pCompressore->storeConfigFile();
        serviceTcp->txData(QByteArray("DATI SALVATI IN CONFIGURAZIONE!\r\n"));
    }else if(data.contains("getBattery"))
    {
        serviceTcp->txData(QString("Battery: %1(V)\r\n").arg(pCompressore->battery).toAscii());
    }else if(data.contains("readPadConfig"))
    {
        pCompressore->readPadCfg();
        pConfig->updatePCB269();

        // Visualizza il contenuto
        for(int i=0; i<10; i++){
            serviceTcp->txData(QString("THRESHOLD[%1]=%2!\r\n").arg(i).arg(pCompressore->config.thresholds[i]).toAscii());
        }
    }else if(data.contains("storePadConfig"))
    {
        pCompressore->storePadCfg();
        serviceTcp->txData(QByteArray("DATI SALVATI NEL FILE PADCALIB.cnf! \r\n"));
    }else if(data.contains("calibThresholds"))
    {
        handleCalibThresholds(data);

        // Visualizza il contenuto
        for(int i=0; i<10; i++){
            serviceTcp->txData(QString("THRESHOLD[%1]=%2!\r\n").arg(i).arg(pCompressore->config.thresholds[i]).toAscii());
        }

        serviceTcp->txData(QByteArray("I dati devono essere salvati e riletti perchÃ¨ siano operativi \r\n"));
    }



}

void serverDebug::handleGetPadList(void)
{
    int i;
    QString stringa;

    serviceTcp->txData(QString("CODE  NAME\n\r").arg(stringa).toAscii());
    for(i=0; i<PAD_ENUM_SIZE; i++)
    {
        if(i>9) stringa = QString("%1)   %2").arg((int) i).arg( pCompressore->getPadName((Pad_Enum) i));
        else stringa = QString("%1)    %2").arg((int) i).arg( pCompressore->getPadName((Pad_Enum) i));
        if(
            (pCompressore->config.pads[i].offset != pCompressore->defPad.offset) ||
            (pCompressore->config.pads[i].kF != pCompressore->defPad.kF) ||
            (pCompressore->config.pads[i].peso != pCompressore->defPad.peso)
           ) stringa.append("  <*>");
        serviceTcp->txData(QString("%1\n\r").arg(stringa).toAscii());
    }
    return;
}


void serverDebug::handleGetCalibNacchera(void)
{
    serviceTcp->txData(QString("OFFSET=%1, kLIN=%2\n\r").arg(pCompressore->config.calibPosOfs).arg(pCompressore->config.calibPosK).toAscii());
    return;
}

void serverDebug::handleSetCalibNacchera(QByteArray data)
{
    QList<QByteArray> parametri;
    int i;

    parametri = getNextFieldsAfterTag(data, QString("setCalibPos"));
    if(parametri.size()!=2) serviceTcp->txData(QByteArray("PARAMETRI ERRATI!\n\r"));

    // Calcola l'offset da aggiungere
    pCompressore->config.calibPosOfs = parametri[0].toInt();
    pCompressore->config.calibPosK = parametri[1].toInt();

    // rende effettivi i valori di calibrazione
    unsigned char buf[4];
    buf[0] = 2; // Comando di calibrazione Pads
    buf[1] = (unsigned char) (pCompressore->config.calibPosOfs);
    buf[2] = (unsigned char) (pCompressore->config.calibPosOfs >> 8);
    buf[3] = (unsigned char) pCompressore->config.calibPosK;
    pConsole->pGuiMcc->sendFrame(MCC_CMD_PCB215_CALIB,1,buf,sizeof(buf));

    return;

}


void serverDebug::handleGetCalibPad(QByteArray data)
{
    int i;

    // Controlla che ci sia un valido PAD selezionato
    if(pCompressore->isValidPad()==FALSE)
    {
        serviceTcp->txData(QByteArray("Nessun Pad valido risulta risconosciuto dal sistema!\n\r"));
        return;
    }

    // Calcola l'offset da aggiungere
    int pad = pCompressore->getPad();
    QString padName = pCompressore->getPadName();
    serviceTcp->txData(QString("PAD:%1, OFFSET=%2, kF=%3, PESO=%4\n\r").arg(padName).arg(pCompressore->config.pads[pad].offset).arg(pCompressore->config.pads[pad].kF).arg(pCompressore->config.pads[pad].peso).toAscii());

    return;

}

void serverDebug::handleSetCalibPad(QByteArray data)
{
    QList<QByteArray> parametri;
    QString frame;
    int i;

    parametri = getNextFieldsAfterTag(data, QString("setCalibPad"));
    if(parametri.size()!=3)
    {
        frame = QString("Questa funzione imposta la calibrazione del pad attualmente utilizzato. \n\r"
                        "> setCalibPad offset,kF,peso(N)\n\r"
                        "  offset: Offset meccanico rispetto allo zero della nacchera\n\r"
                        "  kF: compensazione forza di compressione\n\r"
                        "  peso(N): peso nacchera+pad\n\r");
        serviceTcp->txData(frame.toAscii());
        return;
    }

    // Controlla che ci sia un valido PAD selezionato
    if(pCompressore->isValidPad()==FALSE)
    {
        serviceTcp->txData(QByteArray("Nessun Pad riconosciuto o configurato!\n\r"));
        return;
    }


    // Calcola l'offset da aggiungere
    int pad = pCompressore->getPad();
    pCompressore->config.pads[pad].offset = parametri[0].toInt();
    pCompressore->config.pads[pad].kF = parametri[1].toInt();
    pCompressore->config.pads[pad].peso = parametri[2].toInt();

    // rende effettivi i valori di calibrazione
    unsigned char buf[6];
    buf[0] = 4; // Comando di calibrazione Pads
    buf[1] = pad;
    buf[2] = (unsigned char) pCompressore->config.pads[pad].offset;
    buf[3] = (unsigned char) ((unsigned short) pCompressore->config.pads[pad].offset >> 8);
    buf[4] = (unsigned char) pCompressore->config.pads[pad].kF;
    buf[5] = (unsigned char) pCompressore->config.pads[pad].peso;
    pConsole->pGuiMcc->sendFrame(MCC_CMD_PCB215_CALIB,1,buf,sizeof(buf));

    serviceTcp->txData(QByteArray("PAD correttamente configurato e parametri operativi\n\r"));

    return;

}

void serverDebug::handleCalibThresholds(QByteArray data)
{
    QList<QByteArray> parametri;
    QString frame;

    parametri = getNextFieldsAfterTag(data, QString("calibThresholds"));
    if(parametri.size()!=1)
    {
        frame = QString("Questa funzione imposta la calibrazione delle soglie di rilevamento PAD. \n\r"
                        "> calibThreshold valore\n\r"
                        "  valore: Lettura registro PCB215 0x28 con sblocco inferiore nacchera aperto\n\r");
        serviceTcp->txData(frame.toAscii());
        return;
    }

    pCompressore->calibrateThresholds(parametri[0].toInt());
    return;

}


void serverDebug::handleSetCompressorRelease(QByteArray data)
{
    QList<QByteArray> parametri;
    bool value;
    unsigned char cpu_flags;

    parametri = getNextFieldsAfterTag(data, QString("setCompressorRelease"));
    if(parametri.size()!=1)
    {
        serviceTcp->txData(QByteArray("Wrong data forma: data=1-> enable, data=0 -> disable \r\n"));
        return;
    }

    if(parametri[0].toInt()==1) value=true;
    else value=false;
    if(value==pConfig->userCnf.enableSblocco) return;

    pConfig->userCnf.enableSblocco = value;
    if(value) ApplicationDatabase.setData(_DB_COMPRESSOR_UNLOCK,1,0);
    else  ApplicationDatabase.setData(_DB_COMPRESSOR_UNLOCK,0,0);


    serviceTcp->txData(QByteArray("COMPRESSOR MODE CHANGED \r\n"));
    return;


}
void serverDebug::handleSetThick(QByteArray data)
{
    QList<QByteArray> parametri;
    QString frame;
    parametri = getNextFieldsAfterTag(data, QString("setThick"));
    if(parametri.size()!=1)
    {
        frame = QString("Questa funzione imposta la calibrazione dell'offset del Pad selezionato.\n\r"
                        "> setThick <val>\n\r"
                        "  val: Valore in mm dello spessore reale\n\r"
                        "  Si consiglia di comprimere un fantoccio a circa 40/50 N.\n\r");
        serviceTcp->txData(frame.toAscii());
        return;
    }

    // Controlla che ci sia un valido PAD selezionato
    if(pCompressore->isValidPad()==FALSE)
    {
        serviceTcp->txData(QByteArray("Questa funzione puÃ² essere eseguita solo con un Pad configurato!\n\r"));
        return;
    }

    // Calcola l'offset da aggiungere
    int pad = pCompressore->getPad();
    int diff = pCompressore->breastThick - parametri[0].toInt();
    pCompressore->config.pads[pad].offset -= diff;

    // rende effettivi i valori di calibrazione
    unsigned char buf[6];
    buf[0] = 4; // Comando di calibrazione Pads
    buf[1] = pad;
    buf[2] = (unsigned char) pCompressore->config.pads[pad].offset;
    buf[3] = (unsigned char) ((unsigned short) pCompressore->config.pads[pad].offset >> 8);
    buf[4] = (unsigned char) pCompressore->config.pads[pad].kF;
    buf[5] = (unsigned char) pCompressore->config.pads[pad].peso;
    pConsole->pGuiMcc->sendFrame(MCC_CMD_PCB215_CALIB,1,buf,sizeof(buf));

    serviceTcp->txData(QString("PAD:%1, OFS:%2, kF:%3, PESO(N):%4\n\r").arg(pCompressore->getPadName()).arg(pCompressore->config.pads[pad].offset).arg(pCompressore->config.pads[pad].kF).arg(pCompressore->config.pads[pad].peso).toAscii());
    return;

}

void serverDebug::handleSetKF(QByteArray data)
{
    QList<QByteArray> parametri;
    QString frame;

    parametri = getNextFieldsAfterTag(data, QString("setKF"));
    if(parametri.size()!=1)
    {
        frame = QString("Questa funzione imposta la calibrazione della compensazione della forza (kF).\n\r"
                        "> setKF <val>\n\r"
                        "  val: coefficiente [0:255]\n\r");
        serviceTcp->txData(frame.toAscii());
        return;
    }

    // Controlla che ci sia un valido PAD selezionato
    if(pCompressore->isValidPad()==FALSE)
    {
        serviceTcp->txData(QByteArray("Questa funzione puÃ² essere eseguita solo con un Pad configurato!\n\r"));
        return;
    }

    // Calcola kF
    int pad = pCompressore->getPad();
    pCompressore->config.pads[pad].kF = parametri[0].toInt();

    // rende effettivi i valori di calibrazione
    unsigned char buf[6];
    buf[0] = 4; // Comando di calibrazione Pads
    buf[1] = pad;
    buf[2] = (unsigned char) pCompressore->config.pads[pad].offset;
    buf[3] = (unsigned char) ((unsigned short) pCompressore->config.pads[pad].offset >> 8);
    buf[4] = (unsigned char) pCompressore->config.pads[pad].kF;
    buf[5] = (unsigned char) pCompressore->config.pads[pad].peso;
    pConsole->pGuiMcc->sendFrame(MCC_CMD_PCB215_CALIB,1,buf,sizeof(buf));

    serviceTcp->txData(QString("PAD:%1, OFS:%2, kF:%3, PESO(N):%4\n\r").arg(pCompressore->getPadName()).arg(pCompressore->config.pads[pad].offset).arg(pCompressore->config.pads[pad].kF).arg(pCompressore->config.pads[pad].peso).toAscii());

    return;

}

void serverDebug::handleSetPeso(QByteArray data)
{
    QList<QByteArray> parametri;
    QString frame;
    parametri = getNextFieldsAfterTag(data, QString("setWeight"));
    if(parametri.size()!=1)
    {
        frame = QString("Questa funzione imposta la calibrazione del peso della nacchera+Pad.\n\r"
                        "> setPeso <val>\n\r"
                        "  val: peso in (N)\n\r");
        serviceTcp->txData(frame.toAscii());
        return;
    }

    // Controlla che ci sia un valido PAD selezionato
    if(pCompressore->isValidPad()==FALSE)
    {
        serviceTcp->txData(QByteArray("Questa funzione puÃ² essere eseguita solo con un Pad configurato!\n\r"));
        return;
    }


    // Calcola l'offset da aggiungere
    int pad = pCompressore->getPad();
    pCompressore->config.pads[pad].peso = parametri[0].toInt();

    // rende effettivi i valori di calibrazione
    unsigned char buf[6];
    buf[0] = 4; // Comando di calibrazione Pads
    buf[1] = pad;
    buf[2] = (unsigned char) pCompressore->config.pads[pad].offset;
    buf[3] = (unsigned char) ((unsigned short) pCompressore->config.pads[pad].offset >> 8);
    buf[4] = (unsigned char) pCompressore->config.pads[pad].kF;
    buf[5] = (unsigned char) pCompressore->config.pads[pad].peso;
    pConsole->pGuiMcc->sendFrame(MCC_CMD_PCB215_CALIB,1,buf,sizeof(buf));

    serviceTcp->txData(QString("PAD:%1, OFS:%2, kF:%3, PESO(N):%4\n\r").arg(pCompressore->getPadName()).arg(pCompressore->config.pads[pad].offset).arg(pCompressore->config.pads[pad].kF).arg(pCompressore->config.pads[pad].peso).toAscii());
    return;

}

/*
 */
void serverDebug::handleSetCalibTomo(QByteArray data)
{
    QList<QByteArray> parametri;
    QString frame;
    colliTomoConf_Str* pTomo;
    unsigned char* pLama;

    parametri = getNextFieldsAfterTag(data, QString("setCalibTomo"));
    if(parametri.size()!=28)
    {
        frame = QString("Numero di parametri errato.\n\r");
        serviceTcp->txData(frame.toAscii());
        return;
    }

    if(parametri.at(0)=="W") pTomo = &pCollimatore->colliConf.colliTomoW;
    else if(parametri.at(0)=="Mo") pTomo = &pCollimatore->colliConf.colliTomoMo;
    else {
        frame = QString("Anodo errato.\n\r");
        serviceTcp->txData(frame.toAscii());
        return ;
    }

    if((parametri.at(1)=="L") &&  (parametri.at(2)=="P")) pLama = (pTomo->tomoLeftBladeP);
    else if((parametri.at(1)=="L") &&  (parametri.at(2)=="N")) pLama = (pTomo->tomoLeftBladeN);
    else if((parametri.at(1)=="R") &&  (parametri.at(2)=="P")) pLama = (pTomo->tomoRightBladeP);
    else if((parametri.at(1)=="R") &&  (parametri.at(2)=="N")) pLama = (pTomo->tomoRightBladeN);
    else if((parametri.at(1)=="T") &&  (parametri.at(2)=="P")) pLama = (pTomo->tomoBackTrapP);
    else if((parametri.at(1)=="T") &&  (parametri.at(2)=="N")) pLama = (pTomo->tomoBackTrapN);
    else return ;

    for(int i=0; i<COLLI_DYNAMIC_SAMPLES; i++) pLama[i] = (unsigned char) parametri.at(3+i).toInt();

    serviceTcp->txData(QString("PARAMETRI aggiornati (DOWNLOAD PER SCARICARLI)\n\r").toAscii());
    return;

}

void serverDebug::handleGetCalib(QByteArray data)
{
    QList<QByteArray> parametri;
    QString stringa,tag;
    int i;

    if(pCollimatore->colli_model == _COLLI_TYPE_ASSY_01) printf("COLLIMATOR MODEL: ASSY-01\n");
    else printf("COLLIMATOR MODEL: ASSY-02\n");

    for(i=0; i<4; i++){
        if(pCollimatore->colliConf.filterType[i]==Collimatore::FILTRO_Rh) tag = "Rh";
        else if(pCollimatore->colliConf.filterType[i]==Collimatore::FILTRO_Ag) tag="Ag" ;
        else if(pCollimatore->colliConf.filterType[i]==Collimatore::FILTRO_Al) tag="Al" ;
        else if(pCollimatore->colliConf.filterType[i]==Collimatore::FILTRO_Mo) tag="Mo" ;
        else tag="Cu";
        stringa = QString("FILTRO-%1:%2, %3\n\r").arg(i).arg(pCollimatore->colliConf.filterPos[i]).arg(tag);
        serviceTcp->txData(stringa.toAscii());
    }

    // Hotfix 11C
    stringa = QString("TOMO-FILTER: ");
    for(i=0; i<4; i++)  stringa.append(QString("%1  ").arg(pCollimatore->colliConf.filterTomo[i]));
    stringa.append("\n\r");
    serviceTcp->txData(stringa.toAscii());

    if(pCollimatore->colli_model == _COLLI_TYPE_ASSY_01)
        stringa = QString("MIRROR STEPS:%1\n\r").arg(pCollimatore->colliConf.mirrorSteps_ASSY_01);
    else stringa = QString("MIRROR STEPS:%1\n\r").arg(pCollimatore->colliConf.mirrorSteps_ASSY_02);
    serviceTcp->txData(stringa.toAscii());

    for(i=0; i<pCollimatore->colliConf.colli2D.size(); i++){
        stringa = QString("%1,").arg(pCompressore->getPadTag((Pad_Enum) (pCollimatore->colliConf.colli2D[i].PadCode)).toAscii().data());
        stringa+=QString("L=%1,").arg(pCollimatore->colliConf.colli2D[i].L);
        stringa+=QString("R=%1,").arg(pCollimatore->colliConf.colli2D[i].R);
        stringa+=QString("F=%1,").arg(pCollimatore->colliConf.colli2D[i].F);
        stringa+=QString("B=%1,").arg(pCollimatore->colliConf.colli2D[i].B);
        stringa+=QString("T=%1\n\r").arg(pCollimatore->colliConf.colli2D[i].T);
        serviceTcp->txData(stringa.toAscii());
    }

    stringa = QString("OPEN: %1 %2 %3 %4 %5 \n\r").arg(pCollimatore->colliConf.colliOpen.L).arg(pCollimatore->colliConf.colliOpen.R).arg(pCollimatore->colliConf.colliOpen.F).arg(pCollimatore->colliConf.colliOpen.B).arg(pCollimatore->colliConf.colliOpen.T);
    serviceTcp->txData(stringa.toAscii());

    stringa = QString("\n\rTOMO-W-L-P: ");
    for(i=0; i<COLLI_DYNAMIC_SAMPLES;i++) stringa += QString("%1 ").arg(pCollimatore->colliConf.colliTomoW.tomoLeftBladeP[i]);
    serviceTcp->txData(stringa.toAscii());

    stringa = QString("\n\rTOMO-W-L-N: ");
    for(i=0; i<COLLI_DYNAMIC_SAMPLES;i++) stringa += QString("%1 ").arg(pCollimatore->colliConf.colliTomoW.tomoLeftBladeN[i]);
    serviceTcp->txData(stringa.toAscii());

    stringa = QString("\n\rTOMO-W-R-P: ");
    for(i=0; i<COLLI_DYNAMIC_SAMPLES;i++) stringa += QString("%1 ").arg(pCollimatore->colliConf.colliTomoW.tomoRightBladeP[i]);
    serviceTcp->txData(stringa.toAscii());

    stringa = QString("\n\rTOMO-W-R-N: ");
    for(i=0; i<COLLI_DYNAMIC_SAMPLES;i++) stringa += QString("%1 ").arg(pCollimatore->colliConf.colliTomoW.tomoRightBladeN[i]);
    serviceTcp->txData(stringa.toAscii());

    stringa = QString("\n\rTOMO-W-T-P: ");
    for(i=0; i<COLLI_DYNAMIC_SAMPLES;i++) stringa += QString("%1 ").arg(pCollimatore->colliConf.colliTomoW.tomoBackTrapP[i]);
    serviceTcp->txData(stringa.toAscii());

    stringa = QString("\n\rTOMO-W-T-N: ");
    for(i=0; i<COLLI_DYNAMIC_SAMPLES;i++) stringa += QString("%1 ").arg(pCollimatore->colliConf.colliTomoW.tomoBackTrapN[i]);
    serviceTcp->txData(stringa.toAscii());

    stringa = QString("\n\rTOMO-W-FB: %1, %2").arg(pCollimatore->colliConf.colliTomoW.tomoFront).arg(pCollimatore->colliConf.colliTomoW.tomoBack);
    serviceTcp->txData(stringa.toAscii());


    stringa = QString("\n\rTOMO-Mo-L-P: ");
    for(i=0; i<COLLI_DYNAMIC_SAMPLES;i++) stringa += QString("%1 ").arg(pCollimatore->colliConf.colliTomoMo.tomoLeftBladeP[i]);
    serviceTcp->txData(stringa.toAscii());

    stringa = QString("\n\rTOMO-Mo-L-N: ");
    for(i=0; i<COLLI_DYNAMIC_SAMPLES;i++) stringa += QString("%1 ").arg(pCollimatore->colliConf.colliTomoMo.tomoLeftBladeN[i]);
    serviceTcp->txData(stringa.toAscii());

    stringa = QString("\n\rTOMO-Mo-R-P: ");
    for(i=0; i<COLLI_DYNAMIC_SAMPLES;i++) stringa += QString("%1 ").arg(pCollimatore->colliConf.colliTomoMo.tomoRightBladeP[i]);
    serviceTcp->txData(stringa.toAscii());

    stringa = QString("\n\rTOMO-Mo-R-N: ");
    for(i=0; i<COLLI_DYNAMIC_SAMPLES;i++) stringa += QString("%1 ").arg(pCollimatore->colliConf.colliTomoMo.tomoRightBladeN[i]);
    serviceTcp->txData(stringa.toAscii());

    stringa = QString("\n\rTOMO-Mo-T-P: ");
    for(i=0; i<COLLI_DYNAMIC_SAMPLES;i++) stringa += QString("%1 ").arg(pCollimatore->colliConf.colliTomoMo.tomoBackTrapP[i]);
    serviceTcp->txData(stringa.toAscii());

    stringa = QString("\n\rTOMO-Mo-T-N: ");
    for(i=0; i<COLLI_DYNAMIC_SAMPLES;i++) stringa += QString("%1 ").arg(pCollimatore->colliConf.colliTomoMo.tomoBackTrapN[i]);
    serviceTcp->txData(stringa.toAscii());

    stringa = QString("\n\rTOMO-Mo-FB: %1, %2\n\r\n\r").arg(pCollimatore->colliConf.colliTomoMo.tomoFront).arg(pCollimatore->colliConf.colliTomoMo.tomoBack);
    serviceTcp->txData(stringa.toAscii());

}


// <pad,Mat,L,R,F,B,T>
void serverDebug::handleSetCalib2D(QByteArray data)
{
    QList<QByteArray> parametri;
    QString stringa;
    int i,code;


    parametri = getNextFieldsAfterTag(data, QString("setCalib2D"));
    if(parametri.size()==0)
    {
        serviceTcp->txData(QByteArray("PAD 2D DISPONIBILI:\n\r"));
        for(i=0; i<pCollimatore->colliConf.colli2D.size(); i++)
        {
            stringa = QString(" %1\n\r").arg(pCompressore->getPadTag((Pad_Enum) pCollimatore->colliConf.colli2D[i].PadCode));
            serviceTcp->txData(stringa.toAscii());
        }
        stringa = QString(" OPEN\n\r");
        serviceTcp->txData(stringa.toAscii());

        return;
    }

    if(parametri.size()<6)
    {
        serviceTcp->txData(QByteArray("PARAMETRI ERRATI!\n\r"));
        return;
    }

    // Verifica Pad OPEN
    if(parametri[0]=="OPEN")
    {
        pCollimatore->colliConf.colliOpen.L = parametri[1].toInt();
        pCollimatore->colliConf.colliOpen.R = parametri[2].toInt();
        pCollimatore->colliConf.colliOpen.F = parametri[3].toInt();
        pCollimatore->colliConf.colliOpen.B = parametri[4].toInt();
        pCollimatore->colliConf.colliOpen.T = parametri[5].toInt();
        stringa = QString("NUOVA COLLIMAZIONE OPEN: L=%1 R=%2 F=%3 B=%4 T=%5\n\r").arg(pCollimatore->colliConf.colliOpen.L).arg(pCollimatore->colliConf.colliOpen.R).arg(pCollimatore->colliConf.colliOpen.F).arg(pCollimatore->colliConf.colliOpen.B).arg(pCollimatore->colliConf.colliOpen.T);
        serviceTcp->txData(stringa.toAscii());
        return;
    }

    if(parametri.size()!=7)
    {
        serviceTcp->txData(QByteArray("PARAMETRI ERRATI!\n\r"));
        return;
    }


    // Verifica che la coppia pad+Anodo sia in configurazione

    // Cerca il codice associato al pad indicato
    code = pCompressore->getPadCodeFromTag(parametri[0]);
    if(code>=PAD_ENUM_SIZE)
    {
        serviceTcp->txData(QByteArray("PAD NON IDENTIFICATO\n\r"));
        return;
    }

    for(i=0; i<pCollimatore->colliConf.colli2D.size(); i++)
    {
        if(pCollimatore->colliConf.colli2D[i].PadCode == code)
        {
            pCollimatore->colliConf.colli2D[i].L = parametri[2].toInt();
            pCollimatore->colliConf.colli2D[i].R = parametri[3].toInt();
            pCollimatore->colliConf.colli2D[i].F = parametri[4].toInt();
            pCollimatore->colliConf.colli2D[i].B = parametri[5].toInt();
            pCollimatore->colliConf.colli2D[i].T = parametri[6].toInt();
            stringa = QString("NUOVA COLLIMAZIONE %1,%2: L=%3 R=%4 F=%5 B=%6 T=%7\n\r").arg(QString(parametri[0])).arg(QString(parametri[1])).arg(pCollimatore->colliConf.colli2D[i].L).arg(pCollimatore->colliConf.colli2D[i].R).arg(pCollimatore->colliConf.colli2D[i].F).arg(pCollimatore->colliConf.colli2D[i].B).arg(pCollimatore->colliConf.colli2D[i].T);
            serviceTcp->txData(stringa.toAscii());
            return;
        }
    }

    // Il Pad non era configurato: inserisce un nuovo campo alla configurazione
    _colliPadStr newColli2DItem;
    newColli2DItem.PadCode = code;
    newColli2DItem.L = parametri[2].toInt();
    newColli2DItem.R = parametri[3].toInt();
    newColli2DItem.F = parametri[4].toInt();
    newColli2DItem.B = parametri[5].toInt();
    newColli2DItem.T = parametri[6].toInt();
    pCollimatore->colliConf.colli2D.append(newColli2DItem);
    serviceTcp->txData(QByteArray("AGGIUNTO NUOVO PAD ALLA CONFIGURAZIONE DEI PADS\n\r"));

}

void serverDebug::handleSetCalibFiltro(QByteArray data)
{
    QList<QByteArray> parametri;
    QString tag;
    int i;


    parametri = getNextFieldsAfterTag(data, QString("setCalibFiltro"));
    if(parametri.size()!=2)
    {
        serviceTcp->txData(QByteArray("PARAMETRI ERRATI!\n\r"));
        return;
    }
    if(parametri[1].toInt()>255)
    {
        serviceTcp->txData(QByteArray("VALORE FUORI SCALA!\n\r"));
        return;
    }

    for(i=0; i<4; i++)
    {
        tag = pCollimatore->getFiltroTag(pCollimatore->colliConf.filterType[i]);
        if(parametri[0]==tag)
        {
            unsigned char buffer[4];
            pCollimatore->colliConf.filterPos[i] = parametri[1].toInt();
            buffer[0]=pCollimatore->colliConf.filterPos[0];
            buffer[1]=pCollimatore->colliConf.filterPos[1];
            buffer[2]=pCollimatore->colliConf.filterPos[2];
            buffer[3]=pCollimatore->colliConf.filterPos[3];

            // Invio comando
            pConsole->pGuiMcc->sendFrame(MCC_CALIB_FILTRO,1,buffer, 4);
            serviceTcp->txData(QString("NUOVA POSIZIONE %1:%2\n\r").arg(tag).arg(pCollimatore->colliConf.filterPos[i]).toAscii());
            return;
        }
    }
    serviceTcp->txData(QByteArray("FILTRO INESISTENTE\n\r"));

}

// Hotfix 11C
void serverDebug::handleSetCalibTomoFiltro(QByteArray data)
{
    QList<QByteArray> parametri;


    parametri = getNextFieldsAfterTag(data, QString("setCalibTomoFiltro"));
    if(parametri.size()!=4)
    {
        serviceTcp->txData(QByteArray("PARAMETRI ERRATI!\n\r"));
        return;
    }
    if(parametri[1].toInt()>255)
    {
        serviceTcp->txData(QByteArray("VALORE FUORI SCALA!\n\r"));
        return;
    }

    pCollimatore->colliConf.filterTomo[0] = parametri[0].toInt();
    pCollimatore->colliConf.filterTomo[1] = parametri[1].toInt();
    pCollimatore->colliConf.filterTomo[2] = parametri[2].toInt();
    pCollimatore->colliConf.filterTomo[3] = parametri[3].toInt();

    serviceTcp->txData(QByteArray("ESEGUITO: OCCORRE EFFETTUARE IL COMANDO STORE PER COMPLETARE!\n\r"));
    pConfig->updatePCB249U2();
}


void serverDebug::handleSetCalibMirror(QByteArray data)
{
    QList<QByteArray> parametri;

    parametri = getNextFieldsAfterTag(data, QString("setCalibMirror"));
    if(parametri.size()!=1)
    {
        serviceTcp->txData(QByteArray("PARAMETRI ERRATI!\n\r"));
        return;
    }

    if(pCollimatore->colli_model == _COLLI_TYPE_ASSY_01)
        pCollimatore->colliConf.mirrorSteps_ASSY_01 = parametri[0].toInt();
    else
        pCollimatore->colliConf.mirrorSteps_ASSY_02 = parametri[0].toInt();
    serviceTcp->txData(QString("NEW MIRROR STEP VALUE: %1\n\rDevice driver updated\n\r").arg(parametri[0].toInt()).toAscii());

    // Invio configurazione a dispositivo
    pConfig->updatePCB249U2();

    return;
}

// Handler connesso ai messaggi di log (qDebug)

// Handler messaggi spediti verso console: deve essere trasformato in UNICODE
void serverDebug::serviceTxConsoleHandler(QByteArray data)
{
    QTextCodec *codec = QTextCodec::codecForName(UNICODE_TYPE);
    QString stringa = codec->toUnicode(data);
    stringa = "AWS<" + stringa + "\r\n";
    serviceTcp->txData(stringa.toAscii());
    return;
}

// Handler messaggi ricevuti da console: deve essere trasformato in UNICODE
void serverDebug::serviceRxConsoleHandler(QByteArray data)
{
    QTextCodec *codec = QTextCodec::codecForName(UNICODE_TYPE);
    QString stringa = codec->toUnicode(data);
    stringa = "AWS>" + stringa + "\r\n";
    serviceTcp->txData(stringa.toAscii());
    return;
}

// Messaggi provenienti dalle qDebug di sistema: UNICODE
void serverDebug::serviceLogHandler(QByteArray data)
{
    QTextCodec *codec = QTextCodec::codecForName(UNICODE_TYPE);
    QString stringa = codec->toUnicode(data);
    stringa = "LOG:" + stringa + "\r\n";

    serviceTcp->txData(stringa.toAscii());
    return;
}

// Risposte asincrone verso Console: UNICODE
void serverDebug::serviceTxAsyncHandler(QByteArray data)
{

    QTextCodec *codec = QTextCodec::codecForName(UNICODE_TYPE);
    QString stringa = codec->toUnicode(data);
    stringa = "ASYNC AWS>" + stringa + "\r\n";

    serviceTcp->txData(stringa.toAscii());
     return;
}


void serverDebug::serviceErrorTxHandler(int codice, QString msg)
{
    QString stringa = "ERROR AWS>" + QString("%1:").arg(codice) + msg + "\r\n";
    serviceTcp->txData(stringa.toAscii());
    return;
}


bool serverDebug::mccService(int id, _MccServiceNotify_Code cmd, QByteArray data)
{
    data.prepend((unsigned char) cmd);
    return pConsole->pGuiMcc->sendFrame(MCC_SERVICE,id,(unsigned char*) data.data(), data.size());
}
bool serverDebug::mccService(_MccServiceNotify_Code cmd)
{
    QByteArray data;
    data.prepend((unsigned char) cmd);
    return pConsole->pGuiMcc->sendFrame(MCC_SERVICE,1,(unsigned char*) data.data(), data.size());
}

void serverDebug::handleGetInputs(void)
{
    disconnect(io,SIGNAL(ioUpdated()),this,SLOT(handleGetInputs()));


    return;
}


void serverDebug::handleGetOutputs(void)
{
    disconnect(io,SIGNAL(ioUpdated()),this,SLOT(handleGetOutputs()));


    return;
}

/*
 * Questa funzione forza l'impostazione del PAD come se fosse ricevuto da M4
 */
void serverDebug::handleSetPad(QByteArray data)
{
    int i;
    QByteArray buf;

    data=data.left(data.size()-2);
    data.append(" ");

    i=data.indexOf(":");
    data=data.right(data.size()-i-1);
    i=data.indexOf(" ");
    data=data.left(i);

    serviceTcp->txData(data);

    buf.append((char) pCompressore->comprFlags0);
    buf.append((char) pCompressore->comprFlags1);
    buf.append((char) pCompressore->comprStrenght);
    buf.append((char) pCompressore->breastThick&0xFF);
    buf.append((char) (pCompressore->breastThick>>8) &0xFF);
    buf.append((char) data.toInt());
    buf.append((char) 150);
    pCompressore->pcb215Notify(1,PCB215_NOTIFY_COMPR_DATA,buf);

    serviceTcp->txData(QString("OK %1\r\n").arg((unsigned char) data.toInt()).toAscii());
    return;
}

void serverDebug::handleSetPage(QByteArray data)
{
    int i;
    QByteArray buf;

    data=data.left(data.size()-2);
    data.append(" ");

    i=data.indexOf(":");
    data=data.right(data.size()-i-1);
    i=data.indexOf(" ");
    data=data.left(i);

    serviceTcp->txData(data);
    GWindowRoot.setNewPage(data.toInt(),GWindowRoot.curPage,0);

    return;
}

/*
 *  Questa funzione effettua il tunnel tra Service Terminal e Console
 *  Iniettando il messaggio ricevuto dal Service Terminal sul socket Console
 *  e quindi permettendo di nviare un comando Console da Terminale di servizio
 *  Il messaggio iniettato deve essere compliance con il protocollo della COnsole
 *  Sintax:
 *  console: <......%...%>
 */
void serverDebug::handleConsole(QByteArray data)
{
    if(data.contains("?"))
    {
        serviceTcp->txData(QByteArray("--------------------------------------------------------\r\n"));
        serviceTcp->txData(QByteArray("simulateRx <frame>      Simula ricezione comando da AWS \r\n"));
        serviceTcp->txData(QByteArray("getFormat <frame>       Restituisce info sul frame\r\n"));
        serviceTcp->txData(QByteArray("logOn                   Attiva Sniffer TCP AWS\r\n"));
        serviceTcp->txData(QByteArray("logOff                  Disattiva Sniffer TCP AWS\r\n"));
        serviceTcp->txData(QByteArray("pcPowerOff              Richiede spegnimento PC\r\n"));
        serviceTcp->txData(QByteArray("---------------------------------------------------------\r\n"));
    } else if(data.contains("simulateRx "))
    {
        QTextCodec *codec = QTextCodec::codecForName(UNICODE_TYPE);
        pConsole->consoleRxHandler(codec->fromUnicode(data));
    }else if(data.contains("getFormat"))
    {
        protoConsole messaggio(&data);
        if(messaggio.frame_len!=0)
        {
            serviceTcp->txData(QString("FRAME LENGTH:%1\r\n").arg(messaggio.frame_len).toAscii());
            serviceTcp->txData(QString("NUM PARAMETERS:%1\r\n").arg(messaggio.parametri.size()).toAscii());
            for(int i = 0; i< messaggio.parametri.size(); i++){
                serviceTcp->txData(QString("PAR[%1]= %2\r\n").arg(i).arg(messaggio.parametri[i]).toAscii());
            }
        }
        else serviceTcp->txData(QByteArray("MESSAGGIO NON VALIDO\r\n"));
    }else if(data.contains("logOff"))
    {
        serviceTcp->txData(QByteArray("<AWS TCP CHANNELS DISCONNECTED>\r\n"));

        disconnect(pConsole,SIGNAL(consoleTxHandler(QByteArray)),this,SLOT(serviceTxConsoleHandler(QByteArray)));
        disconnect(pConsole,SIGNAL(consoleRxSgn(QByteArray)),this,SLOT(serviceRxConsoleHandler(QByteArray)));
        disconnect(pToConsole,SIGNAL(logTxHandler(QByteArray)),this,SLOT(serviceLogHandler(QByteArray)));
        disconnect(pToConsole,SIGNAL(notificheTxHandler(QByteArray)),this,SLOT(serviceTxAsyncHandler(QByteArray)));        
        disconnect(paginaAllarmi,SIGNAL(newAlarmSgn(int,QString)),this,SLOT(serviceErrorTxHandler(int,QString)));

    }else if(data.contains("logOn"))
    {
        serviceTcp->txData(QByteArray("<AWS TCP CHANNELS CONNECTED>\r\n"));

        // Connessione con i canali di ricezione comandi da AWS
        connect(pConsole,SIGNAL(consoleTxHandler(QByteArray)),this,SLOT(serviceTxConsoleHandler(QByteArray)),Qt::UniqueConnection);
        connect(pConsole,SIGNAL(consoleRxSgn(QByteArray)),this,SLOT(serviceRxConsoleHandler(QByteArray)),Qt::UniqueConnection);

        // Connessione con il canale LOG verso AWS
        connect(pToConsole,SIGNAL(logTxHandler(QByteArray)),this,SLOT(serviceLogHandler(QByteArray)),Qt::UniqueConnection);

        // Connessione con il canale ASYNC verso AWS
        connect(pToConsole,SIGNAL(notificheTxHandler(QByteArray)),this,SLOT(serviceTxAsyncHandler(QByteArray)),Qt::UniqueConnection);

        // Connessione con il canale di comunicazioni errori verso AWS
        connect(paginaAllarmi,SIGNAL(newAlarmSgn(int,QString)),this,SLOT(serviceErrorTxHandler(int,QString)),Qt::UniqueConnection);


    }else if(data.contains("setMag")){
        ApplicationDatabase.setData(_DB_ACCESSORIO,(unsigned char) POTTER_MAGNIFIER,0);
    }else if(data.contains("set2D")){
        ApplicationDatabase.setData(_DB_ACCESSORIO,(unsigned char) POTTER_2D,0);
    }else if(data.contains("pcPowerOff")){
        pConfig->masterRequestAwsPowerOff();
    }



}

// TO BE DONE: get revisions
void serverDebug::handleGetRevisions(void)
{
    if(pConfig->checkPackage()==true) serviceTcp->txData("REVISIONI CORRETTE\n\r");
    else serviceTcp->txData(pConfig->revisionError.toAscii());
    serviceTcp->txData("\n\r");

}



/*
 *
 *  Attivazione procedura di rodaggio. Occorre spegnere la macchina per
 *  fermare la pendolazione

    buffer[0] = num cicli
    buffer[1] = angolo
    buffer[2] = velocita: 0 = STD, 1 = WIDE, 2 = NARROW
 */
void serverDebug::handleRodaggioTubo(QByteArray data)
{
    QList<QByteArray> parametri = getNextFieldsAfterTag(data, QString("LOOP"));

    if(parametri.size()!=3){
        serviceTcp->txData("Wrong parameters\n\r");
        return;
    }

    if(parametri[0].toInt()>255){
        serviceTcp->txData("Wrong num cycles\n\r");
        return;
    }
    if(parametri[1].toInt()>22){
        serviceTcp->txData("Wrong Angle\n\r");
        return;
    }
    if((parametri[2]!="N")&&(parametri[2]!="W")&&(parametri[2]!="S")){
        serviceTcp->txData("Wrong Speed\n\r");
        return;
    }

    QByteArray buf;
    buf.append((unsigned char) parametri[0].toInt());
    buf.append((unsigned char) parametri[1].toInt());
    if(parametri[2]=="S"){
        buf.append((char) 0);
    }else if(parametri[2]=="W"){
        buf.append((unsigned char) 1);
    }else if(parametri[2]=="N"){
        buf.append((unsigned char) 2);
    }

    if(mccService(0,SRV_RODAGGIO_TUBO,buf)== FALSE) serviceTcp->txData("MCC FALLITO");
    else serviceTcp->txData(QString("TRX LOOP START: %1 %2 %3\n\r").arg(buf[0]).arg(buf[1]).arg(buf[2]).toAscii());

}


void serverDebug:: handleReadConfig(QByteArray data)
{
    QString stringa;
    int i;
    bool ok;
    unsigned char indirizzo;
    unsigned char uC;
    QString filename;

    data=data.left(data.size()-2);
    data.append(" ");

    i = data.indexOf("CONFIG");
    data = data.right(data.size()-i-7);

    // Cerca Target hex format
    i = data.indexOf(",");
    if(i==-1) return;
    stringa = data.left(i);
    indirizzo = (unsigned char) stringa.toInt(&ok,16);
    data = data.right(data.size()-i-1);

    // Cerca uC Target (1/2)
    i = data.indexOf(" ");
    if(i==-1) return;
    stringa = data.left(i);
    uC = (unsigned char) stringa.toInt(&ok,0);


    if(pLoader->readConfig(indirizzo,uC)==FALSE)
    {
        serviceTcp->txData(QString("FALLITO COMANDO MCC").toAscii());
    }else
    {
        connect(pLoader,SIGNAL(readConfigSgn(_picConfigStr)),this,SLOT(handleReadConfigNotify(_picConfigStr)),Qt::UniqueConnection);
    }
    return;
}


// DATA.AT(0) == RISULTATO
// DATA.AT(1) == OPERAZIONE ESEGUITA
void serverDebug::handleReadConfigNotify(_picConfigStr data)
{

    disconnect(pLoader,SIGNAL(readConfigSgn(_picConfigStr)),this,SLOT(handleReadConfigNotify(_picConfigStr)));
    QString id0=QString::number(data.ID[0],16);
    QString id1=QString::number(data.ID[1],16);
    QString id2=QString::number(data.ID[2],16);
    QString id3=QString::number(data.ID[3],16);
    QString cw=QString::number(data.configWord,16);
    serviceTcp->txData(QString("ID0:"+id0 +" ID1:"+id1 +" ID2:"+id2 +" ID3:"+id3 +" CW:"+cw+ "\n\r").toAscii() );

}


/*
 *   loader: UPLOAD Target, uC, file
 */
void serverDebug:: handleLoaderUpload(QByteArray data)
{


    unsigned char target;
    unsigned char uC;
    QString filename,tag;
    QList<QByteArray> parametri;

    parametri = getNextFieldsAfterTag(data, QString("UPLOAD"));
    if(parametri.size()!=1)
    {
        serviceTcp->txData(QByteArray("wrong parameters\n\r"));
        return;
    }

    // Controllo indirizzo
    if(parametri[0]=="PCB249U1") {
        target = 0x1E;
        uC = 1;
        tag = "PCB249U1";
        filename = QString("/home/user/FW249U1.hex");
    }else if(parametri[0]=="PCB249U2") {
        target = 0x1E;
        uC = 2;
        tag = "PCB249U2";
        filename = QString("/home/user/FW249U2.hex");
    }else if(parametri[0]=="PCB240") {
        target = 0;
        uC = 1;
        tag = "PCB240";
        filename = QString("/home/user/FW240.hex");
    }else if(parametri[0]=="PCB244A") {
        target = 0x1A;
        uC = 1;
        tag = "PCB244A";
        filename = QString("/home/user/FW244A.hex");
    }else if(parametri[0]=="PCB190") {
        target = 0x1C;
        uC = 1;
        tag = "PCB190";
        filename = QString("/home/user/FW190A.hex");
    }else if(parametri[0]=="PCB269") {
        target = 0x1B;
        uC = 1;
        tag = "PCB269";
        filename = QString("/home/user/FW269.hex");
    }else{
        serviceTcp->txData(QByteArray("invalid target\n\r"));
        return;
    }


    pLoader->manualFirmwareUpload(target,uC,filename,tag);
    return;
}

/* Calcola il CRC di un file per il download
 * il file deve essere contenuto nella directory /home/user
 */
void serverDebug::handleGetCRC(QByteArray data){

    data=data.left(data.size()-2);
    data.append(" ");

    int i = data.indexOf("getCRC");
    data = data.right(data.size()-i-7);
    data.replace(" ","");

    QString rev;
    unsigned short crc;
    int ret = pLoader->verifyHeader(data,"",&crc,&rev);
    serviceTcp->txData(QString("FILE:" + data + QString("RETCODE=%1, CRC=%2,").arg(ret).arg(QString::number(crc,16)) + " REV="+rev+"\n\r").toAscii() );
}

void serverDebug::handleSetCRC(QByteArray data){
    QList<QByteArray> parametri;
    QString filename;
    parametri = getNextFieldsAfterTag(data, QString("setCRC"));
    if(parametri.size()!=2)
    {
        serviceTcp->txData(QByteArray("wrong parameters\n\r"));
        return;
    }

    // Controllo indirizzo
    if(parametri[0]=="PCB249U1") {
        filename = QString("/home/user/FW249U1.hex");
    }else if(parametri[0]=="PCB249U2") {
        filename = QString("/home/user/FW249U2.hex");
    }else if(parametri[0]=="PCB240") {
        filename = QString("/home/user/FW240.hex");
    }else if(parametri[0]=="PCB244A") {
        filename = QString("/home/user/FW244A.hex");
    }else if(parametri[0]=="PCB190") {
        filename = QString("/home/user/FW190A.hex");
    }else if(parametri[0]=="PCB269") {
        filename = QString("/home/user/FW269.hex");
    }else{
        serviceTcp->txData(QByteArray("invalid target\n\r"));
        return;
    }

    // Prepara il file
    QString command;
    QString rev;
    unsigned short crc;

    command = QString("echo \"<0x0000,%1>\" > /home/user/tmp").arg(QString(parametri[1]));
    system(command.toStdString().c_str());
    command = QString("cat %1 >> /home/user/tmp").arg(filename);
    system(command.toStdString().c_str());
    int ret = pLoader->verifyHeader("/home/user/tmp","",&crc,&rev);
    QString CRC = QString::number(crc,16);

    command = QString("echo \"<0x%1,%2>\" > /home/user/tmp").arg(CRC).arg(QString(parametri[1]));
    system(command.toStdString().c_str());
    command = QString("cat %1 >> /home/user/tmp").arg(filename);
    system(command.toStdString().c_str());
    command = QString("mv /home/user/tmp %1").arg(filename);
    system(command.toStdString().c_str());

    serviceTcp->txData(QString("Command Executed: CRC:0x%1\n\r").arg(CRC).toAscii());
}

void serverDebug::handleSetRemoteCRC(QByteArray data){
    QList<QByteArray> parametri;
    QString filename;
    parametri = getNextFieldsAfterTag(data, QString("setRemoteCRC"));
    if(parametri.size()!=2)
    {
        serviceTcp->txData(QByteArray("wrong parameters\n\r"));
        return;
    }

    // Controllo indirizzo
    if(parametri[0]=="PCB249U1") {
        filename = QString("FW249U1.hex");
    }else if(parametri[0]=="PCB249U2") {
        filename = QString("FW249U2.hex");
    }else if(parametri[0]=="PCB240") {
        filename = QString("FW240.hex");
    }else if(parametri[0]=="PCB244A") {
        filename = QString("FW244A.hex");
    }else if(parametri[0]=="PCB190") {
        filename = QString("FW190A.hex");
    }else if(parametri[0]=="PCB269") {
        filename = QString("FW269.hex");
    }else{
        serviceTcp->txData(QByteArray("invalid target\n\r"));
        return;
    }



    // Prepara il file
    QString command;
    QString rev;
    unsigned short crc;

    command = QString("rm /home/user/%1").arg(filename);
    system(command.toStdString().c_str());

    command = QString("/monta.sh");
    system(command.toStdString().c_str());

    command = QString("cp /mnt/nfs/%1 /home/user/").arg(filename);
    system(command.toStdString().c_str());


    command = QString("echo \"<0x0000,%1>\" > /home/user/tmp").arg(QString(parametri[1]));
    system(command.toStdString().c_str());
    command = QString("cat /home/user/%1 >> /home/user/tmp").arg(filename);
    system(command.toStdString().c_str());
    int ret = pLoader->verifyHeader("/home/user/tmp","",&crc,&rev);
    QString CRC = QString::number(crc,16);

    command = QString("echo \"<0x%1,%2>\" > /home/user/tmp").arg(CRC).arg(QString(parametri[1]));
    system(command.toStdString().c_str());
    command = QString("cat /home/user/%1 >> /home/user/tmp").arg(filename);
    system(command.toStdString().c_str());
    command = QString("mv /home/user/tmp /home/user/%1").arg(filename);
    system(command.toStdString().c_str());


    serviceTcp->txData(QString("Command Executed: CRC:0x%1\n\r").arg(CRC).toAscii());
}

/*

 *
 */
void serverDebug::serviceNotifyFineRaggi(QByteArray buffer)
{
   float vmean;
   unsigned char imean;
   unsigned short iTmed, tpls, mAs;
   QString stringa;

   mAs = buffer.at(MAS_PLS_L) + 256*buffer.at(MAS_PLS_H);
   vmean = pGeneratore->convertPcb190Kv(buffer.at(V_MEAN),1.0);
   imean = pGeneratore->convertPcb190Ianode(buffer.at(I_MEAN));
   tpls = (unsigned short) (buffer.at(T_MEAN_PLS_L)+256*buffer.at(T_MEAN_PLS_H));
   if(tpls!=0) iTmed = mAs * 20 / tpls;
   else iTmed=0;

    switch(buffer.at(RX_END_CODE))
    {
        case 0: stringa.append(QString("MANUAL RX OK:\n\r"));break;
        case ERROR_HV_HIGH: stringa.append(QString("MANUAL RX: KV ALTI\n\r")); break;
        case ERROR_HV_LOW: stringa.append(QString("MANUAL RX: KV BASSI\n\r")); break;
        case ERROR_IA_HIGH:stringa.append(QString("MANUAL RX: I ANODICA ALTA\n\r")); break;
        case ERROR_IA_LOW: stringa.append(QString("MANUAL RX: I ANODICA BASSA\n\r")); break;
        case ERROR_IFIL: stringa.append(QString("MANUAL RX: I-FILAMENT ALTA\n\r")); break;
        case ERROR_TMO_RX: stringa.append(QString("TIMEOUT \n\r")); break;

    default:
        stringa.append(QString("MANUAL RX: ERRORE=%1\n\r").arg((int) buffer.at(RX_END_CODE)));
    break;
    }

    stringa.append(QString("mAs: %1 \n\r").arg((float) mAs/50));
    stringa.append(QString("KV(read): %1(kV) %2(RAW)\n\r").arg(vmean).arg((int) buffer.at(V_MEAN)));
    stringa.append(QString("IA(read): %1(mA) %2(RAW) \n\r").arg((int) imean).arg((int) buffer.at(I_MEAN)));
    stringa.append(QString("T: %1 (ms)\n\r").arg(tpls));
    stringa.append(QString("I(T): %1 (mA) \n\r").arg(iTmed));
    serviceTcp->txData(stringa.toAscii());
}




/*
 *  Gestione delle funzioni legate al Generatore
 *  FUNZIONI:
 *          - setFuoco OFF/ W,Mo Small,Large   Impostazione del fuoco
 *          - validateGen                      Forza la validazione dei dati del generatore
 *          - shot ON/OFF                      Abilitazione dello sparo manuale da file
 *          - starter HS/LS/OFF                  Attiva Starter ad Alta/Bassa velocitÃ  o OFF
 */
void serverDebug::handleGeneratore(QByteArray data)
{
    if(data.contains("?"))
    {
        serviceTcp->txData(QByteArray("--------------------------------------------------------------\r\n"));
        serviceTcp->txData(QByteArray("getMAS                           Restituisce il valore dei mas test\r\n"));
        serviceTcp->txData(QByteArray("setCalibKv [k,cost]              Coefficienti di claibrazionelettura kV  \r\n"));
        serviceTcp->txData(QByteArray("getCalibKv                       Rilettura coeff. calibrazione read-kV  \r\n"));

        serviceTcp->txData(QByteArray("getKvDac [float KV]              Restituisce il DAC assegnato\r\n"));
        serviceTcp->txData(QByteArray("manXray [FILE/AUTO/OFF]          Modo XRAY manuale, da file \r\n"));
        serviceTcp->txData(QByteArray("setManReg Fuoco,Vn,Vdac,In,Idac,dacInc,mAs,Hs,SWA,SWB,TMO   \r\n"));

        serviceTcp->txData(QByteArray("setFuoco [W/Mo/OFF][Small/Large] Imposta il fuoco corrente  \r\n"));
        serviceTcp->txData(QByteArray("setStarter  [HS/LS/OFF]          Attivazione Starter  \r\n"));
        serviceTcp->txData(QByteArray("getStatistics [store]            Legge e opzionalmente forza la scrittura\r\n"));
        serviceTcp->txData(QByteArray("readTube                         Legge dati Tubo e configura PCB190 \r\n"));
        serviceTcp->txData(QByteArray("getTubeRevision                  Legge la revisione del template \r\n"));
        serviceTcp->txData(QByteArray("getAnodeHU                       Legge gli Hu anodici accumulati\r\n"));
        serviceTcp->txData(QByteArray("addAnodeHU kHU                   Aggiunge kHU anodici accumulati\r\n"));
        serviceTcp->txData(QByteArray("getStatAnodeHU                   Legge le variabili di stato per Anode HU\r\n"));
        serviceTcp->txData(QByteArray("startDemoN                       Attiva seq Tomo N se in DEMO\r\n"));
        serviceTcp->txData(QByteArray("startDemoI                       Attiva seq Tomo I se in DEMO\r\n"));
        serviceTcp->txData(QByteArray("startDemoW                       Attiva seq Tomo W se in DEMO\r\n"));
        serviceTcp->txData(QByteArray("getAEC     (int)pl, (int)fm, (int)od, (int)tech   \r\n"));
        serviceTcp->txData(QByteArray("getDose    (int) mm, (int) offset (int) dmAs, (int) dkv, (int) filtro   \r\n"));

        serviceTcp->txData(QByteArray("--------------------------------------------------------------\r\n"));
    }else if(data.contains("getAEC")){
        QList<QByteArray> parametri = getNextFieldsAfterTag(data, QString("getAEC"));
        if(parametri.size() != 4) {
            serviceTcp->txData(QString("Invalid parameters\n\r").toAscii());
            return;
        }

        int plog = parametri[0].toInt();
        int fm =  parametri[1].toInt();
        int od = parametri[2].toInt();
        int tech = parametri[3].toInt();
        int rxfiltro;
        float rxkV;
        int rxdmas;
        int rxpulses;
        int errcode = pGeneratore->pAECprofiles->getAecData(plog,fm,4,od,tech,QString("Mo"), (int) Generatore::FUOCO_LARGE, &rxfiltro,&rxkV,&rxdmas,&rxpulses);

        if(errcode){
            serviceTcp->txData(QString("ERRORE %1 \r\n").arg(errcode).toAscii());
            return;
        }

        serviceTcp->txData(QString("FILTRO: %1, KV:%2, MAS:%3, PULSES:%4 \r\n").arg(rxfiltro).arg(rxkV).arg(rxdmas).arg(rxpulses).toAscii());
        return;


    }else if(data.contains("getDose")){
        QList<QByteArray> parametri = getNextFieldsAfterTag(data, QString("getDose"));
        if(parametri.size() != 5) {
            serviceTcp->txData(QString("Invalid parameters\n\r").toAscii());
            return;
        }

        int mm = parametri[0].toInt();
        int offset = parametri[1].toInt();
        int dmas =  parametri[2].toInt();
        int dkv = parametri[3].toInt();
        int filtro = parametri[4].toInt();

        QString dosestr = pGeneratore->pDose->getDoseUgDebug(mm,offset,dmas,dkv,filtro) + "\n\r";
        serviceTcp->txData(dosestr.toAscii());

        float dose  = pGeneratore->pDose->getDoseUg(mm,offset,dmas,dkv,filtro);
        if(dose==-1) serviceTcp->txData(QString("DOSE NOT AVAILABLE\r\n").toAscii());
        else serviceTcp->txData(QString("DOSE (uG)= %1\r\n").arg(dose).toAscii());
        return;


    }else if(data.contains("getMAS")){
        serviceTcp->txData(QString("MAS = %1 \r\n").arg(pGeneratore->mAsTest).toAscii());
        serviceTcp->txData(QString("ANODICA = %1 \r\n").arg(pGeneratore->anodicaTest).toAscii());
    } else if(data.contains("setFuoco"))
    {
        if(data.contains("OFF")) pGeneratore->fuocoOff();
        else
        {
            if(data.contains("W")) pGeneratore->setFuoco(QString("W"));
            else if(data.contains("Mo")) pGeneratore->setFuoco(QString("Mo"));
            else{
                serviceTcp->txData(QByteArray("Wrong Anode selection!\r\n"));
                return;
            }
            if(data.contains("Small")) pGeneratore->setFuoco(Generatore::FUOCO_SMALL);
            else if(data.contains("Large")) pGeneratore->setFuoco(Generatore::FUOCO_LARGE);
            else{
                serviceTcp->txData(QByteArray("Wrong size selection!\r\n"));
                return;
            }
            pGeneratore->updateFuoco();
            serviceTcp->txData(QByteArray("Command executed!\r\n"));
        }
    }else if(data.contains("setStarter"))
    {
        if(data.contains(" HS")){
            pGeneratore->setStarter(2);
            serviceTcp->txData(QString("Starter: HS mode\r\n").toAscii());
        }else if(data.contains(" LS")){
            pGeneratore->setStarter(1);
            serviceTcp->txData(QString("Starter: LS mode\r\n").toAscii());
        }else if(data.contains(" OFF")){
            pGeneratore->setStarter(0);
            serviceTcp->txData(QString("Starter: OFF\r\n").toAscii());
        }

    }else if(data.contains("getKvDac")){

        QList<QByteArray> parametri = getNextFieldsAfterTag(data, QString("getKvDac"));
        float kv = parametri[0].toFloat();
        unsigned char kvc;
        unsigned short dac;
        if(pGeneratore->getValidKv(kv, &kvc, &dac) == FALSE){
            serviceTcp->txData(QString("KV SELEZIONATI NON VALIDI\r\n").toAscii());
            return;
        }

        serviceTcp->txData(QString("DAC(%1) = %2\r\n").arg(kvc).arg(dac).toAscii());

    }else if(data.contains("getStatistics")){
        if(data.contains("store")) pGeneratore->statChanged = true;

        QString tube = QString(_TUBEPATH) + QString("/") + pConfig->userCnf.tubeFileName + QString("/");
        pGeneratore->saveTubeStatisticsFile(tube);
        pGeneratore->readTubeStatisticsFile(tube);

        serviceTcp->txData(QString("TUBE: %1\r\n").arg(pConfig->userCnf.tubeFileName).toAscii());
        serviceTcp->txData(QString("NSHOTS: %1\r\n").arg(pGeneratore->numShots).toAscii());
        serviceTcp->txData(QString("kHU: %1\r\n").arg((float) pGeneratore->cumulatedJ * 1.33 / 1000).toAscii());
        serviceTcp->txData(QString("N-TOMO: %1\r\n").arg(pGeneratore->nTomo).toAscii());
        serviceTcp->txData(QString("N-STANDARD: %1\r\n").arg(pGeneratore->nStandard).toAscii());
    }else if(data.contains("readTube"))
    {
        if(pGeneratore->openCurrentTube()){
            serviceTcp->txData("Command executed: configuration in progress ..\r\n");
            connect(pConfig,SIGNAL(configUpdatedSgn()),this,SLOT(configurationSlot()),Qt::UniqueConnection);
            pConfig->updatePCB190();
        }else serviceTcp->txData("Command failed!\r\n");
    }else if(data.contains("getTubeRevision")){
        serviceTcp->txData(QString("TEMPLATE REV: %1\r\n").arg(pGeneratore->templateRevision).toAscii());
    }else if(data.contains("setCalibKv")){
        //     (float) k, (float) cost
        QList<QByteArray> parametri = getNextFieldsAfterTag(data, QString("setCalibKv"));
        if(parametri.size() != 2) {
            serviceTcp->txData(QString("Invalid parameters\n\r").toAscii());
            return;
        }

        pGeneratore->genCnf.pcb190.kV_CALIB = (unsigned short) (parametri[0].toFloat() * 1000);
        pGeneratore->genCnf.pcb190.kV_OFS = (short) (parametri[1].toFloat() * 1000);
        pGeneratore->saveTubeKvReadCalibrationFile();
        pConfig->updatePCB190();
        serviceTcp->txData(QString("New coefficients: kv =%1, cost=%2\r\n").arg((float)pGeneratore->genCnf.pcb190.kV_CALIB/1000).arg((float)pGeneratore->genCnf.pcb190.kV_OFS/1000).toAscii());
        return;
    }else if(data.contains("getCalibKv")){
        serviceTcp->txData(QString("New coefficients: kv =%1, cost=%2\r\n").arg((float)pGeneratore->genCnf.pcb190.kV_CALIB/1000).arg((float)pGeneratore->genCnf.pcb190.kV_OFS/1000).toAscii());
        return;
    }else if(data.contains("getAnodeHU")){
        serviceTcp->txData(QString("Current Anode HU:%1\r\n").arg((float)pGeneratore->getCurrentHU()).toAscii());
        return;
    }else if(data.contains("addAnodeHU")){

        QList<QByteArray> parametri = getNextFieldsAfterTag(data, QString("addAnodeHU"));
        if(parametri.size() != 1) {
            serviceTcp->txData(QString("Invalid parameters\n\r").toAscii());
            return;
        }

        pGeneratore->updateAnodeHU(parametri[0].toFloat());
        serviceTcp->txData(QString("Aggiunti %1 kHU. Current Anode HU:%2\r\n").arg(parametri[0].toFloat()).arg((float)pGeneratore->getCurrentHU()).toAscii());
        return;
    }else if(data.contains("getStatAnodeHU")){


        if(pGeneratore->anodeHuInitOk) serviceTcp->txData(QString("INIT OK\r\n").toAscii());
        else serviceTcp->txData(QString("NOT INITIALIZED\r\n").toAscii());

        serviceTcp->txData(QString("anodeHVsaved:%1\r\n").arg(pGeneratore->anodeHUSaved).toAscii());
        int Y = pGeneratore->anodeHuTime.date().year();
        int M = pGeneratore->anodeHuTime.date().month();
        int D = pGeneratore->anodeHuTime.date().day();
        int s = pGeneratore->anodeHuTime.time().second();
        int m = pGeneratore->anodeHuTime.time().minute();
        int h = pGeneratore->anodeHuTime.time().hour();
        if(pGeneratore->anodeHUSaved) serviceTcp->txData(QString("LAST SAVE Y:%1 M:%2 D:%3 H:%4 m:%5 s:%6 \r\n").arg(Y).arg(M).arg(D).arg(h).arg(m).arg(s).toAscii());
        else serviceTcp->txData(QString("LAST SAVE: NA\r\n").toAscii());

        if(pGeneratore->alarmAnodeHU) serviceTcp->txData(QString("ANODE IN ALARM!!\r\n").toAscii());
        else serviceTcp->txData(QString("ANODE NOT IN ALARM!!\r\n").toAscii());

        serviceTcp->txData(QString("T0:%1\r\n").arg(pGeneratore->T0).toAscii());
        serviceTcp->txData(QString("ANODE kHU:%1\r\n").arg(pGeneratore->anodeHU).toAscii());
        serviceTcp->txData(QString("ELAPSED seconds:%1\r\n").arg(pGeneratore->hutimer_elapsed/1000).toAscii());

        return;
    }
}

void serverDebug::configurationSlot(void){
    disconnect(pConfig,SIGNAL(configUpdatedSgn()),this,SLOT(configurationSlot()));
    serviceTcp->txData("Configuration completed!\r\n");
}

QByteArray serverDebug::getNextFieldAfterTag(QByteArray data, QString tag)
{
    int i;
    QByteArray risultato;

    risultato.clear();

    i = data.indexOf(tag);
    data = data.right(data.size()-i-tag.size());

    // Cerca l'inizio
    for(i=0; i<data.size(); i++) if(data.at(i)!=' ') break;

    // Compila fino alla fine
    for(i=0; i<data.size(); i++)
    {
        if(data.at(i)==' ') return risultato;
        if(data.at(i)==',') return risultato;
        if(data.at(i)=='\n') return risultato;
        if(data.at(i)=='\r') return risultato;
        risultato.append(data.at(i));
    }

    return risultato;
}

QList<QByteArray> serverDebug::getNextFieldsAfterTag(QByteArray data, QString tag)
{
    int i;
    QList<QByteArray> risultato;
    QByteArray item;

    risultato.clear();

    i = data.indexOf(tag);
    if(i==-1) return risultato;

    data = data.right(data.size()-i-tag.size());

    // Cerca l'inizio
    for(i=0; i<data.size(); i++) if(data.at(i)!=' ') break;


    // Compila fino alla fine
    item.clear();
    for(i=0; i<data.size(); i++)
    {
        if((data.at(i)==' ') || (data.at(i)==','))
        {
            // Termine parametro
            if(!item.isEmpty()) risultato.append(item);
            item.clear();
        }else if((data.at(i)=='\n') || (data.at(i)=='\r'))
        {
            // Termine sequenza
            if(item.isEmpty()) return risultato;
            else
            {
                risultato.append(item);
                return risultato;
            }
        }else item.append(data.at(i));

    }

    if(item.isEmpty()) return risultato;
    else
    {
        risultato.append(item);
        return risultato;
    }

}

void serverDebug::setManualLameVal(QString lama, int val)
{
    if(lama=="L")         pCollimatore->manualL = val;
    else if(lama=="R")    pCollimatore->manualR = val;
    else if(lama=="B")    pCollimatore->manualB = val;
    else if(lama=="F")    pCollimatore->manualF = val;
    else if(lama=="T")    pCollimatore->manualT = val;
    return;
}


void serverDebug::handleRotazioni(QByteArray data)
{
    QByteArray field;
    int angolo;
    unsigned char buffer[3];


    if(data.contains("?"))
    {
        serviceTcp->txData(QByteArray("rotazioni: ---------- Comandi relativi alla gestione bracci -----------------\r\n"));
        serviceTcp->txData(QByteArray("TRX STOP      Ferma Immediatamente TRX  \r\n"));
        serviceTcp->txData(QByteArray("TRX WHOME     Muove tubo a Wide Home  \r\n"));
        serviceTcp->txData(QByteArray("TRX NHOME     Muove tubo a Narrow Home  \r\n"));
        serviceTcp->txData(QByteArray("TRX IHOME     Muove tubo a Intermediate Home  \r\n"));
        serviceTcp->txData(QByteArray("TRX WEND      Muove tubo a Wide End  \r\n"));
        serviceTcp->txData(QByteArray("TRX NEND      Muove tubo a Wide End  \r\n"));
        serviceTcp->txData(QByteArray("TRX IEND      Muove tubo a Intermediate End  \r\n"));
        serviceTcp->txData(QByteArray("TRX LOOP val  Attiva la procedura di rodaggio tubo\r\n"));
        serviceTcp->txData(QByteArray("TRX [angolo]  Muove tubo a Angolo (+/-)  \r\n"));
        serviceTcp->txData(QByteArray("ARM [angolo]  Muove braccio a Angolo (+/-)  \r\n"));
        serviceTcp->txData(QByteArray("resetGonio    Reset Inclinometri\r\n"));
        serviceTcp->txData(QByteArray("getGonio      Legge inclinometro\r\n"));
        serviceTcp->txData(QByteArray("readTrxConfig Rilegge TRX config e download\r\n"));
        serviceTcp->txData(QByteArray("saveTrxConfig Salva il TRX config \r\n"));
        serviceTcp->txData(QByteArray("readArmConfig Rilegge ARM config e download\r\n"));
        serviceTcp->txData(QByteArray("saveArmConfig Salva il ARM config \r\n"));
        serviceTcp->txData(QByteArray("readLenzeConfig Rilegge Lenze config e download\r\n"));
        serviceTcp->txData(QByteArray("saveLenzeConfig Salva il Lenze config \r\n"));
        serviceTcp->txData(QByteArray("setRotManualMode [ARMS|ARMC|TRXS|TRXC] \r\n"));
        serviceTcp->txData(QByteArray("----------------------------------------------------------------------------------\r\n"));
    }else if(data.contains("setRotManualMode")){
        QList<QByteArray> parametri;
        unsigned char buffer[2];
        parametri = getNextFieldsAfterTag(data, QString("setRotManualMode"));
        if(parametri.size()!=1)
        {
            serviceTcp->txData(QByteArray("wrong parametrs\n\r"));
            return;
        }

        if(parametri[0]=="ARMS") buffer[0] = CALIB_ZERO_MANUAL_ACTIVATION_ARM_STANDARD;
        else if(parametri[0]=="ARMC") buffer[0] = CALIB_ZERO_MANUAL_ACTIVATION_ARM_CALIB;
        else if(parametri[0]=="TRXC") buffer[0] = CALIB_ZERO_MANUAL_ACTIVATION_TRX_CALIB;
        else if(parametri[0]=="TRXS") buffer[0] = CALIB_ZERO_MANUAL_ACTIVATION_TRX_STANDARD;
        else {
             serviceTcp->txData(QByteArray("wrong type\n\r"));
             return;
        }

        pConsole->pGuiMcc->sendFrame(MCC_CALIB_ZERO,0,buffer, sizeof(buffer));
    }else if(data.contains("STOP"))
    {
        buffer[0]=TRX_MOVE_STOP; // Comando WHOME
        buffer[1]=0; // Attende questo comando
        buffer[2]=0;
        // Invio comando
        pConsole->pGuiMcc->sendFrame(MCC_CMD_TRX,0,buffer, 3);

    }
    else if(data.contains("TRX WHOME"))
    {
        buffer[0]=TRX_MOVE_HOME_W; // Comando WHOME
        buffer[1]=MOVE_WAIT_END; // Attende questo comando
        buffer[2]=0;
        // Invio comando
        pConsole->pGuiMcc->sendFrame(MCC_CMD_TRX,0,buffer, 3);

    }else if(data.contains("TRX NHOME"))
    {
        buffer[0]=TRX_MOVE_HOME_N;
        buffer[1]=MOVE_WAIT_END; // Non attende comando precedente + Attende questo comando
        buffer[2]=0;
        // Invio comando
        pConsole->pGuiMcc->sendFrame(MCC_CMD_TRX,0,buffer, 3);

    }else if(data.contains("TRX IHOME"))
    {
        buffer[0]=TRX_MOVE_HOME_I;
        buffer[1]=MOVE_WAIT_END; // Non attende comando precedente + Attende questo comando
        buffer[2]=0;
        // Invio comando
        pConsole->pGuiMcc->sendFrame(MCC_CMD_TRX,0,buffer, 3);

    }else if(data.contains("TRX WEND"))
    {
        buffer[0]=TRX_MOVE_END_W ;
        buffer[1]=MOVE_WAIT_END; // Non attende comando precedente + Attende questo comando
        buffer[2]=0;
        // Invio comando
        pConsole->pGuiMcc->sendFrame(MCC_CMD_TRX,0,buffer, 3);

    }else if(data.contains("TRX NEND"))
    {
        buffer[0]=TRX_MOVE_END_N;
        buffer[1]=MOVE_WAIT_END; // Non attende comando precedente + Attende questo comando
        buffer[2]=0;
        // Invio comando
        pConsole->pGuiMcc->sendFrame(MCC_CMD_TRX,0,buffer, 3);

    }else if(data.contains("TRX IEND"))
    {
        buffer[0]=TRX_MOVE_END_I;
        buffer[1]=MOVE_WAIT_END; // Non attende comando precedente + Attende questo comando
        buffer[2]=0;
        // Invio comando
        pConsole->pGuiMcc->sendFrame(MCC_CMD_TRX,0,buffer, 3);

    }else if(data.contains("TRX LOOP"))
    {
        handleRodaggioTubo(data);
    } else if(data.contains("TRX"))
    {
        handleMoveTrx("TRX ", data);

    } else if(data.contains("ARM"))
    {
        handleMoveArm("ARM ", data);

    }else if(data.contains("getGonio")){

        if(pConsole->pGuiMcc->sendFrame(MCC_GET_GONIO,1,NULL, 0)==FALSE){
            serviceTcp->txData(QByteArray("COMANDO FALLITO!\n\r"));
            return;
        }
        serviceTcp->txData(QByteArray("ATTESA DATI..\n\r"));
        connect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(getGonioNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
    }else if(data.contains("resetGonio"))
    {
        serviceTcp->txData(QByteArray("GONIO RESETTING ..."));
        if(pConsole->pGuiMcc->sendFrame(MCC_RESET_GONIO,1,0, 0)==FALSE)
        {
            serviceTcp->txData(QByteArray("FALLITO!\n\r"));
            return;
        }
        connect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(resetGonioNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

    }else if(data.contains("readTrxConfig")){
        pConfig->readTrxConfig();
        pConfig->updateTrxDriver();
        serviceTcp->txData(QByteArray("FILE DI CONFIGURAZIONE TRX LETTO\n\r"));
        return;
    }else if(data.contains("saveTrxConfig")){
        pConfig->saveTrxConfig();
        serviceTcp->txData(QByteArray("TRX config file Stored!\n\r"));
    }else if(data.contains("readArmConfig")){
        pConfig->readArmConfig();
        pConfig->updateArmDriver();
        serviceTcp->txData(QByteArray("FILE DI CONFIGURAZIONE ARM LETTO\n\r"));
        return;
    }else if(data.contains("saveArmConfig")){
        pConfig->saveArmConfig();
        serviceTcp->txData(QByteArray("ARM config file Stored!\n\r"));
    }else if(data.contains("readLenzeConfig")){
        pConfig->readLenzeConfig();
        pConfig->updateLenzeDriver();
        serviceTcp->txData(QByteArray("FILE DI CONFIGURAZIONE LENZE LETTO\n\r"));
        return;
    }else if(data.contains("saveLenzeConfig")){
        pConfig->saveLenzeConfig();
        serviceTcp->txData(QByteArray("LENZE config file Stored!\n\r"));
    }
}



/*
  Muove Tubo ad angolo
  - data Ã¨ la stringa di comando
  - tag indica la stringa oltre la quale va cercato il parametro
*/
void serverDebug::handleMoveTrx(QString tag, QByteArray data)
{
    int angolo;
    unsigned char buffer[4];

    angolo =  getNextFieldAfterTag(data,tag).toInt(); // Espresso in gradi
    if(angolo>28) angolo=28;
    else if(angolo<-28) angolo=-28;

    buffer[0]=TRX_MOVE_ANGLE;
    buffer[2] = (unsigned char) angolo;
    buffer[3] = (unsigned char) (angolo>>8);

    pConsole->pGuiMcc->sendFrame(MCC_CMD_TRX,0,buffer, sizeof(buffer));

}

/*
  Muove Tubo ad angolo

  Vengono controllati per sicurezza i limiti di rotazione

*/
void serverDebug::handleMoveArm(QString tag, QByteArray data)
{
    int target;
    unsigned char buffer[3];

    target = getNextFieldAfterTag(data,tag).toInt();
    if(target > 180) target = 180;
    else if(target<-180) target = -180;

    // Impostazione Parametro
    buffer[0] =(unsigned char) (target&0xFF);
    buffer[1] =(unsigned char) (target>>8);

    // Invio comando
    pConsole->pGuiMcc->sendFrame(MCC_CMD_ARM,0,buffer, 2);
}

void serverDebug::resetGonioNotify(unsigned char id, unsigned char mcccode, QByteArray buffer)
{
    if(id!=1) return;
    if(mcccode!=MCC_RESET_GONIO) return;

    disconnect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(resetGonioNotify(unsigned char,unsigned char,QByteArray)));
    if(buffer.size()!=0)


    if(buffer.at(0)==1) serviceTcp->txData(QByteArray("Completato con successo \n\r"));
    else serviceTcp->txData(QByteArray("Fallito reset inclinometro PCB249 \n\r"));
}

void serverDebug::getGonioNotify(unsigned char id, unsigned char mcccode, QByteArray buffer)
{

    // Filtra parte dei possibili ID
    if(id!=1) return;
    if(mcccode!=MCC_GET_GONIO) return;

    disconnect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(getGonioNotify(unsigned char,unsigned char,QByteArray)));


    short arm = buffer[0]+256*buffer[1]; // Espressi in decimi di grado
    short trx = buffer[2]+256*buffer[3]; // Espressi in centesimi di grado
    short gonio = buffer[4]+256*buffer[5]; // // Espressi in decimi di grado

    serviceTcp->txData(QString("TRX:(c)%1, ARM:(d)%2, GONIO:(d)%3\n\r").arg(trx).arg(arm).arg(gonio).toAscii());
}

/*
 *  Gestione delle funzioni legate al Collimatore
 *  FUNZIONI:
 *
 */
void serverDebug::handleCollimatore(QByteArray data)
{
    QByteArray field;

    // HELP
    if(data.contains("?"))
    {
        serviceTcp->txData(QByteArray("--------- COMANDI COLLIMATORE -------------------------------------------------\r\n"));
        serviceTcp->txData(QByteArray("readColliConf     Rilegge il file di configurazione  \r\n"));
        serviceTcp->txData(QByteArray("STORE            Salva i dati di collimazione nel file di configurazione\r\n"));

        serviceTcp->txData(QByteArray("\r\n--- COMANDI FILTRO -------------------------\r\n"));
        serviceTcp->txData(QByteArray("setFiltro [Al/Rh/Ag/Us]    Imposta il filtro\r\n"));
        serviceTcp->txData(QByteArray("setCalibTomoFiltro <ENA,ang0,ang1,ang2>    Imposta gli angoli di incremento filtro\r\n"));
        serviceTcp->txData(QByteArray("setCalibFiltro <Filtro,val>    Modifica il valore di calibrazione del filtro\r\n"));

        serviceTcp->txData(QByteArray("\r\n--- COMANDI COLLIMATORE  -------------------------\r\n"));
        serviceTcp->txData(QByteArray("getStatus  Restituisce la collimazione correntemente impostata\r\n"));
        serviceTcp->txData(QByteArray("setAuto           Imposta la modalita' di collimazione Automatica\r\n"));
        serviceTcp->txData(QByteArray("setManual         Imposta modalitÃ  di collimazione Manuale\r\n"));
        serviceTcp->txData(QByteArray("setL  val _____ Imposta la lama sinistra della collimazione corrente   \r\n"));
        serviceTcp->txData(QByteArray("setR  val _____ Imposta la lama destra della collimazione corrente  \r\n"));
        serviceTcp->txData(QByteArray("setT  val _____ Imposta il trapezio della collimazione corrente  \r\n"));
        serviceTcp->txData(QByteArray("setB  val _____ Imposta la lama posteriore della collimazione corrente  \r\n"));
        serviceTcp->txData(QByteArray("setF  val _____ Imposta la lama frontale della collimazione corrente  \r\n"));
        serviceTcp->txData(QByteArray("update    _____ Effettua la collimazione con i valori su elencati  \r\n"));

        serviceTcp->txData(QByteArray("--- COMANDI SPECCHIO/LUCE  -------------------------\r\n"));
        serviceTcp->txData(QByteArray("setMirror [HOME/OUT]  Imposta lo specchio\r\n"));
        serviceTcp->txData(QByteArray("setLamp [ON/OFF]  Accende /spegne la lampada\r\n"));
        serviceTcp->txData(QByteArray("setCalibMirror                 Modifica gli steps in campo per lo specchio  \r\n"));

        serviceTcp->txData(QByteArray("setCalib2D <pad,Mat,L,R,F,B,T> Impostazione collimazioni pad\r\n"));
        serviceTcp->txData(QByteArray("setCalibTomo   <Mat,Lama,P/N, n0..n25> <Mat,Front,Back>\r\n"));
        serviceTcp->txData(QByteArray("getCalib         Restituisce tutta la calibrazione\r\n"));
        serviceTcp->txData(QByteArray("download         Aggiorna configurazione su periferica\r\n"));

        serviceTcp->txData(QByteArray("setTrx val      Muove Tubo ad angolo (copia del comando in rotazioni)\r\n"));
        serviceTcp->txData(QByteArray("testColli val   Muove ciclicamente il formato tra open e CLOSED per Val volte\r\n"));

        serviceTcp->txData(QByteArray("-------------------------------------------------------------------------------\r\n"));
    }

    if(data.contains("download"))
    {
        pConfig->updatePCB249U1();
        return;
    }

    if(data.contains("testColli"))
    {
        QList<QByteArray> parametri = getNextFieldsAfterTag(data, QString("testColli"));

        if(parametri.size() == 0) {
            QString stringa = QString("MISSING NUMBER OF TEST\r\n");
            serviceTcp->txData(stringa.toAscii());
            return;
        }
        pCollimatore->startColliTest(parametri[0].toInt());
        serviceTcp->txData(QByteArray("COLLI TEST STARTED\r\n"));
        return;
    }

    if(data.contains("readColliConf"))
    {
        pCollimatore->readConfigFile();
        pConfig->updatePCB249U1();
        return;
    }

    if(data.contains("getCalib"))
    {
        handleGetCalib(data);
        return;
    }

    if(data.contains("setCalibTomoFiltro"))
    {
        handleSetCalibTomoFiltro(data);
        return;
    }

    if(data.contains("setCalibTomo"))
    {
        handleSetCalibTomo(data);
        return;
    }


    if(data.contains("setAuto")) // Imposta la modalitÃ  AUTOMATICA
    {
        serviceTcp->txData(QByteArray("---COLLIMAZIONE AUTOMATICA-----\r\n"));
        pCollimatore->manualCollimation=FALSE;
        pCollimatore->manualFiltroCollimation=FALSE;
        pCollimatore->setFiltro();
        pCollimatore->updateColli();
         return;
    }

    if(data.contains("getStatus")) // Richiede la collimazione corrente
    {
        if(pCollimatore->colli_model == _COLLI_TYPE_NOT_ASSIGNED){
            serviceTcp->txData(QByteArray("COLLI MODEL: NOT DETECTED\r\n"));
        }else if(pCollimatore->colli_model == _COLLI_TYPE_ASSY_01){
            serviceTcp->txData(QByteArray("COLLI MODEL: ASSY 01\r\n"));
        }else serviceTcp->txData(QByteArray("COLLI MODEL: ASSY 02\r\n"));

        if(pCollimatore->manualCollimation==FALSE)
        {
            serviceTcp->txData(QByteArray("COLLIMAZIONE LAME: AUTOMATICA \r\n"));

        }else
        {
            serviceTcp->txData(QByteArray("---COLLIMAZIONE MANUALE-----\r\n"));
            serviceTcp->txData(QString("LAMA-L:%1\n\r").arg(pCollimatore->manualL).toAscii());
            serviceTcp->txData(QString("LAMA-R:%1\n\r").arg(pCollimatore->manualR).toAscii());
            serviceTcp->txData(QString("LAMA-T:%1\n\r").arg(pCollimatore->manualT).toAscii());
            serviceTcp->txData(QString("LAMA-B:%1\n\r").arg(pCollimatore->manualB).toAscii());
            serviceTcp->txData(QString("LAMA-F:%1\n\r").arg(pCollimatore->manualF).toAscii());

        }

        if(pCollimatore->manualFiltroCollimation==FALSE){
            serviceTcp->txData(QByteArray("COLLIMAZIONE FILTRO: AUTOMATICA \r\n"));
        }else{
            serviceTcp->txData(QString("FILTRO MANUALE:").append(pCollimatore->getFilterTag(pCollimatore->manualFilter)).append("\n\r").toAscii());
        }


        return;
    }

    if(data.contains("setManual")) // Imposta la modalitÃ  MANUALE di collimazione
    {
        pCollimatore->manualCollimation=TRUE;
        serviceTcp->txData(QByteArray("MODO MANUALE ATTIVATO\n\r"));
        return;
    }


    if(data.contains("update"))
    {
        pCollimatore->manualColliUpdate();
        return;
    }

    if(data.contains("setFiltro"))
    {
         pCollimatore->manualFiltroCollimation=TRUE;
        if(data.contains(" Al")) pCollimatore->manualFilter = Collimatore::FILTRO_Al;
        else if(data.contains(" Ag")) pCollimatore->manualFilter = Collimatore::FILTRO_Ag;
        else if(data.contains(" Rh")) pCollimatore->manualFilter = Collimatore::FILTRO_Rh;
        else if(data.contains(" Cu")) pCollimatore->manualFilter = Collimatore::FILTRO_Cu;
        else if(data.contains(" Mo")) pCollimatore->manualFilter = Collimatore::FILTRO_Mo;
        pCollimatore->manualSetFiltro();
        return;
    }

    if(data.contains("setCalibFiltro"))
    {

        handleSetCalibFiltro(data);
        return;
    }

    if(data.contains("STORE"))
    {

        pCollimatore->storeConfigFile();
        serviceTcp->txData(QString("DATI DI COLLIMAZIONE SALVATI IN CONFIGURAZIONE\n\r").toAscii());
        return;
    }
    if(data.contains("setMirror"))
    {
        if(data.contains("HOME")) pCollimatore->setMirror(Collimatore::MIRROR_HOME);
        else if(data.contains("OUT")) pCollimatore->setMirror(Collimatore::MIRROR_OUT);
        return;
    }

    if(data.contains("setCalibMirror"))
    {
        handleSetCalibMirror(data);
        return;
    }

    if(data.contains("setLamp"))
    {
        if(data.contains("ON")) pCollimatore->setLamp(Collimatore::LAMP_ON,20);
        else if(data.contains("OFF")) pCollimatore->setLamp(Collimatore::LAMP_OFF,0);
        return;
    }

    if(data.contains("setCalib2D"))
    {
        handleSetCalib2D(data);
        return;
    }

    if(data.contains("setTrx"))
    {
        handleMoveTrx("setTrx ", data);
        return;
    }


    // Imposta le lame ma solo in modalitÃ  manuale
    if(pCollimatore->manualCollimation==TRUE)
    {
        if(data.contains("setL")) // Imposta le lame correnti
        {
            field = getNextFieldAfterTag(data,"setL ");
            serviceTcp->txData(QString("LAMA SINISTRA:%1 \r\n").arg(field.toInt()).toAscii());
            setManualLameVal("L",field.toInt());

        }else if(data.contains("setR")) // Imposta le lame correnti
        {
            field = getNextFieldAfterTag(data,"setR ");
            serviceTcp->txData(QString("LAMA DESTRA:%1 \r\n").arg(field.toInt()).toAscii());
            setManualLameVal("R",field.toInt());

        }else if(data.contains("setT")) // Imposta le lame correnti
        {
            field = getNextFieldAfterTag(data,"setT ");
            serviceTcp->txData(QString("LAMA TRAPEZIO:%1 \r\n").arg(field.toInt()).toAscii());
            setManualLameVal("T",field.toInt());

        }else if(data.contains("setB")) // Imposta le lame correnti
        {
            field = getNextFieldAfterTag(data,"setB ");
            serviceTcp->txData(QString("LAMA POSTERIORE:%1 \r\n").arg(field.toInt()).toAscii());
            setManualLameVal("B",field.toInt());

        }else if(data.contains("setF")) // Imposta le lame correnti
        {
            field = getNextFieldAfterTag(data,"setF ");
            serviceTcp->txData(QString("LAMA FRONTALE:%1 \r\n").arg(field.toInt()).toAscii());
            setManualLameVal("F",field.toInt());
        }
    }



}



/*
 *  Funzioni legate all'interazione con i drivers
 */
#define SER422_COMMAND  0x0
#define SER422_SPECIAL  0x40
#define SER422_READ     0x80
#define SER422_WRITE    0xC0
void serverDebug::handleDrivers(QByteArray data)
{
    if(data.contains("?"))
    {
        serviceTcp->txData(QByteArray("------------ Comandi diretti ai drivers ------------\r\n"));
        serviceTcp->txData(QByteArray("freeze                       Blocca ciclo automatico\r\n"));
        serviceTcp->txData(QByteArray("run                          Attiva ciclo automatico\r\n"));
        serviceTcp->txData(QByteArray("read8 <target,addr>           Legge indirizzo 8 bit\r\n"));
        serviceTcp->txData(QByteArray("read16 <target,addr>         Legge indirizzo 16 bit\r\n"));
        serviceTcp->txData(QByteArray("write8 <target,addr,val>      Scrive indirizzo 8 bit\r\n"));
        serviceTcp->txData(QByteArray("write16 <target,addr,val>    Scrive indirizzo 16 bit\r\n"));
        serviceTcp->txData(QByteArray("command <target,b1,b2>       Scrive frame di comando\r\n"));
        serviceTcp->txData(QByteArray("special <target,b1,b2>       Scrive frame speciale\r\n"));
        serviceTcp->txData(QByteArray("----------------------------------------------------\r\n"));
    }
    else if(data.contains("freeze")) handleDriverFreeze(TRUE);
    else if(data.contains("run")) handleDriverFreeze(FALSE);
    else if(data.contains("read8")) handleDriverRead8(data);
    else if(data.contains("read16")) handleDriverRead16(data);
    else if(data.contains("write8")) handleDriverWrite8(data);
    else if(data.contains("write16")) handleDriverWrite16(data);
    else if(data.contains("command")) handleDriverCommand(data);
    else if(data.contains("special")) handleDriverSpecial(data);

}



// Restituisce un intero valutando il formato della stringa
int serverDebug::getVal(QString val){
    bool ok;
    if(val.contains("0x")){
        val = val.replace("0x","");
        return val.toAscii().toInt(&ok,16);
    }
    return val.toInt();

}

// read8 target,address
void serverDebug::handleDriverRead8(QByteArray data)
{
    QByteArray buffer;
    unsigned char target;
    unsigned char indirizzo;
    unsigned char banco;

    isCommand = false;
    isSpecial = false;

    QList<QByteArray> parametri;

    parametri = getNextFieldsAfterTag(data, QString("read8"));
    if(parametri.size()!=2)
    {
        serviceTcp->txData(QByteArray("wrong parametrs\n\r"));
        return;
    }

    // Controllo indirizzo
    if(parametri[0]=="PCB269") target = 0x11;
    else if(parametri[0]=="PCB204") target = 0x0B;
    else if(parametri[0]=="PCB190") target = 0x13;
    else if(parametri[0]=="PCB249U1") target = 0x16;
    else if(parametri[0]=="PCB249U2") target = 0x15;
    else if(parametri[0]=="PCB244A") target = 0x17;
    else{
        serviceTcp->txData(QByteArray("invalid target\n\r"));
        return;
    }
    frameTarget = parametri[0];

    // Determina l'indirizzo
    int addr = getVal(parametri[1]);
    if(addr > 255) banco = 1;
    else banco = 0;
    indirizzo = (unsigned char) (0x00FF & addr);

    // Costruisce il buffer
    frameD0 = (unsigned char) (target | SER422_READ);
    buffer.append(frameD0);
    frameD1 = (unsigned char) indirizzo;
    buffer.append(frameD1);
    frameD2 = (unsigned char) banco;
    buffer.append(frameD2);

    frameWrite = false;
    frameData = 0;
    frameFormat16 = false;
    frameDH = false;
    frameCompleted = false;

    if(mccService(1,SRV_SERIAL_SEND,buffer)== FALSE) serviceTcp->txData("MCC FALLITO");
    else connect(pConsole,SIGNAL(mccServiceNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(handleDriverSendNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

    return;


}

// read16 target,address
void serverDebug::handleDriverRead16(QByteArray data){
    QByteArray buffer;
    unsigned char target;
    unsigned char indirizzo;
    unsigned char banco;

    isCommand = false;
    isSpecial = false;

    QList<QByteArray> parametri;

    parametri = getNextFieldsAfterTag(data, QString("read16"));
    if(parametri.size()!=2)
    {
        serviceTcp->txData(QByteArray("wrong parametrs\n\r"));
        return;
    }

    // Controllo indirizzo
    if(parametri[0]=="PCB269") target = 0x11;
    else if(parametri[0]=="PCB204") target = 0x0B;
    else if(parametri[0]=="PCB190") target = 0x13;
    else if(parametri[0]=="PCB249U1") target = 0x16;
    else if(parametri[0]=="PCB249U2") target = 0x15;
    else if(parametri[0]=="PCB244A") target = 0x17;
    else{
        serviceTcp->txData(QByteArray("invalid target\n\r"));
        return;
    }
    frameTarget = parametri[0];

    // Determina l'indirizzo
    int addr = getVal(parametri[1]);
    if(addr > 255) banco = 1;
    else banco = 0;
    indirizzo = (unsigned char) (0x00FF & addr);

    // Costruisce il buffer
    frameD0 = (unsigned char) (target | SER422_READ);
    buffer.append(frameD0);
    frameD1 = (unsigned char) indirizzo;
    buffer.append(frameD1);
    frameD2 = (unsigned char) banco;
    buffer.append(frameD2);

    frameWrite = false;
    frameData = 0;
    frameFormat16 = true;
    frameDH = false;
    frameCompleted = false;

    if(mccService(1,SRV_SERIAL_SEND,buffer)== FALSE) serviceTcp->txData("MCC FALLITO");
    else connect(pConsole,SIGNAL(mccServiceNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(handleDriverSendNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

    return;

}

// write8 target,address,val
void serverDebug::handleDriverWrite8(QByteArray data){
    QByteArray buffer;
    unsigned char target;
    unsigned char indirizzo;

    isCommand = false;
    isSpecial = false;

    QList<QByteArray> parametri;

    parametri = getNextFieldsAfterTag(data, QString("write8"));
    if(parametri.size()!=3)
    {
        serviceTcp->txData(QByteArray("wrong parameters\n\r"));
        return;
    }

    // Controllo indirizzo
    if(parametri[0]=="PCB269") target = 0x11;
    else if(parametri[0]=="PCB204") target = 0x0B;
    else if(parametri[0]=="PCB190") target = 0x13;
    else if(parametri[0]=="PCB249U1") target = 0x16;
    else if(parametri[0]=="PCB249U2") target = 0x15;
    else if(parametri[0]=="PCB244A") target = 0x17;
    else{
        serviceTcp->txData(QByteArray("invalid target\n\r"));
        return;
    }
    frameTarget = parametri[0];

    // Determina l'indirizzo
    int addr = getVal(parametri[1]);
    if(addr > 255){
        serviceTcp->txData(QByteArray("invalid address\n\r"));
        return;
    }
    indirizzo = (unsigned char) (addr);

    // Determina il valore
    frameData = getVal(parametri[2]);

    // Costruisce il buffer
    frameD0 = (unsigned char) (target | SER422_WRITE);
    buffer.append(frameD0);
    frameD1 = (unsigned char) indirizzo;
    buffer.append(frameD1);
    frameD2 = (unsigned char) (frameData & 0x00FF);
    buffer.append(frameD2);

    frameWrite = true;
    frameFormat16 = false;
    frameDH = false;
    frameCompleted = false;

    if(mccService(1,SRV_SERIAL_SEND,buffer)== FALSE) serviceTcp->txData("MCC FALLITO");
    else connect(pConsole,SIGNAL(mccServiceNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(handleDriverSendNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

    return;

}
void serverDebug::handleDriverWrite16(QByteArray data){
    QByteArray buffer;
    unsigned char target;
    unsigned char indirizzo;

    isCommand = false;
    isSpecial = false;

    QList<QByteArray> parametri;

    parametri = getNextFieldsAfterTag(data, QString("write16"));
    if(parametri.size()!=3)
    {
        serviceTcp->txData(QByteArray("wrong parameters\n\r"));
        return;
    }

    // Controllo indirizzo
    if(parametri[0]=="PCB269") target = 0x11;
    else if(parametri[0]=="PCB204") target = 0x0B;
    else if(parametri[0]=="PCB190") target = 0x13;
    else if(parametri[0]=="PCB249U1") target = 0x16;
    else if(parametri[0]=="PCB249U2") target = 0x15;
    else if(parametri[0]=="PCB244A") target = 0x17;
    else{
        serviceTcp->txData(QByteArray("invalid target\n\r"));
        return;
    }
    frameTarget = parametri[0];

    // Determina l'indirizzo
    int addr = getVal(parametri[1]);
    if(addr > 255){
        serviceTcp->txData(QByteArray("invalid address\n\r"));
        return;
    }
    indirizzo = (unsigned char) (addr);

    // Determina il valore
    frameData = getVal(parametri[2]);

    // Costruisce il buffer
    frameD0 = (unsigned char) (target | SER422_WRITE);
    buffer.append(frameD0);
    frameD1 = (unsigned char) indirizzo;
    buffer.append(frameD1);
    frameD2 = (unsigned char) (frameData & 0x00FF);
    buffer.append(frameD2);

    frameWrite = true;
    frameFormat16 = true;
    frameDH = false;
    frameCompleted = false;

    if(mccService(1,SRV_SERIAL_SEND,buffer)== FALSE) serviceTcp->txData("MCC FALLITO");
    else connect(pConsole,SIGNAL(mccServiceNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(handleDriverSendNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

    return;


}

// command target,b1,b2
void serverDebug::handleDriverCommand(QByteArray data){
    QByteArray buffer;
    unsigned char target;
    unsigned char b1;
    unsigned char b2;

    isCommand = true;
    isSpecial = false;

    QList<QByteArray> parametri;

    parametri = getNextFieldsAfterTag(data, QString("command"));
    if(parametri.size()!=3)
    {
        serviceTcp->txData(QByteArray("wrong parametrs\n\r"));
        return;
    }

    // Controllo target
    if(parametri[0]=="PCB269") target = 0x11;
    else if(parametri[0]=="PCB204") target = 0x0B;
    else if(parametri[0]=="PCB190") target = 0x13;
    else if(parametri[0]=="PCB249U1") target = 0x16;
    else if(parametri[0]=="PCB249U2") target = 0x15;
    else if(parametri[0]=="PCB244A") target = 0x17;
    else{
        serviceTcp->txData(QByteArray("invalid target\n\r"));
        return;
    }
    frameTarget = parametri[0];

    // Determina b1
    int val = getVal(parametri[1]);
    if(val > 255){
        serviceTcp->txData(QByteArray("invalid b1 size\n\r"));
        return;
    }
    b1 = (unsigned char) val;

    // Determina b1
    val = getVal(parametri[2]);
    if(val > 255){
        serviceTcp->txData(QByteArray("invalid b2 size\n\r"));
        return;
    }
    b2 = (unsigned char) val;

    // Costruisce il buffer
    frameD0 = (unsigned char) (target | SER422_COMMAND);
    buffer.append(frameD0);
    frameD1 = (unsigned char) b1;
    buffer.append(frameD1);
    frameD2 = (unsigned char) b2;
    buffer.append(frameD2);

    frameWrite = false;
    frameData = 0;
    frameFormat16 = false;
    frameDH = false;
    frameCompleted = false;

    if(mccService(1,SRV_SERIAL_SEND,buffer)== FALSE) serviceTcp->txData("MCC FALLITO");
    else connect(pConsole,SIGNAL(mccServiceNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(handleDriverSendNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

    return;

}
void serverDebug::handleDriverSpecial(QByteArray data){
    QByteArray buffer;
    unsigned char target;
    unsigned char b1;
    unsigned char b2;

    isCommand = false;
    isSpecial = true;

    QList<QByteArray> parametri;

    parametri = getNextFieldsAfterTag(data, QString("special"));
    if(parametri.size()!=3)
    {
        serviceTcp->txData(QByteArray("wrong parametrs\n\r"));
        return;
    }

    // Controllo target
    if(parametri[0]=="PCB269") target = 0x11;
    else if(parametri[0]=="PCB190") target = 0x13;
    else if(parametri[0]=="PCB249U1") target = 0x16;
    else if(parametri[0]=="PCB249U2") target = 0x15;
    else if(parametri[0]=="PCB244A") target = 0x17;
    else{
        serviceTcp->txData(QByteArray("invalid target\n\r"));
        return;
    }
    frameTarget = parametri[0];

    // Determina b1
    int val = getVal(parametri[1]);
    if(val > 255){
        serviceTcp->txData(QByteArray("invalid b1 size\n\r"));
        return;
    }
    b1 = (unsigned char) val;

    // Determina b1
    val = getVal(parametri[2]);
    if(val > 255){
        serviceTcp->txData(QByteArray("invalid b2 size\n\r"));
        return;
    }
    b2 = (unsigned char) val;

    // Costruisce il buffer
    frameD0 = (unsigned char) (target | SER422_SPECIAL);
    buffer.append(frameD0);
    frameD1 = (unsigned char) b1;
    buffer.append(frameD1);
    frameD2 = (unsigned char) b2;
    buffer.append(frameD2);

    frameWrite = false;
    frameData = 0;
    frameFormat16 = false;
    frameDH = false;
    frameCompleted = false;

    if(mccService(1,SRV_SERIAL_SEND,buffer)== FALSE) serviceTcp->txData("MCC FALLITO");
    else connect(pConsole,SIGNAL(mccServiceNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(handleDriverSendNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

    return;

}


void serverDebug::handleDriverSendNotify(unsigned char id,unsigned char cmd, QByteArray data)
{
    if(cmd!=SRV_SERIAL_SEND) return;
    if(frameCompleted){
        disconnect(pConsole,SIGNAL(mccServiceNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(handleDriverSendNotify(unsigned char,unsigned char,QByteArray)));
        return;
    }


    if(isCommand){
        frameCompleted = true;
        serviceTcp->txData(QString("%1-COMMAND RET: %2 %3 (0x%4 0x%5)\n\r").arg(frameTarget).arg((int)data[1],0,10).arg((int)data[2],0,10).arg((int)data[1],0,16).arg((int)data[2],0,16).toAscii());
    }else if(isSpecial){
        frameCompleted = true;
        serviceTcp->txData(QString("%1-SPECIAL RET: %2 %3 (0x%4 0x%5)\n\r").arg(frameTarget).arg((int)data[1],0,10).arg((int)data[2],0,10).arg((int)data[1],0,16).arg((int)data[2],0,16).toAscii());
    }else if(frameWrite){ // Operazione di scrittura
        if(frameFormat16){
            if(frameDH){
                frameCompleted = true;
                serviceTcp->txData(QString("%1[0x%2]: Written!\n\r").arg(frameTarget).arg((int)(data[1]-1),0,16).toAscii());
            }else{
                QByteArray buffer;
                buffer.append(frameD0);
                buffer.append(frameD1+1);
                buffer.append((unsigned char) ((frameData & 0xFF00)>>8));
                mccService(1,SRV_SERIAL_SEND,buffer);
                frameDH = true;
                return;
            }

        }else{
             frameCompleted = true;
             serviceTcp->txData(QString("%1[0x%2]: Written!\n\r").arg(frameTarget).arg((int)data[1],0,16).toAscii());
        }
    }else{// Operazione di lettura
        if(frameFormat16){
            if(frameDH){
                frameCompleted = true;
                serviceTcp->txData(QString("%1[0x%2]:%3 (0x%4)\n\r").arg(frameTarget).arg((int)(data[1]-1),0,16).arg((int)data[2]*256+frameData).arg((int)data[2]*256+frameData,0,16).toAscii());
            }else{
                frameData = data[2];
                QByteArray buffer;
                buffer.append(frameD0);
                buffer.append(frameD1+1);
                buffer.append(frameD2);
                mccService(1,SRV_SERIAL_SEND,buffer);
                frameDH = true;
                return;
            }
        }else{
             frameCompleted = true;
             serviceTcp->txData(QString("%1[0x%2]:%3 (0x%4)\n\r").arg(frameTarget).arg((int)data[1],0,16).arg((int)data[2]).arg((int)data[2],0,16).toAscii());
        }
    }

    disconnect(pConsole,SIGNAL(mccServiceNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(handleDriverSendNotify(unsigned char,unsigned char,QByteArray)));
}

void serverDebug::handleLoader(QByteArray data)
{
    if(data.contains("?"))
    {
        serviceTcp->txData(QByteArray("loader: ------------   Comandi diretti ai drivers ------------------------------\r\n"));        
        serviceTcp->txData(QByteArray("UPLOAD <Target>  Carica il firmware relativo al target \r\n"));
        serviceTcp->txData(QByteArray("setFormat <target,revision>  Formatta il file hex con la revisione dichiarata\r\n"));
        serviceTcp->txData(QByteArray("CONFIG <Target>, <uC>              Richiede la Config Area di un target\r\n"));
        serviceTcp->txData(QByteArray("getCRC  nome_file      calcola il CRC da inserire nel file destinazione in /home/user \r\n"));
        serviceTcp->txData(QByteArray("setCRC  <target,revision>  Formatta il target file con <CRC,REV> \r\n"));
        serviceTcp->txData(QByteArray("setRemoteCRC  <target,revision>  Formatta il target file (remoto) con <CRC,REV> \r\n"));
        serviceTcp->txData(QByteArray("----------------------------------------------------------------------------------\r\n"));
    }
    else if(data.contains("UPLOAD")) handleLoaderUpload(data);
    else if(data.contains("CONFIG")) handleReadConfig(data);
    else if(data.contains("getCRC")) handleGetCRC(data);
    else if(data.contains("setCRC")) handleSetCRC(data);
    else if(data.contains("setRemoteCRC")) handleSetRemoteCRC(data);


}


void serverDebug::handleSetLanguage(QByteArray data)
{
    QList<QByteArray> parametri;

    parametri = getNextFieldsAfterTag(data, QString("setLanguage"));
    if(parametri.size()!=1)
    {
        serviceTcp->txData(QByteArray("Inserire il codice lingua di 3 lettere maiuscole\n\r"));
        return;
    }

    if((parametri[0]=="ITA") || (parametri[0]=="ENG") || (parametri[0]=="FRA") ||
            (parametri[0]=="DEU") || (parametri[0]=="PRT") || (parametri[0]=="RUS") ||
            (parametri[0]=="ESP") || (parametri[0]=="TUR") || (parametri[0]=="POL") ||
            (parametri[0]=="CHN")|| (parametri[0]=="LTU")){

            ApplicationDatabase.setData(_DB_LINGUA,QString(parametri[0]), 0);
            pConfig->userCnf.languageInterface = parametri[0];
            pConfig->saveUserCfg();

    }else{
        serviceTcp->txData(QByteArray("Lingue disponibili:\n\r"));
        serviceTcp->txData(QByteArray("ITA\n\r"));
        serviceTcp->txData(QByteArray("ENG\n\r"));
        serviceTcp->txData(QByteArray("FRA\n\r"));
        serviceTcp->txData(QByteArray("DEU\n\r"));
        serviceTcp->txData(QByteArray("PRT\n\r"));
        serviceTcp->txData(QByteArray("RUS\n\r"));
        serviceTcp->txData(QByteArray("ESP\n\r"));
        serviceTcp->txData(QByteArray("TUR\n\r"));
        serviceTcp->txData(QByteArray("POL\n\r"));
        serviceTcp->txData(QByteArray("CHN\n\r"));
        serviceTcp->txData(QByteArray("LTU\n\r"));
    }


    return;
}



void serverDebug::handleSystem(QByteArray data)
{
    QString field;
    if(data.contains("?"))
    {
        serviceTcp->txData(QByteArray("system:----------------- Comandi di gestione files di configurazione -\r\n"));
        serviceTcp->txData(QByteArray("setDATE Y M D h m s      imposta data e ora corrente \r\n"));
        serviceTcp->txData(QByteArray("getRevisions             Richiede la configurazione del sistema\r\n"));
        serviceTcp->txData(QByteArray("reboot                   Effettua il reboot di entrambi i terminali\r\n"));
        serviceTcp->txData(QByteArray("setPowerOff              Attiva il powerdown \r\n"));
        serviceTcp->txData(QByteArray("sysLog  [ON/OFF/FLUSH]   Imposta la funzione log \r\n"));
        serviceTcp->txData(QByteArray("------------------------------------------------------------------------\r\n"));
    } else
    {
        if(data.contains("sysLog")){
            QList<QByteArray> parametri = getNextFieldsAfterTag(data, QString("sysLog"));
            if(parametri.size() != 1) {
                serviceTcp->txData(QString("Invalid parameters\n\r").toAscii());
                return;
            }
            if(parametri[0]=="ON"){
                pSysLog->activate(false);
                pSysLog->activate(true);
                serviceTcp->txData(QByteArray("LOG ACTIVATED AND FLUSHED!\r\n"));
            }else if(parametri[0]=="OFF"){
                pSysLog->activate(false);
                serviceTcp->txData(QByteArray("LOG DISABLED AND FLUSHED!\r\n"));
            }else{
                serviceTcp->txData(QByteArray("LOG ACTIVATED AND FLUSHED!\r\n"));
                pSysLog->activate(false);
                pSysLog->activate(true);
            }

            return;
        }else if(data.contains("setDATE"))
        {
            if(systemTimeUpdated) {
                serviceTcp->txData(QString("System Time already updated!\n\r").toAscii());
                return;
            }

            QList<QByteArray> parametri = getNextFieldsAfterTag(data, QString("setDATE"));
            if(parametri.size() != 6) {
                serviceTcp->txData(QString("Invalid parameters number (should be six)\n\r").toAscii());
                return;
            }

            QString Y = QString(parametri[0]);
            QString M = QString(parametri[1]);
            QString D = QString(parametri[2]);
            QString h = QString(parametri[3]);
            QString m = QString(parametri[4]);
            QString s = QString(parametri[5]);

            echoDisplay.echoDate(D,M,Y,h,m,s,DBase::_DB_NO_OPT);

            QString command;
            command = QString("date -u %1%2%3%4%5.%6").arg(M).arg(D).arg(h).arg(m).arg(Y).arg(s);
            system(command.toStdString().c_str());

            command = QString("hwclock -w");
            system(command.toStdString().c_str());

            systemTimeUpdated = TRUE;

            // Con la ricezione della data è possibile inizializzare il dispositivo
            // per la misura della quantità di calore accumulata nell'Anodo
            pGeneratore->initAnodeHU();

            return;

        }else  if(data.contains("getRevisions"))
        {
            handleGetRevisions();
        }else if(data.contains("reboot"))
        {
            pConfig->executeReboot();
        }else if(data.contains("setPowerOff")){
            QList<QByteArray> parametri = getNextFieldsAfterTag(data, QString("setPowerOff"));
            if(parametri.size() != 1) {
                serviceTcp->txData(QString("Invalid parameters number \n\r").toAscii());
                return;
            }

            pConfig->executePoweroff((unsigned char) parametri[0].toInt());

        }


    }
}




void serverDebug::handleGetAlarmInfo(QByteArray data)
{
    QList<QByteArray> parametri;


    parametri = getNextFieldsAfterTag(data, QString("getAlarmInfo"));


    if(parametri.size()!=1){
        serviceTcp->txData("PARAMETRI NON CORRETTI \n\r");
        return;
    }

    PageAlarms::_alarmStruct* pErr = paginaAllarmi->getErrorInfo( parametri[0].toInt());
    if(pErr==0){
        serviceTcp->txData("NESSUN ERRORE ASSOCIATO \n\r");
        return;
    }

    serviceTcp->txData(QString("ERRORE: \"%1\"\n\r").arg(pErr->codestr).toAscii());
    serviceTcp->txData(QString("MSG: \"%1\"\n\r").arg(pErr->errmsg).toAscii());
    serviceTcp->txData(QString("INTERNAL CODE: %1 \n\r").arg(pErr->codeval).toAscii());
    serviceTcp->txData(QString("DESCRIPTION: \"%1\" \n\r").arg(pErr->errdescr).toAscii());

    return;
}



#define OUT_MASTER_ENA "MASTER_ENA"
#define OUT_COMPRESSOR_ENA "COMPRESSOR_ENA"
#define OUT_PEND_ENA "PEND_ENA"
#define OUT_ROT_ENA "ROT_ENA"
#define OUT_LIFT_ENA "LIFT_ENA"
#define OUT_XRAY_ENA "XRAY_ENA"
#define OUT_SYS_FAULT "SYS_FAULT"

#define OUT_LAMP_SW1 "LAMP_SW1"
#define OUT_LAMP_SW2 "LAMP_SW2"
#define OUT_XRAY_LED "XRAY_LED"
#define OUT_BUZ_ON "BUZ_ON"
#define OUT_BUZ_MODE "BUZ_MODE"
#define OUT_DEMO "DEMO"
#define OUT_DEMO_CODE "DEMO_CODE"
#define OUT_BURNING "BURNING"

#define OUT_LOADER_PWR_ON "LOADER_ON"
#define OUT_POWER_OFF_REQ "POWER_OFF"
#define OUT_BATT_TEST_REQ "BATT_TEST"

void serverDebug::handleSetOutputs(QByteArray data)
{
    /* TBD
    ioOutputs o;
    QList<QByteArray> parametri;
    parametri = getNextFieldsAfterTag(data, QString("setOutputs"));

    if(parametri.size()!=2){
        //Lista outputs disponibili
        serviceTcp->txData(QByteArray("Available Output tags:\r\n"));
        serviceTcp->txData(QString("%1\r\n").arg(OUT_MASTER_ENA).toAscii());
        serviceTcp->txData(QString("%1\r\n").arg(OUT_COMPRESSOR_ENA).toAscii());
        serviceTcp->txData(QString("%1\r\n").arg(OUT_PEND_ENA).toAscii());
        serviceTcp->txData(QString("%1\r\n").arg(OUT_ROT_ENA).toAscii());
        serviceTcp->txData(QString("%1\r\n").arg(OUT_LIFT_ENA).toAscii());
        serviceTcp->txData(QString("%1\r\n").arg(OUT_XRAY_ENA).toAscii());
        serviceTcp->txData(QString("%1\r\n").arg(OUT_SYS_FAULT).toAscii());


        serviceTcp->txData(QString("%1\r\n").arg(OUT_LAMP_SW1).toAscii());
        serviceTcp->txData(QString("%1\r\n").arg(OUT_LAMP_SW2).toAscii());
        serviceTcp->txData(QString("%1\r\n").arg(OUT_XRAY_LED).toAscii());
        serviceTcp->txData(QString("%1\r\n").arg(OUT_BUZ_ON).toAscii());
        serviceTcp->txData(QString("%1\r\n").arg(OUT_BUZ_MODE).toAscii());
        serviceTcp->txData(QString("%1\r\n").arg(OUT_DEMO).toAscii());
        serviceTcp->txData(QString("%1\r\n").arg(OUT_DEMO_CODE).toAscii());
        serviceTcp->txData(QString("%1\r\n").arg(OUT_BURNING).toAscii());

        serviceTcp->txData(QString("%1\r\n").arg(OUT_LOADER_PWR_ON).toAscii());
        serviceTcp->txData(QString("%1\r\n").arg(OUT_POWER_OFF_REQ).toAscii());
        serviceTcp->txData(QString("%1\r\n").arg(OUT_BATT_TEST_REQ).toAscii());
        return;
    }

    int val;
    if(parametri[1].toInt()) val = 1;
    else val = 0;

    if(parametri[0]==OUT_MASTER_ENA){
        o.mask.CPU_MASTER_ENA=1;
        o.outputs.CPU_MASTER_ENA=val;
    }else if(parametri[0]==OUT_COMPRESSOR_ENA){
        o.mask.CPU_COMPRESSOR_ENA=1;
        o.outputs.CPU_COMPRESSOR_ENA=val;
    }else if(parametri[0]==OUT_ROT_ENA){
        o.mask.CPU_ROT_ENA=1;
        o.outputs.CPU_ROT_ENA=val;
    }else if(parametri[0]==OUT_LIFT_ENA){
        o.mask.CPU_LIFT_ENA=1;
        o.outputs.CPU_LIFT_ENA=val;
    }else if(parametri[0]==OUT_PEND_ENA){
        o.mask.CPU_PEND_ENA=1;
        o.outputs.CPU_PEND_ENA=val;
    }else if(parametri[0]==OUT_SYS_FAULT){
        o.mask.CPU_SYS_FAULT=1;
        o.outputs.CPU_SYS_FAULT=val;
    }else if(parametri[0]==OUT_XRAY_ENA){
        o.mask.CPU_XRAY_ENA=1;
        o.outputs.CPU_XRAY_ENA=val;
    }

    else if(parametri[0]==OUT_LAMP_SW1){
            o.mask.CPU_LMP_SW1=1;
            o.outputs.CPU_LMP_SW1=val;
    }else if(parametri[0]==OUT_LAMP_SW2){
            o.mask.CPU_LMP_SW2=1;
            o.outputs.CPU_LMP_SW2=val;
    }else if(parametri[0]==OUT_XRAY_LED){
        o.mask.CPU_XRAY_LED=1;
        o.outputs.CPU_XRAY_LED=val;
    }else if(parametri[0]==OUT_BUZ_ON){
        o.mask.CPU_BUZ_ON=1;
        o.outputs.CPU_BUZ_ON=val;
    }else if(parametri[0]==OUT_BUZ_MODE){
        o.mask.CPU_BUZ_MODE=1;
        o.outputs.CPU_BUZ_MODE=val;
    }else if(parametri[0]==OUT_DEMO){
        o.mask.CPU_DEMO=1;
        o.outputs.CPU_DEMO=val;
    }else if(parametri[0]==OUT_DEMO_CODE){
        o.mask.CPU_DEMO_CODE=1;
        o.outputs.CPU_DEMO_CODE=val;
    }

    else if(parametri[0]==OUT_LOADER_PWR_ON){
        o.mask.CPU_LOADER_PWR_ON=1;
        o.outputs.CPU_LOADER_PWR_ON=val;
    }else if(parametri[0]==OUT_BATT_TEST_REQ){
        o.mask.CPU_BATT_TEST_REQ=1;
        o.outputs.CPU_BATT_TEST_REQ=val;
    }


    io->setOutputs(o);
    */
}



void serverDebug::handleShell(QByteArray data)
{

    data.replace("shell:","");
    data.replace("\n","");
    data.replace("\r","");

    QString command = QString(data) + QString(" > ppp");
    system(command.toStdString().c_str());


    QFile file("ppp");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        serviceTcp->txData(QByteArray("NON LEGGE IL RISULTATO\n"));
        return;
    }

    QByteArray buffer;
    while(!file.atEnd())
    {
        buffer = file.readLine();
        buffer.append("\r");
        serviceTcp->txData(buffer);
    }
    file.close();


}

void serverDebug::handleMasterShell(QByteArray data)
{

    data.replace("master:","");
    data.replace("\n","");
    data.replace("\r","");

    bool noCmd = true;
    for(int i = 0; i< data.size(); i++){
        if((data.at(i) != ' ' ) && (data.at(i) != 0)) {
            noCmd=false;
            break;
        }
    }
    if(noCmd) return;

    QByteArray risultato = pConfig->executeShell(data);
    serviceTcp->txData(risultato);


}

void serverDebug::handleSlaveShell(QByteArray data)
{

    data.replace("slave:","");
    data.replace("\n","");
    data.replace("\r","");

    bool noCmd = true;
    for(int i = 0; i< data.size(); i++){
        if((data.at(i) != ' ' ) && (data.at(i) != 0)) {
            noCmd=false;
            break;
        }
    }
    if(noCmd) return;


    connect(pConfig,SIGNAL(sgnRemoteShell(QByteArray)),this,SLOT(slotRemoteShell(QByteArray)),Qt::UniqueConnection);
    pConfig->executeSlaveShell(data);

}

void serverDebug::slotRemoteShell(QByteArray data)
{
    serviceTcp->txData(data);
    disconnect(pConfig,SIGNAL(sgnRemoteShell(QByteArray)),this,SLOT(slotRemoteShell(QByteArray)));
}



void serverDebug::handleCanOpen(QByteArray data)
{
    if(data.contains("?"))
    {
        serviceTcp->txData(QByteArray("----------------------------------------------------------------------------------\r\n"));
        serviceTcp->txData(QByteArray("test:  <val> [val val..]     \r\n"));
        serviceTcp->txData(QByteArray("lenzeConf                    \r\n"));
        serviceTcp->txData(QByteArray("trxConf                      \r\n"));
        serviceTcp->txData(QByteArray("armConf                      \r\n"));
        serviceTcp->txData(QByteArray("----------------------------------------------------------------------------------\r\n"));
    }else if(data.contains("test"))
    {
        handleCanOpen_test(data);
    }else if(data.contains("lenzeConf"))
    {
        pConfig->updateLenzeDriver();
    }else if(data.contains("trxConf"))
    {
        pConfig->updateTrxDriver();
    }else if(data.contains("armConf"))
    {
        pConfig->updateArmDriver();
    }

}

// Invia un certo numero di uint32_t verso i driver
// per debug generico su dispositivi CANOPEN
void serverDebug::handleCanOpen_test(QByteArray data)
{
    unsigned char buffer[8];
    QList<QByteArray> parametri;
    parametri = getNextFieldsAfterTag(data, QString("test"));

    int len = parametri.size();
    if(len){
        buffer[0]= (unsigned char) parametri.at(0).toInt();
        if(len>1) {
            int16_t val = parametri.at(1).toShort();
            buffer[1]= (unsigned char) val;
            buffer[2]= (unsigned char) (val>>8);
        }

        if(len>2) {
            int32_t val = parametri.at(2).toLong();
            buffer[3]= (unsigned char) val;val=val>>8;
            buffer[4]= (unsigned char) val;val=val>>8;
            buffer[5]= (unsigned char) val;val=val>>8;
            buffer[6]= (unsigned char) val;

        }

    }

    pConsole->pGuiMcc->sendFrame(MCC_CANOPEN,0,buffer, 7);

}

void serverDebug::handlePotter(QByteArray data)
{
    if(data.contains("?"))
    {
        serviceTcp->txData(QByteArray("----------------------------------------------------------------------------------\r\n"));
        serviceTcp->txData(QByteArray("clearErrors:                 Reset Fault\r\n"));
        serviceTcp->txData(QByteArray("resetBoard:                  Reset Board\r\n"));
        serviceTcp->txData(QByteArray("setGrid2D: <ON/OFF>          Attivazione griglia 2D\r\n"));
        serviceTcp->txData(QByteArray("setGrid3D: <ON/OFF>          Attivazione griglia 3D\r\n"));
        serviceTcp->txData(QByteArray("setGridFreq:<freq>           Impostazione freequenza (0.1Hz/unit)\r\n"));
        serviceTcp->txData(QByteArray("setGridAmp:<ampiezza>        Impostazione ampiezza\r\n"));
        serviceTcp->txData(QByteArray("----------------------------------------------------------------------------------\r\n"));
    }else if(data.contains("clearErrors"))
    {
        handleClearPCB244Errors();
    }else if(data.contains("resetBoard"))
    {
        handleResetPCB244();
    }else if(data.contains("setGrid2D"))
    {
        handleSetGrid2D(data);
    }
}

/*

 */
void serverDebug::handleSetGrid2D(QByteArray data)
{
    QList<QByteArray> parametri;

    parametri = getNextFieldsAfterTag(data, QString("setGrid2D"));
    if(parametri.size()!=1){
        serviceTcp->txData(QByteArray("WRONG PARAMETER! Expected ON/OFF\n\r"));
        return;
    }

    QByteArray buf;

    if(parametri.at(0)=="ON"){
        serviceTcp->txData(QByteArray("Grid ON executed\n\r"));
        buf.append((unsigned char) SRV_START_POTTER_2D_GRID); // Comando di servizio
    }else{
        serviceTcp->txData(QByteArray("Grid OFF executed\n\r"));
        buf.append((unsigned char) SRV_STOP_POTTER_2D_GRID); // Comando di servizio
    }

    pConsole->pGuiMcc->sendFrame(MCC_SERVICE,0,(unsigned char*) buf.data(), buf.size());

}

void serverDebug::handleClearPCB244Errors(void){
    QByteArray buf;
    buf.append((unsigned char) SRV_RESET_FAULT_PCB244);
    pConsole->pGuiMcc->sendFrame(MCC_SERVICE,0,(unsigned char*) buf.data(), buf.size());
}

void serverDebug::handleResetPCB244(void){
    QByteArray buf;
    buf.append((unsigned char) SRV_RESET_PCB244);
    pConsole->pGuiMcc->sendFrame(MCC_SERVICE,0,(unsigned char*) buf.data(), buf.size());

}


void serverDebug::handleGetTrolleyNotify(unsigned char id,unsigned char cmd, QByteArray data){

    if(id!=1) return;
    if(cmd!=MCC_GET_TROLLEY) return;
    serviceTcp->txData(QString("Trolley position:  %1\n\r").arg((int) data.at(0)).toAscii());
    disconnect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(handleGetTrolleyNotify(unsigned char,unsigned char,QByteArray)));
    return;
}

void serverDebug::debugPrint(QString data){
    if(serviceTcp->connection_status==false) return;
    data.prepend("DEBUG:>");
    serviceTcp->txData(data.append("\n\r").toAscii());
}






void serverDebug::handleBiopsy(QByteArray data)
{
    QList<QByteArray> parametri;
    QByteArray cmddata;
    QString stringa;

    if(data.contains("?"))
    {
        serviceTcp->txData(QByteArray("--------------- ACTIVATION ----------------------------\r\n"));
        serviceTcp->txData(QByteArray("moveXYZ x,y,z         ? X,Y,Z in dmm \r\n"));
        serviceTcp->txData(QByteArray("moveHome              ? move to 0,0,0 \r\n"));
        serviceTcp->txData(QByteArray("moveInc    <X|Y|Z>    ? INC Z or Y or Z \r\n"));
        serviceTcp->txData(QByteArray("moveDec    <X|Y|Z>    ? DEC Z or Y or Z \r\n"));
        serviceTcp->txData(QByteArray("stepVal    val        ? set the Step value \r\n"));
        serviceTcp->txData(QByteArray("getStepval            ? return current stepval \r\n"));

        serviceTcp->txData(QByteArray("\r\n------------ WORKFLOW ----------------------------\r\n"));
        serviceTcp->txData(QByteArray("setAgo     val        ? set Ago lenght \r\n"));
        serviceTcp->txData(QByteArray("getAgo                ? return Ago lenght \r\n"));
        serviceTcp->txData(QByteArray("getMaxZ               ? return current maxZ \r\n"));
        serviceTcp->txData(QByteArray("getZlim               ? return current Zlimit \r\n"));
        serviceTcp->txData(QByteArray("getAdapter            ? return current Adapter Id \r\n"));
        serviceTcp->txData(QByteArray("getRevision           ? return current revision \r\n"));

        serviceTcp->txData(QByteArray("\r\n------------ LESION CALCULATION---------------------\r\n"));
        serviceTcp->txData(QByteArray("calcLesione (XRm,YRm XLm,YLm) ..(p)   ? return lesion calculation\r\n"));

        serviceTcp->txData(QByteArray("\r\n------------ CALIBRATION FILE -----------------------\r\n"));
        serviceTcp->txData(QByteArray("setCalX    val        ? (dmm) set Calib Offset X \r\n"));
        serviceTcp->txData(QByteArray("setCalY    val        ? (dmm) set Calib Offset Y \r\n"));
        serviceTcp->txData(QByteArray("setCalZ    val        ? (dmm) set Calib Offset Z \r\n"));
        serviceTcp->txData(QByteArray("setCalFibra    val    ? (mm) set  Distanza Fibra \r\n"));
        serviceTcp->txData(QByteArray("setStore              ? Store calib file \r\n"));
        serviceTcp->txData(QByteArray("showCalibFile         ? Show Calib File \r\n"));
        serviceTcp->txData(QByteArray("readCalibFile         ? Show Calib File \r\n"));

#ifdef __BIOPSY_SIMULATOR
        serviceTcp->txData(QByteArray("\n\r BIOPSY SIMULATOR COMMANDS: --------\r\n"));
        serviceTcp->txData(QByteArray("simConn    [ON/OFF]   ? ON=CONNECTED \r\n"));
        serviceTcp->txData(QByteArray("simAdapter [n]        ? N=ADAPTER CODE \r\n"));
        serviceTcp->txData(QByteArray("simSblocco [ON/OFF]   ? ON=UNLOCK \r\n"));
        serviceTcp->txData(QByteArray("simRST                ? Pulsante RST   \r\n"));
        serviceTcp->txData(QByteArray("simENT                ? Pulsante ENTER  \r\n"));
        serviceTcp->txData(QByteArray("simp_1                ? Pulsante +1 \r\n"));
        serviceTcp->txData(QByteArray("simp10                ? Pulsante +10 \r\n"));
        serviceTcp->txData(QByteArray("simBK                 ? Pulsante BACK \r\n"));
        serviceTcp->txData(QByteArray("simXY X,Y             ? JOYSTIC X,Y in (dmm)\r\n"));
        serviceTcp->txData(QByteArray("-----------------------------------------------------------------\r\n"));
#endif

    } else
    {
        if(data.contains("calcLesione")){
            parametri = getNextFieldsAfterTag(data, QString("calcLesione"));
            if(parametri.size()!=8){
                serviceTcp->txData(QByteArray("PARAMETRI ERRATI!\n"));
                return;
            }

            pBiopsy->dmm_ref_m15_JX = (parametri[0].toFloat()/pBiopsy->config.readerKX);
            pBiopsy->dmm_ref_m15_JY = (parametri[1].toFloat()/pBiopsy->config.readerKY);
            pBiopsy->dmm_les_m15_JX = (parametri[2].toFloat()/pBiopsy->config.readerKX);
            pBiopsy->dmm_les_m15_JY = (parametri[3].toFloat()/pBiopsy->config.readerKY);
            pBiopsy->dmm_ref_p15_JX = (parametri[4].toFloat()/pBiopsy->config.readerKX);
            pBiopsy->dmm_ref_p15_JY = (parametri[5].toFloat()/pBiopsy->config.readerKY);
            pBiopsy->dmm_les_p15_JX = (parametri[6].toFloat()/pBiopsy->config.readerKX);
            pBiopsy->dmm_les_p15_JY = (parametri[7].toFloat()/pBiopsy->config.readerKY);

            pBiopsy->calcLesionPosition();
            serviceTcp->txData(QString("BIO: X:%1, Y:%2, Z:%3\n\r").arg(pBiopsy->Xbio).arg(pBiopsy->Ybio).arg(pBiopsy->Zbio).toAscii().data());
            serviceTcp->txData(QString("TORRETTA: X:%1, Y:%2, Z:%3\n\r").arg(pBiopsy->Xlesione_dmm).arg(pBiopsy->Ylesione_dmm).arg(pBiopsy->Zlesione_dmm).toAscii().data());
            serviceTcp->txData(QString("FIBRA: Z:%1\n\r").arg(pBiopsy->Zfibra_dmm).toAscii().data());
            return;
        }
        if(data.contains("setCalX")){
            parametri = getNextFieldsAfterTag(data, QString("setCalX"));
            if(parametri.size()!=1){
                serviceTcp->txData(QByteArray("PARAMETRI ERRATI!\n"));
                return;
            }

            pBiopsy->config.offsetX = parametri[0].toInt();
            serviceTcp->txData(QString("CALIB OFFSET X:%1 (dmm)\n\r").arg(pBiopsy->config.offsetX).toAscii().data());
            return;
        }
        if(data.contains("setCalY")){
            parametri = getNextFieldsAfterTag(data, QString("setCalY"));
            if(parametri.size()!=1){
                serviceTcp->txData(QByteArray("PARAMETRI ERRATI!\n"));
                return;
            }

            pBiopsy->config.offsetY = parametri[0].toInt();
            serviceTcp->txData(QString("CALIB OFFSET Y:%1 (dmm)\n\r").arg(pBiopsy->config.offsetY).toAscii().data());
            return;
        }
        if(data.contains("setCalZ")){
            parametri = getNextFieldsAfterTag(data, QString("setCalZ"));
            if(parametri.size()!=1){
                serviceTcp->txData(QByteArray("PARAMETRI ERRATI!\n"));
                return;
            }

            pBiopsy->config.offsetZ = parametri[0].toInt();
            serviceTcp->txData(QString("CALIB OFFSET Z:%1 (dmm)\n\r").arg(pBiopsy->config.offsetZ).toAscii().data());
            return;
        }
        if(data.contains("setCalFibra")){
            parametri = getNextFieldsAfterTag(data, QString("setCalFibra"));
            if(parametri.size()!=1){
                serviceTcp->txData(QByteArray("PARAMETRI ERRATI!\n"));
                return;
            }

            pBiopsy->config.offsetFibra = parametri[0].toInt();
            serviceTcp->txData(QString("CALIB DISTANZA FIBRA:%1 (mm)\n\r").arg(pBiopsy->config.offsetFibra).toAscii().data());
            return;
        }
        if(data.contains("setStore")){
            if(pBiopsy->storeConfig()){
                serviceTcp->txData(QString("STORED !\n\r").toAscii().data());
                pBiopsy->updateConfig();
            }else serviceTcp->txData(QString("ERROR\n\r").toAscii().data());
            return;
        }
        if(data.contains("showCalibFile")){
            serviceTcp->txData(QString("CALIB OFFSET X:%1 (dmm)\n\r").arg(pBiopsy->config.offsetX).toAscii().data());
            serviceTcp->txData(QString("CALIB OFFSET Y:%1 (dmm)\n\r").arg(pBiopsy->config.offsetY).toAscii().data());
            serviceTcp->txData(QString("CALIB OFFSET Z:%1 (dmm)\n\r").arg(pBiopsy->config.offsetZ).toAscii().data());
            serviceTcp->txData(QString("CALIB DISTANZA FIBRA:%1 (mm)\n\r").arg(pBiopsy->config.offsetFibra).toAscii().data());
            return;
        }
        if(data.contains("readCalibFile")){
            if(pBiopsy->openCfg()){
                serviceTcp->txData(QString("OK !\n\r").toAscii().data());
                pBiopsy->updateConfig();
            }else serviceTcp->txData(QString("ERROR\n\r").toAscii().data());
            return;
        }

        if(!pBiopsy->connected){
            serviceTcp->txData(QByteArray("BIOPSY DEVICE DISCONNECTED!!\n\r"));
            return;
        }

        if(data.contains("moveXYZ")){
            parametri = getNextFieldsAfterTag(data, QString("moveXYZ"));
            if(parametri.size()!=3) serviceTcp->txData(QByteArray("PARAMETRI ERRATI!\n"));

            if(pBiopsy->moveXYZ(parametri.at(0).toUInt(),parametri.at(1).toUInt(),parametri.at(2).toUInt())==false){
                serviceTcp->txData(QByteArray("FALLITO COMANDO!\n"));
            }
        }else if(data.contains("moveHome")){
            pBiopsy->moveHome();
        }else if(data.contains("moveInc")){
            parametri = getNextFieldsAfterTag(data, QString("moveInc"));
            if(parametri.size()!=1) serviceTcp->txData(QByteArray("PARAMETRI ERRATI!\n"));
            if((parametri[0]=="X") || (parametri[0]=="x")){
                if(pBiopsy->moveIncX()==false){
                    serviceTcp->txData(QByteArray("FALLITO COMANDO!\n"));
                }

            }else if((parametri[0]=="Y") || (parametri[0]=="y")){
                if(pBiopsy->moveIncY()==false){
                    serviceTcp->txData(QByteArray("FALLITO COMANDO!\n"));
                }

            }else if((parametri[0]=="Z") || (parametri[0]=="z")){
                if(pBiopsy->moveIncZ()==false){
                    serviceTcp->txData(QByteArray("FALLITO COMANDO!\n"));
                }
            }

        }else if(data.contains("moveDec")){
            parametri = getNextFieldsAfterTag(data, QString("moveDec"));
            if(parametri.size()!=1) serviceTcp->txData(QByteArray("PARAMETRI ERRATI!\n"));
            if((parametri[0]=="X") || (parametri[0]=="x")){
                if(pBiopsy->moveDecX()==false){
                    serviceTcp->txData(QByteArray("FALLITO COMANDO!\n"));
                }
            }else if((parametri[0]=="Y") || (parametri[0]=="y")){
                if(pBiopsy->moveDecY()==false){
                    serviceTcp->txData(QByteArray("FALLITO COMANDO!\n"));
                }

            }else if((parametri[0]=="Z") || (parametri[0]=="z")){
                if(pBiopsy->moveDecZ()==false){
                    serviceTcp->txData(QByteArray("FALLITO COMANDO!\n"));
                }
            }

        }else if(data.contains("stepVal")){
            parametri = getNextFieldsAfterTag(data, QString("stepVal"));
            if(parametri.size()!=1) serviceTcp->txData(QByteArray("PARAMETRI ERRATI!\n"));
            if(pBiopsy->setStepVal((unsigned char) parametri[0].toInt())==false){
                serviceTcp->txData(QByteArray("FALLITO COMANDO!\n"));
            }

        }else if(data.contains("setAgo")){
            parametri = getNextFieldsAfterTag(data, QString("setAgo"));
            if(parametri.size()!=1) serviceTcp->txData(QByteArray("PARAMETRI ERRATI!\n"));
            if(pBiopsy->setLunghezzaAgo((unsigned char) parametri[0].toInt())==false){
                serviceTcp->txData(QByteArray("FALLITO COMANDO!\n"));
            }

        }else if(data.contains("getAgo")){
            serviceTcp->txData(QString("Ago Lenght: %1\n\r").arg(pBiopsy->lunghezzaAgo).toAscii().data());
        }else if(data.contains("getStepval")){
            serviceTcp->txData(QString("current stepval: %1\n\r").arg(pBiopsy->stepVal).toAscii().data());
        }else if(data.contains("getMaxZ")){
            serviceTcp->txData(QString("current max Z (for compressor): %1\n\r").arg(pBiopsy->maxZ_mm).toAscii().data());
        }else if(data.contains("getZlim")){
            serviceTcp->txData(QString("current Zlimit (compressor and needle): %1\n\r").arg(pBiopsy->zlim_mm).toAscii().data());
        }else if(data.contains("getAdapter")){
            serviceTcp->txData(QString("current adapter: %1\n\r").arg(pBiopsy->adapterId).toAscii().data());
        }else if(data.contains("getRevision")){
            serviceTcp->txData(QString("current adapter: %1\n\r").arg(pBiopsy->revisione).toAscii().data());
        }
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void serverDebug::handleBiopsySimulator(QByteArray data)
{
#ifdef __BIOPSY_SIMULATOR

    QString stringa;
    unsigned char buffer[20];

    if(data.contains("simConn")){
        QList<QByteArray> parametri = getNextFieldsAfterTag(data, QString("simConn"));
        if(parametri.size() != 1) {
            stringa = QString("PARAMETRO: ON/OFF \r\n");
            serviceTcp->txData(stringa.toAscii());
            return;
        }


        if(parametri[0]=="ON") buffer[1] = 1;
        else buffer[1] = 0;
        buffer[0] = 1; // Comando impostazione stato connessione
        pConsole->pGuiMcc->sendFrame(MCC_BIOPSY_SIMULATOR,1,buffer,2);
        serviceTcp->txData(QByteArray("DONE \r\n"));
        return;

    }else if(data.contains("simSblocco")){
        QList<QByteArray> parametri = getNextFieldsAfterTag(data, QString("simSblocco"));
        if(parametri.size() != 1) {
            stringa = QString("PARAMETRO: ON/OFF \r\n");
            serviceTcp->txData(stringa.toAscii());
            return;
        }

        if(parametri[0]=="ON") buffer[1] = 1;
        else buffer[1] = 0;
        buffer[0] = 2; // Comando impostazione stato pulsante
        pConsole->pGuiMcc->sendFrame(MCC_BIOPSY_SIMULATOR,1,buffer,2);
        serviceTcp->txData(QByteArray("DONE \r\n"));
        return;

    }else if(data.contains("simAdapter")){
        QList<QByteArray> parametri = getNextFieldsAfterTag(data, QString("simAdapter"));
        if(parametri.size() != 1) {
            stringa = QString("PARAMETRO: num adapter \r\n");
            serviceTcp->txData(stringa.toAscii());
            return;
        }

        buffer[1] = parametri[0].toInt();
        buffer[0] = 3; // Comando impostazione adattatore
        pConsole->pGuiMcc->sendFrame(MCC_BIOPSY_SIMULATOR,1,buffer,2);
        serviceTcp->txData(QByteArray("DONE \r\n"));
        return;

    }else if(data.contains("simRST")){
        buffer[0] = 4; // Comando impostazione pulsanti console
        buffer[1] = _BP_BIOP_PUSH_RESET;
        pConsole->pGuiMcc->sendFrame(MCC_BIOPSY_SIMULATOR,1,buffer,2);
        serviceTcp->txData(QByteArray("DONE \r\n"));
        return;

    }else if(data.contains("simENT")){
        buffer[0] = 4; // Comando impostazione pulsanti console
        buffer[1] = _BP_BIOP_PUSH_SEQ;
        pConsole->pGuiMcc->sendFrame(MCC_BIOPSY_SIMULATOR,1,buffer,2);
        serviceTcp->txData(QByteArray("DONE \r\n"));
        return;

    }else if(data.contains("simp_1")){
        buffer[0] = 4; // Comando impostazione pulsanti console
        buffer[1] = _BP_BIOP_PUSH_AGO_1;
        pConsole->pGuiMcc->sendFrame(MCC_BIOPSY_SIMULATOR,1,buffer,2);
        serviceTcp->txData(QByteArray("DONE \r\n"));
        return;

    }else if(data.contains("simp10")){
        buffer[0] = 4; // Comando impostazione pulsanti console
        buffer[1] = _BP_BIOP_PUSH_AGO_10;
        pConsole->pGuiMcc->sendFrame(MCC_BIOPSY_SIMULATOR,1,buffer,2);
        serviceTcp->txData(QByteArray("DONE \r\n"));
        return;

    }else if(data.contains("simBK")){
        buffer[0] = 4; // Comando impostazione pulsanti console
        buffer[1] = _BP_BIOP_PUSH_BACK;
        pConsole->pGuiMcc->sendFrame(MCC_BIOPSY_SIMULATOR,1,buffer,2);
        serviceTcp->txData(QByteArray("DONE \r\n"));
        return;

    }else if(data.contains("simXY")){

        QList<QByteArray> parametri = getNextFieldsAfterTag(data, QString("simXY"));
        if(parametri.size() != 2) {
            stringa = QString("PARAMETRO: num adapter \r\n");
            serviceTcp->txData(stringa.toAscii());
            return;
        }

        buffer[0] = 5; // Comando impostazione XY
        buffer[1] = parametri[0].toInt() & 0xFF;
        buffer[2] = parametri[0].toInt() / 256 ;
        buffer[3] = parametri[1].toInt() & 0xFF;
        buffer[4] = parametri[1].toInt() / 256 ;


        pConsole->pGuiMcc->sendFrame(MCC_BIOPSY_SIMULATOR,1,buffer,5);
        serviceTcp->txData(QByteArray("DONE \r\n"));
        return;

    }

#endif
}
