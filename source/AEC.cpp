#include "application.h"
#include "appinclude.h"
#include "globvar.h"
#include "generatore.h"
#include "collimatore.h"

AEC::AEC(QString tubeDir)
{
    parentDirectory = tubeDir + QString("PROFILES/");
    aec_configured = openAECProfiles();
    return;
}


bool AEC::isCurrentProfileCalibrated(int filtro, int size){
    AEC::profileCnf_Str* pProfile = getCurrentProfilePtr();
    if(pProfile==null) return false;

    if( filtro == (int) Collimatore::FILTRO_Mo){
        if(size == (int) Generatore::FUOCO_LARGE){
            if(pProfile->pulseStd_Mo_G.size()) return true;
        }else{
            if(pProfile->pulseStd_Mo_P.size()) return true;
        }
    }else if(filtro == (int) Collimatore::FILTRO_Rh){
        if(size == (int) Generatore::FUOCO_LARGE){
            if(pProfile->pulseStd_Rh_G.size()) return true;
        }else{
            if(pProfile->pulseStd_Rh_P.size()) return true;
        }
    }

    return false;

}

// Verifica se il profilo corrente è calibrato per la combinazione filtro/fuoco
bool AEC::isCurrentProfileCalibrated(QString filtro, int size){
    AEC::profileCnf_Str* pProfile = getCurrentProfilePtr();
    if(pProfile==null) return false;

    if(filtro=="Mo"){
        if(size == (int) Generatore::FUOCO_LARGE){
            if(pProfile->pulseStd_Mo_G.size()) return true;
        }else{
            if(pProfile->pulseStd_Mo_P.size()) return true;
        }
    }else if(filtro=="Rh"){
        if(size == (int) Generatore::FUOCO_LARGE){
            if(pProfile->pulseStd_Rh_G.size()) return true;
        }else{
            if(pProfile->pulseStd_Rh_P.size()) return true;
        }
    }

    return false;
}

// Restituisce il PLOG minimo del profilo selezionato
int AEC::getMinPlog(void){
    if(profileList.size()==0) return 0;
    if(pConfig->analogCnf.current_profile>=profileList.size()) return 0;
    return profileList[pConfig->analogCnf.current_profile].plog_minimo;
}

// Restituisce il nome
QString AEC::getCurrentProfileSymName(void){
    if(profileList.size()==0) return "";
    if(pConfig->analogCnf.current_profile>=profileList.size()) return "";
    return profileList[pConfig->analogCnf.current_profile].symbolicName;
}

// Restituisce il puntatore al profilo correntemente selezionato
AEC::profileCnf_Str* AEC::getCurrentProfilePtr(void){
    if(profileList.size()==0) return (profileCnf_Str*)  null;
    if(pConfig->analogCnf.current_profile>=profileList.size()) return (profileCnf_Str*)  null;
    return &(profileList[pConfig->analogCnf.current_profile]);
}

int AEC::getNumProfiles(void){
    return profileList.size();
}

// Restituisce il puntatore al profilo correntemente selezionato
AEC::profileCnf_Str* AEC::getNextProfilePtr(void){
    if(profileList.size()==0) return (profileCnf_Str*)  null;
    pConfig->analogCnf.current_profile++;
    if(pConfig->analogCnf.current_profile>=profileList.size()) pConfig->analogCnf.current_profile=0;
    return &profileList[pConfig->analogCnf.current_profile];
}

// Restituisce il puntatore al profilo correntemente selezionato
AEC::profileCnf_Str* AEC::getPrevProfilePtr(void){
    if(profileList.size()==0) return (profileCnf_Str*)  null;
    if(pConfig->analogCnf.current_profile==0)  pConfig->analogCnf.current_profile=profileList.size()-1;
    else pConfig->analogCnf.current_profile--;
    return &profileList[pConfig->analogCnf.current_profile];
}

AEC::profileCnf_Str* AEC::selectProfile(unsigned char index){
    if(profileList.size()==0) return (profileCnf_Str*)  null;
    if(index >= profileList.size()) return (profileCnf_Str*)  null;
    pConfig->analogCnf.current_profile = index;    
    return &profileList[pConfig->analogCnf.current_profile];
}

// Selezione il profilo da nome e restituisce l'indice della lista
int AEC::selectProfile(QString name){
    if(profileList.size()==0) return -1;

    for(int i=0; i<profileList.size(); i++){
        if(profileList[i].symbolicName == name){
            PRINT(profileList[i].symbolicName);
            return i;
        }
    }
    return -1;

}


bool AEC::ordinaProfili(QList<profilePoint_Str>* ptr){
    profilePoint_Str p;

    if((*ptr).size()==1) return true;
    for(int i=1; i< (*ptr).size(); i++){
        if((*ptr)[i-1].plog > (*ptr)[i].plog){
            p = (*ptr)[i-1];
            (*ptr)[i-1]=(*ptr)[i];
            (*ptr)[i] = p;
            return false;
        }
    }

    return true;
}

AEC::profileCnf_Str AEC::getProfile(QString filename){
    profileCnf_Str profile;
    profile.filename ="";

    QList<QString> dati;
    QFile file(parentDirectory+filename+".prf");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return profile;

    // Inizializzazione vettori
    profile.pulseStd_Mo_P.clear();
    profile.pulseStd_Mo_G.clear();
    profile.pulseStd_Rh_P.clear();
    profile.pulseStd_Rh_G.clear();
    profile.odindex = 5;
    profile.od[0]=50;
    profile.od[1]=60;
    profile.od[2]=70;
    profile.od[3]=80;
    profile.od[4]=90;
    profile.od[5]=100;
    profile.od[6]=110;
    profile.od[7]=120;
    profile.od[8]=130;
    profile.od[9]=140;
    profile.od[10]=150;
    profile.plog_minimo = 10;
    profile.plog_threshold = 100;
    profile.technic = ANALOG_TECH_PROFILE_STD;
    profile.note = "NA";

    // Legge la prima riga che vale come stringa di note
    if(file.atEnd()){
        file.close();
        return profile;
    }

    // Lettura prima riga, deputata a raccogliere i commenti
    QByteArray  frame = file.readLine();
    profile.note = QString(frame);

    while(1)
    {
        dati = Config::getNextArrayFields(&file);
        if(dati.isEmpty()) break;

        if(dati.size()==2){
            if(dati.at(0)=="NAME")   profile.symbolicName = dati.at(1);
            else if(dati.at(0)=="TECHNIC"){
                if(dati.at(1)=="STD")  profile.technic = ANALOG_TECH_PROFILE_STD;
                else if(dati.at(1)=="HC")  profile.technic = ANALOG_TECH_PROFILE_HC;
                else if(dati.at(1)=="LD")  profile.technic = ANALOG_TECH_PROFILE_LD;
                else profile.technic = ANALOG_TECH_PROFILE_STD;
            }else if(dati.at(0)=="PLATE"){
                if(dati.at(1)=="FILM")  profile.plateType = ANALOG_PLATE_FILM;
                else if(dati.at(1)=="CR")  profile.plateType = ANALOG_PLATE_CR;
            }else if(dati.at(0)=="PLOG_TH"){
                profile.plog_threshold = dati.at(1).toInt();
            }else if(dati.at(0)=="PLOG_MIN"){
                profile.plog_minimo = dati.at(1).toInt();
            }else if(dati.at(0)=="ODINDEX"){
                profile.odindex = dati.at(1).toInt();
            }
        }else if(dati.size() == 5){
            // < Mo/Rh, P/G, PLOG, PULSE, MM>
            int plog = dati.at(2).toInt();
            int pulse = dati.at(3).toInt();
            int mm = dati.at(4).toInt();
            if(pulse<=0) continue;
            if(plog>MAX_PLOG) plog = MAX_PLOG;

            profilePoint_Str point;
            point.plog = plog;
            point.pulse = pulse;
            point.pmmi = mm;
            if(dati.at(0)=="Mo"){
                if(dati.at(1)=="P") profile.pulseStd_Mo_P.append(point);
                else profile.pulseStd_Mo_G.append(point);
            }else if(dati.at(0)=="Rh"){
                if(dati.at(1)=="P") profile.pulseStd_Rh_P.append(point);
                else profile.pulseStd_Rh_G.append(point);
            }
        }else if(dati.size() == 12){
            // < OD,.....>
             if(dati.at(0)=="OD") {
                 profile.od[5] = 100;
                 for(int i=0; i<11; i++){
                     if(i==5) profile.od[i]=100;
                     else if((i<5)&&(dati.at(1+i).toInt()>=100)){
                         profile.od[i]=100;
                     }else if((i>5)&&(dati.at(1+i).toInt()<=100)){
                         profile.od[i]=100;
                     }else if((i>5)&&(dati.at(1+i).toInt()>200)){
                         profile.od[i]=200;
                     }else profile.od[i] =  dati.at(1+i).toInt();
                 }

             }
        }
    }
    file.close();


     // Ordinamento profili acquisiti dal plog piccolo al grande
    while(!ordinaProfili(&profile.pulseStd_Mo_P)) ; // Continua fino a completo ordinamento
    while(!ordinaProfili(&profile.pulseStd_Rh_P)) ; // Continua fino a completo ordinamento
    while(!ordinaProfili(&profile.pulseStd_Mo_G)) ; // Continua fino a completo ordinamento
    while(!ordinaProfili(&profile.pulseStd_Rh_G)) ; // Continua fino a completo ordinamento
    profile.filename = parentDirectory+filename+".prf";
    profile.basefilename = filename;
    return profile;

}

// Salva un profilo su file ed aggiorna se necessario la struttura in memoria
void AEC::saveProfile(AEC::profileCnf_Str profile){
    QFile file;
    QString frame;

    profile.filename = parentDirectory + profile.basefilename + ".prf";
    file.setFileName(profile.filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;


    frame = profile.note + "\n";
    file.write(frame.toAscii().data());

    frame = QString("<NAME,%1>  \n").arg(profile.symbolicName);
    file.write(frame.toAscii().data());

    if(profile.technic==ANALOG_TECH_PROFILE_STD)
        frame = QString("<TECHNIC,STD>  \n");
    else if(profile.technic==ANALOG_TECH_PROFILE_LD)
        frame = QString("<TECHNIC,LD>  \n");
    else if(profile.technic==ANALOG_TECH_PROFILE_HC)
        frame = QString("<TECHNIC,HC>  \n");
    else frame = QString("<TECHNIC,STD>  \n");
    file.write(frame.toAscii().data());

    if(profile.plateType==ANALOG_PLATE_FILM)
        frame = QString("<PLATE,FILM>  \n");
    else if(profile.plateType==ANALOG_PLATE_CR)
        frame = QString("<PLATE,CR>  \n");
    file.write(frame.toAscii().data());

    frame = QString("<PLOG_TH,%1>  \n").arg(profile.plog_threshold);
    file.write(frame.toAscii().data());

    frame = QString("<PLOG_MIN,%1>  \n").arg(profile.plog_minimo);
    file.write(frame.toAscii().data());

    frame = QString("<ODINDEX,%1>  \n").arg(profile.odindex);

    // Salvataggio OD
    frame = QString("<OD,%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11>  \n").arg(profile.od[0])\
            .arg(profile.od[1])\
            .arg(profile.od[2])\
            .arg(profile.od[3])\
            .arg(profile.od[4])\
            .arg(profile.od[5])\
            .arg(profile.od[6])\
            .arg(profile.od[7])\
            .arg(profile.od[8])\
            .arg(profile.od[9])\
            .arg(profile.od[10]);
    file.write(frame.toAscii().data());

    // Salvataggio delle calibrazioni
    QList<profilePoint_Str>* ptr = &profile.pulseStd_Mo_P;
    for(int j=0; j < (*ptr).size(); j++){
        frame = QString("<Mo,P,%1,%2,%3>  \n").arg((*ptr)[j].plog).arg((*ptr)[j].pulse).arg((*ptr)[j].pmmi);
        file.write(frame.toAscii().data());
    }

    ptr = &profile.pulseStd_Mo_G;
    for(int j=0; j < (*ptr).size(); j++){
        frame = QString("<Mo,G,%1,%2,%3>  \n").arg((*ptr)[j].plog).arg((*ptr)[j].pulse).arg((*ptr)[j].pmmi);
        file.write(frame.toAscii().data());
    }

    ptr = &profile.pulseStd_Rh_P;
    for(int j=0; j < (*ptr).size(); j++){
        frame = QString("<Rh,P,%1,%2,%3>  \n").arg((*ptr)[j].plog).arg((*ptr)[j].pulse).arg((*ptr)[j].pmmi);
        file.write(frame.toAscii().data());
    }

    ptr = &profile.pulseStd_Rh_G;
    for(int j=0; j < (*ptr).size(); j++){
        frame = QString("<Rh,G,%1,%2,%3>  \n").arg((*ptr)[j].plog).arg((*ptr)[j].pulse).arg((*ptr)[j].pmmi);
        file.write(frame.toAscii().data());
    }

    file.close();
    file.flush();

    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());

    return;

}

void AEC::eraseProfile(QString filename){

    if(filename=="default_cr") return;
    if(filename=="default_film") return;

    QFile file;
    file.setFileName(parentDirectory + filename +".prf");
    file.remove();

    QList<QString> newValidList;

    for(int i=0; i<validList.size(); i++){
        if(validList[i]!=filename) newValidList.append(validList[i]);
    }
    if(validList.size()!=newValidList.size()){
        validList = newValidList;
        saveValidList(validList);
    }
}

void AEC::eraseAllProfiles(void){
    QFile file;

    // Carica la lista di file profilo presenti nel tubo
    QFileInfoList list = getProfileList();
    if(!list.size()) return ;

    // Cancella tutti i prf ad eccezione dei default
    for(int i=0; i<list.size(); i++){
        if(list[i].baseName()=="default_cr")   continue;
        if(list[i].baseName()=="default_film")   continue;
        file.setFileName(list[i].absoluteFilePath());
        file.remove();
    }

    validList.clear();
    saveValidList(validList);

}



bool AEC::openAECProfiles(void){
    profileCnf_Str profile;
    profileList.clear();
    validList.clear();

    // Carica la lista di file profilo presenti nel tubo
    QFileInfoList list = getProfileList();
    if(!list.size()) return false;

    unsigned char current_profile = pConfig->analogCnf.current_profile;

    // Cerca il file di validazione se esiste
    getValidList();

    // Lettura profili  disponibili
    for (int j = 0; j < list.size(); j++) {
        QFileInfo fileInfo = list.at(j);

        // Verifica se appartiene alla lista di validazione
        bool validated=false;
        if(!validList.size()) validated = true;
        else{
            for(int idx=0; idx<validList.size();idx++){
                if(validList.at(idx)==fileInfo.baseName()){
                    validated = true;
                    break;
                }
            }
        }
        if(!validated) continue;
        profile = getProfile(fileInfo.baseName());
        if(profile.filename!="")  profileList.append(profile);

    } // for profiles

    // Controllo sulla presenza di profili validi
    if(profileList.size()==0){
        pConfig->analogCnf.tech_mode  = ANALOG_TECH_MODE_MANUAL;
        pConfig->analogCnf.current_profile=0;
        if(current_profile != pConfig->analogCnf.current_profile) pConfig->saveAnalogConfig();
        return false;
    }


    // Effettua l'ordinamento secondo la lista di validazione (se necessario)
    if(validList.size()){
        QList<profileCnf_Str> orderedProfileList;
        orderedProfileList.clear();

        for(int vdx=0; vdx < validList.size(); vdx++){
            for(int ldx=0; ldx < profileList.size(); ldx++){
                if(profileList[ldx].basefilename == validList[vdx]){
                    orderedProfileList.append(profileList[ldx]);
                    PRINT(QString("PROFILO:")+profileList[ldx].basefilename + "  ");
                    break;
                }
            }
        }
        profileList = orderedProfileList;
    }

    // Prima di proseguire, per evitare che vi siano problemi di disallineamento tra il numero dei
    // profili e il profilo salvato, occorre fare un test di integrità:
    if(pConfig->analogCnf.current_profile>=profileList.size()){
        pConfig->analogCnf.current_profile=0;
    }
    if(current_profile != pConfig->analogCnf.current_profile) pConfig->saveAnalogConfig();
    return true;
}

//__________________________________________________________________________________
// SALVATAGGIO DEI PROFILI IN MEMORIA
//__________________________________________________________________________________
bool AEC::saveSelectedProfile(void){

    AEC::profileCnf_Str* pProfile = getCurrentProfilePtr();
    if(pProfile)  saveProfile(*pProfile);
    return true;
}

// Restituisce la lista dei descrittori dei files di tipo profilo
// contenuti nel Tubo correntemente selezionato.
QFileInfoList AEC::getProfileList(void){
    QFileInfoList list;

    QDir directory(parentDirectory);
    if(!directory.exists()) return list;

    directory.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    QStringList filtro;
    filtro.append(QString("*.prf"));
    list = directory.entryInfoList(filtro);
    return list;
}

// Restituisce il contenuto del file di ordinamento e validazione
// Le stringhe contenute si riferiscono ai nomi simbolici salvati
// nei relativi files profilo (NON quindi al nome del file medesimo)
void AEC::getValidList(void){
    QList<QString> dati;
    validList.clear();

    QString filename = parentDirectory + QString("list.lst");
    QFile file(filename);
    if(!file.exists()) {
        validList.append("default_cr");
        validList.append("default_film");
        return ;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        validList.append("default_cr");
        validList.append("default_film");
        return ;
    }

    while(1)
    {
        dati = Config::getNextArrayFields(&file);
        if(dati.isEmpty()) break;

        if(dati.at(0)=="LIST"){
            for(int i=0; i<dati.size()-1;i++){
                if(dati.at(i+1)=="default_cr") continue;
                if(dati.at(i+1)=="default_film") continue;
                validList.append(dati.at(i+1));
            }
            break;
        }
    }

    validList.append("default_cr");
    validList.append("default_film");
    file.close();
    return ;
}

void AEC::eraseValidList(void){

    QString filename = parentDirectory + QString("list.lst");
    QFile file(filename);
    if(!file.exists()) return;
    file.remove();
    return ;
}

void AEC::saveValidList(QList<QString> list){
    QString filename = parentDirectory + QString("list.lst");
    QFile file(filename);
    if(!list.size()){
        file.remove();
        return;
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QString frame;

    frame = QString("<LIST");
    for(int i=0; i<list.size(); i++){
        frame.append(QString(",%1").arg(list.at(i)));
    }
    frame.append("> \n");
    file.write(frame.toAscii().data());

    file.close();
    file.flush();

    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());

    return ;
}

// Aggiunge un file alla lista e salva la lista
void AEC::addFileToValidList(QString filename){

    // Si intende che la lista già contenga i default alla fine
    if(validList.size()<3){
        validList.clear();
        validList.append(filename);
        validList.append("default_cr");
        validList.append("default_film");
        saveValidList(validList);
        return;
    }

    QList<QString> newList;
    for(int i=0; i<validList.size()-2; i++){
        if(validList[i]==filename) return; // Già presente
        newList.append(validList[i]);
    }
    newList.append(filename);
    newList.append("default_cr");
    newList.append("default_film");
    validList = newList;
    saveValidList(validList);
    return;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Funzione che restituisce i valori nominali da utilizzare per l'impulso
/*  MODO 0:
 *  DA PLOG + TECHMODE + plateType          -> KV
 *                                          -> mAs (massimi)
 *  DA PLOG e PLOG_THRESHOLD                -> FILTRO
 *  PLOG + FILTRO + FUOCO                   -> PULSE_PRE_OD
 *  PULSE0 + tipo_cassetta, corretto OD     -> PULSE
 *________________________________________________________________________
 * KV: PLOG=160 -> KV = 24 (BASSA DENSITA)
 *     PLOG = 0 ->KV = 29  (ALTA DENSITA)
 *
 * TECHMODE = ANALOG_TECH_PROFILE_STD kv+0
 * TECHMODE = ANALOG_TECH_PROFILE_HC  kv-2
 * TECHMODE = ANALOG_TECH_PROFILE_LD  kv+2
 *
 * ERRORI:
 *  -1 = nessun profilo
 *  -3 = OD index fuori range
 *  -4 = PLOG MINIMO
 *  -5 = Nessun impulso disponibile
 *  -6 = Impulsi uguali a 0
*/

int AEC::getAecData(int plog, int modo_filtro, int selected_filtro, int odindex, int techmode, QString selectedAnodo, int selectedFSize, int* filtro,float* kV, int* dmAs, int* pulses){

    profileCnf_Str* profilePtr = getCurrentProfilePtr();
    if(profilePtr==null)        return -1;   // Nessun profilo caricato
    if(odindex>10)                   return -3;   // Indice di selezione od fuori range
    if( plog < profilePtr->plog_minimo)     return -4;   // PLOG troppo basso (seno troppo denso)


    // Calcolo kV da utilizzare
    if(plog>=MAX_PLOG) plog=MAX_PLOG;
    *kV = 29 - (float) plog * 5 / MAX_PLOG ;

    // Correzione kV secondo tecnica desiderata: il CR vuole sempre Low Dose
    if(profilePtr->plateType==ANALOG_PLATE_CR) *kV +=  2;
    else{
        if(techmode==ANALOG_TECH_PROFILE_HC) *kV -= 2;
        else if(techmode==ANALOG_TECH_PROFILE_LD) *kV += 2;

    }

    // Assegnazione mAs in relazione ai kV selezionati
    *dmAs = pGeneratore->getMaxDMas(*kV,selectedAnodo, selectedFSize);

    // Assegnazione filtro
    if(modo_filtro == ANALOG_FILTRO_FISSO){
        *filtro = selected_filtro; // Filtro attualmente selezionato
    }else{
        if(pConfig->analogCnf.secondo_filtro != Collimatore::FILTRO_ND){
            if( plog < profilePtr->plog_threshold ) *filtro = pConfig->analogCnf.secondo_filtro;
            else *filtro = pConfig->analogCnf.primo_filtro;
        }else *filtro = pConfig->analogCnf.primo_filtro;
    }

    // Lettura impulso assegnato
    QList<profilePoint_Str>* ptr ;
    if(selectedFSize == Generatore::FUOCO_LARGE){
        if(*filtro == Collimatore::FILTRO_Mo) ptr = &profilePtr->pulseStd_Mo_G;
        else ptr = &profilePtr->pulseStd_Rh_G;
    }else{
        if(*filtro == Collimatore::FILTRO_Mo) ptr = &profilePtr->pulseStd_Mo_P;
        else ptr = &profilePtr->pulseStd_Rh_P;
    }

    // Controlla se ci sono punti disponibili
    if(ptr==null) return -5;
    if(ptr->size() < 2) return -5;


    bool pulseFound=false;
    if(plog<=(*ptr)[0].plog){
        int i0 = 0;
        int i1 = 1;
        float k = ((float) (*ptr)[i1].pulse - (float) (*ptr)[i0].pulse) / ((float) (*ptr)[i1].plog - (float) (*ptr)[i0].plog);
        *pulses = (*ptr)[i0].pulse - k * (float) ((*ptr)[i0].plog-plog);
        pulseFound=true;
    }else if(plog >= (*ptr)[(*ptr).size()-1].plog){
        int i0 = (*ptr).size()-2;
        int i1 = (*ptr).size()-1;
        float k = ((float)(*ptr)[i1].pulse - (float)(*ptr)[i0].pulse) / ((float)(*ptr)[i1].plog - (float)(*ptr)[i0].plog);
        *pulses = (*ptr)[i1].pulse + k * (float)(plog-(*ptr)[i1].plog);
        pulseFound=true;

    }else{
        for(int i=1; i < (*ptr).size(); i++){

           // if((*ptr)[i+1].plog == (*ptr)[i].plog) continue;

            if(plog <= (*ptr)[i].plog){
                int i0 = i-1;
                int i1 = i;
                float k = ((float) (*ptr)[i1].pulse - (float) (*ptr)[i0].pulse) / ((float) (*ptr)[i1].plog - (float) (*ptr)[i0].plog);
                *pulses = (*ptr)[i0].pulse + k * (float) (plog-(*ptr)[i0].plog);
                pulseFound=true;
                break;
            }
        }

    }


    if(!pulseFound) return -5;
    if(*pulses<=0) return -6;

    // Correzione OD solo per FILM SCREEN
    if(profilePtr->plateType!=ANALOG_PLATE_CR){
        *pulses = (int) ((float) profilePtr->od[odindex] * (float) (*pulses) / 100);
    }

    if(*pulses<=0) return -6;


    return 0;
}
