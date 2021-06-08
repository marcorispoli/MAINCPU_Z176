#define MAINPAGE_C
#include "application.h"
#include "appinclude.h"
#include "globvar.h"


//____________________________________________________________________________________________
// Temperature Tubo X
#define BOUND_TEMP_CUFFIA_LABEL    45,214,82,240   // TESTO PER CAMPO Temperatura cuffia
#define BOUND_HU_LABEL             147,214,80,280

#define PUSH_TCUFFIA_PATH           8,32,179,190,179,190,242,32,242 // PULSANTE SELEZIONE TCUFFIA
#define PUSH_TCUFFIA_POS            32,179


// HU Anode Label
//____________________________________________________________________________________________
// Sblocco compressore

#define BOUND_INTEST_LABEL     100,8,600,41            // TESTO PER INTESTAZIONE

//________________________________________________________________________________________________

#define BOUND_FORZA_LABEL      122,320,110,42        // TESTO PER CAMPO LABEL FORZA
#define BOUND_SPESSORE_LABEL   570,320,196,42       // TESTO PER CAMPO LABEL SPESSORE
//________________________________________________________________________________________________

#define BOUND_COLLIMAZIONE_LABEL    122,75,110,42    // TESTO PER CAMPO LABEL COLLIMAZIONE
#define BOUND_INGRANDIMENTO_LABEL   570,75,196,42   // TESTO PER CAMPO LABEL INGRANDIMENTO

//________________________________________________________________________________________________

#define BOUND_COLLIMAZIONE_VALUE    38,119,186,40   // TESTO PER CAMPO VALORE COLLIMAZIONE
#define BOUND_INGRANDIMENTO_VALUE   578,119,186,40   // TESTO PER CAMPO VALORE INGRANDIMENTO

//________________________________________________________________________________________________

#define BOUND_COMPRESSIONE_VALUE    38,358,150,40   // TESTO PER CAMPO VALORE COLLIMAZIONE
#define BOUND_SPESSORE_VALUE        578,358,134,40   // TESTO PER CAMPO VALORE INGRANDIMENTO
//________________________________________________________________________________________________

#define IMAGEICON       "://paginaOpenStudy/paginaOpenStudy/imageIcon.png"
#define BACKGROUNDY     "://paginaOpenStudy/paginaOpenStudy/backgroundY2.png"
#define BACKGROUNDC     "://paginaOpenStudy/paginaOpenStudy/backgroundC2.png"
#define READYPIXY       "://paginaOpenStudy/paginaOpenStudy/readyY.png"
#define READYPIXC       "://paginaOpenStudy/paginaOpenStudy/readyC.png"
#define NOTREADYPIXY    "://paginaOpenStudy/paginaOpenStudy/notReadyY.png"
#define NOTREADYPIXC    "://paginaOpenStudy/paginaOpenStudy/notReadyC.png"
#define SELPROIEZIONIY  "://paginaOpenStudy/paginaOpenStudy/selProiezioniY.png"
#define SELPROIEZIONIC  "://paginaOpenStudy/paginaOpenStudy/selProiezioniC.png"
#define SELPROIEZIONI_DEADMAN  "://paginaOpenStudy/paginaOpenStudy/selProiezioniDeadman.png"
#define SELPROIEZIONI_DISABLED  "://paginaOpenStudy/paginaOpenStudy/selProiezioniDisabled.png"
#define ABORTPROIEZIONI "://paginaOpenStudy/paginaOpenStudy/abort.png"
#define ACRPIXY         "://paginaOpenStudy/paginaOpenStudy/acrY.png"
#define ACRPIXC         "://paginaOpenStudy/paginaOpenStudy/acrC.png"
#define XRAYDEMOPIX     "://paginaOpenStudy/paginaOpenStudy/xrayDemoPix.png"

#define SELPROIEZIONIPATH 8,336,93,468,93,468,284,336,284

#define XRAY_PIX        "://paginaOpenStudy/paginaOpenStudy/X-RAY.png"
#define XRAY_POS       290,80

#define ALARM_ON_PIX    "://paginaOpenStudy/paginaOpenStudy/AlarmPix.png"
#define ALARM_ON_POS        650,180
#define ALARM_ON_PATH       8,650,180,730,180,730,240,650,240

#define BOUND_READY_VALUE   305,298,190,45

OpenStudyPage::OpenStudyPage(bool local, QString bgl, QString bgs , bool showLogo, int w,int h, qreal angolo,QPainterPath pn, int pgpn, QPainterPath pp, int pgpp, int pg) : GWindow(bgl,showLogo,w,h, angolo,pn,pgpn,pp,pgpp,pg)
{
    QFont font;

    timerId=0;

    // Disabilita l'uso dei pulsanti di cambio pagina della classe GWINDOW
    nextPageEnabled = false;
    prevPageEnabled = false;

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

    intestazioneValue = new GLabel(this,QRectF(BOUND_INTEST_LABEL ),font,QColor(_C_COL),"",Qt::AlignCenter);
    this->setIntestazione();

    // Campo Temperatura cuffia
    font.setPointSize(23);
    font.setStretch(30);
    cuffiaTLabel = new GLabel(this,QRectF(62,210,80,25),font,QColor(_GREEN_CUFFIA),QString("---"),Qt::AlignLeft);
    pulsanteTcuffia = new GPush((GWindow*) this,setPointPath(8,32,171,171,171,171,237,32,237),32,171,0,0,FALSE);
    cuffiaViewMode = 0;// 0 = HU%, 1=HU, 2 =Â°C

    // HU Anode Label
    font.setPointSize(23);
    font.setStretch(30);
    HuAnodeLabel = new GLabel(this,QRectF(104,187,80,25),font,QColor(_GREEN_CUFFIA),QString("---"),Qt::AlignLeft);

    // Campo Spessore label
    font.setPointSize(30);
    font.setStretch(40);
    spessoreLabel = new GLabel(this,QRectF(BOUND_SPESSORE_LABEL),font,QColor(_DBR_COL),"",Qt::AlignLeft);

    // Campo Compressione label
    font.setPointSize(30);
    font.setStretch(40);
    compressioneLabel = new GLabel(this,QRectF(BOUND_FORZA_LABEL),font,QColor(_DBR_COL),"",Qt::AlignRight);

    font.setPointSize(18);
    font.setStretch(40);
    targetValue = new GLabel(this,QRectF(40,362,70,17),font,QColor(_DBR_COL),"",Qt::AlignLeft);

    // Campo Collimazione label
    font.setPointSize(30);
    font.setStretch(40);
    colliLabel = new GLabel(this,QRectF(BOUND_COLLIMAZIONE_LABEL),font,QColor(_DBR_COL),"",Qt::AlignRight);

    // Campo Accessorio label
    font.setPointSize(30);
    font.setStretch(40);
    ingrLabel = new GLabel(this,QRectF(BOUND_INGRANDIMENTO_LABEL),font,QColor(_DBR_COL),"",Qt::AlignLeft);

    // Campo Valore Collimazione
    font.setPointSize(30);
    font.setStretch(40);
    collimazioneValue = new GLabel(this,QRectF(BOUND_COLLIMAZIONE_VALUE),font,QColor(Qt::white),"------",Qt::AlignCenter);


    // Campo Valore Ingrandimento
    font.setPointSize(30);
    font.setStretch(40);
    ingrandimentoValue = new GLabel(this,QRectF(BOUND_INGRANDIMENTO_VALUE),font,QColor(Qt::white),"------",Qt::AlignCenter);

    // Campo Valore Compressione
    font.setPointSize(40);
    font.setStretch(40);
    compressioneValue = new GLabel(this,QRectF(BOUND_COMPRESSIONE_VALUE),font,QColor(Qt::white),"230",Qt::AlignRight);

    // Campo Valore Spessore
    font.setPointSize(40);
    font.setStretch(40);
    spessoreValue = new GLabel(this,QRectF(BOUND_SPESSORE_VALUE),font,QColor(Qt::white),"",Qt::AlignRight);


    // Testo per l'angolo di inclinazione
    font.setPointSize(40);
    font.setStretch(40);
    angoloValue = new GLabel(this,QRectF(352,356,103,44),font,QColor(Qt::white),"",Qt::AlignCenter);

    pulsanteSbloccoDis = new GPush((GWindow*) this,setPointPath(8,24,303,127,303,127,371,24,371),24,303,0,0,FALSE);

    // Pulsante apertura menu allarmi
    pulsanteAlarmOn = new GPush((GWindow*) this, QPixmap("://paginaOpenStudy/paginaOpenStudy/alrMsg.png"),QPixmap("://paginaOpenStudy/paginaOpenStudy/alrMsg.png"),setPointPath(8,572,173,679,173,679,264,572,264),572,173,0,0);
    pulsanteAlarmOn->setEnable(true);
    font.setPointSize(20);
    font.setStretch(50);
    alarmNum = new GLabel(this,QRectF(638,234,42,20),font,QColor(_R_COL),"",Qt::AlignLeft);


    // Unlock compressor
    unlockCompressorPix = this->addPixmap(QPixmap("://paginaOpenStudy/paginaOpenStudy/unlockComprPix.png"));
    unlockCompressorPix->setPos(24,316);


    // Campo Ready
    readyPix = this->addPixmap(QPixmap(NOTREADYPIXY));
    readyPix->hide();

    // Campo Valore Ready
    font.setPointSize(40);
    font.setStretch(40);
    readyValue = new GLabel(this,QRectF(BOUND_READY_VALUE),font,QColor(Qt::white),"",Qt::AlignCenter);


    // Campo sel Proiezioni
    selProiezioniPix = this->addPixmap(QPixmap(SELPROIEZIONIY));
    selProiezioniPix->setPos(335,93);
    selProiezioniPix->hide();

    // Pulsante selezione proiezion
    pulsanteSelezioneProiezioni = new GPush((GWindow*) this,setPointPath(SELPROIEZIONIPATH),335,93,0,0,FALSE);
    pulsanteSelezioneProiezioni->setEnable(true);
    pulsanteSelezioneProiezioni->setVisible(false);

    // Campo proiezione selezionata
    font.setPointSize(23);
    font.setStretch(60);
    selectedProjPix = this->addPixmap(QPixmap(""));
    selectedProjPix->setPos(340,100);
    selectedProjVal = new GLabel(this,QRectF(340,260,124,20),font,QColor(Qt::white),"",Qt::AlignCenter);

    // Pulsante abort proiezioni
    pulsanteAbortProiezioni = new GPush((GWindow*) this, QPixmap(ABORTPROIEZIONI),QPixmap(ABORTPROIEZIONI),setPointPath(8,315,77,407,77,407,144,315,144),315,77,0,0);
    pulsanteAbortProiezioni->setEnable(true);
    pulsanteAbortProiezioni->setVisible(false);
    //pulsanteAbortProiezioni->setOptions(DBase::_DB_NO_ECHO); // pulsante utilizzato esclusivamente dal terminale proprietario

    // Pulsante di apertura pagina ACR
    acrPix = QPixmap(ACRPIXY);
    pulsanteAcr = new GPush((GWindow*) this, &acrPix, &acrPix,0,setPointPath(8,435,225,507,225,507,282,435,282),447,227,0,0,FALSE,FALSE,TRUE);

    calibPix = addPixmap(QPixmap("://paginaOpenStudy/paginaOpenStudy/calibOpenPage.png"));
    calibPix->setPos(335,93);

    // Simbolo raggi
    xRay_Pix = this->addPixmap(QPixmap(XRAY_PIX));
    xRay_Pix->setPos(XRAY_POS);
    xRay_Pix->hide();
    xRayStat= false;

    timerId = startTimer(1000);
    disableTimedButtons = false; // Abilitazione pulsanti
    isOpen = false;


}

OpenStudyPage::~OpenStudyPage()
{
    this->killTimer(timerId);
   // this->killTimer(timerWDG);
}

// Questa funzione viene chiamata ogni volta che viene ricevuto il segnale di cambio
// pagina dalla Classe Base. Viene utilizzata per effettuare tutte le inizializzazioni del caso
void OpenStudyPage::childStatusPage(bool stat,int opt)
{
    if(stat==false){
        if(timerDisableButton) {
            killTimer(timerDisableButton);
            timerDisableButton = 0;
        }
        disableTimedButtons = false;

        return;
    }

    paginaAllarmi->alarm_enable=true;
    disableButtons(500); // Disabilita i pulsanti in ingresso

    if(!isOpen){
        isOpen = true;
        setOpenStudy();
    }
}

/*_________________________________________________________________
 *
 *  APERTURA DELLA PAGINA CON LA RICEZIONE DELL'EVENTO OPEN STUDY
 *  Inizializzazione di tutti gli elementi grafici e di tutte
 *  le opzioni di ingresso pagina
 ________________________________________________________________ */
void OpenStudyPage::setOpenStudy(void){
    // Aggancia il database
    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
    connect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)), this,SLOT(buttonActivationNotify(int,bool,int)),Qt::UniqueConnection);
    connect(pagina_language,SIGNAL(changeLanguageSgn()), this,SLOT(languageChanged()),Qt::UniqueConnection);

    openStudyEvent(); // Inizializza la pagina
}

void OpenStudyPage::setCloseStudy(void){
    // Sgancia il database
    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
    disconnect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)), this,SLOT(buttonActivationNotify(int,bool,int)));
    disconnect(pagina_language,SIGNAL(changeLanguageSgn()), this,SLOT(languageChanged()));

    isOpen = false;
    if(isMaster) pConfig->selectMainPage();
}

void OpenStudyPage::openStudyEvent(void){

    if (ApplicationDatabase.getDataU(_DB_STUDY_STAT)==_OPEN_STUDY_DICOM)
    {
        setBackground(BACKGROUNDC);
        studyColor = QColor(_C_COL);
        //pulsanteSbloccoDis->setPix(PUSH_SBLK_PIXC, PUSH_SBLK_PIX_SELC );
        acrPix=QPixmap(ACRPIXC);
        pulsanteAcr->setVisible(false);

    }else
    {
        studyColor = QColor(_Y_COL);
        setBackground(BACKGROUNDY);
        //pulsanteSbloccoDis->setPix(PUSH_SBLK_PIXY, PUSH_SBLK_PIX_SELY );
        acrPix=QPixmap(ACRPIXY);
        pulsanteAcr->setVisible(false);
    }


    spessoreLabel->labelColor=studyColor;
    spessoreLabel->update();

    colliLabel->labelColor=studyColor;
    colliLabel->update();

    ingrLabel->labelColor=studyColor;
    ingrLabel->update();

    targetValue->labelColor=studyColor;
    targetValue->labelText = QString("%1:%2 (N)").arg(QApplication::translate("OPENSTUDY-PAGE","TARGET")).arg(ApplicationDatabase.getDataI(_DB_TARGET_FORCE));
    targetValue->update();


    setSpessore();
    setIngrandimento(ApplicationDatabase.getDataS(_DB_ACCESSORY_NAME));
    setCollimazione(ApplicationDatabase.getDataS(_DB_COLLIMAZIONE));
    setCompressione();
    setAngolo(pConfig->convertDangolo(ApplicationDatabase.getDataI(_DB_DANGOLO)));

    setIntestazione();    


    // L'abilitazione all'uso del pulsante raggi attiva il campo READY
    setReady(ApplicationDatabase.getDataU(_DB_READY_EXPOSURE));


    // Immagine presente/ calib pix?
    if(ApplicationDatabase.getDataS(_DB_CALIB_SYM)==""){
        calibPix->hide();
        setProiezione(ApplicationDatabase.getDataS(_DB_SEL_PROJ));
    }else{
        calibPix->show();
        setProiezione("");
    }


    if(ApplicationDatabase.getDataU(_DB_NALLARMI_ATTIVI)==0){
        pulsanteAlarmOn->setVisible(false);
        alarmNum->hide();
    }else{
        pulsanteAlarmOn->setVisible(true);
        alarmNum->show();
        alarmNum->labelText = QString("%1").arg(ApplicationDatabase.getDataU(_DB_NALLARMI_ATTIVI));
        alarmNum->update();
    }

    if(ApplicationDatabase.getDataU(_DB_COMPRESSOR_UNLOCK)){
        unlockCompressorPix->show();
    }else{
        unlockCompressorPix->hide();
    }

    // Aggiornato in funzione della lingua
    languageChanged();
}

void OpenStudyPage::setProiezione(QString name){
    if(name==""){
        // ___________________________________________________________________
        // In questa sezione, la proiezione non è ancora stata selezionata.
        // Deve quindi essere attivata opportunamente l'icona di possibile selezione
        // oppure le icone relative al deadman o situazioni di disabilitazione
        if(ApplicationDatabase.getDataU(_DB_DEAD_MEN)){
        // Con il Deadman non si può selezionare la proiezione da bordo macchina
            selProiezioniPix->setPixmap(QPixmap(SELPROIEZIONI_DEADMAN));
            pulsanteSelezioneProiezioni->setVisible(false);
        }else{
            // Con la rotazione disabilitata non si può selezionare una proiezione
            if(ApplicationDatabase.getDataU(_DB_ENABLE_MOVIMENTI)&ACTUATOR_STATUS_ENABLE_ROT_FLAG){
                if (ApplicationDatabase.getDataU(_DB_STUDY_STAT)==_OPEN_STUDY_DICOM) selProiezioniPix->setPixmap(QPixmap(SELPROIEZIONIC));
                else selProiezioniPix->setPixmap(QPixmap(SELPROIEZIONIY));
                pulsanteSelezioneProiezioni->setVisible(true);
            }else{
                selProiezioniPix->setPixmap(QPixmap(SELPROIEZIONI_DISABLED));
                pulsanteSelezioneProiezioni->setVisible(false);
            }
        }
        selProiezioniPix->show();
        pulsanteAbortProiezioni->setVisible(false);
        selectedProjPix->hide();
        selectedProjVal->setVisible(false);
        pulsanteAcr->setVisible(false);
    }else{
        // ___________________________________________________________________
        // In questa sezione, la proiezione è stata effettuata, e quindi vengono mostrate
        // gli items delle opzioni associae alla selezione corrente
        selProiezioniPix->hide();
        pulsanteAcr->setVisible(true);
        selectedProjection = name;
        selectedProjPix->setPixmap(QPixmap(paginaProjections->getPixFile(selectedProjection)));
        selectedProjPix->show();
        selectedProjVal->labelText = name;
        selectedProjVal->labelColor = studyColor;
        selectedProjVal->setVisible(true);
        selectedProjVal->update();

        pulsanteSelezioneProiezioni->setVisible(false);
        pulsanteAbortProiezioni->setVisible(true);
    }
}

void OpenStudyPage::setReady(bool stat){

    if(ApplicationDatabase.getDataU(_DB_DEMO_MODE)){
        readyPix->setPixmap(QPixmap(XRAYDEMOPIX));
        readyPix->setPos(372,285);
        readyValue->hide();
    }else{
        if(stat){
            if (ApplicationDatabase.getDataU(_DB_STUDY_STAT)==_OPEN_STUDY_DICOM) readyPix->setPixmap(QPixmap(READYPIXC));
            else readyPix->setPixmap(QPixmap(READYPIXY));
            readyValue->labelText=QString(QApplication::translate("OPENSTUDY-PAGE","READY"));
            readyValue->labelColor=QColor(_BK_TEXT);

        }else{
            if (ApplicationDatabase.getDataU(_DB_STUDY_STAT)==_OPEN_STUDY_DICOM) readyPix->setPixmap(QPixmap(NOTREADYPIXC));
            else readyPix->setPixmap(QPixmap(NOTREADYPIXY));
            readyValue->labelText=QString(QApplication::translate("OPENSTUDY-PAGE","NOT READY"));
            readyValue->labelColor=studyColor;
        }
        readyPix->setPos(299,294);
        readyValue->update();
        readyValue->show();
    }
    readyPix->show();

}

void OpenStudyPage::setXrayOn(bool stat){

    // Il simbolo raggi non viene attivato in demo
    if(ApplicationDatabase.getDataU(_DB_DEMO_MODE)){
        xRay_Pix->hide();
        return;
    }
    if(stat){
       xRay_Pix->show();

    }else{
        xRay_Pix->hide();
    }
}


/*
void OpenStudyPage::setXraySym(bool stat){

}
*/
void OpenStudyPage::timerEvent(QTimerEvent* ev)
{
    static bool loc_status=false;


    if(ev->timerId()==timerDisableButton)
    {
        disableTimedButtons = false;
        killTimer(timerDisableButton);
        return;
    }


    if(ev->timerId()==timerId)
    {

        if(systemTimeUpdated)
            dateText->setPlainText(QDateTime::currentDateTime().toString("dd.MM.yy     hh.mm.ss ap"));
        else
            dateText->setPlainText(QString("--.--.--     --.--.--"));

    }
    else
    {
        if(xRayStat==true)
        {
            if(loc_status==false)
            {
                xRay_Pix->show();
                loc_status=true;
            }else
            {
                xRay_Pix->hide();
                loc_status=false;
            }
            return;
        }

        loc_status=false;
        killTimer(ev->timerId());
        xRay_Pix->hide();
    }


}


void OpenStudyPage::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    // Ogni azione di interazione TS viene filtrata per evitare rimbalzi
    if(disableTimedButtons) return;
    GWindow::mousePressEvent(event); // Lancia il default della classe
    disableTimedButtons=true;
    timerDisableButton = startTimer(500);

}


// FUNZIONE DI AGGIORNAMENTO CAMPI VALORE CONNESSO AI CAMPI DEL DATABASE
void OpenStudyPage::valueChanged(int index,int opt)
{
    QString val;
    unsigned char cval;

    switch(index)
    {
    case _DB_PROIEZIONI:
        if(isCurrentPage()) paginaProjections->setProiezioni(); // REfresh delle proiezioni
        break;

    case _DB_SEL_PROJ:
        // Se il database contiene il carattere speciale "?" allora è una rihiesta
        // che deve essere inviata alla AWS e non deve essere visualizzata
        // Se la AWS accetterà la selezione allora invierà il comando SelProiezione
        // Che aggiornerà il database con il nome senza il ?
        val = ApplicationDatabase.getDataS(_DB_SEL_PROJ);
        if(!val.contains("?")) setProiezione(ApplicationDatabase.getDataS(_DB_SEL_PROJ));
        else if(isMaster) {
            // Notifica di avvenuta selezione
            val = val.replace("?","");
            pToConsole->notifyProjectionSelection(val);
        }


        break;

    case _DB_DANGOLO:
        setAngolo(pConfig->convertDangolo(ApplicationDatabase.getDataI(_DB_DANGOLO)));
        break;

    case _DB_DEAD_MEN:
    case _DB_ENABLE_MOVIMENTI:
        // Se l'interfaccia è in modalità selezione proiezione allora effettua il refresh..
        if(ApplicationDatabase.getDataS(_DB_SEL_PROJ)=="") setProiezione("");
        break;


    case _DB_READY_EXPOSURE:
        // L'abilitazione all'uso del pulsante raggi attiva il campo READY
        cval = ApplicationDatabase.getDataU(_DB_READY_EXPOSURE);
        setReady(cval);

        // In caso di disabilitazione del pulsante raggi viene annullata l'ultima selezione
        if(cval==0) ApplicationDatabase.setData(_DB_SEL_PROJ,"",0);
        break;

    case _DB_SPESSORE:
        setSpessore();
       break;

    case _DB_ACCESSORY_NAME:
        setIngrandimento(ApplicationDatabase.getDataS(_DB_ACCESSORY_NAME));
        break;

    case _DB_COLLIMAZIONE:
        setCollimazione(ApplicationDatabase.getDataS(_DB_COLLIMAZIONE));
        break;

    case _DB_FORZA:
        setCompressione();
        break;

    case _DB_TARGET_FORCE:
        targetValue->labelText = QString("%1:%2 (N)").arg(QApplication::translate("OPENSTUDY-PAGE","TARGET")).arg(ApplicationDatabase.getDataI(_DB_TARGET_FORCE));
        targetValue->update();
        break;

    case _DB_XRAY_SYM:
        if(ApplicationDatabase.getDataU(index)) setXrayOn(true);
        else setXrayOn(false);

    break;
    case _DB_COMPRESSOR_UNLOCK:
        if(ApplicationDatabase.getDataU(_DB_COMPRESSOR_UNLOCK)){
            unlockCompressorPix->show();
            pConfig->userCnf.enableSblocco = true;
        }else{
            unlockCompressorPix->hide();
            pConfig->userCnf.enableSblocco = false;
        }

    break;

    case _DB_T_CUFFIA:
        setTempCuffia(ApplicationDatabase.getDataI(index));
        break;

    case _DB_HU_ANODE:
        setHuAnode(ApplicationDatabase.getDataI(index));
        break;

    case _DB_NALLARMI_ATTIVI:
        if(ApplicationDatabase.getDataU(index)==0){
            pulsanteAlarmOn->setVisible(false);
            alarmNum->hide();
        }else{
            pulsanteAlarmOn->setVisible(true);
            alarmNum->show();
            alarmNum->labelText = QString("%1").arg(ApplicationDatabase.getDataU(index));
            alarmNum->update();
        }

        break;

    case _DB_DEMO_MODE:
        // Il simbolo Demo sostituisce il campo ready
        setReady(false);
        break;


    case _DB_CALIB_SYM:
        if(ApplicationDatabase.getDataS(_DB_CALIB_SYM)!=""){
            calibPix->show();
            setProiezione("");
        }else{
            calibPix->hide();
        }
        setIntestazione();
        break;

    case _DB_ACCESSORIO:
        if(!isMaster) return;

        if(ApplicationDatabase.getDataU(index)==BIOPSY_DEVICE){
            GWindowRoot.setNewPage(_PG_BIOPSY_DIGITAL,GWindowRoot.curPage,0);
        }

        break;

    case _DB_REQ_POWEROFF:
        if(!isMaster) return;
        if(ApplicationDatabase.getDataU(index)) pConfig->activatePowerOff();
        break;

    case _DB_XRAY_PUSH_BUTTON:
        if(!isMaster) break;
        // Segnale di attivazione pulsante raggi
        if(ApplicationDatabase.getDataU(_DB_XRAY_PUSH_BUTTON)) pToConsole->activationXrayPush();
        break;

    case _DB_AWS_CONNECTION:
        if(!isMaster) return;

        /*
        // Con la perdita di connessione lo studio deve essere chiuso
        if(ApplicationDatabase.getDataU(_DB_AWS_CONNECTION)==0){
             ApplicationDatabase.setData(_DB_STUDY_STAT,(unsigned char) 0); // Questo fa chiudere la pagina e uscire alla MAIN
        }
        */
        break;

    case _DB_INTESTAZIONE:
        setIntestazione();
        break;

    // Quest'info potrebbe arrivare in ritardo allo slave
    case _DB_STUDY_STAT:
        // Verifica chiusura pagina
        if(!ApplicationDatabase.getDataU(index)) setCloseStudy();
        //openStudyEvent();
        break;

    }
}

/*_________________________________________________________________________________________
    // ATTENZIONE, A FUTURA MEMORIA: I PULSANTI SONO BI-STABILI (salvo i combo)
    // Se si vuole che interagiscano ad ogni click occorre sfruttare entrambi gli stati di ON
    // e OFF
 _________________________________________________________________________________________ */
void OpenStudyPage::buttonActivationNotify(int id, bool status,int opt)
{
    unsigned char cval;

    GPush* pbutton = (GPush*) GWindowRoot.pushList.at(id);
    if(pbutton->parentWindow!=this) return; // Scarta i segnali da altre pagine
    if(opt&DBase::_DB_NO_ACTION) return; // Questa condizione si impone per evitare rimbalzi da echo


    if(pbutton == pulsanteSbloccoDis){
        if(isMaster){
            cval=ApplicationDatabase.getDataU(_DB_COMPRESSOR_UNLOCK);
            if(cval) cval=0;
            else cval=1;
            ApplicationDatabase.setData(_DB_COMPRESSOR_UNLOCK,cval,0);
        }
        return;
    }

    // Segnale ricevuto da entrambe le finestre
    if(pbutton == pulsanteAcr){
        setPage(_PG_ACR,GWindowRoot.curPage,DBase::_DB_NO_ECHO);
        return;
    }

    // Segnale ricevuto da entrambe le finestre
    if(pbutton == pulsanteSelezioneProiezioni){
        if(isMaster){
            if(paginaProjections->getListSize()!=0)
               GWindowRoot.setNewPage(_PG_PROJECTIONS,GWindowRoot.curPage,0);
        }
        return;
    }

    // Segnale ricevuto da entrambe le finestre
    if(pbutton == pulsanteAbortProiezioni){

        ApplicationDatabase.setData(_DB_SEL_PROJ,"",DBase::_DB_NO_ECHO);

        // Il master notifica la richiesta di Abort della selezione in corso
        if(isMaster) pToConsole->notifyAbortProjection();
        return;
    }

    if(pbutton==pulsanteAlarmOn)
    {
        // pulsanteAlarmOn->activate(false,DBase::_DB_NO_ACTION|DBase::_DB_NO_ECHO);
        GWindow::setPage(_PG_ALARM,GWindowRoot.curPage,DBase::_DB_NO_ECHO);
        return;
    }

    if(pbutton==pulsanteTcuffia)
    {
        // pbutton->activate(false,DBase::_DB_NO_ACTION|DBase::_DB_NO_ECHO);
        cuffiaViewMode++;
        if(cuffiaViewMode>=3) cuffiaViewMode=0;
        setTempCuffia(ApplicationDatabase.getDataI(_DB_T_CUFFIA));
        setHuAnode(ApplicationDatabase.getDataI(_DB_HU_ANODE));
        return;
    }

}


void OpenStudyPage::setIntestazione()
{
    QString stringa = ApplicationDatabase.getDataS(_DB_CALIB_SYM) ;

    if(stringa==""){
        if(ApplicationDatabase.getDataU(_DB_STUDY_STAT)==_OPEN_STUDY_DICOM){
            stringa=QString(QApplication::translate("OPENSTUDY-PAGE","Nome Paziente"));
            stringa.append(": ");
            stringa.append(ApplicationDatabase.getDataS(_DB_INTESTAZIONE));
        }else{
            stringa=QString(QApplication::translate("OPENSTUDY-PAGE","Studio Locale"));
            stringa.append(QString::fromUtf8(" NÂ°: "));
            stringa.append(ApplicationDatabase.getDataS(_DB_INTESTAZIONE));
        }
    }

    intestazioneValue->labelText=stringa;
    intestazioneValue->labelColor=studyColor;
    intestazioneValue->update();

}

void OpenStudyPage::setAngolo(int val)
{
    QString str;

    str.setNum(val);
    str.append(QString::fromUtf8("Â°"));
    angoloValue->labelText=str;
    angoloValue->labelColor=studyColor;
    angoloValue->update();
}

void OpenStudyPage::setCollimazione(QString str)
{
    collimazioneValue->labelText=str;
    collimazioneValue->labelColor=studyColor;
    collimazioneValue->update();
}
void OpenStudyPage::setIngrandimento(QString str)
{
    ingrandimentoValue->labelText=str;
    ingrandimentoValue->labelColor=studyColor;
    ingrandimentoValue->update();
}

/*
 */
void OpenStudyPage::setCompressione(void)
{
    unsigned int val = ApplicationDatabase.getDataI(_DB_FORZA);

    if(val==0){
        compressioneLabel->labelColor = studyColor;
        compressioneLabel->update();

        compressioneValue->labelColor=studyColor;
        compressioneValue->labelText="---";
        compressioneValue->update();
    }else{

        compressioneLabel->labelColor = QColor(_GREEN_COMPRESSIONE);;
        compressioneLabel->update();

        compressioneValue->labelColor=studyColor;
        compressioneValue->labelText=QString("%1").arg(val);
        compressioneValue->update();

    }

}


// SCRIVE LO SPESSORE NEL CAMPO RELATIVO DELLA PAGINA
void OpenStudyPage::setSpessore(void)
{
    unsigned int val = ApplicationDatabase.getDataI(_DB_SPESSORE);

    if(val==0){
        spessoreLabel->labelColor = studyColor;
        spessoreLabel->update();

        spessoreValue->labelColor=studyColor;
        spessoreValue->labelText="---";
        spessoreValue->update();
    }else{

        spessoreLabel->labelColor = QColor(_GREEN_COMPRESSIONE);;
        spessoreLabel->update();

        spessoreValue->labelColor=studyColor;
        spessoreValue->labelText=QString("%1").arg(val);
        spessoreValue->update();

    }

}


// Rinfresca tutte le label cambiate
void OpenStudyPage::languageChanged()
{

    setIntestazione();

    // Sezione collimatore
    if(isMaster) pCollimatore->changedPadNotify();

    // Sezione Standard
    spessoreLabel->labelText=QApplication::translate("OPENSTUDY-PAGE","SPESSORE");
    spessoreLabel->update();
    compressioneLabel->labelText=QApplication::translate("OPENSTUDY-PAGE","COMPRESSIONE");
    compressioneLabel->update();    
    colliLabel->labelText=QApplication::translate("OPENSTUDY-PAGE","COLLIMAZIONE");
    colliLabel->update();
    ingrLabel->labelText=QApplication::translate("OPENSTUDY-PAGE","ACCESSORIO");
    ingrLabel->update();

}



/*
 *  Il calcolo degli HU sui tubi IAE viene calcolato con una formula empirica
 *  che mette in relazione la variazione della temperatura dell'anodo a seguito
 *  dell'erogazione di un massimo di HU.
 *  Per i tubi in oggetto IAE dichiara che HUMAX*0.9 produce una variazione
 *  sulla cuffia di circa 40Â°, con HU MAX = 500000;
 *  Il calcolo dunque degli HU accumulati dall'anodo si effettua con una proporzione
 *  tra la variazione di temperatura del tubo e i 40Â° di variazione di riferimento.
 *
 *
 *                          (550000 * 0.9) * (Tcuffia-Tamb)
 *          HU cuffia =    ----------------------------------
 *                                  (40)
 *
 * Non avendo la temperatura ambiente disponibile si assumerÃ  che essa sia 25Â°C.
 * Con questa assunzione potrÃ  accadere che all'accensione, con un temperatura piÃ¹ bassa,
 * gli HU siano negativi (non senso). Per evitare questo paradosso si imporrÃ
 * a zero qualsiasi valore negativo.
 *
 * Per quanto riguarda la risoluzione del mammografo, si deve considerare che
 * la sensibilitÃ  del termometro di misura della T cuffia ha una risoluzione di circa
 * 2Â°/lsb. Pertanto il sistema Ã¨ in grado di misurare differenze di circa
 *                          - DUmin = 24750 HU
 * Questo significa che occorrono almeno 5/6 sequenze raggi (35KV, 100mAs) prima di poter leggere
 * una differenza effettiva.
 *
 */
void OpenStudyPage::setTempCuffia(int temp)
{
    QString str;
    float hu;

    // Estrazione status cuffia
    int cuffiaStat = (temp >> 8)& 0x00FF;
    temp = temp &0x00FF;

    // 2 = ALLARME, 1=WARNING, 0 = OK
    if(cuffiaStat==2) cuffiaTLabel->labelColor=QColor(_RED_CUFFIA);
    else if(cuffiaStat==1) cuffiaTLabel->labelColor=QColor(_ORANGE_CUFFIA);
    else cuffiaTLabel->labelColor=QColor(_GREEN_CUFFIA);

    switch(cuffiaViewMode)
    {
    case 2:
        str.setNum(temp);
        str.append(QString::fromUtf8("Â°"));
    break;
    case 0: // HU%
        hu = (float) 100 * 0.9 * (temp - 25) / 40;
        if(hu<0) hu=0;

        str.setNum((int) hu);
        str.append(QString(" HU%"));
        break;
    case 1: // HU
        hu = (float) 550 * 0.9 * (temp - 25) / 40;
        if(hu<0) hu=0;

        str.setNum((int) hu);
        str.append(QString(" kHU"));
        break;
    }


    cuffiaTLabel->labelText=str;
    cuffiaTLabel->update();

}


void OpenStudyPage::setHuAnode(int khu)
{
    QString str;
    float hu;

    // Estrazione status cuffia
    int huStat = (khu >> 12)& 0x00FF;
    khu = khu & 0x0FFF;


    // 2 = ALLARME, 1=WARNING, 0 = OK
    // Livelli di guardia
    if(huStat==2) HuAnodeLabel->labelColor=QColor(_RED_CUFFIA);
    else if(huStat==1) HuAnodeLabel->labelColor=QColor(_ORANGE_CUFFIA);
    else HuAnodeLabel->labelColor=QColor(_GREEN_CUFFIA);

    switch(cuffiaViewMode)
    {
    case 2:
    case 0: // HU%
        hu = (float) khu * 100 / 300;
        if(hu<0) hu=0;

        str.setNum((int) hu);
        str.append(QString(" HU%"));
        break;
    case 1: // HU
        hu = (float) khu ;
        if(hu<0) hu=0;

        str.setNum((int) hu);
        str.append(QString(" kHU"));
        break;
    }


    HuAnodeLabel->labelText=str;
    HuAnodeLabel->update();

}

