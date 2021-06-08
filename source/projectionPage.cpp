#define PROJECTIONPAGE_C
#include "application.h"
#include "appinclude.h"
#include "globvar.h"


#define BACKGROUNDC "://paginaProiezioni/paginaProiezioni/backgroundC.png"
#define BACKGROUNDY "://paginaProiezioni/paginaProiezioni/backgroundY.png"
#define BOUND_INTEST_LABEL     100,8,600,41            // TESTO PER INTESTAZIONE

#define X0 114
#define X1 255
#define X2 395
#define X3 536


#define Y0 69
#define Y1 253

#define DX 123
#define DY 140

#define YL0 228
#define YL1 412
#define LABELBOX    124,20

ProjectionPage::ProjectionPage(bool local, QString bgl, QString bgs , bool showLogo, int w,int h, qreal angolo,QPainterPath pn, int pgpn, QPainterPath pp, int pgpp, int pg) : GWindow(bgl,showLogo,w,h, angolo,pn,pgpn,pp,pgpp,pg)
{
    QFont font;

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

    // Inizializza il vettore delle immagini
    proiezioni.clear();

    QString prova = getPixFile("RCC");

    Meme[0] = new GPush((GWindow*) this, QPixmap(prova),QPixmap(prova),setPointPath(8,X0,Y0,X0+DX,Y0,X0+DX,Y0+DY,X0,Y0+DY),X0,Y0,0,0,false);
    Meme[0]->setEnable(true);
    Meme[0]->setOptions(DBase::_DB_NO_ECHO); // pulsante utilizzato esclusivamente dal terminale proprietario

    Meme[1] = new GPush((GWindow*) this, QPixmap(prova),QPixmap(prova),setPointPath(8,X1,Y0,X1+DX,Y0,X1+DX,Y0+DY,X1,Y0+DY),X1,Y0,0,1,false);
    Meme[1]->setEnable(true);
    Meme[1]->setOptions(DBase::_DB_NO_ECHO); // pulsante utilizzato esclusivamente dal terminale proprietario

    Meme[2] = new GPush((GWindow*) this, QPixmap(prova),QPixmap(prova),setPointPath(8,X2,Y0,X2+DX,Y0,X2+DX,Y0+DY,X2,Y0+DY),X2,Y0,0,2,false);
    Meme[2]->setEnable(true);
    Meme[2]->setOptions(DBase::_DB_NO_ECHO); // pulsante utilizzato esclusivamente dal terminale proprietario

    Meme[3] = new GPush((GWindow*) this, QPixmap(prova),QPixmap(prova),setPointPath(8,X3,Y0,X3+DX,Y0,X3+DX,Y0+DY,X3,Y0+DY),X3,Y0,0,3,false);
    Meme[3]->setEnable(true);
    Meme[3]->setOptions(DBase::_DB_NO_ECHO); // pulsante utilizzato esclusivamente dal terminale proprietario


    Meme[4] = new GPush((GWindow*) this, QPixmap(prova),QPixmap(prova),setPointPath(8,X0,Y1,X0+DX,Y1,X0+DX,Y1+DY,X0,Y1+DY),X0,Y1,0,4,false);
    Meme[4]->setEnable(true);
    Meme[4]->setOptions(DBase::_DB_NO_ECHO); // pulsante utilizzato esclusivamente dal terminale proprietario

    Meme[5] = new GPush((GWindow*) this, QPixmap(prova),QPixmap(prova),setPointPath(8,X1,Y1,X1+DX,Y1,X1+DX,Y1+DY,X1,Y1+DY),X1,Y1,0,5,false);
    Meme[5]->setEnable(true);
    Meme[5]->setOptions(DBase::_DB_NO_ECHO); // pulsante utilizzato esclusivamente dal terminale proprietario

    Meme[6] = new GPush((GWindow*) this, QPixmap(prova),QPixmap(prova),setPointPath(8,X2,Y1,X2+DX,Y1,X2+DX,Y1+DY,X2,Y1+DY),X2,Y1,0,6,false);
    Meme[6]->setEnable(true);
    Meme[6]->setOptions(DBase::_DB_NO_ECHO); // pulsante utilizzato esclusivamente dal terminale proprietario

    Meme[7] = new GPush((GWindow*) this, QPixmap(prova),QPixmap(prova),setPointPath(8,X3,Y1,X3+DX,Y1,X3+DX,Y1+DY,X3,Y1+DY),X3,Y1,0,7,false);
    Meme[7]->setEnable(true);
    Meme[7]->setOptions(DBase::_DB_NO_ECHO); // pulsante utilizzato esclusivamente dal terminale proprietario

    font.setPointSize(23);
    font.setStretch(60);
    MemeLabel[0] = new GLabel(this,QRectF(X0,YL0,LABELBOX),font,QColor(_BK_TEXT),QString(""),Qt::AlignCenter);
    MemeLabel[1] = new GLabel(this,QRectF(X1,YL0,LABELBOX),font,QColor(_BK_TEXT),QString(""),Qt::AlignCenter);
    MemeLabel[2] = new GLabel(this,QRectF(X2,YL0,LABELBOX),font,QColor(_BK_TEXT),QString(""),Qt::AlignCenter);
    MemeLabel[3] = new GLabel(this,QRectF(X3,YL0,LABELBOX),font,QColor(_BK_TEXT),QString(""),Qt::AlignCenter);

    MemeLabel[4] = new GLabel(this,QRectF(X0,YL1,LABELBOX),font,QColor(_BK_TEXT),QString(""),Qt::AlignCenter);
    MemeLabel[5] = new GLabel(this,QRectF(X1,YL1,LABELBOX),font,QColor(_BK_TEXT),QString(""),Qt::AlignCenter);
    MemeLabel[6] = new GLabel(this,QRectF(X2,YL1,LABELBOX),font,QColor(_BK_TEXT),QString(""),Qt::AlignCenter);
    MemeLabel[7] = new GLabel(this,QRectF(X3,YL1,LABELBOX),font,QColor(_BK_TEXT),QString(""),Qt::AlignCenter);



    timerId = startTimer(1000);
    disableTimedButtons = false; // Abilitazione pulsanti
    timerDisableButton = 0;
}

ProjectionPage::~ProjectionPage()
{
    this->killTimer(timerId);

}

// Questa funzione viene chiamata ogni volta che viene ricevuto il segnale di cambio
// pagina dalla Classe Base. Viene utilizzata per effettuare tutte le inizializzazioni del caso
void ProjectionPage::childStatusPage(bool stat,int opt)
{
    if(stat==false){
        if(timerDisableButton) {
            killTimer(timerDisableButton);
            timerDisableButton = 0;
        }
        disableTimedButtons = false;
        disconnect(pagina_language,SIGNAL(changeLanguageSgn()), this,SLOT(languageChanged()));
        disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
        disconnect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)), this,SLOT(buttonActivationNotify(int,bool,int)));

        return;
    }


    connect(pagina_language,SIGNAL(changeLanguageSgn()), this,SLOT(languageChanged()),Qt::UniqueConnection);
    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
    connect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)), this,SLOT(buttonActivationNotify(int,bool,int)),Qt::UniqueConnection);

    disableButtons(1000); // Disabilita i pulsanti in ingresso
    initWindow();
    return;

}


void ProjectionPage::timerEvent(QTimerEvent* ev)
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


void ProjectionPage::mousePressEvent(QGraphicsSceneMouseEvent* event)
{

    GWindow::mousePressEvent(event); // Lancia il default della classe

}
void ProjectionPage::nextPageHandler(void)
{
    // Azione disabilitata
    if(disableTimedButtons) return ;

    GWindow::nextPageHandler();
}

void ProjectionPage::prevPageHandler(void)
{
    // Azione disabilitata
    if(disableTimedButtons) return ;

    GWindow::prevPageHandler();
}


// FUNZIONE DI AGGIORNAMENTO CAMPI VALORE CONNESSO AI CAMPI DEL DATABASE
void ProjectionPage::valueChanged(int index,int opt)
{
    QString val;

    switch(index)
    {
    case _DB_PROIEZIONI:
        initWindow(); // REfresh delle proiezioni
        break;

    case _DB_SEL_PROJ:
        // Se il database contiene il carattere speciale "?" allora è una rihiesta
        // che deve essere inviata alla AWS e non deve essere visualizzata
        // Se la AWS accetterà la selezione allora invierà il comando SelProiezione
        // Che aggiornerà il database con il nome senza il ?
        val = ApplicationDatabase.getDataS(_DB_SEL_PROJ);
        if(val.contains("?")){
            if(!isMaster) return;
            val = val.replace("?","");
            pToConsole->notifyProjectionSelection(val);
            return;
        }

        default:
        break;
    }
}

/*_________________________________________________________________________________________
    // ATTENZIONE, A FUTURA MEMORIA: I PULSANTI SONO BI-STABILI (salvo i combo)
    // Se si vuole che interagiscano ad ogni click occorre sfruttare entrambi gli stati di ON
    // e OFF
 _________________________________________________________________________________________ */
void ProjectionPage::buttonActivationNotify(int id, bool status,int opt)
{
    GPush* pbutton = (GPush*) GWindowRoot.pushList.at(id);
    if(pbutton->parentWindow!=this) return; // Scarta i segnali da altre pagine
    if(opt&DBase::_DB_NO_ACTION) return; // Questa condizione si impone per evitare rimbalzi da echo
    if(disableTimedButtons) return;
    if(!isCurrentPage()) return;

    if(pbutton->pulsanteData>7) return;

    // Attiva il pannello di warning con i colori dello studio in corso
    pWarningBox->userData = proiezioni[pbutton->pulsanteData];

    QString title = QString(QApplication::translate("PROJECTION-PAGE","PROIEZIONE SELEZIONATA"));
    title.append(QString(":%1").arg(proiezioni[pbutton->pulsanteData]));
    QString msg = QString(QApplication::translate("PROJECTION-PAGE","MESSAGGIO CONFERMA SELEZIONE PROIEZIONE"));

    if(ApplicationDatabase.getDataU(_DB_STUDY_STAT)==_OPEN_STUDY_DICOM)
        pWarningBox->activate(title,msg,60,msgBox::_BUTTON_OK|msgBox::_BUTTON_CANC|msgBox::_COLOR_DICOM);
    else
        pWarningBox->activate(title,msg,60,msgBox::_BUTTON_OK|msgBox::_BUTTON_CANC|msgBox::_COLOR_LOCAL);
    connect(pWarningBox,SIGNAL(buttonOkSgn()),this,SLOT(onOkSelection()),Qt::UniqueConnection);
    connect(pWarningBox,SIGNAL(buttonCancSgn()),this,SLOT(onCancSelection()),Qt::UniqueConnection);
    pWarningBox->setCANCLabel(QString(QApplication::translate("PROJECTION-PAGE","BOTTONE ANNULLA")));
    pWarningBox->setOKLabel(QString(QApplication::translate("PROJECTION-PAGE","BOTTONE ACCETTA")));

    // Disabilita i bottoni del pannello principale per evitare sovrapposizioni
    if(timerDisableButton) killTimer(timerDisableButton);
    timerDisableButton = 0;
    disableTimedButtons=true;

}

void ProjectionPage::onOkSelection(void)
{
    disableTimedButtons=false;

    // Aggiorna il database con il carattere speciale di richiesta ad AWS e nopn di visualizzazione
    ApplicationDatabase.setData(_DB_SEL_PROJ,pWarningBox->userData.append("?"),0);
    prevPageHandler();

}

void ProjectionPage::onCancSelection(void)
{
    disableTimedButtons=false;
    prevPageHandler();
}


QString ProjectionPage::getPixFile(QString name)
{
    return QString("://paginaProiezioni/paginaProiezioni/%1.png").arg(name);
}

/*_________________________________________________________________
 *
 *  APERTURA DELLA PAGINA CON LA RICEZIONE DELL'EVENTO OPEN STUDY
 *  Inizializzazione di tutti gli elementi grafici e di tutte
 *  le opzioni di ingresso pagina
 ________________________________________________________________ */

void ProjectionPage::initWindow(void){

    if (ApplicationDatabase.getDataU(_DB_STUDY_STAT)==_OPEN_STUDY_DICOM)
    {
        setBackground(BACKGROUNDC);
        studyColor = QColor(_C_COL);

    }else
    {
        studyColor = QColor(_Y_COL);
        setBackground(BACKGROUNDY);

    }

    setIntestazione();
    setProiezioni();




    // Carica le mime assegnate dalla AWS
    for(int index=0; index<8;index++){
        if(index<proiezioni.size()){

            // I pulsanti devono essere utilizzati su entrambi gli stati
            Meme[index]->pulsanteNotActivatedPix->load(getPixFile(proiezioni[index]));
            Meme[index]->pulsanteActivatedPix->load(getPixFile(proiezioni[index]));
            Meme[index]->setVisible(true);
            MemeLabel[index]->labelText = QString(proiezioni[index]);
            MemeLabel[index]->update();
        }else{
            Meme[index]->setVisible(false);
            MemeLabel[index]->labelText = "";
            MemeLabel[index]->update();
        }
    }
}

void ProjectionPage::setProiezioni(){
    // Dal Database
    if(ApplicationDatabase.getDataS(_DB_PROIEZIONI)=="") proiezioni.clear();
    else proiezioni = getList(ApplicationDatabase.getDataS(_DB_PROIEZIONI));
}

void ProjectionPage::setIntestazione()
{
    intestazioneValue->labelText=QString(QApplication::translate("PROJECTION-PAGE","SELEZIONE PROIEZIONE"));
    intestazioneValue->labelColor=studyColor;
    intestazioneValue->update();
    return;
}

// Rinfresca tutte le label cambiate
void ProjectionPage::languageChanged()
{
    setIntestazione();
}


QList<QString> ProjectionPage::getList(QString stringa)
{
    QList<QString> lista;
    QByteArray frame = stringa.toAscii();
    QString item;
    int i;

    lista.clear();
    while(frame.size())
    {
        i = frame.indexOf(" ");
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
