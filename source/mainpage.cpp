#define MAINPAGE_C
#include "application.h"
#include "appinclude.h"
#include "globvar.h"

#include "audio.h"
extern audio* pAudio;

#include "ANALOG/pageOpenAnalogic.h"
extern AnalogPageOpen* paginaOpenStudyAnalogic;

#define LOGO_POS   0,423
#define LOGO_SIZE  250,60

MainPage::MainPage(bool local, QString bgl, QString bgs , bool showLogo, int w,int h, qreal angolo,QPainterPath pn, int pgpn, QPainterPath pp, int pgpp, int pg) : GWindow(bgl,showLogo,w,h, angolo,pn,pgpn,pp,pgpp,pg)
{
    QFont font;


    nextPageEnabled = false;
    prevPageEnabled = false;
    // setBackground("://MainPage/MainPage/background.png");

    timerId=0;
    timerPowerOffButton = 0;

    // Definizione del testo per la data
    font.setFamily("DejaVu Serif");
    font.setBold(true);
    font.setWeight(75);
    font.setItalic(false);
    font.setPointSize(22);
    font.setStretch(60);

    dateText=this->addText("----------",font);
    dateText->setDefaultTextColor(Qt::white);
    dateText->setPos(DATE_LABEL_POSITION);

    // Campo Intestazione
    font.setPointSize(30);
    font.setStretch(40);
    intestazioneValue = new GLabel(this,QRectF(120,8,533,44 ),font,QColor(_W_TEXT),"",Qt::AlignCenter);


    // Definizione del testo per la data
    font.setPointSize(40);
    font.setStretch(40);
    // Campo ANGOLO
    angoloValue = new GLabel(this,QRectF(358,254,87,66),font,QColor(_W_TEXT),"0");
    // Campo TILT
    tiltValue = new GLabel(this,QRectF(358,163,87,66),font,QColor(_W_TEXT),"0");



     // Label campo accessorio
    font.setPointSize(16);
    font.setStretch(40);
    accessoryLabel = new GLabel(this,QRectF(29,93,190,20),font,QColor(_W_TEXT),"ACCESSORIO",Qt::AlignLeft);
    compressorLabel = new GLabel(this,QRectF(29,176,190,20),font,QColor(_W_TEXT),"COMPRESSORE",Qt::AlignLeft);
    positionLabel= new GLabel(this,QRectF(35,258,180,20),font,QColor(_W_TEXT),"POSIZIONE",Qt::AlignLeft);
    spessoreLabel= new GLabel(this,QRectF(35,258,180,20),font,QColor(_W_TEXT),"SPESSORE",Qt::AlignRight);
    forceLabel= new GLabel(this,QRectF(35,340,180,20),font,QColor(_W_TEXT),"FORZA",Qt::AlignLeft);
    targetLabel= new GLabel(this,QRectF(35,340,180,20),font,QColor(_W_TEXT),"TARGET",Qt::AlignRight);


    font.setPointSize(23);
    font.setStretch(40);
    accessoryValue = new GLabel(this,QRectF(29,112,190,24),font,QColor(_W_TEXT),"",Qt::AlignLeft);
    compressorValue = new GLabel(this,QRectF(29,195,190,24),font,QColor(_W_TEXT),"",Qt::AlignLeft);
    positionValue= new GLabel(this,QRectF(35,277,180,24),font,QColor(_W_TEXT),"",Qt::AlignLeft);
    spessoreValue= new GLabel(this,QRectF(35,277,180,24),font,QColor(_W_TEXT),"",Qt::AlignRight);
    forceValue= new GLabel(this,QRectF(35,359,180,24),font,QColor(_W_TEXT),"",Qt::AlignLeft);
    targetValue= new GLabel(this,QRectF(35,359,180,24),font,QColor(_W_TEXT),"",Qt::AlignRight);


    pulsanteRotazioni = new GPush((GWindow*) this,setPointPath(8,269,249,533,249,533,389,269,389),269,249,0,0,FALSE);
    pulsanteTilt = new GPush((GWindow*) this,setPointPath(8,334,97,465,97,465,224,334,224),334,97,0,0,FALSE);
    pulsantePowerOff= new GPush((GWindow*) this,setPointPath(8,692,0,800,0,800,115,692,115),692,0,0,0,FALSE);
    pulsanteToolsOn = new GPush((GWindow*) this,setPointPath(8,0,0,120,0,120,120,0,120),0,0,0,0,FALSE);
    pulsanteOperatingMode = new GPush((GWindow*) this,setPointPath(8,539,279,662,279,6662,363,539,362),539,279,0,0,FALSE);


    acPresent = this->addPixmap(QPixmap("://MainPage/MainPage/ac.png"));
    acPresent->setPos(726,133);
    font.setPointSize(18);
    font.setStretch(40);
    acLabel = new GLabel(this,QRectF(754,161,42,20),font,QColor("black"),"",Qt::AlignLeft);

    battPix = this->addPixmap(QPixmap());
    battPix->setPos(721,206);

    closedDoorPix = this->addPixmap(QPixmap("://MainPage/MainPage/closed_door_pix.png"));
    closedDoorPix->setPos(718,242);

    awsPresent = this->addPixmap(QPixmap("://MainPage/MainPage/aws.png"));
    awsPresent->setPos(727,374);
    awsPresent->hide();



    rotDisabledPix = addPixmap(QPixmap("://MainPage/MainPage/rotdisabled.png"));
    rotDisabledPix->setPos(257,89);
    tiltDisabledPix = addPixmap(QPixmap("://MainPage/MainPage/penddisabled.png"));
    tiltDisabledPix->setPos(257,89);
    deadmanPix = this->addPixmap(QPixmap("://MainPage/MainPage/deadman.png"));
    deadmanPix->setPos(468,290);
    demoPix = this->addPixmap(QPixmap("://MainPage/MainPage/demo.png"));
    demoPix->setPos(733,309);


    // Creazione della pixmap per il logo
    logoPix = addPixmap(QPixmap("/resource/config/logo.png"));
    logoPix->setPos(LOGO_POS);
    logoPix->show();


    // Pulsante apertura menu allarmi
    pulsanteAlarmOn = new GPush((GWindow*) this, QPixmap("://MainPage/MainPage/alrMsg.png"),QPixmap("://MainPage/MainPage/alrMsg.png"),setPointPath(8,550,125,657,125,657,216,550,216),550,125,0,0);
    pulsanteAlarmOn->setEnable(true);
    font.setPointSize(20);
    font.setStretch(50);
    alarmNum = new GLabel(this,QRectF(618,186,42,20),font,QColor(_R_COL),"",Qt::AlignLeft);


    panelTool = this->addPixmap(QPixmap());
    panelTool->setPos(0,0);
    panelTool->hide();
    pulsanteCancRot = new GPush((GWindow*) this,setPointPath(8,91,167,203,167,203,343,91,343),91,167,10,-1,FALSE);
    pulsanteOkRot = new GPush((GWindow*) this,setPointPath(8,354,194,455,194,455,293,354,293),354,194,10,255,FALSE);
    pulsanteRot0 = new GPush((GWindow*) this, 0,QPixmap("://MainPage/MainPage/Selezione0.png"),0,setPointPath(8,370,67,437,67,437,139,370,139),216,48,10,0,FALSE,FALSE,TRUE);
    pulsanteRot45 = new GPush((GWindow*) this, 0,QPixmap("://MainPage/MainPage/Selezione45.png"),0,setPointPath(8,457,144,505,97,555,148,506,195),216,48,10,45,FALSE,FALSE,TRUE);
    pulsanteRot90 = new GPush((GWindow*) this, 0,QPixmap("://MainPage/MainPage/Selezione90.png"),0,setPointPath(8,510,209,576,208,577,279,507,279),216,48,10,90,FALSE,FALSE,TRUE);
    pulsanteRot135 = new GPush((GWindow*) this, 0,QPixmap("://MainPage/MainPage/Selezione135.png"),0,setPointPath(8,503,274,551,340,505,391,458,339),216,48,10,135,FALSE,FALSE,TRUE);
    pulsanteRot_45 = new GPush((GWindow*) this, 0,QPixmap("://MainPage/MainPage/Selezione_45.png"),0,setPointPath(8,257,143,305,96,351,143,303,192),216,48,10,-45,FALSE,FALSE,TRUE);
    pulsanteRot_90 = new GPush((GWindow*) this, 0,QPixmap("://MainPage/MainPage/Selezione_90.png"),0,setPointPath(8,232,211,299,211,300,279,230,281),216,48,10,-90,FALSE,FALSE,TRUE);
    pulsanteRot_135 = new GPush((GWindow*) this, 0,QPixmap("://MainPage/MainPage/Selezione_135.png"),0,setPointPath(8,258,338,308,294,355,338,307,387),216,48,10,-135,FALSE,FALSE,TRUE);
    pulsanteRotP = new GPush((GWindow*) this, 0,QPixmap("://MainPage/MainPage/SelezioneP.png"),0,setPointPath(8,370,346,436,346,436,417,370,417),216,48,10,200,FALSE,FALSE,TRUE);


    // Pannello Power Off
    pulsanteCancPowerOff= new GPush((GWindow*) this,setPointPath(8,194,212,316,212,316,344,194,344),194,212,0,0,FALSE);
    pulsanteOkPowerOff= new GPush((GWindow*) this,setPointPath(8,452,212,586,212,586,344,452,344),452,212,0,0,FALSE);
    font.setPointSize(30);
    font.setStretch(40);
    powerOffLabel = new GLabel(this,QRectF(185,113,413,91),font,QColor(_W_TEXT),"",Qt::AlignCenter);

    timerId = startTimer(1000);
    disableTimedButtons = false; // Abilitazione pulsanti

    // Connessioni

    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
    connect(pagina_language,SIGNAL(changeLanguageSgn()), this,SLOT(languageChanged()),Qt::UniqueConnection);
    connect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)), this,SLOT(buttonActivationNotify(int,bool,int)),Qt::UniqueConnection);


    pulsanteAudioMute = new GPush((GWindow*) this, QPixmap("://MainPage/MainPage/audioOn.png"),QPixmap("://MainPage/MainPage/audioMuted.png"),setPointPath(8,730,365,800,365,800,400,730,400),730,365,0,0);
    pulsanteAudioMute->setEnable(true);
    pulsanteAudioMute->setVisible(false);

}

MainPage::~MainPage()
{
    this->killTimer(timerId);
   // this->killTimer(timerWDG);
}

// Questa funzione viene chiamata ogni volta che viene ricevuto il segnale di cambio
// pagina dalla Classe Base. Viene utilizzata per effettuare tutte le inizializzazioni del caso
void MainPage::childStatusPage(bool stat,int opt)
{
    if(stat==false){
        if(timerDisableButton) {
            killTimer(timerDisableButton);
            timerDisableButton = 0;
        }
        disableTimedButtons = false;
        disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));

        return;
    }


    disableButtons(2000); // Disabilita i pulsanti in ingresso

    // Assegna la password da utilizzare per il pannello di service
    if(isMaster){
        ApplicationDatabase.setData(_DB_PASSWORD,pConfig->userCnf.ServicePassword);
    }
    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);

    changePannello(_MAIN_PANEL);
    paginaAllarmi->alarm_enable=true;

}

void MainPage::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId()==timerDisableButton)
    {
        disableTimedButtons = false;
        killTimer(timerDisableButton);
        timerDisableButton=0;
        return;
    }

    if(ev->timerId()==timerId)
    {

        if(systemTimeUpdated){
            if(isMaster){
                pConfig->year = QDateTime::currentDateTime().toString("yyyy").toInt();
                pConfig->month = QDateTime::currentDateTime().toString("MM").toInt();
                pConfig->day = QDateTime::currentDateTime().toString("dd").toInt();
                pConfig->hour = QDateTime::currentDateTime().toString("hh").toInt();
                pConfig->min = QDateTime::currentDateTime().toString("mm").toInt();
                pConfig->sec = QDateTime::currentDateTime().toString("ss").toInt();
            }
            dateText->setPlainText(QDateTime::currentDateTime().toString("dd.MM.yy     hh.mm.ss ap"));
        }else{
            dateText->setPlainText(QString("--.--.--     --.--.--"));
        }

    }

    // Solo il master
    if(ev->timerId()==timerPowerOffButton)
    {
        killTimer(timerPowerOffButton);
        timerPowerOffButton=0;
        pConfig->activatePowerOff();
        return;
    }

}


void MainPage::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    // Ogni azione di interazione TS viene filtrata per evitare rimbalzi
    if(disableTimedButtons) return;
    GWindow::mousePressEvent(event); // Lancia il default della classe
    disableTimedButtons=true;
    timerDisableButton = startTimer(500);
}

void MainPage::setBattPix(unsigned char val){
    static unsigned char oldval=255;

    val = (val+10) / 25;
    if(val==oldval) return;
    oldval=val;

    switch(val){
    case 4:
        battPix->show();
        battPix->setPixmap(QPixmap("://MainPage/MainPage/batt100.png"));
        break;
    case 3:
        battPix->show();
        battPix->setPixmap(QPixmap("://MainPage/MainPage/batt75.png"));
        break;
    case 2:
        battPix->show();
        battPix->setPixmap(QPixmap("://MainPage/MainPage/batt50.png"));
        break;
    case 1:
        battPix->show();
        battPix->setPixmap(QPixmap("://MainPage/MainPage/batt25.png"));
        break;
    case 0:
        battPix->hide();
        break;
    }

}

// FUNZIONE DI AGGIORNAMENTO CAMPI VALORE CONNESSO AI CAMPI DEL DATABASE
void MainPage::valueChanged(int index,int opt)
{
    int val;

    switch(index)
    {

    case _DB_REQ_POWEROFF:
        if(!isMaster) return;
        if(ApplicationDatabase.getDataU(index)) pConfig->activatePowerOff();
        break;

    case _DB_VPRIMARIO:
    case _DB_ACVOLT:
        if(ApplicationDatabase.getDataI(_DB_ACVOLT)==0){
            acPresent->hide();
            acLabel->hide();
        }else{
            acPresent->show();
            val = ApplicationDatabase.getDataI(_DB_VPRIMARIO);
            if(val){
                val = ApplicationDatabase.getDataI(_DB_ACVOLT) * val /2200;
                acLabel->labelText = QString("%1").arg(val);
            }else acLabel->labelText = QString("---");
            acLabel->show();
            acLabel->update();
        }
        break;

    case _DB_BATTCHARGE:
        setBattPix(ApplicationDatabase.getDataU(_DB_BATTCHARGE));
        break;
    case _DB_TRX:
        tiltValue->labelText = getTiltString();
        tiltValue->update();
        break;
    case _DB_COMPRESSOR_POSITION:
        positionValue->labelText= QString("%1 (mm)").arg(ApplicationDatabase.getDataI(_DB_COMPRESSOR_POSITION));
        positionValue->update();
        break;
    case _DB_COMPRESSOR_PAD:
        compressorValue->labelText=ApplicationDatabase.getDataS(_DB_COMPRESSOR_PAD);
        compressorValue->update();
        break;
    case _DB_DANGOLO:
        angoloValue->labelText = getArmString();
        angoloValue->update();
        break;
    case _DB_SPESSORE:
        spessoreValue->labelText= QString("%1 (mm)").arg(ApplicationDatabase.getDataI(_DB_SPESSORE));
        spessoreValue->update();
       break;
    case _DB_ACCESSORY_NAME:
        accessoryValue->labelText=ApplicationDatabase.getDataS(_DB_ACCESSORY_NAME);
        accessoryValue->update();
        break;
    case _DB_FORZA:
        forceValue->labelText= QString("%1 (N)").arg(ApplicationDatabase.getDataI(_DB_FORZA));
        forceValue->update();
        break;


    case _DB_NALLARMI_ATTIVI:
        if(ApplicationDatabase.getDataU(index)==0){
            pulsanteAlarmOn->setVisible(false);
            alarmNum->hide();
        }else{
            pulsanteAlarmOn->setVisible(true);
            alarmNum->show();
            alarmNum->labelText = QString("%1").arg(ApplicationDatabase.getDataU(index));
            alarmNum->update();
        }

        break;

    case _DB_TARGET_FORCE:
        targetValue->labelText= QString("%1 (N)").arg(ApplicationDatabase.getDataI(_DB_TARGET_FORCE));
        targetValue->update();
        break;


    case _DB_AUDIO_PRESENT:

        if(ApplicationDatabase.getDataU(_DB_AUDIO_PRESENT)==0) pulsanteAudioMute->setVisible(false);
        else pulsanteAudioMute->setVisible(true);

        break;


    case _DB_CLOSED_DOOR:
        if(ApplicationDatabase.getDataU(_DB_CLOSED_DOOR)){
            closedDoorPix->hide();
        }else{
            closedDoorPix->show();
        }
        break;

    case _DB_ALLARMI_ALR_ARM:
    case _DB_ALLARMI_ALR_TRX:
    case _DB_ENABLE_MOVIMENTI:
    case _DB_DEAD_MEN:
        setRotGroupEnaView();
        break;
    case _DB_DEMO_MODE:
        if(ApplicationDatabase.getDataU(_DB_DEMO_MODE)){
            demoPix->hide();
        }else{
            demoPix->show();
        }
        break;
    }

}


void MainPage::buttonActivationNotify(int id, bool status,int opt)
{

    GPush* pbutton = (GPush*) GWindowRoot.pushList.at(id);
    if(pbutton->parentWindow!=this) return; // Scarta i segnali da altre pagine

    switch(pannello){
    case _MAIN_PANEL:
        if(pbutton==pulsanteRotazioni){

            // Se la rotazione non è configurata allora NON mostra MAI il pannello di rotazioni
            if(!(ApplicationDatabase.getDataU(_DB_SYSTEM_CONFIGURATION)&_ARCH_ARM_MOTOR)) return;

            changePannello(_ROT_PANEL);
            return;
        }
        if(pbutton==pulsanteTilt){            
            changePannello(_TILT_PANEL);
            return;
        }
        if(pbutton==pulsantePowerOff){
            changePannello(_PWROFF_PANEL);
            return;
        }


        if((isMaster)&&(pbutton==pulsanteToolsOn)){
             GWindowRoot.setNewPage(_PG_SERVICE_MENU,GWindowRoot.curPage,0);
        }


        if((isMaster)&&(pbutton==pulsanteAlarmOn)){
            paginaAllarmi->openAlarmPage();
            return;
        }

        if((isMaster)&&(pbutton==pulsanteAudioMute)){
            if(opt==DBase::_DB_NO_ACTION) return;
            pAudio->setMute(status);
            return;
        }

        // Pulsante di apertura studio Analogico (Solo per macchine analogiche!)
        if((isMaster)&&(pbutton==pulsanteOperatingMode)){
            paginaOpenStudyAnalogic->openPageRequest();
            return;
        }
        break;

    case _ROT_PANEL:
        if(pbutton==pulsanteCancRot){
            selRotAngolo=-1;
            changePannello(_MAIN_PANEL);
            return;
        }
        if(pbutton==pulsanteOkRot){
            activateRot(selRotAngolo);
            changePannello(_MAIN_PANEL);
            return;
        }

        selRotAngolo=pbutton->pulsanteData;

        break;
    case _TILT_PANEL:
        if(pbutton==pulsanteCancRot){
            selTiltAngolo=-1;
            changePannello(_MAIN_PANEL);
            return;

        }
        if(pbutton==pulsanteOkRot){
            activateTilt(selTiltAngolo);
            changePannello(_MAIN_PANEL);
            return;
        }

        selTiltAngolo=pbutton->pulsanteData;
        break;

    case _PWROFF_PANEL:                
        if((isMaster) && (pbutton==pulsanteOkPowerOff))
            timerPowerOffButton = startTimer(100);

        changePannello(_MAIN_PANEL);
        break;
    }


}



// Rinfresca tutte le label cambiate
void MainPage::languageChanged()
{

    setWindowUpdate();

}

QString MainPage::getArmString(void){

    // Angolo in decimi di grado
    int angolo = ApplicationDatabase.getDataI(_DB_DANGOLO);

    // Approssima a+-0.5
    int intero = angolo/10;
    int decimale = angolo-intero*10;

    if(decimale>5) return QString("%1").arg(intero+1);
    else return QString("%1").arg(intero);
}

QString MainPage::getTiltString(void){

    // Angolo in decimi di grado
    int angolo = ApplicationDatabase.getDataI(_DB_TRX);

    // Approssima a+-0.5
    int intero = angolo/10;
    int decimale = angolo-intero*10;

    if(decimale>5) return QString("%1").arg(intero+1);
    else return QString("%1").arg(intero);
}

void MainPage::setRotGroupEnaView(void){
    unsigned char cval;
    unsigned char trxerr,armerr;

    if(ApplicationDatabase.getDataU(_DB_DEAD_MEN)){
        pulsanteRotazioni->setVisible(false);
        pulsanteTilt->setVisible(false);
        rotDisabledPix->show();
        tiltDisabledPix->show();
        deadmanPix->show();
    }else{
        deadmanPix->hide();
        cval=ApplicationDatabase.getDataU(_DB_ENABLE_MOVIMENTI);
        trxerr=ApplicationDatabase.getDataI(_DB_ALLARMI_ALR_TRX);
        armerr=ApplicationDatabase.getDataI(_DB_ALLARMI_ALR_ARM);

        if(ApplicationDatabase.getDataU(_DB_SYSTEM_CONFIGURATION)&_ARCH_ARM_MOTOR){

            if(armerr){
                // Errori sulla rotazione
                pulsanteRotazioni->setVisible(false);
                rotDisabledPix->show();
           }else if(cval&ACTUATOR_STATUS_ENABLE_ROT_FLAG){
                pulsanteRotazioni->setVisible(true);
                rotDisabledPix->hide();
            }else{
                pulsanteRotazioni->setVisible(false);
                rotDisabledPix->show();
            }
        }else{
            pulsanteRotazioni->setVisible(false);
            rotDisabledPix->show();
        }

        // Se il TRX non è configurato non viene MAI visto il pulsante
        if(ApplicationDatabase.getDataU(_DB_SYSTEM_CONFIGURATION)&_ARCH_TRX_MOTOR){
            if(trxerr){
                // Errori sulla pendolazione
                pulsanteTilt->setVisible(false);
                tiltDisabledPix->show();
            }else if(cval&ACTUATOR_STATUS_ENABLE_PEND_FLAG){
                pulsanteTilt->setVisible(true);
                tiltDisabledPix->hide();
            }else{
                pulsanteTilt->setVisible(false);
                tiltDisabledPix->show();
            }
        }else{
            pulsanteTilt->setVisible(false);
            tiltDisabledPix->show();
        }
    }
}

// Funzione chiamata solo per il cambio pannello
void MainPage::changePannello(int newpanel){

    setBackground("://MainPage/MainPage/backgroundAnalogic.png");
    pulsanteOperatingMode->setVisible(true);

    // Controllo sulla correttezza della configurazione
   if(isMaster){
       pConfig->testConfigError(true,false);

       // Controllo sulla fermata dello starter
       if(pGeneratore->timerStarter){
           pGeneratore->stopStarterSlot();
       }
   }

    pannello=newpanel;

    studyColor = QColor(_W_TEXT);


    // Attivazione Pulsanti relativi ai pannelli
    pulsanteRotazioni->setVisible(false);
    pulsanteTilt->setVisible(false);
    pulsantePowerOff->setVisible(false);
    pulsanteToolsOn->setVisible(false);

    pulsanteCancRot->setVisible(false);
    pulsanteRot0->setVisible(false);
    pulsanteRot45->setVisible(false);
    pulsanteRot90->setVisible(false);
    pulsanteRot135->setVisible(false);
    pulsanteRot_45->setVisible(false);
    pulsanteRot_90->setVisible(false);
    pulsanteRot_135->setVisible(false);
    pulsanteRotP->setVisible(false);
    pulsanteOkRot->setVisible(false);

    pulsanteCancPowerOff->setVisible(false);
    pulsanteOkPowerOff->setVisible(false);
    powerOffLabel->setVisible(false);

    switch(pannello){
    case _MAIN_PANEL:
            pulsantePowerOff->setVisible(true);
            pulsanteToolsOn->setVisible(true);
    break;

    case _ROT_PANEL:
        selRotAngolo = -1;
        panelTool->setPixmap(QPixmap("://MainPage/MainPage/frameRotazioni.png"));
        panelTool->show();
        pulsanteCancRot->setVisible(true);
        pulsanteRot0->setVisible(true);
        pulsanteRot135->setVisible(true);
        pulsanteRot_135->setVisible(true);
        pulsanteRotP->setVisible(true);
        pulsanteOkRot->setVisible(true);

        pulsanteRot45->setVisible(true);
        pulsanteRot45->pulsanteData = 45;
        pulsanteRot90->setVisible(true);
        pulsanteRot90->pulsanteData=90;
        pulsanteRot_45->setVisible(true);
        pulsanteRot_45->pulsanteData=-45;
        pulsanteRot_90->setVisible(true);
        pulsanteRot_90->pulsanteData=-90;
        break;
    case _TILT_PANEL:
        panelTool->setPixmap(QPixmap("://MainPage/MainPage/frameTilt.png"));
        panelTool->show();
        pulsanteCancRot->setVisible(true);
        pulsanteOkRot->setVisible(true);
        pulsanteRot0->setVisible(true);
        pulsanteRot45->setVisible(true);
        pulsanteRot45->pulsanteData = 15;
        pulsanteRot90->setVisible(true);
        pulsanteRot90->pulsanteData=20;
        pulsanteRot_45->setVisible(true);
        pulsanteRot_45->pulsanteData=-15;
        pulsanteRot_90->setVisible(true);
        pulsanteRot_90->pulsanteData=-20;
        break;
    case _PWROFF_PANEL:
        panelTool->setPixmap(QPixmap("://MainPage/MainPage/framePowerOff.png"));
        panelTool->show();
        pulsanteCancPowerOff->setVisible(true);
        pulsanteOkPowerOff->setVisible(true);
        powerOffLabel->setVisible(true);
        break;
    }

    setWindowUpdate();
}

void MainPage::setWindowUpdate(void)
{
    int val;

    switch(pannello){
    case _MAIN_PANEL:

        if(ApplicationDatabase.getDataU(_DB_NALLARMI_ATTIVI)==0){
            pulsanteAlarmOn->setVisible(false);
            alarmNum->hide();
        }else{
            pulsanteAlarmOn->setVisible(true);
            alarmNum->show();
            alarmNum->labelText = QString("%1").arg(ApplicationDatabase.getDataU(_DB_NALLARMI_ATTIVI));
            alarmNum->update();
        }


        if(ApplicationDatabase.getDataI(_DB_ACVOLT)==0){
            acPresent->hide();
            acLabel->hide();
        }else{
            acPresent->show();
            val = ApplicationDatabase.getDataI(_DB_VPRIMARIO);
            if(val){
                val = ApplicationDatabase.getDataI(_DB_ACVOLT) * val /2200;
                acLabel->labelText = QString("%1").arg(val);
            }else acLabel->labelText = QString("---");
            acLabel->show();
            acLabel->update();
        }


        setBattPix(ApplicationDatabase.getDataU(_DB_BATTCHARGE));


        if(ApplicationDatabase.getDataU(_DB_CLOSED_DOOR)){
            closedDoorPix->hide();
        }else{
            closedDoorPix->show();
        }


        if(ApplicationDatabase.getDataU(_DB_DEMO_MODE)){
            demoPix->hide();
        }else{
            demoPix->show();
        }


        awsPresent->hide();
        if(ApplicationDatabase.getDataU(_DB_AUDIO_PRESENT)==0)  pulsanteAudioMute->setVisible(false);
        else{
            if(ApplicationDatabase.getDataU(_DB_AUDIO_MUTE)==0)   pulsanteAudioMute->activate(false,DBase::_DB_NO_ACTION);
            else pulsanteAudioMute->activate(true,DBase::_DB_NO_ACTION);
            pulsanteAudioMute->setVisible(true);
            pulsanteAudioMute->setEnable(true);
        }


        intestazioneValue->labelText=QApplication::translate("MAIN-PAGE","INTESTAZIONE");
        intestazioneValue->update();

        accessoryLabel->labelText=QApplication::translate("MAIN-PAGE","ACCESSORIO");
        accessoryLabel->update();

        compressorLabel->labelText=QApplication::translate("MAIN-PAGE","COMPRESSORE");
        compressorLabel->update();

        positionLabel->labelText=QApplication::translate("MAIN-PAGE","POSIZIONE");
        positionLabel->update();
        spessoreLabel->labelText=QApplication::translate("MAIN-PAGE","SPESSORE");
        spessoreLabel->update();

        forceLabel->labelText=QApplication::translate("MAIN-PAGE","FORZA");
        forceLabel->update();
        targetLabel->labelText=QApplication::translate("MAIN-PAGE","TARGET");
        targetLabel->update();


        accessoryValue->labelText=ApplicationDatabase.getDataS(_DB_ACCESSORY_NAME);
        accessoryValue->update();
        compressorValue->labelText=ApplicationDatabase.getDataS(_DB_COMPRESSOR_PAD);
        compressorValue->update();

        positionValue->labelText= QString("%1 (mm)").arg(ApplicationDatabase.getDataI(_DB_COMPRESSOR_POSITION));
        positionValue->update();

        spessoreValue->labelText= QString("%1 (mm)").arg(ApplicationDatabase.getDataI(_DB_SPESSORE));
        spessoreValue->update();

        forceValue->labelText= QString("%1 (N)").arg(ApplicationDatabase.getDataI(_DB_FORZA));
        forceValue->update();
        targetValue->labelText= QString("%1 (N)").arg(ApplicationDatabase.getDataI(_DB_TARGET_FORCE));
        targetValue->update();


        angoloValue->labelText = getArmString();
        angoloValue->update();

        tiltValue->labelText = getTiltString();
        tiltValue->update();

        panelTool->hide();

        // Enable rotazioni da TS
        setRotGroupEnaView();

        break;
    case _ROT_PANEL:

        break;
    case _TILT_PANEL:

        break;
    case _PWROFF_PANEL:
        powerOffLabel->labelText = QApplication::translate("MAIN-PAGE","MESSAGGIO ATTIVAZIONE POWER OFF");
        powerOffLabel->update();
        break;
    }

}




//_________________________________________________________________________________
// FUNZIONI ESECUTIVE
//_________________________________________________________________________________

// ____________________________________________
// Attivazione della rotazione motorizzata
// ____________________________________________

void MainPage::activateRot(int angolo)
{
    if(!isMaster) return;

    if(angolo!=200){
        if(angolo > 180) angolo = 180;
        else if(angolo<-180) angolo = -180;
    }

    // Impostazione Parametro
    unsigned char buffer[2];
    buffer[0] =(unsigned char) (angolo&0xFF);
    buffer[1] =(unsigned char) (angolo>>8);

    pConsole->pGuiMcc->sendFrame(MCC_CMD_ARM,0,buffer, 2);
}

// ____________________________________________
// Attivazione della rotazione TRX
// ____________________________________________
void MainPage::activateTilt(int angolo)
{
    if(!isMaster) return;
    unsigned char buffer[4];

    if(angolo>28) angolo=28;
    else if(angolo<-28) angolo=-28;

    buffer[0]=TRX_MOVE_ANGLE;
    buffer[2] = (unsigned char) angolo;
    buffer[3] = (unsigned char) (angolo>>8);

    pConsole->pGuiMcc->sendFrame(MCC_CMD_TRX,0,buffer, sizeof(buffer));

}


