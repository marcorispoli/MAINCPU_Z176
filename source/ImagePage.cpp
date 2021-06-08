#define IMAGEPAGE_C
#include "application.h"
#include "appinclude.h"
#include "globvar.h"


ImagePage::ImagePage(QString bg, int w,int h, qreal angolo,QPainterPath pn, int pgpn, QPainterPath pp, int pgpp, int pg) : GWindow(bg,false,w,h, angolo,pn,pgpn,pp,pgpp,pg)
{

    // Abilita solo il pulsante per la pagina precedente
    nextPageEnabled = false;
    prevPageEnabled = true;

    // Crea la Pixmap che potrà essere successivamente modificata
    Pix = this->addPixmap(QPixmap(""));
    Pix->hide();

    ExitPix = this->addPixmap(QPixmap(""));
    ExitPix->show();

    imageName="";

    // Timers
    timerOn=0;
    timerId=0;
    disableTimedButtons = false; // Abilitazione pulsanti
    timerDisableButton=0;

    findTimer=0;
    findAttempt=0;
}

ImagePage::~ImagePage()
{
    this->killTimer(timerId);

}



// Questa funzione viene chiamata ogni volta che viene ricevuto il segnale di cambio
// pagina dalla Classe Base. Viene utilizzata per effettuare tutte le inizializzazioni del caso
void ImagePage::childStatusPage(bool stat,int opt)
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
    if(activateTimer) timerOn = startTimer(10000);
    prevPage = GWindowRoot.parentPage;

}

void ImagePage::timerEvent(QTimerEvent* ev)
{

    if(ev->timerId()==timerDisableButton)
    {
        disableTimedButtons = false;
        killTimer(timerDisableButton);
        return;
    }

    if(ev->timerId()==timerOn)
    {
        killTimer(timerOn);
        timerOn=0;
        if(activateTimer==false) return;
        activateTimer=false;
        prevPageHandler();
        return;
    }

    if(ev->timerId()==findTimer)
    {
        findAttempt--;
        if(existImage(imageName)==true){
            killTimer(findTimer);
            findTimer = 0;
            OpenPage();
        }else{
            if(!findAttempt){
                killTimer(findTimer);
                findTimer = 0;

                // Annulla la validità dell'immagine nel database
                imageName="";
                ApplicationDatabase.setData(_DB_IMAGE_NAME,"",DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO) ;
            }
        }
        return;
    }






}


void ImagePage::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    // Ogni azione di interazione TS viene filtrata per evitare rimbalzi
    if(disableTimedButtons) return;
    GWindow::mousePressEvent(event); // Lancia il default della classe
    disableTimedButtons=true;
    timerDisableButton = startTimer(500);


}


void ImagePage::valueChanged(int index,int opt)
{
    QString img;

    switch(index)
    {
        case _DB_IMAGE_NAME:
            img = ApplicationDatabase.getDataS(index);
            if(img=="") ClosePage();
            else if((imageName!=img)&&(isCurrentPage())) this->prevPageHandler();

            imageName = img;
            findTimer= startTimer(500);
            findAttempt = 20;
        break;

        case _DB_STUDY_STAT:
            ClosePage();
        break;

    default:
        break;
    }
}

void ImagePage::buttonActivationNotify(int id, bool status,int opt)
{


    GPush* pbutton = (GPush*) GWindowRoot.pushList.at(id);
    if(pbutton->parentWindow!=this) return; // Scarta i segnali da altre pagine


    // Solo stati attivi
    if(opt&DBase::_DB_NO_ACTION) return; // Questa condizione si impone per evitare rimbalzi da echo

    if(status==false) return;


}


void ImagePage::prevPageHandler(void)
{
    // Azione disabilitata
   // if(disableTimedButtons) return ;

    GWindow::prevPageHandler();
}

//______________________________________________________
// Parent page è la pagina chiamante
void ImagePage::OpenPage(void){
    QString filename=FILE_HOME;
    filename.append(imageName);

    // Imposta lo sfondo corrente
    if(ApplicationDatabase.getDataU(_DB_STUDY_STAT)==_OPEN_STUDY_DICOM){
        setBackground("://paginaImmagine/paginaImmagine/backgroundC.png");
        ExitPix->setPixmap(QPixmap("://paginaImmagine/paginaImmagine/exit_c.png"));
    }else{
        setBackground("://paginaImmagine/paginaImmagine/backgroundY.png");
        ExitPix->setPixmap(QPixmap("://paginaImmagine/paginaImmagine/exit_y.png"));
    }

    // Carica l'immagine
    Pix->setPixmap(QPixmap(filename));
    int w = (800-Pix->pixmap().width())/2;
    int h = (480-Pix->pixmap().height())/2;
    Pix->setPos(w,h);
    Pix->show();

    this->prevPage=GWindowRoot.parentPage; // pagina di rientro
    activateTimer=true;
    this->activatePage(DBase::_DB_NO_ECHO); // Non propaga il cambio pagina tramite echo-display

    return ;
}

// Chiude la pagina e cancella il file (file invalidato)
void ImagePage::ClosePage(void){
    if(findTimer) killTimer(findTimer);
    findTimer = 0;


    ApplicationDatabase.setData(_DB_IMAGE_NAME,"",DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO) ;
    if(this->isCurrentPage()) this->prevPageHandler();

}

void ImagePage::eraseImage(void){
    imageName == "";

    // Cancella il file
    QString command;
    command = QString("rm /home/user/*.png");
    system(command.toStdString().c_str());

}

void ImagePage::showPage(void){
    if(imageName=="") return;
    this->prevPage=GWindowRoot.parentPage; // pagina di rientro
    activateTimer=false;
    this->activatePage(DBase::_DB_NO_ECHO); // Non propaga il cambio pagina tramite echo-display
}


bool ImagePage::existImage(QString filename){
    if(filename=="") return false;
    filename.prepend(FILE_HOME);
    QFile file(filename.toStdString().c_str());
    return file.exists();
}
