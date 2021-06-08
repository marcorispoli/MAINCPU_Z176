#include "pannelloKv.h"
#include "analog.h"
#include "../application.h"
#include "../appinclude.h"
#include "../globvar.h"


pannelloKv::pannelloKv(QGraphicsView* view){
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
    this->setSceneRect(0,0,473,473);

    backgroundPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannellokV.png"));
    backgroundPix->setPos(0,0);

    font.setPointSize(80);
    font.setStretch(60);
    kvSelezionati = new GLabel(this,QRectF(0,18,473,74),font,QColor(_W_TEXT),"---",Qt::AlignCenter);


    timerDisable=0;
    open_flag = false;
    parent->hide();
}

pannelloKv::~pannelloKv()
{
    return;
}

void pannelloKv::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId()==timerDisable)
    {
        killTimer(timerDisable);
        timerDisable=0;
    }

}

#define X1PAD 83
#define X2PAD 390
#define Y1PAD 116
#define Y2PAD 356
#define DX ((X2PAD-X1PAD)/4)
#define DY ((Y2PAD-Y1PAD)/4)


void pannelloKv::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mousePressEvent(event);

    if(timerDisable) return;
    timerDisable=startTimer(500);


    QPointF mouse = event->scenePos();

    // Selezione pulsante
    if((mouse.x()>=X1PAD)&&(mouse.x()<=X2PAD)&&(mouse.y()>=Y1PAD)&&(mouse.y()<=Y2PAD)){
        if(ApplicationDatabase.getDataI(_DB_TECH_MODE) != ANALOG_TECH_MODE_MANUAL) return;

        // Tastierino numerico
        int  i = ((mouse.x()-X1PAD)/DX) ;
        int  j = ((mouse.y()-Y1PAD)/DY) ;

        selected_dkV = (20 + i + j * 4) * 10;
        ApplicationDatabase.setData(_DB_DKV,(int) (selected_dkV));
        return;
    }

    // Tasto +
    if((mouse.x()<=70)&&(mouse.y()>=116)&&(mouse.y()<=353)){


        if(selected_dkV >= max_dKv) return;

        selected_dkV+=5;
        if(selected_dkV > max_dKv) selected_dkV = max_dKv;
        ApplicationDatabase.setData(_DB_DKV,(int) (selected_dkV));
        return;
    }


    // Tasto -
    if((mouse.x()>=400)&&(mouse.y()>=116)&&(mouse.y()<=353)){

        if(selected_dkV <= min_dKv) return;

        selected_dkV-=5;
        if(selected_dkV < min_dKv) selected_dkV = min_dKv;
        ApplicationDatabase.setData(_DB_DKV,(int) (selected_dkV));

        return;
    }

    // Tasto cancel
    if((mouse.x()>=245)&&(mouse.y()>=378)&&(mouse.x()<=353)){
        disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
        ApplicationDatabase.setData(_DB_DKV,(int) entry_dkV);
        ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_KVEXIT_SELECTION ,DBase::_DB_FORCE_SGN);
        return;
    }

    // Tasto OK
    if((mouse.x()>=54)&&(mouse.y()>=378)&&(mouse.x()<230)){
        disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
        ApplicationDatabase.setData(_DB_DKV,(int) selected_dkV);
        ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_KVEXIT_SELECTION ,DBase::_DB_FORCE_SGN);
        return;
    }

}


void pannelloKv::open(){
    if(open_flag) return;
    open_flag = true;
    parent->show();

    if(ApplicationDatabase.getDataI(_DB_TECH_MODE) == ANALOG_TECH_MODE_MANUAL)
        backgroundPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannellokV.png"));
    else
        backgroundPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannellokVauto.png"));

    entry_dkV = ApplicationDatabase.getDataI(_DB_DKV);
    selected_dkV = entry_dkV;
    max_dKv = (ApplicationDatabase.getDataI(_DB_MIN_MAX_DKV)>>16);
    min_dKv = (ApplicationDatabase.getDataI(_DB_MIN_MAX_DKV) &0xFFFF);



    kvSelezionati->setPlainText(QString("%1").arg((float) selected_dkV/10).toAscii().data());
    kvSelezionati->update();

    timerDisable=startTimer(500);
    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
}

void pannelloKv::exit(){
    if(!open_flag) return;

    // Operazioni in chiusura
    open_flag = false;
    parent->hide();

}


void pannelloKv::valueChanged(int index,int opt)
{
    if((isMaster)&&(opt&DBase::_DB_ONLY_SLAVE_ACTION)) return;
    if((!isMaster)&&(opt&DBase::_DB_ONLY_MASTER_ACTION)) return;

    switch(index){
    case _DB_DKV:
        kvSelezionati->setPlainText(QString("%1").arg((float) (ApplicationDatabase.getDataI(index))/10).toAscii().data());
        kvSelezionati->update();
        break;
    }
}
