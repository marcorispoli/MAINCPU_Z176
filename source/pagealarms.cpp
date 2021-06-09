#include "application.h"
#include "appinclude.h"
#include "globvar.h"
#include "shared_a5_m4/shared.h"

#include "systemlog.h"
extern systemLog* pSysLog;

#define _PG_ALARM_SCROLL_PIX       "://AlarmPage/AlarmPage/ScrollPix.png"
#define _PG_ALARM_SCROLL_PATH      8,378,50,478,50,478,150,378,150
#define _PG_ALARM_SCROLL_POS       378,50

#define _PG_ALARM_UPDATE_BUTTON_PIX       "://AlarmPage/AlarmPage/bottone_update.png"
#define _PG_ALARM_UPDATE_BUTTON_PATH      8,611,3,766,3,766,116,611,116
#define _PG_ALARM_UPDATE_BUTTON_POS       611,3


#define _PG_ALARM_INTEST_LABEL     117,10,570,44
#define _PG_ALARM_CODE_LABEL       38,122,430,270

// Pix relativi agli allarmi
#define _PG_ALARM_UNLOCK_COMPR_PIX  "://AlarmPage/AlarmPage/Unlocked_Compressor.png"
#define _PG_ALARM_UNLOCK_COMPR_POS  492,122

#define _PG_ALARM_UNLOCK_PAD_PIX    "://AlarmPage/AlarmPage/Unlocked_Pad.png"
#define _PG_ALARM_UNLOCK_PAD_POS    492,122

#define _PG_ALARM_WRONG_PAD_PIX      "://AlarmPage/AlarmPage/Wrong_Pad.png"

#define _PG_ALARM_PROT_PB_PIX          "://AlarmPage/AlarmPage/ProtezionePiombo.png"
#define _PG_ALARM_PROT_PB_POS  492,122
#define _PG_ALARM_PROT_2D_PIX          "://AlarmPage/AlarmPage/Protezione2D.png"
#define _PG_ALARM_PROT_2D_POS  492,122
#define _PG_ALARM_FANTOCCIO_CALIB_PIX  "://AlarmPage/AlarmPage/FantoccioCalibrazione.png"
#define _PG_ALARM_FANTOCCIO_CALIB_POS  492,122


#define ALR_FAULT_TAMP_PIX "://fault_tamp_pix.png"
#define ALR_FAULT_GND_PIX "://fault_gnd_pix.png"
#define ALR_FAULT_R16_PIX "://fault_r16_pix.png"
#define ALR_FAULT_MAS_PIX "://fault_mAs_pix.png"
#define ALR_FAULT_HVCALIB_PIX "://fault_hvcalib_pix.png"
#define ALR_FAULT_HV_PIX "://fault_hv_pix.png"
#define ALR_FAULT_IFIL_PIX "://fault_ifil_pix.png"

QPixmap appendPix1Pix2(QPixmap p1, QPixmap p2, int offset){
    if(p1.width()==0) offset = 0;
    QPixmap ris(qMax(p1.width(),p2.width()), offset + 1 + p1.height() + p2.height());
    ris.fill(Qt::transparent);
    QPainter p;
    p.begin(&ris);
    p.drawPixmap(0,0,p1);
    p.drawPixmap(0,offset + p1.height(),p2);
    p.end();
    return ris;
}

QPixmap concatPix1Pix2(QPixmap p1, QPixmap p2, int offset){
    if(p1.height()==0) offset = 0;
    QPixmap ris(p1.width() + p2.width() + offset, qMax(p1.height(),p2.height()));
    ris.fill(Qt::transparent);
    QPainter p;
    p.begin(&ris);
    p.drawPixmap(0,0,p1);
    p.drawPixmap(offset + p1.width(),0, p2);
    p.end();
    return ris;
}

QPixmap matrixColPixmap(QPixmap pix, int C, bool reset){
    static QPixmap rowfull(0,0);
    static QPixmap rowparz(0,0);
    static int col =0;

    if(reset) {
       col=0;
       rowfull = QPixmap(0,0);
       rowparz = QPixmap(0,0);
       return pix;
    }

    col++;
    if(col<=C)
    {
        rowparz = concatPix1Pix2(rowparz, pix, 10);
    }else {
        rowfull = appendPix1Pix2(rowfull,rowparz,10);
        rowparz = pix;
        col=1;
    }
    return appendPix1Pix2(rowfull,rowparz,10);

}

PageAlarms::PageAlarms(QString bg ,bool showLogo, int w,int h, qreal angolo,QPainterPath pn, int pgpn, QPainterPath pp, int pgpp, int pg) : GWindow(bg,showLogo,w,h, angolo,pn,pgpn,pp,pgpp,pg)
{
    QFont font;
    QPen  pen;


    timerId=0;
    setBackground(bg);

    // Disabilita il pulsante di avanti
    nextPageEnabled = false;

    // Pixmap di allarme prima di tutti
    activePix = this->addPixmap(QPixmap(0,0));
    activePix->hide();

    // Definizione del testo per la data
    font.setFamily("DejaVu Serif");
    font.setBold(true);
    font.setWeight(75);
    font.setItalic(false);
    font.setPointSize(22);
    font.setStretch(60);

    dateText=this->addText("----------",font);
    dateText->setDefaultTextColor(Qt::white);
    dateText->setPos(DATE_LABEL_POSITION);


    // Definizione del testo per la data
    font.setFamily("DejaVuSerif");
    font.setBold(true);
    font.setWeight(90);
    font.setItalic(false);
    font.setPointSize(70);
    font.setStretch(50);

    // Campo Intestazione
    font.setPointSize(30);
    font.setStretch(40);
    intestazioneValue = new GLabel(this,QRectF(_PG_ALARM_INTEST_LABEL ),font,QColor(_R_COL),"",Qt::AlignCenter);
    //this->setIntestazione();

    pulsanteScorriAllarmi = new GPush((GWindow*) this, QPixmap(_PG_ALARM_SCROLL_PIX),QPixmap(_PG_ALARM_SCROLL_PIX),setPointPath(_PG_ALARM_SCROLL_PATH),_PG_ALARM_SCROLL_POS,0,0);
    pulsanteScorriAllarmi->setEnable(false);

    pulsanteUpdateRevision = new GPush((GWindow*) this, QPixmap(_PG_ALARM_UPDATE_BUTTON_PIX),QPixmap(_PG_ALARM_UPDATE_BUTTON_PIX),setPointPath(_PG_ALARM_UPDATE_BUTTON_PATH),_PG_ALARM_UPDATE_BUTTON_POS,0,0);
    pulsanteUpdateRevision->setEnable(false);


    // Campo Alarm label
    #define _ORANGE_COL 245,177,18
    font.setPointSize(28);
    font.setStretch(30);
    alarmLabel = new GLabel(this,QRectF(_PG_ALARM_CODE_LABEL),font,QColor(_Y_COL),"",Qt::AlignCenter);

    // Campo info label
    font.setPointSize(23);
    font.setStretch(30);
    infoLabel = new GLabel(this,QRectF(481,88,278,294),font,QColor(_W_TEXT),"",Qt::AlignTop);

    // Funzione chiamata al cambio pagina: viene chiamata in fase di init della pagina manualmente
    childStatusPage(status,1);

    timerId = startTimer(1000);
    timerPg=0;
    alarm_enable = true;
    curClass=0;
    numAlarm = 0;


    createMessageList();

}

PageAlarms::~PageAlarms()
{
    this->killTimer(timerId);
    if(timerPg) killTimer(timerPg);
}

// Questa funzione viene chiamata ogni volta che viene ricevuto il segnale di cambio
// pagina dalla Classe Base. Viene utilizzata per effettuare tutte le inizializzazioni del caso
void PageAlarms::childStatusPage(bool stat,int opt)
{

    // Update Data field
    if(stat)
    {

        this->prevPage = GWindowRoot.parentPage;        
        setWindow();

        if(systemTimeUpdated)
            dateText->setPlainText(QDateTime::currentDateTime().toString("dd.MM.yy     hh.mm.ss ap"));
        else
            dateText->setPlainText(QString("--.--.--     --.--.--"));
    }else
    {       
        if(numAlarm){
            // All'uscita cancella tutti gli allarmi auto-ripristinanti
            for(int i=FIRST_ALR_CLASS; i<=LAST_ALR_CLASS;i++)
            {
                if(ApplicationDatabase.getDataI(i)&0xFF00) ApplicationDatabase.setData(i,(int)0, DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
            }

            refreshAlarmStatus();
            curClass=0;
        }

        // Quando si lascia la pagina allarmi bisogna annullare tutte le condizioni di allarme che abbiano
        // il self reset attivo
        if(timerPg) killTimer(timerPg);
        timerPg=0;
    }


}


void PageAlarms::timerEvent(QTimerEvent* ev)
{

    // Dopo un certo tempo dall'apertura della finestra, questa viene chiusa
    // Alla chiusura vengono azzrati tutti gli allarmi one-shot
    if(ev->timerId()==timerPg)
    {
        if(this->isCurrentPage()&&(isMaster)) this->prevPageHandler();

    }

    // Timer interno della classe, attivo ogni secondo
    if(ev->timerId()==timerId)
    {
        if(systemTimeUpdated)
            dateText->setPlainText(QDateTime::currentDateTime().toString("dd.MM.yy     hh.mm.ss ap"));
        else
            dateText->setPlainText(QString("--.--.--     --.--.--"));
    }


}


void PageAlarms::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    GWindow::mousePressEvent(event); // Lancia il default della classe

}


void PageAlarms::setWindow(void){

    int classe=curClass;
    int allarme,i;

    if(!numAlarm) return;

    if((timerPg)) killTimer(timerPg);
    timerPg = startTimer(20000);

    if(!classe){
        // Si seleziona uno tra quelli ancora attivi
        for(i=FIRST_ALR_CLASS; i<=LAST_ALR_CLASS;i++){
            if((0xFF&ApplicationDatabase.getDataI(i))) {classe = i;break;}
        }
        if(i>LAST_ALR_CLASS) return;
    }

    allarme = (0xFF&ApplicationDatabase.getDataI(classe));
    curClass=classe;

    // Impostazione grafica
    _alarmStruct* pErr = setErrorWindow(classe,allarme);
    if(pErr==0) return;
    if(!isMaster) return;

    // Preparazione messaggio per AWS:
    if(!newAlarm) return;
    newAlarm=false;

    emit newAlarmSgn(pErr->codestr.toInt(),pErr->errmsg);
    return;

}


unsigned char PageAlarms::refreshAlarmStatus(void){

    int num=0;
    for(int i=FIRST_ALR_CLASS; i<=LAST_ALR_CLASS;i++)
    {
        if(0xFF&ApplicationDatabase.getDataI(i)) num++;
    }



    if(isMaster) ApplicationDatabase.setData(_DB_NALLARMI_ATTIVI,(unsigned char) num,0);
    numAlarm = num;
    return num;

}

// FUNZIONE DI AGGIORNAMENTO CAMPI VALORE CONNESSO AI CAMPI DEL DATABASE
void PageAlarms::valueChanged(int index,int opt)
{
    int allarme;
    int num;

    if((index>=FIRST_ALR_CLASS)&&(index<=LAST_ALR_CLASS))
    {
        // Verifica quanti allarmi sono presenti
        num=refreshAlarmStatus();

        // Nessun allarme  attivo: chiusura pagina
        if((isMaster)&&(!num)&&(this->isCurrentPage())){
            this->prevPageHandler();
            return;
        }

        // valore relativo all'allarme cambiato
        allarme = (0xFF&ApplicationDatabase.getDataI(index));

        // Se la variazione riguarda l'attivazione di un allarme allora lo mostra
        if(allarme!=0){
            newAlarm=true;
            curClass = index;
            if(isMaster){
                _alarmStruct* pAlarm = this->getErrorInfo(curClass,allarme);
                if(pAlarm)  pSysLog->log(QString("ERROR - M%1").arg(pAlarm->codestr));
            }

            if((alarm_enable)&&(!(opt & DBase::_DB_NO_ACTION))) {
                this->activatePage(DBase::_DB_NO_ECHO);
            }
        }else{
            curClass=0;
            newAlarm=false;
            setWindow();
        }

        // Verifica se deve mostrare la pixmap di scorrimento sul pannello allarmi
        if(num>1) pulsanteScorriAllarmi->setEnable(true);
        else pulsanteScorriAllarmi->setEnable(false);

        return;
    }

    // Gestione della pressione del pulsante update (solo se la finestra allarme è presente
    // L'operazione, per motivi di sicurezza, può essere effettuata una volta sola!

    if(isCurrentPage()){
        if(index==_DB_SERVICE1_INT){
            if(ApplicationDatabase.getDataI(index)==100){
                pulsanteUpdateRevision->setEnable(false);
                ApplicationDatabase.setData(index, (int) 255,DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO); // Blocca ogni possibile altra transizione

                // Il Master da inizio alle operazioni
                if(pConfig->isMaster) pConfig->onInstallPackage();
            }
            return;
        }
    }

}


void PageAlarms::buttonActivationNotify(int id, bool status,int opt)
{
    int i,codice;


    GPush* pbutton = (GPush*) GWindowRoot.pushList.at(id);
    if(pbutton->parentWindow!=this) return; // Scarta i segnali da altre pagine

    // Pulsante di Update Attivato da uno dei terminali
    if(pbutton==pulsanteUpdateRevision)
    {
        if(status==false) return;

        if( ApplicationDatabase.getDataI(_DB_SERVICE1_INT) != 255 ) {
            ApplicationDatabase.setData(_DB_SERVICE1_INT, (int) 100);
        }
    }


    if(numAlarm<2) return;

    if(pbutton==pulsanteScorriAllarmi)
    {

        if(curClass<FIRST_ALR_CLASS) curClass = 0;
        if(curClass>LAST_ALR_CLASS)  curClass = 0;
        if(!curClass) return;
        i=curClass+1;
        if(i>LAST_ALR_CLASS) i = FIRST_ALR_CLASS;
        while(i!=curClass)
        {
            codice = (0xFF&ApplicationDatabase.getDataI(i));
            if(codice)
            {
                curClass = i;
                newAlarm=false;
                setWindow();
                return;
            }else i++;
            if(i>LAST_ALR_CLASS) i = FIRST_ALR_CLASS;
        }
        return;
    }

}

// Rinfresca tutte le label cambiate
void PageAlarms::languageChanged()
{
    // setIntestazione();

    // Rinfresca le labels
    alarmLabel->labelText=QApplication::translate("PAGINA-ERRORI","");
    alarmLabel->update();

    createMessageList();
}


bool PageAlarms::isAlarmOn(int classe)
{
    int codice = (0xFF&ApplicationDatabase.getDataI(classe));
    if(codice!=0) return true;

    return false;
}




// Formatta la stringa di errore
#define MAX_FORMAT_WORDS   55
#define MAX_FORMAT_ROW      5
QString PageAlarms::Format(QString code, QString stringa){
    int i,len,tag,row;

    // Nel messaggio: in testa c'è [M:00000]\n
    // Poi il titolo \n
    // Poi il corpo del messaggio

    // Il primo \n non deve essere tolto poichÃ¨ identifica il titolo del messaggio.
    // Tutti i successivi vengono invece rivisti e corretti.
    i = stringa.indexOf('\n',0);
    if(i>=0){
        i = stringa.indexOf('\n',i+1); // Cerca il secondo
        if(i>=0){
            QString stringa1 = stringa.right(stringa.size()-i-1);
            stringa = stringa.left(i+1);
            stringa1.replace("\n"," ");
            stringa.append(stringa1);
        }
    }

    // Formattazione rispetto alla lunghezza massima
    for(i=0,len=0,row=2; i<stringa.size();i++){
        if(stringa.at(i)=='\n') {
            len =0;
            row++;
        }

        if((stringa.at(i)==' ')||(stringa.at(i)==',')||(stringa.at(i)==';')||(stringa.at(i)=='.')) tag = i;
        if(++len > MAX_FORMAT_WORDS){
            if(++row>MAX_FORMAT_ROW){
                if(MAX_FORMAT_WORDS)
                stringa.insert(i-3,"...");
                stringa = stringa.left(i);
                break;
            }
            stringa.insert(tag+1,"\n");
            i=tag+1;
            len=0;
        }
    }

    // Attenzione, cambiando il codice del prefisso bisogna cambiare anche la funzione FormatExcel
    return stringa;
    // return QString("[M:")+QString(code) + QString("]\n") + stringa;
}

// Separa le colonne del codice errore, Titolo, Contenuto
QString PageAlarms::FormatExcel(QString stringa,char separatore){
    stringa[9] = separatore;
    int i = stringa.indexOf("\n");
    if(i>=0) stringa[i] = separatore;
    stringa.replace("\n"," ");
    stringa.append("\n");
    return stringa;
}


QString PageAlarms::getErrorString(int classe, int code){
    unsigned short index=classe-FIRST_ALR_CLASS;
    if(index>=errors.size()) return "";

    for(int i=0; i<errors[index].errlist.size(); i++){
        if(code==errors[index].errlist[i].codeval){
            // Impostazione stringa errore
            return Format(errors[index].errlist[i].codestr,errors[index].errlist[i].errmsg);
        }
    }
    return "";
}

/*
 *
 *     QString   codestr;  // Codice stringa assegnato all'errore
      int       codeval;  // Codice numerico relativo all'errore
      QString   errmsg;   // Messaggio in lingua
      QPixmap   errpix;   // Pixmap associato all'errore
      QString   errdescr; // (opzionale) descrizione dell'errore
 */
PageAlarms::_alarmStruct*  PageAlarms::setErrorWindow(int classe, int code)
{
    unsigned short index=classe-FIRST_ALR_CLASS;
    if(index>=errors.size()) return 0;

    for(int i=0; i<errors[index].errlist.size(); i++){

        if(code==errors[index].errlist[i].codeval){

            // Impostazione Pixmap
            activePix->setPixmap(errors[index].errlist[i].errpix);
            activePix->show();

            // CASO PARTICOLARE DI ERRORE REVISIONE
            if((classe==_DB_ALLARMI_ALR_SOFT)&&(code==ERROR_REVISIONS)){
                // Abilitazione pulsante di update
                if(ApplicationDatabase.getDataS(_DB_SERVICE1_STR).contains("PACKAGE_PRESENT")) pulsanteUpdateRevision->setEnable(true);
                else pulsanteUpdateRevision->setEnable(false);

                infoLabel->labelText=ApplicationDatabase.getDataS(_DB_REVISION_ERROR_STRING);
                infoLabel->show();
                activePix->setPos(464,70);
                if((timerPg)) killTimer(timerPg); // Allunga ad un'ora il tempo di mantenimento della prima visualizzazione
                timerPg = startTimer(3600000);
            }else{

                infoLabel->hide();
                activePix->setPos(624-errors[index].errlist[i].errpix.width()/2,255-errors[index].errlist[i].errpix.height()/2);
            }

            // Impostazione codice stringa errore
            intestazioneValue->labelText=errors[index].errlist[i].codestr;
            intestazioneValue->update();

            // Impostazione stringa errore
            alarmLabel->labelText=Format(errors[index].errlist[i].codestr,errors[index].errlist[i].errmsg);
            alarmLabel->update();
            return &(errors[index].errlist[i]);
        }
    }

    return 0;
}


// Restituisce la struttura errore associata al codice
PageAlarms::_alarmStruct* PageAlarms::getErrorInfo(int code){
    for(int i=0; i<errors.size();i++){
        for(int ii=0; ii<errors[i].errlist.size();ii++){
            if(errors[i].errlist[ii].codestr.toInt()==code) return &(errors[i].errlist[ii]);
        }
    }

    return 0;
}

// Restituisce la struttura errore associata alla coppia classe codice
PageAlarms::_alarmStruct* PageAlarms::getErrorInfo(int classe, int code){
    unsigned short index=classe-FIRST_ALR_CLASS;
    if(index>=errors.size()) return 0;

    for(int i=0; i<errors[index].errlist.size(); i++){
        if(code==errors[index].errlist[i].codeval) return &(errors[index].errlist[i]);
    }

    return 0;
}

/*__________________________________________________________________________________________________________________
 *          ATTIVAZIONE/DISATTIVAZIONE DI UN ALLARME NON AUTORIPRISTINABILE
 */
bool PageAlarms::activateNewAlarm(int classe, int codice)
{
#ifdef __DISABLE_ERRORS
    return true;
#endif

    ApplicationDatabase.setData(classe,((int)codice&0x00FF), 0);

    if(codice==0) return FALSE;
    else return TRUE;


}

/*__________________________________________________________________________________________________________________
 *          ATTIVAZIONE/DISATTIVAZIONE DI UN ALLARME NON AUTORIPRISTINABILE SENZA APERTURA DELLA WINDOW
 */
bool PageAlarms::addNewAlarm(int classe, int codice)
{
#ifdef __DISABLE_ERRORS
    return true;
#endif

    ApplicationDatabase.setData(classe,((int)codice&0x00FF), DBase::_DB_NO_ACTION);


    if(codice==0) return FALSE;
    else return TRUE;

}

/*__________________________________________________________________________________________________________________
 *          ATTIVAZIONE DI UN ALLARME CON OPZIONE AUTORIPRISTINABILE
 *      Se self_resetting==true -> La disattivazione è automatica dopo un timeout di circa 10 secondi
 */
bool PageAlarms::activateNewAlarm(int classe, int codice, bool self_resetting)
{
#ifdef __DISABLE_ERRORS
    return true;
#endif



    int options=0;
    // Impedisce di tentare di aprire la pagina durante un cambio pagina (illegale)
    if((codice) && (GWindowRoot.changePagePending)) options =  DBase::_DB_NO_ACTION;

    if(self_resetting)
    {
        if(codice==0) return FALSE; // Se il tipo di errore self resetting allora non deve resettarlo manualmente
        ApplicationDatabase.setData(classe,((int)codice&0x00FF)|0x100, options);
    }
    else ApplicationDatabase.setData(classe,((int)codice&0x00FF), options);

    if(codice==0) return FALSE;
    else return TRUE;

}

bool PageAlarms::debugActivateNewAlarm(int classe, int codice, bool self_resetting)
{
    int options=0;
    // Impedisce di tentare di aprire la pagina durante un cambio pagina (illegale)
    if((codice) && (GWindowRoot.changePagePending)) options =  DBase::_DB_NO_ACTION;

    if(self_resetting)
    {
        if(codice==0) return FALSE; // Se il tipo di errore Ã¨ self resetting allora non deve resettarlo manualmente
        ApplicationDatabase.setData(classe,((int)codice&0x00FF)|0x100, options);
    }
    else ApplicationDatabase.setData(classe,((int)codice&0x00FF), options);

    if(codice==0) return FALSE;
    else return TRUE;

}

//______________________________________________________________________________________________________________-
// INTERFACCI ADI RIAPERTURA FINESTRA ALLARME NON AUTO RIPRISTINABILE GIA'ATTIVO
void PageAlarms::reopenExistingAlarm(int classe, int codice, bool self_resetting)
{

    ApplicationDatabase.setData(classe,(int) 0, 0); // Elimina allarme
    activateNewAlarm(classe,codice,self_resetting);
}


/*______________________________________________________________________________________________
 *   INTERFACCIA PER FORZARE LA CHIUSURA DEGLI ALLARMI AUTO-RIPRISTINABILI
 *   QUesta funzione viene utilizzata solo dal pannello master
 *
 *  La funzione resetta tutti e soli gli allarmi ripristinabili.
 *  Per eveitare che il database forzi per ognuno di essi l'elaborazione grafica,
 *  solo sull'ultimo allarme di quelli attivi viene permesso l'elaborazione.
 */
void PageAlarms::resetOneShotAlarms(void){
    int i;

    if(!isMaster) return;

    if(!isCurrentPage()){
        for(i=FIRST_ALR_CLASS; i<=LAST_ALR_CLASS;i++)
        {
            if(ApplicationDatabase.getDataI(i)&0xFF00) ApplicationDatabase.setData(i,(int)0, DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
        }

        refreshAlarmStatus();
    }else this->prevPageHandler();

    return;
}


/*______________________________________________________________________________________________
 *   INTERFACCIA PER RICHIEDERE L'APERTURA DELLA PAGINA ALLARMI
 */
bool PageAlarms::openAlarmPage(void){
    if(!alarm_enable) return false;
    if(!numAlarm) return false;

    GWindowRoot.setNewPage(_PG_ALARM,GWindowRoot.curPage,0);
    return true;
}

void PageAlarms::exportMessageList(void){

    QFile   file("/home/user/alrlist.txt");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {

        return ;
    }

    for(int i= 0; i< errors.size(); i++){
        file.write("-----------------------------------------------------------"); file.write("\n");
        file.write(errors[i].classDescription.toAscii().data()); file.write("\n");
        for(int j= 0; j< errors[i].errlist.size(); j++){
            file.write("[M:");
            file.write(errors[i].errlist[j].codestr.toAscii().data());
            file.write("] -> ");
            file.write(errors[i].errlist[j].errmsg.toAscii().data());
            file.write("\n");
        }

    }

    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());

    return ;

}

void PageAlarms::createMessageList(void){
    _alarmStruct erritem;
    _alarmClass classitem;

    errors.clear();
    for(int i=0; i<_DB_ALLARMI_ALR_SOFT-_DB_NALLARMI_ATTIVI; i++){
        errors.append(classitem);
    }
    //errors.reserve(_DB_ALLARMI_ALR_SOFT-_DB_NALLARMI_ATTIVI);




    // ___________________      _DB_ALLARMI_BIOPSIA ____________________________________________________________________________________________________________________________________________
    classitem.className=QString("00000");
    classitem.classDescription=QString("ERRORI RELATIVI ALLA BIOPSIA");
    classitem.errlist.clear();

    erritem.codestr = QString("00001");
    erritem.codeval = ERROR_BIOP_MOVE_XYZ;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-BIOPSIA","BIOPSIA:ERRORE MOVIMENTO CURSORE", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("The cursor  didn't complete the activation properly");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00002");
    erritem.codeval = ERROR_BIOP_APPLY_COMPRESSION;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-BIOPSIA","BIOPSIA:APPLICARE COMPRESSIONE", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("The sequence cannot proceed without a valid compression");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00003");
    erritem.codeval = ERROR_BIOP_MISSING_COMPRESSION;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-BIOPSIA","BIOPSIA:MANCA COMPRESSIONE", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("The sequence cannot proceed without a valid compression");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00004");
    erritem.codeval = ERROR_BIOP_TIMEOUT;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-BIOPSIA","BIOPSIA: TIMEOUT MOVIMENTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("Timeout during Biopsy activation");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00005");
    erritem.codeval = ERROR_BIOP_BUSY;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-BIOPSIA","BIOPSIA: BUSY", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("A command is pending or the system queue is busy");
    classitem.errlist.append(erritem);


    erritem.codestr = QString("00006");
    erritem.codeval = ERROR_BIOP_INVALID_REFERENCES;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-BIOPSIA","INVALID READER POINTS", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("The reader points are invalid. Check the reader calibration or repeat the measuring.");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00007");
    erritem.codeval = ERROR_BIOP_LESION_TOO_LOWER;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-BIOPSIA","LESION MEASURED TOO CLOSED TO THE POTTER PLANE", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("The measured lesion is closed (less than 4mm) to the potter plane.");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00008");
    erritem.codeval = ERROR_BIOP_LESION_TOO_HIGH;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-BIOPSIA","LESION MEASURED TOO CLOSED TO THE SKIN SURFACE", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("The measured lesion is closed to the skin surface.");
    classitem.errlist.append(erritem);


    errors.replace(_DB_ALLARMI_BIOPSIA-FIRST_ALR_CLASS,classitem);
    //___________________________________________________________________________________________________________________________________________________________________

    // ___________________      _DB_ALLARMI_ALR_ARM ____________________________________________________________________________________________________________________________________________
    classitem.className=QString("00100");
    classitem.classDescription=QString("ERRORI RELATIVI ALLA ROTAZIONE C-ARM");
    classitem.errlist.clear();

    erritem.codestr = QString("00101");
    erritem.codeval = ARM_ERROR_INVALID_STATUS;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-ARM","ARM: ATTIVAZIONE NON CONSENTITA", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(WARN_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00102");
    erritem.codeval = ARM_ERROR_ZERO_SETTING_INIT;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-ARM","ARM: ERRORE PARAMETRI DI PREPARAZIONE AZZERAMENTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00103");
    erritem.codeval = ARM_ERROR_ZERO_SETTING_EXECUTION;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-ARM","ARM: ERRORE PARAMETRI DI MOVIMENTO DI AZZERAMENTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00104");
    erritem.codeval = ARM_ERROR_ZERO_SETTING_TMO;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-ARM","ARM: TIMEOUT MOVIMENTO DI AZZERAMENTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(WARN_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00105");
    erritem.codeval = ARM_ERROR_POSITION_SETTING_INIT;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-ARM","ARM: ERRORE PARAMETRI PREPARAZIONE POSIZIONAMENTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00106");
    erritem.codeval = ARM_ERROR_POSITION_EXECUTION;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-ARM","ARM: ERRORE PARAMETRI MOVIMENTO DI POSIZIONAMENTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00107");
    erritem.codeval = ARM_ERROR_POSITION_TMO;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-ARM","ARM: TIMEOUT MOVIMENTO DI POSIZIONAMENTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(WARN_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00108");
    erritem.codeval = ARM_OBSTACLE_ERROR;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-ARM","ARM: RILEVATO OSTACOLO DURANTE POSIZIONAMENTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(WARN_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00109");
    erritem.codeval = ARM_DEVICE_ERROR;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-ARM","ARM:ERRORE INTERNO DISPOSITIVO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00110");
    erritem.codeval = ARM_DISABLED_ERROR;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-ARM","ARM: MOVIMENTO NON ABILITATO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(WARN_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00111");
    erritem.codeval = ARM_RANGE_ERROR;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-ARM","ARM: INTERVALLO DI MOVIMENTO NON CONSENTITO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(WARN_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00112");
    erritem.codeval = ARM_BUSY;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-ARM","ARM: COMANDO IN CORSO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(WARN_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00113");
    erritem.codeval = ARM_SECURITY_SWITCHES;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-ARM","ARM: PROTEZIONE MOTORE ROTAZIONE", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00114");
    erritem.codeval = ARM_CAN_ERROR;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-ARM","ARM: ERRORE DI COMUNICAZIONE SUL CAN BUS", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00115");
    erritem.codeval = ARM_OBSTACLE_BLOCKED_ERROR;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-ARM","ARM: DISPOSITIVO RILEVAMENTO OSTACOLO BLOCCATO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00116");
    erritem.codeval = ARM_OBSTACLE_OBSTRUCTION_ERROR;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-ARM","ARM: RILEVATA OSTRUZIONE DURANTE IL MOVIMENTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(WARN_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);




    errors.replace(_DB_ALLARMI_ALR_ARM-FIRST_ALR_CLASS,classitem);
    //___________________________________________________________________________________________________________________________________________________________________

    // ___________________      _DB_ALLARMI_ALR_COLLI ____________________________________________________________________________________________________________________________________________
   classitem.className=QString("00300");
   classitem.classDescription=QString("ERRORI DI COLLIMAZIONE");
   classitem.errlist.clear();

   erritem.codestr = QString("00301");
   erritem.codeval = COLLI_UPDATE_FALLITO;
   erritem.errmsg  = QString(QApplication::translate("ERRORE-COLLIMATORE","COLLIMAZIONE FALLITA\nRIPETERE SEQUENZA DI COLLIMAZIONE\n", 0, QApplication::UnicodeUTF8));
   erritem.errpix =  QPixmap(WARN_PIX);
   erritem.errdescr= QString("");
   classitem.errlist.append(erritem);
   erritem.codestr = QString("00302");
   erritem.codeval = COLLI_FILTRO_FALLITO;
   erritem.errmsg  = QString(QApplication::translate("ERRORE-COLLIMATORE","IMPOSTAZIONE FILTRO FALLITA!\n", 0, QApplication::UnicodeUTF8));
   erritem.errpix =  QPixmap(WARN_PIX);
   erritem.errdescr= QString("");
   classitem.errlist.append(erritem);
   erritem.codestr = QString("00303");
   erritem.codeval = COLLI_SPECCHIO_FALLITO;
   erritem.errmsg  = QString(QApplication::translate("ERRORE-COLLIMATORE","IMPOSTAZIONE SPECCHIO FALLITA!\n", 0, QApplication::UnicodeUTF8));
   erritem.errpix =  QPixmap(WARN_PIX);
   erritem.errdescr= QString("");
   classitem.errlist.append(erritem);

   erritem.codestr = QString("00304");
   erritem.codeval = COLLI_LAMP_FALLITO;
   erritem.errmsg  = QString(QApplication::translate("ERRORE-COLLIMATORE","IMPOSTAZIONE LAMPADA CENTRATORE FALLITA!\n", 0, QApplication::UnicodeUTF8));
   erritem.errpix =  QPixmap(WARN_PIX);
   erritem.errdescr= QString("");
   classitem.errlist.append(erritem);


   errors.replace(_DB_ALLARMI_ALR_COLLI-FIRST_ALR_CLASS,classitem);

   //___________________________________________________________________________________________________________________________________________________________________

    // ___________________      _DB_ALLARMI_ALR_TRX ____________________________________________________________________________________________________________________________________________
    classitem.className=QString("00400");
    classitem.classDescription=QString("ERRORI RELATIVI ALLA ROTAZIONE TUBO");
    classitem.errlist.clear();

    erritem.codestr = QString("00401");
    erritem.codeval = TRX_ERROR_INVALID_STATUS;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-TRX","TRX: ATTIVAZIONE NON CONSENTITA", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(WARN_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00402");
    erritem.codeval = TRX_ERROR_ZERO_SETTING_INIT;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-TRX","TRX: ERRORE PARAMETRI DI PREPARAZIONE AZZERAMENTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00403");
    erritem.codeval = TRX_ERROR_ZERO_SETTING_EXECUTION;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-TRX","TRX: ERRORE PARAMETRI DI MOVIMENTO DI AZZERAMENTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00404");
    erritem.codeval = TRX_ERROR_ZERO_SETTING_TMO;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-TRX","TRX: TIMEOUT MOVIMENTO DI AZZERAMENTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(WARN_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00405");
    erritem.codeval = TRX_ERROR_POSITION_SETTING_INIT;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-TRX","TRX: ERRORE PARAMETRI PREPARAZIONE POSIZIONAMENTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00406");
    erritem.codeval = TRX_ERROR_POSITION_EXECUTION;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-TRX","TRX: ERRORE PARAMETRI MOVIMENTO DI POSIZIONAMENTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00407");
    erritem.codeval = TRX_TIMEOUT_ERROR;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-TRX","TRX: TIMEOUT DURANTE IL MOVIMENTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(WARN_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00408");
    erritem.codeval = TRX_OBSTACLE_ERROR;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-TRX","TRX: RILEVATO OSTACOLO DURANTE POSIZIONAMENTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(WARN_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00409");
    erritem.codeval = TRX_DEVICE_ERROR;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-TRX","ERRORE INTERNO DISPOSITIVO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00410");
    erritem.codeval = TRX_DISABLED_ERROR;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-TRX","TRX: MOVIMENTO NON ABILITATO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(WARN_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00411");
    erritem.codeval = TRX_RANGE_ERROR;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-TRX","TRX: INTERVALLO DI MOVIMENTO NON CONSENTITO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00412");
    erritem.codeval = TRX_BUSY;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-TRX","TRX: COMANDO IN CORSO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(WARN_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00413");
    erritem.codeval = TRX_SAFETY_FAULT;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-TRX","TRX: DISPOSITIVO DI SICUREZZA ATTIVATO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00414");
    erritem.codeval = TRX_CAN_ERROR;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-TRX","TRX: ERRORE DI COMUNICAZIONE SUL CAN BUS", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00415");
    erritem.codeval = TRX_OBSTACLE_BLOCKED_ERROR;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-TRX","TRX: DISPOSITIVO RILEVAMENTO OSTACOLO BLOCCATO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00416");
    erritem.codeval = TRX_OBSTACLE_OBSTRUCTION_ERROR;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-TRX","TRX: RILEVATA OSTRUZIONE DURANTE IL MOVIMENTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(WARN_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    errors.replace(_DB_ALLARMI_ALR_TRX-FIRST_ALR_CLASS,classitem);
    //___________________________________________________________________________________________________________________________________________________________________


    // ___________________      _DB_ALLARMI_ALR_GEN ____________________________________________________________________________________________________________________________________________
    classitem.className=QString("00500");
    classitem.classDescription=QString("ERRORI RELATIVI AL PCB190");
    classitem.errlist.clear();

    erritem.codestr = QString("00501");
    erritem.codeval = ERROR_CUFFIA_CALDA;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","TEMPERATURA TUBO ECCESSIVA", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00502");
    erritem.codeval = GEN_SET_FUOCO;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","IMPOSTAZIONE FUOCO FALLITA\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(WARN_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00503");
    erritem.codeval = GEN_R16_FAULT;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","ANOMALIA SENSING CORRENTE ANODICA", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(WARN_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00504");
    erritem.codeval = GEN_GND_FAULT;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","ANOMALIA CONNESSIONE CONNESSIONE DI TERRA", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00505");
    erritem.codeval = GEN_CALIB_HV;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","MANCANZA CALIBRAZIONE HV", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(WARN_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00506");
    erritem.codeval = GEN_HV_FAULT;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","ANOMALIA TENSIONE HV", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00507");
    erritem.codeval = GEN_MAS_FAULT;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","ANOMALIA mAsMETRO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00508");
    erritem.codeval = GEN_IFIL_FAULT;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","ANOMALIA CORRENTE DI FILAMENTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00509");
    erritem.codeval = GEN_AMPTEMP_FAULT;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","ANOMALIA TEMPERATURA AMPLIFICATORE DI FILAMENTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00510");
    erritem.codeval = ARFLT_MAIN_OFF;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","ANOMALIA STARTER BASSA VELOCITA\nCORRENTE MAIN-OFF", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00511");
    erritem.codeval = ARFLT_SHIFT_OFF;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","ANOMALIA STARTER BASSA VELOCITA\nCORRENTE SHIFT-OFF", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00512");
    erritem.codeval = ARFLT_MAIN_RUN_MAX;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","ANOMALIA STARTER BASSA VELOCITA\nCORRENTE MAIN RUN-MAX", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00513");
    erritem.codeval = ARFLT_MAIN_RUN_MIN;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","ANOMALIA STARTER BASSA VELOCITA\nCORRENTE MAIN RUN-MIN", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00514");
    erritem.codeval = ARFLT_MAIN_KEEP_MAX;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","ANOMALIA STARTER BASSA VELOCITA\nCORRENTE MAIN KEEP-MAX", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00515");
    erritem.codeval = ARFLT_MAIN_KEEP_MIN;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","ANOMALIA STARTER BASSA VELOCITA\nCORRENTE MAIN KEEP-MIN", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00516");
    erritem.codeval = ARFLT_SHIFT_RUN_MAX;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","ANOMALIA STARTER BASSA VELOCITA\nCORRENTE SHIFT RUN-MAX", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00517");
    erritem.codeval = ARFLT_SHIFT_RUN_MIN;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","ANOMALIA STARTER BASSA VELOCITA\nCORRENTE SHIFT RUN-MIN", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00518");
    erritem.codeval = ARFLT_SHIFT_KEEP_MAX;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","ANOMALIA STARTER BASSA VELOCITA\nCORRENTE SHIFT KEEP-MAX", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00519");
    erritem.codeval = ARFLT_SHIFT_KEEP_MIN;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","ANOMALIA STARTER BASSA VELOCITA\nCORRENTE SHIFT KEEP-MIN", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00520");
    erritem.codeval = GEN_STARTER_NOT_CALIBRATED;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","STARTER NON CALIBRATO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(WARN_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00521");
    erritem.codeval = ERROR_ANODE_HU;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","HU ANODO ECCESSIVI", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00522");
    erritem.codeval = ERROR_SENS_CUFFIA;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-GENERATORE","ANOMALIA SENSORE TEMPERATURA TUBO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);


    errors.replace(_DB_ALLARMI_ALR_GEN-FIRST_ALR_CLASS,classitem);
    //___________________________________________________________________________________________________________________________________________________________________

    // ___________________      _DB_ALLARMI_ALR_SOFT ____________________________________________________________________________________________________________________________________________
      classitem.className=QString("00600");
      classitem.classDescription=QString("ERRORI INTERNI AI TERMINALI");
      classitem.errlist.clear();

      erritem.codestr = QString("00601");
      erritem.codeval = ERROR_MCC;
      erritem.errmsg  = QString(QApplication::translate("ERRORE-SOFTWARE","ALLARME SOFTWARE!!!\nCODE DI PROCESSO PROVVISORIAMENTE PIENE\nRIPETERE L'ULTIMA OPERAZIONE", 0, QApplication::UnicodeUTF8));
      erritem.errpix =  QPixmap(ERR_PIX);
      erritem.errdescr= QString("");
      classitem.errlist.append(erritem);

      erritem.codestr = QString("00602");
      erritem.codeval = ERROR_REVISIONS;
      erritem.errmsg  = QString(QApplication::translate("ERRORE-SOFTWARE","ALLARME SOFTWARE!!!\nRILEVATE REVISIONI SOFTWARE NON COMPATIBILI\nCONTATTARE L'ASSISTENZA", 0, QApplication::UnicodeUTF8));
      erritem.errpix =  QPixmap("://AlarmPage/AlarmPage/RevisionPix.png");
      erritem.errdescr= QString("");
      classitem.errlist.append(erritem);


      erritem.codestr = QString("00603");
      erritem.codeval = WARNIN_SPEGNIMENTO;
      erritem.errmsg  = QString(QApplication::translate("ERRORE-SOFTWARE","ATTENZIONE: SPEGNIMENTO SISTEMA IN CORSO", 0, QApplication::UnicodeUTF8));
      erritem.errpix =  QPixmap(WARN_PIX);
      erritem.errdescr= QString("");
      classitem.errlist.append(erritem);


      errors.replace(_DB_ALLARMI_ALR_SOFT-FIRST_ALR_CLASS,classitem);
     //___________________________________________________________________________________________________________________________________________________________________



    // ___________________      _DB_ALLARMI_ALR_PAD ____________________________________________________________________________________________________________________________________________
    classitem.className=QString("00700");
    classitem.classDescription=QString("ERRORI RICONOSCIMENTO COMPRESSORE");
    classitem.errlist.clear();

    erritem.codestr = QString("00701");
    erritem.codeval = _ALR_UNLOCK_COMPR;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-PAD","COMPRESSORE SBLOCCATO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(_PG_ALARM_UNLOCK_COMPR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00702");
    erritem.codeval = _ALR_UNLOCK_PAD;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-PAD","SUPPORTO COMPRESSORE SBLOCCATO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(_PG_ALARM_UNLOCK_PAD_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00703");
    erritem.codeval = _ALR_WRONG_PAD;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-PAD","COMPRESSORE NON RICONOSCIUTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00704");
    erritem.codeval = _ALR_COMPR_CLOSED_STUDY;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-PAD","COMPRESSIONE IN CORSO A STUDIO CHIUSO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("Only with the initial page and only the first time in compresison.");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00705");
    erritem.codeval = _ALR_UNLOCK_POTTER;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-PAD","ACCESSORIO DISCONNESSO\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("The Accessory has not been detected or it is unmounted");
    classitem.errlist.append(erritem);


    errors.replace(_DB_ALLARMI_ALR_PAD-FIRST_ALR_CLASS,classitem);
    //___________________________________________________________________________________________________________________________________________________________________
    // ___________________      _DB_ALLARMI_ALR_POTTER ____________________________________________________________________________________________________________________________________________
    classitem.className=QString("00800");
    classitem.classDescription=QString("ERRORI RICONOSCIMENTO ACCESSORIO");
    classitem.errlist.clear();

    erritem.codestr = QString("00801");
    erritem.codeval = ERROR_MAG_READ;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-POTTER","INGRANDITORE: FATTORE DI INGRANDIMENTO NON VALIDO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("Invalid identification code");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00802");
    erritem.codeval = ERROR_MAG_CONF;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-POTTER","INGRANDITORE: FATTORE DI INGRANDIMENTO NON CONFIGURATO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("The accessory code hasn't the corresponding magnification factor in the configuration file.");
    classitem.errlist.append(erritem);

    errors.replace(_DB_ALLARMI_ALR_POTTER-FIRST_ALR_CLASS,classitem);
    //___________________________________________________________________________________________________________________________________________________________________


    // ___________________      _DB_ALLARMI_ALR_RAGGI ____________________________________________________________________________________________________________________________________________
    classitem.className=QString("00900");
    classitem.classDescription=QString("ERRORI RILEVATI DURANTE UN'ESPOSIZIONE");
    classitem.errlist.clear();


    erritem.codestr = QString("00901");
    erritem.codeval = ERROR_MISS_PIOMPO;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","IMPOSSIBILE PROCEDERE CON I RAGGI!!\nINSERIRE ACCESSORIO PIOMBO\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(_PG_ALARM_PROT_PB_PIX);
    erritem.errdescr= QString("The Lead plate is required for this exposure");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00902");
    erritem.codeval = ERROR_MISS_PLEXY;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","IMPOSSIBILE PROCEDERE CON I RAGGI!!\nINSERIRE ACCESSORIO PLEXYGLASS\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(_PG_ALARM_FANTOCCIO_CALIB_PIX);
    erritem.errdescr= QString("The Plexyglass phantom is required for this exposure.");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00903");
    erritem.codeval = ERROR_MISSA_PROT_PAZIENTE;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","IMPOSSIBILE PROCEDERE CON I RAGGI!!\nINSERIRE ACCESSORIO PROTEZIONE PAZIENTE\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(_PG_ALARM_PROT_2D_PIX);
    erritem.errdescr= QString("The patient protection is required during the exposure");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00904");
    erritem.codeval = ERROR_CLOSED_DOOR;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","IMPOSSIBILE PROCEDERE CON I RAGGI!!\nPORTA STUDIO APERTA\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00905");
    erritem.codeval = ERROR_INVALID_DATA;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","ERRORE DATI SU DRIVER PCB190!!\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00906");
    erritem.codeval = ERROR_TMO_XRAY_ENA;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","IO ERROR: IMPOSSIBILE ATTIVARE XRAY ENA\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00907");
    erritem.codeval = ERROR_TMO_AR;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","TIMEOUT STARTER ANODO ROTANTE\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00908");
    erritem.codeval = ERROR_PUSHRX_XRAY;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","RILASCIO ANTICIPATO DEL PULSANTE RAGGI\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00909");
    erritem.codeval = ERROR_PUSHRX_NO_PREP;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","RILASCIO ANTICIPATO DEL PULSANTE RAGGI NO PREP\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00910");
    erritem.codeval = ERROR_PUSHRX_AFTER_PREP;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","RILASCIO ANTICIPATO DEL PULSANTE RAGGI AFTER PREP\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00911");
    erritem.codeval = ERROR_AR_BUSY;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","STARTER BUSY\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00912");
    erritem.codeval = ERROR_DT_EXPWIN;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","TIMEOUT ATTESA SEGNALE EXP WIN DA DETECTOR\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00913");
    erritem.codeval = ERROR_HV_HIGH;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","DIAGNOSTICA PCB190 SEGNALA HV ALTI\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00914");
    erritem.codeval = ERROR_HV_LOW;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","DIAGNOSTICA PCB190 SEGNALA HV BASSI\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00915");
    erritem.codeval = ERROR_IA_HIGH;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","DIAGNOSTICA PCB190 SEGNALA CORRENTE ANODICA ALTA\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00916");
    erritem.codeval = ERROR_IA_LOW;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","DIAGNOSTICA PCB190 SEGNALA CORRENTE ANODICA BASSA\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00917");
    erritem.codeval = ERROR_IFIL;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","DIAGNOSTICA PCB190 SEGNALA CORRENTE DI FILAMENTO ALTA\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00918");
    erritem.codeval = ERROR_VFIL;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","DIAGNOSTICA PCB190 SEGNALA ANOMALIA SU V DI FILAMENTO\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00919");
    erritem.codeval = ERROR_PWR_HV;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","DIAGNOSTICA PCB190 SEGNALA BASSA TENSIONE DI ALIMENTAZIONE POTENZA\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00920");
    erritem.codeval = ERROR_TMO_RX;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","TIMEOUT DURANTE ESECUZIONE RAGGI\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00921");
    erritem.codeval = ERROR_INVALID_KV;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SELEZIONE kV NON VALIDA\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00922");
    erritem.codeval = ERROR_INVALID_MAS;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SELEZIONE mAs NON VALIDA\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00923");
    erritem.codeval = ERROR_INVALID_GEN_CONFIG;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","CONFIGURAZIONE PARAMETRI GENERATORE NON VALIDA\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00924");
    erritem.codeval = ERROR_NOT_CALIBRATED_KV;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","kv SELEZIONATI RISULTANO NON CALIBRATI\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00925");
    erritem.codeval = ERROR_NOT_CALIBRATED_I;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","CORRENTE DI FILAMENTO NON CALIBRATA\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00926");
    erritem.codeval = ERROR_TUBE_NOT_CONFIGURED;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","PARAMETRI GENERATORE NON CONFIGURATI\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00927");
    erritem.codeval = ERROR_INVALID_FUOCO;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SELEZIONE FUOCO NON VALIDA\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00928");
    erritem.codeval = ERROR_INVALID_FILTRO;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SELEZIONE FILTRO NON VALIDA\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00929");
    erritem.codeval = ERROR_INVALID_COLLI;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SELEZIONE COLLIMAZIONE NON VALIDA\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00930");
    erritem.codeval = ERROR_GRID;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","ANOMALIA TENSIONE DI GRIGLIA\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00931");
    erritem.codeval = ERROR_VCC;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","ANOMALIA ALIMENTAZIONE SCHEDA PCB190\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00932");
    erritem.codeval = ERROR_INVALID_PAD;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","ATTIVAZIONE RAGGI NON CONSENTITA\nPAD NON IDONEO AL CONTESTO\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00933");
    erritem.codeval = ERROR_MISSING_PAD;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","ATTIVAZIONE RAGGI NON CONSENTITA\nPAD NON RICONOSCIUTO\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00934");
    erritem.codeval = ERROR_INVALID_POTTER;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","ATTIVAZIONE RAGGI NON CONSENTITA\nPOTTER NON RICONOSCIUTO\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00935");
    erritem.codeval = ERROR_ANGOLO_ARM;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","ATTIVAZIONE RAGGI NON CONSENTITA\nBRACCIO FUORI POSIZIONE\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00936");
    erritem.codeval = ERROR_MISSING_COMPRESSION;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","ATTIVAZIONE RAGGI NON CONSENTITA\nSENO NON COMPRESSO\nAPPLICARE COMPRESSIONE PER PROSEGUIRE", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00937");
    erritem.codeval = _SEQ_WRITE_REGISTER;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nOPERAZIONE DI SCRITTURA REGISTRI\nFALLITA\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00938");
    erritem.codeval = _SEQ_READ_REGISTER;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nOPERAZIONE DI LETTURA REGISTRI\nFALLITA\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00939");
    erritem.codeval = _SEQ_IO_TIMEOUT;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nIO NETWORK BUSY\nRILASCIARE IL PULSANTE RAGGI E RIPETERE\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00940");
    erritem.codeval = _SEQ_DRIVER_FREEZE;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nIMPOSSIBILE BLOCCARE I DRIVER\nEFFETTUARE RESET HARDWARE!!\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00941");
    erritem.codeval = _SEQ_DRIVER_READY;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nIMPOSSIBILE SBLOCCARE I DRIVER\nEFFETTUARE RESET HARDWARE!!\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00942");
    erritem.codeval = _SEQ_UPLOAD190_PARAM;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nIMPOSSIBILE CARICARE I PARAMETRI\nDI ESPOSIZIONE!!\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00943");
    erritem.codeval = _SEQ_PCB190_BUSY;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nDRIVER PCB190 BUSY\nRIPETERE ESPOSIZIONE\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);


    erritem.codestr = QString("00944");
    erritem.codeval = _SEQ_PCB190_TMO;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nTIMEOUT ATTESA FINE ESPOSIZIONE\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00945");
    erritem.codeval = _SEQ_WAIT_AEC_DATA;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nTIMEOUT ATTESA DATI AEC\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00946");
    erritem.codeval = _SEQ_AEC_NOT_AVAILABLE;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nDATI AEC NON DISPONIBILI\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00947");
    erritem.codeval = _SEQ_ERR_COLLI_TOMO;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nIMPOSSIBILE IMPOSTARE MODALITA' TOMO\nSU COLLIMATORE\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00948");
    erritem.codeval = _SEQ_ERR_NARROW_HOME;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nERRORE POSIZIONAMENTO TUBO IN HOME\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00949");
    erritem.codeval = _SEQ_ERR_WIDE_HOME;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nERRORE POSIZIONAMENTO TUBO IN HOME WIDE\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00950");
    erritem.codeval = _SEQ_ERR_NARROW_END;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nERRORE ATTIVAZIONE TUBO PER SCANSIONE\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00951");
    erritem.codeval = _SEQ_ERR_WIDE_END;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nERRORE ATTIVAZIONE TUBO PER SCANSIONE WIDE\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);


    erritem.codestr = QString("00952");
    erritem.codeval = _SEQ_ERR_TRX_CC;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nERRORE ATTIVAZIONE TUBO IN POSIZIONE CC\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00953");
    erritem.codeval = ERROR_MCC_COMMAND;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nERRORE SOFTWARE: CODA MCC PIENA\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);


    erritem.codestr = QString("00954");
    erritem.codeval = ERROR_INVALID_MAG_FACTOR;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nERRORE INGRANDITORE: INGRANDIMENTO NON VALIDO\nO NON RICONOSCIUTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00955");
    erritem.codeval = ERROR_INVALID_MAG_FUOCO;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nSI STA UTILIZZANDO IL FUOCO GRANDE\nCON L'INGRANDITORE INSERITO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00956");
    erritem.codeval = ERROR_INVALID_SMALL_FOCUS;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nSI STA UTILIZZANDO IL FUOCO PICCOLO\nSENZA USO DELL'INGRANDITORE", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00957");
    erritem.codeval = ERROR_TRX_FAULT_AFTER_PREP;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","ANOMALIA MOVIMENTO TUBO DURANTE RAGGI", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00958");
    erritem.codeval = ERROR_MIRROR_LAMP;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","ERRORE SPECCHIO FUORI CAMPO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);


    erritem.codestr = QString("00959");
    erritem.codeval = _SEQ_ERR_INTERMEDIATE_HOME;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nERRORE POSIZIONAMENTO TUBO IN HOME INTERMEDIATE", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00960");
    erritem.codeval = _SEQ_ERR_INTERMEDIATE_END;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nERRORE ATTIVAZIONE TUBO PER SCANSIONE INTERMEDIATE", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00961");
    erritem.codeval = ERROR_MISS_PROT_PAZIENTE_3D;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","PROTEZIONE PAZIENTE 3D\n", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00962");
    erritem.codeval = ARFLT_MAIN_OFF;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nANOMALIA STARTER BASSA VELOCITA\nERROR CODE: %1", 0, QApplication::UnicodeUTF8)).arg(ARFLT_MAIN_OFF);
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00962");
    erritem.codeval = ARFLT_SHIFT_OFF;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nANOMALIA STARTER BASSA VELOCITA\nERROR CODE: %1", 0, QApplication::UnicodeUTF8)).arg(ARFLT_SHIFT_OFF);
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00962");
    erritem.codeval = ARFLT_MAIN_RUN_MAX;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nANOMALIA STARTER BASSA VELOCITA\nERROR CODE: %1", 0, QApplication::UnicodeUTF8)).arg(ARFLT_MAIN_RUN_MAX);
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00962");
    erritem.codeval = ARFLT_MAIN_RUN_MIN;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nANOMALIA STARTER BASSA VELOCITA\nERROR CODE: %1", 0, QApplication::UnicodeUTF8)).arg(ARFLT_MAIN_RUN_MIN);
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00962");
    erritem.codeval = ARFLT_MAIN_KEEP_MAX;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nANOMALIA STARTER BASSA VELOCITA\nERROR CODE: %1", 0, QApplication::UnicodeUTF8)).arg(ARFLT_MAIN_KEEP_MAX);
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00962");
    erritem.codeval = ARFLT_MAIN_KEEP_MIN;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nANOMALIA STARTER BASSA VELOCITA\nERROR CODE: %1", 0, QApplication::UnicodeUTF8)).arg(ARFLT_MAIN_KEEP_MIN);
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00962");
    erritem.codeval = ARFLT_SHIFT_RUN_MAX;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nANOMALIA STARTER BASSA VELOCITA\nERROR CODE: %1", 0, QApplication::UnicodeUTF8)).arg(ARFLT_SHIFT_RUN_MAX);
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00962");
    erritem.codeval = ARFLT_SHIFT_RUN_MIN;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nANOMALIA STARTER BASSA VELOCITA\nERROR CODE: %1", 0, QApplication::UnicodeUTF8)).arg(ARFLT_SHIFT_RUN_MIN);
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00962");
    erritem.codeval = ARFLT_SHIFT_KEEP_MAX;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nANOMALIA STARTER BASSA VELOCITA\nERROR CODE: %1", 0, QApplication::UnicodeUTF8)).arg(ARFLT_SHIFT_KEEP_MAX);
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00962");
    erritem.codeval = ARFLT_SHIFT_KEEP_MIN;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SEQUENZA RAGGI INTERROTTA\nANOMALIA STARTER BASSA VELOCITA\nERROR CODE: %1", 0, QApplication::UnicodeUTF8)).arg(ARFLT_SHIFT_KEEP_MIN);
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00963");
    erritem.codeval = ERROR_ANODE_HU;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","ATTENZIONE, HU ANODO ECCESSIVI\nATTENDERE CHE IL TUBO SI RAFFREDDI\nPER PROSEGUIRE", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00964");
    erritem.codeval = ERROR_CUFFIA_CALDA;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","ATTENZIONE, TEMPERATURA TUBO ECCESSIVA\nATTENDERE CHE IL TUBO SI RAFFREDDI\nPER PROSEGUIRE", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00965");
    erritem.codeval = ERROR_SENS_CUFFIA;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","ATTENZIONE, SENSORE TEMPERATURA TUBO MALFUNZIONANTE", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("00966");
    erritem.codeval =_SEQ_ERR_INVALID_TUBE_ACTIVATION ;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","MOVIMENTO TUBO NON CONSENTITO. POSSIBILE IMPATTO CON IL SUOLO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00967");
    erritem.codeval = ESPOSIMETRO_BREAST_DENSE ;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","SENO TROPPO DENSO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);

    erritem.codestr = QString("00968");
    erritem.codeval = ESPOSIMETRO_AEC_SOVRAESPOSTO ;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-RAGGI","ESPOSIMETRO SOVRAESPOSTO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("");
    classitem.errlist.append(erritem);




    /* Prevedere i seguenti errori
    - 967 -> AEC sovraesposto"
    - 968 -> AEC sottoesposto"


    */
    errors.replace(_DB_ALLARMI_ALR_RAGGI-FIRST_ALR_CLASS,classitem);
    //___________________________________________________________________________________________________________________________________________________________________



    // ___________________      _DB_ALLARMI_ALR_COMPRESSORE ____________________________________________________________________________________________________________________________________________
    classitem.className=QString("01000");
    classitem.classDescription=QString("ERRORI DI SICUREZZA RELATIVI ALLA PCB269");
    classitem.errlist.clear();

    erritem.codestr = QString("01003");
    erritem.codeval = _COMPR_SAFETY_FAULT;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-PCB269","COMPRESSIONE ECCESSIVA", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("The excess compression force has been measured by the magnetic sensor.");
    classitem.errlist.append(erritem);

    errors.replace(_DB_ALLARMI_ALR_COMPRESSORE-FIRST_ALR_CLASS,classitem);

    //___________________________________________________________________________________________________________________________________________________________________


    // ___________________      _DB_ALLARMI_POWERDOWN ____________________________________________________________________________________________________________________________________________
    classitem.className=QString("01100");
    classitem.classDescription=QString("ERRORI RELATIVI ALLA MANCANZA TENSIONE");
    classitem.errlist.clear();

    erritem.codestr = QString("01101");
    erritem.codeval = ERROR_POWER_DOWN_MAINS;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-POWERDOWN","ERRORE MANCANZA RETE", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("The AC/DC 24VDC power supply has been detected OFF.\nIt is direct connected to the MAIN.");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("01102");
    erritem.codeval = ERROR_POWER_DOWN_EMERGENCY_BUTTON;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-POWERDOWN","ERRORE PULSANTE DI EMERGENZA", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("The Lenze VBUS is detected below of 100V.\nThis can be caused by the Emergency button or a fault in the Lenze power supply chain.");
    classitem.errlist.append(erritem);
    erritem.codestr = QString("01103");
    erritem.codeval = ERROR_POWER_DOWN_WARNING_BLITERS_ON;
    erritem.errmsg  = QString(QApplication::translate("ERRORE-POWERDOWN","WARNING CARICA CONDENSATORI IN CORSO", 0, QApplication::UnicodeUTF8));
    erritem.errpix =  QPixmap(ERR_PIX);
    erritem.errdescr= QString("The power resistor are still charging the Power Capacitors");
    classitem.errlist.append(erritem);

    errors.replace(_DB_ALLARMI_POWERDOWN-FIRST_ALR_CLASS,classitem);
    //___________________________________________________________________________________________________________________________________________________________________


   // ___________________      _DB_ALLARMI_ALR_LENZE ____________________________________________________________________________________________________________________________________________
   classitem.className=QString("01200");
   classitem.classDescription=QString("ERRORI DRIVER LENZE");
   classitem.errlist.clear();

   erritem.codestr = QString("01201");
   erritem.codeval = LENZE_ANALOG_CONNECTION_ERROR;
   erritem.errmsg  = QString(QApplication::translate("ERRORE-LENZE","LENZE: ERRORE CONNESSIONE SENSORE DI POSIZIONE", 0, QApplication::UnicodeUTF8));
   erritem.errpix =  QPixmap(ERR_PIX);
   erritem.errdescr= QString("");
   classitem.errlist.append(erritem);
   erritem.codestr = QString("01202");
   erritem.codeval = LENZE_ANALOG_INPUT_ERROR;
   erritem.errmsg  = QString(QApplication::translate("ERRORE-LENZE","LENZE: ERRORE SENSORE DI POSIZIONE", 0, QApplication::UnicodeUTF8));
   erritem.errpix =  QPixmap(ERR_PIX);
   erritem.errdescr= QString("");
   classitem.errlist.append(erritem);
   erritem.codestr = QString("01203");
   erritem.codeval = LENZE_DEVICE_ERROR;
   erritem.errmsg  = QString(QApplication::translate("ERRORE-LENZE","LENZE: ERRORE DISPOSITIVO", 0, QApplication::UnicodeUTF8));
   erritem.errpix =  QPixmap(ERR_PIX);
   erritem.errdescr= QString("");
   classitem.errlist.append(erritem);
   erritem.codestr = QString("01204");
   erritem.codeval = LENZE_DROP_ARM;
   erritem.errmsg  = QString(QApplication::translate("ERRORE-LENZE","LENZE: ALLARME CADUTA BRACCIO", 0, QApplication::UnicodeUTF8));
   erritem.errpix =  QPixmap(ERR_PIX);
   erritem.errdescr= QString("");
   classitem.errlist.append(erritem);


   errors.replace(_DB_ALLARMI_ALR_LENZE-FIRST_ALR_CLASS,classitem);
   //___________________________________________________________________________________________________________________________________________________________________



  // __________________________ _DB_ALLARME_XRAY_PUSH,// Diagnostica sul pulsante raggi
  classitem.className=QString("01300");
  classitem.classDescription=QString("ERRORI PULSANTE RAGGI");
  classitem.errlist.clear();

  erritem.codestr = QString("01301");
  erritem.codeval = XRAY_PUSH_TIMEOUT;
  erritem.errmsg  = QString(QApplication::translate("ERRORE-XRAYPUSH","PULSANTE RAGGI BLOCCATO\n", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(ERR_PIX);
  erritem.errdescr= QString("");
  classitem.errlist.append(erritem);

  errors.replace(_DB_ALLARME_XRAY_PUSH-FIRST_ALR_CLASS,classitem);
  //___________________________________________________________________________________________________________________________________________________________________
  // __________________________ _DB_ALLARME_CMP_PUSH, // Diagnostica sulla pedaliera del compressore
  classitem.className=QString("01400");
  classitem.classDescription=QString("ERRORI PEDALIERA COMPRESSORE");
  classitem.errlist.clear();

  erritem.codestr = QString("01401");
  erritem.codeval = COMPRESSOR_INVALID_PUSH;
  erritem.errmsg  = QString(QApplication::translate("ERRORE-COMPPUSH","PEDALI COMPRESSORE BLOCCATI\n", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(ERR_PIX);
  erritem.errdescr= QString("");
  classitem.errlist.append(erritem);

  errors.replace(_DB_ALLARME_CMP_PUSH-FIRST_ALR_CLASS,classitem);
  //___________________________________________________________________________________________________________________________________________________________________
  // __________________________ _DB_ALLARME_LIFT_PUSH,// Diagnostica sulla pedaliera del motore alto/basso
  classitem.className=QString("01500");
  classitem.classDescription=QString("ERRORI PEDALIERA ALTO/BASSO");
  classitem.errlist.clear();

  erritem.codestr = QString("01501");
  erritem.codeval = LIFT_INVALID_PUSH;
  erritem.errmsg  = QString(QApplication::translate("ERRORE-LIFTPUSH","PEDALI ALTO/BASSO BLOCCATI\n", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(ERR_PIX);
  erritem.errdescr= QString("");
  classitem.errlist.append(erritem);

  errors.replace(_DB_ALLARME_LIFT_PUSH-FIRST_ALR_CLASS,classitem);
  //___________________________________________________________________________________________________________________________________________________________________
  // __________________________ _DB_ALLARME_ARM_PUSH, // Diagnostica sui pulsanti di rotazione
  classitem.className=QString("01600");
  classitem.classDescription=QString("ERRORI PEDALIERA ROTAZIONE C-ARM");
  classitem.errlist.clear();

  erritem.codestr = QString("01601");
  erritem.codeval = ARM_PUSH_TIMEOUT;
  erritem.errmsg  = QString(QApplication::translate("ERRORE-ARMPUSH","PULSANTI ROTAZIONE MANUALE BLOCCATI\n", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(ERR_PIX);
  erritem.errdescr= QString("");
  classitem.errlist.append(erritem);

  errors.replace(_DB_ALLARME_ARM_PUSH-FIRST_ALR_CLASS,classitem);

  //___________________________________________________________________________________________________________________________________________________________________
  // __________________________ _DB_ALLARME_INFO_STAT, // Messaggi di informazione generali
  classitem.className=QString("01700");
  classitem.classDescription=QString("MESSAGGI INFORMATIVI");
  classitem.errlist.clear();

  erritem.codestr = QString("01701");
  erritem.codeval = INFOMSG_NOT_READY_STARTUP;
  erritem.errmsg  = QString(QApplication::translate("ERROR NOT-READY","STARTUP INCOMPLETO", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(INFO_PIX);
  erritem.errdescr= QString("Il sistema non ha completato correttamente lo startup");
  classitem.errlist.append(erritem);

  erritem.codestr = QString("01702");
  erritem.codeval = INFOMSG_NOT_READY_HV_NOT_CALIBRATED;
  erritem.errmsg  = QString(QApplication::translate("ERROR NOT-READY","LETTURA HV NON CALIBRATA", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(INFO_PIX);
  erritem.errdescr= QString("Manca la calibrazione della lettura della tensione di rete");
  classitem.errlist.append(erritem);

  erritem.codestr = QString("01703");
  erritem.codeval = INFOMSG_NOT_READY_LSSTARTER_NOT_CALIBRATED;
  erritem.errmsg  = QString(QApplication::translate("ERROR NOT-READY","LOW SPEED STARTER NON CALIBRATO", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(INFO_PIX);
  erritem.errdescr= QString("Lo starter a bassa velocità è attivo ma non calibrato");
  classitem.errlist.append(erritem);

  erritem.codestr = QString("01704");
  erritem.codeval = INFOMSG_NOT_READY_STUDY_OPEN;
  erritem.errmsg  = QString(QApplication::translate("ERROR NOT-READY","STUDIO APERTO", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(INFO_PIX);
  erritem.errdescr= QString("La porta dello studio risulta aperta");
  classitem.errlist.append(erritem);

  erritem.codestr = QString("01705");
  erritem.codeval = INFOMSG_NOT_READY_INVALID_PAD;
  erritem.errmsg  = QString(QApplication::translate("ERROR NOT-READY","COMPRESSORE NON RICONOSCIUTO", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(INFO_PIX);
  erritem.errdescr= QString("Il compressore non è stato riconosciuto o non correttamente bloccato");
  classitem.errlist.append(erritem);

  erritem.codestr = QString("01706");
  erritem.codeval = INFOMSG_NOT_READY_NOT_COMPRESSED;
  erritem.errmsg  = QString(QApplication::translate("ERROR NOT-READY","SISTEMA NON IN COMPRESSIONE", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(INFO_PIX);
  erritem.errdescr= QString("Non è stata riconosciuta una valida compressione");
  classitem.errlist.append(erritem);

  erritem.codestr = QString("01707");
  erritem.codeval = INFOMSG_NOT_READY_INVALID_POTTER;
  erritem.errmsg  = QString(QApplication::translate("ERROR NOT-READY","POTTER NON VALIDO O NON RICONOSCIUTO", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(INFO_PIX);
  erritem.errdescr= QString("Non è stato riconosciuto un potter valido");
  classitem.errlist.append(erritem);

  erritem.codestr = QString("01708");
  erritem.codeval = INFOMSG_NOT_READY_MISSING_PATIENT_PROTECTION;
  erritem.errmsg  = QString(QApplication::translate("ERROR NOT-READY","MANCA PROTEZIONE PAZIENTE", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(INFO_PIX);
  erritem.errdescr= QString("Non è stato riconosciuto un potter valido");
  classitem.errlist.append(erritem);


  erritem.codestr = QString("01709");
  erritem.codeval = INFOMSG_OPERATING_PAGE_DISABLED_WITH_AWS;
  erritem.errmsg  = QString(QApplication::translate("ERROR NOT-READY","PAGINA OPERATIVA DISABILITATA CON PC CONNESSO", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(INFO_PIX);
  erritem.errdescr= QString("Con il PC connesso non è ammesso accedere alla pagina operativa Analogica");
  classitem.errlist.append(erritem);


  erritem.codestr = QString("01710");
  erritem.codeval = INFOMSG_NOT_READY_MISSING_CASSETTE;
  erritem.errmsg  = QString(QApplication::translate("ERROR NOT-READY","MANCA CASSETTA", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(INFO_PIX);
  erritem.errdescr= QString("Mancanza della cassetta duranze esposizione a studio aperto");
  classitem.errlist.append(erritem);

  erritem.codestr = QString("01711");
  erritem.codeval = INFOMSG_NOT_READY_EXPOSED_CASSETTE;
  erritem.errmsg  = QString(QApplication::translate("ERROR NOT-READY","CASSETTA ESPOSTA", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(INFO_PIX);
  erritem.errdescr= QString("Cassetta già esposta. Cambiare cassetta");
  classitem.errlist.append(erritem);

  errors.replace(_DB_ALLARME_INFO_STAT-FIRST_ALR_CLASS,classitem);

  //___________________________________________________________________________________________________________________________________________________________________
  // ___________________      _DB_ALLARMI_SYSCONF ____________________________________________________________________________________________________________________________________________
  classitem.className=QString("01800");
  classitem.classDescription=QString("MESSAGGI DI ERRORE DI CONFIGURAZIONE");
  classitem.errlist.clear();

  erritem.codestr = QString("01801");
  erritem.codeval = ERROR_CONF_GENERATOR;
  erritem.errmsg  = QString(QApplication::translate("ERRORE-CONFIGURAZIONE","ERRORE CONFIG GENERATORE", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(ERR_PIX);
  erritem.errdescr= QString("Il file di configurazione del generatore è errato");
  classitem.errlist.append(erritem);

  erritem.codestr = QString("01802");
  erritem.codeval = ERROR_DOSE_CALCULATOR_CONFIGURATION;
  erritem.errmsg  = QString(QApplication::translate("ERRORE-CONFIGURAZIONE","CALCOLATORE DI DOSE NON CONFIGURATO", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(ERR_PIX);
  erritem.errdescr= QString("I file di configurazione per CG o airkerma non sono correttamente configurati o sono corrotti");
  classitem.errlist.append(erritem);


  erritem.codestr = QString("01803");
  erritem.codeval = ERROR_DOSE_FILTER_CALCULATOR;
  erritem.errmsg  = QString(QApplication::translate("ERRORE-CONFIGURAZIONE","CALCOLATORE DI DOSE NON CONFIGURATO PER I FILTRI ATTIVI", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(ERR_PIX);
  erritem.errdescr= QString("I file di configurazione per CG o airkerma non sono correttamente configurati o sono corrotti");
  classitem.errlist.append(erritem);


  erritem.codestr = QString("01804");
  erritem.codeval = ERROR_STARTUP;
  erritem.errmsg  = QString(QApplication::translate("ERRORE-CONFIGURAZIONE","STARTUP ERROR IN OPERATIVO", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(ERR_PIX);
  erritem.errdescr= QString("Il sistema non è riuscito a completare con successo lo start-up");
  classitem.errlist.append(erritem);

  erritem.codestr = QString("01805");
  erritem.codeval = COLLI_WRONG_CONFIG;
  erritem.errmsg  = QString(QApplication::translate("ERRORE-CONFIGURAZIONE","ERRORE FILE DI CONFIGURAZIONE COLLIMATORE!\n", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(ERR_PIX);
  erritem.errdescr= QString("Il file di configurazione è assente o è corrotto");
  classitem.errlist.append(erritem);

  erritem.codestr = QString("01806");
  erritem.codeval = ERROR_CONF_COMPRESSOR;
  erritem.errmsg  = QString(QApplication::translate("ERRORE-CONFIGURAZIONE","ERRORE FILE DI CONFIGURAZIONE COMPRESSORE!\n", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(ERR_PIX);
  erritem.errdescr= QString("Il file di configurazione è assente o è corrotto");
  classitem.errlist.append(erritem);

  erritem.codestr = QString("01807");
  erritem.codeval = ERROR_SYS_CONFIG;
  erritem.errmsg  = QString(QApplication::translate("ERRORE-CONFIGURAZIONE","ERRORE FILE DI CONFIGURAZIONE SISTEMA!\n", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(ERR_PIX);
  erritem.errdescr= QString("Il file di configurazione è assente o è corrotto");
  classitem.errlist.append(erritem);

  erritem.codestr = QString("01808");
  erritem.codeval = ERROR_USER_CONFIG;
  erritem.errmsg  = QString(QApplication::translate("ERRORE-CONFIGURAZIONE","ERRORE FILE DI CONFIGURAZIONE USER!\n", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(ERR_PIX);
  erritem.errdescr= QString("Il file di configurazione è assente o è corrotto");
  classitem.errlist.append(erritem);

  erritem.codestr = QString("01809");
  erritem.codeval = ERROR_PACKAGE_CONFIG;
  erritem.errmsg  = QString(QApplication::translate("ERRORE-CONFIGURAZIONE","ERRORE FILE DI CONFIGURAZIONE PACKAGE!\n", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(ERR_PIX);
  erritem.errdescr= QString("Il file di configurazione è assente o è corrotto");
  classitem.errlist.append(erritem);

  erritem.codestr = QString("01810");
  erritem.codeval = ERROR_SN_CONFIG;
  erritem.errmsg  = QString(QApplication::translate("ERRORE-CONFIGURAZIONE","ERRORE MANCANZA SERIAL NUMBER!\n", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(ERR_PIX);
  erritem.errdescr= QString("Il file di configurazione è assente o è corrotto");
  classitem.errlist.append(erritem);


  erritem.codestr = QString("01811");
  erritem.codeval = ERROR_ANALOG_CONFIG;
  erritem.errmsg  = QString(QApplication::translate("ERRORE-CONFIGURAZIONE","ERRORE MANCANZA CONFIGURAZIONE ANALOGICA!\n", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(ERR_PIX);
  erritem.errdescr= QString("Il file di configurazione è assente o è corrotto");
  classitem.errlist.append(erritem);


  errors.replace(_DB_ALLARMI_SYSCONF-FIRST_ALR_CLASS,classitem);

  //___________________________________________________________________________________________________________________________________________________________________

  // __________________________ _DB_ALLARMI_ANALOGICA, // Diagnostica sui pulsanti di rotazione
  classitem.className=QString("01900");
  classitem.classDescription=QString("ERRORI RISERVATI A VERSIONE ANALOGICA");
  classitem.errlist.clear();

  erritem.codestr = QString("01901");
  erritem.codeval = ERROR_SETTING_DET_FIELD;
  erritem.errmsg  = QString(QApplication::translate("ERRORE-ANALOGICA","IMPOSTAZIONE CAMPO ESPOSIMETRO", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(ERR_PIX);
  erritem.errdescr= QString("Il comando di impostazione del campo esposimetro è fallito");
  classitem.errlist.append(erritem);

  erritem.codestr = QString("01902");
  erritem.codeval = ERROR_NO_AEC_PROFILE;
  erritem.errmsg  = QString(QApplication::translate("ERRORE-ANALOGICA","NESSUN PROFILO AEC DISPONIBILE", 0, QApplication::UnicodeUTF8));
  erritem.errpix =  QPixmap(ERR_PIX);
  erritem.errdescr= QString("Non esistono profili AEC disponibili");
  classitem.errlist.append(erritem);



  errors.replace(_DB_ALLARMI_ANALOGICA-FIRST_ALR_CLASS,classitem);

  //___________________________________________________________________________________________________________________________________________________________________
}

