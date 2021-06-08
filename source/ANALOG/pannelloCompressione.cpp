#include "pannelloCompressione.h"
#include "../application.h"
#include "../appinclude.h"
#include "../globvar.h"


pannelloCompressione::pannelloCompressione(QGraphicsView* view){
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
    this->setSceneRect(0,0,473,473);

    if(isMaster) view->setGeometry(20,285,126,126);
    else view->setGeometry(655,285,126,126);


    forzaPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/forza.png"));
    forzaPix->setPos(0,0);
    font.setPointSize(40);
    font.setStretch(60);
    forzaLabel = new GLabel(this,QRectF(11,40,104,56),font,QColor(_W_TEXT),"150",Qt::AlignCenter);

    font.setPointSize(20);
    font.setStretch(40);
    targetLabel = new GLabel(this,QRectF(72,25,34,18),font,QColor(_W_TEXT),"150",Qt::AlignCenter);
    timerDisable=0;

}

pannelloCompressione::~pannelloCompressione()
{
    return;
}

void pannelloCompressione::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId()==timerDisable)
    {
        killTimer(timerDisable);
        timerDisable=0;
    }

}

void pannelloCompressione::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mousePressEvent(event);

    if(timerDisable) return;
    timerDisable=startTimer(500);


}

void pannelloCompressione::databaseChanged(int index,int opt){

    if((isMaster)&&(opt&DBase::_DB_ONLY_SLAVE_ACTION)) return;
    if((!isMaster)&&(opt&DBase::_DB_ONLY_MASTER_ACTION)) return;

    switch(index){
        case _DB_TARGET_FORCE:
            targetLabel->setPlainText(QString("[%1]").arg(ApplicationDatabase.getDataI(_DB_TARGET_FORCE)).toAscii().data());
            targetLabel->update();
        break;
        case _DB_FORZA:
            updateForza();
        break;
    }
}


void pannelloCompressione::init(){

    updateForza();
    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(databaseChanged(int,int)),Qt::UniqueConnection);
}

void pannelloCompressione::exit(){
    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(databaseChanged(int,int)));

}

void pannelloCompressione::updateForza(){

    if(ApplicationDatabase.getDataI(_DB_FORZA) == 0){
        forzaLabel->setPlainText(QString("---").toAscii().data());
        forzaLabel->labelColor = QColor(_W_TEXT);
    }else{
        forzaLabel->labelColor = QColor(_ORANGE_CUFFIA);
        forzaLabel->setPlainText(QString("%1").arg(ApplicationDatabase.getDataI(_DB_FORZA)).toAscii().data());
    }
    forzaLabel->update();

}


