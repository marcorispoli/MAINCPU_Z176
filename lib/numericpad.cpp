#include "numericpad.h"
#include "ui_numericpad.h"
#include  <QDebug>


numericPad::numericPad(int rotview,  QGraphicsView *parentView, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::numericPad)
{
    ui->setupUi(this);
    scene = new QGraphicsScene();
    view = new QGraphicsView(scene);
    proxy = scene->addWidget(this);

    view->setWindowFlags(Qt::FramelessWindowHint);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setFixedSize(500,446);    // Dimensione della vista
    scene->setSceneRect(0,0,500,446);
    view->rotate(rotview);       // Angolo di rotazione della vista corrente
    view->setAlignment(Qt::AlignRight);
    view->setScene(scene);
    view->move(150,20);

    this->parent = parent;
    this->parentView = parentView;

    connect(ui->push0,SIGNAL(released()),this,SLOT(on0Pressed()),Qt::UniqueConnection);
    connect(ui->push1,SIGNAL(released()),this,SLOT(on1Pressed()),Qt::UniqueConnection);
    connect(ui->push2,SIGNAL(released()),this,SLOT(on2Pressed()),Qt::UniqueConnection);
    connect(ui->push3,SIGNAL(released()),this,SLOT(on3Pressed()),Qt::UniqueConnection);
    connect(ui->push4,SIGNAL(released()),this,SLOT(on4Pressed()),Qt::UniqueConnection);
    connect(ui->push5,SIGNAL(released()),this,SLOT(on5Pressed()),Qt::UniqueConnection);
    connect(ui->push6,SIGNAL(released()),this,SLOT(on6Pressed()),Qt::UniqueConnection);
    connect(ui->push7,SIGNAL(released()),this,SLOT(on7Pressed()),Qt::UniqueConnection);
    connect(ui->push8,SIGNAL(released()),this,SLOT(on8Pressed()),Qt::UniqueConnection);
    connect(ui->push9,SIGNAL(released()),this,SLOT(on9Pressed()),Qt::UniqueConnection);
    connect(ui->pushA,SIGNAL(released()),this,SLOT(onAPressed()),Qt::UniqueConnection);
    connect(ui->pushB,SIGNAL(released()),this,SLOT(onBPressed()),Qt::UniqueConnection);
    connect(ui->pushC,SIGNAL(released()),this,SLOT(onCPressed()),Qt::UniqueConnection);
    connect(ui->pushD,SIGNAL(released()),this,SLOT(onDPressed()),Qt::UniqueConnection);
    connect(ui->pushE,SIGNAL(released()),this,SLOT(onEPressed()),Qt::UniqueConnection);
    connect(ui->pushF,SIGNAL(released()),this,SLOT(onFPressed()),Qt::UniqueConnection);
    connect(ui->pushCls,SIGNAL(released()),this,SLOT(onClsPressed()),Qt::UniqueConnection);
    connect(ui->pushOX,SIGNAL(released()),this,SLOT(onOXPressed()),Qt::UniqueConnection);


    connect(ui->pushDot,SIGNAL(released()),this,SLOT(onDotPressed()),Qt::UniqueConnection);
    connect(ui->pushSgn,SIGNAL(released()),this,SLOT(onSgnPressed()),Qt::UniqueConnection);

    connect(ui->pushBack,SIGNAL(released()),this,SLOT(onBackPressed()),Qt::UniqueConnection);
    connect(ui->pushOk,SIGNAL(released()),this,SLOT(onOKPressed()),Qt::UniqueConnection);
    connect(ui->pushCanc,SIGNAL(released()),this,SLOT(onCancelPressed()),Qt::UniqueConnection);


    disableTimer = 0;
    disableButton = false;


}

numericPad::~numericPad()
{
    delete ui;
}


void  numericPad::hide(){
    view->hide();
    parentView->show();
}

void  numericPad::on0Pressed()
{
   if(disableButton) return;
   disableButton = true;
   disableTimer = startTimer(300);

   valore.append("0");
   criptoText.append("*");
   if(criptato) ui->risultatoText->setText(criptoText);
   else ui->risultatoText->setText(valore);
}

void  numericPad::on1Pressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("1");
    criptoText.append("*");
    if(criptato) ui->risultatoText->setText(criptoText);
    else ui->risultatoText->setText(valore);
}
void  numericPad::on2Pressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("2");
    criptoText.append("*");
    if(criptato) ui->risultatoText->setText(criptoText);
    else ui->risultatoText->setText(valore);
}
void  numericPad::on3Pressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("3");
    criptoText.append("*");
    if(criptato) ui->risultatoText->setText(criptoText);
    else ui->risultatoText->setText(valore);
}
void  numericPad::on4Pressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("4");
    criptoText.append("*");
    if(criptato) ui->risultatoText->setText(criptoText);
    else ui->risultatoText->setText(valore);

}
void  numericPad::on5Pressed(){
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("5");
    criptoText.append("*");
    if(criptato) ui->risultatoText->setText(criptoText);
    else ui->risultatoText->setText(valore);

 }
void  numericPad::on6Pressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("6");
    criptoText.append("*");
    if(criptato) ui->risultatoText->setText(criptoText);
    else ui->risultatoText->setText(valore);

}
void  numericPad::on7Pressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("7");
    criptoText.append("*");
    if(criptato) ui->risultatoText->setText(criptoText);
    else ui->risultatoText->setText(valore);

}
void  numericPad::on8Pressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("8");
    criptoText.append("*");
    if(criptato) ui->risultatoText->setText(criptoText);
    else ui->risultatoText->setText(valore);

}
void  numericPad::on9Pressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("9");
    criptoText.append("*");
    if(criptato) ui->risultatoText->setText(criptoText);
    else ui->risultatoText->setText(valore);

}
void  numericPad::onClsPressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.clear();
    criptoText.clear();
    ui->risultatoText->setText(valore);
    hexMode=false;

}
void  numericPad::onAPressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("A");
    criptoText.append("*");
    if(criptato) ui->risultatoText->setText(criptoText);
    else ui->risultatoText->setText(valore);

}
void  numericPad::onBPressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("B");
    criptoText.append("*");
    if(criptato) ui->risultatoText->setText(criptoText);
    else ui->risultatoText->setText(valore);


}
void  numericPad::onCPressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("C");
    criptoText.append("*");
    if(criptato) ui->risultatoText->setText(criptoText);
    else ui->risultatoText->setText(valore);

}
void  numericPad::onDPressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("D");
    criptoText.append("*");
    if(criptato) ui->risultatoText->setText(criptoText);
    else ui->risultatoText->setText(valore);

}
void  numericPad::onEPressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("E");
    criptoText.append("*");
    if(criptato) ui->risultatoText->setText(criptoText);
    else ui->risultatoText->setText(valore);

}
void  numericPad::onFPressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("F");
    criptoText.append("*");
    if(criptato) ui->risultatoText->setText(criptoText);
    else ui->risultatoText->setText(valore);

}
void  numericPad::onOXPressed()
{
    if(hexMode) return;
    hexMode=true;
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.prepend("0x");
    criptoText.append("*");
    if(criptato) ui->risultatoText->setText(criptoText);
    else ui->risultatoText->setText(valore);

}


void  numericPad::onDotPressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append(".");
    criptoText.append("*");
    if(criptato) ui->risultatoText->setText(criptoText);
    else ui->risultatoText->setText(valore);
}

void  numericPad::onSgnPressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    if(valore.size()==0) return;
    if(valore.at(0)=='-') valore = valore.right(valore.size()-1);
    else valore.prepend("-");
    criptoText.append("*");
    if(criptato) ui->risultatoText->setText(criptoText);
    else ui->risultatoText->setText(valore);
}

void  numericPad::onBackPressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore = valore.left(valore.size()-1);
    criptoText = criptoText.left(criptoText.size()-1);
    if(criptato) ui->risultatoText->setText(criptoText);
    else ui->risultatoText->setText(valore);
}

void  numericPad::onOKPressed()
{
    if(disableButton) return;
    hide();

    if(risultato!=NULL) *risultato= valore;
    else if(rislabel) rislabel->setText(valore);
    else rislineedit->setText(valore);

    validazione = TRUE;

    emit calcSgn(TRUE);

}

void  numericPad::onCancelPressed()
{
    if(disableButton) return;
    hide();

    validazione = FALSE;
    emit calcSgn(FALSE);

}

void  numericPad::activate(QString* field, QString name)
{
    criptato=false;
    valore = *field;
    criptoText.clear();
    for(int i=0; i<valore.size(); i++) criptoText.append("*");
    ui->risultatoText->setText(*field);
    ui->calcTitle->setText(name);
    risultato=field;
    rislabel=NULL;
    rislineedit = NULL;
    disableButton = true;
    disableTimer = startTimer(300);
    view->show();
    hexMode=false;

}

void  numericPad::activate(QString* field, QString name, int code)
{
    criptato=false;
    valore = *field;
    criptoText.clear();
    for(int i=0; i<valore.size(); i++) criptoText.append("*");
    ui->risultatoText->setText(*field);
    ui->calcTitle->setText(name);
    risultato=field;
    rislabel=NULL;
    rislineedit = NULL;
    disableButton = true;
    disableTimer = startTimer(300);
    activation_code = code;
    view->show();
    hexMode=false;
}
void  numericPad::activate(QLabel* field, QString name)
{
    criptato=false;
    valore = field->text();
    criptoText.clear();
    for(int i=0; i<valore.size(); i++) criptoText.append("*");
    ui->risultatoText->setText(field->text());
    ui->calcTitle->setText(name);
    rislabel = field;
    risultato=NULL;
    rislineedit = NULL;
    disableButton = true;
    disableTimer = startTimer(300);
    view->show();
    hexMode=false;
}
void  numericPad::activate(QLineEdit* field, QString name)
{
    criptato=false;
    valore = field->text();
    criptoText.clear();
    for(int i=0; i<valore.size(); i++) criptoText.append("*");
    ui->risultatoText->setText(field->text());
    ui->calcTitle->setText(name);
    rislineedit = field;
    risultato=NULL;
    rislabel=NULL;
    disableButton = true;
    disableTimer = startTimer(300);
    view->show();

}
void  numericPad::setCripto(void){
    criptato=true;
    ui->risultatoText->setText(criptoText);
}


void numericPad::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId() == disableTimer){
        killTimer(disableTimer);
        disableTimer = 0;
        disableButton = false;
    }

}
