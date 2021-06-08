#include "pannelloProiezioni.h"
#include "analog.h"
#include "../application.h"
#include "../appinclude.h"
#include "../globvar.h"


pannelloProiezioni::pannelloProiezioni(QGraphicsView* view){

    parent = view;

    QFont font;
    font.setFamily("DejaVu Serif");
    font.setBold(true);
    font.setWeight(75);
    font.setItalic(false);

    view->setScene(this);
    view->setStyleSheet("background-image: url(:/transparent.png);");
    view->setWindowFlags(Qt::FramelessWindowHint);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    view->setGeometry(163,2,473,473);
    view->setSceneRect(0,0,473,473);

    backgroundPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloProiezioni.png"));
    backgroundPix->setOffset(0,0);

    latRect = QRectF(185,180,100,60);
    exitRect = QRectF(185,260,100,40);


    timerDisable=0;
    open_flag = false;
    parent->hide();

}

pannelloProiezioni::~pannelloProiezioni()
{
    return;
}

void pannelloProiezioni::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId()==timerDisable)
    {
        killTimer(timerDisable);
        timerDisable=0;
    }

}

void pannelloProiezioni::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mousePressEvent(event);

    if(timerDisable) return;
    timerDisable=startTimer(500);    

    // Segnale di cambio lateralità
    if(latRect.contains(event->scenePos())){
        // Cambio lateralità
        if(ApplicationDatabase.getDataI(_DB_CURRENT_LAT) == BREAST_L) ApplicationDatabase.setData(_DB_CURRENT_LAT,(int) BREAST_R);
        else ApplicationDatabase.setData(_DB_CURRENT_LAT,(int) BREAST_L);
    }else if(exitRect.contains(event->scenePos())){
        ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_PROJEXIT_SELECTION ,DBase::_DB_FORCE_SGN);
    }else{
        QPointF mouse = event->scenePos();
        // Verifica della vista selezionata
        int selected = PROJ_UNDEF;

        if(ApplicationDatabase.getDataI(_DB_CURRENT_LAT) == BREAST_L){
            if((mouse.x() < 180)){
                if((mouse.y() < 176)){
                    selected = PROJ_LSIO;
                }else if((mouse.y() > 295)){
                    selected = PROJ_LLMO;
                }else{
                    selected = PROJ_LLM;
                }
            } else if((mouse.x() > 290)){
                if((mouse.y() < 176)){
                    selected = PROJ_LMLO;
                }else if((mouse.y() > 295)){
                    selected = PROJ_LISO;
                }else{
                    selected = PROJ_LML;
                }
            }else{
                if((mouse.y() < 130)){
                     selected = PROJ_LCC;
                }else if((mouse.y() > 340)){
                     selected = PROJ_LFB;
                }
            }
        }else{
            if((mouse.x() < 180)){
                if((mouse.y() < 176)){
                    selected = PROJ_RMLO;
                }else if((mouse.y() > 295)){
                    selected = PROJ_RISO;
                }else{
                    selected = PROJ_RML;
                }
            } else if((mouse.x() > 290)){
                if((mouse.y() < 176)){
                    selected = PROJ_RSIO;
                }else if((mouse.y() > 295)){
                    selected = PROJ_RLMO;
                }else{
                    selected = PROJ_RLM;
                }
            }else{
                if((mouse.y() < 130)){
                     selected = PROJ_RCC;
                }else if((mouse.y() > 340)){
                     selected = PROJ_RFB;
                }
            }

        }

        if(selected != PROJ_UNDEF){

            ApplicationDatabase.setData(_DB_SELECTED_PROJECTION,(int) selected ,DBase::_DB_NO_CHG_SGN);
            ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_PROJEXIT_SELECTION ,DBase::_DB_FORCE_SGN);
        }
    }



}

/* _____________________________________________________________________________________________________________________________________
 * Impostazione lateralità corrente
 _____________________________________________________________________________________________________________________________________ */

void pannelloProiezioni::setLat(int lat){

    if(lat == BREAST_L){
        backgroundPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloRotondoProiezioniL.png"));
    }else{
        backgroundPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloRotondoProiezioniR.png"));
    }
    return;
}
/* _____________________________________________________________________________________________________________________________________
 * Chiamato per aprire il pannello comandi

 _____________________________________________________________________________________________________________________________________ */

void pannelloProiezioni::open(void){
    if(open_flag) return;

    open_flag = true;
    setLat(ApplicationDatabase.getDataI(_DB_CURRENT_LAT));
    parent->show();

    timerDisable=startTimer(500);
}

void pannelloProiezioni::exit(){
    if(!open_flag) return;

    // Operazioni in chiusura
    open_flag = false;
    parent->hide();
}

