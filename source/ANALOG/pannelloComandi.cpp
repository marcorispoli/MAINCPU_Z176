#include "pannelloComandi.h"
#include "analog.h"
#include "../application.h"
#include "../appinclude.h"
#include "../globvar.h"
#include "pageOpenAnalogic.h"
extern AnalogPageOpen* paginaOpenStudyAnalogic;
pannelloComandi::pannelloComandi(QGraphicsView* view){

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
    view->setSceneRect(0,0,473,473);

    // Imposta di default il background per rotazione motorizzata
    comandiPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloComandi.png"));
    comandiPix->setOffset(0,0);
    currentBackground = 1;

    readyNotReadyPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/notready.png"));
    readyNotReadyPix->setOffset(166,279);

    campiPix = this->addPixmap(QPixmap("://sym/sym/campi_front.png"));
    campiPix->setOffset(332,193);

    optionsPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/options.png"));
    optionsPix->setOffset(54,186);

    platePix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/film.png"));
    //platePix->setOffset(75,135);
    platePix->setOffset(315,135);


    focusPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/smallFocus.png"));
    focusPix->setOffset(33,304);




    font.setPointSize(120);
    font.setStretch(60);
    masLabel = new GLabel(this,QRectF(160,341,150,100),font,QColor(_W_TEXT),"640",Qt::AlignCenter);


    font.setPointSize(100);
    font.setStretch(60);
    kvLabel = new GLabel(this,QRectF(160,10,150,100),font,QColor(_W_TEXT),"35",Qt::AlignCenter);

    font.setPointSize(16);
    font.setStretch(50);
   // odLabel = new GLabel(this,QRectF(354,132,100,20),font,QColor(_W_TEXT),"+5",Qt::AlignLeft);
   // technicLabel = new GLabel(this,QRectF(354,148,100,20),font,QColor(_W_TEXT),"STD",Qt::AlignLeft);
    odLabel = new GLabel(this,QRectF(379,132,100,20),font,QColor(_W_TEXT),"+5",Qt::AlignLeft);
    technicLabel = new GLabel(this,QRectF(379,147,100,20),font,QColor(_W_TEXT),"STD",Qt::AlignLeft);

    font.setPointSize(20);
    font.setStretch(50);
    techModeLabel = new GLabel(this,QRectF(423,141,20,20),font,QColor(_W_TECH_MODE),"M",Qt::AlignLeft);


    font.setPointSize(26);
    font.setStretch(50);
//    profileLabel = new GLabel(this,QRectF(100,135,150,30),font,QColor(_W_TEXT)," ---",Qt::AlignLeft);
    profileLabel = new GLabel(this,QRectF(75,135,150,30),font,QColor(_W_TEXT)," ---",Qt::AlignLeft);

    font.setPointSize(20);
    font.setStretch(50);
    anodeLabel = new GLabel(this,QRectF(90,315,19,16),font,QColor(_W_TEXT),"Mo",Qt::AlignCenter);

    font.setPointSize(20);
    font.setStretch(50);
    filterLabel = new GLabel(this,QRectF(131,315,19,16),font,QColor(_W_TEXT),"Mo",Qt::AlignCenter);

    font.setPointSize(19);
    font.setStretch(50);
    autoFilterLabel = new GLabel(this,QRectF(116,320,14,10),font,QColor(_W_FLT_MODE),"",Qt::AlignCenter);

    projectionPix = this->addPixmap(QPixmap(QString("://paginaProiezioni/paginaProiezioni/RCC.png")));
    projectionPix->setOffset(184,110);


    timerDisable=0;
    open_flag = false;
    parent->hide();

    // Impostazione vettori proiezioni (solo Master)
    projectionsFiles[PROJ_LCC] = QString("://paginaProiezioni/paginaProiezioni/LCC.png");
    projectionsFiles[PROJ_LMLO]=QString("://paginaProiezioni/paginaProiezioni/LMLO.png");
    projectionsFiles[PROJ_LML]=QString("://paginaProiezioni/paginaProiezioni/LML.png");
    projectionsFiles[PROJ_LISO]=QString("://paginaProiezioni/paginaProiezioni/LISO.png");
    projectionsFiles[PROJ_LFB]=QString("://paginaProiezioni/paginaProiezioni/LFB.png");
    projectionsFiles[PROJ_LSIO]=QString("://paginaProiezioni/paginaProiezioni/LSIO.png");
    projectionsFiles[PROJ_LLM]=QString("://paginaProiezioni/paginaProiezioni/LLM.png");
    projectionsFiles[PROJ_LLMO]=QString("://paginaProiezioni/paginaProiezioni/LLMO.png");
    projectionsFiles[PROJ_RCC] = QString("://paginaProiezioni/paginaProiezioni/RCC.png");
    projectionsFiles[PROJ_RMLO]=QString("://paginaProiezioni/paginaProiezioni/RMLO.png");
    projectionsFiles[PROJ_RML]=QString("://paginaProiezioni/paginaProiezioni/RML.png");
    projectionsFiles[PROJ_RISO]=QString("://paginaProiezioni/paginaProiezioni/RISO.png");
    projectionsFiles[PROJ_RFB]=QString("://paginaProiezioni/paginaProiezioni/RFB.png");
    projectionsFiles[PROJ_RSIO]=QString("://paginaProiezioni/paginaProiezioni/RSIO.png");
    projectionsFiles[PROJ_RLM]=QString("://paginaProiezioni/paginaProiezioni/RLM.png");
    projectionsFiles[PROJ_RLMO]=QString("://paginaProiezioni/paginaProiezioni/RLMO.png");


    wrongProjectionPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/wrongProjection.png"));
    wrongProjectionPix->setOffset(181,107);
    wrongProjectionPix->hide();


    // Stringhe di dose kV e mAs effettivi ultima esposizione
    font.setPointSize(20);
    font.setStretch(50);
    doseLabel = new GLabel(this,QRectF(337,300,100,20),font,QColor(_W_TECH_MODE),"AGD: ----",Qt::AlignLeft);

    font.setPointSize(20);
    font.setStretch(50);
    kvXLabel = new GLabel(this,QRectF(337,321,100,20),font,QColor(_W_TECH_MODE),"kV: ----",Qt::AlignLeft);

    font.setPointSize(20);
    font.setStretch(50);
    mAsXLabel = new GLabel(this,QRectF(337,342,100,20),font,QColor(_W_TECH_MODE),"mAs: ----",Qt::AlignLeft);


    font.setPointSize(30);
    font.setStretch(60);
    //angoloArm = new GLabel(this,QRectF(218,224,54,32),font,QColor(_ORANGE_CUFFIA),"---",Qt::AlignCenter);
    angoloArm = new GLabel(this,QRectF(218,224,54,32),font,QColor(0,0,0),"---",Qt::AlignCenter);
    angoloArm->hide();

    // Questa deve essere l'ultima poichè deve stare sopra tutto
    xrayPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/xrayon.png"));
    xrayPix->setOffset(0,0);


}

pannelloComandi::~pannelloComandi()
{
    return;
}

void pannelloComandi::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId()==timerDisable)
    {
        killTimer(timerDisable);
        timerDisable=0;
    }

}

void pannelloComandi::mousePressEvent(QGraphicsSceneMouseEvent* event)
{

    if(timerDisable){
        QGraphicsScene::mousePressEvent(event);
        return;
    }
    timerDisable=startTimer(500);

    QPointF mouse = event->scenePos();

    // Selezione proiezione se abilitato
    if((mouse.x()>=185)&&(mouse.x()<=308)&&(mouse.y()>=116)&&(mouse.y()<=260)){
        // Solo con rotazione motorizzata
        if(ApplicationDatabase.getDataI(_DB_ROT_MODE))   ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_COMANDI_PROJ_SELECTION ,DBase::_DB_FORCE_SGN);
    }else if(kvLabel->boundingRect().contains(event->scenePos())){
        // Apertura pannello kV solo se non si trova in automatismo 0 punti!
        if(ApplicationDatabase.getDataI(_DB_TECH_MODE)==ANALOG_TECH_MODE_AUTO) return;
        ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_COMANDI_KV_SELECTION ,DBase::_DB_FORCE_SGN);

    }else if(masLabel->boundingRect().contains(event->scenePos())){
         if(ApplicationDatabase.getDataI(_DB_TECH_MODE)==ANALOG_TECH_MODE_AUTO) return;
        ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_COMANDI_MAS_SELECTION ,DBase::_DB_FORCE_SGN);
    }else if(campiPix->boundingRect().contains(event->scenePos())) stepCampi();
    else if(optionsPix->boundingRect().contains(event->scenePos())) ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_COMANDI_OPTION_SELECTION ,DBase::_DB_FORCE_SGN);
    else    QGraphicsScene::mousePressEvent(event);
}

/* _____________________________________________________________________________________________________________________________________
 * Chiamato per aprire il pannello comandi
   - ncampi:       ANALOG_AECFIELD_FRONT,
                   ANALOG_AECFIELD_MIDDLE,
                   ANALOG_AECFIELD_BACK
 _____________________________________________________________________________________________________________________________________ */
void pannelloComandi::open(void){
    int flags;

    // Impostazione nuovo background se necessario

    if(ApplicationDatabase.getDataI(_DB_ROT_MODE) !=  currentBackground){
        currentBackground = ApplicationDatabase.getDataI(_DB_ROT_MODE);

        if(currentBackground){
            angoloArm->hide();
            comandiPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloComandi.png"));
        }else{
           comandiPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloComandiRotManuale.png"));
           angoloArm->show();
        }
    }
    if(open_flag) return;
    open_flag = true;
    parent->show();
    xrayPix->hide();
    config_changed = false;

    // Acquisizione dei flags
    flags = ApplicationDatabase.getDataI(_DB_ANALOG_FLAGS);
    setTechMode(ApplicationDatabase.getDataI(_DB_TECH_MODE));
    setKv(ApplicationDatabase.getDataI(_DB_DKV));
    setdmAs(ApplicationDatabase.getDataI(_DB_DMAS));
    setCampi(ApplicationDatabase.getDataI(_DB_CAMPI));
    setFuoco(ApplicationDatabase.getDataI(_DB_CURRENT_FUOCO));
    setFilterField(ApplicationDatabase.getDataI(_DB_SELECTED_FILTER),ApplicationDatabase.getDataI(_DB_FILTER_MODE));
    setAnode(ApplicationDatabase.getDataS(_DB_ANODE_NAME));
    setReady(flags&_DB_ANFLG_EXP_READY);
    setProjectionPix();


    // Aggiorna i campi del pannello profilo
    if(ApplicationDatabase.getDataI(_DB_NUMERO_PROFILI)==0){
        setProfileLabel("   ---   ");
        setPlate(255);
        setTechnic(255);
        setOd(255);
    }else{
        setProfileLabel(ApplicationDatabase.getDataS(_DB_PROFILE_NAME));
        setPlate((unsigned char) ApplicationDatabase.getDataI(_DB_PLATE_TYPE));
        setTechnic(ApplicationDatabase.getDataI(_DB_TECHNIC));
        setOd(ApplicationDatabase.getDataI(_DB_OD));
    }

    timerDisable=startTimer(500);
    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
}

void pannelloComandi::exit(){
    if(!open_flag) return;

    // Operazioni in chiusura
    disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
    open_flag = false;
    parent->hide();

}


void pannelloComandi::stepCampi(void){
    int campi = ApplicationDatabase.getDataI(_DB_CAMPI);

    if(campi==ANALOG_AECFIELD_FRONT) campi = ANALOG_AECFIELD_CENTER;
    else if(campi==ANALOG_AECFIELD_CENTER) campi = ANALOG_AECFIELD_BACK;
    else campi = ANALOG_AECFIELD_FRONT;

    ApplicationDatabase.setData(_DB_CAMPI,campi);
}

/* _____________________________________________________________________________________________________________________________________
 *Imposta il campo del campo dell'AEC selezionato.
   - ncampi:       ANALOG_AECFIELD_FRONT,
                   ANALOG_AECFIELD_MIDDLE,
                   ANALOG_AECFIELD_BACK
 _____________________________________________________________________________________________________________________________________ */
void pannelloComandi::setCampi(unsigned char ncampi){
    if(ncampi==ANALOG_AECFIELD_FRONT) campiPix->setPixmap(QPixmap("://Sym/Sym/campi_front.png"));
    else if(ncampi==ANALOG_AECFIELD_CENTER) campiPix->setPixmap(QPixmap("://Sym/Sym/campi_center.png"));
    else  campiPix->setPixmap(QPixmap("://Sym/Sym/campi_back.png"));

    //parent->update();
}


/* _____________________________________________________________________________________________________________________________________
 * Imposta il campo dei kV (val = kv*10)
 _____________________________________________________________________________________________________________________________________ */
void pannelloComandi::setKv(int val){
    if(ApplicationDatabase.getDataI(_DB_TECH_MODE)==ANALOG_TECH_MODE_AUTO) kvLabel->setPlainText(QString("-").toAscii().data());
    else kvLabel->setPlainText(QString("%1").arg((float) val /10).toAscii().data());
    kvLabel->update();
}


/* _____________________________________________________________________________________________________________________________________
 * Imposta il campo dei mAs
 _____________________________________________________________________________________________________________________________________ */
void pannelloComandi::setdmAs(int dmAs){
    if(ApplicationDatabase.getDataI(_DB_TECH_MODE)!=ANALOG_TECH_MODE_MANUAL) masLabel->setPlainText(QString("-").toAscii().data());
    else masLabel->setPlainText(QString("%1").arg(((float)dmAs)/10).toAscii().data());
    masLabel->update();
}


/* _____________________________________________________________________________________________________________________________________
 * Imposta il campo dei mAs
 _____________________________________________________________________________________________________________________________________ */
void pannelloComandi::setArm(int angolo){
    angoloArm->setPlainText(QString("%1").arg(angolo).toAscii().data());
    angoloArm->update();
}

/* _____________________________________________________________________________________________________________________________________
 * Imposta il campo del nome del profilo correntemente selezionato (se disponibile)
 _____________________________________________________________________________________________________________________________________ */
void pannelloComandi::setProfileLabel(QString name){

    profileLabel->setPlainText(QString("%1").arg(name).toAscii().data());
    profileLabel->update();
}

/* _____________________________________________________________________________________________________________________________________
 * Impostazione icona plate
 _____________________________________________________________________________________________________________________________________ */
void pannelloComandi::setPlate(unsigned char plateType){

    platePix->show();
    if(plateType==ANALOG_PLATE_FILM) platePix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/film.png"));
    else if(plateType==ANALOG_PLATE_CR) platePix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/CR.png"));
    else platePix->hide();

}

/* _____________________________________________________________________________________________________________________________________
 * Impostazione OD
 _____________________________________________________________________________________________________________________________________ */
void pannelloComandi::setOd(int od){

    if(od==255) odLabel->setPlainText(QString("").arg(od).toAscii().data());
    else if(od>5) odLabel->setPlainText(QString("+%1").arg(od-5).toAscii().data());
    else if(od<5) odLabel->setPlainText(QString("%1").arg(od-5).toAscii().data());
    else odLabel->setPlainText(QString("0").toAscii().data());

    odLabel->update();
}


/* _____________________________________________________________________________________________________________________________________
 * Impostazione Technic
 _____________________________________________________________________________________________________________________________________ */
void pannelloComandi::setTechnic(int tech){

    if(tech==ANALOG_TECH_PROFILE_STD) technicLabel->setPlainText(QString("STD").toAscii().data());
    else if(tech==ANALOG_TECH_PROFILE_HC) technicLabel->setPlainText(QString("HC").toAscii().data());
    else if(tech==ANALOG_TECH_PROFILE_LD) technicLabel->setPlainText(QString("LD").toAscii().data());
    else technicLabel->setPlainText(QString("").toAscii().data());

    technicLabel->update();
}

/* _____________________________________________________________________________________________________________________________________
 * Impostazione Tech mode
 _____________________________________________________________________________________________________________________________________ */
void pannelloComandi::setTechMode(int tm){

    if(tm==ANALOG_TECH_MODE_MANUAL) techModeLabel->setPlainText(QString("M").toAscii().data());
    else if(tm==ANALOG_TECH_MODE_SEMI) techModeLabel->setPlainText(QString("1").toAscii().data());
    else if(tm==ANALOG_TECH_MODE_AUTO) techModeLabel->setPlainText(QString("0").toAscii().data());
    else techModeLabel->setPlainText(QString("").toAscii().data());

    techModeLabel->update();
}

/* _____________________________________________________________________________________________________________________________________
 * Impostazione proiezione
 _____________________________________________________________________________________________________________________________________ */
void pannelloComandi::setProjectionPix(void){

    int index = ApplicationDatabase.getDataI(_DB_SELECTED_PROJECTION);
    if(index>16) return;

    projectionPix->setPixmap(QPixmap(projectionsFiles[index]));
}

/* _____________________________________________________________________________________________________________________________________
 * Impostazione Fuoco
 _____________________________________________________________________________________________________________________________________ */
void pannelloComandi::setFuoco(int fuoco){
    if(fuoco==Generatore::FUOCO_SMALL) focusPix->show();
    else focusPix->hide();
}

/* _____________________________________________________________________________________________________________________________________
 * Impostazione Campo filtri
 _____________________________________________________________________________________________________________________________________ */
void pannelloComandi::setFilterField(int filtro, int automode){
    if(filtro==Collimatore::FILTRO_Mo) filterLabel->setPlainText(QString("Mo").toAscii().data());
    else if(filtro==Collimatore::FILTRO_Rh)  filterLabel->setPlainText(QString("Rh").toAscii().data());
    else  filterLabel->setPlainText(QString("?").toAscii().data());

    if(automode == ANALOG_FILTRO_AUTO) autoFilterLabel->setPlainText(QString("A").toAscii().data());
    else         autoFilterLabel->setPlainText(QString("").toAscii().data());
}

/* _____________________________________________________________________________________________________________________________________
 * Impostazione Campo Anodo
 _____________________________________________________________________________________________________________________________________ */
void pannelloComandi::setAnode(QString anodo){
    anodeLabel->setPlainText(anodo.toAscii().data());
}

/* _____________________________________________________________________________________________________________________________________
 * Impostazione Campo Ready Push Button
 _____________________________________________________________________________________________________________________________________ */
void pannelloComandi::setReady(bool ready){
    if(ready)   readyNotReadyPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/ready.png"));
    else readyNotReadyPix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/notready.png"));
}



void pannelloComandi::valueChanged(int index,int opt)
{
    if((isMaster)&&(opt&DBase::_DB_ONLY_SLAVE_ACTION)) return;
    if((!isMaster)&&(opt&DBase::_DB_ONLY_MASTER_ACTION)) return;

    switch(index){
    case _DB_DKV:
        setKv(ApplicationDatabase.getDataI(_DB_DKV));
        break;
    case _DB_DMAS:
        setdmAs(ApplicationDatabase.getDataI(_DB_DMAS));
        break;

    case  _DB_CAMPI:
        setCampi(ApplicationDatabase.getDataI(_DB_CAMPI));

        // Impostazione campo sull'esposimetro
        if(isMaster) {
            pPotter->setDetectorField(ApplicationDatabase.getDataI(index));
            pConfig->analogCnf.aec_field = ApplicationDatabase.getDataI(index);
            config_changed = true;
        }
        break;
    case _DB_PROFILE_NAME:
        setProfileLabel(ApplicationDatabase.getDataS(_DB_PROFILE_NAME));
        break;
    case _DB_PLATE_TYPE:
        setPlate((unsigned char) ApplicationDatabase.getDataI(_DB_PLATE_TYPE));
        break;

    case _DB_TECHNIC:
         setTechnic(ApplicationDatabase.getDataI(_DB_TECHNIC));
        break;

    case _DB_OD:
        setOd(ApplicationDatabase.getDataI(_DB_OD));
        break;

    case _DB_TECH_MODE:
        setTechMode(ApplicationDatabase.getDataI(_DB_TECH_MODE));
        break;

    case _DB_CURRENT_FUOCO:
        setFuoco(ApplicationDatabase.getDataI(_DB_CURRENT_FUOCO));
        break;

    case _DB_FILTER_MODE:
    case _DB_SELECTED_FILTER:
        setFilterField(ApplicationDatabase.getDataI(_DB_SELECTED_FILTER),ApplicationDatabase.getDataI(_DB_FILTER_MODE));
        break;
    case _DB_ANODE_NAME:
        setAnode(ApplicationDatabase.getDataS(_DB_ANODE_NAME));
        break;

    case _DB_ANALOG_FLAGS:        
        setReady(ApplicationDatabase.getDataI(index) & _DB_ANFLG_EXP_READY);
        if(ApplicationDatabase.getDataI(index) & _DB_WRONG_PROJECTION) wrongProjectionPix->show();
        else wrongProjectionPix->hide();
        break;

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
    }

}


void pannelloComandi::xrayPixActivation(bool stat){
    if (stat) xrayPix->show();
    else xrayPix->hide();
    return;
}


