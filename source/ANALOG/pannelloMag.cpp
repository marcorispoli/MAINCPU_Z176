#include "pannelloMag.h"
#include "analog.h"
#include "../application.h"
#include "../appinclude.h"
#include "../globvar.h"
#include "pageOpenAnalogic.h"
extern AnalogPageOpen* paginaOpenStudyAnalogic;

pannelloMag::pannelloMag(QGraphicsView* view, QWidget* widget){
    parentView = view;
    parentWidget = widget;


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
    this->setSceneRect(0,0,473,473);

    backgroundPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloIngranditore.png"));
    backgroundPix->setPos(0,0);

    formatList.clear();    
    formatList.append("FORMAT 24x30");
    formatList.append("FORMAT 18x24");

    font.setPointSize(26);
    font.setStretch(50);
    formatLabel = new GLabel(this,QRectF(118,70,260,60),font,QColor(_W_TEXT),"",Qt::AlignCenter);

    timerDisable=0;
    timerBlink = 0;
    open_flag = false;
    parentView->hide();
}

pannelloMag::~pannelloMag()
{
    return;
}

void pannelloMag::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId()==timerDisable)
    {
        killTimer(timerDisable);
        timerDisable=0;
        return;
    }

    if(ev->timerId()==timerBlink)
    {
        blinking = !blinking;

        if(blinking) formatLabel->show();
        else formatLabel->hide();
        return;
    }

}


void pannelloMag::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mousePressEvent(event);

    if(timerDisable) return;
    timerDisable=startTimer(500);
    QPointF mouse = event->scenePos();



    // scroll
    if(mouse.y()<=150){
        if(mouse.x()<=230){
            // scroll left
            onScrollLeft();

        }else{
            // scroll right
            onScrollRight();
        }

        return;
    }



    // Tasto Ok
    if(mouse.y()>=380){
        onOk();
        disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
        ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_MAGEXIT_SELECTION ,DBase::_DB_FORCE_SGN);
        return;
    }


}


void pannelloMag::open(){
    if(open_flag) return;
    open_flag = true;
    parentView->show();
    timerDisable=startTimer(500);

    // Imposta la selezione corrente
    int modo = ApplicationDatabase.getDataI(_DB_MAGNIFIER_COLLI_FORMAT);
    if(modo > 1) modo = 0;
    // ApplicationDatabase.setData(_DB_MAGNIFIER_COLLI_FORMAT, modo, DBase::_DB_NO_ACTION);

    blinking = true;
    formatLabel->show();
    timerBlink = startTimer(200);
    formatLabel->setPlainText(formatList[modo].toAscii().data());
    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
}

void pannelloMag::exit(){
    if(!open_flag) return;

    // Operazioni in chiusura
    open_flag = false;
    parentView->hide();

    if(timerBlink){
        killTimer(timerBlink);
        timerBlink = 0;
    }

    if(timerDisable){
        killTimer(timerDisable);
        timerDisable = 0;
    }

}


void pannelloMag::valueChanged(int index,int opt)
{
    if((isMaster)&&(opt&DBase::_DB_ONLY_SLAVE_ACTION)) return;
    if((!isMaster)&&(opt&DBase::_DB_ONLY_MASTER_ACTION)) return;
    int modo;

    switch(index){
    case _DB_MAGNIFIER_COLLI_FORMAT:
        modo = ApplicationDatabase.getDataI(index);
        if(modo > 1) modo = 0;

        // Imposta la selezione corrente
        formatLabel->setPlainText(formatList[modo].toAscii().data());
        break;
    }
}

// Callback sulla pressione delle frecce.
void pannelloMag::onScrollLeft(void){
    int modo = ApplicationDatabase.getDataI(_DB_MAGNIFIER_COLLI_FORMAT);
    if(modo > 1) modo = 0;

    if(modo==0) modo = 1;
    else modo = 0;
    ApplicationDatabase.setData(_DB_MAGNIFIER_COLLI_FORMAT, modo, DBase::_DB_NO_ACTION);
}

void pannelloMag::onScrollRight(void){
    onScrollLeft();
}

void pannelloMag::onOk(void){
    int modo = ApplicationDatabase.getDataI(_DB_MAGNIFIER_COLLI_FORMAT);
    if(modo > 1){
        modo = 0;
        ApplicationDatabase.setData(_DB_MAGNIFIER_COLLI_FORMAT, modo, DBase::_DB_NO_ACTION);
    }
}


