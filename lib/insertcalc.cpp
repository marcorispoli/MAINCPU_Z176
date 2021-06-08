#include "insertcalc.h"
#include "stdio.h"

// Definizione della classe che verr√  utilizzata per definire la grafica.
// Questa classe √® copiata direttamente dal codice auto generato dal QTDesigner
// e adattato allo scopo.
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QHeaderView>
#include <QtGui/QLCDNumber>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

#define FORM_STYLE   "color: rgb(255, 255, 255);\n"\
                     "background-image:url(/transparent.png);\n"\
                     "background-color: qlineargradient(spread:pad, x1:0.75, y1:0.824, x2:0.037, y2:0.05, stop:0 rgba(30,30,30, 200), stop:1 rgba(200, 200, 200, 120));\n"\
                     "border: 1px solid  rgb(74, 74, 74);\n"\
                     "border-radius:10px;\n"\
                     "border-right-color: rgba(255, 255, 153,0);\n"\
                     "border-left-color: rgba(255, 255, 153,0);\n"\
                     "border-top-color: rgba(255, 255, 153,0);\n"

#define DISPLAY_STYLE   "color: rgb(255, 255, 255);\n"\
                        "background-image:url(/transparent.png);\n"\
                        "background-color: qrgba(0, 0, 0, 100);\n"\
                        "border: 2px solid  rgb(74, 74, 74);\n"\
                        "border-radius:1px;\n"\
                        "border-right-color: rgba(255, 255, 153,0);\n"\
                        "border-left-color: rgba(255, 255, 153,0);\n"\
                        "border-top-color: rgba(255, 255, 153,0);\n"

#define BUTTON_STYLE    "color: rgb(255, 255, 255);\n"\
                        "background-image:url(/transparent.png);\n"\
                        "background-color: rgba(139, 139, 139, 50);\n"\
                        "border: 2px solid  rgb(0, 0, 0);\n"\
                        "border-radius:4px;\n"\
                        "border-right-color: rgba(0, 0, 0,40);\n"\
                        "border-left-color: rgba(255, 255, 255,100);\n"\
                        "border-top-color: rgba(255, 255, 153,0);\n"

#define TITLE_STYLE     "color: rgb(255, 255, 255);\n"\
                        "background-image:url(/transparent.png);\n"\
                        "background-color: rgba(255, 251, 123,40);\n"\
                        "border: 2px solid  rgb(74, 74, 74);\n"\
                        "border-radius:4px;\n"\
                        "border-right-color: rgba(255, 255, 153,0);\n"\
                        "border-left-color: rgba(255, 255, 153,0);\n"\
                        "border-top-color: rgba(255, 255, 153,0);\n"


class uiInsertCalc
{
    public:
        QFrame *calcForm;
        QLCDNumber *risultatoText;
        QPushButton *push0;
        QPushButton *push1;
        QPushButton *push2;
        QPushButton *push3;
        QPushButton *push6;
        QPushButton *push4;
        QPushButton *push5;
        QPushButton *push9;
        QPushButton *push7;
        QPushButton *push8;
        QPushButton *pushBack;
        QPushButton *pushOk;
        QPushButton *pushCanc;
        QPushButton *pushDot;
        QPushButton *pushSgn;

        QLabel *calcTitle;

        uiInsertCalc(QWidget* parent){
            calcForm = new QFrame(parent);
            calcForm->setObjectName(QString::fromUtf8("calcForm"));
            calcForm->setGeometry(QRect(85, 30, 306, 446));
            calcForm->setStyleSheet(QString::fromUtf8(FORM_STYLE));
            calcForm->setFrameShape(QFrame::StyledPanel);
            calcForm->setFrameShadow(QFrame::Raised);
            risultatoText = new QLCDNumber(calcForm);
            risultatoText->setObjectName(QString::fromUtf8("risultatoText"));
            risultatoText->setGeometry(QRect(15, 50, 171, 56));
            risultatoText->setStyleSheet(QString::fromUtf8(DISPLAY_STYLE));

            QFont font;
            font.setFamily(QString::fromUtf8("Bitstream Charter"));
            font.setPointSize(22);
            font.setBold(true);
            font.setItalic(false);
            font.setWeight(75);
            push0 = new QPushButton(calcForm);
            push0->setObjectName(QString::fromUtf8("push0"));
            push0->setGeometry(QRect(110, 310, 85, 51));
            push0->setFont(font);
            push0->setStyleSheet(QString::fromUtf8(BUTTON_STYLE));

            push1= new QPushButton(calcForm);
            push1->setObjectName(QString::fromUtf8("push1"));
            push1->setGeometry(QRect(15, 120, 85, 51));
            push1->setFont(font);
            push1->setStyleSheet(QString::fromUtf8(BUTTON_STYLE));

            push2 = new QPushButton(calcForm);
            push2->setObjectName(QString::fromUtf8("push2"));
            push2->setGeometry(QRect(110, 120, 85, 51));
            push2->setFont(font);
            push2->setStyleSheet(QString::fromUtf8(BUTTON_STYLE));
            push3 = new QPushButton(calcForm);
            push3->setObjectName(QString::fromUtf8("push3"));
            push3->setGeometry(QRect(205, 120, 85, 51));
            push3->setFont(font);
            push3->setStyleSheet(QString::fromUtf8(BUTTON_STYLE));
            push6 = new QPushButton(calcForm);
            push6->setObjectName(QString::fromUtf8("push6"));
            push6->setGeometry(QRect(205, 180, 85, 51));
            push6->setFont(font);
            push6->setStyleSheet(QString::fromUtf8(BUTTON_STYLE));
            push4 = new QPushButton(calcForm);
            push4->setObjectName(QString::fromUtf8("push4"));
            push4->setGeometry(QRect(15, 180, 85, 51));
            push4->setFont(font);
            push4->setStyleSheet(QString::fromUtf8(BUTTON_STYLE));
            push5 = new QPushButton(calcForm);
            push5->setObjectName(QString::fromUtf8("push5"));
            push5->setGeometry(QRect(110, 180, 85, 51));
            push5->setFont(font);
            push5->setStyleSheet(QString::fromUtf8(BUTTON_STYLE));
            push9 = new QPushButton(calcForm);
            push9->setObjectName(QString::fromUtf8("push9"));
            push9->setGeometry(QRect(205, 245, 85, 51));
            push9->setFont(font);
            push9->setStyleSheet(QString::fromUtf8(BUTTON_STYLE));
            push7 = new QPushButton(calcForm);
            push7->setObjectName(QString::fromUtf8("push7"));
            push7->setGeometry(QRect(15, 245, 85, 51));
            push7->setFont(font);
            push7->setStyleSheet(QString::fromUtf8(BUTTON_STYLE));
            push8 = new QPushButton(calcForm);
            push8->setObjectName(QString::fromUtf8("push8"));
            push8->setGeometry(QRect(110, 245, 85, 51));
            push8->setFont(font);
            push8->setStyleSheet(QString::fromUtf8(BUTTON_STYLE));
            pushDot = new QPushButton(calcForm);
            pushDot->setObjectName(QString::fromUtf8("pushDot"));
            pushDot->setGeometry(QRect(15, 310, 85, 51));
            pushDot->setFont(font);
            pushDot->setStyleSheet(QString::fromUtf8(BUTTON_STYLE));
            pushSgn = new QPushButton(calcForm);
            pushSgn->setObjectName(QString::fromUtf8("pushSgn"));
            pushSgn->setGeometry(QRect(205, 310, 85, 51));
            pushSgn->setFont(font);
            pushSgn->setStyleSheet(QString::fromUtf8(BUTTON_STYLE));


            pushBack = new QPushButton(calcForm);
            pushBack->setObjectName(QString::fromUtf8("pushBack"));
            pushBack->setGeometry(QRect(205, 55, 85, 51));
            pushBack->setFont(font);
            pushBack->setStyleSheet(QString::fromUtf8(BUTTON_STYLE));
            pushOk = new QPushButton(calcForm);
            pushOk->setObjectName(QString::fromUtf8("pushOk"));
            pushOk->setGeometry(QRect(15, 375, 126, 51));
            pushOk->setFont(font);
            pushOk->setStyleSheet(QString::fromUtf8(BUTTON_STYLE));
            pushCanc = new QPushButton(calcForm);
            pushCanc->setObjectName(QString::fromUtf8("pushCanc"));
            pushCanc->setGeometry(QRect(165, 375, 126, 51));
            pushCanc->setFont(font);
            pushCanc->setStyleSheet(QString::fromUtf8(BUTTON_STYLE));

            push0->setText("0");
            push1->setText("1");
            push2->setText("2");
            push3->setText("3");
            push4->setText("4");
            push5->setText("5");
            push6->setText("6");
            push7->setText("7");
            push8->setText("8");
            push9->setText("9");
            pushDot->setText(".");
            pushSgn->setText("+/-");
            pushOk->setText("OK");
            pushCanc->setText("CANC");
            pushBack->setText("<<");

            // Titolo del frame
            calcTitle = new QLabel(calcForm);
            calcTitle->setObjectName(QString::fromUtf8("calcTitle"));
            calcTitle->setGeometry(QRect(16, 5, 271, 36));
            QFont font1;
            font1.setFamily(QString::fromUtf8("Bitstream Charter"));
            font1.setPointSize(16);
            font1.setBold(true);
            font1.setWeight(75);
            calcTitle->setFont(font1);
            calcTitle->setStyleSheet(QString::fromUtf8(TITLE_STYLE));
            calcTitle->setAlignment(Qt::AlignCenter);

        }

        ~uiInsertCalc()
        {
            delete calcForm;
        }
};


uiInsertCalc* ui;


InsertCalc::InsertCalc(QWidget *parent) :
    QWidget(parent)

{

    ui = new uiInsertCalc(parent);


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
    connect(ui->pushDot,SIGNAL(released()),this,SLOT(onDotPressed()),Qt::UniqueConnection);
    connect(ui->pushSgn,SIGNAL(released()),this,SLOT(onSgnPressed()),Qt::UniqueConnection);

    connect(ui->pushBack,SIGNAL(released()),this,SLOT(onBackPressed()),Qt::UniqueConnection);
    connect(ui->pushOk,SIGNAL(released()),this,SLOT(onOKPressed()),Qt::UniqueConnection);
    connect(ui->pushCanc,SIGNAL(released()),this,SLOT(onCancelPressed()),Qt::UniqueConnection);


    disableTimer = 0;
    disableButton = false;

}

InsertCalc::~InsertCalc()
{
    delete ui;

}

void  InsertCalc::hide(){
    ui->calcForm->hide();
}

void  InsertCalc::on0Pressed()
{
   if(disableButton) return;
   disableButton = true;
   disableTimer = startTimer(300);

   valore.append("0");
   ui->risultatoText->display(valore);
}

void  InsertCalc::on1Pressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("1");
    ui->risultatoText->display(valore);
}
void  InsertCalc::on2Pressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("2");
    ui->risultatoText->display(valore);
}
void  InsertCalc::on3Pressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("3");
    ui->risultatoText->display(valore);
}
void  InsertCalc::on4Pressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("4");
    ui->risultatoText->display(valore);

}
void  InsertCalc::on5Pressed(){
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("5");
    ui->risultatoText->display(valore);

 }
void  InsertCalc::on6Pressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("6");
    ui->risultatoText->display(valore);

}
void  InsertCalc::on7Pressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("7");
    ui->risultatoText->display(valore);

}
void  InsertCalc::on8Pressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("8");
    ui->risultatoText->display(valore);

}
void  InsertCalc::on9Pressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append("9");
    ui->risultatoText->display(valore);

}
void  InsertCalc::onDotPressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore.append(".");
    ui->risultatoText->display(valore);
}

void  InsertCalc::onSgnPressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    if(valore.size()==0) return;
    if(valore.at(0)=='-') valore = valore.right(valore.size()-1);
    else valore.prepend("-");
    ui->risultatoText->display(valore);
}

void  InsertCalc::onBackPressed()
{
    if(disableButton) return;
    disableButton = true;
    disableTimer = startTimer(300);
    valore = valore.left(valore.size()-1);
    ui->risultatoText->display(valore);
}

void  InsertCalc::onOKPressed()
{
    if(disableButton) return;
    ui->calcForm->hide();

    if(risultato!=NULL) *risultato= valore;
    else if(rislabel) rislabel->setText(valore);
    else rislineedit->setText(valore);

    validazione = TRUE;

    emit calcSgn(TRUE);

}

void  InsertCalc::onCancelPressed()
{
    if(disableButton) return;
    ui->calcForm->hide();

    validazione = FALSE;
    emit calcSgn(FALSE);

}

void  InsertCalc::activate(QString* field, QString name)
{
    valore = *field;
    ui->risultatoText->display(*field);
    ui->calcTitle->setText(name);
    risultato=field;
    rislabel=NULL;
    rislineedit = NULL;
    disableButton = true;
    disableTimer = startTimer(300);
    ui->calcForm->show();
}

void  InsertCalc::activate(QString* field, QString name, int code)
{
    valore = *field;
    ui->risultatoText->display(*field);
    ui->calcTitle->setText(name);
    risultato=field;
    rislabel=NULL;
    rislineedit = NULL;
    disableButton = true;
    disableTimer = startTimer(300);
    activation_code = code;
    ui->calcForm->show();
}
void  InsertCalc::activate(QLabel* field, QString name)
{
    valore = field->text();
    ui->risultatoText->display(field->text());
    ui->calcTitle->setText(name);
    rislabel = field;
    risultato=NULL;
    rislineedit = NULL;
    disableButton = true;
    disableTimer = startTimer(300);
    ui->calcForm->show();
}
void  InsertCalc::activate(QLineEdit* field, QString name)
{
    valore = field->text();
    ui->risultatoText->display(field->text());
    ui->calcTitle->setText(name);
    rislineedit = field;
    risultato=NULL;
    rislabel=NULL;
    disableButton = true;
    disableTimer = startTimer(300);
    ui->calcForm->show();
}


void InsertCalc::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId() == disableTimer){
        killTimer(disableTimer);
        disableTimer = 0;
        disableButton = false;
    }

}
