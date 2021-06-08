#include "analog.h"
#include "pannelloSpessore.h"
#include "../application.h"
#include "../appinclude.h"
#include "../globvar.h"


pannelloSpessore::pannelloSpessore(QGraphicsView* view){
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

    if(isMaster) view->setGeometry(20,70,126,126);
    else view->setGeometry(655,70,126,126);


    spessorePix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/spessore.png"));
    spessorePix->setPos(0,0);
    font.setPointSize(40);
    font.setStretch(60);
    spessoreLabel = new GLabel(this,QRectF(11,40,104,56),font,QColor(_W_TEXT),"100",Qt::AlignCenter);

    font.setPointSize(20);
    font.setStretch(40);
    magFactor = new GLabel(this,QRectF(71,15,30,25),font,QColor(_W_TEXT),"",Qt::AlignCenter);

    timerDisable=0;


}

pannelloSpessore::~pannelloSpessore()
{
    return;
}

void pannelloSpessore::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId()==timerDisable)
    {
        killTimer(timerDisable);
        timerDisable=0;
    }

}

void pannelloSpessore::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mousePressEvent(event);

    if(timerDisable) return;
    timerDisable=startTimer(500);


}

void pannelloSpessore::databaseChanged(int index,int opt){
    if((isMaster)&&(opt&DBase::_DB_ONLY_SLAVE_ACTION)) return;
    if((!isMaster)&&(opt&DBase::_DB_ONLY_MASTER_ACTION)) return;

    switch(index){
    case _DB_SPESSORE:
    case _DB_MAG_FACTOR:
        updateSpessore();
        break;
    }
}

void pannelloSpessore::init(){

    updateSpessore();
    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(databaseChanged(int,int)),Qt::UniqueConnection);

}

void pannelloSpessore::exit(){
    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(databaseChanged(int,int)));

}


void pannelloSpessore::updateSpessore(){

    if(ApplicationDatabase.getDataI(_DB_SPESSORE) == 0){
        spessoreLabel->setPlainText(QString("---").toAscii().data());
        spessoreLabel->labelColor = QColor(_W_TEXT);
    }else{
        spessoreLabel->setPlainText(QString("%1").arg(ApplicationDatabase.getDataI(_DB_SPESSORE)).toAscii().data());
        spessoreLabel->labelColor = QColor(_ORANGE_CUFFIA);
    }
    spessoreLabel->update();

    if(ApplicationDatabase.getDataI(_DB_MAG_FACTOR) == 0) magFactor->setPlainText(QString("").toAscii().data());
    else if(ApplicationDatabase.getDataI(_DB_MAG_FACTOR) == 10) magFactor->setPlainText(QString("").toAscii().data());
    else if(ApplicationDatabase.getDataI(_DB_MAG_FACTOR) == 15) magFactor->setPlainText(QString("[1.5x]").toAscii().data());
    else if(ApplicationDatabase.getDataI(_DB_MAG_FACTOR) == 18) magFactor->setPlainText(QString("[1.8x]").toAscii().data());
    else if(ApplicationDatabase.getDataI(_DB_MAG_FACTOR) == 20) magFactor->setPlainText(QString("[2.0x]").toAscii().data());
    else magFactor->setPlainText(QString("").toAscii().data());
    magFactor->labelColor = QColor(_ORANGE_CUFFIA);
    magFactor->update();


}


