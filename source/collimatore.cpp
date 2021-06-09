#include "application.h"
#include "appinclude.h"
#include "globvar.h"

#define COLLICFG "/resource/config/collimazione.cnf"

void Collimatore::activateConnections(void){
   // connect(pCompressore,SIGNAL(padChangedSgn()),this,SLOT(changedPadNotify()),Qt::UniqueConnection);
    connect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(guiNotifySlot(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
    connect(pConsole,SIGNAL(mccPcb249U1Notify(unsigned char,unsigned char,QByteArray)),this,SLOT(pcb249U1Notify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
}

Collimatore::Collimatore(QObject *parent) :
    QObject(parent)
{
    alrCuffia = FALSE;
    alrSensCuffia = FALSE;
    accessorio = COLLI_ACCESSORIO_ND;


    mirrorStat = MIRROR_ND; // non determinato
    lampStat = FALSE;       // Lampada Spenta
    filtroStat  = filtroCmd = FILTRO_ND;
    manualCollimation = FALSE;
    manualFiltroCollimation = FALSE;
    manualFilter = FILTRO_Rh;
    manualF = 0;// front
    manualB = 0;// back
    manualL = 0;// Left Blade
    manualR = 0;// Right Blade
    manualT = 40;// back Trap


    // Legge la configurazione ed aggiorna le periferiche interessate
    pConfig->collimator_configured = readConfigFile();
    colli_model = _COLLI_TYPE_NOT_ASSIGNED;
    colliConfUpdated = FALSE; // Configurazione non ancora inviata a dispositivi

    colliTestNumber = 0;
    colliTestTimer = 0;
}

void Collimatore::timerEvent(QTimerEvent* ev)
{

    // Dopo un certo tempo dall'apertura della finestra, questa viene chiusa
    // Alla chiusura vengono azzrati tutti gli allarmi one-shot
    if(ev->timerId()==colliTestTimer)
    {
        if(colliTestNumber==0){
            killTimer(colliTestTimer);
            colliTestTimer=0;
            manualCollimation = false;
            updateColli();
            return;
        }

        colliTestNumber--;
        if((colliTestNumber%4)==0){
            if(pCollimatore->manualF==40) pCollimatore->manualF = 0;
            else pCollimatore->manualF = 40;
        }
        if(colliTestStatus){
            colliTestStatus = false;
            pCollimatore->manualB = 190;
            pCollimatore->manualL = 200;
            pCollimatore->manualR = 200;
            manualColliUpdate();
        }else{
            colliTestStatus = true;
            pCollimatore->manualB = 0;
            pCollimatore->manualL = 0;
            pCollimatore->manualR = 0;
            manualColliUpdate();
        }


    }


}

/*
 *  buffer[0] = SYS_FLAGS0
 *  buffer[1] = raw Temperatura cuffia
 *  buffer[2] = codice identificativo accessorio collimatore
 */
void Collimatore::pcb249U1Notify(unsigned char id, unsigned char notifyCode, QByteArray buffer)
{
    static bool startupEvent = true;

    int   tempCuffia; // Temperatura cuffia in °C

    if(id!=1) return;
    switch(notifyCode)
    {
        case PCB249U1_NOTIFY_DATA:
            if(pConfig->userCnf.demoMode){
                tempCuffia = 25; // 25° in demo
                alrCuffia = FALSE;
                alrSensCuffia = FALSE;
            }else{
                // Formula temperatura cuffia: TRAW = 139 + 0.51 * T(�C)
                tempCuffia = (int) buffer.at(1);
                if((tempCuffia<15)||(tempCuffia>90)){                    
                    alrSensCuffia = TRUE;
                    alrCuffia = FALSE;
                }else if(tempCuffia > pConfig->userCnf.tempCuffiaAlr){
                    alrSensCuffia = FALSE;
                    alrCuffia = TRUE;
                }else if(tempCuffia < pConfig->userCnf.tempCuffiaAlrOff){
                    alrSensCuffia = FALSE;
                    alrCuffia = FALSE;
                }

            }

            // Aggiunta dello status
            if(tempCuffia>pConfig->userCnf.tempCuffiaAlr) tempCuffia|=0x0200;
            else if(tempCuffia>pConfig->userCnf.tempCuffiaAlrOff) tempCuffia|=0x0100;
            ApplicationDatabase.setData(_DB_T_CUFFIA,(int)tempCuffia, 0);
            accessorio = buffer.at(2);

            startupEvent = false;
        break;
    }

}

// Restituisce la posizione del PAD + mat nella lista
// o -1 se non lo trova
int Collimatore::getColli2DIndex(int pad)
{
    int i;

    // PAD TOMO
    if(pad == PAD_TOMO_24x30) return -1;

    // PAD 2D
    for(i=0; i< colliConf.colli2D.size();i++)
    {
        if(colliConf.colli2D.at(i).PadCode==pad) return i;
    }
    return -1;
}




/*____________________________________________________________________________________________________________________________
 *                          IMPOSTA LO STATO DELLO SPECCHIO
 *  I Possibili comandi sono: MIRROR_OUT, MIRROR_HOME
 *  La Funzione attiva l'attesa della notifica con l'esito del comando e
 *  con lo stato corrente. La notifica giunge al termine del comando
 */
void Collimatore::setMirror(_MirrorCmd_Enum cmd)
{
   unsigned char data;
   mirrorStat = MIRROR_ND; // Fino ad avvenuta notifica lo stato è indeterminato

   if(cmd == MIRROR_OUT) data = 1;
   else data = 0;
   pConsole->pGuiMcc->sendFrame(MCC_SET_MIRROR,_COLLI_ID,&data, 1);

}

void Collimatore::setToggleMirrorLamp(unsigned short steps)
{
    unsigned char data[2];
    mirrorStat = MIRROR_ND; // Fino ad avvenuta notifica lo stato e' indeterminato

    data[0] = (unsigned char) steps &0xFF;
    data[1] = (unsigned char) (steps>>8) &0xFF;

    pConsole->pGuiMcc->sendFrame(MCC_UPDATE_MIRROR_LAMP,_COLLI_ID,data, 2);

}

/*____________________________________________________________________________________________________________________________
 *      IMPOSTA LO STATO DELLA LAMPADA + SPECCHIO + Timeout lampada
 *
 *      cmd = Comando: LAMP_OFF,LAMP_ON,LAMPMIRR_OFF,LAMPMIRR_ON
 *                     LAMP_OFF: Spegne la lampada senza muovere lo specchio
 *                     LAMP_ON:  Accende la lampada senza muovere lo specchio
 *                     LAMPMIRR_OFF: Spegne la lampada e fa rientrare lo specchio
 *                     LAMPMIRR_ON:  Accende la lampada e fa uscire lo specchio in campo
 *      tmo = TIMEOUT accensione lampada:
 *            0 = infinito;
 *            1:64 = 0.5 secondi unit, max 32 secondi
 *
 *      La funzione attiva la notifica e al termine del comando
 *      viene impostato lo stato corrente della lampada e dello specchio
 */
void Collimatore::setLamp(_LampCmd_Enum cmd, unsigned char tmo)
{
    unsigned char data[4];

   // Imposta il comando
   data[0] = (unsigned char) cmd;
   data[1] = tmo&0x3F;

   if(pCollimatore->colli_model == _COLLI_TYPE_ASSY_01){
       data[2] = (unsigned char) (colliConf.mirrorSteps_ASSY_01&0xFF); // Step Specchio in campo
       data[3] = (unsigned char) ((colliConf.mirrorSteps_ASSY_01>>8)&0xFF); // Step Specchio in campo
   }else{
       data[2] = (unsigned char) (colliConf.mirrorSteps_ASSY_02&0xFF); // Step Specchio in campo
       data[3] = (unsigned char) ((colliConf.mirrorSteps_ASSY_02>>8)&0xFF); // Step Specchio in campo
   }
   lampStat = pConsole->pGuiMcc->sendFrame(MCC_SET_LAMP,_COLLI_ID,data, 4);

}


/*
 *  Questa funzione effettua l'impostazione di tutti i parametri di collimazione
 *  e serve per agevolare il relativo comando ricevuto dalla console
 *  param ==TRUE -> Ritira lo specchio
 *  force == TRUE Forza l'update
 *
 * STRUTTURA MESSAGGIO MCC
 *

   // Collimazione 2D:
   data[3] = Front;// front
   data[4] = Back;// back
   data[5] = Left;// Left Blade
   data[6] = Right;// Right Blade
   data[7] = Trap;// back Trap

   // Collimazione tomo
   data[3] = Front;// front
   data[4] = Back;// back
   data[5:15] = 0° .. 2°.. 20° Left
   data[16:25] = -2° .. -20°   Left
   data[26:36] = 0° .. 2°.. 20° Right
   data[37:46] = -2° .. -20°   Right
   data[47:52] = 0° .. 4°.. 20° Trap
   data[53:57] = -4° .. -20°   Trap


NOTIFY:
    data[0] = Successo/Insuccesso

 *
 */



unsigned char Collimatore::colliFormatFromPad(int pad){
     if(pad == PAD_24x30)   return    _COLLI_FORMAT_24x30;

     if(pad == PAD_18x24)   return    _COLLI_FORMAT_18x24;
     if(pad == PAD_D75_CNT) return    _COLLI_FORMAT_18x24;
     if(pad == PAD_BIOP_2D) return    _COLLI_FORMAT_18x24;
     if(pad == PAD_10x24)   return    _COLLI_FORMAT_18x24;

     if(pad == PAD_BIOP_3D) return    _COLLI_FORMAT_BIOPSY;

     if(pad == PAD_9x21)    return    _COLLI_FORMAT_MAGNIFIER;
     if(pad == PAD_D75_MAG) return    _COLLI_FORMAT_MAGNIFIER;
     return _COLLI_FORMAT_UNDEFINED;
}

bool Collimatore::updateColli(void)
{
    unsigned char data[COLLI_LEN];
    int colliIndex;
    QString mat;
    int pad;


    // Controlli preliminari sui comandi
    if(pConfig->collimator_configured==FALSE)
    {
        ApplicationDatabase.setData(_DB_COLLIMAZIONE,QString(QApplication::translate("COLLIMATORE","NON DEFINITA", 0, QApplication::UnicodeUTF8)),0);
        ApplicationDatabase.setData(_DB_COLLI_FORMAT,(unsigned char) _COLLI_FORMAT_UNDEFINED);
        return FALSE;
    }

    // Collimazione manuale attivata
    if(manualCollimation)
    {
        ApplicationDatabase.setData(_DB_COLLIMAZIONE,QString(QApplication::translate("COLLIMATORE","MANUALE", 0, QApplication::UnicodeUTF8)),0);
        ApplicationDatabase.setData(_DB_COLLI_FORMAT,(unsigned char) _COLLI_FORMAT_MANUAL);
        return TRUE;
    }

    // In modo operativo necessariamente ci deve essere una collimazione pre-definita.
    // Non � ammessa una collimazione OPEN
    if(pConsole->isOperatingMode())
    {
        // Se non c'� potter e non c'e la Biopsia non viene effettuata nessuna collimazione
        if((!pPotter->isValid())&&(pBiopsy->connected==FALSE)){

            ApplicationDatabase.setData(_DB_COLLIMAZIONE,QString(QApplication::translate("COLLIMATORE","NON DEFINITA", 0, QApplication::UnicodeUTF8)),0);
            ApplicationDatabase.setData(_DB_COLLI_FORMAT,(unsigned char) _COLLI_FORMAT_UNDEFINED);
            return FALSE;
        }

        pad = pCompressore->getPad();

        // Con il compressore sbloccato non viene effettuata nessuna collimazione
        if(pad>=PAD_ENUM_SIZE){
            ApplicationDatabase.setData(_DB_COLLIMAZIONE,QString(QApplication::translate("COLLIMATORE","NON DEFINITA", 0, QApplication::UnicodeUTF8)),0);
            ApplicationDatabase.setData(_DB_COLLI_FORMAT,(unsigned char) _COLLI_FORMAT_UNDEFINED);
            return FALSE;
        }

        // Deriva il codice di collimazione
        if(pad!=PAD_TOMO_24x30)  colliIndex = getColli2DIndex(pad);
        else colliIndex = getColli2DIndex(PAD_24x30);

        // Determinazione del formato associato ai pad
        ApplicationDatabase.setData(_DB_COLLI_FORMAT,(unsigned char) colliFormatFromPad(pad));


    }else
    {

        // In modalit�  non operativa utilizza la collimazione OPEN
        colliIndex=-1; // Collimazione OPEN in calibrazione
        ApplicationDatabase.setData(_DB_COLLIMAZIONE,QString(QApplication::translate("COLLIMATORE","APERTA", 0, QApplication::UnicodeUTF8)),0);
        ApplicationDatabase.setData(_DB_COLLI_FORMAT,(unsigned char) _COLLI_FORMAT_UNDEFINED);
    }


   // Formato
   if(colliIndex<0) // OPEN
   {
       // Apre tutto il collimatore
       data[COLLI_F] = colliConf.colliOpen.F;// front
       data[COLLI_B] = colliConf.colliOpen.B;// back
       data[COLLI_L] = colliConf.colliOpen.L;// Left Blade
       data[COLLI_R] = colliConf.colliOpen.R;// Right Blade
       data[COLLI_T] = colliConf.colliOpen.T;// back Trap
   }else
   {
       ApplicationDatabase.setData(_DB_COLLIMAZIONE,pCompressore->getPadName((Pad_Enum)pad),0);
       _colliPadStr colli2D = colliConf.colli2D.at(colliIndex);

       // Carica le posizioni
       data[COLLI_F] = colli2D.F;// front
       data[COLLI_B] = colli2D.B;// back
       data[COLLI_L] = colli2D.L;// Left Blade
       data[COLLI_R] = colli2D.R;// Right Blade
       data[COLLI_T] = colli2D.T;// back Trap

   }

   // Invio comando
   if(pConsole->pGuiMcc->sendFrame(MCC_SET_COLLI,_COLLI_ID,data, COLLI_LEN)==FALSE)
   {
       qDebug() << "MCC FALLITO";
       return FALSE;
   }

   return TRUE;
}

// Attiva un loop mdi formato open close per verificare il collimatore
void Collimatore::startColliTest(unsigned char nseq){

    if(nseq==0){
        // Ferma il test in corso
        colliTestNumber=1;
        return;
    }
    colliTestNumber = nseq;
    colliTestStatus = true;
    manualCollimation = true;
    pCollimatore->manualF = 0;
    pCollimatore->manualB = 0;
    pCollimatore->manualL = 0;
    pCollimatore->manualR = 0;
    manualColliUpdate();

    colliTestTimer = startTimer(7000);
}

// Notifica di avvenuto cambio PAD: effettua un update della collimazione corrente
void Collimatore::changedPadNotify(void)
{
    updateColli();

}

// Imposta il filtro utilizzando una tag tra quelle definite
// cmd � uno tra i codici filtro disponibili
// update==true impone la selezione del filtro
bool Collimatore::setFiltro(_FilterCmd_Enum cmd, bool update)
{
    unsigned char data[2];
    int i;

    // Collimazione manuale attivata
    if(manualFiltroCollimation)
    {
        qDebug() << "setFiltro: collimatore in modalit�  manuale";
        return TRUE;
    }

    // Controlli preliminari sui comandi
    if(pConfig->collimator_configured==FALSE)
    {
        qDebug() << "setFiltro: collimatore non configurato";
        return FALSE;
    }

    // Verifica se il filtro esiste
    for(i=0;i<4;i++) if(pCollimatore->colliConf.filterType[i] == cmd) break;
    if(i==4) return false;

    // Il filtro esiste
    filtroCmd = cmd;

    // non procede con l'effettiva impostazione del filtro (successivamente occorrer� un udate
    if(update==false) return true;
    if(filtroStat == filtroCmd) return true; // Filtro gi� selezionato

    data[0] = i;
    data[1] = pCollimatore->colliConf.filterPos[i];

    // Invio comando
    if(pConsole->pGuiMcc->sendFrame(MCC_SET_FILTRO,_COLLI_ID,data, sizeof(data))==FALSE)
    {
        qDebug() << "MCC FALLITO";
        return FALSE;
    }

    return TRUE;

}

/* Imposta esclusivamente il filtro */
bool Collimatore::setFiltro(void)
{
    unsigned char data[2];
    int i;

    // Collimazione manuale attivata
    if(manualFiltroCollimation)
    {
        qDebug() << "setFiltro: collimatore in modalit�  manuale";
        return TRUE;
    }

    // Controlli preliminari sui comandi
    if(pConfig->collimator_configured==FALSE)
    {
        qDebug() << "setFiltro: collimatore non configurato";
        return FALSE;
    }

    // Se il filtro non è definito, non procede con l'impostazione
    if(filtroCmd == FILTRO_ND) return TRUE;

    // Selezione la posizione del filtro
    for(i=0;i<4;i++) if(pCollimatore->colliConf.filterType[i] == filtroCmd) break;
    data[0] = i;
    data[1] = pCollimatore->colliConf.filterPos[i];

    // Invio comando
    if(pConsole->pGuiMcc->sendFrame(MCC_SET_FILTRO,_COLLI_ID,data, sizeof(data))==FALSE)
    {
        qDebug() << "MCC FALLITO";
        return FALSE;
    }

    return TRUE;

}

QString Collimatore::getFiltroTag(unsigned char code)
{
    if(code == FILTRO_Ag) return "Ag";
    else if(code == FILTRO_Rh) return "Rh";
    else if(code == FILTRO_Al) return "Al";
    else if(code == FILTRO_Cu) return "Cu";
    else if(code == FILTRO_Mo) return "Mo";
    else return "";
}


/* Imposta esclusivamente il filtro */
bool Collimatore::manualSetFiltro(void)
{
    unsigned char data[2];
    int i;

    // Controlli preliminari sui comandi
    if(pConfig->collimator_configured==FALSE)
    {
        qDebug() << "setFiltro: collimatore non configurato";
        return FALSE;
    }

    // Collimazione manuale attivata
    if(!manualFiltroCollimation)
    {
        qDebug() << "manualSetFiltro: collimatore in AUTOMATICO";
        return FALSE;
    }

    // Selezione la posizione del filtro
    for(i=0;i<4;i++) if(pCollimatore->colliConf.filterType[i] == manualFilter) break;
    data[0] = i;
    data[1] = pCollimatore->colliConf.filterPos[i];

    // Invio comando
    return pConsole->pGuiMcc->sendFrame(MCC_SET_FILTRO,_COLLI_ID,data, sizeof(data));

}

// Imposta il filtro in base alla posizione numerica da 0 a 3
// il codice del filtro relativo viene scritto di conseguenza in funzione dell'impostazione corrente
bool Collimatore::manualSetFiltro(unsigned char index)
{
    unsigned char data[2];

    if(index>3) return false;

    // Collimazione manuale attivata
    if(!manualFiltroCollimation)
    {
        return FALSE;
    }

    // Selezione la posizione del filtro
    data[0] = index;
    data[1] = pCollimatore->colliConf.filterPos[index];

    // Invio comando
    if(pConsole->pGuiMcc->sendFrame(MCC_SET_FILTRO,_COLLI_ID,data, sizeof(data))==FALSE)
    {
        return FALSE;
    }


    manualFilter = pCollimatore->colliConf.filterType[index];
    return TRUE;

}

bool Collimatore::manualColliUpdate(void)
{
    unsigned char data[COLLI_LEN];

    // Collimazione manuale attivata
    if(manualCollimation==FALSE) return FALSE;

    // Imposta a display la collimazione Manuale
    ApplicationDatabase.setData(_DB_COLLIMAZIONE,QString("MANUALE"),0);

    // Apre tutto il collimatore
    data[COLLI_F] = manualF;// front
    data[COLLI_B] = manualB;// back
    data[COLLI_L] = manualL;// Left Blade
    data[COLLI_R] = manualR;// Right Blade
    data[COLLI_T] = manualT;// back Trap

    // Invio comando
    return pConsole->pGuiMcc->sendFrame(MCC_SET_COLLI,_COLLI_ID,data, COLLI_LEN);
}

void Collimatore::guiNotifySlot(unsigned char id, unsigned char mcccode, QByteArray buffer)
{
    if(id!=_COLLI_ID) return;
    switch(mcccode)
    {
        case MCC_CALIB_FILTRO:
            // Notifica delle posizioni correnti sul device PCB249U2 del filtro
            // Verifica se è necessario salvare nel file di configurazione i nuovi valori
            // pCollimatore->colliConf.filterPos[4] indica se i dati si riferiscono ad un device
            // calibrato. Se non è cosi' essi NON devono essere salvati
            pCollimatore->colliConf.filterPos[0] = buffer.at(0);
            pCollimatore->colliConf.filterPos[1] = buffer.at(1);
            pCollimatore->colliConf.filterPos[2] = buffer.at(2);
            pCollimatore->colliConf.filterPos[3] = buffer.at(3);
            pCollimatore->storeConfigFile();
            break;
        case MCC_SET_LAMP:
            if(buffer.at(1)==1) lampStat=TRUE;
            else lampStat=FALSE;

            if(buffer.at(2)==0) mirrorStat = MIRROR_HOME;
            else if(buffer.at(2)==1) mirrorStat = MIRROR_OUT;
            else  mirrorStat = MIRROR_ND;

            if(buffer.at(0)==0)
            {
                // CONDIZIONE DI ERRORE
                PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_COLLI, COLLI_LAMP_FALLITO, TRUE);
                return;
            }
        break;

    case MCC_SET_FILTRO:
        if(buffer.at(0)==0)
        {
            // CONDIZIONE DI ERRORE
            PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_COLLI, COLLI_FILTRO_FALLITO, TRUE);
            return;
        }else
        {
            if(manualFiltroCollimation)
                filtroStat = (_FilterCmd_Enum) manualFilter;
            else
                filtroStat = filtroCmd; // Aggiorna lo stato del filtro
        }
    break;

    case MCC_SET_COLLI: // Notifica dal processo di collimazione
        // buffer.at(0) = esito collimazione: 1=OK, 0=NOK
        // buffer.at(1) = processo di collimazione: 1= laterali, 0=frontali+back
        // buffer.at(2) = left/frontf;
        // buffer.at(3) = right/back
        // buffer.at(4) = trap

        if(buffer.at(1)==1){
            // Collimazione lame laterali
            if(buffer.at(0)==0){
               PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_COLLI, COLLI_UPDATE_FALLITO, TRUE);
            }
        }else{
            // Collimazine lama frontale + back
            if(buffer.at(0)==0){
               PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_COLLI, COLLI_UPDATE_FALLITO, TRUE);
            }
        }

    break;

    case MCC_SET_MIRROR:
        if(buffer.at(0)==0)
        {
            // Notifica l'errore a monitor
            PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_COLLI, COLLI_SPECCHIO_FALLITO, TRUE);
            return;
        }
    break;

    }

    return;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  CONFIGURATION FILES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool Collimatore::readConfigFile(void)
{
    QString filename;
    QList<QString> dati;
    int i=0;
    int pad;
    int fileRevision = 0;

    filename =  QString("/resource/config/collimazione_analogica.cnf");
    QFile file(filename.toAscii());
    if(!file.exists()) return false;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    // Init di tutte le grandezze legate al collimatore a valori di default
    colliConf.filterType[0]=FILTRO_Rh;
    colliConf.filterType[1]=FILTRO_Mo;
    colliConf.filterType[2]=FILTRO_Al;
    colliConf.filterType[3]=FILTRO_Cu;

    colliConf.filterPos[0] = 39;
    colliConf.filterPos[1] = 97;
    colliConf.filterPos[2] = 163;
    colliConf.filterPos[3] = 224;

    colliConf.mirrorSteps_ASSY_01 = _FACTORY_MIRROR_STEPS_ASSY01;
    colliConf.mirrorSteps_ASSY_02 = _FACTORY_MIRROR_STEPS_ASSY02;

    colliConf.colli2D.clear();
    colliConf.colliOpen.PadCode = PAD_ENUM_SIZE;
    colliConf.colliOpen.L = 0;
    colliConf.colliOpen.R = 0;
    colliConf.colliOpen.F = 0;
    colliConf.colliOpen.B = 0;
    colliConf.colliOpen.T = 45;

    colliConf.colliTomoMo.enabled=false;
    colliConf.colliTomoW.enabled=false;

    // Imposta collimazione Custom a 18x24
    customL = 0;
    customR = 0;
    customT = 0;
    customB = 0;
    customF = 0;



    // Procede con la lettura del formato corrente
    while(1){

        dati = Config::getNextArrayFields(&file);
        if(dati.isEmpty()) break;

        if(dati.at(0)=="REV"){
            fileRevision = dati.at(1).toInt();
        }

        // Assegnazione tipologia filtri
        if(dati.at(0)=="FILTRI"){
            for(i=1;i<5;i++)
            {
                if(dati.at(i)=="Rh") colliConf.filterType[i-1]=FILTRO_Rh;
                else if(dati.at(i)=="Ag") colliConf.filterType[i-1]=FILTRO_Ag;
                else if(dati.at(i)=="Al") colliConf.filterType[i-1]=FILTRO_Al;
                else if(dati.at(i)=="Mo") colliConf.filterType[i-1]=FILTRO_Mo;
                else if(dati.at(i)=="Cu") colliConf.filterType[i-1]=FILTRO_Cu;
                else colliConf.filterType[i-1]=FILTRO_ND; // Filtro non configurato
            }
            continue;
        }

        // Configurazione posizione slots filtri
        if(dati.at(0)=="PFILTRI"){
            for(i=0;i<4;i++) colliConf.filterPos[i] = (unsigned char) dati.at(i+1).toInt();
            continue;
        }

        // Configurazione step specchio
        if(dati.at(0)=="SPECCHIO"){
            colliConf.mirrorSteps_ASSY_01 = dati.at(1).toInt();

            // Aggiunge gli steps in campo per il nuovo assy
            if(dati.size()==3)
                colliConf.mirrorSteps_ASSY_02 = dati.at(2).toInt();
            continue;
        }

        // Collimazione OPEN
        if(dati.at(0)=="OPEN"){
            colliConf.colliOpen.PadCode = PAD_ENUM_SIZE;
            colliConf.colliOpen.L = (unsigned char) dati.at(1).toInt();
            colliConf.colliOpen.R = (unsigned char) dati.at(2).toInt();
            colliConf.colliOpen.F = (unsigned char) dati.at(3).toInt();
            colliConf.colliOpen.B = (unsigned char) dati.at(4).toInt();
            colliConf.colliOpen.T = (unsigned char) dati.at(5).toInt();
            continue;
        }

        // Collimazione CUSTOM
        if(dati.at(0)=="CUSTOM"){
            customL = (unsigned char) dati.at(1).toInt();
            customR = (unsigned char) dati.at(2).toInt();
            customF = (unsigned char) dati.at(3).toInt();
            customB = (unsigned char) dati.at(4).toInt();
            customT = (unsigned char) dati.at(5).toInt();
            continue;
        }

        // ---------------------------- Collimazioni 2D --------------------------------------- //
        if(dati.at(0).contains("PAD")){

            // Legge il codice numerico del PAD associato al tag alfanumerico
            pad = pCompressore->getPadCodeFromTag(dati.at(0));
            if(pad==-1) continue;

            // Verifica se è una duplicazione o se il numero di parametri non è corretto
            if((getColli2DIndex(pad)!=-1) ||(dati.size()!=7)) continue;

            // Crea il nuovo item di collimazione
            _colliPadStr newColli2DItem;
            newColli2DItem.PadCode = pad;
            newColli2DItem.L = (unsigned char) dati.at(2).toInt();
            newColli2DItem.R = (unsigned char) dati.at(3).toInt();
            newColli2DItem.F = (unsigned char) dati.at(4).toInt();
            newColli2DItem.B = (unsigned char) dati.at(5).toInt();
            newColli2DItem.T = (unsigned char) dati.at(6).toInt();
            colliConf.colli2D.append(newColli2DItem);
            continue;
        }
    }


    // C'� stata una variazione di revisione
    if( fileRevision != COLLI_CNF_REV){
        // Salvataggio
        storeConfigFile();
    }

    file.close();
    return true;
}

bool Collimatore::storeConfigFile(void)
{
    QString filename;
    QString command;
    QString data;
    int i=0;

    filename =  QString("/resource/config/collimazione_analogica.cnf");
    QFile filecpy(filename.toAscii());
    if (!filecpy.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    // Salva la revisione
    data = QString("<REV,%1>\n").arg(COLLI_CNF_REV);
    filecpy.write(data.toAscii().data());


    data = QString("<FILTRI,%1,%2,%3,%4>\n").arg(getFilterTag(colliConf.filterType[0])).arg(getFilterTag(colliConf.filterType[1])).arg(getFilterTag(colliConf.filterType[2])).arg(getFilterTag(colliConf.filterType[3]));
    filecpy.write(data.toAscii().data());

    data = QString("<PFILTRI,%1,%2,%3,%4>\n").arg(colliConf.filterPos[0]).arg(colliConf.filterPos[1]).arg(colliConf.filterPos[2]).arg(colliConf.filterPos[3]);
    filecpy.write(data.toAscii().data());

    data = QString("<SPECCHIO,%1,%2>\n\n").arg(colliConf.mirrorSteps_ASSY_01).arg(colliConf.mirrorSteps_ASSY_02);
    filecpy.write(data.toAscii().data());

    // Collimazione 2D
    for(i=0; i<colliConf.colli2D.size();i++)
    {
        data = QString("<%1,%2,%3,%4,%5,%6,%7>\n").arg(pCompressore->getPadTag((Pad_Enum) (colliConf.colli2D[i].PadCode))).arg(QString("-")).arg((int) colliConf.colli2D[i].L).arg((int) colliConf.colli2D[i].R).arg((int) colliConf.colli2D[i].F).arg((int) colliConf.colli2D[i].B).arg((int) colliConf.colli2D[i].T);
        filecpy.write(data.toAscii().data());
    }

    // Collimazione OPEN
    data = QString("<OPEN,%1,%2,%3,%4,%5>\n\n").arg((int) colliConf.colliOpen.L).arg((int) colliConf.colliOpen.R).arg((int) colliConf.colliOpen.F).arg((int) colliConf.colliOpen.B).arg((int) colliConf.colliOpen.T);
    filecpy.write(data.toAscii().data());

    // Collimazione CUSTOM
    data = QString("<CUSTOM,%1,%2,%3,%4,%5>\n\n").arg((int) customL).arg((int) customR).arg((int) customF).arg((int) customB).arg((int) customT);
    filecpy.write(data.toAscii().data());


    filecpy.flush();
    filecpy.close();

    PRINT("COLLIMATION FILE CHANGED!");
    pSysLog->log("CONFIG: ANALOG COLLIMATION FILE");

    command = QString("sync");
    system(command.toStdString().c_str());
    return TRUE;
}
