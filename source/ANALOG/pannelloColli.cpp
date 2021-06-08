#include "pannelloColli.h"
#include "analog.h"
#include "../application.h"
#include "../appinclude.h"
#include "../globvar.h"
#include "pageOpenAnalogic.h"
extern AnalogPageOpen* paginaOpenStudyAnalogic;

pannelloColli::pannelloColli(QGraphicsView* view, QWidget* widget){
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

    backgroundPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloManualColliOFF.png"));
    backgroundPix->setPos(0,0);

    formatList.clear();
    formatList.append(""); // Auto Mode
    formatList.append("FORMAT 24x30");
    formatList.append("FORMAT 18x24");
    formatList.append("FORMAT BIOPSY");
    formatList.append("FORMAT MAGNIFIER");
    formatList.append("FORMAT CUSTOM");



    font.setPointSize(26);
    font.setStretch(50);
    formatLabel = new GLabel(this,QRectF(118,70,260,60),font,QColor(_W_TEXT),"",Qt::AlignCenter);

    font.setPointSize(18);
    font.setStretch(50);
    LLabel = new GLabel(this,QRectF(54,156,90,38),font,QColor(_BK_TEXT),"",Qt::AlignCenter);
    RLabel = new GLabel(this,QRectF(330,332,90,38),font,QColor(_BK_TEXT),"",Qt::AlignCenter);
    BLabel = new GLabel(this,QRectF(330,156,90,38),font,QColor(_BK_TEXT),"",Qt::AlignCenter);
    FLabel = new GLabel(this,QRectF(54,332,90,38),font,QColor(_BK_TEXT),"",Qt::AlignCenter);



    timerDisable=0;
    open_flag = false;
    parentView->hide();
}

pannelloColli::~pannelloColli()
{
    return;
}

void pannelloColli::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId()==timerDisable)
    {
        killTimer(timerDisable);
        timerDisable=0;
    }

}


void pannelloColli::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mousePressEvent(event);

    if(timerDisable) return;
    timerDisable=startTimer(500);
    QPointF mouse = event->scenePos();



    // scroll
    if(mouse.y()<=117){
        if(mouse.x()<=230){
            // scroll left
            onScrollLeft();

        }else{
            // scroll right
            onScrollRight();
        }

        return;
    }

    // Left + Back
    if(mouse.y()<=280){
        if(mouse.x()<=230){
            // LEFT
            onLeft();
        }else{
            // BACK
            onBack();
        }

        return;
    }

    // Front + Right
    if(mouse.y()<=360){
        if(mouse.x()<=230){
            // FRONT
            onFront();
        }else{
            // RIGHT
            onRight();
        }

        return;
    }

    // Tasto cancel
    if(mouse.y()>=390){
        disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
        ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_COLLIEXIT_SELECTION ,DBase::_DB_FORCE_SGN);
        return;
    }


}


void pannelloColli::open(){
    if(open_flag) return;
    open_flag = true;
    parentView->show();
    timerDisable=startTimer(500);

    // Associa il background corretto
    int modo = ApplicationDatabase.getDataI(_DB_ANALOG_MANUAL_COLLI_STAT);
    if( modo >= formatList.size()) modo = formatList.size()-1;

    if(modo==_AN_COLLI_AUTO){
        backgroundPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloColli_Auto.png"));
    }else if(modo==_AN_COLLI_MAN_CUSTOM){
        backgroundPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloColli_Custom.png"));
    }else backgroundPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloColli_ManualFormat.png"));

    // Imposta la selezione corrente
    formatLabel->setPlainText(formatList[modo].toAscii().data());
    pCalculator = new numericPad(0,parentView, parentWidget);
    connect(pCalculator,SIGNAL(calcSgn(bool)),this,SLOT(customCalcSlot(bool)));

    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
}

void pannelloColli::exit(){
    if(!open_flag) return;

    // Elimina il calcolatore
    disconnect(pCalculator);
    pCalculator->deleteLater(); // importante !!!
    pCalculator = 0;

    // Operazioni in chiusura
    open_flag = false;
    parentView->hide();

}


void pannelloColli::valueChanged(int index,int opt)
{
    if((isMaster)&&(opt&DBase::_DB_ONLY_SLAVE_ACTION)) return;
    if((!isMaster)&&(opt&DBase::_DB_ONLY_MASTER_ACTION)) return;
    int modo;

    switch(index){
    case _DB_ANALOG_MANUAL_COLLI_STAT:
        LLabel->hide();
        RLabel->hide();
        BLabel->hide();
        FLabel->hide();

        modo = ApplicationDatabase.getDataI(index);
        if(modo==_AN_COLLI_AUTO){
            backgroundPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloColli_Auto.png"));
        }else if(modo==_AN_COLLI_MAN_CUSTOM){
            backgroundPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloColli_Custom.png"));
            LLabel->setPlainText(QString("%1").arg(ApplicationDatabase.getDataI(_DB_ANALOG_CUSTOM_COLLI_LEFT)).toAscii().data());
            RLabel->setPlainText(QString("%1").arg(ApplicationDatabase.getDataI(_DB_ANALOG_CUSTOM_COLLI_RIGHT)).toAscii().data());
            BLabel->setPlainText(QString("%1").arg(ApplicationDatabase.getDataI(_DB_ANALOG_CUSTOM_COLLI_BACK)).toAscii().data());
            FLabel->setPlainText(QString("%1").arg(ApplicationDatabase.getDataI(_DB_ANALOG_CUSTOM_COLLI_FRONT)).toAscii().data());
            LLabel->show();
            RLabel->show();
            BLabel->show();
            FLabel->show();

        }else backgroundPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloColli_ManualFormat.png"));

        // Imposta la selezione corrente
        formatLabel->setPlainText(formatList[modo].toAscii().data());
        break;
    case _DB_ANALOG_CUSTOM_COLLI_LEFT:
        LLabel->setPlainText(QString("%1").arg(ApplicationDatabase.getDataI(index)).toAscii().data());
        break;
    case _DB_ANALOG_CUSTOM_COLLI_RIGHT:
        RLabel->setPlainText(QString("%1").arg(ApplicationDatabase.getDataI(index)).toAscii().data());
        break;
    case _DB_ANALOG_CUSTOM_COLLI_BACK:
        BLabel->setPlainText(QString("%1").arg(ApplicationDatabase.getDataI(index)).toAscii().data());
        break;
    case _DB_ANALOG_CUSTOM_COLLI_FRONT:
        FLabel->setPlainText(QString("%1").arg(ApplicationDatabase.getDataI(index)).toAscii().data());
        break;
    }
}

// Callback sulla pressione delle frecce.
void pannelloColli::onScrollLeft(void){
    int modo = ApplicationDatabase.getDataI(_DB_ANALOG_MANUAL_COLLI_STAT);
    if(modo==0) modo = _AN_COLLI_MAN_CUSTOM;
    else modo--;

    ApplicationDatabase.setData(_DB_ANALOG_MANUAL_COLLI_STAT, modo);
}

void pannelloColli::onScrollRight(void){
    int modo = ApplicationDatabase.getDataI(_DB_ANALOG_MANUAL_COLLI_STAT);
    if(modo==_AN_COLLI_MAN_CUSTOM) modo = 0;
    else modo++;

    ApplicationDatabase.setData(_DB_ANALOG_MANUAL_COLLI_STAT, modo);
}


// IMPOSTAZIONE DELLE LAME CUSTOM _________________________________________
void pannelloColli::onLeft(void){
    int modo = ApplicationDatabase.getDataI(_DB_ANALOG_MANUAL_COLLI_STAT);
    if(modo!=_AN_COLLI_MAN_CUSTOM) return;

    dataField=QString("%1").arg((int) ApplicationDatabase.getDataI(_DB_ANALOG_CUSTOM_COLLI_LEFT));
    pCalculator->activate(&dataField, QString("CUSTOM LEFT BLADE SETUP [0:230]"), 1);
}

void pannelloColli::onRight(void){
    int modo = ApplicationDatabase.getDataI(_DB_ANALOG_MANUAL_COLLI_STAT);
    if(modo!=_AN_COLLI_MAN_CUSTOM) return;

    dataField=QString("%1").arg((int) ApplicationDatabase.getDataI(_DB_ANALOG_CUSTOM_COLLI_RIGHT));
    pCalculator->activate(&dataField, QString("CUSTOM RIGHT BLADE SETUP [0:230]"), 2);
}

void pannelloColli::onBack(void){
    int modo = ApplicationDatabase.getDataI(_DB_ANALOG_MANUAL_COLLI_STAT);
    if(modo!=_AN_COLLI_MAN_CUSTOM) return;

    dataField=QString("%1").arg((int) ApplicationDatabase.getDataI(_DB_ANALOG_CUSTOM_COLLI_BACK));
    pCalculator->activate(&dataField, QString("CUSTOM BACK BLADE SETUP [0:200]"), 3);

}

void pannelloColli::onFront(void){
    int modo = ApplicationDatabase.getDataI(_DB_ANALOG_MANUAL_COLLI_STAT);
    if(modo!=_AN_COLLI_MAN_CUSTOM) return;

    dataField=QString("%1").arg((int) ApplicationDatabase.getDataI(_DB_ANALOG_CUSTOM_COLLI_FRONT));
    pCalculator->activate(&dataField, QString("CUSTOM FRONT BLADE SETUP [0:255]"), 4);

}

void pannelloColli::customCalcSlot(bool stat){
    if(!stat) return;

    int val = dataField.toInt();


    if(pCalculator->activation_code == 1) {
        if(val<0) val=0;
        else if(val>230) val = 230;

        // LEFT BLADE
        ApplicationDatabase.setData(_DB_ANALOG_CUSTOM_COLLI_LEFT, (int) val);
    }else if(pCalculator->activation_code == 2) {
        if(val<0) val=0;
        else if(val>230) val = 230;
        // RIGHT BLADE
        ApplicationDatabase.setData(_DB_ANALOG_CUSTOM_COLLI_RIGHT, (int) val);
    }else if(pCalculator->activation_code == 3) {
        if(val<0) val=0;
        else if(val>200) val = 200;
        // BACK BLADE
        ApplicationDatabase.setData(_DB_ANALOG_CUSTOM_COLLI_BACK, (int) val);
    }else if(pCalculator->activation_code == 4) {
        if(val<0) val=0;
        else if(val>255) val = 255;
        // FRONT BLADE
        ApplicationDatabase.setData(_DB_ANALOG_CUSTOM_COLLI_FRONT, (int) val);
    }
}
