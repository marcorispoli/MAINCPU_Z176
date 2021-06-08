#include "msgbox.h"
#include "ui_msgbox.h"
#include "gwindow.h"
#include "insertcalc.h"

#define FRAME_TEXT_YPOS 60
#define DELTA_POS       10
#define HBAR            36
#define HEDITABLE       60
#define BUTTON_FRAME    66


#define WY0 271     // Dimensione finestra 1
#define WY1 330     // Dimensione finestra 2

#define BFY0 202    // Posizione frame bottoni 1
#define BFY1 255    // Posizione frame bottoni 2

//extern InsertCalc* pInsertCalc;

extern GWinObj GWindowRoot;
msgBox::msgBox(int rotation, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::msgBox)
{
    ui->setupUi(this);
    scene = new QGraphicsScene();
    view = new QGraphicsView(scene);
    proxy = scene->addWidget(this);

    view->setWindowFlags(Qt::FramelessWindowHint);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setFixedSize(420,440);    // Dimensione della vista
    scene->setSceneRect(0,0,420,440);
    view->setAlignment(Qt::AlignLeft);
    view->rotate(rotation);       // Angolo di rotazione della vista corrente
    view->setScene(scene);
    view->move(190,20);

    connect(ui->okButton,SIGNAL(released()),this,SLOT(onOkButtonPressed()),Qt::UniqueConnection);
    connect(ui->cancButton,SIGNAL(released()),this,SLOT(onCancButtonPressed()),Qt::UniqueConnection);
    connect(ui->funcButton,SIGNAL(clicked()),this,SLOT(onFuncButtonPressed()),Qt::UniqueConnection);

    connect(ui->editable,SIGNAL(selectionChanged()),this,SLOT(onEditableSelection()),Qt::UniqueConnection);

    hText = 140;
    timerTimeout = 0;
    this->rotation = rotation;
}

msgBox::~msgBox()
{
    delete ui;
}

void msgBox::setWindow(int num)
{
    QRect rect;
    int hWindow;

    // Calcola l'altezza totale della finestra
    return;
    switch(num)
    {
        case 0: // Senza progress bar
            if(flags==0) hWindow = FRAME_TEXT_YPOS + hText+10 + DELTA_POS ;
            else hWindow = FRAME_TEXT_YPOS + hText+10 + DELTA_POS + BUTTON_FRAME + DELTA_POS;

            // Dimensione del frame di testo
            rect = ui->frameText->geometry();
            rect.setTop(FRAME_TEXT_YPOS);
            rect.setHeight(hText+10);
            ui->frameText->setGeometry(rect);

            // Dimensione del testo
            rect = ui->content->geometry();
            rect.setHeight(hText);
            rect.setTop(5);
            ui->content->setGeometry(rect);

            // Frame dei bottoni
            rect = ui->buttonFrame->geometry();
            rect.setTop(FRAME_TEXT_YPOS + hText+10 + DELTA_POS);
            rect.setHeight(BUTTON_FRAME);
            ui->buttonFrame->setGeometry(rect);

        break;
        case 1: // Con progress bar
            if(flags==msgBox::_PROGRESS_BAR) hWindow = FRAME_TEXT_YPOS + hText+10 + DELTA_POS + HBAR + DELTA_POS;
            else hWindow = FRAME_TEXT_YPOS + hText+10 + DELTA_POS + HBAR + DELTA_POS + BUTTON_FRAME + DELTA_POS;


            // Dimensione del frame di testo
            rect = ui->frameText->geometry();
            rect.setTop(FRAME_TEXT_YPOS);
            rect.setHeight(hText+10);
            ui->frameText->setGeometry(rect);

            // Dimensione del testo
            rect = ui->content->geometry();
            rect.setHeight(hText);
            rect.setTop(5);
            ui->content->setGeometry(rect);

            // Barra
            rect = ui->progressBar->geometry();
            rect.setY(FRAME_TEXT_YPOS + hText+10 + DELTA_POS);
            rect.setHeight(HBAR);
            ui->progressBar->setGeometry(rect);

            // Frame dei bottoni
            rect = ui->buttonFrame->geometry();
            rect.setY(FRAME_TEXT_YPOS + hText+10 + DELTA_POS + HBAR + DELTA_POS);
            rect.setHeight(BUTTON_FRAME);
            ui->buttonFrame->setGeometry(rect);


        break;
        case 2: // Con editable
            if(flags==msgBox::_EDITABLE) hWindow = FRAME_TEXT_YPOS + hText+10 + DELTA_POS + HEDITABLE + DELTA_POS;
            else hWindow = FRAME_TEXT_YPOS + hText+10 + DELTA_POS + HEDITABLE + DELTA_POS + BUTTON_FRAME + DELTA_POS;


            // Dimensione del frame di testo
            rect = ui->frameText->geometry();
            rect.setTop(FRAME_TEXT_YPOS);
            rect.setHeight(hText+10);
            ui->frameText->setGeometry(rect);

            // Dimensione del testo
            rect = ui->content->geometry();
            rect.setHeight(hText);
            rect.setTop(5);
            ui->content->setGeometry(rect);


            // Editable
            rect = ui->editable->geometry();
            rect.setY(FRAME_TEXT_YPOS + hText+10 + DELTA_POS);
            rect.setHeight(HEDITABLE);
            ui->editable->setGeometry(rect);

            // Frame dei bottoni
            rect = ui->buttonFrame->geometry();
            rect.setY(FRAME_TEXT_YPOS + hText+10 + DELTA_POS + HEDITABLE + DELTA_POS);
            rect.setHeight(BUTTON_FRAME);
            ui->buttonFrame->setGeometry(rect);


        break;
    default:
        break;
    }



    view->setFixedSize(800,480);    // Dimensione della vista
    scene->setSceneRect(0,0,800,480);
    view->setScene(scene);

}

void msgBox::activate(QString title, QString message, int opt)
{

    flags = opt;

    ui->title->setText(title);
    ui->content->setText(message);
    messaggio=message;

    if(flags&_BUTTON_FUNC) ui->funcButton->show();
    else ui->funcButton->hide();
    if(flags&_BUTTON_CANC) ui->cancButton->show();
    else ui->cancButton->hide();
    if(flags&_BUTTON_OK) ui->okButton->show();
    else ui->okButton->hide();

    if((flags==0)||(opt==_PROGRESS_BAR)) ui->buttonFrame->hide();
    else ui->buttonFrame->show();

    if(flags&_PROGRESS_BAR)
    {
        setWindow(1);
        ui->progressBar->show();
        ui->editable->hide();
    }
    else
    {
         ui->progressBar->hide();
        if(flags&_EDITABLE)
        {
            setWindow(2);
            ui->editable->show();
        }
        else
        {
            setWindow(0);
            ui->editable->hide();
        }
    }



    // Selezione stylesheet del colore dello studio
    if(!(opt&_COLOR_DEFAULT)){
        QString style;
        QString content;
        if(opt&_COLOR_DICOM){
            style = "background-color: rgb(79, 79, 79); font: 80 14pt \"DejaVu Sans\"; border-color: rgb(176,234,239);color: rgb(176,234,239);";
            content ="background-color: rgb(201, 201, 201); font: 80 12pt \"DejaVu Sans\";border-color: rgb(176,234,239);color: rgb(0,0,0);";
        }else if(opt&_COLOR_LOCAL){
            style = "background-color: rgb(79, 79, 79); font: 80 14pt \"DejaVu Sans\"; border-color: rgb(255, 255, 153);color: rgb(255, 255, 153);";
            content ="background-color: rgb(201, 201, 201); font: 80 12pt \"DejaVu Sans\"; border-color: rgb(255, 255, 153);color: rgb(0, 0, 0);";
        }
        ui->okButton->setStyleSheet(style);
        ui->cancButton->setStyleSheet(style);
        ui->funcButton->setStyleSheet(style);
        ui->title->setStyleSheet(style);

    }





    view->show();

}

void msgBox::activate(QString title, QString message, int height, int flags)
{
    hText = height;
    this->activate(title, message, flags);
}



void msgBox::timerEvent(QTimerEvent* ev)
{
    // Estingue la finestra dopo un timeout impostato dall'esterno
    if(ev->timerId()==timerTimeout){
        if(tic100msTimeout--==0){
            killTimer(timerTimeout);
            onCancButtonPressed();
            return;
        }else{
            setProgressBar(0,maxTimeout,maxTimeout-tic100msTimeout);
        }
    }
}

void  msgBox::setTimeout(long timeout)
{
    if(timerTimeout) killTimer(timerTimeout);
    if(timeout==0) {
        onCancButtonPressed();
        return;
    }
    ui->progressBar->show();

    tic100msTimeout = timeout/100;
    maxTimeout = tic100msTimeout;
    timerTimeout = startTimer(100);
    setProgressBar(0,100,0);

}

void  msgBox::setTitleLabel(QString title)
{
    ui->title->setText(title);

}

void  msgBox::setEditable(QString def, QString title)
{
    ui->editable->setText(def);
    editableTitle = title;
}

void  msgBox::setSuffix(QString def)
{
    ui->content->setText(messaggio + "\n" + def);

}

void  msgBox::setTextLabel(QString testo)
{
    ui->content->setText(testo);

}

void  msgBox::setTextAlignment(Qt::Alignment align)
{
    ui->content->setAlignment(align);
}

void msgBox::setProgressBar(int min, int max, int value)
{
    ui->progressBar->setMaximum(max);
    ui->progressBar->setMinimum(min);
    ui->progressBar->setValue(value);

}

void msgBox::setProgressBar(QString style)
{
    ui->progressBar->setStyleSheet(style);
}

void msgBox::setFuncLabel(QString lab)
{
    ui->funcButton->setText(lab);
}

void msgBox::setOKLabel(QString lab)
{
    ui->okButton->setText(lab);
}

void msgBox::setCANCLabel(QString lab)
{
    ui->cancButton->setText(lab);
}


QString msgBox::getEditable()
{
    return messaggio;
}

// Se viene chiamata la funzione ::keepAlive() nella callback, al termine
// della chiamata la MesgBox NON viene terninata. Di default viene iinvece terminata al termine della
// CALLBACK
void msgBox::onOkButtonPressed()
{
    terminate = true;
    emit buttonOkSgn();
    if(!terminate) return;
    this->view->hide();
    this->disconnect();
    sleepThread::msleep(500);
    //GWindowRoot.showCurrentPage();
}

// Se viene chiamata la funzione ::keepAlive() nella callback, al termine
// della chiamata la MesgBox NON viene terninata. Di default viene iinvece terminata al termine della
// CALLBACK
void msgBox::onCancButtonPressed()
{
    terminate = true;
    emit buttonCancSgn();
    if(!terminate) return;
    this->view->hide();
    this->disconnect();
    sleepThread::msleep(500);
    //GWindowRoot.showCurrentPage();
}

// Se viene chiamata la funzione ::keepAlive() nella callback, al termine
// della chiamata la MesgBox NON viene terninata. Di default viene iinvece terminata al termine della
// CALLBACK
void msgBox::onFuncButtonPressed()
{
    terminate = true;
    emit buttonFuncSgn();
    if(!terminate) return;
    this->view->hide();
    this->disconnect();
    sleepThread::msleep(500);
    //GWindowRoot.showCurrentPage();
}

void msgBox::onEditableSelection(void)
{
   // connect(pInsertCalc,SIGNAL(calcSgn(bool)),this,SLOT(onEditableSelectionNotify(bool)),Qt::UniqueConnection);
   //ui->editable->setText("");
   // pInsertCalc->activate(ui->editable,editableTitle);
    return;
}

void msgBox::onEditableSelectionNotify(bool result)
{
    if(result==false) return;

    terminate = true;
    emit editableFuncSgn();
    if(!terminate) return;
    this->view->hide();
    this->disconnect();
    sleepThread::msleep(500);
    //GWindowRoot.showCurrentPage();
}

void msgBox::externalHide(void)
{
    terminate = true;
    view->hide();
    disconnect();
    sleepThread::msleep(500);
    //GWindowRoot.showCurrentPage();
}
