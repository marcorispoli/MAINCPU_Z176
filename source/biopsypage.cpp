#include "biopsypage.h"

#define BIOPSY_PAGE_C
#include "application.h"
#include "appinclude.h"
#include "globvar.h"

#define _BACKGROUND_Y_BIOPSY  "://BiopsyPage/BiopsyPage/background_biopsy_Y.png"
#define _BACKGROUND_C_BIOPSY  "://BiopsyPage/BiopsyPage/background_biopsy_C.png"

#define _PUSH_UP_C_PIX "://BiopsyPage/BiopsyPage/biopsy_manual_up_C.png"
#define _PUSH_DWN_C_PIX "://BiopsyPage/BiopsyPage/biopsy_manual_dwn_C.png"
#define _PUSH_UP_Y_PIX "://BiopsyPage/BiopsyPage/biopsy_manual_up_Y.png"
#define _PUSH_DWN_Y_PIX "://BiopsyPage/BiopsyPage/biopsy_manual_dwn_Y.png"

static QGraphicsPixmapItem* holderPix;
static QGraphicsPixmapItem* alarmHolderPix;
static QGraphicsPixmapItem* lineaMarginePix;


//pagina0 = new MainPage(true,QString(_BACKGROUND_Y_PG_MAIN),QString(_BACKGROUND_C_PG_MAIN),TRUE,800,480,rotView,pagina0->setPointPath(RIGHT_ARROW_FRAME),(int)_PG_ACR,pagina0->setPointPath(LEFT_ARROW_FRAME),(int)_PG_SERVICE_MENU,(int)_PG_MAIN);
BiopsyPage::BiopsyPage(bool local, QString bgl, QString bgs , bool showLogo, int w,int h, qreal angolo,QPainterPath pn, int pgpn, QPainterPath pp, int pgpp, int pg) : GWindow(bgl,showLogo,w,h, angolo,pn,pgpn,pp,pgpp,pg)
{
    QFont font;

    enableBiopMoveButtons = FALSE;

    localStudy = true;
    studyColor = QColor(_Y_COL);
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

    // Campi Angolo
    font.setPointSize(35);
    font.setStretch(35);
    #define BOUND_BIOPSY_ARM 129,91,80,40
    #define BOUND_BIOPSY_TRX 21,91,80,40
    armValue = new GLabel(this,QRectF(BOUND_BIOPSY_ARM),font,QColor(_W_TEXT),"",Qt::AlignCenter);
    trxValue = new GLabel(this,QRectF(BOUND_BIOPSY_TRX),font,QColor(_W_TEXT),QString(""),Qt::AlignCenter);


    // Campo Intestazione
    #define BOUND_BIOPSY_INTESTAZIONE 14,10,460,40
    font.setPointSize(30);
    font.setStretch(40);
    intestazioneValue = new GLabel(this,QRectF(BOUND_BIOPSY_INTESTAZIONE ),font,studyColor,"",Qt::AlignCenter);


    // Campo holder
    #define BOUND_BIOPSY_HOLDER      250,258,160,30
    font.setPointSize(25);
    font.setStretch(30);
    biopHolderValue = new GLabel(this,QRectF(BOUND_BIOPSY_HOLDER),font,QColor(_W_TEXT),"",Qt::AlignCenter);

    // Campo XYZ
    #define BOUND_BIOPSY_X      23,223,50,22   // TESTO PER CAMPO X BIOPSIA
    #define BOUND_BIOPSY_Y      89,223,50,22   // TESTO PER CAMPO Y BIOPSIA
    #define BOUND_BIOPSY_Z      157,223,50,22  // TESTO PER CAMPO Z BIOPSIA

    font.setPointSize(28);
    font.setStretch(40);
    biopXValue = new GLabel(this,QRectF(BOUND_BIOPSY_X),font,QColor(_W_TEXT),"",Qt::AlignCenter);
    biopYValue = new GLabel(this,QRectF(BOUND_BIOPSY_Y),font,QColor(_W_TEXT),"",Qt::AlignCenter);
    biopZValue = new GLabel(this,QRectF(BOUND_BIOPSY_Z),font,QColor(_W_TEXT),"",Qt::AlignCenter);


    lineaMarginePix = addPixmap(QPixmap("://BiopsyPage/BiopsyPage/linea_margine.png"));
    lineaMarginePix->setPos(387,197);
    biopMaxZValue= new GLabel(this,QRectF(424,303,100,40),font,QColor(_W_TEXT),"",Qt::AlignLeft);
    biopMargineValue= new GLabel(this,QRectF(424,357,100,40),font,QColor(_W_TEXT),"",Qt::AlignLeft);

    // Campo Valore Compressione
    #define BOUND_BIOPSY_COMPRESSIONE      65,362,84,40
    font.setPointSize(40);
    font.setStretch(40);
    compressioneValue = new GLabel(this,QRectF(BOUND_BIOPSY_COMPRESSIONE),font,QColor(Qt::white),"",Qt::AlignRight);
    font.setPointSize(18);
    font.setStretch(40);
    targetValue = new GLabel(this,QRectF(23,385,70,17),font,QColor(_DBR_COL),"",Qt::AlignLeft);


    // Campo Valore Spessore
    #define BOUND_BIOPSY_SPESSORE      65,302,84,40
    font.setPointSize(40);
    font.setStretch(40);
    spessoreValue = new GLabel(this,QRectF(BOUND_BIOPSY_SPESSORE),font,QColor(Qt::white),"",Qt::AlignRight);

    // Bottoni movimento alto basso
    buttonsPix = addPixmap(QPixmap("://BiopsyPage/BiopsyPage/buttonsYnoHome.png"));
    buttonsPix->setPos(654,62);

    pulsanteBiopHome =  new GPush((GWindow*) this,setPointPath(8,675,82,774,82,774,144,675,144),675,82,0,0,FALSE);
    pulsanteBiopStepUp =  new GPush((GWindow*) this,setPointPath(8,675,178,774,178,774,227,675,227),675,178,0,0,FALSE);
    pulsanteBiopStepDown =  new GPush((GWindow*) this,setPointPath(8,675,254,774,254,774,306,675,306),675,254,0,0,FALSE);

    // Pulsante Immagine 2D
    pulsanteImgOn = new GPush((GWindow*) this, QPixmap("://BiopsyPage/BiopsyPage/imageIcon.png"),QPixmap("://BiopsyPage/BiopsyPage/imageIcon.png"),setPointPath(8,700,3,800,3,800,100,700,100),700,3,0,0);
    pulsanteImgOn->setEnable(true);
    pulsanteImgOn->setVisible(false);
    pulsanteImgOn->setOptions(DBase::_DB_NO_ECHO); // pulsante utilizzato esclusivamente dal terminale proprietario


    // Presenza dell'Holder
    #define BIOPSY_HOLDER_POS   310,131
    #define BIOPSY_HOLDER_PIX  "://BiopsyPage/BiopsyPage/biopsy_holder.png"
    #define BIOPSY_ALR_HOLDER_POS   373,103
    #define BIOPSY_ALR_HOLDER_PIX  "://BiopsyPage/BiopsyPage/allarme_holder.png"
    holderPix = addPixmap(QPixmap(BIOPSY_HOLDER_PIX));
    holderPix->setPos(BIOPSY_HOLDER_POS);
    alarmHolderPix = addPixmap(QPixmap(BIOPSY_ALR_HOLDER_PIX));
    alarmHolderPix->setPos(BIOPSY_ALR_HOLDER_POS);

    #define BIOPSY_DEMO_POS   4,4
    #define BIOPSY_DEMO_PIX  "://BiopsyPage/BiopsyPage/biopsy_demo.png"
    demoPix = addPixmap(QPixmap(BIOPSY_DEMO_PIX));
    demoPix->setPos(BIOPSY_DEMO_POS);

    calibPix = addPixmap(QPixmap("://BiopsyPage/BiopsyPage/calib_pix.png"));
    calibPix->setPos(730,4);

    rotPix = addPixmap(QPixmap("://BiopsyPage/BiopsyPage/rotPixY.png"));
    rotPix->setPos(225,76);

    #define BIOPSY_XRAY_POS   0,0
    #define BIOPSY_XRAY_PIX  "://BiopsyPage/BiopsyPage/biopsy_xray.png"
    xrayPix = addPixmap(QPixmap(BIOPSY_XRAY_PIX));
    xrayPix->setPos(BIOPSY_XRAY_POS);

    disableTimedButtons = false; // Abilitazione pulsanti
    timerId = startTimer(1000);

    isOpen = false;
}

BiopsyPage::~BiopsyPage()
{
   // this->killTimer(timerWDG);
}

void BiopsyPage::setOpenStudy(void){
    // Aggancia il database
    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), paginaImmagine,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
    connect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)), paginaImmagine,SLOT(buttonActivationNotify(int,bool,int)),Qt::UniqueConnection);

    connect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)), this,SLOT(buttonActivationNotify(int,bool,int)),Qt::UniqueConnection);

    // Con Apertura studio si apre anche la pagina
    InitBiopsyPage();

}
void BiopsyPage::setCloseStudy(void){
    // Sgancia il database
    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), paginaImmagine,SLOT(valueChanged(int,int)));
    disconnect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)), paginaImmagine,SLOT(buttonActivationNotify(int,bool,int)));
    disconnect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)), this,SLOT(buttonActivationNotify(int,bool,int)));
    paginaImmagine->eraseImage();

    isOpen = false;
    if(isMaster) pConfig->selectMainPage();
}

/*
 *
 *  Funzione chiamata al cambio pagina
 *  Il cambio pagina viene richiesto all'attivazione
 *  della Biopsia.
 *
 */
void BiopsyPage::InitBiopsyPage(void)
{    

    if(ApplicationDatabase.getDataU(_DB_STUDY_STAT)==_OPEN_STUDY_LOCAL) localStudy = true;
    else localStudy=false;

    if (localStudy==false)
    {
        studyColor = QColor(_C_COL);
        setBackground(_BACKGROUND_C_BIOPSY);
        buttonsPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/buttonsCnoHome.png"));
        rotPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/rotPixC.png"));

    }
    else
    {
        studyColor = QColor(_Y_COL);
        setBackground(_BACKGROUND_Y_BIOPSY);
        buttonsPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/buttonsYnoHome.png"));
        rotPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/rotPixY.png"));
    }

    pulsanteImgOn->setVisible(false);

    xrayPix->hide();

    if(ApplicationDatabase.getDataU(_DB_DEMO_MODE)==1){
        demoPix->show();
    }else{
        demoPix->hide();
    }

    if(ApplicationDatabase.getDataU(_DB_BIOP_UNLOCK_BUTTON)) rotPix->show();
    else   rotPix->hide();

    targetValue->labelColor=studyColor;
    targetValue->labelText = QString("%1:%2 (N)").arg(QApplication::translate("BIOPSY-PAGE","TARGET")).arg(ApplicationDatabase.getDataI(_DB_TARGET_FORCE));
    targetValue->update();

    if(ApplicationDatabase.getDataS(_DB_CALIB_SYM)!=""){
        calibPix->show();
    }else{
        calibPix->hide();
        pulsanteImgOn->setVisible(ImagePage::existImage(ApplicationDatabase.getDataS(_DB_IMAGE_NAME)));
    }



    // Attiva i campi valore per la biopsia
    setTrxAngolo();
    setArmAngolo();
    updateHolder();
    setBiopXYZ();
    setIntestazione();
    setSpessore();
    setCompressione();
    updateManualActivation();

}


// Questa funzione viene chiamata ogni volta che viene ricevuto il segnale di cambio
// pagina dalla Classe Base. Viene utilizzata per effettuare tutte le inizializzazioni del caso
void BiopsyPage::childStatusPage(bool stat,int opt)
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
    }else{
        // Caso di rientro da allarme o immagine
        if(ApplicationDatabase.getDataS(_DB_CALIB_SYM)==""){
            pulsanteImgOn->setVisible(ImagePage::existImage(ApplicationDatabase.getDataS(_DB_IMAGE_NAME)));
        }

    }
}

void BiopsyPage::timerEvent(QTimerEvent* ev)
{
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
        return;
    }


}


void BiopsyPage::mousePressEvent(QGraphicsSceneMouseEvent* event)
{

    // Ogni azione di interazione TS viene filtrata per evitare rimbalzi
    if(disableTimedButtons) return;
    GWindow::mousePressEvent(event); // Lancia il default della classe
    disableTimedButtons=true;
    timerDisableButton = startTimer(500);

}


void BiopsyPage::setIntestazione()
{
    QString stringa = ApplicationDatabase.getDataS(_DB_CALIB_SYM) ;

    if(stringa==""){
        if(ApplicationDatabase.getDataU(_DB_STUDY_STAT)==_OPEN_STUDY_DICOM){
            stringa=QString(QApplication::translate("BIOPSY-PAGE","Nome Paziente"));
            stringa.append(": ");
            stringa.append(ApplicationDatabase.getDataS(_DB_INTESTAZIONE));
        }else{
            stringa=QString(QApplication::translate("BIOPSY-PAGE","Studio Locale"));
            stringa.append(QString::fromUtf8(" N°: "));
            stringa.append(ApplicationDatabase.getDataS(_DB_INTESTAZIONE));
        }
    }

    intestazioneValue->labelText=stringa;
    intestazioneValue->labelColor=studyColor;
    intestazioneValue->update();

}

void BiopsyPage::setArmAngolo(void)
{
    QString str;

    // Prende l'angolo del Braccio dal database
    str.setNum(pConfig->convertDangolo(ApplicationDatabase.getDataI(_DB_DANGOLO)));
    str.append(QString::fromUtf8("°"));
    armValue->labelText=str;
    armValue->labelColor=QColor(_W_TEXT);
    armValue->update();
}

void BiopsyPage::setTrxAngolo(void)
{
    QString str;

    int angolo = pConfig->convertDangolo(ApplicationDatabase.getDataI(_DB_TRX));

    if((angolo>=14) && (angolo<=16)) trxValue->labelText="L";
    else if((angolo>=-16) && (angolo<=-14)) trxValue->labelText="R";
    else if((angolo>=-1) && (angolo<=1)) trxValue->labelText="S";
    else
    {
        str.setNum(angolo);
        str.append(QString::fromUtf8("°"));
        trxValue->labelText=str;
        trxValue->labelColor=QColor(_R_COL);
        trxValue->update();
        return;
    }

    trxValue->labelColor=QColor(_W_TEXT);
    trxValue->update();
}


void BiopsyPage::setBiopXYZ(void)
{
    QString x = ApplicationDatabase.getDataS(_DB_BIOP_X);
    QString y = ApplicationDatabase.getDataS(_DB_BIOP_Y);
    QString z = ApplicationDatabase.getDataS(_DB_BIOP_Z);
    QString m = ApplicationDatabase.getDataS(_DB_BIOP_MARG);
    QString mz = ApplicationDatabase.getDataS(_DB_BIOP_MAXZ);

    biopXValue->labelText=QString("%1").arg(x);
    biopYValue->labelText=QString("%1").arg(y);
    biopZValue->labelText=QString("%1").arg(z);
    if(m!="")  biopMargineValue->labelText=QString("%1").arg(m);
    else biopMargineValue->labelText=QString("");
    if(mz!="") biopMaxZValue->labelText=QString("%1").arg(mz);
    else biopMaxZValue->labelText=QString("");

    biopXValue->update();
    biopYValue->update();
    biopZValue->update();
    biopMargineValue->update();
    biopMaxZValue->update();
}


/*
 * La stringa contiene una tag per indicare che tipo di valore è:
 * s == TARGET DI COMPRESSIONE
 * f == FORZA DI COMPRESSIONE
 */
void BiopsyPage::setCompressione(void)
{

    unsigned int val = ApplicationDatabase.getDataI(_DB_FORZA);

    if(val==0){
        compressioneValue->labelColor=studyColor;
        compressioneValue->labelText="---";
        compressioneValue->update();
    }else{
        compressioneValue->labelColor=studyColor;
        compressioneValue->labelText=QString("%1").arg(val);
        compressioneValue->update();

    }

}


// SCRIVE LO SPESSORE NEL CAMPO RELATIVO DELLA PAGINA
void BiopsyPage::setSpessore(void)
{
    unsigned int val = ApplicationDatabase.getDataI(_DB_SPESSORE);

    if(val==0){

        spessoreValue->labelColor=studyColor;
        spessoreValue->labelText="---";
        spessoreValue->update();
    }else{


        spessoreValue->labelColor=studyColor;
        spessoreValue->labelText=QString("%1").arg(val);
        spessoreValue->update();

    }

}



// FUNZIONE DI AGGIORNAMENTO CAMPI VALORE CONNESSO AI CAMPI DEL DATABASE
void BiopsyPage::valueChanged(int index,int opt)
{

    switch(index)
    {
    case _DB_REQ_POWEROFF:
        if(!isMaster) return;
        if(ApplicationDatabase.getDataU(index)) pConfig->activatePowerOff();
        break;

    case _DB_XRAY_PUSH_BUTTON:
        if(!isMaster) break;
        // Segnale di attivazione pulsante raggi
        if(ApplicationDatabase.getDataU(_DB_XRAY_PUSH_BUTTON)) pToConsole->activationXrayPush();
        break;


    case _DB_DANGOLO:
        setArmAngolo();
        break;

    case _DB_BIOP_HOLDER:
        updateHolder();
        break;

    case _DB_BIOP_AGO:

        break;

    case _DB_BIOP_X:
    case _DB_BIOP_Y:
    case _DB_BIOP_Z:
    case _DB_BIOP_MARG:
    case _DB_BIOP_MAXZ:
        setBiopXYZ();
        break;
    case _DB_BIOP_MANUAL_ENA:
        updateManualActivation();
        break;

    case _DB_TRX:
        setTrxAngolo();
        break;

    case _DB_SPESSORE:
        setSpessore();
       break;

    case _DB_FORZA:
        setCompressione();
        break;

    case _DB_INTESTAZIONE:
        setIntestazione();
        break;

    case _DB_DEMO_MODE:
        if(ApplicationDatabase.getDataU(_DB_DEMO_MODE)==1) demoPix->show();
        else   demoPix->hide();
        break;

    case _DB_XRAY_SYM:
        if(ApplicationDatabase.getDataU(_DB_DEMO_MODE)==1) return;

        if(ApplicationDatabase.getDataU(index)) xrayPix->show();
        else  xrayPix->hide();

        break;
    case _DB_TARGET_FORCE:
        targetValue->labelText = QString("%1:%2 (N)").arg(QApplication::translate("BIOPSY-PAGE","TARGET")).arg(ApplicationDatabase.getDataI(_DB_TARGET_FORCE));
        targetValue->update();
        break;

    case _DB_IMAGE_NAME:
        if(ApplicationDatabase.getDataS(_DB_CALIB_SYM)!="") return;
        pulsanteImgOn->setVisible(ImagePage::existImage(ApplicationDatabase.getDataS(index)));
        break;

    case _DB_CALIB_SYM:
        if(ApplicationDatabase.getDataS(_DB_CALIB_SYM)!="") calibPix->show();
        else   calibPix->hide();
        setIntestazione();
        break;

    case _DB_BIOP_UNLOCK_BUTTON:
        if(ApplicationDatabase.getDataU(_DB_BIOP_UNLOCK_BUTTON)) rotPix->show();
        else   rotPix->hide();
        break;

    case _DB_STUDY_STAT:
        if(!ApplicationDatabase.getDataU(index)) setCloseStudy();
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

    case _DB_ACCESSORIO:
        if(!isMaster) return;

        // Si estrae la biopsia con lo studio aperto
        if(ApplicationDatabase.getDataU(index)!=BIOPSY_DEVICE){
            ApplicationDatabase.setData(_DB_STUDY_STAT,(unsigned char) 0); // Questo fa chiudere la pagina e uscire alla MAIN
        }
        break;


    }
}


void BiopsyPage::buttonActivationNotify(int id, bool status,int opt)
{

    GPush* pbutton = (GPush*) GWindowRoot.pushList.at(id);
    if(pbutton->parentWindow!=this) return; // Scarta i segnali da altre pagine


    // Solo stati attivi
    if(opt&DBase::_DB_NO_ACTION) return; // Questa condizione si impone per evitare rimbalzi da echo

    if(pbutton == pulsanteImgOn){
        paginaImmagine->showPage();
        return;
    }

    // Gestione pulsanti di step Z per biopsia
    if(isMaster)
    {
        bool ret;
        if(pbutton == pulsanteBiopStepUp) ret = pBiopsy->stepZ(-10);
        else if(pbutton == pulsanteBiopStepDown) ret = pBiopsy->stepZ(10);
        else if(pbutton == pulsanteBiopHome) ret = pBiopsy->setBiopsyData(0,0,0,193,0,0,0,"",0);

        // In caso di comando di attivazione accettato, il sistema disabilita ulteriori pressioni fino al termine del comando
        if(ret)    ApplicationDatabase.setData(_DB_BIOP_MANUAL_ENA,(unsigned char) 0 ,0);

    }

}


// Rinfresca tutte le label cambiate
void BiopsyPage::languageChanged()
{
    setIntestazione();
}


// Funzione interna pper aggiornare la visualizzazione dell'holder
void BiopsyPage::updateHolder(void){

    QString holder = ApplicationDatabase.getDataS(_DB_BIOP_HOLDER);
    if(holder.size()==0) {
        // Holder non presente o non riconosciuto
        alarmHolderPix->show();
        holderPix->hide();
        biopHolderValue->hide();
    }else{
        alarmHolderPix->hide();
        holderPix->show();
        biopHolderValue->labelText=holder;
        biopHolderValue->show();
        biopHolderValue->update();
    }

}

// Funzione di aggiornamento grafica legata ai movimenti manuali
// Oltre all'attivazione dei pulsante, la funzione aggiorna anche
// il margine di movimento disponibile
void BiopsyPage::updateManualActivation(void){
    // Verifica se i pulsanti Manuali sono abilitati
    unsigned char val = ApplicationDatabase.getDataU(_DB_BIOP_MANUAL_ENA);
    if(val) enableBiopMoveButtons=true;
    else enableBiopMoveButtons = false;

    pulsanteBiopStepUp->setEnable(enableBiopMoveButtons);
    pulsanteBiopStepDown->setEnable(enableBiopMoveButtons);
    // pulsanteBiopHome->setEnable(enableBiopMoveButtons);
    pulsanteBiopHome->setEnable(false);

    // Visualizzazione margine di movimento
    if(enableBiopMoveButtons){
        biopMargineValue->show();
        biopMaxZValue->show();
        buttonsPix->show();
        lineaMarginePix->show();
    }else{
        biopMargineValue->hide();
        biopMaxZValue->hide();
        buttonsPix->hide();
        lineaMarginePix->hide();
    }
}

