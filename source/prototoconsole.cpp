#include "appinclude.h"
#include "globvar.h"
#include "audio.h"
// Comandi da inviare a Console
extern audio* pAudio;

void  logOutput(QtMsgType type, const char *msg);

void  logOutput(QtMsgType type, const char *msg)
{
    pToConsole->logMessages(QString(msg));
    return;
}

void protoToConsole::activateConnections(void){
    QObject::connect(notificheTcp,SIGNAL(clientConnection(bool)),this,SLOT(notificheConnectionHandler(bool)),Qt::UniqueConnection);
    QObject::connect(this,SIGNAL(notificheTxHandler(QByteArray)), notificheTcp,SLOT(txData(QByteArray)),Qt::UniqueConnection);
    notificheTcp->Start(QHostAddress(_CONSOLE_IP),_CONSOLE_OUT_PORT);

    QObject::connect(msgErrorTcp,SIGNAL(clientConnection(bool)),this,SLOT(errConnectionHandler(bool)),Qt::UniqueConnection);
    QObject::connect(this,SIGNAL(errorTxHandler(QByteArray)), msgErrorTcp,SLOT(txData(QByteArray)),Qt::UniqueConnection);
    msgErrorTcp->Start(QHostAddress(_CONSOLE_IP),_CONSOLE_ERROR_PORT);

    QObject::connect(msgLogTcp,SIGNAL(clientConnection(bool)),this,SLOT(logConnectionHandler(bool)),Qt::UniqueConnection);
    msgLogTcp->Start(QHostAddress(_CONSOLE_IP),_CONSOLE_LOG_PORT);

    qInstallMsgHandler(logOutput);

}

protoToConsole::protoToConsole(QObject *parent) :
    QObject(0)
{
    notificheConnected = FALSE;

    logConnected = FALSE;
    localDebugEnable=FALSE;

    // Socket per segnali asincroni
    notificheTcp = new TcpIpClient();


    // Socket per notifica errori
    msgErrorTcp = new TcpIpClient();    

    // Socket per notifica di loggin
    msgLogTcp = new TcpIpClient();

}


// Risposta di acknowledge a comando
void protoToConsole::endCommandAck(unsigned char id, unsigned char code)
{
    protoConsole ack(id,UNICODE_FORMAT);
    emit notificheTxHandler(ack.ackToQByteArray(code));
}



/*
 *  SLOT DI RICEZIONE DATI DA SEQUENZA RAGGI DI CALIBRAZIONE KV
 *
 *  CAMPI VALORE PARAMETRO data

    Stringa di Risposta:	<ID LEN %SetFineRaggiCalibKv PAR0 PAR1%>
    PARAMETRI:      Tipo        dato        Valore              Note
    PAR0            Int         Risultato
                                            0=OK
                                            1: KV BASSI
                                            2: KV ALTI
                                            3: IA BASSA
                                            4: IA ALTA
                                            5: ERRORE DI ALTRO TIPO

    PAR1            float         Valore Kv	Se PAR0 == 0 -> kV letti (Monitor)
                                            Se PAR0 == 5 -> Codice di errore
                                            Else: 0

    PAR2            int         Valore ImA	Se PAR0 == 0 -> ImA letti (Monitor)
                                            Else: 0
    Frame di risposta atteso: NESSUNO

 */
void protoToConsole::fineRaggiCalibKv(QByteArray data)
{
    int par0,  par2;
    float par1;
    protoConsole cmd(1,UNICODE_FORMAT);
    int tpls,mas,imean;

    disconnect(pConsole,SIGNAL(raggiDataSgn(QByteArray)),pToConsole,SLOT(fineRaggiCalibKv(QByteArray)));

    float vmean = data.at(V_MEAN);
    mas = (data.at(MAS_PLS_L)+256*data.at(MAS_PLS_H)); // In 1/50 mAs unit
    tpls = (unsigned short) (data.at(T_MEAN_PLS_L)+256*data.at(T_MEAN_PLS_H));

    // Calcolo corrente media sulla durata dell'impulso
    if(tpls!=0) imean = mas * 20 / tpls;
    else if(data.at(I_MEAN)!=0) imean = pGeneratore->convertPcb190Ianode(data.at(I_MEAN));
    else imean=0;

    par1 = vmean;   // Valore riletto
    par2 = imean;   // Valore corrente riletto

    // Valutazione del risultato
    switch(data[RX_END_CODE])
    {
        case 0:
        case ERROR_IA_LOW:
        case ERROR_IA_HIGH:
        case ERROR_TMO_RX:
        case ERROR_HV_LOW:
        case ERROR_HV_HIGH:
            par0 = 0;        // Tutto OK
            break;
        default:
            par0 = 5;
            par1 = 0;
            par2 = 0;
            par1 = data[RX_END_CODE];
            break;
    }

    // Notifica la Console sul fine raggi
    cmd.addParam(QString("%1").arg(par0));
    cmd.addParam(QString("%1").arg(par1));
    cmd.addParam(QString("%1").arg(par2));


    emit notificheTxHandler(cmd.cmdToQByteArray(NOTIFY_SEND_FINE_RAGGI_CALIB_KV));

    // Disattiva simbolo grafico per raggi
    pConsole->handleSetXraySym(false);

    return;
}

/* Invio dati durata impulsi Tomo */
void protoToConsole::setSamples(QByteArray data)
{
    int ciclo;
    protoConsole cmd(1,UNICODE_FORMAT);

    cmd.addParam(QString("%1").arg((int) data.size()));
    for(ciclo=0; ciclo<data.size(); ciclo++){
        cmd.addParam(QString("%1").arg((int) data.at(ciclo)));
    }
    emit notificheTxHandler(cmd.cmdToQByteArray(NOTIFY_SEND_SET_PUSH_CMD));
    return;
}


/*
 *  SLOT DI RICEZIONE DATI DA SEQUENZA RAGGI DI CALIBRAZIONE KV
 *
 *  CAMPI VALORE PARAMETRO data

    Stringa di Risposta:	<ID LEN %SetFineRaggiCalibKv PAR0 PAR1%>
    PARAMETRI:      Tipo        dato        Valore              Note
    PAR0            Int         Risultato
                                            0=OK
                                            1: KV BASSI
                                            2: KV ALTI
                                            3: IA BASSA
                                            4: IA ALTA
                                            5: ERRORE DI ALTRO TIPO

    PAR1            float         Valore Kv	Se PAR0 == 0 -> kV letti (Monitor)
                                            Se PAR0 == 5 -> Codice di errore
                                            Else: 0

    PAR2            int         Valore ImA	Se PAR0 == 0 -> ImA letti (Monitor)
                                            Else: 0
    Frame di risposta atteso: NESSUNO

 */
void protoToConsole::fineRaggiCalibIa(QByteArray data)
{
    int par0,  par2;
    float par1;
    protoConsole cmd(1,UNICODE_FORMAT);
    int tpls,mas,imean;

    disconnect(pConsole,SIGNAL(raggiDataSgn(QByteArray)),pToConsole,SLOT(fineRaggiCalibIa(QByteArray)));

    float vmean = pGeneratore->convertPcb190Kv(data.at(V_MEAN),1.0);
    mas = (data.at(MAS_PLS_L)+256*data.at(MAS_PLS_H)); // In 1/50 mAs unit
    tpls = (unsigned short) (data.at(T_MEAN_PLS_L)+256*data.at(T_MEAN_PLS_H));

    // Calcolo corrente media sulla durata dell'impulso
    if(tpls!=0) imean = mas * 20 / tpls;
    else if(data.at(I_MEAN)!=0) imean = pGeneratore->convertPcb190Ianode(data.at(I_MEAN));
    else imean=0;

    par1 = vmean;   // Valore riletto
    par2 = imean;   // Valore corrente riletto

    // Valutazione del risultato
    switch(data[RX_END_CODE])
    {
        case 0:
        case ERROR_IA_LOW:
        case ERROR_IA_HIGH:
        case ERROR_TMO_RX:
            par0 = 0;        // Tutto OK
            break;
        case ERROR_HV_LOW:
                par0 = 1;
            break;
        case ERROR_HV_HIGH:
                par0 = 2;
            break;
        default:
            par0 = 5;
            par1 = 0;
            par2 = 0;
            par1 = data[RX_END_CODE];
            break;
    }

    // Notifica la Console sul fine raggi
    cmd.addParam(QString("%1").arg(par0));
    cmd.addParam(QString("%1").arg(par1));
    cmd.addParam(QString("%1").arg(par2));


    emit notificheTxHandler(cmd.cmdToQByteArray(NOTIFY_SEND_FINE_RAGGI_CALIB_IA));

    // Disattiva simbolo grafico per raggi
    pConsole->handleSetXraySym(false);

    return;
}


//______________________________________________________________________
// Fine raggi procedura di calibrazione detector per macchine Analogiche
/* if(data.at(0)){
            // Esito negativo
            PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_RAGGI, data.at(0),TRUE); // Self resetting

        }else{
            // Assegna i dati ricevuti
            ApplicationDatabase.setData(_DB_DETECTOR_DMAS,(int) ((int) data[1]+ 256 * (int) data[2])  );
            ApplicationDatabase.setData(_DB_DETECTOR_PLOG,(int) ((int) data[3]+ 256 * (int) data[4])  );
            ApplicationDatabase.setData(_DB_DETECTOR_RAD,(int) ((int) data[5]+ 256 * (int) data[6])  );
            ApplicationDatabase.setData(_DB_DETECTOR_OFSET,(int) ((int) data[9]+ 256 * (int) data[10])  );
        }
*/
void protoToConsole::fineRaggiAnalogCalibDetector(QByteArray data)
{

    protoConsole cmd(1,UNICODE_FORMAT);

    // Notifica la Console sul fine raggi
    cmd.addParam(QString("%1").arg((unsigned char) data[0])); // Esito fine raggi
    cmd.addParam(QString("%1").arg((float) (data[1]+ 256 * data[2])/10));    // mAs
    cmd.addParam(QString("%1").arg((int) (data[3]+ 256 * data[4])));         // PLOG
    cmd.addParam(QString("%1").arg((int) (data[5]+ 256 * data[6])));         // RAD


    emit notificheTxHandler(cmd.cmdToQByteArray(NOTIFY_FINERAGGI_CALIB_DETECTOR));
    return;
}

void protoToConsole::fineRaggiAnalogCalibTube(unsigned char result, unsigned char rawKv, int dkV, int dIa)
{

    protoConsole cmd(1,UNICODE_FORMAT);

    // Notifica la Console sul fine raggi
    cmd.addParam(QString("%1").arg((unsigned char) result)); // Esito fine raggi
    cmd.addParam(QString("%1").arg((unsigned char) rawKv)); // Esito fine raggi
    cmd.addParam(QString("%1").arg((float) dkV/10));    // kV
    cmd.addParam(QString("%1").arg((float) dIa/10));    // Ia

    emit notificheTxHandler(cmd.cmdToQByteArray(NOTIFY_ANALOG_FINERAGGI_CALIB_TUBE));
    return;
}

/*
    data[0]=0/Error code;
    data[2:1]=DMAS
    data[4:3]=Pulses
    data[6:5]=dkV;
    data[8:7]=PLOG;
*/
void protoToConsole::fineRaggiAnalogCalibProfile(QByteArray data)
{

    protoConsole cmd(1,UNICODE_FORMAT);

    // Notifica la Console sul fine raggi
    cmd.addParam(QString("%1").arg((unsigned char) data[0]));                // Esito fine raggi
    cmd.addParam(QString("%1").arg((float) (data[1]+ 256 * data[2])/10));    // mAs
    cmd.addParam(QString("%1").arg((int) (data[3]+ 256 * data[4])));         // pulse
    cmd.addParam(QString("%1").arg(((float) (data[5]+ 256 * data[6]))/10));  // kV
    cmd.addParam(QString("%1").arg((int) (data[7]+ 256 * data[8])));         // plog


    emit notificheTxHandler(cmd.cmdToQByteArray(NOTIFY_ANALOG_FINERAGGI_CALIB_PROFILE));
    return;
}

void protoToConsole::notificheConnectionHandler(bool stat)
{
    notificheConnected = stat;
    if(stat)
    {

        ApplicationDatabase.setData(_DB_AWS_CONNECTION,(unsigned char) 1,0);
    }
    else
    {
        PRINT("AWS DISCONNECTED");
        ApplicationDatabase.setData(_DB_AWS_CONNECTION,(unsigned char) 0,0);
    }
    return;
}


void protoToConsole::logConnectionHandler(bool stat)
{
    logConnected = stat;
    if(stat)
    {
        connect(this,SIGNAL(logTxHandler(QByteArray)), msgLogTcp,SLOT(txData(QByteArray)),Qt::UniqueConnection);

    }else
    {
        disconnect(this,SIGNAL(logTxHandler(QByteArray)), msgLogTcp,SLOT(txData(QByteArray)));

    }
    return;
}


void protoToConsole::errConnectionHandler(bool stat)
{
    errorConnected = stat;
    if(stat)
    {
        connect(paginaAllarmi,SIGNAL(newAlarmSgn(int,QString)),this,SLOT(alarmNotify(int,QString)),Qt::UniqueConnection);

    }else
    {
        disconnect(paginaAllarmi,SIGNAL(newAlarmSgn(int,QString)),this,SLOT(alarmNotify(int,QString)));

    }
    return;
}

//////////////////////////////////////////////////////////////////////////////
/*

 */
//////////////////////////////////////////////////////////////////////////////
void protoToConsole::enableXrayPush(bool enable)
{
    xrayEnable = enable;
    if(enable) ApplicationDatabase.setData(_DB_READY_EXPOSURE,(unsigned char) 1,0);
    else ApplicationDatabase.setData(_DB_READY_EXPOSURE,(unsigned char) 0,0);

}

/* Questa funzione viene lanciata solo se il pulsante è stato abilitato
 */
void protoToConsole::activationXrayPush(void)
{
    if(xrayEnable)
    {
        protoConsole cmd(1,UNICODE_FORMAT);
        emit notificheTxHandler(cmd.cmdToQByteArray(NOTIFY_SEND_PUSH_CMD));
    }

}


void protoToConsole::notifyReadyForExposure(bool status)
{
    protoConsole cmd(1,UNICODE_FORMAT);
    if(status) cmd.addParam("ON");
    else cmd.addParam("OFF");
    emit notificheTxHandler(cmd.cmdToQByteArray(NOTIFY_SET_READY));

    if(status) pAudio->playAudio(AUDIO_READY_FOR_EXPOSURE);

}

/*
 *  Questa funzione viene lanciata per notificare la AWS di un comando di movimento braccio in corso
 */
void protoToConsole::notifyMovingArm(void)
{
    protoConsole cmd(1,UNICODE_FORMAT);
    emit notificheTxHandler(cmd.cmdToQByteArray(NOTIFY_SEND_TUBE_MOVING));

}

/*
 *  Questa funzione viene lanciata per notificare la AWS di un comando di movimento braccio in corso
 */
void protoToConsole::notifyProjectionSelection(QString projection)
{
    protoConsole cmd(1,UNICODE_FORMAT);
    cmd.addParam(projection);

    emit notificheTxHandler(cmd.cmdToQByteArray(NOTIFY_SEND_SELECTED_PROJECTION));

}

void protoToConsole::notifyAbortProjection(void)
{
    protoConsole cmd(1,UNICODE_FORMAT);
    emit notificheTxHandler(cmd.cmdToQByteArray(NOTIFY_SEND_ABORT_PROJECTION));

}
void protoToConsole::notifyRequestPowerOff(void)
{
    protoConsole cmd(1,UNICODE_FORMAT);
    emit notificheTxHandler(cmd.cmdToQByteArray(NOTIFY_REQUEST_POWER_OFF));
}




/*  HANDLER DI INVIO MESSAGGIO DI ERRORE SUL SOCKET DELLA CONSOLE
 *  IL MESSAGGIO E' Composto secondo il protocollo toConsole con
 *  i seguenti campi:
 *  %CODEICE MESSAGGIO%
 *  Questo messaggio è attivo solo a studio aperto
 */
void protoToConsole::alarmNotify(int codice, QString msg)
{
    protoConsole cmd(0,UNICODE_FORMAT);
    cmd.addParam(msg);

    if((errorConnected)&&(ApplicationDatabase.getDataU(_DB_STUDY_STAT)!=_CLOSED_STUDY_STATUS)) emit errorTxHandler(cmd.cmdToQByteArray(QString("%1").arg(codice)));

    return;
}


/*  HANDLER DI INVIO MESSAGGI DI LOG SUL SOCKET DELLA CONSOLE
 *  IL MESSAGGIO E' Composto secondo il protocollo toConsole con
 *  i seguenti campi:
 *  %MESSAGGIO%
 *
 */
void protoToConsole::logMessages(QString msg)
{

    protoConsole cmd(0,UNICODE_FORMAT);
    QString parsed;
    char ch;
    int i;

    // Effettua il parsing della stringa per eliminare i caratteri speciali
    parsed.clear();
    for(i=0;i< msg.size();i++)
    {
        ch = msg.at(i).toAscii();
        // if(ch=='\n') ch=' ';
        // else if(ch=='\r') continue;
        if(ch=='%') ch = '$';
        else if(ch=='<') ch = '|';
        else if(ch=='>') ch = '|';
        parsed.append(ch);
    }

    // Invia al LOGGER una versione della stringa parserata per togliere i caratteri speciali
    emit logTxHandler(cmd.cmdToQByteArray(parsed));
    return;
}

// Funzione di notifica per classi esterne
void  protoToConsole::sendNotificheTcp(QByteArray frame)
{


    if(!notificheConnected) return;
    emit notificheTxHandler(frame);
}


/**
 * @brief protoToConsole::systemUpdateStatus
 * @param perc
 *
 * Invia la percentuale di agigornamnto del sistema
 */
void protoToConsole::systemUpdateStatus(int perc, QString filename){
    protoConsole cmd(1,UNICODE_FORMAT);

    cmd.addParam(QString("%1 0 '%2'").arg(perc).arg(filename));
    sendNotificheTcp(cmd.cmdToQByteArray(NOTIFY_SEND_FINE_SYS_UPDATE));

    return;
}

void protoToConsole::fineSystemUpdate(bool ris, QString errstr)
{
    protoConsole cmd(1,UNICODE_FORMAT);

    if(ris==true){
        cmd.addParam(QString("100 0"));
        sendNotificheTcp(cmd.cmdToQByteArray(NOTIFY_SEND_FINE_SYS_UPDATE));
    }else
    {
        cmd.addParam(QString("100 -1 '%1'").arg(errstr));
        sendNotificheTcp(cmd.cmdToQByteArray(NOTIFY_SEND_FINE_SYS_UPDATE));
    }

    return;
}

void protoToConsole::setBiopsyPosition(int curX, int curY, int curZ){
    protoConsole cmd(1,UNICODE_FORMAT);
    cmd.addParam(QString("%1 %2 %3").arg(curX).arg(curY).arg(curZ));
    sendNotificheTcp(cmd.cmdToQByteArray(NOTIFY_SET_BIOPSY_POSITION));

   return;
}

void protoToConsole::notifyTubeTemp(int anodeT, int tubeT){
    protoConsole cmd(1,UNICODE_FORMAT);
    cmd.addParam(QString("%1 %2 ").arg(anodeT).arg(tubeT));
    sendNotificheTcp(cmd.cmdToQByteArray(NOTIFY_TUBE_TEMPERATURE));
    return;
}

