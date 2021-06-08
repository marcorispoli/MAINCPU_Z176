
#include "application.h"
#include "appinclude.h"
#include "globvar.h"

#define _PGLNG_BOUND_INTEST_LABEL     94,8,288,41      // TESTO PER INTESTAZIONE

PageLanguages::PageLanguages(QApplication* app, LANGUAGES deflang, bool local, QString bgl, QString bgs ,bool showLogo,int w,int h, qreal angolo,QPainterPath pn, int pgpn, QPainterPath pp, int pgpp, int pg) : GWindow(bgl,showLogo,w,h, angolo,pn,pgpn,pp,pgpp,pg)
{
    QFont font;
    QPen  pen;

    parent = app;

    timerId=0;
    localStudy = local;
    localStudyBackground=bgl;
    openStudyBackground=bgs;
    if (localStudy==false)
        setBackground(openStudyBackground);

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
    intestazioneValue = new GLabel(this,QRectF(_PGLNG_BOUND_INTEST_LABEL ),font,QColor(_C_COL),"",Qt::AlignCenter);


    // Funzione chiamata al cambio pagina: viene chiamata in fase di init della pagina manualmente
    childStatusPage(status,1);
    setLanguage(deflang);


    pulsanteIta = new GPush((GWindow*) this, QPixmap(_PG_LNG_SELECT_PIX),setPointPath(_PG_LNG_PUSH_ITA_PATH),_PG_LNG_PUSH_ITA_POS,_COMBO_INDEX_LNG,(int)_LNG_ITA,true );
    pulsanteSpa = new GPush((GWindow*) this, QPixmap(_PG_LNG_SELECT_PIX ),setPointPath(_PG_LNG_PUSH_SPA_PATH),_PG_LNG_PUSH_SPA_POS,_COMBO_INDEX_LNG,(int)_LNG_ESP);
    pulsanteEng = new GPush((GWindow*) this, QPixmap(_PG_LNG_SELECT_PIX),setPointPath(_PG_LNG_PUSH_ENG_PATH),_PG_LNG_PUSH_ENG_POS,_COMBO_INDEX_LNG,(int)_LNG_ENG);
    pulsanteGer = new GPush((GWindow*) this, QPixmap(_PG_LNG_SELECT_PIX ),setPointPath(_PG_LNG_PUSH_GER_PATH),_PG_LNG_PUSH_GER_POS,_COMBO_INDEX_LNG,(int)_LNG_DEU);
    pulsantePor = new GPush((GWindow*) this, QPixmap(_PG_LNG_SELECT_PIX ),setPointPath(_PG_LNG_PUSH_POR_PATH),_PG_LNG_PUSH_POR_POS,_COMBO_INDEX_LNG,(int)_LNG_PRT);
    pulsanteTur = new GPush((GWindow*) this, QPixmap(_PG_LNG_SELECT_PIX ),setPointPath(_PG_LNG_PUSH_TUR_PATH),_PG_LNG_PUSH_TUR_POS,_COMBO_INDEX_LNG,(int)_LNG_TUR);
    pulsanteRus = new GPush((GWindow*) this, QPixmap(_PG_LNG_SELECT_PIX ),setPointPath(_PG_LNG_PUSH_RUS_PATH),_PG_LNG_PUSH_RUS_POS,_COMBO_INDEX_LNG,(int)_LNG_RUS);
    pulsanteFra = new GPush((GWindow*) this, QPixmap(_PG_LNG_SELECT_PIX ),setPointPath(_PG_LNG_PUSH_FRA_PATH),_PG_LNG_PUSH_FRA_POS,_COMBO_INDEX_LNG,(int)_LNG_FRA);

    // Manca la cina e la polonia

    timerId = startTimer(1000);
    linguaOK = false;
}

PageLanguages::~PageLanguages()
{
    this->killTimer(timerId);
}

// Questa funzione viene chiamata ogni volta che viene ricevuto il segnale di cambio
// pagina dalla Classe Base. Viene utilizzata per effettuare tutte le inizializzazioni del caso
void PageLanguages::childStatusPage(bool stat,int opt)
{
    if(systemTimeUpdated)
        dateText->setPlainText(QDateTime::currentDateTime().toString("dd.MM.yy     hh.mm.ss ap"));
    else
        dateText->setPlainText(QString("--.--.--     --.--.--"));

}

void PageLanguages::timerEvent(QTimerEvent* ev)
{

    if(systemTimeUpdated)
        dateText->setPlainText(QDateTime::currentDateTime().toString("dd.MM.yy     hh.mm.ss ap"));
    else
        dateText->setPlainText(QString("--.--.--     --.--.--"));
}

// Attivazione mouse nell'area della finestra
void PageLanguages::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    GWindow::mousePressEvent(event); // Lancia il default della classe

}



// NOTIFICA ASSOCIATA AI PULSANTI ( DA ATTIVARE CON CONNECT)
void PageLanguages::buttonActivationNotify(int id, bool status,int opt)
{
    // Trova il puntatore al bottone attivato
    GPush* pbutton = (GPush*) GWindowRoot.pushList.at(id);
    if(pbutton->parentWindow!=this) return; // Scarta i segnali da altre pagine

    if(opt& DBase::_DB_NO_ACTION) return;

    QString language;


    switch(pbutton->pulsanteData)
    {
    case _LNG_ITA:
        language="ITA";
       break;
    case  _LNG_DEU:
        language="DEU";
        break;
    case _LNG_FRA:
        language="FRA";
        break;
     case _LNG_ENG:
        language="ENG";
        break;
     case _LNG_PRT:
        language="PRT";
        break;
     case _LNG_RUS:
        language="RUS";
        break;
     case _LNG_ESP:
        language="ESP";
        break;
     case _LNG_TUR:
        language="TUR";
        break;
    case _LNG_POL:
       language="POL";
       break;
    case _LNG_CHN:
       language="CHN";
       break;
    case _LNG_LTU:
       language="LTU";
       break;
    }

    if(isMaster)
    {
        pConfig->userCnf.languageInterface = language;
        pConfig->saveUserCfg();
    }

    // Aggiornamento Data Base senza nessun echo o azione conseguente
    ApplicationDatabase.setData(_DB_LINGUA,language, DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);

    // Aggiornamento locale del linguaggio
    setLanguage((LANGUAGES) pbutton->pulsanteData);

}

void PageLanguages::setIntestazione()
{

    if (localStudy==false)
    {
        QString stringa=QString(QApplication::translate("LINGUA","LINGUA CORRENTE: ITALIANO"));
        intestazioneValue->labelText=stringa;
        intestazioneValue->labelColor=QColor(_C_COL);
        intestazioneValue->update();
    }
    else
    {
        QString stringa=QString(QApplication::translate("LINGUA","LINGUA CORRENTE: ITALIANO"));
        intestazioneValue->labelText=stringa;
        intestazioneValue->labelColor=QColor(_Y_COL);
        intestazioneValue->update();
    }

}

bool PageLanguages::isLanguage(QString lng){

    if(lng == "ITA") return TRUE;
    else if(lng == "DEU") return TRUE;
    else if(lng == "FRA")return TRUE;
    else if(lng == "ENG")return TRUE;
    else if(lng == "PRT")return TRUE;
    else if(lng == "RUS")return TRUE;
    else if(lng == "ESP")return TRUE;
    else if(lng == "TUR")return TRUE;
    else if(lng == "POL")return TRUE;
    else if(lng == "CHN")return TRUE;
    else if(lng == "LTU")return TRUE;
    return FALSE;


}

void PageLanguages::setLanguage(LANGUAGES lng)
{
    bool ris;
    selectedLanguage = lng;

    switch(lng)
    {
     case _LNG_ITA:
        ris = traduttore.load("traduzione_ita.qm",":/Translate/Translate");
        break;
     case  _LNG_DEU:
        ris = traduttore.load("traduzione_ger.qm",":/Translate/Translate");
        break;
     case _LNG_FRA:
        ris = traduttore.load("traduzione_fra.qm",":/Translate/Translate");
        break;
      case _LNG_ENG:
        ris = traduttore.load("traduzione_eng.qm",":/Translate/Translate");
        break;
      case _LNG_PRT:
        ris = traduttore.load("traduzione_por.qm",":/Translate/Translate");
        break;
      case _LNG_RUS:
        ris = traduttore.load("traduzione_rus.qm",":/Translate/Translate");
        break;
      case _LNG_ESP:
        ris = traduttore.load("traduzione_spa.qm",":/Translate/Translate");
        break;
      case _LNG_TUR:
        ris = traduttore.load("traduzione_tur.qm",":/Translate/Translate");
        break;
      case _LNG_POL:
        ris = traduttore.load("traduzione_pol.qm",":/Translate/Translate");
       break;
      case _LNG_CHN:
        ris = traduttore.load("traduzione_chn.qm",":/Translate/Translate");
      break;
      case _LNG_LTU:
       ris = traduttore.load("traduzione_ltu.qm",":/Translate/Translate");
      break;

    }

    parent->installTranslator(&traduttore);
    setIntestazione();
    emit changeLanguageSgn();

    // Questo flag è utilizzato dalla funzione di startup per attendere
    // l'istallazione della lingua. Questo FLAG non deve essere utilizzato come diagnostica
    linguaOK = true;
}


// FUNZIONE DI AGGIORNAMENTO CAMPI VALORE CONNESSO AI CAMPI DEL DATABASE
void PageLanguages::valueChanged(int index,int opt)
{
    int cpu_flags;
    int lngCode;
    QString lingua;

    if(opt&DBase::_DB_NO_ACTION) return;

    switch(index)
    {

    case _DB_STUDY_STAT:

        switch(ApplicationDatabase.getDataU(index)){
        case _CLOSED_STUDY_STATUS:
            break;
        case _OPEN_STUDY_LOCAL:
            localStudy=true;
            setBackground(localStudyBackground);
            setIntestazione();
            break;
        case _OPEN_STUDY_DICOM:
            localStudy=false;
            setBackground(openStudyBackground);
            setIntestazione();
            break;
        }


        break;
    case _DB_LINGUA:

        lingua = ApplicationDatabase.getDataS(index);
        lngCode = (int) PageLanguages::_LNG_ITA;

        if(lingua == "ITA")
        {
            lngCode = PageLanguages::_LNG_ITA;
            pulsanteIta->activate(TRUE,DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
        }
        else if(lingua == "ENG")
        {
            lngCode = PageLanguages::_LNG_ENG;
            pulsanteEng->activate(TRUE,DBase::_DB_NO_ACTION|DBase::_DB_NO_ECHO);
        }
        else if(lingua == "ESP")
        {
            lngCode = PageLanguages::_LNG_ESP;
            pulsanteSpa->activate(TRUE,DBase::_DB_NO_ACTION|DBase::_DB_NO_ECHO);
        }
        else if(lingua == "DEU")
        {
            lngCode = PageLanguages::_LNG_DEU;
            pulsanteGer->activate(TRUE,DBase::_DB_NO_ACTION|DBase::_DB_NO_ECHO);
        }
        else if(lingua == "PRT")
        {
            lngCode = PageLanguages::_LNG_PRT;
            pulsantePor->activate(TRUE,DBase::_DB_NO_ACTION|DBase::_DB_NO_ECHO);
        }
        else if(lingua == "TUR")
        {
            lngCode = PageLanguages::_LNG_TUR;
            pulsanteTur->activate(TRUE,DBase::_DB_NO_ACTION|DBase::_DB_NO_ECHO);
        }
        else if(lingua == "RUS")
        {
            lngCode = PageLanguages::_LNG_RUS;
            pulsanteRus->activate(TRUE,DBase::_DB_NO_ACTION|DBase::_DB_NO_ECHO);
        }
        else if(lingua == "FRA")
        {
            lngCode = PageLanguages::_LNG_FRA;
            pulsanteFra->activate(TRUE,DBase::_DB_NO_ACTION|DBase::_DB_NO_ECHO);
        }
        else if(lingua == "POL")
        {
            lngCode = PageLanguages::_LNG_POL;
            pulsanteFra->activate(TRUE,DBase::_DB_NO_ACTION|DBase::_DB_NO_ECHO);
        }
        else if(lingua == "CHN")
        {
            lngCode = PageLanguages::_LNG_CHN;
            pulsanteFra->activate(TRUE,DBase::_DB_NO_ACTION|DBase::_DB_NO_ECHO);
        }else if(lingua == "LTU")
        {
            lngCode = PageLanguages::_LNG_LTU;
            //pulsanteFra->activate(TRUE,DBase::_DB_NO_ACTION|DBase::_DB_NO_ECHO);
        }

        setLanguage((PageLanguages::LANGUAGES)lngCode);
        break;
    }
}

