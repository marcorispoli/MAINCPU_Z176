#include "pannelloMas.h"
#include "analog.h"
#include "../application.h"
#include "../appinclude.h"
#include "../globvar.h"

static unsigned short mAsTab[4][16]={\
        {10,11,12,14,16,18,20,22,25,28,32,36,40,45,50,56},\
        {63,71,80,90,100,110,125,140,160,180,200,220,250,280,320,360},\
        {400,450,500,550,560,600,630,650,700,710,750,800,850,900,1000,1100},\
        {1250,1400,1600,1800,2000,2200,2500,2800,3200,3600,4000,4500,5000,5600,6300,6400}\
        };


pannelloMas::pannelloMas(QGraphicsView* view){
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

    backgroundPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloMas1.png"));
    backgroundPix->setPos(0,0);

    for(int i=0; i<16; i++){
        disabledPix[i] = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/disable_mas.png"));
        disabledPix[i]->setPos(85 + (i%4)*79,119+(i/4)*61);
        disabledPix[i]->hide();
    }


    font.setPointSize(80);
    font.setStretch(60);
    masSelezionati = new GLabel(this,QRectF(0,18,473,74),font,QColor(_W_TEXT),"---",Qt::AlignCenter);


    timerDisable=0;
    open_flag = false;
    parent->hide();
}

pannelloMas::~pannelloMas()
{
    return;
}

void pannelloMas::timerEvent(QTimerEvent* ev)
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


void pannelloMas::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mousePressEvent(event);

    if(timerDisable) return;
    timerDisable=startTimer(500);


    QPointF mouse = event->scenePos();

    // Selezione pulsante
    if((mouse.x()>=X1PAD)&&(mouse.x()<=X2PAD)&&(mouse.y()>=Y1PAD)&&(mouse.y()<=Y2PAD)){
        unsigned short* pTab = mAsTab[ApplicationDatabase.getDataI(_DB_CURRENT_MAS_TAB)%4];

        // Tastierino numerico
        int  i = ((mouse.x()-X1PAD)/DX) ;
        int  j = ((mouse.y()-Y1PAD)/DY) ;
        if((i>3)||(j>3)) return;
        if(!masEnabled[j*4+i]) return ;
        ApplicationDatabase.setData(_DB_DMAS,(int) ( pTab[j*4+i]));
        return;
    }

    // Tasto -
    if((mouse.x()<=70)&&(mouse.y()>=116)&&(mouse.y()<=353)){


        // Cambia pannello
        int currentTab = ApplicationDatabase.getDataI(_DB_CURRENT_MAS_TAB)-1;
        if(currentTab<0) currentTab=0;
        ApplicationDatabase.setData(_DB_CURRENT_MAS_TAB,(int) currentTab);
        return;
    }


    // Tasto +
    if((mouse.x()>=400)&&(mouse.y()>=116)&&(mouse.y()<=353)){

        // Cambia pannello
        int currentTab = ApplicationDatabase.getDataI(_DB_CURRENT_MAS_TAB)+1;
        if(currentTab>3) currentTab=3;
        ApplicationDatabase.setData(_DB_CURRENT_MAS_TAB,(int) currentTab);

        return;
    }


    // Tasto cancel
    if((mouse.x()>=245)&&(mouse.y()>=378)&&(mouse.x()<=353)){
        disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
        ApplicationDatabase.setData(_DB_DMAS,(int) entryMas*10);
        ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_MASEXIT_SELECTION ,DBase::_DB_FORCE_SGN);

        return;
    }

    // Tasto OK
    if((mouse.x()>=54)&&(mouse.y()>=378)&&(mouse.x()<230)){
        disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
        ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_MASEXIT_SELECTION ,DBase::_DB_FORCE_SGN);
        return;
    }

}


void pannelloMas::open(){
    if(open_flag) return;
    open_flag = true;
    parent->show();

    unsigned char tab,tab1,index;
    getNearestR20DMas(ApplicationDatabase.getDataI(_DB_DMAS), &tab, &index);
    entryMas = (float) mAsTab[tab][index]/10;
    ApplicationDatabase.setData(_DB_CURRENT_MAS_TAB,(int) tab,DBase::_DB_NO_CHG_SGN||DBase::_DB_NO_ECHO);

    maxdMas = (ApplicationDatabase.getDataI(_DB_MIN_MAX_DMAS)>>16);
    mindMas = (ApplicationDatabase.getDataI(_DB_MIN_MAX_DMAS) &0xFFFF);

    masSelezionati->setPlainText(QString("%1").arg((float) entryMas).toAscii().data());
    masSelezionati->update();

    timerDisable=startTimer(500);
    setCurrentTab(tab,mindMas,maxdMas);  // Impostazione della tabella corrente

    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
}

void pannelloMas::exit(){
    if(!open_flag) return;

    // Operazioni in chiusura
    open_flag = false;
    parent->hide();

}


void pannelloMas::valueChanged(int index,int opt)
{
    if((isMaster)&&(opt&DBase::_DB_ONLY_SLAVE_ACTION)) return;
    if((!isMaster)&&(opt&DBase::_DB_ONLY_MASTER_ACTION)) return;

    switch(index){
    case _DB_CURRENT_MAS_TAB:
        setCurrentTab(ApplicationDatabase.getDataI(index),mindMas,maxdMas);
        break;
    case _DB_DMAS:
        masSelezionati->setPlainText(QString("%1").arg((float) ApplicationDatabase.getDataI(index)/10).toAscii().data());
        masSelezionati->update();
        break;
    }
}

/* ______________________________________________________________________________

  Restituisce il più prossimo valore di mAs nella gamma R20
  return -1 se non c'è nessuna corrispondenza valida
_________________________________________________________________________________ */
void pannelloMas::getNearestR20DMas(unsigned short dmas, unsigned char* tab, unsigned char* index){
    unsigned short* pTab;


    if(dmas>=1250){
        *tab=3;
    }else if(dmas>=400){
        *tab=2;
    }else if(dmas>=63){
        *tab=1;
    }else{
        *tab=0;
    }

    pTab = mAsTab[*tab];
    *index=1;
    for(; (*index)<16; (*index)++){

        if(pTab[*index]>dmas) break;
    }
    (*index)--;
    return;

}
/* ______________________________________________________________________________

  Restituisce il più prossimo valore di mAs nella gamma R20
  return -1 se non c'è nessuna corrispondenza valida
_________________________________________________________________________________ */
void pannelloMas::setCurrentTab(unsigned char tab, unsigned short dmin, unsigned short dmax){

    if(tab==0) backgroundPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloMas0.png"));
    else if(tab==1) backgroundPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloMas1.png"));
    else if(tab==2) backgroundPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloMas2.png"));
    else if(tab==3) backgroundPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloMas3.png"));

    // Attivazio dei disable
    for(int i=0; i<16; i++){
        if((mAsTab[tab][i]<dmin)||(mAsTab[tab][i]>dmax)){
            disabledPix[i]->show();
            masEnabled[i] = false;
        }else{
            disabledPix[i]->hide();
            masEnabled[i] = true;
        }
    }


    return;

}



