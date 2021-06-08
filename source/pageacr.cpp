#include "application.h"
#include "appinclude.h"
#include "globvar.h"



#define DISABLEDVIEWPIX     "://acrPage/acrPage/disableView.png"
#define DISABLEDSUFFIXPIX     "://acrPage/acrPage/disableSuffix.png"
#define SELECTEDPIX_Y   "://acrPage/acrPage/selectYView.png"
#define SELECTEDPIX_C   "://acrPage/acrPage/selectCView.png"
#define SELECTEDSUFX_Y   "://acrPage/acrPage/selectYSuffix.png"
#define SELECTEDSUFX_C   "://acrPage/acrPage/selectCSuffix.png"

PageACR::PageACR(bool local, QString bgl, QString bgs ,bool showLogo, int w,int h, qreal angolo,QPainterPath pn, int pgpn, QPainterPath pp, int pgpp, int pg) : GWindow(bgl,showLogo,w,h, angolo,pn,pgpn,pp,pgpp,pg)
{
    QFont font;
    QPen  pen;

    timerId=0;

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


    // Campo Intestazione con lingua selezionata
    font.setPointSize(30);
    font.setStretch(40);
    intestazioneValue = new GLabel(this,QRectF(194,10,557,40),font,QColor(_C_COL),"CODICI ACR",Qt::AlignCenter);

    activationViewPix = QPixmap(SELECTEDPIX_C);
    activationSuffixPix = QPixmap(SELECTEDSUFX_Y);
    disableViewPix = QPixmap(DISABLEDVIEWPIX);
    disableSuffixPix = QPixmap(DISABLEDSUFFIXPIX);

    // SET DI PULSANTI IN COMBO (COMBO-INDEX=1) PER L'IMPOSTAZIONE DEI MODIFICATORI DI VISTA
    pulsanteXCC = new GPush((GWindow*) this, 0, &activationViewPix, &disableViewPix, setPointPath(8,345,106,476,106,476,223,345,223),345,106,1,1,FALSE,TRUE,TRUE);
    pulsanteCV = new GPush((GWindow*) this,0, &activationViewPix, &disableViewPix,setPointPath(8,480,106,611,106,480,223,611,223),480,106,1,1,FALSE,TRUE,TRUE);
    pulsanteAT = new GPush((GWindow*) this,0, &activationViewPix, &disableViewPix,setPointPath(8,615,106,746,106,615,223,746,223),615,106,1,1,FALSE,TRUE,TRUE);

    // SET DI PULSANTI IN COMBO (COMBO-INDEX=2) PER L'IMPOSTAZIONE DEI CODICI ACR
    pulsanteId = new GPush((GWindow*) this,0, &activationSuffixPix,&disableSuffixPix,setPointPath(8,346,253,410,253,410,362,346,362),346,253,2,2,FALSE,TRUE,TRUE);
    pulsanteS = new GPush((GWindow*) this,0, &activationSuffixPix, &disableSuffixPix,setPointPath(8,427,253,491,253,491,362,427,362),427,253,2,2,FALSE,TRUE,TRUE);
    pulsanteRM = new GPush((GWindow*) this,0, &activationSuffixPix, &disableSuffixPix,setPointPath(8,508,253,573,253,573,362,508,362),508,253,2,2,FALSE,TRUE,TRUE);
    pulsanteRL = new GPush((GWindow*) this,0, &activationSuffixPix, &disableSuffixPix,setPointPath(8,591,253,655,253,655,362,591,362),591,253,2,2,FALSE,TRUE,TRUE);
    pulsanteTAN = new GPush((GWindow*) this,0, &activationSuffixPix, &disableSuffixPix,setPointPath(8,675,253,740,253,740,362,675,362),675,253,2,2,FALSE,TRUE,TRUE);
    suffixEna = true;

    // Crea Pixmap Vuote
    memePix= this->addPixmap(QPixmap());
    memePix->setPos(164,102);
    memeName = new GLabel(this,QRectF(165,263,124,26),font,QColor(Qt::white),"",Qt::AlignCenter);

    // Lateralità
    latPix = this->addPixmap(QPixmap());
    latPix->setPos(164,302);


    currentView=_ACR_UNDEF_VIEW;
    currentSuffix = _ACR_UNDEF_SUFFIX;
    currentBreast = _ACR_UNDEF_BREAST;
    currViewName ="";

    timerId = startTimer(1000);


}

PageACR::~PageACR()
{
    this->killTimer(timerId);
}

// Questa funzione viene chiamata ogni volta che viene ricevuto il segnale di cambio
// pagina dalla Classe Base. Viene utilizzata per effettuare tutte le inizializzazioni del caso
void PageACR::childStatusPage(bool stat,int opt)
{
    if(stat==false){
        if(timerDisableButton) {
            killTimer(timerDisableButton);
            timerDisableButton = 0;
        }
        disableTimedButtons = false;
        disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), paginaAcr,SLOT(valueChanged(int,int)));
        disconnect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)), paginaAcr,SLOT(buttonActivationNotify(int,bool,int)));
        disconnect(pagina_language,SIGNAL(changeLanguageSgn()), paginaAcr,SLOT(languageChanged()));
        return;
    }

    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), paginaAcr,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
    connect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)), paginaAcr,SLOT(buttonActivationNotify(int,bool,int)),Qt::UniqueConnection);
    connect(pagina_language,SIGNAL(changeLanguageSgn()), paginaAcr,SLOT(languageChanged()),Qt::UniqueConnection);

    disableButtons(1000); // Disabilita i pulsanti in ingresso

    setView();
}

void PageACR::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId()==timerDisableButton)
    {
        disableTimedButtons = false;
        killTimer(timerDisableButton);
        timerDisableButton = 0;
        return;
    }

    if(ev->timerId()==timerId)
    {

        if(systemTimeUpdated)
            dateText->setPlainText(QDateTime::currentDateTime().toString("dd.MM.yy     hh.mm.ss ap"));
        else
            dateText->setPlainText(QString("--.--.--     --.--.--"));

    }
}

// Attivazione mouse nell'area della finestra
void PageACR::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    GWindow::mousePressEvent(event); // Lancia il default della classe


}

void PageACR::updateProjection(void){
    QString val = ApplicationDatabase.getDataS(_DB_SEL_PROJ);
    if(val.contains("?")) return;

    currViewName = val;
    currentView = _ACR_UNDEF_VIEW;
    currentSuffix = _ACR_UNDEF_SUFFIX;


    // Opzione selezione annullata
    if(val=="") return;

    // Reset dei pulsanti in combo
    GPush::pushResetCombo(1,DBase::_DB_NO_ECHO);
    GPush::pushResetCombo(2,DBase::_DB_NO_ECHO);

    // Estrae la vista corrente
    if(val.toAscii().at(0)=='L') currentBreast = _ACR_L_BREAST;
    else currentBreast = _ACR_R_BREAST;

    // Seleziona le viste di base
    val = val.right(val.size()-1);
    if(val.contains("CC"))          currentView = _ACR_CC_VIEW;
    else if(val.contains("MLO"))    currentView = _ACR_MLO_VIEW;
    else if(val.contains("ML"))     currentView = _ACR_ML_VIEW;
    else if(val.contains("LMO"))    currentView = _ACR_LMO_VIEW;
    else if(val.contains("LM"))     currentView = _ACR_LM_VIEW;
    else if(val.contains("MLO"))    currentView = _ACR_MLO_VIEW;
    else if(val.contains("SIO"))    currentView = _ACR_SIO_VIEW;
    else if(val.contains("MLO"))    currentView = _ACR_MLO_VIEW;
    else if(val.contains("FB"))     currentView = _ACR_FB_VIEW;
    else{
        // Nessuna vista valida
        return;
    }

}

// FUNZIONE DI AGGIORNAMENTO CAMPI VALORE CONNESSO AI CAMPI DEL DATABASE ( DA ATTIVARE CON CONNECT)
void PageACR::valueChanged(int index,int opt)
{
    QString val;

    switch(index)
    {
        case _DB_SEL_PROJ:
        if(ApplicationDatabase.getDataS(_DB_SEL_PROJ)==""){
            // Chiude la pagina poichè è stata annullata l'operazione
            prevPageHandler();
            return;
        }
        case _DB_ACCESSORIO:
            setView();
        break;

    }
}


// NOTIFICA ASSOCIATA AI PULSANTI ( DA ATTIVARE CON CONNECT)
void PageACR::buttonActivationNotify(int id, bool status,int opt)
{
    if(!isCurrentPage()) return;
    if(disableTimedButtons) return;

    // Trova il puntatore al bottone attivato
    GPush* pbutton = (GPush*) GWindowRoot.pushList.at(id);
    if(pbutton->parentWindow!=this) return; // Scarta i segnali da altre pagine


    ACR_VIEW backView = currentView;
    if(pbutton->pulsanteData==1){// Gruppo modificatori di vista
        switch(currentView){
        case _ACR_CC_VIEW:
            if(!status) currentView = _ACR_CC_VIEW;
            else if(pbutton==pulsanteXCC) currentView = _ACR_XCC_VIEW;
            else if(pbutton==pulsanteCV) currentView = _ACR_CCCV_VIEW;
            break;
        case _ACR_XCC_VIEW:
            if(!status) currentView = _ACR_CC_VIEW;
            else if(pbutton==pulsanteCV) currentView = _ACR_CCCV_VIEW;
        case _ACR_CCCV_VIEW:
            if(!status) currentView = _ACR_CC_VIEW;
            else if(pbutton==pulsanteXCC) currentView = _ACR_XCC_VIEW;
            break;
        case _ACR_MLO_VIEW:
            if(!status) currentView = _ACR_MLO_VIEW;
            else if(pbutton==pulsanteAT) currentView = _ACR_MLOAT_VIEW;
            break;
        case _ACR_MLOAT_VIEW:
            if(!status) currentView = _ACR_MLO_VIEW;
            break;
        }

        // Riaggiorna gli enables
        if(backView!=currentView) updateEnables();

    }else if(pbutton->pulsanteData==2){// Gruppo modificatori di suffisso
        if(!status) currentSuffix = _ACR_UNDEF_SUFFIX;
        else {
            if(pbutton==pulsanteId) currentSuffix = _ACR_ID;
            else if(pbutton==pulsanteS) currentSuffix = _ACR_S;
            else if(pbutton==pulsanteRM) currentSuffix = _ACR_RM;
            else if(pbutton==pulsanteRL) currentSuffix = _ACR_RL;
            else if(pbutton==pulsanteTAN) currentSuffix = _ACR_TAN;
            else currentSuffix = _ACR_UNDEF_SUFFIX;
        }
    }


}
void PageACR::updateEnables(){
    switch(currentView){
    case _ACR_CC_VIEW:
        pulsanteXCC->setEnable(true);
        pulsanteCV->setEnable(true);
        pulsanteAT->setEnable(false);
        break;
    case _ACR_XCC_VIEW:
        pulsanteXCC->setEnable(true);
        pulsanteCV->setEnable(true);
        pulsanteAT->setEnable(false);
        break;
    case _ACR_CCCV_VIEW:
        pulsanteXCC->setEnable(true);
        pulsanteCV->setEnable(true);
        pulsanteAT->setEnable(false);
        break;

    case _ACR_FB_VIEW:
    case _ACR_SIO_VIEW:
    case _ACR_LM_VIEW:
    case _ACR_LMO_VIEW:
    case _ACR_ML_VIEW:
        pulsanteXCC->setEnable(false);
        pulsanteCV->setEnable(false);
        pulsanteAT->setEnable(false);

        break;
    case _ACR_MLO_VIEW:
        pulsanteXCC->setEnable(false);
        pulsanteCV->setEnable(false);
        pulsanteAT->setEnable(true);

        break;
    case _ACR_MLOAT_VIEW:
        pulsanteXCC->setEnable(false);
        pulsanteCV->setEnable(false);
        pulsanteAT->setEnable(true);
        break;
    }


}

void PageACR::setIntestazione()
{
    QString stringa=QString(QApplication::translate("PAGINA-ACR","SELEZIONE CODICI ACR"));
    intestazioneValue->labelText=stringa;
    intestazioneValue->labelColor=studyColor;
    intestazioneValue->update();

}



void PageACR::setView()
{

    if(ApplicationDatabase.getDataU(_DB_STUDY_STAT)==_OPEN_STUDY_DICOM){
        setBackground("://acrPage/acrPage/backgroundC.png");
        studyColor = QColor(_C_COL);
        activationViewPix = QPixmap(SELECTEDPIX_C);
        activationSuffixPix = QPixmap(SELECTEDSUFX_C);

    }else{
        setBackground("://acrPage/acrPage/backgroundY.png");
        studyColor = QColor(_Y_COL);
        activationViewPix = QPixmap(SELECTEDPIX_Y);
        activationSuffixPix = QPixmap(SELECTEDSUFX_Y);
    }

    if(ApplicationDatabase.getDataU(_DB_ACCESSORIO)==POTTER_MAGNIFIER){
        suffixEna = false;
    }else{
        suffixEna = true;
    }

    updateProjection();
    setIntestazione();
    updateEnables();

    // Visualizzazione del Meme e nome relativo
    memePix->setPixmap(QPixmap(paginaProjections->getPixFile(currViewName)));
    memeName->labelText = currViewName;
    memeName->labelColor = studyColor;
    memeName->update();

    // Visualizzazione della lateralità
    if(currentBreast==_ACR_L_BREAST) latPix->setPixmap(QPixmap("://acrPage/acrPage/acr-l.png"));
    else latPix->setPixmap(QPixmap("://acrPage/acrPage/acr-r.png"));



    // I pulsanti vengono disabilitati se il Potter Mag è presente
    // E' sufficiente effettuare il setting all'apertura della pagione
    pulsanteId->setEnable(suffixEna);
    pulsanteTAN->setEnable(suffixEna);
    pulsanteRL->setEnable(suffixEna);
    pulsanteRM->setEnable(suffixEna);
    pulsanteS->setEnable(suffixEna);

}


void  PageACR::languageChanged(void)
{
    setIntestazione();
}


unsigned short PageACR::getAcrView(){
    return currentView | currentBreast;
}

unsigned char PageACR::getAcrSuffix(){
    if(!suffixEna) return _ACR_S;
    return currentSuffix;
}

