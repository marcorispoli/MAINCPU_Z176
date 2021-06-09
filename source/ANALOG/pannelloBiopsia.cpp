#include "pannelloBiopsia.h"
#include "analog.h"
#include "../application.h"
#include "../appinclude.h"
#include "../globvar.h"
#include "../audio.h"
extern audio* pAudio;

pannelloBiopsia::pannelloBiopsia(QGraphicsView* view){
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

    backgroundPix = this->addPixmap(QPixmap(""));
    backgroundPix->setPos(0,0);

    // Grafica messaggi di stato
    font.setPointSize(25);
    font.setStretch(35);
    statusMsgLabel = new GLabel(this,QRectF(153,329,167,56),font,QColor(0,0,0),"",Qt::AlignCenter);

    // Grafica mAs
    font.setPointSize(50);
    font.setStretch(60);
    masLabel = new GLabel(this,QRectF(312,120,70,46),font,QColor(_W_TEXT),"640",Qt::AlignCenter);

    // Grafica KV
    font.setPointSize(50);
    font.setStretch(60);
    kvLabel = new GLabel(this,QRectF(74,120,56,46),font,QColor(_W_TEXT),"35",Qt::AlignCenter);

    // Grafica Ready Not Ready
    readyNotReadyPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/notready.png"));
    readyNotReadyPix->setOffset(155,278);

    // Grafica rotazione braccio
    armEnabledPix = this->addPixmap(QPixmap("://BiopsyPage/BiopsyPage/armEnabled.png"));
    armEnabledPix->setOffset(174,109);
    armEnabledPix->hide();
    font.setPointSize(30);
    font.setStretch(60);
    angoloArm = new GLabel(this,QRectF(205,219,60,45),font,QColor(0,0,0),"---",Qt::AlignCenter);

    // Grafica del tubo
    trxPix = this->addPixmap(QPixmap(""));
    trxPix->setOffset(128,1);

    // Stringhe lato Lesione
    QString stringa;

    font.setPointSize(18);
    font.setStretch(50);
    stringa = QString(QApplication::translate("BIOPSY-PAGE","LESION POSITION", 0, QApplication::UnicodeUTF8));
    lesionlabel = new GLabel(this,QRectF(307,175,134,20),font,QColor(255,255,255),stringa,Qt::AlignLeft);
    lesionlabel->hide();
    lesionPosition = new GLabel(this,QRectF(307,195,134,20),font,QColor(_Y_COL),"",Qt::AlignLeft);
    lesionPosition->hide();

    // Target nella finestra di gestione movimento
    font.setPointSize(18);
    font.setStretch(50);
    targetPosition= new GLabel(this,QRectF(280,220,134,20),font,QColor(_Y_COL),"",Qt::AlignLeft);
    targetPosition->hide();

    // Finestra di check position e aggiustamento
    font.setPointSize(18);
    font.setStretch(50);
    margPosition= new GLabel(this,QRectF(155,333,164,20),font,QColor(255,255,255),"",Qt::AlignCenter);
    margPosition->hide();

    font.setPointSize(18);
    font.setStretch(50);
    zPosition= new GLabel(this,QRectF(155,355,164,20),font,QColor(255,255,255),"",Qt::AlignCenter);
    zPosition->hide();


    stringa = QString(QApplication::translate("BIOPSY-PAGE","HOLDER", 0, QApplication::UnicodeUTF8));
    holderLabel = new GLabel(this,QRectF(307,255,134,20),font,QColor(255,255,255),stringa,Qt::AlignLeft);
    holderLabel->hide();


    stringa = QString(QApplication::translate("BIOPSY-PAGE","NEEDLE LENGTH", 0, QApplication::UnicodeUTF8));
    needleLengthLabel = new GLabel(this,QRectF(35,175,134,20),font,QColor(255,255,255),stringa,Qt::AlignLeft);
    needleLengthLabel->hide();
    needleMinLengthLabel = new GLabel(this,QRectF(35,195,134,20),font,QColor(_Y_COL),"",Qt::AlignLeft);
    needleMinLengthLabel->hide();
    needleMaxLengthLabel = new GLabel(this,QRectF(35,215,134,20),font,QColor(_Y_COL),"",Qt::AlignLeft);
    needleMaxLengthLabel->hide();
    needleLength = new GLabel(this,QRectF(35,235,134,20),font,QColor(_Y_COL),"",Qt::AlignLeft);
    needleLength->hide();

    // Label per il pannello di selezione ago
    font.setPointSize(50);
    font.setStretch(60);
    needleSelection = new GLabel(this,QRectF(96,63,285,68),font,QColor(0,0,0),"",Qt::AlignCenter);
    needleSelection->hide();

    font.setPointSize(28);
    font.setStretch(30);
    wrongCalcLabel = new GLabel(this,QRectF(70,170,340,150),font,QColor(255,0,0),stringa,Qt::AlignCenter);
    wrongCalcLabel->hide();


    // Stringhe di dose kV e mAs effettivi ultima esposizione
    font.setPointSize(20);
    font.setStretch(50);
    doseLabel = new GLabel(this,QRectF(329,310,100,20),font,QColor(_W_TECH_MODE),"AGD: ----",Qt::AlignLeft);

    font.setPointSize(20);
    font.setStretch(50);
    kvXLabel = new GLabel(this,QRectF(329,331,100,20),font,QColor(_W_TECH_MODE),"kV: ----",Qt::AlignLeft);

    font.setPointSize(20);
    font.setStretch(50);
    mAsXLabel = new GLabel(this,QRectF(329,352,100,20),font,QColor(_W_TECH_MODE),"mAs: ----",Qt::AlignLeft);

    // Questa deve essere l'ultima poichè deve stare sopra tutto
    xrayPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/xrayon.png"));
    xrayPix->setOffset(0,0);

    // Inizializza lo stato di esecuzione
    workflow = _BIOPSY_NO_STATUS;

    timerStatus = 0;
    timerDisable = 0;
    open_flag = false;
    parent->hide();
}



pannelloBiopsia::~pannelloBiopsia()
{
    return;
}

void pannelloBiopsia::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId()==timerDisable)
    {
        killTimer(timerDisable);
        timerDisable=0;
    }

    if(ev->timerId()==timerStatus)
    {
        if(!isMaster) return;

        if((workflow == _BIOPSY_MOVING) && (pBiopsy->movingCommand == _BIOPSY_MOVING_COMPLETED)){
            pBiopsy->movingCommand = _BIOPSY_MOVING_NO_COMMAND;

            if(pBiopsy->movingError == _BIOPSY_MOVING_NO_ERROR){
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_CHECK_POSITION);
            }else{
                switch(pBiopsy->movingError){
                    case _BIOPSY_MOVING_ERROR_BUSY:
                    case _BIOPSY_MOVING_ERROR_MCC:
                        PageAlarms::activateNewAlarm(_DB_ALLARMI_BIOPSIA,ERROR_BIOP_BUSY,TRUE);
                        ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_MOVE_BYM);
                    break;
                case _BIOPSY_MOVING_ERROR_TARGET:
                    PageAlarms::activateNewAlarm(_DB_ALLARMI_BIOPSIA,ERROR_BIOP_MOVE_XYZ,TRUE);
                    ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_INIT);
                    break;

                case _BIOPSY_MOVING_ERROR_TIMEOUT:
                    PageAlarms::activateNewAlarm(_DB_ALLARMI_BIOPSIA,ERROR_BIOP_TIMEOUT,TRUE);
                    ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_INIT);
                    break;
                }



            }
        }

    }
}



void pannelloBiopsia::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mousePressEvent(event);

    if(timerDisable) return;
    timerDisable=startTimer(500);
    QPointF mouse = event->scenePos();
    int num;

    switch(workflow){
        case _BIOPSY_INIT: // Inizializzazione modalità biopsia
            // Verifica pressione dei pulsanti per i kV e i mAs: sempre disponibili per ogni stato
            if(kvLabel->boundingRect().contains(event->scenePos())){
                ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_COMANDI_KV_SELECTION ,DBase::_DB_FORCE_SGN);
                return;
            }else if(masLabel->boundingRect().contains(event->scenePos())){
                ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_COMANDI_MAS_SELECTION ,DBase::_DB_FORCE_SGN);
                return;
            }

            // Pulsante start sequence: consentita solo con una compressione in corso!!
            if((mouse.x()>=75)&&(mouse.x()<=397)&&(mouse.y()>=397)&&(mouse.y()<=467)){
                if(ApplicationDatabase.getDataI(_DB_FORZA) == 0){
                     ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_MESSAGES, (int) _DB_ANALOG_BIOPSY_MESSAGES_ERROR_MISSING_COMPRESSION, DBase::_DB_FORCE_SGN);
                }else ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) (ApplicationDatabase.getDataI(_DB_ANALOG_BIOPSY_WORKFLOW)+1));
                return;
            }
        break;

        case _BIOPSY_SHOT_LEFT:
        case _BIOPSY_SHOT_RIGHT: // Inizializzazione modalità biopsia
            // Verifica pressione dei pulsanti per i kV e i mAs: sempre disponibili per ogni stato
            if(kvLabel->boundingRect().contains(event->scenePos())){
                ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_COMANDI_KV_SELECTION ,DBase::_DB_FORCE_SGN);
                return;
            }else if(masLabel->boundingRect().contains(event->scenePos())){
                ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_COMANDI_MAS_SELECTION ,DBase::_DB_FORCE_SGN);
                return;
            }

            // Pulsante Abort sequence
            if((mouse.x()>=75)&&(mouse.x()<=216)&&(mouse.y()>=397)&&(mouse.y()<=467)){
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_INIT);
                return;
            }

            // Pulsante Next sequence
            if((mouse.x()>=246)&&(mouse.x()<=397)&&(mouse.y()>=397)&&(mouse.y()<=467)){
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) (ApplicationDatabase.getDataI(_DB_ANALOG_BIOPSY_WORKFLOW)+1));
                return;
            }
        break;

        case  _BIOPSY_POINT_LESION:
        case  _BIOPSY_WAIT_REFERENCE_P15:
        case  _BIOPSY_WAIT_LESION_P15:
        case  _BIOPSY_WAIT_REFERENCE_M15:
        case  _BIOPSY_WAIT_LESION_M15:

            // Pulsante Abort sequence
            if((mouse.x()>=75)&&(mouse.x()<=397)&&(mouse.y()>=397)&&(mouse.y()<=467)){
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_INIT);
                return;
            }

        break;

        case _BIOPSY_WRONG_LESION_CALC:

            // Pulsante Abort sequence
            if((mouse.x()>=75)&&(mouse.x()<=230)&&(mouse.y()>=397)&&(mouse.y()<=467)){
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_INIT);
                return;
            }

            // Pulsante Repeat measurement
            if((mouse.x()>=240)&&(mouse.x()<=397)&&(mouse.y()>=397)&&(mouse.y()<=467)){
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_POINT_LESION);
                return;
            }

        break;
        case _BIOPSY_SELECTION_NEEDLE:
            // Pulsante Abort sequence
            if((mouse.x()>=75)&&(mouse.x()<=216)&&(mouse.y()>=397)&&(mouse.y()<=467)){
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_INIT);
                return;
            }

            // Pulsante Start Measurement
            if((mouse.x()>=72)&&(mouse.x()<=135)&&(mouse.y()>=308)&&(mouse.y()<=387)){
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_WAIT_REFERENCE_P15);
                return;
            }

            // Pulsante Next sequence
            if((mouse.x()>=246)&&(mouse.x()<=397)&&(mouse.y()>=397)&&(mouse.y()<=467)){
                if(ApplicationDatabase.getDataI(_DB_BIOP_AGO) == 0) {
                    ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_MESSAGES, (int) _DB_ANALOG_BIOPSY_MESSAGES_ERROR_INVALID_NEEDLE_LENGHT, DBase::_DB_FORCE_SGN);
                    return;
                }
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_MOVE_BYM);
                return;
            }

            // Pulsante Select Needle
            if((mouse.x()>=157)&&(mouse.x()<=316)&&(mouse.y()>=335)&&(mouse.y()<=383)){
                ApplicationDatabase.setData(_DB_BIOP_AGO, (int) 0);
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_NEEDLE_LENGTH);
                return;
            }
        break;
        case _BIOPSY_MOVE_BYM:
            // Pulsante Abort sequence
            if((mouse.x()>=180)&&(mouse.x()<=296)&&(mouse.y()>=397)&&(mouse.y()<=467)){
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_SELECTION_NEEDLE);
                return;
            }

            // Pulsante Move
            if((mouse.x()>=189)&&(mouse.x()<=277)&&(mouse.y()>=49)&&(mouse.y()<=155)){
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW,(int) _BIOPSY_MOVING);
                return;
            }

        break;
        case _BIOPSY_CHECK_POSITION:
            // Pulsante Abort sequence
            if((mouse.x()>=180)&&(mouse.x()<=296)&&(mouse.y()>=397)&&(mouse.y()<=467)){
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_INIT);
                return;
            }
            // Verifica pressione dei pulsanti per i kV e i mAs: sempre disponibili per ogni stato
            if(kvLabel->boundingRect().contains(event->scenePos())){
                ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_COMANDI_KV_SELECTION ,DBase::_DB_FORCE_SGN);
                return;
            }else if(masLabel->boundingRect().contains(event->scenePos())){
                ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_COMANDI_MAS_SELECTION ,DBase::_DB_FORCE_SGN);
                return;
            }


            // Pulsantiera remota
            if((mouse.x()>=113)&&(mouse.x()<=290)&&(mouse.y()>=113)&&(mouse.y()<=268)){

                if(mouse.x()<=236){
                    if(mouse.y()<=163){ // X-
                        ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_BUTTONS, (int) _DB_ANALOG_BIOPSY_BUTTON_Xm,DBase::_DB_FORCE_SGN|DBase::_DB_ONLY_MASTER_ACTION);
                    }else if(mouse.y()<=214){ // Y-
                        ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_BUTTONS, (int) _DB_ANALOG_BIOPSY_BUTTON_Ym,DBase::_DB_FORCE_SGN|DBase::_DB_ONLY_MASTER_ACTION);
                    }else{ // Z-
                        ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_BUTTONS, (int) _DB_ANALOG_BIOPSY_BUTTON_Zm,DBase::_DB_FORCE_SGN|DBase::_DB_ONLY_MASTER_ACTION);
                    }

                }else{
                    if(mouse.y()<=163){ // X+
                        ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_BUTTONS, (int) _DB_ANALOG_BIOPSY_BUTTON_Xp,DBase::_DB_FORCE_SGN|DBase::_DB_ONLY_MASTER_ACTION);
                    }else if(mouse.y()<=214){ // Y+
                        ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_BUTTONS, (int) _DB_ANALOG_BIOPSY_BUTTON_Yp,DBase::_DB_FORCE_SGN|DBase::_DB_ONLY_MASTER_ACTION);
                    }else{ // Z+
                        ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_BUTTONS, (int) _DB_ANALOG_BIOPSY_BUTTON_Zp,DBase::_DB_FORCE_SGN|DBase::_DB_ONLY_MASTER_ACTION);
                    }

                }
                return;
            }


            // Pulsante attivazione TRX EP15
            if((mouse.x()>=318)&&(mouse.x()<=360)&&(mouse.y()>=82)&&(mouse.y()<=116)){
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_BUTTONS, (int) _DB_ANALOG_BIOPSY_BUTTON_TRX_EP15,DBase::_DB_FORCE_SGN|DBase::_DB_ONLY_MASTER_ACTION);
                return;
            }
            // Pulsante attivazione TRX P15
            if((mouse.x()>=278)&&(mouse.x()<=322)&&(mouse.y()>=24)&&(mouse.y()<=69)){
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_BUTTONS, (int) _DB_ANALOG_BIOPSY_BUTTON_TRX_P15,DBase::_DB_FORCE_SGN|DBase::_DB_ONLY_MASTER_ACTION);
                return;
            }
            // Pulsante attivazione TRX CC
            if((mouse.x()>=216)&&(mouse.x()<=260)&&(mouse.y()>=6)&&(mouse.y()<=56)){
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_BUTTONS, (int) _DB_ANALOG_BIOPSY_BUTTON_TRX_CC,DBase::_DB_FORCE_SGN|DBase::_DB_ONLY_MASTER_ACTION);
                return;
            }
            // Pulsante attivazione TRX M15
            if((mouse.x()>=146)&&(mouse.x()<=195)&&(mouse.y()>=26)&&(mouse.y()<=69)){
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_BUTTONS, (int) _DB_ANALOG_BIOPSY_BUTTON_TRX_M15,DBase::_DB_FORCE_SGN|DBase::_DB_ONLY_MASTER_ACTION);
                return;
            }
            // Pulsante attivazione TRX EM15
            if((mouse.x()>=118)&&(mouse.x()<=162)&&(mouse.y()>=80)&&(mouse.y()<=124)){
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_BUTTONS, (int) _DB_ANALOG_BIOPSY_BUTTON_TRX_EM15,DBase::_DB_FORCE_SGN|DBase::_DB_ONLY_MASTER_ACTION);
                return;
            }




        break;
        case _BIOPSY_NEEDLE_LENGTH:
            // Pulsante Abort
            if((mouse.x()>=272)&&(mouse.x()<=379)&&(mouse.y()>=404)&&(mouse.y()<=467)){
                ApplicationDatabase.setData(_DB_BIOP_AGO, (int) 0);
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_SELECTION_NEEDLE);
                return;
            }

            // Pulsante OK
            if((mouse.x()>=110)&&(mouse.x()<=200)&&(mouse.y()>=404)&&(mouse.y()<=467)){

                // Controllo minimo massimo
                num = ApplicationDatabase.getDataI(_DB_BIOP_AGO);
                if(num < ApplicationDatabase.getDataI(_DB_BIOP_MIN_AGO)) num = 0;
                else if(num > ApplicationDatabase.getDataI(_DB_BIOP_MAX_AGO)) num = 0;
                if(num==0)  ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_MESSAGES, (int) _DB_ANALOG_BIOPSY_MESSAGES_ERROR_INVALID_NEEDLE_LENGHT, DBase::_DB_FORCE_SGN);
                ApplicationDatabase.setData(_DB_BIOP_AGO, (int) num);
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_SELECTION_NEEDLE);
                return;
            }

            // Pulsantiera _______________________________________________________________
            if((mouse.x()>=86)&&(mouse.x()<=400)&&(mouse.y()>=146)&&(mouse.y()<=400)){
                num = ApplicationDatabase.getDataI(_DB_BIOP_AGO);
                if((mouse.x()>=86)&&(mouse.x()<=183)){

                    if((mouse.y()>=146)&&(mouse.y()<=215)){ // 7
                        num = num*10 + 7;
                    }else if((mouse.y()>=216)&&(mouse.y()<=269)){ // 4
                        num = num*10 + 4;
                    }else if((mouse.y()>=270)&&(mouse.y()<=334)){ // 1
                        num = num*10 + 1;
                    }else if((mouse.y()>=335)&&(mouse.y()<=400)){ // BK
                        num = num/10;
                    }
                }else  if((mouse.x()>=184)&&(mouse.x()<=293)){
                    if((mouse.y()>=146)&&(mouse.y()<=215)){ // 8
                        num = num*10 + 8;
                    }else if((mouse.y()>=216)&&(mouse.y()<=269)){ // 5
                        num = num*10 + 5;
                    }else if((mouse.y()>=270)&&(mouse.y()<=334)){ // 2
                        num = num*10 + 2;
                    }else if((mouse.y()>=335)&&(mouse.y()<=400)){ // 0
                        num = num*10 + 0;
                    }

                }else  if((mouse.x()>=294)&&(mouse.x()<=400)){
                    if((mouse.y()>=146)&&(mouse.y()<=215)){ // 9
                        num = num*10 + 9;
                    }else if((mouse.y()>=216)&&(mouse.y()<=269)){ // 6
                        num = num*10 + 6;
                    }else if((mouse.y()>=270)&&(mouse.y()<=334)){ // 3
                        num = num*10 + 3;
                    }else if((mouse.y()>=335)&&(mouse.y()<=400)){ // CLS
                        num = 0;
                    }
                }

                ApplicationDatabase.setData(_DB_BIOP_AGO, (int) num);

            }
            //____________________________________________________________________________
        break;
    }


}

/* Questa funzione deve essere eseguita solo dal master */
/*
 * unsigned short dmm_ref_p15_JX;
    unsigned short dmm_ref_p15_JY;
    unsigned short dmm_les_p15_JX;
    unsigned short dmm_les_p15_JY;
    unsigned short dmm_ref_m15_JX;
    unsigned short dmm_ref_m15_JY;
    unsigned short dmm_les_m15_JX;
    unsigned short dmm_les_m15_JY;
 */
void pannelloBiopsia::manageConsoleButtons(int buttons){
    if(!isMaster) return;

    // In ogni stato viene gestito il pulsante di reset:
    // Si richiede una conferma ...
    if(buttons & _BP_BIOP_PUSH_RESET){

        return;
    }

    // GEstione pulsante ENTER
    if(buttons & _BP_BIOP_PUSH_SEQ){
        switch(workflow){
            case _BIOPSY_POINT_LESION:
                // Il pulsante ENT inizia la sequenza di ricerca della lesione
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) (ApplicationDatabase.getDataI(_DB_ANALOG_BIOPSY_WORKFLOW)+1)); // Step dell'acquisizione
            break;
            case  _BIOPSY_WAIT_REFERENCE_P15:
                pBiopsy->dmm_ref_p15_JX = (float) pBiopsy->dmmJX;
                pBiopsy->dmm_ref_p15_JY = (float) pBiopsy->dmmJY;
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) (ApplicationDatabase.getDataI(_DB_ANALOG_BIOPSY_WORKFLOW)+1)); // Step dell'acquisizione
            break;
            case  _BIOPSY_WAIT_LESION_P15:
                pBiopsy->dmm_les_p15_JX =(float)  pBiopsy->dmmJX;
                pBiopsy->dmm_les_p15_JY = (float) pBiopsy->dmmJY;
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) (ApplicationDatabase.getDataI(_DB_ANALOG_BIOPSY_WORKFLOW)+1)); // Step dell'acquisizione
            break;
            case  _BIOPSY_WAIT_REFERENCE_M15:
                pBiopsy->dmm_ref_m15_JX = (float) pBiopsy->dmmJX;
                pBiopsy->dmm_ref_m15_JY = (float) pBiopsy->dmmJY;
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) (ApplicationDatabase.getDataI(_DB_ANALOG_BIOPSY_WORKFLOW)+1)); // Step dell'acquisizione
            break;
            case _BIOPSY_WAIT_LESION_M15:
                pBiopsy->dmm_les_m15_JX = (float) pBiopsy->dmmJX;
                pBiopsy->dmm_les_m15_JY =(float)  pBiopsy->dmmJY;

                // A questo punto si può effettuare il calcolo della lesione
                if(pBiopsy->calcLesionPosition() == false){
                    ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_ERR_CALC, (int)  pBiopsy->calcError, DBase::_DB_FORCE_SGN);
                    ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_WRONG_LESION_CALC); // Step dell'acquisizione
                }else{
                    ApplicationDatabase.setData(_DB_BIOP_LES_X, (int) pBiopsy->Xlesione_dmm);
                    ApplicationDatabase.setData(_DB_BIOP_LES_Y, (int) pBiopsy->Ylesione_dmm);
                    ApplicationDatabase.setData(_DB_BIOP_LES_Z, (int) pBiopsy->Zlesione_dmm);
                    ApplicationDatabase.setData(_DB_BIOP_LES_ZFIBRA, (int) pBiopsy->Zfibra_dmm);

                    // Calcolo Massimo Ago
                    ApplicationDatabase.setData(_DB_BIOP_MAX_AGO, (int) pBiopsy->Zlesione_dmm / 10);

                    // Calcolo Min Ago
                    ApplicationDatabase.setData(_DB_BIOP_MIN_AGO, (int) (pBiopsy->Zlesione_dmm/10) - pBiopsy->maxZ_mm );
                    ApplicationDatabase.setData(_DB_BIOP_AGO, (int) 0);
                    ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_SELECTION_NEEDLE); // Step dell'acquisizione
                }
            break;
        }

        return;
    }

    // Bottone BACKWORD
    if(buttons & _BP_BIOP_PUSH_BACK){
        int num;
        switch(workflow){
            case _BIOPSY_POINT_LESION:
            case  _BIOPSY_WAIT_REFERENCE_P15:
            case  _BIOPSY_WAIT_LESION_P15:
            case  _BIOPSY_WAIT_REFERENCE_M15:
            case _BIOPSY_WAIT_LESION_M15:
                ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_POINT_LESION); // Ricomincia il puntamento
            break;
            case _BIOPSY_SELECTION_NEEDLE:
                num = ApplicationDatabase.getDataI(_DB_BIOP_AGO);
                if(num==0) return;
                num -= last_increment;
                if(num < ApplicationDatabase.getDataI(_DB_BIOP_MIN_AGO)) num = ApplicationDatabase.getDataI(_DB_BIOP_MIN_AGO);
                ApplicationDatabase.setData(_DB_BIOP_AGO, (int) num);
            break;
        }

        return;
    }

    // Bottone +1
    if(buttons & _BP_BIOP_PUSH_AGO_1){
        int num;
        switch(workflow){
            case _BIOPSY_SELECTION_NEEDLE:
                last_increment = 1;
                num = ApplicationDatabase.getDataI(_DB_BIOP_AGO);
                num ++;
                if(num > ApplicationDatabase.getDataI(_DB_BIOP_MAX_AGO)) num = ApplicationDatabase.getDataI(_DB_BIOP_MAX_AGO);
                ApplicationDatabase.setData(_DB_BIOP_AGO, (int) num);
            break;
        }

        return;
    }

    // Bottone +10
    if(buttons & _BP_BIOP_PUSH_AGO_10){
        int num;
        switch(workflow){
            case _BIOPSY_SELECTION_NEEDLE:
                last_increment = 10;
                num = ApplicationDatabase.getDataI(_DB_BIOP_AGO);
                num += 10;
                if(num > ApplicationDatabase.getDataI(_DB_BIOP_MAX_AGO)) num = ApplicationDatabase.getDataI(_DB_BIOP_MAX_AGO);
                ApplicationDatabase.setData(_DB_BIOP_AGO, (int) num);
            break;
        }

        return;
    }

    return;
}


void pannelloBiopsia::open(){
    if(open_flag) return;
    open_flag = true;

    // Inizializza lo stato corrente
    changeBiopsyWorkflowStatus(ApplicationDatabase.getDataI(_DB_ANALOG_BIOPSY_WORKFLOW), true);

    int flags = ApplicationDatabase.getDataI(_DB_ANALOG_FLAGS);

    // Impostazione kV e mAs
    setKv(ApplicationDatabase.getDataI(_DB_DKV));
    setdmAs(ApplicationDatabase.getDataI(_DB_DMAS));

    // Ready / Not ready
    setReady(flags&_DB_ANFLG_EXP_READY);

    // Visualizzazione Posizione Tubo
    setTrxPicture();

    // Visualizzazione stato rotazioni
    setEnableRot();


    //backgroundPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannellokV.png"));
    timerDisable=startTimer(500);
    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);



}


// Funzione di impostazione Items in relazione allo stato corrente
void pannelloBiopsia::changeBiopsyWorkflowStatus(unsigned char wf, bool force){

    int X,Y,Z;

    if(isMaster){
        if(timerStatus) killTimer(timerStatus);
        timerStatus = 0;
    }

    // SE non c'è cambio stato non fa nulla a meno che non vi sia un force
    if((!force)&&(wf == workflow)) return;


    statusMsgLabel->hide();
    masLabel->hide();
    kvLabel->hide();
    readyNotReadyPix->hide();
    angoloArm->hide();
    trxPix->hide();
    armEnabledPix->hide();
    lesionlabel->hide();
    lesionPosition->hide();
    needleLengthLabel->hide();
    needleMinLengthLabel->hide();
    needleMaxLengthLabel->hide();
    needleLength->hide();
    needleSelection->hide();
    targetPosition->hide();
    holderLabel->hide();    
    zPosition->hide();
    margPosition->hide();
    wrongCalcLabel->hide();
    mAsXLabel->hide();
    kvXLabel->hide();
    doseLabel->hide();

    switch(wf){
        case _BIOPSY_INIT: // Inizializzazione modalità biopsia           
            backgroundPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/backgroundBiopsia_INIT.png"));
            trxPix->show();
            angoloArm->show();
            statusMsgLabel->show();
            masLabel->show();
            kvLabel->show();
            readyNotReadyPix->show();

            // Solo su cambio di stato e non a seguito di un force da init panel
            if(wf!=workflow){
               workflow = wf;
               ApplicationDatabase.setData(_DB_BIOP_AGO,(int) 0, DBase::_DB_NO_CHG_SGN|DBase::_DB_NO_ECHO);
               if(isMaster){
                   setTrx(0);
                   pBiopsy->moveHome();

                   // Inizializza i dati di dose
                   ApplicationDatabase.setData(_DB_XDMAS,(int) 0);
                   ApplicationDatabase.setData(_DB_XDKV,(int) 0);
                   ApplicationDatabase.setData(_DB_X_UDOSE, QString("AGD: ----"));
               }
            }
        break;
        case _BIOPSY_SHOT_RIGHT: // Inizializzazione modalità biopsia
            backgroundPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/backgroundBiopsia_SHOT_RIGHT.png"));
            trxPix->show();
            angoloArm->show();
            statusMsgLabel->show();
            masLabel->show();
            kvLabel->show();
            readyNotReadyPix->show();
            mAsXLabel->show();
            kvXLabel->show();
            doseLabel->show();

            // Solo su cambio di stato e non a seguito di un force
            if(wf!=workflow){
               workflow = wf;
               if(isMaster){
                   setTrx(15);
                   pAudio->playAudio(AUDIO_BIOPSY_EXPOSE_RIGHT);
               }
            }
        break;
        case _BIOPSY_SHOT_LEFT:
            trxPix->show();
            angoloArm->show();
            statusMsgLabel->show();
            masLabel->show();
            kvLabel->show();
            readyNotReadyPix->show();
            mAsXLabel->show();
            kvXLabel->show();
            doseLabel->show();

            // Solo su cambio di stato e non a seguito di un force
            if(wf!=workflow){
               workflow = wf;
               if(isMaster){
                   setTrx(-15);
                   pAudio->playAudio(AUDIO_BIOPSY_EXPOSE_LEFT);
               }
            }
        break;
        case _BIOPSY_POINT_LESION:
            backgroundPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/backgroundBiopsia_POINT_LESION.png"));

            // Solo su cambio di stato e non a seguito di un force
            if(wf!=workflow){
               workflow = wf;
               if(isMaster){
                   pAudio->playAudio(AUDIO_BIOPSY_INIT_POINTING);
               }
            }
        break;
        case _BIOPSY_WAIT_REFERENCE_M15:
            backgroundPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/backgroundBiopsia_REFm15.png"));

            // Solo su cambio di stato e non a seguito di un force
            if(wf!=workflow){
               workflow = wf;
               if(isMaster){
                   pAudio->playAudio(AUDIO_BIOPSY_RIGHT_REFERENCE);
               }
            }
        break;
        case _BIOPSY_WAIT_LESION_M15:
            backgroundPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/backgroundBiopsia_LESm15.png"));

            // Solo su cambio di stato e non a seguito di un force
            if(wf!=workflow){
               workflow = wf;
               if(isMaster){
                   pAudio->playAudio(AUDIO_BIOPSY_RIGHT_LESION);
               }
            }
        break;
        case _BIOPSY_WAIT_REFERENCE_P15:
            backgroundPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/backgroundBiopsia_REF15.png"));

            // Solo su cambio di stato e non a seguito di un force
            if(wf!=workflow){
               workflow = wf;
               if(isMaster){
                   pAudio->playAudio(AUDIO_BIOPSY_LEFT_REFERENCE);
               }
            }
        break;
        case _BIOPSY_WAIT_LESION_P15:
            backgroundPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/backgroundBiopsia_LES15.png"));

            // Solo su cambio di stato e non a seguito di un force
            if(wf!=workflow){
               workflow = wf;
               if(isMaster){
                   pAudio->playAudio(AUDIO_BIOPSY_LEFT_LESION);
               }
            }
        break;
        case _BIOPSY_WRONG_LESION_CALC:
            backgroundPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/backgroundBiopsia_WRONG_CALC.png"));

            wrongCalcLabel->show();
            // Show Error string

        break;
        case _BIOPSY_SELECTION_NEEDLE:
            backgroundPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/backgroundBiopsia_SELECT_NEEDLE.png"));
            trxPix->show();
            angoloArm->show();
            statusMsgLabel->show();
            masLabel->show();
            kvLabel->show();
            readyNotReadyPix->show();
            last_increment = 1;

            needleLengthLabel->setPos(35,175);
            needleLength->setPos(35,235);

            lesionPosition->setPos(307,195);
            lesionlabel->setPos(307,175);

            setLesionPosition();
            setNeedleLength();

            // Solo su cambio di stato e non a seguito di un force
            if(wf!=workflow){
               workflow = wf;
               if(isMaster){
                   if(ApplicationDatabase.getDataI(_DB_BIOP_AGO) == 0) pAudio->playAudio(AUDIO_BIOPSY_SELECT_NEEDLE);
               }
            }
        break;
        case _BIOPSY_MOVING:
            backgroundPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/backgroundBiopsia_MOVING.png"));

            // Solo su cambio di stato e non a seguito di un force
            if(wf!=workflow){
               workflow = wf;
               if(isMaster){
                   if(timerStatus==0) timerStatus = startTimer(1000);
                   pBiopsy->moveXYZ(pBiopsy->targetX_dmm, pBiopsy->targetY_dmm, pBiopsy->targetZ_dmm);
               }
            }

            break;

        case _BIOPSY_MOVE_BYM:
            backgroundPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/backgroundBiopsia_MOVE_BYM.png"));

            // Ago
            needleLengthLabel->setPos(147,261);
            needleLength->show();
            needleLength->setPos(147,281);

            // Puntamento meccanico torretta
            X = ApplicationDatabase.getDataI(_DB_BIOP_LES_X);
            Y = ApplicationDatabase.getDataI(_DB_BIOP_LES_Y);
            Z = ApplicationDatabase.getDataI(_DB_BIOP_LES_Z) - ApplicationDatabase.getDataI(_DB_BIOP_AGO) * 10;

            targetPosition->setPlainText(QString("X:%1 Y:%2 Z:%3").arg(X).arg(Y).arg(Z).toAscii().data());
            targetPosition->update();
            targetPosition->show();

            // Lesion position
            lesionlabel->setPos(280,316);
            lesionPosition->setPos(280,336);
            lesionPosition->show();

            if(wf!=workflow){
                workflow = wf;
                if(isMaster){
                    pAudio->playAudio(AUDIO_BIOPSY_READY_TO_MOVE);
                    pBiopsy->targetX_dmm = X;
                    pBiopsy->targetY_dmm = Y;
                    pBiopsy->targetZ_dmm = Z;
                }
            }
        break;
        case _BIOPSY_CHECK_POSITION:
            backgroundPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/backgroundBiopsia_CHECK_POSITION.png"));
            trxPix->show();
            //angoloArm->show();
            statusMsgLabel->show();
            masLabel->show();
            kvLabel->show();
            readyNotReadyPix->show();

            needleLengthLabel->setPos(35,175);
            needleLength->setPos(35,235);

            lesionlabel->setPos(307,175);
            lesionPosition->setPos(307,195);

            mAsXLabel->show();
            kvXLabel->show();
            doseLabel->show();

            if(wf!=workflow){
                workflow = wf;
                setCurrentNeedlePosition();
                setLesionPosition();
                setNeedleLength();
                setTrxPicture();
            }
        break;

        // Pannello speciale di inserimento lunghezza ago
        case _BIOPSY_NEEDLE_LENGTH:
            needleSelection->show();
            backgroundPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/backgroundNeedleLength.png"));

        break;
    }

    workflow = wf;
    setStatusMessage(); // Aggiorna lo status message
    parent->show();

    //parent->update();
}

void pannelloBiopsia::exit(){
    if(!open_flag) return;
    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));


    // Operazioni in chiusura
    open_flag = false;
    parent->hide();

}

/* _____________________________________________________________________________________________________________________________________
 * Imposta i messgagi di testo in funzione della fase corrente
 _____________________________________________________________________________________________________________________________________ */
void pannelloBiopsia::setStatusMessage(void){
    QString stringa;
    switch(workflow){
        case _BIOPSY_INIT:
            stringa = QString(QApplication::translate("BIOPSY-MESSAGE","PRESS START", 0, QApplication::UnicodeUTF8));
        break;
        case _BIOPSY_SHOT_RIGHT: // Inizializzazione modalità biopsia
            stringa = QString(QApplication::translate("BIOPSY-MESSAGE","PUT THE CASSETTE \nRIGHT SIDE", 0, QApplication::UnicodeUTF8));
        break;
        case _BIOPSY_SHOT_LEFT:
            stringa = QString(QApplication::translate("BIOPSY-MESSAGE","PUT THE CASSETTE \nLEFT SIDE", 0, QApplication::UnicodeUTF8));
        break;
        case _BIOPSY_POINT_LESION:
            stringa = QString(QApplication::translate("BIOPSY-MESSAGE","PRESS START MEASURE", 0, QApplication::UnicodeUTF8));
        break;        

        case _BIOPSY_MOVE_BYM:
            stringa = QString(QApplication::translate("BIOPSY-MESSAGE","WAIT FOR POINTING", 0, QApplication::UnicodeUTF8));
        break;
        case _BIOPSY_CHECK_POSITION:
            stringa = QString(QApplication::translate("BIOPSY-MESSAGE","", 0, QApplication::UnicodeUTF8));
        break;
    }

    statusMsgLabel->setPlainText(stringa.toAscii().data());
    statusMsgLabel->update();
}


void pannelloBiopsia::valueChanged(int index,int opt)
{

    if((isMaster)&&(opt&DBase::_DB_ONLY_SLAVE_ACTION)) return;
    if((!isMaster)&&(opt&DBase::_DB_ONLY_MASTER_ACTION)) return;


    switch(index){
    case _DB_FORZA:
        if(!isMaster) return;

        // Nel caso in cui la procedura sia già cominciata un rilascio della compressione invalida tutto
        if(workflow != _BIOPSY_INIT){
            if(ApplicationDatabase.getDataI(index) == 0){
                // Emette l'errore relativo alla mancata compressione e il messaggio vocale
                actionAfterAlarm = BIOPSY_AFTER_ALARM_RESET_SEQUENCE;
                PageAlarms::activateNewAlarm(_DB_ALLARMI_BIOPSIA,ERROR_BIOP_MISSING_COMPRESSION,TRUE);
                pAudio->playAudio(AUDIO_BIOPSY_INVALID_COMPRESSION_RELEASE);
            }
        }
        break;
    case _DB_DKV:
        setKv(ApplicationDatabase.getDataI(_DB_DKV));
        break;
    case _DB_DMAS:
        setdmAs(ApplicationDatabase.getDataI(_DB_DMAS));
        break;

    case _DB_ANALOG_FLAGS:
        setReady(ApplicationDatabase.getDataI(index) & _DB_ANFLG_EXP_READY);
        break;

    case _DB_TRX:
        setTrxPicture();
        break;

    case _DB_ALLARMI_ALR_ARM:
    case _DB_ALLARMI_ALR_TRX:
    case _DB_ENABLE_MOVIMENTI:
        setEnableRot();
        break;

    case _DB_ANALOG_BIOPSY_WORKFLOW: // Cambio del Workflow
        changeBiopsyWorkflowStatus(ApplicationDatabase.getDataI(index), false);
        break;

    case _DB_BIOP_AGO:
        needleSelection->setPlainText(QString("%1 (mm)").arg(ApplicationDatabase.getDataI(index)).toAscii().data());
        needleSelection->update();

        if(isMaster) pBiopsy->setLunghezzaAgo((unsigned char) ApplicationDatabase.getDataI(index));
        break;

    case _DB_ANALOG_BIOPSY_MESSAGES:
        if(!isMaster) return;
        switch(ApplicationDatabase.getDataI(index)){

            // Errore generato al tentativo di inizio sequenza senza compressione in corso
            case _DB_ANALOG_BIOPSY_MESSAGES_ERROR_MISSING_COMPRESSION:
                actionAfterAlarm = BIOPSY_AFTER_ALARM_NO_ACTION;
                // Emette l'errore relativo alla mancata compressione e il messaggio vocale
                PageAlarms::activateNewAlarm(_DB_ALLARMI_BIOPSIA,ERROR_BIOP_APPLY_COMPRESSION,TRUE);
                pAudio->playAudio(AUDIO_NOT_READY_APPLY_COMPRESSION);
            break;
        case _DB_ANALOG_BIOPSY_MESSAGES_ERROR_INVALID_NEEDLE_LENGHT:
            if(isMaster){
                pAudio->playAudio(AUDIO_BIOPSY_INVALID_NEEDLE_SELECTED);
            }

            break;
        }

        break;

    case _DB_BIOP_CONSOLE_BUTTON:
        if(!isMaster) return;
        manageConsoleButtons(ApplicationDatabase.getDataI(index));
        break;
    case _DB_ANALOG_BIOPSY_BUTTONS:

        switch(ApplicationDatabase.getDataI(index)){
            case _DB_ANALOG_BIOPSY_HOME:
                pBiopsy->moveHome();
                break;
            case _DB_ANALOG_BIOPSY_BUTTON_Xp:
                pBiopsy->moveIncX();
                break;
            case _DB_ANALOG_BIOPSY_BUTTON_Xm:
                pBiopsy->moveDecX();
                break;
            case _DB_ANALOG_BIOPSY_BUTTON_Yp:
                pBiopsy->moveIncY();
                break;
            case _DB_ANALOG_BIOPSY_BUTTON_Ym:
                pBiopsy->moveDecY();
                break;
            case _DB_ANALOG_BIOPSY_BUTTON_Zp:
                pBiopsy->moveIncZ();
                break;
            case _DB_ANALOG_BIOPSY_BUTTON_Zm:
                pBiopsy->moveDecZ();
                break;

            case _DB_ANALOG_BIOPSY_BUTTON_TRX_P15:
                setTrx(15);
                break;
            case _DB_ANALOG_BIOPSY_BUTTON_TRX_M15:
                setTrx(-15);
                break;
            case _DB_ANALOG_BIOPSY_BUTTON_TRX_CC:
                setTrx(0);
                break;
            case _DB_ANALOG_BIOPSY_BUTTON_TRX_EP15:
                setTrx(25);
                break;
            case _DB_ANALOG_BIOPSY_BUTTON_TRX_EM15:
                setTrx(-25);
                break;
        }

        break;

    case _DB_BIOP_Z:
    case _DB_BIOP_Y:
    case _DB_BIOP_X:
        if(workflow!=_BIOPSY_CHECK_POSITION) return;
        setCurrentNeedlePosition();

    break;
    case _DB_ANALOG_BIOPSY_ERR_CALC:
        wrongCalcLabel->setPlainText(paginaAllarmi->getErrorString(_DB_ALLARMI_BIOPSIA, ApplicationDatabase.getDataI(_DB_ANALOG_BIOPSY_ERR_CALC)).toAscii().data());
    break;

    // Esito Dosimetrico esposizione
    case _DB_X_UDOSE:
        doseLabel->setPlainText(ApplicationDatabase.getDataS(index).toAscii().data());
        doseLabel->update();
        break;

    case _DB_XDMAS:
        if(ApplicationDatabase.getDataI(index)==0) mAsXLabel->setPlainText(QString("mAs: ----").toAscii().data());
        else   mAsXLabel->setPlainText(QString("mAs: %1").arg(((float)ApplicationDatabase.getDataI(index))/10).toAscii().data());
        mAsXLabel->update();
        break;
    case _DB_XDKV:
        if(ApplicationDatabase.getDataI(index)==0) kvXLabel->setPlainText(QString("kV: ----").toAscii().data());
        else   kvXLabel->setPlainText(QString("kV: %1").arg(((float)ApplicationDatabase.getDataI(index))/10).toAscii().data());
        kvXLabel->update();
        break;

    default:
        break;
    }
}

void pannelloBiopsia::setCurrentNeedlePosition(void){
    int X,Y,Z,MG,ZLM;

    // Differenza Lesione Posizione attuale
    X = -ApplicationDatabase.getDataI(_DB_BIOP_LES_X) + ApplicationDatabase.getDataI(_DB_BIOP_X);
    Y = -ApplicationDatabase.getDataI(_DB_BIOP_LES_Y) + ApplicationDatabase.getDataI(_DB_BIOP_Y);
    Z = -ApplicationDatabase.getDataI(_DB_BIOP_LES_Z) + ApplicationDatabase.getDataI(_DB_BIOP_Z) + ApplicationDatabase.getDataI(_DB_BIOP_AGO) * 10;

    // Pargini di spostamento rispetto alla fibra di carbonio e al compressore
    MG = ApplicationDatabase.getDataI(_DB_BIOP_MARG);
    ZLM = ApplicationDatabase.getDataI(_DB_BIOP_ZLIMIT) - ApplicationDatabase.getDataI(_DB_BIOP_Z) / 10;

    if((MG < 1) || (ZLM < 1)){
        margPosition->setColor(QColor(_R_COL));
        zPosition->setColor(QColor(_R_COL));
    } else  if((MG < 20) || (ZLM < 20)) {
        margPosition->setColor(QColor(225,117,22));
        zPosition->setColor(QColor(225,117,22));
    }else{
        margPosition->setColor(QColor(0,0,0));
        zPosition->setColor(QColor(0,0,0));
    }

    margPosition->setPlainText(QString("MG:%1 - CMP:%2 (mm)").arg(MG/10).arg(ZLM).toAscii().data());
    zPosition->setPlainText(QString("dX:%1 dY:%2 dZ:%3").arg((float)X/10).arg((float)Y/10).arg((float)Z/10).toAscii().data());
    zPosition->update();
    margPosition->update();
    zPosition->show();
    margPosition->show();
}

/* _____________________________________________________________________________________________________________________________________
 * Imposta il campo dei kV (val = kv*10)
 _____________________________________________________________________________________________________________________________________ */
void pannelloBiopsia::setKv(int val){    
    kvLabel->setPlainText(QString("%1").arg((float) val /10).toAscii().data());
    kvLabel->update();
}


/* _____________________________________________________________________________________________________________________________________
 * Imposta il campo dei mAs
 _____________________________________________________________________________________________________________________________________ */
void pannelloBiopsia::setdmAs(int dmAs){    
    masLabel->setPlainText(QString("%1").arg(((float)dmAs)/10).toAscii().data());
    masLabel->update();
}

/* _____________________________________________________________________________________________________________________________________
 * Impostazione Campo Ready Push Button
 _____________________________________________________________________________________________________________________________________ */
void pannelloBiopsia::setReady(bool ready){
    if(workflow == _BIOPSY_NEEDLE_LENGTH) return;
    if(ready)   readyNotReadyPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/ready.png"));
    else readyNotReadyPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/notready.png"));
}

/* _____________________________________________________________________________________________________________________________________
 * Imposta il campo Angolo
 _____________________________________________________________________________________________________________________________________ */
void pannelloBiopsia::setArm(int angolo){
    if(workflow == _BIOPSY_NEEDLE_LENGTH) return;
    angoloArm->setPlainText(QString("%1").arg(angolo).toAscii().data());
    angoloArm->update();
}

/* _____________________________________________________________________________________________________________________________________
 * Imposta la Pixmap relativa alla posizione e stato del TRX
 _____________________________________________________________________________________________________________________________________ */
void pannelloBiopsia::setTrxPicture(void){
    int angolo = pConfig->convertDangolo(ApplicationDatabase.getDataI(_DB_TRX));


    if(workflow <= _BIOPSY_POINT_LESION) {
        if((angolo>=14) && (angolo<=16)) trxPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/trxRightDisabled.png"));
        else if((angolo>=-16) && (angolo<=-14)) trxPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/trxLeftDisabled.png"));
        else if((angolo>=-1) && (angolo<=1)) trxPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/trxCenterDisabled.png"));
        trxPix->setOffset(128,1);
    }else if(workflow == _BIOPSY_CHECK_POSITION){
        trxPix->setPixmap(QPixmap("://BiopsyPage/BiopsyPage/tubePositioner.png"));
        if((angolo<-5)) trxPix->setOffset(159,35);
        else if((angolo<5)) trxPix->setOffset(230,17);
        else trxPix->setOffset(297,36);
    }
}

/*
 *  X,Y = distanza lesione da zero torretta
 *  Z = Distanza lesione da fibra di carbonio
 */
void pannelloBiopsia::setLesionPosition(void){
    float X,Y,Z;

    X = (float) ApplicationDatabase.getDataI(_DB_BIOP_LES_X)/10;
    Y = (float) ApplicationDatabase.getDataI(_DB_BIOP_LES_Y)/10;
    Z = (float) ApplicationDatabase.getDataI(_DB_BIOP_Z_FIBRA) - (float) ApplicationDatabase.getDataI(_DB_BIOP_LES_Z)/10;

    lesionlabel->show();
    lesionPosition->show();
    lesionPosition->setPlainText(QString("X=%1; Y=%2; Z=%3").arg(X).arg(Y).arg(Z).toAscii().data());
    lesionPosition->update();

}

void pannelloBiopsia::setNeedleLength(void){

    needleLengthLabel->show();
    needleMinLengthLabel->show();
    needleMaxLengthLabel->show();
    needleLength->show();

    QString stringa = QString(QApplication::translate("BIOPSY-PAGE","MIN LENGTH", 0, QApplication::UnicodeUTF8));
    stringa+=QString(":%1(mm)").arg(ApplicationDatabase.getDataI(_DB_BIOP_MIN_AGO));
    needleMinLengthLabel->setPlainText(stringa.toAscii().data());
    needleMinLengthLabel->update();

    stringa = QString(QApplication::translate("BIOPSY-PAGE","MAX LENGTH", 0, QApplication::UnicodeUTF8));
    stringa+=QString(":%1(mm)").arg(ApplicationDatabase.getDataI(_DB_BIOP_MAX_AGO));
    needleMaxLengthLabel->setPlainText(stringa.toAscii().data());
    needleMaxLengthLabel->update();

    stringa = QString(QApplication::translate("BIOPSY-PAGE","SELECTED", 0, QApplication::UnicodeUTF8));
    stringa+=QString(":%1(mm)").arg(ApplicationDatabase.getDataI(_DB_BIOP_AGO));
    needleLength->setPlainText(stringa.toAscii().data());
    needleLength->update();

}



void pannelloBiopsia::setTrx(int angolo){
    if(workflow == _BIOPSY_NEEDLE_LENGTH) return;
    unsigned char buffer[4];
    if(angolo>28) angolo=28;
    else if(angolo<-28) angolo=-28;

    buffer[0]=TRX_MOVE_ANGLE;
    buffer[2] = (unsigned char) angolo;
    buffer[3] = (unsigned char) (angolo>>8);
    pConsole->pGuiMcc->sendFrame(MCC_CMD_TRX,0,buffer, sizeof(buffer));
}


void pannelloBiopsia::setEnableRot(void){
    if(workflow == _BIOPSY_NEEDLE_LENGTH) return;
    if(workflow == _BIOPSY_WAIT_REFERENCE_P15) return;
    if(workflow == _BIOPSY_WAIT_LESION_P15) return;
    if(workflow == _BIOPSY_WAIT_REFERENCE_M15) return;
    if(workflow == _BIOPSY_WAIT_LESION_M15) return;
    if(workflow == _BIOPSY_WRONG_LESION_CALC) return;
    if(workflow == _BIOPSY_CHECK_POSITION) return;


    unsigned char cval;
    unsigned char trxerr,armerr;

    cval=ApplicationDatabase.getDataU(_DB_ENABLE_MOVIMENTI);
    trxerr=ApplicationDatabase.getDataI(_DB_ALLARMI_ALR_TRX);
    armerr=ApplicationDatabase.getDataI(_DB_ALLARMI_ALR_ARM);

    if((!armerr) && (cval&ACTUATOR_STATUS_ENABLE_ROT_FLAG)) armEnabledPix->show();
    else armEnabledPix->hide();

}


void pannelloBiopsia::manageAfterAlarmActions(void){
    int action = actionAfterAlarm;
    actionAfterAlarm = BIOPSY_AFTER_ALARM_NO_ACTION;
    switch(action){

    // Reset della sequenza, del braccio e della torretta
    case BIOPSY_AFTER_ALARM_RESET_SEQUENCE:
        ApplicationDatabase.setData(_DB_ANALOG_BIOPSY_WORKFLOW, (int) _BIOPSY_INIT);
        break;

    default:
        break;
    }

}
void pannelloBiopsia::xrayPixActivation(bool stat){
    if (stat) xrayPix->show();
    else xrayPix->hide();
    return;
}


