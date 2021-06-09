#include "pannelloOpzioni.h"
#include "analog.h"
#include "../application.h"
#include "../appinclude.h"
#include "../globvar.h"
#include "pageOpenAnalogic.h"
extern AnalogPageOpen* paginaOpenStudyAnalogic;

#define SEL_MANUAL_POS 140,91
#define SEL_SEMI_POS 213,91
#define SEL_AUTO_POS 286,91

#define SEL_FILTERMo_POS 140,172
#define SEL_FILTERRh_POS 213,172
#define SEL_FILTERAUTO_POS 286,172

pannelloOpzioni::pannelloOpzioni(QGraphicsView* view){
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

    backgroundPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/pannelloOpzioni.png"));
    backgroundPix->setPos(0,0);

    selButtonTechModePix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/selButton.png"));
    selButtonTechModePix->setPos(0,0);

    selButtonFilterModePix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/selButton.png"));
    selButtonFilterModePix->setPos(0,0);

    selButtonODPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/selButton.png"));
    selButtonODPix->setPos(0,0);

    selButtonTechPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/selButton.png"));
    selButtonTechPix->setPos(0,0);


    font.setPointSize(26);
    font.setStretch(50);
    profileLabel = new GLabel(this,QRectF(162,206,215,46),font,QColor(_W_TEXT)," ---",Qt::AlignCenter);
    indexLabel = new GLabel(this,QRectF(128,206,30,46),font,QColor(_W_TEXT)," ---",Qt::AlignCenter);

    platePix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/film.png"));
    platePix->setOffset(104,215);


    disableOdPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/disable_od.png"));
    disableOdPix->setPos(45,260);
    disableTechPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/disable_tech.png"));
    disableTechPix->setPos(125,337);
    disableFilmPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/disable_film.png"));
    disableFilmPix->setPos(14,203);

    disableFiltroAutoPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/disable_button.png"));
    disableFiltroAutoPix->setPos(284,125);
    disableFiltroMoPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/disable_button.png"));
    disableFiltroMoPix->setPos(138,125);
    disableFiltroRhPix = this->addPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/disable_button.png"));
    disableFiltroRhPix->setPos(211,125);


    timerDisable=0;
    open_flag = false;
    parent->hide();
}

pannelloOpzioni::~pannelloOpzioni()
{
    return;
}

void pannelloOpzioni::timerEvent(QTimerEvent* ev)
{
    if(ev->timerId()==timerDisable)
    {
        killTimer(timerDisable);
        timerDisable=0;
    }

}


void pannelloOpzioni::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mousePressEvent(event);

    if(timerDisable) return;
    timerDisable=startTimer(500);

    QPointF mouse = event->scenePos();

    // Pulsante tecnica Manuale uno zero punti.. solo se esistono profoli
    if(mouse.y()<=107){        
        if((mouse.x()>=140)&&(mouse.x()<=200)){
            // Tecnica Manuale
            analog_conf_changed = true;
            ApplicationDatabase.setData(_DB_TECH_MODE,(int) ANALOG_TECH_MODE_MANUAL);
            ApplicationDatabase.setData(_DB_FILTER_MODE,(int) ANALOG_FILTRO_FISSO,DBase::_DB_NO_CHG_SGN);
            ApplicationDatabase.setData(_DB_SELECTED_FILTER,ApplicationDatabase.getDataI(_DB_PRIMO_FILTRO),DBase::_DB_FORCE_SGN);
        }else{
            if(ApplicationDatabase.getDataI(_DB_NUMERO_PROFILI)==0) return;
            if((mouse.x()>=214)&&(mouse.x()<=273)){
                // Tecnica ad 1 punto
                analog_conf_changed = true;
                ApplicationDatabase.setData(_DB_TECH_MODE,(int) ANALOG_TECH_MODE_SEMI);
            }else if((mouse.x()>=287)&&(mouse.x()<=347)){
                // Tecnica a 0 punti
                analog_conf_changed = true;
                ApplicationDatabase.setData(_DB_TECH_MODE,(int) ANALOG_TECH_MODE_AUTO);
            }
        }

        return;
    }


    // Pulsante selezione filtro
    if(mouse.y()<=190){
        if(ApplicationDatabase.getDataI(_DB_SECONDO_FILTRO)==Collimatore::FILTRO_ND) return;

        if((mouse.x()>=140)&&(mouse.x()<=200)){
            analog_conf_changed = true;
            ApplicationDatabase.setData(_DB_FILTER_MODE,(int) ANALOG_FILTRO_FISSO,DBase::_DB_NO_CHG_SGN);
            ApplicationDatabase.setData(_DB_SELECTED_FILTER,(int) Collimatore::FILTRO_Mo, DBase::_DB_FORCE_SGN);
        }else if((mouse.x()>=214)&&(mouse.x()<=273)){
            analog_conf_changed = true;
            ApplicationDatabase.setData(_DB_FILTER_MODE,(int) ANALOG_FILTRO_FISSO,DBase::_DB_NO_CHG_SGN);
            ApplicationDatabase.setData(_DB_SELECTED_FILTER,(int) Collimatore::FILTRO_Rh, DBase::_DB_FORCE_SGN);
        }else if((mouse.x()>=287)&&(mouse.x()<=347)){
            analog_conf_changed = true;
            ApplicationDatabase.setData(_DB_FILTER_MODE,(int) ANALOG_FILTRO_AUTO);
            ApplicationDatabase.setData(_DB_SELECTED_FILTER,(int) ApplicationDatabase.getDataI(_DB_PRIMO_FILTRO));
        }
        return;
    }


    // Pulsanti selezione profilo
    if(mouse.y()<=258){
        if(ApplicationDatabase.getDataI(_DB_NUMERO_PROFILI)==0) return;
        if(ApplicationDatabase.getDataI(_DB_TECH_MODE)==ANALOG_TECH_MODE_MANUAL) return;

         analog_conf_changed = true;
        // Trigger al master
        if(mouse.x()<=166)
            ApplicationDatabase.setData(_DB_SET_PROFILE,(int) 2,DBase::_DB_FORCE_SGN); // Backward
        else
            ApplicationDatabase.setData(_DB_SET_PROFILE,(int) 1,DBase::_DB_FORCE_SGN); // Forward


        return;
    }

    // Pulsanti selezione OD
    if(mouse.y()<=331){
        if(ApplicationDatabase.getDataI(_DB_NUMERO_PROFILI)==0) return;
        if(ApplicationDatabase.getDataI(_DB_TECH_MODE)==ANALOG_TECH_MODE_MANUAL) return;
        if(ApplicationDatabase.getDataI(_DB_PLATE_TYPE) != ANALOG_PLATE_FILM) return;

        int sel = ((mouse.x() - 50) / 34);
        if(sel>10) sel=10;
        if(sel<0) sel=0;

        ApplicationDatabase.setData(_DB_OD,(int) sel);
        profile_conf_changed = true;
        return;
    }


    // Pulsanti selezione TECH
    if(mouse.y()<=400){
        if(ApplicationDatabase.getDataI(_DB_NUMERO_PROFILI)==0) return;
        if(ApplicationDatabase.getDataI(_DB_TECH_MODE)==ANALOG_TECH_MODE_MANUAL) return;
        if(ApplicationDatabase.getDataI(_DB_PLATE_TYPE) != ANALOG_PLATE_FILM) return;

        if(mouse.x()<189)  ApplicationDatabase.setData(_DB_TECHNIC,(int)ANALOG_TECH_PROFILE_LD);
        else if(mouse.x()>284)  ApplicationDatabase.setData(_DB_TECHNIC,(int)ANALOG_TECH_PROFILE_HC);
        else ApplicationDatabase.setData(_DB_TECHNIC,(int)ANALOG_TECH_PROFILE_STD);
        profile_conf_changed = true;
        return;
    }

    // Tasto OK
    if((mouse.y()>=410)){
        disconnect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)));
        ApplicationDatabase.setData(_DB_CALLBACKS,(int) CALLBACK_OPTIONEXIT_SELECTION ,DBase::_DB_FORCE_SGN);
        return;
    }

}


void pannelloOpzioni::open(){
    if(open_flag) return;
    open_flag = true;
    parent->show();
    timerDisable=startTimer(500);

    analog_conf_changed = false;
    profile_conf_changed = false;


    // Inizializza cursore di selezione tecnica
    if(ApplicationDatabase.getDataI(_DB_TECH_MODE)==ANALOG_TECH_MODE_MANUAL)     selButtonTechModePix->setPos(SEL_MANUAL_POS);
    else if(ApplicationDatabase.getDataI(_DB_TECH_MODE)==ANALOG_TECH_MODE_SEMI)     selButtonTechModePix->setPos(SEL_SEMI_POS);
    else selButtonTechModePix->setPos(SEL_AUTO_POS);

    // Sezione selezione filtro
    if(ApplicationDatabase.getDataI(_DB_FILTER_MODE) == ANALOG_FILTRO_AUTO) // Filtro automatico (quindi ci sono almeno due filtri configurati)
    {
        selButtonFilterModePix->setPos(SEL_FILTERAUTO_POS);
    }else if(ApplicationDatabase.getDataI(_DB_SELECTED_FILTER) == Collimatore::FILTRO_Mo){
        selButtonFilterModePix->setPos(SEL_FILTERMo_POS);
    }else selButtonFilterModePix->setPos(SEL_FILTERRh_POS);


    setEnables();
    setProfile();

    connect(&ApplicationDatabase,SIGNAL(dbDataChanged(int,int)), this,SLOT(valueChanged(int,int)),Qt::UniqueConnection);
}

void pannelloOpzioni::exit(){
    if(!open_flag) return;

    // Operazioni in chiusura
    open_flag = false;
    parent->hide();

}


void pannelloOpzioni::valueChanged(int index,int opt)
{
    if((isMaster)&&(opt&DBase::_DB_ONLY_SLAVE_ACTION)) return;
    if((!isMaster)&&(opt&DBase::_DB_ONLY_MASTER_ACTION)) return;

    switch(index){
    case _DB_TECH_MODE:
        setEnables();
        if(ApplicationDatabase.getDataI(_DB_TECH_MODE)==ANALOG_TECH_MODE_MANUAL)     selButtonTechModePix->setPos(SEL_MANUAL_POS);
        else if(ApplicationDatabase.getDataI(_DB_TECH_MODE)==ANALOG_TECH_MODE_SEMI)     selButtonTechModePix->setPos(SEL_SEMI_POS);
        else selButtonTechModePix->setPos(SEL_AUTO_POS);

        // Aggiorna i dati del profilo in configurazione
        if(isMaster){
            pConfig->analogCnf.tech_mode = ApplicationDatabase.getDataI(_DB_TECH_MODE);
            setExpositionRange(DBase::_DB_NO_CHG_SGN);
        }
        break;

    case _DB_FILTER_MODE:
    case _DB_SELECTED_FILTER:
        if(ApplicationDatabase.getDataI(_DB_FILTER_MODE)==ANALOG_FILTRO_AUTO) selButtonFilterModePix->setPos(SEL_FILTERAUTO_POS);
        else if(ApplicationDatabase.getDataI(_DB_SELECTED_FILTER) == Collimatore::FILTRO_Mo) selButtonFilterModePix->setPos(SEL_FILTERMo_POS);
        else if(ApplicationDatabase.getDataI(_DB_SELECTED_FILTER) == Collimatore::FILTRO_Rh) selButtonFilterModePix->setPos(SEL_FILTERRh_POS);

        // Aggiorna i dati del profilo in configurazione
        if(isMaster){
            pConfig->analogCnf.selected_filtro = ApplicationDatabase.getDataI(_DB_SELECTED_FILTER);
            if(ApplicationDatabase.getDataI(_DB_FILTER_MODE) == ANALOG_FILTRO_AUTO)
                pConfig->analogCnf.auto_filtro_mode = true;
            else pConfig->analogCnf.auto_filtro_mode = false;

            paginaOpenStudyAnalogic->emitQueuedExecution(QUEUED_SELECTED_FILTER,0,""); // Impostazione Filtro
            //pCollimatore->setFiltro((Collimatore::_FilterCmd_Enum)  pConfig->analogCnf.selected_filtro, true);


        }
        break;

    case _DB_TECHNIC:
    case _DB_OD:
        setProfile();

        // Aggiorna i dati del profilo in configurazione
        if(isMaster){
            pGeneratore->pAECprofiles->getCurrentProfilePtr()->odindex = ApplicationDatabase.getDataI(_DB_OD);
            pGeneratore->pAECprofiles->getCurrentProfilePtr()->technic = ApplicationDatabase.getDataI(_DB_TECHNIC);
            setExpositionRange(DBase::_DB_NO_CHG_SGN);
        }
        break;

    case _DB_SET_PROFILE:
        if(ApplicationDatabase.getDataI(index)==0){

            setProfile();// Refresh Profile
            return;
        }

        // Preparazione per il refresh (solo master)
        if(!isMaster) return;

        // Solo il Master esegue la selezione di un nuovo profilo
        stepProfile(ApplicationDatabase.getDataI(index));
        ApplicationDatabase.setData(index,(int) 0); // Comanda il refresh
        setExpositionRange(DBase::_DB_NO_CHG_SGN);

        break;
    }
}

/* _____________________________________________________________________________________________________________________________________
 * Imposta il nuovo profilo cercando il successivo o il precedente rispetto al parametro
 * dir==1 ->Forward
 _____________________________________________________________________________________________________________________________________ */
void pannelloOpzioni::stepProfile(int dir){
    AEC::profileCnf_Str* profilePtr;
    if(!isMaster) return;

    if(dir==1) profilePtr = pGeneratore->pAECprofiles->getNextProfilePtr();
    else profilePtr = pGeneratore->pAECprofiles->getPrevProfilePtr();

    // Reimposta conseguentemente tutti i registri associati
    if(profilePtr==null){
        ApplicationDatabase.setData(_DB_NUMERO_PROFILI,(int) 0,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        return;
    }

    ApplicationDatabase.setData(_DB_PROFILE_INDEX,(int) pConfig->analogCnf.current_profile ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
    ApplicationDatabase.setData(_DB_PROFILE_NAME,profilePtr->symbolicName ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
    ApplicationDatabase.setData(_DB_PLATE_TYPE,(int) profilePtr->plateType ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
    if(profilePtr->plateType == ANALOG_PLATE_FILM){
        ApplicationDatabase.setData(_DB_TECHNIC,(int) profilePtr->technic ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_OD,(int) profilePtr->odindex ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
    }else{
        ApplicationDatabase.setData(_DB_TECHNIC,(int) ANALOG_TECH_PROFILE_LD ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
        ApplicationDatabase.setData(_DB_OD,(int) 5 ,DBase::_DB_FORCE_SGN|DBase::_DB_NO_CHG_SGN);
    }

}

#define OD_SELY_POS 309
static unsigned short od_posx[11]={50,85,119,153,187,221,255,289,323,357,391};
void pannelloOpzioni::setProfile(void){
    setProfileLabel(ApplicationDatabase.getDataS(_DB_PROFILE_NAME));
    setPlate((unsigned char) ApplicationDatabase.getDataI(_DB_PLATE_TYPE));
    setProfileIndex(ApplicationDatabase.getDataI(_DB_PROFILE_INDEX));

    int i=ApplicationDatabase.getDataI(_DB_OD);
    if(i>10||i<0) i=5;
    selButtonODPix->setPos(od_posx[i]+9,OD_SELY_POS);

    if(ApplicationDatabase.getDataI(_DB_TECHNIC)==ANALOG_TECH_PROFILE_LD)
        selButtonTechPix->setPos(128,385);
    else if(ApplicationDatabase.getDataI(_DB_TECHNIC)==ANALOG_TECH_PROFILE_STD)
        selButtonTechPix->setPos(208,385);
    else if(ApplicationDatabase.getDataI(_DB_TECHNIC)==ANALOG_TECH_PROFILE_HC)
        selButtonTechPix->setPos(288,385);


    setEnables();
}


/* _____________________________________________________________________________________________________________________________________
 * Imposta lo stato dei pannelli di disabilitazione in fonzione delle impostazioni correnti
 _____________________________________________________________________________________________________________________________________ */
void pannelloOpzioni::setEnables(void){
    // Campi relativi al profilo corrente _____________________________
    if(ApplicationDatabase.getDataI(_DB_TECH_MODE) == ANALOG_TECH_MODE_MANUAL){

        // Disabilitazioni delle opzioni FILM
        disableOdPix->show();
        selButtonODPix->hide();
        selButtonTechPix->hide();
        disableTechPix->show();
        disableFilmPix->show();
    }else{
        disableFilmPix->hide();

        // In caso di profilo CR disabilita la possibilità di selezionare la tecnica e la densità
        if(ApplicationDatabase.getDataI(_DB_PLATE_TYPE) != ANALOG_PLATE_FILM){
            disableOdPix->show();
            selButtonODPix->hide();
            selButtonTechPix->hide();
            disableTechPix->show();
        }else{
            disableOdPix->hide();
            selButtonODPix->show();
            selButtonTechPix->show();
            disableTechPix->hide();
        }
    }

    // Filtro Automatico/Manuale
    if(ApplicationDatabase.getDataI(_DB_SECONDO_FILTRO)==Collimatore::FILTRO_ND){
        disableFiltroAutoPix->show();
        if(ApplicationDatabase.getDataI(_DB_PRIMO_FILTRO)==Collimatore::FILTRO_Mo){
            disableFiltroMoPix->hide();
            disableFiltroRhPix->show();
        }else{
            disableFiltroMoPix->show();
            disableFiltroRhPix->hide();
        }
    }else{
        disableFiltroAutoPix->hide();
        disableFiltroMoPix->hide();
        disableFiltroRhPix->hide();
    }

}

/* _____________________________________________________________________________________________________________________________________
 * Imposta il campo del nome del profilo correntemente selezionato (se disponibile)
 _____________________________________________________________________________________________________________________________________ */
void pannelloOpzioni::setProfileLabel(QString name){

    profileLabel->setPlainText(QString("%1").arg(name).toAscii().data());
    profileLabel->update();
}

/* _____________________________________________________________________________________________________________________________________
 * Impostazione icona plate
 _____________________________________________________________________________________________________________________________________ */
void pannelloOpzioni::setPlate(unsigned char plateType){

    platePix->show();
    if(plateType==ANALOG_PLATE_FILM) platePix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/film.png"));
    else if(plateType==ANALOG_PLATE_CR) platePix->setPixmap(QPixmap("://paginaOperativaAnalogica/paginaOperativaAnalogica/CR.png"));
    else platePix->hide();

}

/* _____________________________________________________________________________________________________________________________________
 * Impostazione icona plate
 _____________________________________________________________________________________________________________________________________ */
void pannelloOpzioni::setProfileIndex(int index){

    if(index==-1) {
        indexLabel->hide();
        return;
    }
    indexLabel->show();


    indexLabel->setPlainText(QString("[%1]").arg(index).toAscii().data());
    indexLabel->update();

}

// Imposta i range di funzionamento, kV e mAs in funzione
// della modalità e del profilo corrente.
// Questa funzione deve essere lanciata dal Master ad ogni cambio
// di modalita di funzionamento oppure all'apertura della finestra operativa
void pannelloOpzioni::setExpositionRange(int options){

    int min, max;
    if(!isMaster) return;


    AEC::profileCnf_Str* profilePtr = pGeneratore->pAECprofiles->getCurrentProfilePtr();

    // In caso di incongruenze tra modalità e assenza di profilo viene
    // impostato nuovamente il modo manuale
    if((!profilePtr) && (pConfig->analogCnf.tech_mode != ANALOG_TECH_MODE_MANUAL)){
        ApplicationDatabase.setData(_DB_NUMERO_PROFILI,(int) 0);
        pConfig->analogCnf.tech_mode=ANALOG_TECH_MODE_MANUAL;

        // Modalità manuale imposta o selezionata
        ApplicationDatabase.setData(_DB_PROFILE_INDEX,(int) -1 ,options);
        ApplicationDatabase.setData(_DB_PROFILE_NAME,"---" ,options);
        ApplicationDatabase.setData(_DB_PLATE_TYPE,(int) ANALOG_PLATE_UNDEF ,options);
        ApplicationDatabase.setData(_DB_TECHNIC, 255 ,options);
        ApplicationDatabase.setData(_DB_OD,(int) 255 ,options);
        ApplicationDatabase.setData(_DB_TECH_MODE,(int) ANALOG_TECH_MODE_MANUAL ,options);
    }

    // Impostazione kV e range selezionabile da tastierino (se applicabile)
    if(ApplicationDatabase.getDataI(_DB_TECH_MODE) == ANALOG_TECH_MODE_MANUAL){
        min = _MIN_KV*10;
        max = pGeneratore->getMaxKv()*10;
    }else if(ApplicationDatabase.getDataI(_DB_TECH_MODE) == ANALOG_TECH_MODE_SEMI){
        if(profilePtr->plateType == ANALOG_PLATE_FILM){
            min = pConfig->analogCnf.minKvFilm*10;
            max = pConfig->analogCnf.maxKvFilm*10;


            // Correzione per HC/LD/STD
            if(profilePtr->technic == ANALOG_TECH_PROFILE_HC){
                min+=pConfig->analogCnf.DKV_HC*10;
                max+=pConfig->analogCnf.DKV_HC*10;
            }else if(profilePtr->technic == ANALOG_TECH_PROFILE_LD){
                min+=pConfig->analogCnf.DKV_LD*10;
                max+=pConfig->analogCnf.DKV_LD*10;
            }

        }else{
            min = pConfig->analogCnf.minKvCR*10;
            max = pConfig->analogCnf.maxKvCR*10;
        }
    }

    ApplicationDatabase.setData(_DB_MIN_MAX_DKV,(int) ((max<<16)|min),options);

    if( ApplicationDatabase.getDataI(_DB_DKV) > max) ApplicationDatabase.setData(_DB_DKV, (int) max,options);
    else if ( ApplicationDatabase.getDataI(_DB_DKV) < min) ApplicationDatabase.setData(_DB_DKV, (int) min,options);

    // Imposta i mAs e il range di selezione
    min = 10;
    max = pGeneratore->getMaxDMas();
    ApplicationDatabase.setData(_DB_MIN_MAX_DMAS,(int) ((max<<16)|min),options);

    ApplicationDatabase.setData(_DB_CURRENT_MAS_TAB,(int) 0,options); // Imposta il pannello di selezione R20 più basso

    if( ApplicationDatabase.getDataI(_DB_DMAS) > max) ApplicationDatabase.setData(_DB_DMAS, (int) max,options);
    else if ( ApplicationDatabase.getDataI(_DB_DMAS) < min) ApplicationDatabase.setData(_DB_DMAS, (int) min,options);


}
