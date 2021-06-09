#include "application.h"
#include "appinclude.h"
#include "globvar.h"
#include "DOSE.h"

#include "systemlog.h"
extern systemLog* pSysLog;


#include "ANALOG/Calibration/pageCalibAnalogic.h"
extern AnalogCalibPageOpen* paginaCalibAnalogic;

// NOTA: SetFocus attiva il Fuoco
void console::activateConnections(void){
    // Creazione del socket di comunicazione esterna con la Console
    consoleSocketTcp = new TcpIpServer();
    if(consoleSocketTcp->Start(_CONSOLE_IN_PORT)<0)
    {
        qDebug() << "IMPOSSIBILE APIRE LA PORTA DI COMUNICAZIONE CON LA CONSOLE!!";
        return;
    }

    QObject::connect(consoleSocketTcp,SIGNAL(rxData(QByteArray)),this,SLOT(consoleRxHandler(QByteArray)),Qt::UniqueConnection);
    QObject::connect(consoleSocketTcp,SIGNAL(serverConnection(bool)),this,SLOT(consoleConnectionHandler(bool)),Qt::UniqueConnection);
    QObject::connect(this,SIGNAL(consoleTxHandler(QByteArray)), consoleSocketTcp,SLOT(txData(QByteArray)),Qt::UniqueConnection);

    //QObject::connect(this,SIGNAL(raggiStdNotify(QByteArray)),this,SLOT(raggiStdReceptionNotify(QByteArray)),Qt::UniqueConnection);
    connect(this,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(guiNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
    connect(this,SIGNAL(raggiDataSgn(QByteArray)),this,SLOT(rxDataLog(QByteArray)),Qt::UniqueConnection);

}

console::console(QObject *parent) :
    QObject(parent)
{

    // Creazione comunicazione con core M4 MASTER
    pGuiMcc = new mccCom(_DEF_MCC_GUI_TO_M4_MASTER,FALSE);  // Comunicazione Comandi  Per M4 MASTER
    pMasterRxMcc = new mccMasterCom(_DEF_MCC_MASTER_TO_APP_MASTER,TRUE);// Comunicazione Comandi  Da M4 MASTER


    // Init parametri
    currentMas=40*50;

    // Alla partenza di default la modalit√  di calibrazione √® disabilitata
    xSequence.workingMode=_EXPOSURE_MODE_OPERATING_MODE;
    ApplicationDatabase.setData(_DB_EXPOSURE_MODE,(unsigned char) xSequence.workingMode);

    openStudy = FALSE;
    xSequence.disable_check_compression = false;


}



void console::consoleConnectionHandler(bool stat)
{
    consoleConnected = stat;



    return;
}



///////////////////////////////////////////////////////////////////
/*
void console::consoleRxHandler(QByteArray data)

    This handler function is connected to the Socket RX.
    This is the Console protocol entry point.
    Follows a Frame protocol description


    "<ID LEN ... %Corpo del messaggio% ..>"

    ID = progressive ID number of the message;
    LEN = lenght of the whole message

 Data: 15/10/2014
 Autore: M.Rispoli

*/
///////////////////////////////////////////////////////////////////
void console::consoleRxHandler(QByteArray rxbuffer)
{
    int code;


    // Trasforma in Unicode il buffer ricevuto
#if (UNICODE_FORMAT == 0)
    protoConsole protocollo(&rxbuffer);
#else
    QTextCodec *codec = QTextCodec::codecForName(UNICODE_TYPE);
    QString frame = codec->toUnicode(rxbuffer);
    protoConsole protocollo(&frame);
#endif

    // Segnale per eventuali debug
    emit consoleRxSgn(rxbuffer);

    if(protocollo.isValid==FALSE)
    {
        protoConsole answ(protocollo.id,UNICODE_FORMAT);
        emit consoleTxHandler(answ.answToQByteArray("NOK FORMATO"));
        return;
    }

    protoConsole answ(protocollo.id,UNICODE_FORMAT);
    QString comando = protocollo.comando;

    // Se lo startup non Ë completato, nessun comando puÚ essere eseguito
    if(!pConfig->startupCompleted){
        emit consoleTxHandler(answ.answToQByteArray("NOK GANTRY_STARTUP_NOT_COMPLETED"));
        return;
    }

    if(comando==GET_PUSH){
        unsigned char prx = ApplicationDatabase.getDataU(_DB_XRAY_PUSH_BUTTON);
        if(prx)  emit consoleTxHandler( answ.cmdToQByteArray("OK 1"));
        else emit consoleTxHandler( answ.cmdToQByteArray("OK 0"));
    }else if(comando==GET_IA_RX)
    {
        if(pGeneratore->validated==FALSE) emit consoleTxHandler(answ.answToQByteArray("NOK 0"));
        else
        {
            answ.addParam(QString("%1").arg(pGeneratore->selectedIn));
            emit consoleTxHandler(answ.answToQByteArray("OK"));
            return;
        }

    }else if(comando == SET_XRAY_SYM)
    {
        if(protocollo.parametri.count()!=1) return;
        emit consoleTxHandler( answ.answToQByteArray(_IMMEDIATO));
        if(protocollo.parametri[0]=="ON") handleSetXraySym(true);
        else handleSetXraySym(false);
        return;

    }else if(comando==SET_SERIAL_NUMBER){
        if(protocollo.parametri.count()!=1) emit consoleTxHandler( answ.cmdToQByteArray("NOK 1"));
        else if(pConfig->SN_Configured ) emit consoleTxHandler( answ.cmdToQByteArray("NOK 2"));
        else{
            pConfig->userCnf.SN = protocollo.parametri[0];
            emit consoleTxHandler( answ.answToQByteArray(_IMMEDIATO));
            pConfig->saveUserCfg();
            pConfig->SN_Configured = true;
            pConfig->testConfigError(true,false);
        }
        return;

    }else if(comando==SET_SERVICE_PSW){
        if(protocollo.parametri.count()!=1) emit consoleTxHandler( answ.cmdToQByteArray("NOK 1"));
        else{
            pConfig->userCnf.ServicePassword = protocollo.parametri[0];
            emit consoleTxHandler( answ.answToQByteArray(_IMMEDIATO));
            pConfig->saveUserCfg();
        }
        return;

    }else if(comando==GET_SERIAL_NUMBER){
        if(!pConfig->SN_Configured) emit consoleTxHandler( answ.cmdToQByteArray("NOK 1"));
        else {
            answ.addParam(QString("%1").arg(pConfig->userCnf.SN));
            emit consoleTxHandler( answ.cmdToQByteArray("OK"));
        }
        return;

    }else if(comando == SET_CUR_DATE)
    {
        if(protocollo.parametri.count()!=2) return;

        // La funzione interna usa una system, quindi occorre attendere la trasmisiione della risposta..
        consoleSocketTcp->txData(answ.answToQByteArray(_IMMEDIATO),100000);
        // emit consoleTxHandler( answ.answToQByteArray(_IMMEDIATO));
        handleSetDate(QString(protocollo.parametri[0]),QString(protocollo.parametri[1]));
        return;
    }else if(comando==RESET)
    {
        emit consoleTxHandler( answ.answToQByteArray(_IMMEDIATO));
        //handleSetKv(param);
        return;
    }else if(comando==SET_READY)
    {
        emit consoleTxHandler(answ.answToQByteArray("OK 0"));
        //handleSetKv(param);
        return;
    }else if(comando==GET_POTTER)
    {
        if(pPotter->getPotId()==POTTER_2D)  emit consoleTxHandler(answ.answToQByteArray("2D 0"));
        else if(pPotter->getPotId()==POTTER_MAGNIFIER)  emit consoleTxHandler(answ.answToQByteArray(QString("MG %1").arg(pCompressore->config.fattoreIngranditore[pPotter->getPotFactor()])));
        else  emit consoleTxHandler(answ.answToQByteArray("ND 0"));
        return;
    }
    else if(comando==SET_KV)
    {
        if(protocollo.parametri.count()!=1) return;
        emit consoleTxHandler( answ.answToQByteArray(_IMMEDIATO));
        handleSetKv(QString(protocollo.parametri[0]));
        return;
    }
    else if(comando==SET_MAS)
    {
        if(protocollo.parametri.count()!=1) return;
        emit consoleTxHandler( answ.answToQByteArray(_IMMEDIATO));
        handleSetMas(QString(protocollo.parametri[0]));
        return;
    }else if(comando==SET_MODE)
    {
        int ris = handleSetMode(&protocollo);
        answ.addParam(QString("%1").arg(ris));
        if(ris) emit consoleTxHandler( answ.cmdToQByteArray("NOK"));
        else emit consoleTxHandler( answ.cmdToQByteArray("OK"));
        return;
    }else if(comando==SET_CALIB_MODE)
    {
        // Verifica tipologia di calibrazione
        if(protocollo.parametri.count()==0) return;


        if(protocollo.parametri[0]=="KV"){
            ApplicationDatabase.setData(_DB_EXPOSURE_MODE,(unsigned char) _EXPOSURE_MODE_CALIB_MODE_KV);
        }else if(protocollo.parametri[0]=="IA"){
            ApplicationDatabase.setData(_DB_EXPOSURE_MODE,(unsigned char) _EXPOSURE_MODE_CALIB_MODE_IA);
        }else if(protocollo.parametri[0]=="KERMA"){
            ApplicationDatabase.setData(_DB_EXPOSURE_MODE,(unsigned char) _EXPOSURE_MODE_CALIB_MODE_KERMA);
        }else if(protocollo.parametri[0]=="DETECTOR"){
            if(protocollo.parametri.count()!=3) {
                emit consoleTxHandler( answ.answToQByteArray("NOK 1"));
                return;
            }

            // Aggiunge i dati di accettabilit‡
            paginaCalibAnalogic->rmmi_reference = protocollo.parametri[1].toInt();
            paginaCalibAnalogic->rmmi_toll = protocollo.parametri[2].toInt();
            ApplicationDatabase.setData(_DB_EXPOSURE_MODE,(unsigned char) _EXPOSURE_MODE_CALIB_MODE_EXPOSIMETER);
        }else if(protocollo.parametri[0]=="SHOT"){
            ApplicationDatabase.setData(_DB_EXPOSURE_MODE,(unsigned char) _EXPOSURE_MODE_RX_SHOT_NODET_MODE);
        }else if(protocollo.parametri[0]=="PROFILE"){
            paginaCalibAnalogic->pc_data_valid =false;
            ApplicationDatabase.setData(_DB_EXPOSURE_MODE,(unsigned char) _EXPOSURE_MODE_CALIB_MODE_PROFILE);
        }else{
            return;
        }


        emit consoleTxHandler( answ.answToQByteArray(_IMMEDIATO));

        ApplicationDatabase.setData(_DB_STUDY_STAT,(unsigned char) _OPEN_STUDY_ANALOG);

        pConfig->selectOperatingPage();
        return;


    }else if(comando==SET_OPER_MODE)
    {
        if(protocollo.parametri.count()!=0) return;
        emit consoleTxHandler( answ.answToQByteArray(_IMMEDIATO));
        handleSetOperatingMode();
        return;
    }else if(comando==SET_TUBE)
    {
        // Per i movimenti manuali il comando non deve avere effetti ulteriori
        if(!pConfig->sys.armMotor) {
            emit consoleTxHandler( answ.answToQByteArray(_IMMEDIATO));
            return;
        }

        if(protocollo.parametri.count()!=1) emit consoleTxHandler( answ.answToQByteArray("NOK 1"));
        else if(handleSetTube(QString(protocollo.parametri[0]),protocollo.id)==FALSE) emit consoleTxHandler( answ.answToQByteArray("NOK 2"));
        else {
            emit consoleTxHandler(answ.answToQByteArray("OK 255"));
            pToConsole->notifyMovingArm(); // Avvisa la AWS che sta per muovere
        }
        return;
    }else if(comando==SET_STOP_MOVE)
    {
        handleSetStopMove();
        emit consoleTxHandler(answ.answToQByteArray("OK 0"));
        return;
    }else if(comando==SET_ARM)
    {
        if(protocollo.parametri.count()<1) return;
        int angolo = (int) protocollo.parametri[0].toFloat();
        int min=angolo-5;
        int max=angolo+5;

        if(protocollo.parametri.count()>1) min = (int) protocollo.parametri[1].toFloat();
        if(protocollo.parametri.count()>2) max = (int) protocollo.parametri[2].toFloat();
        handleSetArm(angolo,min,max,protocollo.id);

        //pToConsole->notifyMovingArm(); // Avvisa la AWS che sta per muovere
        return;
    }else if(comando==GET_ARM)
    {
        emit consoleTxHandler( answ.answToQByteArray(QString("OK %1").arg(pConfig->convertDangolo(ApplicationDatabase.getDataI(_DB_DANGOLO)))));
    }else if(comando==GET_TRX)
    {
        emit consoleTxHandler( answ.answToQByteArray(QString("OK %1").arg(pConfig->convertDangolo(ApplicationDatabase.getDataI(_DB_TRX)))));
    }else if(comando==SET_DEAD_MAN)
    {
        if(protocollo.parametri.count()!=1) {
            answ.answToQByteArray(QString("NOK 1"));
            return;
        }
        emit consoleTxHandler( answ.answToQByteArray(QString("OK 0")));
        if(protocollo.parametri[0].toInt() == 1) handleSetDeadMan(true);
        else handleSetDeadMan(false);
        return;
    }else if(comando==SET_PUSH_ENA)
    {
        if(protocollo.parametri.count()!=1){
            emit consoleTxHandler( answ.answToQByteArray(QString("NOK 1")));
            return;
        }
        if(handleSetPushEna(QString(protocollo.parametri[0]))==FALSE) emit consoleTxHandler( answ.answToQByteArray(QString("NOK 2")));
        else emit consoleTxHandler( answ.answToQByteArray(_IMMEDIATO));
        return;
    }else if(comando==OPEN_STUDYL)
    {
        if(!pConfig->SN_Configured){
            emit consoleTxHandler( answ.answToQByteArray(QString("NOK 1")));
            return;
        }
        emit consoleTxHandler( answ.answToQByteArray(_IMMEDIATO));
        handleOpenStudy(TRUE, &protocollo);
        return;
    }
    else if(comando==OPEN_STUDYW)
    {
        if(!pConfig->SN_Configured){
            emit consoleTxHandler( answ.answToQByteArray(QString("NOK 1")));
            return;
        }
        emit consoleTxHandler( answ.answToQByteArray(_IMMEDIATO));
        handleOpenStudy(FALSE, &protocollo);
        return;
    }else if(comando==CLOSE_STUDY)
    {
        emit consoleTxHandler( answ.answToQByteArray(_IMMEDIATO));
        handleCloseStudy();
        return;
    } else if(comando==GET_THICK)
    {
        answ.addParam(QString("%1").arg((unsigned short) pCompressore->breastThick));
        emit consoleTxHandler(answ.answToQByteArray());
        return;
    }
    else if(comando==GET_FORCE)
    {
        answ.addParam(QString("%1").arg((unsigned char) pCompressore->comprStrenght));
        emit consoleTxHandler( answ.answToQByteArray());
        return;
    }
    else if(comando==GET_COMPRESSOR)
    {
        // <id size %OK pad%>
        if(pCompressore->getPad()>=PAD_ENUM_SIZE) emit consoleTxHandler( answ.cmdToQByteArray(QString("OK %1").arg((int)PAD_ND)));
        else  emit consoleTxHandler( answ.cmdToQByteArray(QString("OK %1").arg((int)pCompressore->getPad())));
        return;
    }else if(comando==GET_TROLLEY)
    {   // Richiede la posizione del compressore. Richiesta asincrona
        emit consoleTxHandler(answ.cmdToQByteArray("OK 255"));

        // Invio comando
        unsigned char data=0;
        if(pConsole->pGuiMcc->sendFrame(MCC_GET_TROLLEY,protocollo.id,&data, 1)==FALSE)
        {
            qDebug() << "CONSOLE <GetTrolley>: ERRORE COMANDO MCC";
            PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_SOFT,ERROR_MCC,TRUE); // Self resetting
        }

        return;
    }else if(comando==SET_EXPOSE_NO_COMPRESSION)
    { // Attiva la modalit‡ di esposizione senza compressione

        if(protocollo.parametri.count()!=1) {
            emit consoleTxHandler(answ.cmdToQByteArray("NOK 1"));
            return;
        }

        if(protocollo.parametri[0].toInt()==1)
            xSequence.disable_check_compression = true;
        else
            xSequence.disable_check_compression = false;
        emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
        return;

    }else if(comando==SET_COMPRESSOR_RELEASE)
    { // Imposta la modalit‡ di rilascio compressione dopo esposizione
        if(protocollo.parametri.count()!=1) {
            emit consoleTxHandler(answ.cmdToQByteArray("NOK 1"));
            return;
        }
        emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));

        bool value;
        if(protocollo.parametri[0].toInt()==1) value=true;
        else value=false;

        if(value==pConfig->userCnf.enableSblocco) return;
        pConfig->userCnf.enableSblocco = value;
        if(value) ApplicationDatabase.setData(_DB_COMPRESSOR_UNLOCK,(unsigned char) 1);
        else  ApplicationDatabase.setData(_DB_COMPRESSOR_UNLOCK,(unsigned char) 0);

        return;


    }else if(comando==SET_XRAY_LAMP)
    {
        if(protocollo.parametri.count()!=1) return;
        emit consoleTxHandler( answ.answToQByteArray(_IMMEDIATO));
        handleSetXrayLamp(QString(protocollo.parametri[0]));
        return;
    }else if(comando==SET_STARTER)
    {
        if(handleSetStarter(&protocollo)==TRUE) emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
        else emit consoleTxHandler(answ.cmdToQByteArray("NOK"));
    }else if(comando==GET_PACKAGE)
    {
        handleGetSoftwareRevisions(&answ);
    }else if(comando==GET_TUBES)
    {
        handleGetTubes(&answ);
    }else if(comando==SELECT_TUBE)
    {
        // Il comando richiede un parametro
        if(protocollo.parametri.size()!=1) emit consoleTxHandler(answ.cmdToQByteArray("NOK"));
        else handleSelectTube(protocollo.parametri.at(0), &answ);
    }else if(comando==STORE_TUBE_CONFIG_DATA)
    {
        // Il comando richiede un parametro
        if(protocollo.parametri.size()!=1) emit consoleTxHandler(answ.cmdToQByteArray("NOK 1"));
        else if(protocollo.parametri.size()==1) handleStoreTube(protocollo.parametri.at(0), &answ); // Nuovo file

    }else if(comando==GET_KV_INFO)
    {
        handleGetKvInfo(&answ); // Stesso file
    }else if(comando==SET_KV_VDAC_DATA)
    {
        if(protocollo.parametri.size()!=30) emit consoleTxHandler(answ.cmdToQByteArray("NOK"));
        else handleSetKvVdacData(&protocollo, &answ);
    }else if(comando==SET_CALIB_KV_READ)
    {
        if(protocollo.parametri.size()!=2) emit consoleTxHandler(answ.cmdToQByteArray("NOK"));
        else handleSetCalibKvRead(&protocollo, &answ);
    }else if(comando==SET_KV_MONITOR_DATA)
    {
        if(protocollo.parametri.size()!=30) emit consoleTxHandler(answ.cmdToQByteArray("NOK"));
        else handleSetKvMonitorData(&protocollo, &answ);
    }else if(comando==GET_IA_INFO)
    {
        handleGetIaInfo(&protocollo, &answ);

    }else if(comando==GET_IA_TOMO_INFO)
    {
        handleGetIaTomoInfo(&answ);

    }else if(comando==SET_IDAC_DATA)
    {

        // Prima verifica l'esattezza dei dati e poi eventualmente effettua le modifiche
        if(handleSetIdacData(&protocollo, FALSE)==FALSE) emit consoleTxHandler(answ.cmdToQByteArray("NOK 1"));
        else
        {
            handleSetIdacData(&protocollo, TRUE);
            emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
        }

    }else if(comando==SET_ANALOG_FOCUS)
    {  // Forza l'impostazione del fuoco corrente
        if(protocollo.parametri.count()!=1) {
            emit consoleTxHandler(answ.cmdToQByteArray("NOK 1"));
            return;
        }

        if(protocollo.parametri[0]=="S"){
            pGeneratore->setFuoco((Generatore::_FuocoSize_Enum) Generatore::FUOCO_SMALL);
        }else{
            pGeneratore->setFuoco((Generatore::_FuocoSize_Enum) Generatore::FUOCO_LARGE);
        }
        pGeneratore->updateFuoco();
        emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
        return;

    }else if(comando==GET_TUBE_TEMP)
    {
        emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
        pToConsole->notifyTubeTemp(ApplicationDatabase.getDataI(_DB_HU_ANODE), ApplicationDatabase.getDataI(_DB_T_CUFFIA));
        return;

    } else if(comando==SET_IA_RX_DATA)
    {
        if(handleSetIaRxData(&protocollo, &answ)==FALSE) emit consoleTxHandler(answ.cmdToQByteArray("NOK"));
        else emit consoleTxHandler(answ.cmdToQByteArray("OK"));
    }else if(comando==GET_BIOPSY_Z)
    {
        //handleGetBiopsyZ(&answ);
    }else if(comando==SET_BIOPSY_HOME)
    {
        code = handleSetBiopsyHome(&protocollo);
        if(code>0)
        {
            answ.addParam(QString("%1").arg((int) code));
            emit consoleTxHandler(answ.cmdToQByteArray("NOK"));
        }else
        {
            emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
        }
    }else if(comando==SET_LINGUA)
    {
        code = handleSetLingua(&protocollo);
        if(code==0) emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
        else
        {
            answ.addParam(QString("%1").arg(code));
            emit consoleTxHandler(answ.cmdToQByteArray("NOK"));
        }
    }else if(comando==GET_HU)
    {

        int temp = ApplicationDatabase.getDataI(_DB_T_CUFFIA) & 0x00FF;
        long hu;

        // Percentuale hu anodici consentiti rispetto al massimo fissato in 300kHU
        if((protocollo.parametri.size()==1)&&(protocollo.parametri[0]=="P")){
            hu = (long) pGeneratore->getCurrentHU()*100/300;
            // hu = ((long) ((temp-25) * 100 / 40)); // Percentuale
            if(hu<0) hu = 0;

        }else if((protocollo.parametri.size()==1)&&(protocollo.parametri[0]=="G"))
            hu = temp; // Gradi
        else{
            // Hu anodici in HU units
            hu = (long) pGeneratore->getCurrentHU()*1000;
            //hu = ((long) (temp-25)) * 12375;
            if(hu<0) hu=0;
        }

        answ.addParam(QString("%1").arg((long) hu));
        emit consoleTxHandler(answ.cmdToQByteArray("OK"));
    }else if(comando==SET_FIRMWARE_UPDATE){
        handleSetFirmwareUpdate(&protocollo, &answ);
        return;

    }else if(comando == SET_SYS_BACKUP){
        handleSetSystemBackup(&protocollo, &answ);
        return;
    }else if(comando == SET_SYS_RESTORE){
        handleSetSystemRestore(&protocollo, &answ);
        return;
    }else if(comando == SET_USER_DIR_CLEAN){
        handleSetUserDirClean(&protocollo, &answ);
        return;
    }else if (comando == SET_AUTO_COMPRESSION){
        emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
        handleCompressorActivation(1);
    }else if (comando == SET_UNLOCK_COMPRESSION){
        emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
        handleCompressorActivation(0);
    }else if (comando == SET_OUTPUT_PULSE){
        if(protocollo.parametri.size()!=2) emit consoleTxHandler(answ.cmdToQByteArray("NOK 1"));
        else emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
        handleOutputPulse(protocollo.parametri[0],protocollo.parametri[1]);
    }else if (comando == SET_DEMO_MODE){
        if(protocollo.parametri.size()!=1) emit consoleTxHandler(answ.cmdToQByteArray("NOK 1"));
        bool demo;
        if(protocollo.parametri[0].toInt() == 1) demo = true;
        else demo = false;

        // In caso lo stato non corrisponda a quello atteso comunica che deve fare il cambio di stato
        if(pConfig->setDemoMode(demo))  emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
        else emit consoleTxHandler(answ.cmdToQByteArray("OK 1"));

    }//////// COMANDI COLLIMAZIONE /////////////////////////////////////////////////////////////////////////////////////////
    else if(comando==SET_COLLI_FILTRO)
    {
        if(handleSetColliFilter(&protocollo)==TRUE)  emit consoleTxHandler( answ.cmdToQByteArray("OK 0"));
        else emit consoleTxHandler( answ.cmdToQByteArray("NOK"));
    }else if(comando==SET_COLLI_MIRROR)
    {
        if(handleSetColliMirror(&protocollo)==TRUE)  emit consoleTxHandler( answ.cmdToQByteArray("OK 0"));
        else emit consoleTxHandler( answ.cmdToQByteArray("NOK"));
    }else if(comando==GET_COLLI_FILTRO)
    {
        handleGetColliFilter(&protocollo,&answ);
    }else if(comando==GET_COLLI_MIRROR)
    {
        handleGetColliMirror(&protocollo,&answ);
    }else if(comando==SET_COLLI_STORE)
    {
        // La risposta negativa indica che la periferica non deve essere aggiornata e quindi
        // l'operazione √® istantanea.
        if(handleSetColliStore()){
            idColliStore = protocollo.id;
            emit consoleTxHandler( answ.cmdToQByteArray("OK 255"));
        }
        else emit consoleTxHandler( answ.cmdToQByteArray("OK 0"));
    }else if(comando==SET_COLLI_MODE)
    {
        if(handleSetColliMode(&protocollo)) emit consoleTxHandler( answ.cmdToQByteArray("OK 0"));
        else emit consoleTxHandler( answ.cmdToQByteArray("NOK 0"));
    }else if(comando==SET_MANUAL_COLLI)
    {
        if(handleSetManualColli(&protocollo)) emit consoleTxHandler( answ.cmdToQByteArray("OK 0"));
        else emit consoleTxHandler( answ.cmdToQByteArray("NOK 0"));

    }else if(comando==SET_COLLI_2D)
    {
        if(handleSetColli2D(&protocollo)) emit consoleTxHandler( answ.cmdToQByteArray("OK 0"));
        else emit consoleTxHandler( answ.cmdToQByteArray("NOK 0"));
    }else if(comando==GET_COLLI_2D)
    {
        handleGetColli2D(&protocollo, &answ);
    }else if(comando==SET_TEST_CMD){
        unsigned char data;
        pGuiMcc->sendFrame(MCC_TEST,1,&data,0);
        emit consoleTxHandler( answ.cmdToQByteArray("OK 0"));
    }else if(comando==RESET_ALARMS){
        paginaAllarmi->resetOneShotAlarms();
        emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
        return;
    }else if(comando==GET_TUBE_STATISTICS){
        answ.addParam(pConfig->userCnf.tubeFileName);
        answ.addParam(QString("%1").arg(pGeneratore->numShots));
        answ.addParam(QString("%1").arg((float)pGeneratore->cumulatedJ * 1.33 / 1000));
        answ.addParam(QString("%1").arg(pGeneratore->nTomo));
        answ.addParam(QString("%1").arg(pGeneratore->nStandard));
        emit consoleTxHandler(answ.cmdToQByteArray("OK"));
        return;
    }else if(comando==GET_GEN_CONF){
        if(pConfig->sys.highSpeedStarter)
            emit consoleTxHandler( answ.cmdToQByteArray(QString("OK %1 HS").arg(pGeneratore->max_selectable_kv)));
        else
            emit consoleTxHandler( answ.cmdToQByteArray(QString("OK %1 LS").arg(pGeneratore->max_selectable_kv)));
    }else if(comando==SET_POWER_OFF){
        emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
        handlePowerOff();

    }else if(comando==SET_REBOOT){
        emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
        pConfig->executeReboot();
    }else if(comando==SET_IMAGE){
        if(protocollo.parametri.size()==0) {
            emit consoleTxHandler(answ.cmdToQByteArray("NOK 1"));
            return;
        }

        if(protocollo.parametri.size()==1) {
            ApplicationDatabase.setData(_DB_IMAGE_NAME,QString(""),0);
            emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
            return;
        }


        if(protocollo.parametri[1].toInt()==0) {
            ApplicationDatabase.setData(_DB_IMAGE_NAME,QString(""),0);
        }else{
            ApplicationDatabase.setData(_DB_IMAGE_NAME,protocollo.parametri[0],0);
        }

        emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
    }else if(comando==SET_PROIEZIONI){
        if(protocollo.parametri.size()==0){
            emit consoleTxHandler(answ.cmdToQByteArray("NOK 1"));
            return; // Comando non valido
        }
        handleSetProiezioni(&protocollo);
        emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));

    }else if(comando==SEL_PROIEZIONE){
        if(protocollo.parametri.size()==0)  {
            emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
            ApplicationDatabase.setData(_DB_SEL_PROJ, "",0);
            return;
        }

        if(protocollo.parametri.size()==1)  {
            // Aggiorna la selezione corrente
            ApplicationDatabase.setData(_DB_SEL_PROJ, protocollo.parametri[0],0);
            emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
            return;
        }

        emit consoleTxHandler(answ.cmdToQByteArray("NOK 1"));
        return;

    }else if(comando==ABORT_PROJECTION){
        // Abort della selezione corrente
        emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
        ApplicationDatabase.setData(_DB_SEL_PROJ,"",0);
        return;

    }else if(comando==GET_TOMO_CONFIG){
        // SKIP - INTERLEAVE - N-SAMPLES
        if(protocollo.parametri.size()!=1)  {
            emit consoleTxHandler(answ.cmdToQByteArray("NOK 1"));
        }else{
            if(protocollo.parametri[0]=="W"){
                answ.addParam(QString("%1").arg(pConfig->trxConfig.tomo.w.pre_samples));
                answ.addParam(QString("%1").arg(pConfig->trxConfig.tomo.w.skip_samples));
                answ.addParam(QString("%1").arg(pConfig->trxConfig.tomo.w.samples));
                emit consoleTxHandler(answ.cmdToQByteArray("OK"));
            }else if(protocollo.parametri[0]=="I"){
                answ.addParam(QString("%1").arg(pConfig->trxConfig.tomo.i.pre_samples));
                answ.addParam(QString("%1").arg(pConfig->trxConfig.tomo.i.skip_samples));
                answ.addParam(QString("%1").arg(pConfig->trxConfig.tomo.i.samples));
                emit consoleTxHandler(answ.cmdToQByteArray("OK"));
            }else if(protocollo.parametri[0]=="N"){
                answ.addParam(QString("%1").arg(pConfig->trxConfig.tomo.n.pre_samples));
                answ.addParam(QString("%1").arg(pConfig->trxConfig.tomo.n.skip_samples));
                answ.addParam(QString("%1").arg(pConfig->trxConfig.tomo.n.samples));
                emit consoleTxHandler(answ.cmdToQByteArray("OK"));
            }
        }

        return;

    }else if(comando==SET_AEC_FIELD){
        // Impostazione campo Detector Analogico ma solo se calibrazione detector
        if(ApplicationDatabase.getDataU(_DB_EXPOSURE_MODE)!=_EXPOSURE_MODE_CALIB_MODE_EXPOSIMETER){
            emit consoleTxHandler(answ.cmdToQByteArray("NOK 2"));
            return;
        }
        if(protocollo.parametri.size()!=1){
            emit consoleTxHandler(answ.cmdToQByteArray("NOK 1"));
            return;
        }
        if(protocollo.parametri[0].toInt()>2){
            emit consoleTxHandler(answ.cmdToQByteArray("NOK 3"));
            return;
        }

        paginaCalibAnalogic->setAecField(protocollo.parametri[0].toInt());
        emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
        return;

    }else if(comando==SET_LOGO){
        // Non pi˘ usata
        emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));

    }else if(comando==SET_LAT){
        // Non pi˘ usata
        emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));

    }else if(comando==SET_CALIB_FIELD){
        if(protocollo.parametri.size()!=4){
            emit consoleTxHandler(answ.cmdToQByteArray("NOK 1"));
            return;
        }

        pConfig->analogCnf.calib_f1 = protocollo.parametri[0].toInt();
        pConfig->analogCnf.calib_f2 = protocollo.parametri[1].toInt();
        pConfig->analogCnf.calib_f3 = protocollo.parametri[2].toInt();
        pConfig->analogCnf.calib_margine=  protocollo.parametri[3].toInt();
        pConfig->saveAnalogConfig();
        pSysLog->log("CONFIG: ANALOG CONFIGURATION FILE");
        emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
        return;

    }else if(comando==GET_CALIB_FIELD){

        emit consoleTxHandler(answ.cmdToQByteArray(QString("OK %1 %2 %3 %4").arg(pConfig->analogCnf.calib_f1).arg(pConfig->analogCnf.calib_f2).arg(pConfig->analogCnf.calib_f3).arg(pConfig->analogCnf.calib_margine)));
        return;

    }else if(comando==SET_STORE_ANALOG_CONFIG){

        emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));
        pConfig->saveAnalogConfig();
        pSysLog->log("CONFIG: ANALOG CONFIGURATION FILE");
    }else if(comando==SET_CALIB_PROFILE_DATA){

        if(protocollo.parametri.size()!=6) {
            emit consoleTxHandler( answ.answToQByteArray("NOK 1 INVALID PARAM"));
            return;
        }

        if(ApplicationDatabase.getDataU(_DB_EXPOSURE_MODE) != _EXPOSURE_MODE_CALIB_MODE_PROFILE){
            emit consoleTxHandler( answ.answToQByteArray("NOK 2 ONLY-VALID-IN-PROFILE-CALIB-MODE"));
            return;
        }

        // Impostazione nome del profilo attraverso il nome
        paginaCalibAnalogic->pc_selected_profile_index = pGeneratore->pAECprofiles->selectProfile(protocollo.parametri[0]);
        if(paginaCalibAnalogic->pc_selected_profile_index<0){
            emit consoleTxHandler( answ.answToQByteArray("NOK 3 INVALID-PROFILE-NAME"));
            return;
        }


        if(protocollo.parametri[1].toInt() > 70){
            emit consoleTxHandler( answ.answToQByteArray("NOK 4 INVALID-PMMI"));
            return;
        }
        paginaCalibAnalogic->pc_selected_pmmi = protocollo.parametri[1].toInt();

        if(protocollo.parametri[2]=="FP") paginaCalibAnalogic->pc_selected_fuoco = Generatore::FUOCO_SMALL; // Fuoco piccolo
        else if(protocollo.parametri[2]=="FG")  paginaCalibAnalogic->pc_selected_fuoco =Generatore::FUOCO_LARGE; // Fuoco grande
        else{
            emit consoleTxHandler( answ.answToQByteArray("NOK 5 INVALID-FOCUS"));
            return;
        }

        if(protocollo.parametri[3]=="Rh") paginaCalibAnalogic->pc_selected_filtro = Collimatore::FILTRO_Rh;
        else if(protocollo.parametri[3]=="Mo") paginaCalibAnalogic->pc_selected_filtro = Collimatore::FILTRO_Mo;
        else {
            emit consoleTxHandler( answ.answToQByteArray("NOK 6 INVALID-FILTER"));
            return;
        }

        if(protocollo.parametri[4]=="FRONT") paginaCalibAnalogic->pc_selected_field = ANALOG_AECFIELD_FRONT;
        else if(protocollo.parametri[4]=="CENTER") paginaCalibAnalogic->pc_selected_field = ANALOG_AECFIELD_CENTER;
        else if(protocollo.parametri[4]=="BACK") paginaCalibAnalogic->pc_selected_field = ANALOG_AECFIELD_BACK;
        else {
            emit consoleTxHandler( answ.answToQByteArray("NOK 7 INVALID-FIELD"));
            return;
        }

        paginaCalibAnalogic->pc_selected_pulses = protocollo.parametri[5].toInt();
        if(paginaCalibAnalogic->pc_selected_pulses) emit consoleTxHandler(answ.cmdToQByteArray("OK 0 BYPASS-PULSES"));
        else emit consoleTxHandler(answ.cmdToQByteArray("OK 0"));


        paginaCalibAnalogic->setProfileData();

    }else if(comando==SET_ANALOG_KERMA_CALIB_DATA){
        handleSetAnalogKermaCalibData(&protocollo, &answ);
    }else if(comando==SET_ANALOG_KV_CALIB_TUBE_DATA){
        handleSetAnalogKvCalibTubeData(&protocollo, &answ);

    }else if(comando==SET_ANALOG_IA_CALIB_TUBE_DATA){
        handleSetAnalogIaCalibTubeData(&protocollo, &answ);

    }else if(comando==SET_ANALOG_CALIB_TUBE_OFFSET){
        handleSetAnalogCalibTubeOffset(&protocollo, &answ);

    }else if(comando==GET_PROFILE_LIST){
        handleGetProfileList(&protocollo, &answ);
    }else if(comando==GET_VALID_LIST){
        handleGetValidList(&protocollo, &answ);
    }else if(comando==SET_VALID_LIST){
        handleSetValidList(&protocollo, &answ);
    }else if(comando==GET_PROFILE){
        handleGetProfile(&protocollo, &answ);
    }else if(comando==SET_PROFILE){
        handleSetProfile(&protocollo, &answ);
    }else if(comando==GET_PROFILE_NOTE){
        handleGetProfileNote(&protocollo, &answ);
    }else if(comando==SET_PROFILE_NOTE){
        handleSetProfileNote(&protocollo, &answ);
    }else if(comando==ERASE_PROFILE){
        handleEraseProfile(&protocollo, &answ);
    }else if(comando==ERASE_ALL_PROFILES){
        handleEraseAllProfiles(&protocollo, &answ);
    }else if(comando==GET_ANALOG_AF_SETUP){
        handleGetAnalogAfSetup(&protocollo, &answ);
    }else if(comando==SET_ANALOG_STORE_KERMA){
        handleSetAnalogStoreKerma(&protocollo, &answ);
    }else if(comando==GET_ANALOG_KERMA_DATA){
        handleGetAnalogKermaData(&protocollo, &answ);
    }else if(comando==GET_ANALOG_GET_CGS){
        handleGetAnalogCgs(&protocollo, &answ);
    }else if(comando==SET_ANALOG_PARAM){
        handleSetAnalogParam(&protocollo, &answ);
    }else if(comando==GET_ANALOG_PARAM){
        handleGetAnalogParam(&protocollo, &answ);
    }else if(comando==STORE_ANALOG_PARAM){
        handleStoreAnalogParam(&protocollo, &answ);
    }else if(comando==SET_ANALOG_START_LOG){
        pSysLog->activate(true);
        emit consoleTxHandler( answ.cmdToQByteArray(QString("OK 0")));
    }else if(comando==SET_ANALOG_STOP_LOG){
        pSysLog->activate(false);
        emit consoleTxHandler( answ.cmdToQByteArray(QString("OK 0")));
    }else{
        emit consoleTxHandler( answ.cmdToQByteArray(QString("NA")));
    }



    return;
}

// ______________________________________________________________________________
// Comando della console per impostare la sequenza di MEME disponibili per
// selezionare una data proiezione durante uno studio
// PARAM[0] = N. PROIEZIONI
// PARAM[1...8] = PROIEZIONI
// Il massimo numero di proiezioni disponibili e' 8
// ______________________________________________________________________________
void console::handleSetProiezioni(protoConsole* protocollo)
{
    QString proiezioni;
    int index;

    if(protocollo->parametri.size()==0) return; // Comando non valido

    if(protocollo->parametri[0].toInt()==0) {
        ApplicationDatabase.setData(_DB_PROIEZIONI,"",0);
        return;
    }

    // Limita il numero di meme a 8
    int items = protocollo->parametri[0].toInt();
    if(items>8) items=8;

    for(index=0; index<items;index++){
        proiezioni.append(protocollo->parametri[index+1]).append(" ");
    }

    ApplicationDatabase.setData(_DB_PROIEZIONI,proiezioni,0);
    return;

}

///////////////////////////////////////////////////////////////////
/*
void console::handleSetDate(QString param)
    This function set the interface date and time.
    The command format is:

    param = "dd/mm/aaaa hh:mm:ss"
    dd = day;
    mm = mounth;
    aaaa = year;
    hh = hour;
    mm = minute;
    ss = second;

 Data: 14/10/2014
 Autore: M.Rispoli

*/
///////////////////////////////////////////////////////////////////

void console::handleSetDate(QString data, QString tempo)
{
    QString Y,M,D,h,m,s,command;


    D = data.left(2);data = data.right(data.size()-3);
    M = data.left(2);data = data.right(data.size()-3);
    Y = data;

    h = tempo.left(2);tempo = tempo.right(tempo.size()-3);
    m = tempo.left(2);tempo = tempo.right(tempo.size()-3);
    s = tempo.left(2);tempo = tempo.right(tempo.size()-3);

    echoDisplay.echoDate(D,M,Y,h,m,s,DBase::_DB_NO_OPT);

    command = QString("date -u %1%2%3%4%5.%6").arg(M).arg(D).arg(h).arg(m).arg(Y).arg(s);
    system(command.toStdString().c_str());

    command = QString("hwclock -w");
    system(command.toStdString().c_str());

    systemTimeUpdated = TRUE;

    // Con la ricezione della data Ë possibile inizializzare il dispositivo
    // per la misura della quantit‡ di calore accumulata nell'Anodo
    pGeneratore->initAnodeHU();

    return;
}

///////////////////////////////////////////////////////////////////
/*


 Data: 14/10/2014
 Autore: M.Rispoli

*/
///////////////////////////////////////////////////////////////////
void console::handleSetKv(QString param)
{
    float kv;
    kv=param.toFloat();

    // Seleziona i KV che dovranno essere utilizzati
    pGeneratore->setkV(kv);
}

///////////////////////////////////////////////////////////////////
/*
    mAs sono i mas da assegnare all'array.
    Questa funzione vale per esposizioni 2D

 Data: 14/10/2014
 Autore: M.Rispoli

*/
///////////////////////////////////////////////////////////////////
void console::handleSetMas(QString qmAs)
{
    double mAs;

    mAs = qmAs.toDouble();
    if(pGeneratore->setmAs(mAs)==FALSE) qDebug() << "CONSOLE <SetmAs>: INVALID mAs:" << qmAs;

}

///////////////////////////////////////////////////////////////////
/*mccGuiCom::


 Data: 14/10/2014
 Autore: M.Rispoli


  param: [W/Mo][Rh/Ag/Al]

*/
///////////////////////////////////////////////////////////////////
bool console::handleSetAf(QString param)
{
    // Imposta il materiale
    if(param=="WRh")
    {
        if(!pGeneratore->setFuoco(W_STR)) return FALSE;
        pCollimatore->filtroCmd = Collimatore::FILTRO_Rh;
    }
    else if(param=="WAg")
    {
        if(!pGeneratore->setFuoco(W_STR)) return FALSE;
        pCollimatore->filtroCmd = Collimatore::FILTRO_Ag;
    }
    else if(param=="WAl")
    {
        if(!pGeneratore->setFuoco(W_STR)) return FALSE;
        pCollimatore->filtroCmd = Collimatore::FILTRO_Al;
    }else if(param=="WCu")
    {
        if(!pGeneratore->setFuoco(W_STR)) return FALSE;
        pCollimatore->filtroCmd = Collimatore::FILTRO_Cu;
    }else if(param=="WMo")
    {
        if(!pGeneratore->setFuoco(W_STR)) return FALSE;
        pCollimatore->filtroCmd = Collimatore::FILTRO_Mo;
    }
    else if(param=="MoRh")
    {
        if(!pGeneratore->setFuoco(Mo_STR)) return FALSE;
        pCollimatore->filtroCmd = Collimatore::FILTRO_Rh;
    }
    else if(param=="MoAg")
    {
        if(!pGeneratore->setFuoco(Mo_STR)) return FALSE;
        pCollimatore->filtroCmd = Collimatore::FILTRO_Ag;
    }
    else if(param=="MoAl")
    {
        if(!pGeneratore->setFuoco(Mo_STR)) return FALSE;
        pCollimatore->filtroCmd = Collimatore::FILTRO_Al;
    }else if(param=="MoCu")
    {
        if(!pGeneratore->setFuoco(Mo_STR)) return FALSE;
        pCollimatore->filtroCmd = Collimatore::FILTRO_Cu;
    }else if(param=="MoMo")
    {
        if(!pGeneratore->setFuoco(Mo_STR)) return FALSE;
        pCollimatore->filtroCmd = Collimatore::FILTRO_Mo;
    }else return FALSE;

    // Impostazione del Filtro
    if(pCollimatore->setFiltro()==FALSE)
    {
        qDebug() << "CONSOLE: <handleSetAf> FALLITA!";
        return FALSE;
    }

    return TRUE;
}

bool console::handleSetFocus(QString materiale, QString dimensione)
{

    if(!pGeneratore->setFuoco(materiale))
    {
        qDebug() << "CONSOLE: <handleSetFocus> FUOCO NON VALIDO" <<pGeneratore->confF1 << pGeneratore->confF2;
        return FALSE;
    }


    // Imposta la dimensione
    if(dimensione=="Small") pGeneratore->setFuoco(Generatore::FUOCO_SMALL);
    else if(dimensione=="Large")  pGeneratore->setFuoco(Generatore::FUOCO_LARGE);
    else return FALSE;


    // Verifica se deve effettuare l'update del fuoco
    pGeneratore->updateFuoco();

    return TRUE;

}



/*
 *  Impostazione della modalit√  OPERATIVA
 */
void console::handleSetOperatingMode(void)
{
    // Impostazione modalit√  OPERATIVA
    xSequence.workingMode = _EXPOSURE_MODE_OPERATING_MODE;
    ApplicationDatabase.setData(_DB_EXPOSURE_MODE,(unsigned char) xSequence.workingMode);
    // Spegne il simbolo della calibrazione
     ApplicationDatabase.setData(_DB_CALIB_SYM,"",0);

     // Update collimazione
     pCollimatore->updateColli();

}

/*
 *  Modalit√  calibrazione KV Generatore
 */
void console::handleSetKvCalibMode(void)
{
    // Impostazione modalit√  OPERATIVA
    xSequence.workingMode = _EXPOSURE_MODE_CALIB_MODE_KV;
    ApplicationDatabase.setData(_DB_EXPOSURE_MODE,(unsigned char) xSequence.workingMode);

    // Accende simbolo di calibrazione in corso
    // <TBD> impostare il simbolo relativo alla calibrazione in corso
    //pagina0->setCalibOn(TRUE);

    // Imposta la Modalit√  di funzionamento del generatore
    pGeneratore->tomoMode=FALSE;

    // Annulla eventuali allarmi PAD
    PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_PAD,0);

}


///////////////////////////////////////////////////////////////////
/*


 Data: 29/08/2017
 Autore: M.Rispoli

 [AEC/NOAEC] [TN/TW/TI/2D] [COMBO] [STATIC] [HIGHLOW]

 Questa funzione definisce la tipologi di sequenza raggi che verr√  eseguita.

 return 0 OK

*/
///////////////////////////////////////////////////////////////////
int console::handleSetMode(protoConsole* protocollo)
{
    // Inizializzazione sequenza di default
    xSequence.isTomoN=FALSE;
    xSequence.isTomoW=FALSE;
    xSequence.isTomoI=FALSE;
    xSequence.isTomoMoving=TRUE;
    xSequence.is2D=FALSE;
    xSequence.isCombo=FALSE;
    xSequence.isAEC=FALSE;
    xSequence.isAE= FALSE;


    // Passa in rassegna tutti i parametri
    for(int ciclo=0; ciclo < protocollo->parametri.size(); ciclo++){
        if(protocollo->parametri[ciclo]=="AEC")         xSequence.isAEC=TRUE;
        else if(protocollo->parametri[ciclo]=="TN")     xSequence.isTomoN=TRUE;
        else if(protocollo->parametri[ciclo]=="TW")     xSequence.isTomoW=TRUE;
        else if(protocollo->parametri[ciclo]=="TI")     xSequence.isTomoI=TRUE;
        else if(protocollo->parametri[ciclo]=="2D")     xSequence.is2D=TRUE;
        else if(protocollo->parametri[ciclo]== "COMBO") xSequence.isCombo=TRUE;
        else if(protocollo->parametri[ciclo]== "STATIC") xSequence.isTomoMoving=FALSE;
        else if(protocollo->parametri[ciclo]== "HIGHLOW") {
            xSequence.isAE= TRUE;
            xSequence.is2D=TRUE;
        }
    }

    // Imposta la Modalit√  di funzionamento nel generatore
    if(!xSequence.is2D) pGeneratore->tomoMode=TRUE;
    else        pGeneratore->tomoMode=FALSE;

    pGeneratore->aecMode = xSequence.isAEC;

    return 0;
}
///////////////////////////////////////////////////////////////////
/*


 Data: 29/06/2016
 Autore: M.Rispoli

 param:
 - CC   0¬∞
 - P15  15¬∞
 - M15 -15¬∞
 - HW  Home Wide
 - HN  Home Narrow
 - HI  Home Intermediate
 - val valore arbitrario +-26¬∞

*/
///////////////////////////////////////////////////////////////////
bool console::handleSetTube(QString param,unsigned char id)
{
    unsigned char data[4];


    // Imposta i parametri del comando
    data[1]=MOVE_WAIT_START|MOVE_WAIT_END; // Attende comando precedente + Attende questo comando
    data[2]=0;

    if(param=="CC")
        data[0]=TRX_MOVE_CC; // Comando CC
    else if(param=="P15"){
        data[0]=TRX_MOVE_P15; // Comando P15
    }
    else if(param=="M15"){
        data[0]=TRX_MOVE_M15; // Comando M15
    }
    else if(param=="HW"){
        data[0]=TRX_MOVE_HOME_W; // Comando Home Wide
    }
    else if(param=="HN"){
        data[0]=TRX_MOVE_HOME_N; // Comando Home Narrow
    }
    else if(param=="HI"){
        data[0]=TRX_MOVE_HOME_I; // Comando Home Intermediate
    }
    else
    {
        int angolo = param.toInt();
        if((angolo>26)||(angolo<-26)){
            qDebug() << "CONSOLE <SetTube>: ERRORE ANGOLO(<=26):" << angolo;
            PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_TRX,TRX_INVALID_ANGOLO,TRUE); // Self resetting
            return FALSE;
        }

        data[0]=TRX_MOVE_ANGLE; // Movimento con angolo aggiuntivo
        data[1]=0;
        data[2] = (unsigned char) angolo;
        data[3] = (unsigned char) (angolo>>8);
    }

    // Invio comando
    if(pConsole->pGuiMcc->sendFrame(MCC_CMD_TRX,id,data, 4)==FALSE)
    {
        qDebug() << "CONSOLE <SetTube>: ERRORE COMANDO MCC";
        PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_SOFT,ERROR_MCC,TRUE); // Self resetting
        return FALSE;
    }

    return TRUE;

}

/*
 *  Comando di interruzione immediato di qualsiasi movimento
 *  Utilizza il comando del gruppo service, per comodit√
 */
void console::handleSetStopMove(void){
    QByteArray buf;
    buf.append((unsigned char) SRV_ARM_STOP); // Comando di servizio

    // Invia il comando per fermare il movimento
    pConsole->pGuiMcc->sendFrame(MCC_SERVICE,0,(unsigned char*) buf.data(), buf.size());

}

/*
 *  Imposta la modalit√  dead-men
 */
void console::handleSetDeadMan(bool val){
    if(val==pConfig->userCnf.deadman) return; // E' gi√  impostata come richiesto
    pConfig->userCnf.deadman = val;
    pConfig->saveUserCfg();

    if(val) ApplicationDatabase.setData(_DB_DEAD_MEN,(unsigned char) 1,DBase::_DB_NO_OPT);
    else ApplicationDatabase.setData(_DB_DEAD_MEN,(unsigned char) 0,DBase::_DB_NO_OPT);

}

///////////////////////////////////////////////////////////////////
/*

 Questo comando richiede l'attivazione del braccio C-ARM
 Devono essere attivi i controlli su:
    - Compressione in corso
    - Angolo critico

 Il segnale hardware di disattivazione del movimento invece
 non viene controllato in quanto questa sequenza avviene al
 di fuori della sequenza raggi e senza una paziente sotto
 compressione..

 Data: 14/10/2014
 Autore: M.Rispoli

*/
///////////////////////////////////////////////////////////////////
void console::handleSetArm(int target,int minimo, int massimo,int id)
{
    unsigned char data[3];
    signed short angolo;
    int angolo_corrente;
    protoConsole answ(id,UNICODE_FORMAT);


    // Se c'Ë la biopsia il massimo angolo attivabile Ë 90∞
    if(pBiopsy->connected){
        if(abs(target)>90){
            answ.addParam(QString("%1").arg((int)ARM_RANGE_ERROR));
            emit consoleTxHandler(answ.cmdToQByteArray("NOK 9"));
            PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_ARM,ARM_RANGE_ERROR,TRUE);
            return;
        }
    }


    // Angolo in posizione: risponde immediato
    angolo_corrente =  pConfig->convertDangolo(ApplicationDatabase.getDataI(_DB_DANGOLO));
    angolo = target;

    xSequence.arm_angolo = angolo;
    xSequence.arm_min = minimo;
    xSequence.arm_max = massimo;


    // Per i movimenti manuali il comando non deve avere effetti ulteriori
    if(!pConfig->sys.armMotor) {
        emit consoleTxHandler( answ.answToQByteArray(_IMMEDIATO));
        return;
    }

    // SE si trova in un angolo tra +-2 il target allora non si muove
    if((angolo >= angolo_corrente-2) && (angolo <= angolo_corrente+2))
    {
       emit consoleTxHandler( answ.answToQByteArray(_IMMEDIATO));
       return;
    }

    // Se il braccio √® da spostare, vengono controllate le condizioni di movimento
    if(pCompressore->isCompressed())
    {
        answ.addParam(QString("%1").arg((int)ARM_ERROR_INVALID_STATUS));
        emit consoleTxHandler(answ.cmdToQByteArray("NOK"));
        PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_ARM,ARM_ERROR_INVALID_STATUS,TRUE);
        return;
    }

    // Risponde di attendere il completamento
    emit consoleTxHandler( answ.answToQByteArray(255));

    // Impostazione Parametro in gradi
    data[0] =(unsigned char) (angolo&0xFF);
    data[1] =(unsigned char) (angolo>>8);

    // Invio comando
    if(pConsole->pGuiMcc->sendFrame(MCC_CMD_ARM,id,data, 2)==FALSE)
    {
        qDebug() << "CONSOLE <SetArm>: ERRORE COMANDO MCC";
        PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_SOFT,ERROR_MCC,TRUE); // Self resetting
    }

    pToConsole->notifyMovingArm(); // Avvisa la AWS che sta per muovere
    return;
}



///////////////////////////////////////////////////////////////////
/*

    Abilitazione pulsante raggi.

    Quando il pulsante raggi √® abilitato, alla pressione
    del pulsante raggi deve essere inviato alla console
    il comando di SET_PUSH per avvisare la console della
    richiesta di invio raggi.

    PARAMETRI:
        - param = 1 -> Abilitato
        - param = 0 -> Disabilitato

 Data: 14/10/2014
 Autore: M.Rispoli

*/
///////////////////////////////////////////////////////////////////
bool console::handleSetPushEna(QString param)
{
    if(param=="ON")
    {
        // Verifica di alcune condizioni possibili di errore
        if(pConfig->testConfigError(true,true)) return false;

        // Verifica diagnostica generatore
        if(pGeneratore->dgn_fault){
            PageAlarms::reopenExistingAlarm(_DB_ALLARMI_ALR_GEN,pGeneratore->dgn_fault_code,FALSE);
            return FALSE;
        }

        // Verifica diagnostica compressore
        if(pCompressore->safety_fault){
            PageAlarms::reopenExistingAlarm(_DB_ALLARMI_ALR_COMPRESSORE,_COMPR_SAFETY_FAULT,FALSE);
            return FALSE;
        }

        // Verifica in caso di starter low speed che esso sia calibrato
        if(pConfig->sys.highSpeedStarter==false){
            if(!pGeneratore->isStarterCalibrated()){                
                PageAlarms::reopenExistingAlarm(_DB_ALLARMI_ALR_GEN, GEN_STARTER_NOT_CALIBRATED, TRUE);
                return FALSE;
            }
        }

        // Verifica che la porta dello studio sia chiusa
        if(ApplicationDatabase.getDataU(_DB_CLOSED_DOOR)==0){
            PageAlarms::reopenExistingAlarm(_DB_ALLARMI_ALR_RAGGI,ERROR_CLOSED_DOOR,TRUE);
            return FALSE;
        }

        // Aggiorname
        // Abilitazione pulsante raggi
        pToConsole->enableXrayPush(TRUE);

    }else
    {
        // Disabilitazione pulsante raggi
        pToConsole->enableXrayPush(FALSE);
    }


    return TRUE;
}

///////////////////////////////////////////////////////////////////
/*


 Data: 31/10/2014
 Autore: M.Rispoli

*/
///////////////////////////////////////////////////////////////////
void console::handleSetSblocco(void)
{
}

/*
 *  Dati di post esposizione inviati come log a Console
 *  data (vedi m4.h)    
*/
void console::rxDataLog(QByteArray buffer)
{

    QString stringa;
    int i;
    int NAEC = buffer.at(NSAMPLES_AEC);
    int NPLS = buffer.at(NSAMPLES_PLS);


    stringa.append(QString("\n\r------ RX_DATA_OUTPUT:%1 ---------\n\r").arg((int) buffer.at(RX_END_CODE)));
    stringa.append(QString("VBUS(V):%1 \n\r").arg((int) pGeneratore->convertHvexp(buffer.at(HV_POST_RAGGI))));
    stringa.append(QString("IFIL(mA):%1 \n\r").arg(47.98 * (float) buffer.at(IFIL_POST_RAGGI)));
    stringa.append(QString("mAs(pre):%1 \n\r").arg((int) (buffer.at(MAS_PRE_L)+256*buffer.at(MAS_PRE_H))));
    stringa.append(QString("mAs(pls):%1 \n\r").arg((int) (buffer.at(MAS_PLS_L)+256*buffer.at(MAS_PLS_H))/50));
    stringa.append(QString("Tpre(ms):%1 \n\r").arg((int) (buffer.at(T_MEAN_PRE_L)+256*buffer.at(T_MEAN_PRE_H))));
    stringa.append(QString("Tpls(ms):%1 \n\r").arg((int) (buffer.at(T_MEAN_PLS_L)+256*buffer.at(T_MEAN_PLS_H))));

    if((NAEC+NPLS)==0){
        stringa.append(QString("NO VOLTAGE/CURRENT SAMPLES AVAILABLE \n\r"));
    }else{
        float vmean = pGeneratore->convertPcb190Kv(buffer.at(V_MEAN),1.0);
        unsigned char imean = pGeneratore->convertPcb190Ianode(buffer.at(I_MEAN));
        stringa.append(QString("READ-KV(kV):%1 \n\r").arg(vmean));
        float sigma = (float) buffer.at(V_SIGMA) / 10.;
        if(sigma > 0.5) stringa.append(QString("\n\r WARNING!!!:------------------> SIGMA-KV(kV):%1 \n\r\n\r").arg(sigma));
        else stringa.append(QString("SIGMA-KV(kV):%1 \n\r").arg(sigma));
        stringa.append(QString("READ-IA(mA):%1 \n\r").arg((int) imean));


        // Aggiunge in coda i campionamenti
        if(NAEC){
            stringa.append(QString("AEC SAMPLES:%1  \n\r").arg(NAEC));
            for(i = 0; i< NAEC;i++){
                int rawI = (int) (buffer.at(SAMPLES_BUFFER + i));
                int rawV = (int) (buffer.at(SAMPLES_BUFFER + NAEC + NPLS + i));
                int mA = rawI*200/255;
                float kV = pGeneratore->convertPcb190Kv((unsigned char) rawV,1.0);
                stringa.append(QString("%1: I=%2(mA) %3(RAW) V=%4(kV) %5(RAW)] \n\r").arg(i).arg(mA).arg(rawI).arg(kV).arg(rawV));
            }

        }

        if(NPLS){
            stringa.append(QString("PULSE SAMPLES:%1  \n\r").arg(NPLS));
            for(i = 0; i< NPLS;i++){
                int rawI = (int) (buffer.at(SAMPLES_BUFFER + NAEC+ i));
                int rawV = (int) (buffer.at(SAMPLES_BUFFER + NAEC + NPLS + NAEC + i));
                int mA = rawI*200/255;
                float kV = pGeneratore->convertPcb190Kv((unsigned char) rawV,1.0);
                stringa.append(QString("%1: I=%2(mA) %3(RAW) V=%4(kV) %5(RAW)] \n\r").arg(i).arg(mA).arg(rawI).arg(kV).arg(rawV));
            }

        }
    }

    // Campionamenti di tempo
    QByteArray data;
    int NTIME=0;
    if(SAMPLES_BUFFER + 2 * (NAEC + NPLS) < buffer.size()){
        NTIME = buffer.at(SAMPLES_BUFFER + 2 * (NAEC + NPLS) );
        if(NTIME){
            stringa.append(QString("PULSE TIME SAMPLES:%1  \n\r").arg(NTIME));
            for(i=0;i<NTIME; i++){
                stringa.append(QString("%1: T=%2(ms) \n\r").arg(i).arg((int) buffer.at(SAMPLES_BUFFER + 2 * (NAEC + NPLS)+1+i)));
                data.append(buffer.at(SAMPLES_BUFFER + 2 * (NAEC + NPLS)+1+i));
            }
            // Invio Samples
            pToConsole->setSamples(data);

        }else{
            stringa.append(QString("NO PULSE TIME SAMPLES AVAILABLE  \n\r"));
        }
    }else{
        stringa.append(QString("NO PULSE TIME SAMPLES AVAILABLE  \n\r"));
    }

    qDebug() << stringa;

}



///////////////////////////////////////////////////////////////////
/*
void console::handleOpenStudy(bool local, QString param)
    This function set the Study to Open.

 PARAM:
    - local: TRUE the sudy is local else is remote
    - param:
             if local it is a code string;
             in case of REMOTE it is in the format <name>^<surname>


 Data: 14/10/2014
 Autore: M.Rispoli

*/
///////////////////////////////////////////////////////////////////
void console::handleOpenStudy(bool local, protoConsole* pFrame)
{
    QString stringa;
    int i;


    // Costruisce il nome completo in caso di spazi
    for(i=0; i< pFrame->parametri.size(); i++)
    {
        if(i!=0) stringa.append(" ");
        stringa.append(pFrame->parametri[i]);
    }

    setOpenStudy(local , stringa);

}

// COMANDO PER L'APERTURA DELLA PAGINA OPERATIVA
// Prima di richiedere al config di scegliere quale pagina
// aprire Ë necessario impostare la tipologia di pagina che deve essere aperta
// tra _OPEN_STUDY_LOCAL/_OPEN_STUDY_DICOM
void console::setOpenStudy(bool local, QString stringa)
{

    xSequence.arm_angolo = pConfig->convertDangolo(ApplicationDatabase.getDataI(_DB_DANGOLO));
    xSequence.arm_min = xSequence.arm_angolo - 5;
    xSequence.arm_max = xSequence.arm_angolo + 5;


    ApplicationDatabase.setData(_DB_INTESTAZIONE,stringa);
    if(local) ApplicationDatabase.setData(_DB_STUDY_STAT,(unsigned char) _OPEN_STUDY_LOCAL );
    else ApplicationDatabase.setData(_DB_STUDY_STAT,(unsigned char) _OPEN_STUDY_DICOM );
    openStudy = TRUE;

    // Attiva la pagina operativa
    pConfig->selectOperatingPage();
}


///////////////////////////////////////////////////////////////////
/*

 Data: 31/10/2014
 Autore: M.Rispoli

*/
///////////////////////////////////////////////////////////////////
void console::handleCloseStudy(void)
{
    openStudy = FALSE;
    ApplicationDatabase.setData(_DB_STUDY_STAT,(unsigned char) 0); // Questo fa chiudere la pagina e uscire alla MAIN

    // Aggiornamento file di statistiche del generatore se necessario
    pGeneratore->saveTubeStatisticsFile(QString(_TUBEPATH) + QString("/") +pConfig->userCnf.tubeFileName + QString("/")) ;

    // Apertura pagina Main
    //pConfig->selectMainPage();
    return;
}

void console::handleSetXrayLamp(QString par)
{
    ioOutputs out;
    out.mask.CPU_LMP_SW1=1;

    if(par=="ON") out.outputs.CPU_LMP_SW1=1;
    else out.outputs.CPU_LMP_SW1=0;


    io->setOutputs(out);
    return;
}



///////////////////////////////////////////////////////////////////
/*

 Data: 09/11/2014
 Autore: M.Rispoli

*/
///////////////////////////////////////////////////////////////////
void console::demoBiopsy(bool status)
{

    pGuiMcc->sendFrame(MCC_BIOPSY_DEMO_CMD,0,(unsigned char*) &status, sizeof(status));

}



#define NBYTE_FINE_RAGGI 6
void console::guiNotify(unsigned char id, unsigned char mcccode, QByteArray data)
{
    short arm ;
    short trx;
    short gonio;
    unsigned char arm_mem_dir;
    protoConsole cmd(id,UNICODE_FORMAT);

    switch(mcccode)
    {

    case MCC_GET_TROLLEY:

        // Notifica asincrona posizione del compressore
        pToConsole->sendNotificheTcp(cmd.ackToQByteArray(data.at(0)));
        break;
    case MCC_CMD_ARM:
        // Handler di segnali di fine movimento Braccio
        PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_ARM, data.at(0),TRUE);// in caso di fallimento viene segnalato un   errore auto-resettante
        pToConsole->sendNotificheTcp(cmd.ackToQByteArray(data.at(0))); // Segnalazione alla AWS di movimento terminato
        break;

    case MCC_CMD_TRX:
        // Handler di segnali di fine movimento Tubo
        PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_TRX, data.at(0),TRUE); // in caso di fallimento viene segnalato un   errore auto-resettante
        pToConsole->sendNotificheTcp(cmd.ackToQByteArray(data.at(0)));      // Segnalazione alla AWS di movimento terminato
        break;

    case MCC_ARM_ERRORS:
        // Handler di segnalazione di errori ARM direttamente dal driver
        // data.at(0) -> Codice errore, data.at(1) -> Sub-Codice
        PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_ARM, data.at(0),false);
        break;

    case MCC_TRX_ERRORS:
        // Handler di segnalazione di errori TRX direttamente dal driver
        // data.at(0) -> Codice errore, data.at(1) -> Sub-Codice
        PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_TRX, data.at(0),false);
        break;

    case MCC_LENZE_ERRORS:
        // Handler di segnalazione di errori TRX direttamente dal driver
        // data.at(0) -> Codice errore, data.at(1) -> Sub-Codice
        PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_LENZE, data.at(0),false);
        break;

    case MCC_GET_GONIO:
        // Formato: 0-1 -> ARM
        // Formato: 2-3 -> TRX
        // Formato: 4-5 -> GONIO
        // Tutti gli angoli sono in formatoo decimo di grado
        arm =     (short) (data.at(0) + 256 * data.at(1));      // Espresso in decimi di grado
        trx =     (short) (data.at(2) + 256 * data.at(3)) / 10; // Espresso in decimi di grado
        gonio =   (short) (data.at(4) + 256 * data.at(5));      // Espresso in decimi di grado
        arm_mem_dir = (unsigned char) data.at(6);

        // Salvataggio precoce dello stato del braccio
        if(pConfig->armConfig.direction_memory!= arm_mem_dir){
            pConfig->armConfig.direction_memory = arm_mem_dir;
            pConfig->saveArmConfig();
        }

        ApplicationDatabase.setData(_DB_DANGOLO,(int) arm,0);
        ApplicationDatabase.setData(_DB_TRX,(int) trx,0);
        ApplicationDatabase.setData(_DB_GONIO,(int) trx,0);

        break;

    case MCC_AUDIO:
        if(data.at(0)) ApplicationDatabase.setData(_DB_AUDIO_PRESENT,(unsigned char) 1);
        else ApplicationDatabase.setData(_DB_AUDIO_PRESENT,(unsigned char) 0);
        break;
    default:
            return;
        break;
    }
}

// ----------------------------------------------------------------------------- DA FARE
///////////////////////////////////////////////////////////////////
/*

            GESTIONE MESSAGGI PROVENIENTI DA MASTER M4
            PER COMUNICAZIONI FINE SEQUENZE

            QUESTA ROUTINE HA LA STESSA AFFINITA' DELLA GUI !!!

 Data: 25/02/2015
 Autore: M.Rispoli

*/
///////////////////////////////////////////////////////////////////
void mccMasterCom::mccRxHandler(_MccFrame_Str mccframe)
{
 int ciclo;
 QByteArray bufdata;
 unsigned char cval;

#ifdef __DISABLE_MCC_RX
 return;
#endif


 switch(mccframe.cmd)
  {
        case MCC_GUI_NOTIFY: // Stato di esecuzione del processo su M4

         bufdata.clear();
         for(ciclo=0; ciclo<mccframe.buffer[1]; ciclo++ )
             bufdata.append(mccframe.buffer[2+ciclo]);
         pConsole->emitMccGuiNotify(mccframe.id, mccframe.buffer[0],bufdata);
         break;


        case MCC_PCB215_NOTIFY: // Notifiche da Driver PCB215

        bufdata.clear();
        for(ciclo=0; ciclo<mccframe.buffer[1]; ciclo++ )
          bufdata.append(mccframe.buffer[2+ciclo]);
        pConsole->emitPcb215Notify(mccframe.id, mccframe.buffer[0],bufdata);
        break;

        case MCC_PCB190_NOTIFY: // Notifiche da Driver PCB190

        bufdata.clear();
        for(ciclo=0; ciclo<mccframe.buffer[1]; ciclo++ )
           bufdata.append(mccframe.buffer[2+ciclo]);
        pConsole->emitPcb190Notify(mccframe.id, mccframe.buffer[0],bufdata);
        break;

        case MCC_PCB249U1_NOTIFY: // Notifiche da Driver PCB249U1

        bufdata.clear();
        for(ciclo=0; ciclo<mccframe.buffer[1]; ciclo++ )
           bufdata.append(mccframe.buffer[2+ciclo]);
        pConsole->emitPcb249U1Notify(mccframe.id, mccframe.buffer[0],bufdata);
        break;
        case MCC_SERVICE_NOTIFY:

        bufdata.clear();
        for(ciclo=0; ciclo<mccframe.buffer[1]; ciclo++ )
          bufdata.append(mccframe.buffer[2+ciclo]);
        pConsole->emitServiceNotify(mccframe.id, mccframe.buffer[0],bufdata);
        break;

         case MCC_244_A_FUNCTIONS:

         bufdata.clear();
         for(ciclo=0; ciclo<mccframe.buffer[1]; ciclo++ )
           bufdata.append(mccframe.buffer[2+ciclo]);
         pConsole->emitPcb244ANotify(mccframe.id, mccframe.buffer[0],bufdata);
         break;

        case MCC_LOADER_NOTIFY: // Notifiche dal Loader M4

        bufdata.clear();
        for(ciclo=0; ciclo<mccframe.buffer[1]; ciclo++ )
          bufdata.append(mccframe.buffer[2+ciclo]);
        pConsole->emitLoaderNotify(mccframe.id, mccframe.buffer[0],bufdata);
        break;



        case MCC_BIOP_NOTIFY: // Notifiche da Driver PCB215

        bufdata.clear();
        for(ciclo=0; ciclo<mccframe.buffer[1]; ciclo++ )
           bufdata.append(mccframe.buffer[2+ciclo]);
        pConsole->emitBiopsyNotify(mccframe.id, mccframe.buffer[0],bufdata);
        break;

        case MCC_CONFIG_NOTIFY: // Notifica configuratore M4

        for(ciclo=0; ciclo<mccframe.buffer[1]; ciclo++ )
            bufdata.append(mccframe.buffer[2+ciclo]);
        pConsole->emitConfigNotify(mccframe.id, mccframe.buffer[0],bufdata);
        break;


        // Questo messaggio viene inviato da M4 a seguito di un'esposizione
        // per comunicare i dati registrati durante l'esposizione.
        case  MCC_RAGGI_DATA:      // Messaggio di notifica dati di esposizione

         bufdata.clear();
         for(ciclo=0; ciclo<mccframe.len; ciclo++)
             bufdata.append(mccframe.buffer[ciclo]);
         pConsole->emitRaggiData(bufdata);
        break;
 }
}


/////////////// CALIBRAZIONI GUIDATE DA CONSOLE /////////////////////////////////////////////////////////////////////

/*  Comando di impostazione contenuto file di configurazione
 *  per l'impostazione dei filtri
 *
 *  Il comando rende immediatamente operative le modifiche, dato che in
 *  modalit√  2D, la collimazione viene impostata ogni volta che viene effettuata
 *  una sequenza raggi.
 *
 *  FORMATO:  %  MAT0 MAT1 MAT2 MAT3 POS0 POS1 POS2 POS3 %
 *  MAT0,MAT1,MAT2,MAT3 [Rh,Ag,Al,Us]
 *  POS0,POS1,POS2,POS3 [0:255] -> -1 non viene modificato
 *  return TRUE: %OK 0%
 *  return FALSE:%NOK 0%
 */
bool console::handleSetColliFilter(protoConsole* frame)
{
    int i;

    if(frame->parametri.size() != 8) return FALSE;

    // CONTROLLO MATERIALI
    for(i=0;i<4;i++)
        if((frame->parametri[i]!="Rh")&&(frame->parametri[i]!="Al")&&(frame->parametri[i]!="Ag")&&(frame->parametri[i]!="Us")&&(frame->parametri[i]!="Cu")&&(frame->parametri[i]!="Mo")) return FALSE;

    // CONTROLLO POSIZIONI
    for(i=4;i<8;i++)
        if((frame->parametri[i].toInt()<-1)||(frame->parametri[i].toInt()>255)) return FALSE;

    for(i=0;i<4;i++)
    {
        if(frame->parametri[i]=="Rh")      pCollimatore->colliConf.filterType[i] = (unsigned char) Collimatore::FILTRO_Rh;
        else if(frame->parametri[i]=="Ag") pCollimatore->colliConf.filterType[i] = (unsigned char) Collimatore::FILTRO_Ag;
        else if(frame->parametri[i]=="Al") pCollimatore->colliConf.filterType[i] = (unsigned char) Collimatore::FILTRO_Al;
        else if(frame->parametri[i]=="Mo") pCollimatore->colliConf.filterType[i] = (unsigned char) Collimatore::FILTRO_Mo;
        else                               pCollimatore->colliConf.filterType[i] = (unsigned char) Collimatore::FILTRO_Cu;
    }

    for(i=4;i<8;i++)
        if(frame->parametri[i].toInt()!=-1) pCollimatore->colliConf.filterPos[i-4] = frame->parametri[i].toInt();

    return TRUE;
}

/*  Comando di impostazione steps per specchio in campo
 *
 *  Il comando rende immediatamente operative le modifiche, dato che in
 *  modalit√  2D, la collimazione viene impostata ogni volta che viene effettuata
 *  una sequenza raggi.
 *
 *  FORMATO:  %  STEPS %
 *  STEPS [0:2000] -> -1 non viene modificato
 *  return TRUE: operazione eseguita
 *  return FALSE: dati non corretti
 */
bool console::handleSetColliMirror(protoConsole* frame)
{
    int* pToMirrorSteps;
    if(frame->parametri.size() != 1) return FALSE;

    if(pCollimatore->colli_model == _COLLI_TYPE_ASSY_01) pToMirrorSteps = & (pCollimatore->colliConf.mirrorSteps_ASSY_01);
    else pToMirrorSteps = & (pCollimatore->colliConf.mirrorSteps_ASSY_02);

    if((frame->parametri[0].toInt()<-1)||(frame->parametri[0].toInt()>2000)) return FALSE;
    if(frame->parametri[0].toInt()!=-1){
        if(*pToMirrorSteps != frame->parametri[0].toInt()){
            *pToMirrorSteps = frame->parametri[0].toInt();
            pCollimatore->storeConfigFile();
        }
    }
    // Forza una nuova collimazione se necessario
    if(pCollimatore->manualCollimation==TRUE) pCollimatore->manualColliUpdate();

    // Spegne lo specchio per forzare l'update ..
    pCollimatore->setLamp(Collimatore::LAMPMIRR_OFF,0);
    return TRUE;
}




/*  Comando di impostazione contenuto file di configurazione collimazione
 *  per le lame di collimazione in modalit√  2D.
 *
 *  FORMATO:  % PAD ANODO INGRANDIMENTO FRONT BACK RIGHT LEFT TRAP%
 *  PAD = Stringa PAD
 *  FUOCO = [W/Mo]
 *  BACK, FRONT, LEFT, RIGHT, TRAP --> [-1/0:250] Se -1 il relativo valore non viene modificato
 *
 *  return true:  OK
 *  return false: error
 */
bool console::handleSetColli2D(protoConsole* frame)
{

    if(frame->parametri.size() != 8) return false;

    // Controllo sulla coppia PAD+ANODO
    QString pad = frame->parametri[0];
    QString anodo = frame->parametri[1];
    QString mag = frame->parametri[2]; // Predisposizione: ora non usato

    // Collimazione OPEN
    if(pad=="OPEN"){

        pCollimatore->colliConf.colliOpen.F = frame->parametri[3].toInt();
        pCollimatore->colliConf.colliOpen.B = frame->parametri[4].toInt();
        pCollimatore->colliConf.colliOpen.R = frame->parametri[5].toInt();
        pCollimatore->colliConf.colliOpen.L = frame->parametri[6].toInt();
        pCollimatore->colliConf.colliOpen.T = frame->parametri[7].toInt();
        return true;
    }

    if(pad=="PADTOMO"){

        if(anodo=="W"){
            pCollimatore->colliConf.colliTomoW.tomoFront =  frame->parametri[3].toInt();
            pCollimatore->colliConf.colliTomoW.tomoBack =  frame->parametri[4].toInt();
        }else if(anodo=="Mo"){
            pCollimatore->colliConf.colliTomoMo.tomoFront =   frame->parametri[3].toInt();
            pCollimatore->colliConf.colliTomoMo.tomoBack = frame->parametri[4].toInt();
        }
        return true;
    }

    int code = pCompressore->getPadCodeFromTag(pad);
    if(code>=PAD_ENUM_SIZE) return false;

    for(int i=0; i<pCollimatore->colliConf.colli2D.size(); i++)
    {
        if(pCollimatore->colliConf.colli2D[i].PadCode == code)
        {
            pCollimatore->colliConf.colli2D[i].F = frame->parametri[3].toInt();
            pCollimatore->colliConf.colli2D[i].B = frame->parametri[4].toInt();
            pCollimatore->colliConf.colli2D[i].R = frame->parametri[5].toInt();
            pCollimatore->colliConf.colli2D[i].L = frame->parametri[6].toInt();
            pCollimatore->colliConf.colli2D[i].T = frame->parametri[7].toInt();

            return true;
        }
    }

    // Se √® arrivato qui allora il dato PAD non √® configurato
    _colliPadStr newColli2DItem;
    newColli2DItem.PadCode = code;
    newColli2DItem.F = frame->parametri[3].toInt();
    newColli2DItem.B = frame->parametri[4].toInt();
    newColli2DItem.R = frame->parametri[5].toInt();
    newColli2DItem.L = frame->parametri[6].toInt();
    newColli2DItem.T = frame->parametri[7].toInt();
    pCollimatore->colliConf.colli2D.append(newColli2DItem);
    return true;

}


bool console::handleSetColliMode(protoConsole* frame){
    if(frame->parametri.size() != 1) return false;

    if(frame->parametri[0]=="MANUAL"){
        pCollimatore->manualCollimation=TRUE;
        pCollimatore->manualColliUpdate();
        return true;
    }else if(frame->parametri[0]=="AUTO"){
        pCollimatore->manualCollimation=FALSE;
        pCollimatore->manualFiltroCollimation=FALSE;
        pCollimatore->setFiltro();
        pCollimatore->updateColli();

        return true;
    }

    return false;
}

bool console::handleSetManualColli(protoConsole* frame){

    if(frame->parametri.size() != 5) return false;
    if(pCollimatore->manualCollimation==FALSE) return false;

    pCollimatore->manualF = frame->parametri[0].toInt();
    pCollimatore->manualB = frame->parametri[1].toInt();
    pCollimatore->manualR = frame->parametri[2].toInt();
    pCollimatore->manualL = frame->parametri[3].toInt();
    pCollimatore->manualT = frame->parametri[4].toInt();


    return true;
}


void console::handleGetColli2D(protoConsole* frame, protoConsole* answer){

    if(frame->parametri.size() != 3) {
        emit  consoleTxHandler(answer->cmdToQByteArray("NOK 1"));
        return ;
    }

    // Verifica se si tratta della collimazione OPEN
    QString pad = frame->parametri[0];
    QString anodo = frame->parametri[1];
    QString mag = frame->parametri[2];
    QString risultato;

    if(pad=="OPEN"){
        risultato = QString("OK %1 %2 %3 %4 %5").arg(pCollimatore->colliConf.colliOpen.F).arg(pCollimatore->colliConf.colliOpen.B).arg(pCollimatore->colliConf.colliOpen.R).arg(pCollimatore->colliConf.colliOpen.L).arg(pCollimatore->colliConf.colliOpen.T);
        emit  consoleTxHandler(answer->cmdToQByteArray(risultato));
        return;
    }

    if(pad=="PADTOMO"){
        if(anodo=="W"){
            risultato = QString("OK %1 %2 0 0 0").arg(pCollimatore->colliConf.colliTomoW.tomoFront).arg( pCollimatore->colliConf.colliTomoW.tomoBack);
        }else if(anodo=="Mo"){
            risultato = QString("OK %1 %2 0 0 0").arg(pCollimatore->colliConf.colliTomoMo.tomoFront).arg( pCollimatore->colliConf.colliTomoMo.tomoBack);
        }
        emit  consoleTxHandler(answer->cmdToQByteArray(risultato));
        return ;
    }

    int code = pCompressore->getPadCodeFromTag(pad);
    if((code>=PAD_ENUM_SIZE)||(code<0)) {
        emit  consoleTxHandler(answer->cmdToQByteArray("NOK 2"));
        return ;
    }

    for(int i=0; i<pCollimatore->colliConf.colli2D.size(); i++)
    {
        if(pCollimatore->colliConf.colli2D[i].PadCode == code)
        {
            QString risultato = QString("OK %1 %2 %3 %4 %5").arg(pCollimatore->colliConf.colli2D[i].F).arg(pCollimatore->colliConf.colli2D[i].B).arg(pCollimatore->colliConf.colli2D[i].R).arg(pCollimatore->colliConf.colli2D[i].L).arg(pCollimatore->colliConf.colli2D[i].T);
            emit  consoleTxHandler(answer->cmdToQByteArray(risultato));
            return;
        }
    }

    // Se il collimatore non √® configurato restituisce la OPEN
    risultato = QString("OK %1 %2 %3 %4 %5").arg(pCollimatore->colliConf.colliOpen.F).arg(pCollimatore->colliConf.colliOpen.B).arg(pCollimatore->colliConf.colliOpen.R).arg(pCollimatore->colliConf.colliOpen.L).arg(pCollimatore->colliConf.colliOpen.T);
    emit  consoleTxHandler(answer->cmdToQByteArray(risultato));
    return ;

}


bool console::handleSetStarter(protoConsole* frame)
{
    unsigned char data;

    if(frame->parametri.size()!=1) return FALSE;
    if(frame->parametri[0]=="STOP") data = 0;
    else if(frame->parametri[0]=="L") data = 1;
    else if(frame->parametri[0]=="H") data = 2;
    else return FALSE;

    return pConsole->pGuiMcc->sendFrame(MCC_STARTER,1,&data, 1);

}



/* Comando di richiesta parametri filtro
 *
 *  FORMATO:  % no param %
 *
 *  return %OK MAT0 MAT1 MAT2 MAT3 POS0 POS1 POS2 POS3
 *  MAT0,MAT1,MAT2,MAT3 ->[Rh,Ag,Al,Us]
 *  POS0,POS1,POS2,POS3 ->[0:255]
 */
void console::handleGetColliFilter(protoConsole* frame,protoConsole* answ)
{
    int i;

    // Materiali associati alle posizioni del filtro
    for(i=0; i<4;i++)
    {
        if(pCollimatore->colliConf.filterType[i]==Collimatore::FILTRO_Al) answ->parametri.append("Al");
        else if(pCollimatore->colliConf.filterType[i]==Collimatore::FILTRO_Ag) answ->parametri.append("Ag");
        else if(pCollimatore->colliConf.filterType[i]==Collimatore::FILTRO_Rh) answ->parametri.append("Rh");
        else if(pCollimatore->colliConf.filterType[i]==Collimatore::FILTRO_Mo) answ->parametri.append("Mo");
        else answ->parametri.append("Cu");
    }

    // Posizioni associate ai filtri
    for(i=0; i<4;i++)
        answ->parametri.append(QString("%1").arg(pCollimatore->colliConf.filterPos[i]));

    // Invio risposta
    emit consoleTxHandler(answ->answToQByteArray("OK"));
    return;
}

/* Comando di richiesta parametri Specchio
 *
 *  FORMATO:  % no param %
 *
 *  return %OK STEPS%
 *  STEPS ->[0:2000]
 *
 */
void console::handleGetColliMirror(protoConsole* frame,protoConsole* answ)
{
    int* pToMirrorSteps;
    if(pCollimatore->colli_model == _COLLI_TYPE_ASSY_01) pToMirrorSteps = & (pCollimatore->colliConf.mirrorSteps_ASSY_01);
    else pToMirrorSteps = & (pCollimatore->colliConf.mirrorSteps_ASSY_02);

    // Steps specchio in campo
    answ->parametri.append(QString("%1").arg(*pToMirrorSteps));

    // Invio risposta
    emit consoleTxHandler(answ->answToQByteArray("OK"));

    return;
}

/*
 *  Effettua il salvataggio dei dati di collimazione. Se √® stato utilizzato
 *  il comando SetColliTomo, oltre al salvataggio dei dati verr√  anche aggiornata la periferica
 *  PCB249U1 con il velttore di dati di collimazione dinamica.
 *  RETURN:
 *  - FALSE: non viene effettuato l'aggiornamento della periferica
 *  - TRUE: √® in corso l'aggiornamento della periferica
 */
bool console::handleSetColliStore(void)
{

    // Salva tutto il contenuto in RAM nel file di configurazione
    pCollimatore->storeConfigFile();


    // Verifica se occorre aggiornare la perioferica
    if(!pConfig->pcb249U1UpdateRequired) return false;

    connect(pConfig,SIGNAL(configUpdatedSgn()),this,SLOT(pcb249U1ConfigCompletedSlot()),Qt::UniqueConnection);
    pConfig->updatePCB249U1();
    return true;
}

void console::pcb249U1ConfigCompletedSlot(void){
    disconnect(pConfig,SIGNAL(configUpdatedSgn()),this,SLOT(pcb249U1ConfigCompletedSlot()));
    pToConsole->endCommandAck(idColliStore,0);

}



/*
 *  Richiesta revisioni dei software di sistema
 *  FORMATO:  %no param%
 *  return %OK REVGUI-M REVGUI-S REVMS REVSL REVPCB215 REVPCB204 REVPCB240 REVPCB249U1 REVPCB249U2 REVPCB190%
 *  RVxx = STRINGA REVISIONE
 *
 */
void console::handleGetSoftwareRevisions(protoConsole* frame)
{
    //pendingId = frame->id;
    //connect(pConfig,SIGNAL(configRevisionSgn()),this,SLOT(consoleSoftwareRevisionsNotify()),Qt::UniqueConnection);
    //pConfig->updateSoftwareRevision();
    // Costruisce il frame di risposta
    frame->addParam("0");
    frame->addParam(pConfig->swConf.rvPackage); // Questo √® rilevato dal file di configurazione

    // I campi seguenti sono letti direttamente dalle periferiche
    frame->addParam(pConfig->rvGuiMaster);
    frame->addParam(pConfig->rvGuiSlave);
    frame->addParam(pConfig->rvM4Master);
    frame->addParam(pConfig->rvM4Slave);
    frame->addParam(pConfig->rv269);
    frame->addParam(pConfig->rv240);
    frame->addParam(pConfig->rv249U1);
    frame->addParam(pConfig->rv249U2);
    frame->addParam(pConfig->rv190);
    frame->addParam(pConfig->rv244A);


    emit consoleTxHandler(frame->answToQByteArray()); // Invio a console

    return;
}

void console::consoleSoftwareRevisionsNotify(void)
{
    protoConsole frame(pendingId,UNICODE_FORMAT);
    disconnect(pConfig,SIGNAL(configRevisionSgn()),this,SLOT(consoleSoftwareRevisionsNotify()));

    // Costruisce il frame di risposta
    frame.addParam("0");
    frame.addParam(pConfig->swConf.rvPackage); // Questo √® rilevato dal file di configurazione

    // I campi seguenti sono letti direttamente dalle periferiche
    frame.addParam(pConfig->rvGuiMaster);
    frame.addParam(pConfig->rvGuiSlave);
    frame.addParam(pConfig->rvM4Master);
    frame.addParam(pConfig->rvM4Slave);
    frame.addParam(pConfig->rv269);
    frame.addParam(pConfig->rv240);
    frame.addParam(pConfig->rv249U1);
    frame.addParam(pConfig->rv249U2);
    frame.addParam(pConfig->rv190);
    frame.addParam(pConfig->rv244A);


    emit consoleTxHandler(frame.answToQByteArray()); // Invio a console
}

/*
 *  Funzione per ottenere la lista dei Tubi configurati
 *  PARAMETRI: nessuno
 *  FRAME DI RISPOSTA:     <ID LEN %OK PAR0 PAR1 .. PAR30 %>

            Tipo    dato                Valore
    PAR0	Int     Numero Parametri	0 = Nessun Tubo configurato nel sistema, N = N tubi configurati

    PAR1 .. N  Elenco tubi
    PAR N+1 Tubo selezionato

 */
void console::handleGetTubes(protoConsole* answer)
{
    int i;
    QStringList list;
    QDir tubes(_TUBEPATH);

    // Legge il contrenuto della directory
    list = tubes.entryList();

    if(list.count()<3)
    {
        // Nessun file configurato
        emit  consoleTxHandler(answer->cmdToQByteArray("OK 0"));
        return        ;
    }

    // Costruisce la risposta scandendo i nomi dei tubi configurati
    int n=list.count()-2;


    if(!pConfig->sys.highSpeedStarter){
       // Filtra i tubi abilitati per la bassa velocit‡
       for(i=2; i< list.count(); i++){
           if(list.at(i).contains("TEMPLATE")){
               if(list.at(i).contains("H")){
                   n--;
               }
           }
       }
    }else{
        // Filtra i tubi abilitati per la alta velocit‡
        for(i=2; i< list.count(); i++){
            if(list.at(i).contains("TEMPLATE")){
                if(!list.at(i).contains("H")){
                    n--;
                }
            }
        }
    }

    if(n==0) {
        // Nessun file configurato
        emit  consoleTxHandler(answer->cmdToQByteArray("OK 0"));
        return        ;
    }

    answer->addParam(QString("%1").arg(n));
    for(i=2; i< list.count(); i++){
       // Non aggiunge i files Template che non sono abilitati
       if( (!pConfig->sys.highSpeedStarter) && (list.at(i).contains("TEMPLATE")) &&  (list.at(i).contains("H"))) continue;
       else if( (pConfig->sys.highSpeedStarter) && (list.at(i).contains("TEMPLATE")) &&  (!list.at(i).contains("H"))) continue;
       answer->addParam(list.at(i));
    }

    // Aggiunge il tubo corrrentemente selezionato
    answer->addParam(pConfig->userCnf.tubeFileName);
    emit  consoleTxHandler(answer->cmdToQByteArray("OK"));

    return;
}

/*
 *  Funzione per selezionare un Tubo configurato.
 *  Attenzione, la funzione modifica il file di configurazione syscfg.cnf
 *  Se il Tubo esiste, il sistema caricher√  i dati del Tubo in memoria.
 *  PARAMETRI: PAR0 = Nome File
 *  FRAME DI RISPOSTA:     <ID LEN %OK/NOK%>
 */
void console::handleSelectTube(QString tubeName, protoConsole* answer)
{
    int i;
    QDir tubes(_TUBEPATH);

    if(tubes.exists(tubeName)==FALSE)
    {
        // Il Tubo richiesto non √® valido
        emit  consoleTxHandler(answer->cmdToQByteArray("NOK"));
        return;
    }

    // Procede con la selezione del Tubo. Per fare ci√≤ il sistema
    // deve prima modificare il nome del file di configurazione nella configurazione di sistema.
    // Successivamente deve rileggere la configurazione del Tubo.

    if(pConfig->userCnf.tubeFileName==tubeName)
    {
        // Termina perch√® il file √® gi√  quello selezionato
        emit  consoleTxHandler(answer->cmdToQByteArray("OK"));
        return;
    }

    // Modifica e salvataggio nuova configurazione
    pConfig->userCnf.tubeFileName = tubeName;
    pConfig->saveUserCfg();

    // Se l'applicazione Ë analogica e il sistema Ë in calibrazione
    // si comunica l'avvenuta selezione del tubo in oggetto
    if((ApplicationDatabase.getDataU(_DB_EXPOSURE_MODE)==_EXPOSURE_MODE_CALIB_MODE_KV) || (ApplicationDatabase.getDataU(_DB_EXPOSURE_MODE)==_EXPOSURE_MODE_CALIB_MODE_IA)){
        paginaCalibAnalogic->selectTube(pConfig->userCnf.tubeFileName);
    }

    // Rilettura file di configurazione del Tubo
    QString tubeDir = QString(_TUBEPATH) + QString("/") + pConfig->userCnf.tubeFileName + QString("/");

    if(pGeneratore->openTube(tubeDir)==FALSE)
    {
        emit  consoleTxHandler(answer->cmdToQByteArray("NOK"));
        return;
    }
    else
    {
        emit  consoleTxHandler(answer->cmdToQByteArray("OK"));
        return;
    }


}

/*
 *  Funzione per salvare i dati in un file di configurazione con nome.
 *  ATTENZIONE: il contenuto del Tubo di pari nome verr√  cancellato e sostituito
 *
 *  PARAMETRI: PAR0 = Nome Tubo
 *  FRAME DI RISPOSTA:     <ID LEN %OK/NOK%>
 */
void console::handleStoreTube(QString nomeTubo, protoConsole* answer)
{
    if(nomeTubo.contains("TEMPLATE")){
        emit  consoleTxHandler(answer->cmdToQByteArray("NOK 2"));
        return;
    }
    if(pGeneratore->saveTube(nomeTubo)==TRUE)
    {
        emit  consoleTxHandler(answer->cmdToQByteArray("OK"));
        return;
    }
    else
    {
        emit  consoleTxHandler(answer->cmdToQByteArray("NOK 3"));
        return;
    }

}

/*
 *  Funzione per salvare i dati nel file Tubo correntemente selezionato.
 *  ATTENZIONE: il contenuto del Tubo verr√  cancellato e sostituito (rigenerato)
 *
 *  La funzione richiama la versione con il parametro <nome file> passandogli il
 *  il nome del Tubo configurato in sysCfg.cnf
 *  PARAMETRI: -
 *  FRAME DI RISPOSTA:     <ID LEN %OK/NOK%>
 */
void console::handleStoreTube(protoConsole* answer)
{
    handleStoreTube(pConfig->userCnf.tubeFileName, answer);
    return;
}

/*  <ID LEN %GetKvInfo nome_file%>
 *  Funzione per richiedere info sulla calibrazione corrente dei Kv
 *
    Parametri: NESSUNO
    Risposta:    <ID LEN %OK PAR0 PAR1 .. PAR30 %>
    PARAMETRI	Tipo    dato        Valore
    PAR0        String	Nome Tubo	Tubo correntemente selezionato
    PAR1        String  Anodo       Anodo utilizzato per le sequenze

    PAR2        Int     VDAC(20kV)	Se il valore √® 0 significa che il KVI non √® attivo
    PAR3        Int     VDAC(21kV)	"
    ...			"
    PAR31       Int     VDAC(49kV)	"
 *
 */
void console::handleGetKvInfo(protoConsole* answer)
{
    int i;



    answer->addParam(pGeneratore->getKvCalibAnode()); // Aggiunge l'anodo che verr√  utilizzato

    // Carica i dati richiesti
    for(i=0; i<_MAX_KV-_MIN_KV+1; i++)
    {
        if(pGeneratore->tube[i].vRef.enable)
            answer->addParam(QString("%1").arg(pGeneratore->tube[i].vRef.Vdac));
        else
            answer->addParam(QString("0"));
    }


    // Ricarica in memoria quelli originali al momento selezionati
    emit  consoleTxHandler(answer->cmdToQByteArray("OK"));
    return;
}

/* ATTENZIONE: QUESTA FUNZIONE E' ATTIVA SOLO IN CALIBRAZIONE KV!!
 * <ID LEN %SetKvVdacData PAR0 .. PAR29%>
 * Funzione per l'impostazionedei dati di calibrazione kV
 * in memoria, per il Tubo corrente
 *
    PARAMETRI:	Tipo dato	Valore      Note
    PAR0        Int         VDAC(20kV)	Se il valore √® 0 significa che il KVI non √® attivo
    ...			"
    PAR29       Int         VDAC(49kV)	"

    Frame di risposta:    <ID LEN %OK/NOK%>
 *
 */
void console::handleSetKvVdacData(protoConsole* frame, protoConsole* answer)
{
    int i;
    int val;
    int prev=0;


    // Prima di modificare il contenuto della memoria
    // vengono validati i dati ma solo per i kv consentiti
    for(i=0; i<_MAX_KV-_MIN_KV+1; i++)
    {
        val=frame->parametri[i].toInt();

        // Il valore massimo √® 4095
        if(val>4095) val=0; // Annulla per valori eccessivi

        // I valori di calibrazione devono essere crescenti (oppure 0)
        if((val!=0))
        {
            if(val<=prev)
            {
                emit  consoleTxHandler(answer->cmdToQByteArray(QString("NOK %1").arg(i)));
                return;
            }
            prev = val;
        }
    }

    // Procede con il salvataggio in memoria dei nuovi dati
    for(i=0; i<_MAX_KV-_MIN_KV+1; i++)
    {
        val=frame->parametri[i].toInt();
        if(val==0) pGeneratore->tube[i].vRef.enable = FALSE;
        pGeneratore->tube[i].vRef.Vdac = val;
    }
    emit  consoleTxHandler(answer->cmdToQByteArray("OK 0"));
    return;
}

/*
 * <ID LEN %SetKvMonitorData PAR0 .. PAR29%>
 * Funzione per l'impostazionedei dati di calibrazione della lettura dei kV
 *
 *
    PARAMETRI:	Tipo dato	Valore      Note
    PAR0        Int         VDAC(20kV)	Se il valore √® 0 significa che il valore corrente non va modificato
    ...			"
    PAR29       Int         VDAC(49kV)	"

    Frame di risposta:    <ID LEN %OK/NOK%>
 *  OBSOLETO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */
void console::handleSetKvMonitorData(protoConsole* frame, protoConsole* answer)
{
    int i;
    int val;
    int prev=0;

    // Solo in calibrazione KV √® consentito
    if(xSequence.workingMode!=_EXPOSURE_MODE_CALIB_MODE_KV)
    {
        emit  consoleTxHandler(answer->cmdToQByteArray("NOK"));
        return;
    }

    // Prima di modificare il contenuto della memoria
    // vengono validati i dati.
    for(i=0; i<_MAX_KV-_MIN_KV+1; i++)
    {
        val=frame->parametri[i].toInt();

        // Il valore massimo √® 4095
        if(val>4095) val=0;

        // I valori di calibrazione devono essere crescenti (oppure 0)
        if((val!=0))
        {
            if(val<=prev)
            {
                emit  consoleTxHandler(answer->cmdToQByteArray("NOK"));
                return;
            }
            prev = val;
        }
    }

    // Procede con il salvataggio in memoria dei nuovi dati
    for(i=0; i<_MAX_KV-_MIN_KV+1; i++)
    {
        val=frame->parametri[i].toInt();
        if(val==0) continue;
        pGeneratore->tube[i].vRef.Vread = val;
    }

    emit  consoleTxHandler(answer->cmdToQByteArray("OK"));
    return;
}


/*
 *  Questo comando permette di calibrare la rilettura dei KV
 *  il comando deve essere utilizzato solo durante la calibrazione dei KV
 *  param[0] = k;
 *  param[1] = cost;
 *  kV = RAW * k + cost
 */
void console::handleSetCalibKvRead(protoConsole* frame, protoConsole* answer)
{


    // Imposta i nuovi coefficienti
    pGeneratore->genCnf.pcb190.kV_CALIB = (unsigned short) (frame->parametri[0].toFloat() * 1000);
    pGeneratore->genCnf.pcb190.kV_OFS = (short) (frame->parametri[1].toFloat() * 1000);

    // Salva i coefficienti
    pGeneratore->saveTubeKvReadCalibrationFile();

    // Aggiorna la configurazione
    pConfig->updatePCB190();

    emit  consoleTxHandler(answer->cmdToQByteArray("OK"));
    return;
}

/*_____________________________________________________________________________________________
 *
 *          IMPOSTAZIONE DATI PER CALIBRAZIONE AIR KERMA
 *  PARAM
 *  - FILTRO: Mo/Rh
 *  - KV
 *  - mAs
 *____________________________________________________________________________________________*/
bool console::handleSetAnalogKermaCalibData(protoConsole* frame, protoConsole* answ)
{

    // Sezione dedicata alla macchina Analogica
    if(ApplicationDatabase.getDataU(_DB_EXPOSURE_MODE)!=_EXPOSURE_MODE_CALIB_MODE_KERMA) {
        emit consoleTxHandler( answ->answToQByteArray("NOK 2 INVALID-CALIB-MODE"));
        return false;
    }

    // Il fuoco da selezionare per l'esposizione di calibrazione Kerma Ë sempre il fuoco grande
    paginaCalibAnalogic->pc_selected_fuoco = Generatore::FUOCO_LARGE;

    // Selezione filtro
    if(frame->parametri[0]=="Mo") paginaCalibAnalogic->pc_selected_filtro = Collimatore::FILTRO_Mo;
    else paginaCalibAnalogic->pc_selected_filtro = Collimatore::FILTRO_Rh;

    // Selezione kV
    paginaCalibAnalogic->pc_selected_kV = frame->parametri[1].toInt();
    paginaCalibAnalogic->pc_selected_mAs = frame->parametri[2].toInt();
    emit consoleTxHandler( answ->answToQByteArray("OK 0"));

    paginaCalibAnalogic->pc_data_valid = true;
    paginaCalibAnalogic->setTubeData();

    return true;
}

/*_____________________________________________________________________________________________
 *
 *          IMPOSTAZIONE DATI PER CALIBRAZIONE TUBO MACCHINA ANALOGICA
 *          KV
 _____________________________________________________________________________________________*/
bool console::handleSetAnalogKvCalibTubeData(protoConsole* frame, protoConsole* answ)
{

    // Sezione dedicata alla macchina Analogica
    if(ApplicationDatabase.getDataU(_DB_EXPOSURE_MODE)!=_EXPOSURE_MODE_CALIB_MODE_KV) {
        emit consoleTxHandler( answ->answToQByteArray("NOK 2 INVALID-CALIB-MODE"));
        return false;
    }

    // Il fuoco da selezionare per l'esposizione di calibrazione kV Ë sempre il fuoco grande
    paginaCalibAnalogic->pc_selected_fuoco = Generatore::FUOCO_LARGE;

    // Selezione filtro
    if(frame->parametri[0]=="Mo") paginaCalibAnalogic->pc_selected_filtro = Collimatore::FILTRO_Mo;
    else paginaCalibAnalogic->pc_selected_filtro = Collimatore::FILTRO_Rh;
    paginaCalibAnalogic->pc_selected_kV = frame->parametri[1].toInt();
    if((paginaCalibAnalogic->pc_selected_kV<_MIN_KV) ||(paginaCalibAnalogic->pc_selected_kV>_MAX_KV)) {
        emit consoleTxHandler( answ->answToQByteArray("NOK 4 INVALID-KV-VALUE"));
        return false;
    }
    // Valore atteso tensione dac e atteso
    paginaCalibAnalogic->pc_selected_vdac = frame->parametri[2].toInt();
    if((paginaCalibAnalogic->pc_selected_vdac == 0) || (paginaCalibAnalogic->pc_selected_vdac > 4095) ) {
        emit consoleTxHandler( answ->answToQByteArray("NOK 3 INVALID-VDAC-VALUE"));
        return false;
    }

    // BUG-KV
    paginaCalibAnalogic->pc_selected_mAs = frame->parametri[3].toInt();
    if(paginaCalibAnalogic->pc_selected_mAs > 400) {
        emit consoleTxHandler( answ->answToQByteArray("NOK 5 INVALID-MAS-VALUE"));
        return false;
    }
    // Si seleziona sempre il valore pi˘ piccolo della corrente relativa (e solo per il fuoco grande!)
    if(pGeneratore->getIdacForKvCalibration(paginaCalibAnalogic->pc_selected_kV, pGeneratore->selectedAnodo,
                                            &paginaCalibAnalogic->pc_selected_Idac,
                                            &paginaCalibAnalogic->pc_selected_Ia)==false) return false;

    emit consoleTxHandler( answ->answToQByteArray("OK 0"));

    paginaCalibAnalogic->pc_data_valid = true;
    paginaCalibAnalogic->setTubeData();

    return true;
}


bool console::handleSetAnalogCalibTubeOffset(protoConsole* frame, protoConsole* answ){
    if(frame->parametri.size()!=7){
        emit consoleTxHandler( answ->answToQByteArray("NOK 1 INVALID-PARAMETERS"));
        return false;
    }

    int KVI =  frame->parametri[0].toInt()-20;
    int in_fg =  frame->parametri[1].toInt();
    int ic_fg =  frame->parametri[2].toInt();
    int idac_fg =  frame->parametri[3].toInt();
    int in_fp =  frame->parametri[4].toInt();
    int ic_fp =  frame->parametri[5].toInt();
    int idac_fp =  frame->parametri[6].toInt();

    if(idac_fg > pGeneratore->genCnf.pcb190.IFIL_MAX_SET){
        emit consoleTxHandler( answ->answToQByteArray("NOK 2 INVALID-IDAC-FG"));
        return false;
    }

    if(idac_fp > pGeneratore->genCnf.pcb190.IFIL_MAX_SET){
        emit consoleTxHandler( answ->answToQByteArray("NOK 3 INVALID-IDAC-FP"));
        return false;
    }

    // Applica offset al tubo selezionato e lo salva
    // Blocco dati WG
    QString Anodo=pGeneratore->confF1;

    // Calcolo differenza fuoco grande:
    int dacdif=0;
    int i;
    for(i=0; i<pGeneratore->tube[KVI].iTab.count(); i++)
    {
        if((pGeneratore->tube[KVI].iTab[i].anode==Anodo) && (pGeneratore->tube[KVI].iTab[i].In ==in_fg) && (pGeneratore->tube[KVI].iTab[i].fsize == Generatore::FUOCO_LARGE))
        {
            dacdif = idac_fg - pGeneratore->tube[KVI].iTab[i].Idac;
            pGeneratore->tube[KVI].iTab[i].INcalib = ic_fg;
        }
    }


    for(int kv=0; kv<=20;kv++)
        for(i=0; i<pGeneratore->tube[kv].iTab.count(); i++)
        {
            if((pGeneratore->tube[kv].iTab[i].anode==Anodo) && (pGeneratore->tube[kv].iTab[i].In !=0) && (pGeneratore->tube[kv].iTab[i].fsize == Generatore::FUOCO_LARGE))
            {
                if(pGeneratore->tube[kv].iTab[i].Idac + dacdif <= pGeneratore->genCnf.pcb190.IFIL_MAX_SET){
                    pGeneratore->tube[kv].iTab[i].Idac += dacdif;
                }
            }
        }

    // Calcolo differenza fuoco piccolo:
    dacdif=0;
    for(i=0; i<pGeneratore->tube[KVI].iTab.count(); i++)
    {
        if((pGeneratore->tube[KVI].iTab[i].anode==Anodo) && (pGeneratore->tube[KVI].iTab[i].In ==in_fp) && (pGeneratore->tube[KVI].iTab[i].fsize == Generatore::FUOCO_SMALL))
        {
            dacdif = idac_fp - pGeneratore->tube[KVI].iTab[i].Idac;
            pGeneratore->tube[KVI].iTab[i].INcalib = ic_fp;
        }
    }


    for(int kv=0; kv<=20;kv++)
        for(i=0; i<pGeneratore->tube[kv].iTab.count(); i++)
        {
            if((pGeneratore->tube[kv].iTab[i].anode==Anodo) && (pGeneratore->tube[kv].iTab[i].In !=0) && (pGeneratore->tube[kv].iTab[i].fsize == Generatore::FUOCO_SMALL))
            {
                if(pGeneratore->tube[kv].iTab[i].Idac + dacdif <= pGeneratore->genCnf.pcb190.IFIL_MAX_SET){
                    pGeneratore->tube[kv].iTab[i].Idac += dacdif;

                }
            }
        }


    // Store del Tubo
    pGeneratore->saveTube(pConfig->userCnf.tubeFileName);

    // Salvataggio dati tubo modificati
    emit consoleTxHandler( answ->answToQByteArray("OK 0"));
    return true;

}

/*_____________________________________________________________________________________________
 *
 *          IMPOSTAZIONE DATI PER CALIBRAZIONE TUBO MACCHINA ANALOGICA
 *          CORRENTE ANODICA
 _____________________________________________________________________________________________*/
bool console::handleSetAnalogIaCalibTubeData(protoConsole* frame, protoConsole* answ)
{

    paginaCalibAnalogic->pc_data_valid = false;

    // Sezione dedicata alla macchina Analogica
    if(ApplicationDatabase.getDataU(_DB_EXPOSURE_MODE)!=_EXPOSURE_MODE_CALIB_MODE_IA) {
        emit consoleTxHandler( answ->answToQByteArray("NOK 2 INVALID-CALIB-MODE"));
        return false;
    }

    if(frame->parametri[0]=="L") paginaCalibAnalogic->pc_selected_fuoco = Generatore::FUOCO_LARGE;
    else  paginaCalibAnalogic->pc_selected_fuoco = Generatore::FUOCO_SMALL;

    // Selezione filtro
    paginaCalibAnalogic->pc_selected_filtro = pConfig->analogCnf.primo_filtro;

    // Valore atteso tensione dac e atteso
    unsigned char KV;
    unsigned short KVDAC;
    pGeneratore->getValidKv((float)frame->parametri[1].toInt(), &KV , &KVDAC );
    paginaCalibAnalogic->pc_selected_kV = KV;
    paginaCalibAnalogic->pc_selected_vdac = KVDAC;

    // Corrente nominale
    paginaCalibAnalogic->pc_selected_Ia = frame->parametri[2].toInt();

    // Corrente Idac
    paginaCalibAnalogic->pc_selected_Idac = frame->parametri[3].toInt();
    if((paginaCalibAnalogic->pc_selected_Idac > pGeneratore->genCnf.pcb190.IFIL_MAX_SET)||
       (paginaCalibAnalogic->pc_selected_Idac < pGeneratore->genCnf.filData.IFILdac)) {
        emit consoleTxHandler( answ->answToQByteArray("NOK 5 INVALID-IDAC-VALUE"));
        return false;
    }

    // mAs esposizione
    paginaCalibAnalogic->pc_selected_mAs = frame->parametri[4].toInt();
    if(paginaCalibAnalogic->pc_selected_mAs > 400) {
        emit consoleTxHandler( answ->answToQByteArray("NOK 6 INVALID-MAS-VALUE"));
        return false;
    }
    emit consoleTxHandler( answ->answToQByteArray("OK 0"));

    paginaCalibAnalogic->pc_data_valid = true;
    paginaCalibAnalogic->setTubeData();

    return true;

}

/*
 * <ID LEN %SetIaCalibMode%>
 * Funzione per l'attivazione della modalit√  di calibrazione
 * della corrente Anodica
 *
 *  PARAMETRI:	Nessuno
 *  Frame di risposta:    <ID LEN %OK/NOK%>
 *
 */
bool console::handleSetIaCalibMode(void)
{
    // Impostazione modalit√  OPERATIVA
    xSequence.workingMode = _EXPOSURE_MODE_CALIB_MODE_IA;
    ApplicationDatabase.setData(_DB_EXPOSURE_MODE,(unsigned char) xSequence.workingMode);

    // Accende simbolo di calibrazione in corso
    // <TBD> impostare il simbolo relativo alla calibrazione in corso
    //pagina0->setCalibOn(TRUE);

    // Imposta la Modalit√  di funzionamento del generatore
    pGeneratore->tomoMode=FALSE;

    // Annulla eventuali allarmi PAD
    PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_PAD,0);

    return TRUE;
}

/*
 *  Stringa di comando	<ID LEN %GetIaInfo PAR0%>
    PARAMETRI	Tipo	Valore          Note
    PAR0        Int     KVI             Correnti associate a KVI

    Frame di risposta: <ID LEN %OK PAR0 [BLOCCO WG] [BLOCCO WP] [BLOCCO MoG] [BLOCCO MoP] %>
    PARAMETRI	Tipo        Valore      Note
    PAR0        String      Nome Tubo	Tubo correntemente selezionato
        [BLOCCO DATI RELATIVI AL FUOCO WG]
    ...         String      "WG"        Tag di riconoscimento sezione
    ...         Int         N-Correnti	Numero totale correnti da calibrare per WG
                                        Se 0, passare alla sezione successiva
    ...         Int         IAN-1       Valore nominale corrente [1:200]
    ...         Int         IANC-1      Valore  calibrato [1:200]
    ...         Int         IDAC-1      Valore da calibrare [1:4095]
                Float       Derivata
    ...         Int         IAN-2       Valore nominale corrente [1:200]
    ...         Int         IANC-2      Valore  calibrato [1:200]
    ...         Int         IDAC-2      Valore da calibrare [1:4095]
    ...         Int         IAN-...n	Valore nominale corrente [1:200]
    ...         Int         IANC-n      Valore  calibrato [1:200]
    ...         Int         IDAC-...n	Valore da calibrare [1:4095]
        [BLOCCO DATI RELATIVI AL FUOCO WP]
    ...         String      "WP"        Tag di riconoscimento sezione
    ...
        [BLOCCO DATI RELATIVI AL FUOCO Mo]
    ...         String      "MoG"       Tag di riconoscimento sezione
    ...
        [BLOCCO DATI RELATIVI AL FUOCO MoP]
    ...         String      "MoP"       Tag di riconoscimento sezione
    ...
    ______________________________________________________________________________
    La funzione restituisce tutto il set di correnti da calibrare per ogni
    fuoco configurato nel tubo in esame
 *
 */
void console::handleGetIaInfo(protoConsole* frame, protoConsole* answer)
{
    int i,ii,n;
    if(frame->parametri.size()!=1){
        emit  consoleTxHandler(answer->cmdToQByteArray("NOK 1"));
        return;
    }

    int KVI = frame->parametri[0].toInt() -_MIN_KV;
    if((KVI>_MAX_KV-_MIN_KV)||(KVI<0))
    {
        emit  consoleTxHandler(answer->cmdToQByteArray("NOK 3"));
        return;
    }

    // Blocco dati WG
    answer->addParam(QString("WG"));
    n=0;
    ii = answer->parametri.size();
    answer->addParam(QString("%1").arg(n));
    for(i=0; i<pGeneratore->tube[KVI].iTab.count(); i++)
    {
        if((pGeneratore->tube[KVI].iTab[i].anode=="W") && (pGeneratore->tube[KVI].iTab[i].fsize == Generatore::FUOCO_LARGE))
        {
            n++;
            answer->addParam(QString("%1").arg(pGeneratore->tube[KVI].iTab[i].In));
            answer->addParam(QString("%1").arg(pGeneratore->tube[KVI].iTab[i].INcalib));
            answer->addParam(QString("%1").arg(pGeneratore->tube[KVI].iTab[i].Idac));
            answer->addParam(QString("%1").arg(pGeneratore->tube[KVI].iTab[i].derivata));
        }
    }
    answer->parametri[ii] = QString("%1").arg(n);

    // Blocco dati WP
    answer->addParam(QString("WP"));
    n=0;
    ii = answer->parametri.size();
    answer->addParam(QString("%1").arg(n));
    for(i=0; i<pGeneratore->tube[KVI].iTab.count(); i++)
    {
        if((pGeneratore->tube[KVI].iTab[i].anode=="W") && (pGeneratore->tube[KVI].iTab[i].fsize == Generatore::FUOCO_SMALL))
        {
            n++;
            answer->addParam(QString("%1").arg(pGeneratore->tube[KVI].iTab[i].In));
            answer->addParam(QString("%1").arg(pGeneratore->tube[KVI].iTab[i].INcalib));
            answer->addParam(QString("%1").arg(pGeneratore->tube[KVI].iTab[i].Idac));
            answer->addParam(QString("%1").arg(pGeneratore->tube[KVI].iTab[i].derivata));
        }
    }
    answer->parametri[ii] = QString("%1").arg(n);

    // Blocco dati MoG
    answer->addParam(QString("MoG"));
    n=0;
    ii = answer->parametri.size();
    answer->addParam(QString("%1").arg(n));
    for(i=0; i<pGeneratore->tube[KVI].iTab.count(); i++)
    {
        if((pGeneratore->tube[KVI].iTab[i].anode=="Mo") && (pGeneratore->tube[KVI].iTab[i].fsize == Generatore::FUOCO_LARGE))
        {
            n++;
            answer->addParam(QString("%1").arg(pGeneratore->tube[KVI].iTab[i].In));
            answer->addParam(QString("%1").arg(pGeneratore->tube[KVI].iTab[i].INcalib));
            answer->addParam(QString("%1").arg(pGeneratore->tube[KVI].iTab[i].Idac));
            answer->addParam(QString("%1").arg(pGeneratore->tube[KVI].iTab[i].derivata));
        }
    }
    answer->parametri[ii] = QString("%1").arg(n);

    // Blocco dati MoP
    answer->addParam(QString("MoP"));
    n=0;
    ii = answer->parametri.size();
    answer->addParam(QString("%1").arg(n));
    for(i=0; i<pGeneratore->tube[KVI].iTab.count(); i++)
    {
        if((pGeneratore->tube[KVI].iTab[i].anode=="Mo") && (pGeneratore->tube[KVI].iTab[i].fsize == Generatore::FUOCO_SMALL))
        {
            n++;
            answer->addParam(QString("%1").arg(pGeneratore->tube[KVI].iTab[i].In));
            answer->addParam(QString("%1").arg(pGeneratore->tube[KVI].iTab[i].INcalib));
            answer->addParam(QString("%1").arg(pGeneratore->tube[KVI].iTab[i].Idac));
            answer->addParam(QString("%1").arg(pGeneratore->tube[KVI].iTab[i].derivata));
        }
    }
    answer->parametri[ii] = QString("%1").arg(n);

    emit  consoleTxHandler(answer->cmdToQByteArray("OK"));
    return;
}

/*
 *  Stringa di comando:	<ID LEN %GetIaTomoInfo%>
    PARAMETRI:	Nessuno
    Frame di risposta: <ID LEN %OK PAR0 [BLOCCO WG] [BLOCCO MoG]%>
    PARAMETRI:      Tipo            Valore          Note
    PAR0            String          Nome Tubo       Tubo correntemente selezionato
        [BLOCCO DATI RELATIVI AL FUOCO WG]
    PAR1            String          "WG"            Tag di riconoscimento sezione
    ...             Int              N              Numero correnti definite (coppie di parametri)
                                                    - 0: il fuoco non √® configurato
    ...             Int             IAN(20KV)       Valore nominale corrente [1:200] per 20KV
    ...             Int             IDAC(20KV)      Valore da calibrare [1:4095] per 49KV
                    Float           Derivata
    ...             ...             ...
    ...             Int             IAN(49KV)       Valore nominale corrente [1:200] per 49KV
    ...             Int             IDAC(49KV)      Valore da calibrare [1:4095] per 49KV

        [BLOCCO DATI RELATIVI AL FUOCO MoG]
    ...             String          "MoG"           Tag di riconoscimento sezione
    ...             Int              N              Numero correnti definite (coppie di parametri)
                                                    - 0: il fuoco non √® configurato
    ...             Int             IAN(20KV)       Valore nominale corrente [1:200] per 20KV
    ...             Int             IDAC(20KV)      Valore da calibrare [1:4095] per 49KV
    ...             ...             ...
    ...             Int             IAN(49KV)       Valore nominale corrente [1:200] per 49KV
    ...             Int             IDAC(49KV)      Valore da calibrare [1:4095] per 49KV

    ______________________________________________________________________________
    La funzione restituisce tutto il set di correnti da calibrare per le scansioni
    Tomografiche

 *
 */
void console::handleGetIaTomoInfo(protoConsole* answer)
{
    int i,ii,n;

    // Non Ë possibile effettuare la Tomo senza starter
    if(pConfig->sys.highSpeedStarter == FALSE)  {
        answer->addParam(QString("WG"));
        n=0;
        answer->addParam(QString("%1").arg(n));
        answer->addParam(QString("MoG"));
        answer->addParam(QString("%1").arg(n));
        emit  consoleTxHandler(answer->cmdToQByteArray("OK"));
        return;
    }


    // Blocco dati WG
    answer->addParam(QString("WG"));
    n=0;
    if((pGeneratore->confF1=="W")||(pGeneratore->confF2=="W"))
    {
        n = _MAX_KV-_MIN_KV +1;
        answer->addParam(QString("%1").arg(n));

        for(i=0; i<n; i++)
        {
            if(pGeneratore->tube[i].vRef.enable)
            {
                for(ii=0; ii<pGeneratore->tube[i].tomo.count();ii++)
                {
                    if((pGeneratore->tube[i].tomo[ii].anode=="W") && (pGeneratore->tube[i].tomo[ii].enabled))
                    {
                        answer->addParam(QString("%1").arg(pGeneratore->tube[i].tomo[ii].In));
                        answer->addParam(QString("%1").arg(pGeneratore->tube[i].tomo[ii].INcalib));
                        answer->addParam(QString("%1").arg(pGeneratore->tube[i].tomo[ii].Idac));
                        answer->addParam(QString("%1").arg(pGeneratore->tube[i].tomo[ii].derivata));
                        break;
                    }
                }
                if(ii==pGeneratore->tube[i].tomo.count())
                {
                    answer->addParam(QString("%1").arg((int) 0));
                    answer->addParam(QString("%1").arg((int) 0));
                    answer->addParam(QString("%1").arg((int) 0));
                    answer->addParam(QString("%1").arg((int) 0));
                }
            }else
            {   // KV non abilitati o calibrati
                answer->addParam(QString("%1").arg((int) 0));
                answer->addParam(QString("%1").arg((int) 0));
                answer->addParam(QString("%1").arg((int) 0));
                answer->addParam(QString("%1").arg((int) 0));
            }
        }
    }else  answer->addParam(QString("%1").arg(n));


    // Blocco dati MoG
    answer->addParam(QString("MoG"));
    n=0;
    if((pGeneratore->confF1=="Mo")||(pGeneratore->confF2=="Mo"))
    {
        n = _MAX_KV-_MIN_KV +1;
        answer->addParam(QString("%1").arg(n));

        for(i=0; i<n; i++)
        {
            if(pGeneratore->tube[i].vRef.enable)
            {
                for(ii=0; ii<pGeneratore->tube[i].tomo.count();ii++)
                {
                    if((pGeneratore->tube[i].tomo[ii].anode=="Mo") && (pGeneratore->tube[i].tomo[ii].enabled))
                    {
                        answer->addParam(QString("%1").arg(pGeneratore->tube[i].tomo[ii].In));
                        answer->addParam(QString("%1").arg(pGeneratore->tube[i].tomo[ii].INcalib));
                        answer->addParam(QString("%1").arg(pGeneratore->tube[i].tomo[ii].Idac));
                        answer->addParam(QString("%1").arg(pGeneratore->tube[i].tomo[ii].derivata));
                        break;
                    }
                }
                if(ii==pGeneratore->tube[i].tomo.count())
                {
                    answer->addParam(QString("%1").arg((int) 0));
                    answer->addParam(QString("%1").arg((int) 0));
                }
            }else
            {   // KV non abilitati o calibrati
                answer->addParam(QString("%1").arg((int) 0));
                answer->addParam(QString("%1").arg((int) 0));
            }
        }
    }else  answer->addParam(QString("%1").arg(n));


    emit  consoleTxHandler(answer->cmdToQByteArray("OK"));
    return;

}

/*
 *  Stringa di comando:	<ID LEN %SetIdacData  PAR0 [WG][WP][MoG][MoP]%>
    PARAMETRI:          Tipo    	Valore              Note
    PAR0                Int         KVI                 KVI a cui i dati si riferiscono
    PAR1                String      "WG/WP/MoG/MoP"     Tag di riconoscimento fuoco da modificare
    ...                 Int         N                   Numero totale correnti da calibrare
                                                        0 se nessuna corrente √® presente su questo fuoco
    ...                 Int         IAN-1               Valore nominale corrente [1:200]
    ...                 Int         IANC-1           Valore nominale corrente [1:200]

    ...                 Int         IDAC-1          Valore da calibrare [1:4095]
    ...
    ...                 Int         IAN-...n        Valore nominale corrente [1:200]
    ...                 Int         IDAC-...n       Valore da calibrare [1:4095]


    Frame di risposta:  <ID LEN %OK/NOK%>
    ______________________________________________________________________________
    La funzione modifica il contenuto dei valori di calibrazione delle correnti
    utilizzate.
    La CPU effettua un controllo sulla coerenza dei dati: la funzione infatti
    non pu√≤ modificare i valori nominali e gli stessi devono corrispondere
    a quelli tabellati in memoria.

 */
bool console::handleSetIdacData(protoConsole* frame,  bool modifica)
{
    int i,ii,iii,n;
    int in,idac,ine;
    QString AnodoFuoco;
    QString anodo;
    unsigned char fsize;

    // Controllo sui parametri
    if(frame->parametri.size()<3) return FALSE;

    // Legge i KV corrispondenti alla modifica
    int KVI = frame->parametri[0].toInt() -_MIN_KV;
    if(KVI>_MAX_KV-_MIN_KV) return FALSE;

    i = 1;

    // Legge la combinazione Anodo/Fuoco
    AnodoFuoco = frame->parametri[i++];
    if(AnodoFuoco=="WG")
    {
        anodo = "W";
        fsize = Generatore::FUOCO_LARGE;

    }else if(AnodoFuoco=="WP")
    {
        anodo = "W";
        fsize = Generatore::FUOCO_SMALL;

    }else if(AnodoFuoco=="MoG")
    {
        anodo = "Mo";
        fsize = Generatore::FUOCO_LARGE;

    }else if(AnodoFuoco=="MoP")
    {
        anodo = "Mo";
        fsize = Generatore::FUOCO_SMALL;
    }else return FALSE;

    // Controllo sul numero dei parametri aspettati
    n = frame->parametri[i++].toInt();
    if(frame->parametri.size()!= 3 + n*3) return FALSE;

    if(n!=0)
    {
        // Passa tutte le correnti richieste
        for(iii=0; iii<n; iii++)
        {
            // Legge la coppia di correnti
           in = frame->parametri[i++].toInt();
           ine = frame->parametri[i++].toInt();
           idac = frame->parametri[i++].toInt();

           // Controlla valori
           if(in>200) return FALSE;
           if(idac>4095) return FALSE;

           // Cerca la corrispondente In da calibrare
           for(ii=0; ii<pGeneratore->tube[KVI].iTab.count();ii++)
               if((pGeneratore->tube[KVI].iTab[ii].anode==anodo)&&(pGeneratore->tube[KVI].iTab[ii].fsize == fsize))
                   if(pGeneratore->tube[KVI].iTab[ii].In == in) break;
           if(ii==pGeneratore->tube[KVI].iTab.count()) return FALSE; // Nessuna (In) corrispondente
           if(modifica)
           {
               // Procede con la modifica
               pGeneratore->tube[KVI].iTab[ii].In = in;
               pGeneratore->tube[KVI].iTab[ii].Idac = idac;
               pGeneratore->tube[KVI].iTab[ii].INcalib = ine;
           }
        }
    }


    return TRUE;
}

/*
    COMANDO: SetIdacTomoData  (solo in calibrazione IA)
    Frame di risposta: <ID LEN %OK/NOK%>
        OK	Dato corretto
        NOK	Errore formato comando

*/
bool console::handleSetIdacTomoData(protoConsole* frame, protoConsole* answer)
{
    int kv,parOffset, i, ii;
    int In,Idac,Ine;
    bool check;
    QString anodoStr;


    // Controllo sulla lunghezza dei parametri
    if(frame->parametri.size()!=91) return FALSE;

    // Controlla quale anodo √® richiesto
    if((frame->parametri[0]!="W") && (frame->parametri[0]!="Mo")) return FALSE;

    // Controlla se l'anodo e' configurato
    if(!pGeneratore->isValidAnode(frame->parametri[0])) return FALSE;

    // Controllo parametri
    anodoStr=frame->parametri[0];
    parOffset = 1;
    for(i=0; i<=_MAX_KV-_MIN_KV; i++)
    {
        In = frame->parametri[parOffset + i*3].toInt();
        Ine = frame->parametri[parOffset + i*3+1].toInt();
        Idac = frame->parametri[parOffset + i*3+2].toInt();
        if(In==0) continue;
        if(Idac==0) return FALSE;
        if(Idac>4095) return FALSE;

        // Controllo che In sia configurato per il tubo in oggetto
        check = FALSE;
        for(ii=0; ii<pGeneratore->tube[i].tomo.count();ii++)
        {
            if((pGeneratore->tube[i].tomo[ii].anode==anodoStr)&&(pGeneratore->tube[i].tomo[ii].In == In))
            {
                check=TRUE;
                break;
            }
        }
        if(!check) return FALSE;
    }

    // Modifica parametri
    for(i=0; i<=_MAX_KV-_MIN_KV; i++)
    {
        In = frame->parametri[parOffset + i*3].toInt();
        Ine = frame->parametri[parOffset + i*3+1].toInt();
        Idac = frame->parametri[parOffset + i*3+2].toInt();
        if(In==0) continue;

        // Cerca la corrispondente In da calibrare
        for(ii=0; ii<pGeneratore->tube[i].tomo.count();ii++)
        {
            if((pGeneratore->tube[i].tomo[ii].anode==anodoStr)&&(pGeneratore->tube[i].tomo[ii].In == In))
            {
                pGeneratore->tube[i].tomo[ii].Idac = Idac;
                pGeneratore->tube[i].tomo[ii].INcalib = Ine;
                pGeneratore->tube[i].tomo[ii].enabled = TRUE;
            }
        }
    }

    return TRUE;
}

/*
 *
 *  PAR0	Stringa	Anodo	"W"/"Mo" seleziona tungsteno/Molibdeno
    PAR1	Stringa	Dimensione fuoco	"G"/"P" seleziona fuoco grande/piccolo
    PAR2	Int	KVI	Indica i KVI nominali da utilizzare
    PAR3	Int	IDAC	[1:4095] valore da calibrare
    PAR4	Int	Inom	Valore corrente in mA attesa
    PAR5	Int	mAs	mAs richiesti dall'esposizione

      bool  validated; // I dati sono stati rinfrescati dall'ultima sequenza
      QString anodo;   // Anodo selezionato per la sequenza (W/Mo)
      QString fuoco;   // Filtro selezionato per la sequenza
      int   Vnom;      // Tensione nominale (intera) aspettata
      int   Idac;      // Corrente analogica da utilizzare
      int   Inom;      // Corrente nominale aspettata
      int   mAs;       // mAs da utilizzare durante la sequenza
    }_iACalibData_Str;

 *
*/
bool console::handleSetIaRxData(protoConsole* frame, protoConsole* answer)
{
    // In ogni caso toglie la validazione a quanto c'√® gi√  in memoria
    iACalibData.validated = FALSE;

    // Solo in calibrazione KV √® consentito
    if(xSequence.workingMode!=_EXPOSURE_MODE_CALIB_MODE_IA) return FALSE;

    // Imposta l'anodo usato per la sequenza e il fuoco grande
    if(!pGeneratore->isValidAnode(frame->parametri[0])) return FALSE;
    iACalibData.anodo = frame->parametri[0];

    // Imposta il fuoco da utilizzare
    if((frame->parametri[1]!="P") && (frame->parametri[1]!="G")) return FALSE;
    iACalibData.fuoco = frame->parametri[1];

    // Seleziona subito il fuoco per riscaldarlo
    if(frame->parametri[1]=="P")
    {
        pGeneratore->setFuoco(Generatore::FUOCO_SMALL);
        pGeneratore->setFuoco(iACalibData.anodo);
    }
    else
    {
        pGeneratore->setFuoco(Generatore::FUOCO_LARGE);
        pGeneratore->setFuoco(iACalibData.anodo);
    }
    if(pGeneratore->updateFuoco()==FALSE) return FALSE;

    // Valore nominale tensione: verifica che sia calibrato e disponibile
    if(pGeneratore->isValidKv(frame->parametri[2].toInt())==FALSE) return FALSE;
    iACalibData.Vnom = frame->parametri[2].toInt();

    // Valore analogico corrente anodica: verifica che il valore massimo non ecceda quello previsto.
    int idac = frame->parametri[3].toInt();
    if(idac > pGeneratore->genCnf.pcb190.IFIL_MAX_SET) return false;
    if(idac < pGeneratore->genCnf.filData.IFILdac) return false;
    iACalibData.Idac = idac;

    // Valore atteso corrente anodica
    iACalibData.Inom = frame->parametri[4].toInt();
    if((iACalibData.Inom==0) ||(iACalibData.Inom>200)) return FALSE;

    // Valore mAs da utilizzare durante la sequenza
    iACalibData.mAs  = frame->parametri[5].toInt();

    // Modifica per la Tomo: occorrono almeno 20mAs
    if(iACalibData.mAs<20) iACalibData.mAs=20;
    if((iACalibData.mAs==0) ||(iACalibData.mAs>100)) return FALSE;

    // Dati validati e caricati in memoria
    iACalibData.validated = TRUE;

    return TRUE;
}




/*
    Stringa di comando	<ID LEN %handleSetBiopsyHome PAR0%>
    PARAMETRI:	Tipo    dato                        Valore
    PAR0        Int     Posizione base del cursore  (mm)

    Frame di risposta: <ID LEN %OK%>

 */
int console::handleSetBiopsyHome(protoConsole* frame)
{

    return 0;

}

/*
    Stringa di comando	<ID LEN %SetLingua PAR0%>
    PARAMETRI:	Tipo    dato                        Valore
    PAR0        String  Codice stringua lingua

    Frame di risposta:
        <ID LEN %OK 0%>  ->OK
        <ID LEN %NOK 1%>  ->Formato non corretto
        <ID LEN %NOK 2%>  ->Lingua non gestita
 */
int  console::handleSetLingua(protoConsole* frame){

    // Controllo sul formato del comando
    if(frame->parametri.size()!=1) return 1;
    if(pagina_language->isLanguage(frame->parametri[0])==FALSE) return 2;
    ApplicationDatabase.setData(_DB_LINGUA,frame->parametri[0], 0);
    pConfig->userCnf.languageInterface = frame->parametri[0];
    pConfig->saveUserCfg();

    return 0;

}


/*
 *  Comando di attivazione dell'aggiornamento generale del sistema
 */
void  console::handleSetFirmwareUpdate(protoConsole* frame, protoConsole* answer){

    QString errstr;

    // Risponde subito
    //emit consoleTxHandler(answer->answToQByteArray("OK 0"));

    // Usa la funzione diretta con timeout perch√® a seguire vi sono delle system
    consoleSocketTcp->txData(answer->answToQByteArray("OK 0"),100000);

    // Connessione del segnale di fine aggiornamento della funzione sysUpdate
    // con lo slot del modulo toConsole per l'invio del comando asincrono di fine
    // aggiornamento, comunque sia andato.
    connect(pConfig,SIGNAL(sysUpdateCompletedSgn(bool,QString)), pToConsole,SLOT(fineSystemUpdate(bool,QString)),Qt::UniqueConnection);
    connect(pConfig,SIGNAL(sysUpdateCompletedSgn(bool,QString)),pConfig,SLOT(localInstallMonitorCompleted(bool,QString)),Qt::UniqueConnection);
    pConfig->sysUpdate(frame->parametri[0]);
    return;

}


void  console::handleSetSystemBackup(protoConsole* frame, protoConsole* answer){

    // Risponde subito
    emit consoleTxHandler(answer->answToQByteArray("OK 255"));
    //consoleSocketTcp->txData(answer->answToQByteArray("OK 255"));
    pConfig->sysBackup(TRUE,frame->parametri[0],frame->id);
    return;

}

void  console::handleSetSystemRestore(protoConsole* frame, protoConsole* answer){

    emit consoleTxHandler(answer->answToQByteArray("OK 255"));
    pConfig->sysRestore(TRUE,frame->parametri[0],frame->id);
    return;

}

void  console::handleSetUserDirClean(protoConsole* frame, protoConsole* answer){

    // Risponde subito
    emit consoleTxHandler(answer->answToQByteArray("OK 255"));
    pConfig->userDirClean(TRUE,frame->id);
    return;

}
void console::protocolAnswer(protoConsole* answer, QString cmd){
    emit consoleTxHandler(answer->answToQByteArray(cmd));
}

void console::handleCompressorActivation(unsigned char mode){
    unsigned char data[5];

    if(mode==1){
        pGuiMcc->sendFrame(MCC_CMD_CMP_AUTO,1,data,0);
    }else{
        data[0]=100;
        pGuiMcc->sendFrame(MCC_CMD_PAD_UP,1,data,1);
    }

}

void console::handleOutputPulse(QString nout, QString time){

    io->setSpareOutputPulse(nout.toInt(), time.toLong());
    //io->setSpareOutputClr(nout.toInt());
}



//_____________________________________________________________
// Gestione attivazione segnale di raggi in corso
void console::handleSetXraySym(bool stat){
    if(stat) ApplicationDatabase.setData(_DB_XRAY_SYM,(unsigned char) 1,0);
    else ApplicationDatabase.setData(_DB_XRAY_SYM,(unsigned char) 0,0);
}

//______________________________________________________________________________________________________________________
void console::handlePowerOff(void){
    pConfig->executePoweroff(20);
    return;
}


//____________________________________________________________________________________________________________________
void  console::handleGetProfileList(protoConsole* frame, protoConsole* answer){
    QFileInfoList list = pGeneratore->pAECprofiles->getProfileList();
    int i;

    for(i=0; i<list.size(); i++){
        answer->addParam(list.at(i).baseName());
    }

    answer->addParam(QString("VL"));
    answer->addParam(QString("%1").arg(pGeneratore->pAECprofiles->validList.size()));
    for(int i=0; i<pGeneratore->pAECprofiles->validList.size(); i++)
        answer->addParam(pGeneratore->pAECprofiles->validList.at(i));

    emit consoleTxHandler(answer->answToQByteArray(QString("OK %1").arg(i)));
    return;
}

/*
 *  Upload del contenuto della lista di validazione
 */
void  console::handleGetValidList(protoConsole* frame, protoConsole* answer){
    int i;

    for(i=0; i<pGeneratore->pAECprofiles->validList.size(); i++){
        answer->addParam(pGeneratore->pAECprofiles->validList.at(i));
    }

    emit consoleTxHandler(answer->answToQByteArray(QString("OK %1").arg(i)));

    return;
}

/*
 *  Sostituisce una lista di validazione
 *  Se non ci sono elementi nella nuova lista allora verr‡ eliminata dal file system
 *  Ricarica in memoria i profili
 */
void  console::handleSetValidList(protoConsole* frame, protoConsole* answer){
    QList<QString> valid_list;
    emit consoleTxHandler(answer->answToQByteArray("OK 0"));

    // Si carica la lista disponibile
    QFileInfoList profile_list = pGeneratore->pAECprofiles->getProfileList();

    // Cancella la lista di validit‡ corrente
    pGeneratore->pAECprofiles->eraseValidList();

    for(int vdx=0; vdx< frame->parametri.size(); vdx++){
        if(frame->parametri[vdx]=="defaul_cr") continue;
        if(frame->parametri[vdx]=="defaul_film") continue;

        // Verifica il contenuto con i files presenti
        for(int i=0; i<profile_list.size(); i++){
            if(profile_list[i].baseName()==frame->parametri[vdx]){
                valid_list.append(frame->parametri[vdx]);
            }
        }
    }

    pGeneratore->pAECprofiles->saveValidList(valid_list);
    pGeneratore->pAECprofiles->openAECProfiles();

    return;
}


/*
 *  Upload del contenuto di un profilo presente nel file system
 *  Il profilo puÚ anche non essere presente nella lista di validazione
 */
void  console::handleGetProfile(protoConsole* frame, protoConsole* answer){

    if(frame->parametri.size()!=1){
        emit consoleTxHandler(answer->answToQByteArray("NOK 1 INVALID-PARAM"));
        return;
    }

    AEC::profileCnf_Str profile = pGeneratore->pAECprofiles->getProfile(frame->parametri[0]);
    if(profile.filename==""){
        emit consoleTxHandler(answer->answToQByteArray("NOK 2 INVALID-PROFILE"));
        return;
    }

    QString data = profile.symbolicName + " ";
    if(profile.plateType == ANALOG_PLATE_CR) data+="CR ";
    else data+= "FILM ";

    data+= QString("%1 %2 ").arg(profile.plog_threshold).arg(profile.plog_minimo);

    for(int i=0; i<11; i++)
        data+=QString("%1 ").arg(profile.od[i]);

    // Sequenza punti MoG
    data += QString("MoG %1 ").arg(profile.pulseStd_Mo_G.size());
    if(profile.pulseStd_Mo_G.size()){
        for(int i=0; i<profile.pulseStd_Mo_G.size(); i++){
            data += QString("%1 %2 %3 ").arg(profile.pulseStd_Mo_G[i].plog).arg(profile.pulseStd_Mo_G[i].pulse).arg(profile.pulseStd_Mo_G[i].pmmi);
        }
    }

    // Sequenza punti MoP
    data += QString("MoP %1 ").arg(profile.pulseStd_Mo_P.size());
    if(profile.pulseStd_Mo_P.size()){
        for(int i=0; i<profile.pulseStd_Mo_P.size(); i++){
            data += QString("%1 %2 %3 ").arg(profile.pulseStd_Mo_P[i].plog).arg(profile.pulseStd_Mo_P[i].pulse).arg(profile.pulseStd_Mo_P[i].pmmi);
        }
    }

    // Sequenza punti RhG
    data += QString("RhG %1 ").arg(profile.pulseStd_Rh_G.size());
    if(profile.pulseStd_Rh_G.size()){
        for(int i=0; i<profile.pulseStd_Rh_G.size(); i++){
            data += QString("%1 %2 %3 ").arg(profile.pulseStd_Rh_G[i].plog).arg(profile.pulseStd_Rh_G[i].pulse).arg(profile.pulseStd_Rh_G[i].pmmi);
        }
    }

    // Sequenza punti RhP
    data += QString("RhP %1 ").arg(profile.pulseStd_Rh_P.size());
    if(profile.pulseStd_Rh_P.size()){
        for(int i=0; i<profile.pulseStd_Rh_P.size(); i++){
            data += QString("%1 %2 %3 ").arg(profile.pulseStd_Rh_P[i].plog).arg(profile.pulseStd_Rh_P[i].pulse).arg(profile.pulseStd_Rh_P[i].pmmi);
        }
    }

    emit consoleTxHandler(answer->answToQByteArray("OK " + data));
    return;
}

void  console::handleSetProfileNote(protoConsole* frame, protoConsole* answer){
    AEC::profileCnf_Str profile = pGeneratore->pAECprofiles->getProfile(frame->parametri[0]);
    if(profile.filename==""){
        emit consoleTxHandler(answer->answToQByteArray("NOK 1 INVALID-PROFILE"));
        return;
    }

    if(frame->parametri.size()>1){
        profile.note.clear();
        for(int i=1; i< frame->parametri.size(); i++){
            profile.note.append(frame->parametri[i]+ " ");
        }
    }

    // Salva il profilo
    pGeneratore->pAECprofiles->saveProfile(profile);

    // Non ricarica poichË non Ë un dato utile al Gantry

    emit consoleTxHandler(answer->answToQByteArray("OK 0"));
    return;

}

void  console::handleGetProfileNote(protoConsole* frame, protoConsole* answer){
    AEC::profileCnf_Str profile = pGeneratore->pAECprofiles->getProfile(frame->parametri[0]);
    if(profile.filename==""){
        emit consoleTxHandler(answer->answToQByteArray("NOK 1 INVALID-PROFILE"));
        return;
    }

    emit consoleTxHandler(answer->answToQByteArray("OK \"" + profile.note +"\""));
    return;

}

/*
 *  Crea/Sostituisce il contenuto di un profilo
 *  Non modifica la lista di validazione!
 *  Ricarica in memoria i profili
 */
void  console::handleSetProfile(protoConsole* frame, protoConsole* answer){

    AEC::profileCnf_Str profile;
    int j;
    int parsize=24;

    if(frame->parametri.size()<parsize){
        emit consoleTxHandler(answer->answToQByteArray("NOK 1 INVALID-PARAM"));
        return;
    }

    profile.basefilename = frame->parametri[0];                     // name
    profile.symbolicName = frame->parametri[1];                     // symbolic name

    if(frame->parametri[2]=="CR") profile.plateType = ANALOG_PLATE_CR;     // plate type
    else if(frame->parametri[2]=="FILM") profile.plateType = ANALOG_PLATE_FILM;
    else{
        emit consoleTxHandler(answer->answToQByteArray("NOK 2 INVALID-PLATE"));
        return;
    }

    profile.technic = ANALOG_TECH_PROFILE_LD;
    profile.plog_threshold = frame->parametri[3].toInt();           // PLOG th
    profile.plog_minimo = frame->parametri[4].toInt();              // PLOG minimo
    profile.odindex = 5;

    for(int i=0; i<11; i++) profile.od[i] = frame->parametri[5+i].toInt();  // OD INDEX

    j=16;
    if(frame->parametri[j++] != "MoG"){
        emit consoleTxHandler(answer->answToQByteArray("NOK 3 INVALID-MoG"));
        return;
    }

    int cnt = frame->parametri[j++].toInt();
    parsize+=(cnt*3);
    if(frame->parametri.size()<parsize){
        emit consoleTxHandler(answer->answToQByteArray("NOK 4 INVALID-PARAM"));
        return;
    }
    profile.pulseStd_Mo_G.clear();
    for(int i=0; i<cnt; i++){
        AEC::profilePoint_Str item;
        item.plog = frame->parametri[j++].toInt();
        item.pulse = frame->parametri[j++].toInt();
        item.pmmi = frame->parametri[j++].toInt();
        profile.pulseStd_Mo_G.append(item);
    }
    if(frame->parametri[j++] != "MoP"){
        emit consoleTxHandler(answer->answToQByteArray("NOK 5 INVALID-MoP"));
        return;
    }

    cnt = frame->parametri[j++].toInt();
    parsize+=(cnt*3);
    if(frame->parametri.size()<parsize){
        emit consoleTxHandler(answer->answToQByteArray("NOK 6 INVALID-PARAM"));
        return;
    }
    profile.pulseStd_Mo_P.clear();
    for(int i=0; i<cnt; i++){
        AEC::profilePoint_Str item;
        item.plog = frame->parametri[j++].toInt();
        item.pulse = frame->parametri[j++].toInt();
        item.pmmi = frame->parametri[j++].toInt();
        profile.pulseStd_Mo_P.append(item);
    }

    if(frame->parametri[j++] != "RhG"){
        emit consoleTxHandler(answer->answToQByteArray("NOK 7 INVALID-RhG"));
        return;
    }
    cnt = frame->parametri[j++].toInt();
    parsize+=(cnt*3);
    if(frame->parametri.size()<parsize){
        emit consoleTxHandler(answer->answToQByteArray("NOK 8 INVALID-PARAM"));
        return;
    }
    profile.pulseStd_Rh_G.clear();
    for(int i=0; i<cnt; i++){
        AEC::profilePoint_Str item;
        item.plog = frame->parametri[j++].toInt();
        item.pulse = frame->parametri[j++].toInt();
        item.pmmi = frame->parametri[j++].toInt();
        profile.pulseStd_Rh_G.append(item);
    }

    if(frame->parametri[j++] != "RhP"){
        emit consoleTxHandler(answer->answToQByteArray("NOK 9 INVALID-RhP"));
        return;
    }

    cnt = frame->parametri[j++].toInt();
    parsize+=(cnt*3);
    if(frame->parametri.size()<parsize){
        emit consoleTxHandler(answer->answToQByteArray("NOK 10 INVALID-PARAM"));
        return;
    }
    profile.pulseStd_Rh_P.clear();
    for(int i=0; i<cnt; i++){
        AEC::profilePoint_Str item;
        item.plog = frame->parametri[j++].toInt();
        item.pulse = frame->parametri[j++].toInt();
        item.pmmi = frame->parametri[j++].toInt();
        profile.pulseStd_Rh_P.append(item);
    }

    pGeneratore->pAECprofiles->saveProfile(profile);

    // Aggiunge il nuovo file alla lista di validazione e ricarica i profili
    pGeneratore->pAECprofiles->addFileToValidList(profile.basefilename);

    // Ricarica i profili con la nuova lista
    pGeneratore->pAECprofiles->openAECProfiles();

    emit consoleTxHandler(answer->answToQByteArray("OK 0"));
    return;
}

/*
 *  Cancella un profilo in memoria
 *  Aggiorna la lista di validazione (eliminando il file, se presente)
 *  Ricarica in memoria i profili
 */
void  console::handleEraseProfile(protoConsole* frame, protoConsole* answer){

    if(frame->parametri.size()!=1){
        emit consoleTxHandler(answer->answToQByteArray("NOK 1 INVALID-PARAM"));
        return;
    }

    // Cancella un profilo dal file system e dalla lista di validazione
    pGeneratore->pAECprofiles->eraseProfile(frame->parametri[0]);

    // Ricarica i profili con la nuova lista
    pGeneratore->pAECprofiles->openAECProfiles();

    emit consoleTxHandler(answer->answToQByteArray("OK 0"));
    return;
}

/*
 *  Cancella tutti i profili ad eccezione di quelli di default
 *  Aggiorna la lista di validazione
 *  Ricarica in memoria i profili
 */
void  console::handleEraseAllProfiles(protoConsole* frame, protoConsole* answer){

    // Cancella un profilo dal file system e dalla lista di validazione
    pGeneratore->pAECprofiles->eraseAllProfiles();

    // Ricarica i profili con la nuova lista
    pGeneratore->pAECprofiles->openAECProfiles();

    emit consoleTxHandler(answer->answToQByteArray("OK 0"));
    return;
}

/*
 *  Funzione che restituisce la combinazione Anodo Filtro
 *  configurata. L'Anodo dipende dal Tubo selezionato
 *  ed il filtro dalla presenza del doppio filtro o singolo
 *  filtro
 *  RESULT: OK 1/0 1/0 1/0
 *      - Il primo rappresenta la combinazione MoMo
 *      - La seconda MoRh
 *      - La terza WRh
 */
void  console::handleGetAnalogAfSetup(protoConsole* frame, protoConsole* answer){
    bool mo=false;
    bool rh=false;
    QString stringa;

    // Determiinazione dell'anodo
    if(pGeneratore->confF1=="Mo"){
        if(pConfig->analogCnf.primo_filtro == Collimatore::FILTRO_Mo) mo=true;
        else if(pConfig->analogCnf.primo_filtro == Collimatore::FILTRO_Rh) rh=true;

        if(pConfig->analogCnf.secondo_filtro == Collimatore::FILTRO_Mo) mo=true;
        else if(pConfig->analogCnf.secondo_filtro == Collimatore::FILTRO_Rh) rh=true;

        stringa = "OK ";
        if(mo) stringa += "1 ";
        else stringa += "0 ";
        if(rh) stringa += "1 ";
        else stringa += "0 ";
        stringa += "0";


    }else{
        // Solo WRh Ë ammesso
        stringa = "OK 0 0 1";

    }

    emit consoleTxHandler(answer->answToQByteArray(stringa));
    return;
}

/*
 *  Salvataggio file di calibrazione AIR KERMA nel tubo correntemente selezionato
 *  PARAMETRI:
 *  - Fitro Mo/Rh
 *  - S (CGS)
 *  - D0: distanza strumento da fuoco
 *  - D1: distanza piano di compressione (potter)
 *  - KVH + VAL
 *  - KVM + VAL
 *  - KVL + VAL
 */
void  console::handleSetAnalogStoreKerma(protoConsole* frame, protoConsole* answer){

    if(frame->parametri.size()!=9){
        emit consoleTxHandler(answer->answToQByteArray("NOK 1 INVALID-PARAM"));
        return;
    }

    QString filtro = frame->parametri[0];
    int D0 = frame->parametri[1].toInt();
    int D1 = frame->parametri[2].toInt();
    int KVH = frame->parametri[3].toInt();
    float AKH = frame->parametri[4].toFloat();
    int KVM = frame->parametri[5].toInt();
    float AKM = frame->parametri[6].toFloat();
    int KVL = frame->parametri[7].toInt();
    float AKL = frame->parametri[8].toFloat();

    if(pGeneratore->pDose->storeKermaConfig(filtro,D0,D1,KVH,AKH,KVM,AKM,KVL,AKL)==true) {
        emit consoleTxHandler(answer->answToQByteArray("OK 0"));
    }else emit consoleTxHandler(answer->answToQByteArray("NOK 2 INVALID-STORE"));

    return;
}

/*
 *  Richiede i dati di calibrazione Kerma per un dato filtro
 *  PARAMETRI:
 *  - Fitro Mo/Rh
 *  RISULTATO:
 *  - D0: distanza strumento da fuoco
 *  - D1: distanza piano di compressione (potter)
 *  - KVH + VAL
 *  - KVM + VAL
 *  - KVL + VAL
 */
void  console::handleGetAnalogKermaData(protoConsole* frame, protoConsole* answer){

    if(frame->parametri.size()!=1){
        emit consoleTxHandler(answer->answToQByteArray("NOK 1 INVALID-PARAM"));
        return;
    }

    QString filtro = frame->parametri[0];
    if((filtro!="Mo")&&(filtro!="Rh")){
        emit consoleTxHandler(answer->answToQByteArray("NOK 2 INVALID-FILTER"));
        return;
    }

    DOSE::kermaCnf_Str* pKerma = pGeneratore->pDose->getKermaPtr(filtro);
    QString stringa = "OK ";

    stringa.append(QString("%1 ").arg(pKerma->D0));
    stringa.append(QString("%1 ").arg(pKerma->D1));
    stringa.append(QString("%1 ").arg(pKerma->KVH));
    stringa.append(QString("%1 ").arg(pKerma->AKH));
    stringa.append(QString("%1 ").arg(pKerma->KVM));
    stringa.append(QString("%1 ").arg(pKerma->AKM));
    stringa.append(QString("%1 ").arg(pKerma->KVL));
    stringa.append(QString("%1 ").arg(pKerma->AKL));

    emit consoleTxHandler(answer->answToQByteArray(stringa));


    return;
}

/*
 *  Richiede i dati della tabella CGS per filtro e kV
 *  PARAMETRI:
 *  - Fitro (QString) ["Mo"/"Rh"]
 *  - kV:   (float)   [20,35]
 *
 *  RISULTATO: "<OK Filtro kV cgs(20mm), cgs(20.5mm) .... cgs(80mm)>"
 *
 */
void  console::handleGetAnalogCgs(protoConsole* frame, protoConsole* answer){

    if(frame->parametri.size()!=2){
        emit consoleTxHandler(answer->answToQByteArray("NOK 1 INVALID-PARAM"));
        return;
    }

    QString filtro = frame->parametri[0];
    if((filtro!="Mo")&&(filtro!="Rh")){
        emit consoleTxHandler(answer->answToQByteArray("NOK 2 INVALID-FILTER"));
        return;
    }

    float kV = frame->parametri[1].toFloat();
    if((kV<20) || (kV>35)) {
        emit consoleTxHandler(answer->answToQByteArray("NOK 3 INVALID-KV RANGE"));
        return;
    }

    DOSE::kermaCnf_Str* pKerma = pGeneratore->pDose->getKermaPtr(filtro);
    if(pKerma==0){
        emit consoleTxHandler(answer->answToQByteArray("NOK 4 AGD NOT AVAILABLE"));
        return;
    }


    QString stringa = "OK " + filtro + " " + frame->parametri[1] + " ";

    int kvi = (kV-20)*2;
    for(int i=0; i<=13; i++ ){
        stringa += QString("%1 ").arg(pKerma->cgs[kvi][i]);
    }

    emit consoleTxHandler(answer->answToQByteArray(stringa));
    return;
}

/*
 *typedef struct
{


    QString ServicePassword;     // Password per entrare nel menu
    bool    audioEnable;         // Abilitazione messaggi audio

}userCnf_Str;
 *
 *
 *
 *
 *  LISTA PARAMETRI VARIABILE: <TAG> <VAL>
 *
 */
void console::handleSetAnalogParam(protoConsole* frame, protoConsole* answer){

    // Solo numero parametri pari
    if(frame->parametri.size()%2){
        emit consoleTxHandler(answer->answToQByteArray("NOK 1 INVALID-PARAM"));
        return;
    }

    // Scrolla la lista dei parametri per assegnarne il valore
    for(int i=0; i<frame->parametri.size(); i+=2){
        QString tag = frame->parametri[i];
        QString val = frame->parametri[i+1];

        //DEADMAN
        //DEMO_MODE

        if(tag=="LCC"){
            pConfig->analogCnf.LCC = val.toInt();

        }else if(tag=="LMLO"){
            pConfig->analogCnf.LMLO = val.toInt();


        }else if(tag=="LML"){
            pConfig->analogCnf.LML = val.toInt();

        }else if(tag=="LISO"){
            pConfig->analogCnf.LISO = val.toInt();


        }else if(tag=="LFB"){
            pConfig->analogCnf.LFB = val.toInt();

        }else if(tag=="LSIO"){
            pConfig->analogCnf.LSIO = val.toInt();


        }else if(tag=="LLM"){
            pConfig->analogCnf.LLM = val.toInt();


        }else if(tag=="LLMO"){
            pConfig->analogCnf.LLMO = val.toInt();


        }else if(tag=="RCC"){
            pConfig->analogCnf.RCC = val.toInt();

        }else if(tag=="RMLO"){
            pConfig->analogCnf.RMLO = val.toInt();

        }else if(tag=="RML"){
            pConfig->analogCnf.RML = val.toInt();

        }else if(tag=="RISO"){
            pConfig->analogCnf.RISO = val.toInt();

        }else if(tag=="RFB"){
            pConfig->analogCnf.RFB = val.toInt();

        }else if(tag=="RSIO"){
            pConfig->analogCnf.RSIO = val.toInt();

        }else if(tag=="RLM"){
            pConfig->analogCnf.RLM = val.toInt();

        }else if(tag=="RLMO"){
            pConfig->analogCnf.RLMO = val.toInt();

        }else if(tag=="DOSE_FORMAT"){
            if(val=="u")
                pConfig->analogCnf.doseFormat = 'u';
            else if(val=="m")
                pConfig->analogCnf.doseFormat = 'm';
            else if(val=="z")
                pConfig->analogCnf.doseFormat = 'z';

        }else if(tag=="FIRST_FILTER"){
            if(val=="Mo")
                pConfig->analogCnf.primo_filtro = Collimatore::FILTRO_Mo;
            else if(val=="Rh")
                 pConfig->analogCnf.primo_filtro = Collimatore::FILTRO_Rh;
            else
                pConfig->analogCnf.primo_filtro = Collimatore::FILTRO_Mo;


        }else if(tag=="SECOND_FILTER"){
            if(val=="Mo")
                pConfig->analogCnf.secondo_filtro = Collimatore::FILTRO_Mo;
            else if(val=="Rh")
                 pConfig->analogCnf.secondo_filtro = Collimatore::FILTRO_Rh;
            else
                pConfig->analogCnf.secondo_filtro = Collimatore::FILTRO_ND;
        }else if(tag=="LANGUAGE"){
            pConfig->userCnf.languageInterface = val;

        }else if(tag=="PASSWORD"){
            pConfig->userCnf.ServicePassword = val;

        }else if(tag=="AUDIO"){
            if(val=="ON")
                pConfig->userCnf.audioEnable = true;
            else
                pConfig->userCnf.audioEnable = false;
        }else if(tag=="DEADMAN"){
            if(val=="ON")
                pConfig->userCnf.deadman = true;
            else
                pConfig->userCnf.deadman = false;
            // ON OFF
        }else if(tag=="DEMO_MODE"){
            if(val=="ON")
                pConfig->userCnf.demoMode = true;
            else
                pConfig->userCnf.demoMode = false;
        }else{
            emit consoleTxHandler(answer->answToQByteArray("NOK 2 INVALID-TAG"));
            return;
        }
    }

    // Applicazione di regole di consistenza sul filtro
    if(pConfig->analogCnf.primo_filtro == pConfig->analogCnf.secondo_filtro)
        pConfig->analogCnf.secondo_filtro = Collimatore::FILTRO_ND;

    if(pConfig->analogCnf.secondo_filtro == Collimatore::FILTRO_ND){
        pConfig->analogCnf.auto_filtro_mode = false;
        pConfig->analogCnf.selected_filtro = pConfig->analogCnf.primo_filtro;
    }


    emit consoleTxHandler(answer->answToQByteArray("OK 0"));
    return;
}


// Restituisce in blocco tutti i parametri
void console::handleGetAnalogParam(protoConsole* frame, protoConsole* answer){

    QString stringa = "OK ";


    if(pConfig->userCnf.deadman) stringa.append(QString("DEADMAN ON "));
    else stringa.append(QString("DEADMAN OFF "));
    if(pConfig->userCnf.demoMode) stringa.append(QString("DEMO_MODE ON "));
    else stringa.append(QString("DEMO_MODE OFF "));

    stringa.append(QString("LCC %1 ").arg(pConfig->analogCnf.LCC));
    stringa.append(QString("LMLO %1 ").arg(pConfig->analogCnf.LMLO));
    stringa.append(QString("LML %1 ").arg(pConfig->analogCnf.LML));
    stringa.append(QString("LISO %1 ").arg(pConfig->analogCnf.LISO));
    stringa.append(QString("LFB %1 ").arg(pConfig->analogCnf.LFB));
    stringa.append(QString("LSIO %1 ").arg(pConfig->analogCnf.LSIO));
    stringa.append(QString("LLM %1 ").arg(pConfig->analogCnf.LLM));
    stringa.append(QString("LLMO %1 ").arg(pConfig->analogCnf.LLMO));

    stringa.append(QString("RCC %1 ").arg(pConfig->analogCnf.RCC));
    stringa.append(QString("RMLO %1 ").arg(pConfig->analogCnf.RMLO));
    stringa.append(QString("RML %1 ").arg(pConfig->analogCnf.RML));
    stringa.append(QString("RISO %1 ").arg(pConfig->analogCnf.RISO));
    stringa.append(QString("RFB %1 ").arg(pConfig->analogCnf.RFB));
    stringa.append(QString("RSIO %1 ").arg(pConfig->analogCnf.RSIO));
    stringa.append(QString("RLM %1 ").arg(pConfig->analogCnf.RLM));
    stringa.append(QString("RLMO %1 ").arg(pConfig->analogCnf.RLMO));

    stringa.append(QString("DOSE_FORMAT %1 ").arg(pConfig->analogCnf.doseFormat));
    if(pConfig->analogCnf.primo_filtro == Collimatore::FILTRO_Mo) stringa.append(QString("FIRST_FILTER Mo "));
    else stringa.append(QString("FIRST_FILTER Rh "));

    if(pConfig->analogCnf.secondo_filtro == Collimatore::FILTRO_ND) stringa.append(QString("SECOND_FILTER ND "));
    else if(pConfig->analogCnf.secondo_filtro == Collimatore::FILTRO_Mo) stringa.append(QString("SECOND_FILTER Mo "));
    else stringa.append(QString("SECOND_FILTER Rh "));


    stringa.append(QString("LANGUAGE %1 ").arg(pConfig->userCnf.languageInterface));
    stringa.append(QString("PASSWORD %1 ").arg(pConfig->userCnf.ServicePassword));
    if(pConfig->userCnf.audioEnable) stringa.append(QString("AUDIO ON "));
    else stringa.append(QString("AUDIO OFF "));

    emit consoleTxHandler(answer->answToQByteArray(stringa));
}

void console::handleStoreAnalogParam(protoConsole* frame, protoConsole* answer){
    pConfig->saveUserCfg();
    pConfig->saveAnalogConfig();
    pSysLog->log("CONFIG: ANALOG CONFIGURATION FILE");
    emit consoleTxHandler(answer->answToQByteArray("OK 0"));
}


