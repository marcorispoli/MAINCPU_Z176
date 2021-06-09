#define COMPRESSOR_C
#include "application.h"
#include "appinclude.h"
#include "globvar.h"

extern QString padTagArray[];
extern QString padNameArray[];
#define COMPRCFG "/resource/config/compressore.cnf"

void Compressor::activateConnections(void){
    // Aggancia le notifiche dalla PCB215 per il formato
    connect(pConsole,SIGNAL(mccPcb215Notify(unsigned char,unsigned char,QByteArray)),this,SLOT(pcb215Notify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

}

Compressor::Compressor(QObject *parent) :
    QObject(parent)
{
    padTags.append(QString("PAD24x30"));
    padTags.append(QString("PAD18x24_C"));
    padTags.append(QString("PAD18x24_L"));
    padTags.append(QString("PAD18x24_R"));
    padTags.append(QString("PAD9x21"));
    padTags.append(QString("PAD10x24"));
    padTags.append(QString("PADD75_CNT"));
    padTags.append(QString("PADD75_MAG"));
    padTags.append(QString("PADBIOP_2D"));
    padTags.append(QString("PADBIOP_3D"));
    padTags.append(QString("PADTOMO"));

    padNames.append(QString("24x30"));
    padNames.append(QString("18x24 CENTER"));
    padNames.append(QString("18x24 LEFT"));
    padNames.append(QString("18x24 RIGHT"));
    padNames.append(QString("9x21"));
    padNames.append(QString("10x24"));
    padNames.append(QString("D75 CONTACT"));
    padNames.append(QString("D75 MAGNIFY"));
    padNames.append(QString("BIOP 2D"));
    padNames.append(QString("BIOPSY STEREO"));
    padNames.append(QString("TOMO 24x30"));


    comprStat = COMPR_ND;
    comprMagnifier = 1;
    comprPad = PAD_ND;    

    configUpdate = FALSE;   
    pConfig->compressor_configured = (readConfigFile() && readPadCfg());

    // Inizializzazione della diagnostica
    fault = false;
    battery_fault = false;
    battery_low = false;
    safety_fault = false;

    enable_compressione_closed_study = true;


}

bool Compressor::readConfigFile(void)
{
    QString filename;
    int i=0;
    int array[10];

    filename =  QString(COMPRCFG);

    QFile file(filename.toAscii());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() <<"IMPOSSIBILE APRIRE IL FILE:" << filename;
        return FALSE;
    }


    // Lettura calibrazione nacchera
    Config::getNextArrayLine(&file,array, 2);
    config.calibPosOfs = array[0];
    config.calibPosK = array[1];

    // Inizializza i PADS utilizzando un valore impossibile per il peso
    for(i=0;i<PAD_ENUM_SIZE;i++) config.pads[i].peso = 255;

    // Lettura configurazione Pads fino a DEFPAD
    QList<QString> params;
    int offset,kF,peso; // default
    offset=kF=peso=0;
    for(i=0;i<PAD_ENUM_SIZE+1;i++)
    {
        params = Config::getNextArrayFields(&file);
        if(params.size()!=4) return FALSE;
        if(params.at(0)=="DEFPAD")
        {
            // DEFPAD deve essere l'ultimo della lista dei pads
            defPad.offset=params.at(1).toInt();
            defPad.kF = params.at(2).toInt();
            defPad.peso = params.at(3).toInt();
            break;
        }else
        {
            int index = getPadCodeFromTag(params.at(0));
            if(index<0) return FALSE;           
            config.pads[index].offset = params.at(1).toInt();
            if(params.at(0)=="PADBIOP_3D") config.pads[index].offset -= 8; // Correzione per diversa posizione del top
            config.pads[index].kF = params.at(2).toInt();
            config.pads[index].peso = params.at(3).toInt();
        }
    }

    // Ripassa la lista per assegnare il default ai pads non definiti
    // Il valore di default
    for(i=0;i<PAD_ENUM_SIZE;i++)
    {
        if(config.pads[i].peso == 255)
        {
            config.pads[i].offset = defPad.offset;
            config.pads[i].kF = defPad.kF;
            config.pads[i].peso = defPad.peso;
        }
    }

    // Lettura coefficienti per la forza
    Config::getNextArrayLine(&file,array, 4);

    // Coefficienti per la forza
    config.F0 = array[0];
    config.KF0 = array[1];
    config.F1 = array[2];
    config.KF1=array[3];

    // Lettura dati limitazione altezza
    Config::getNextArrayLine(&file,array, 1);
    config.maxMechPosition = array[0];
    Config::getNextArrayLine(&file,array, 1);
    config.maxPosition = array[0];
    Config::getNextArrayLine(&file,array, 1);
    config.maxProtection = array[0];


    // Lettura dati Ingranditore
    for(i=0; i<8; i++)
    {
        Config::getNextArrayLine(&file,array, 2);
        config.sbalzoIngranditore[i] = array[0];
        config.fattoreIngranditore[i] = array[1];
    }

    file.close();
    return true;
}

/*
 *  ATTENZIONE: il file di configurazione del compressore non è sequenziale e contiene
 *  campi opzionali. Pertanto nel salvataggio del file di configurazione verranno
 *  persi tutti i commenti in linea con i campi opzionali. Verranno tuttavia mantenuti
 *  tutti i commenti in linea per i campi sequenziali.
 *
 */
bool Compressor::storeConfigFile(void)
{
    QString filename;
    QString filenamecpy;
    QString command;
    int i=0;
    int array[10];

    filename =  QString(COMPRCFG);
    filenamecpy = "comprtemp.cnf";


    // Copia il file da modificare in file.cnf.bak per sicurezza
    command = QString("cp %1 %1.bak").arg(filename);
    system(command.toStdString().c_str());

    QFile file(filename.toAscii());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() <<"IMPOSSIBILE APRIRE IL FILE:" << filename;
        return FALSE;
    }

    QFile filecpy(filenamecpy.toAscii());
    if (!filecpy.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() <<"IMPOSSIBILE APRIRE IL FILE IN SCRITTURA:" << filenamecpy;
        return FALSE;
    }

    // Scrittura calibrazione nacchera
    array[0] =config.calibPosOfs;
    array[1] =config.calibPosK;
    Config::writeNextArrayLine(&filecpy,&file,array,2);

    // Scrittura configurazione Pads (solo per quelli diversi dal default)
    // Attenzione, questi sono campi opzionali ..
    QByteArray lastLine = Config::alignFileWithValidField(&filecpy,&file);

    for(i=0;i<PAD_ENUM_SIZE;i++)
    {
        if((defPad.offset==config.pads[i].offset) && (defPad.kF==config.pads[i].kF) && (defPad.peso==config.pads[i].peso))
            continue;

        // Scrive il pad nella forna <PADNAME,offset,kF,peso>
        Config::writeNextStringLine(&filecpy,QString("%1,%2,%3,%4").arg(padTags.at(i)).arg((int)config.pads[i].offset).arg((int)config.pads[i].kF).arg((int)config.pads[i].peso));
    }

    // Scrive il pad DEFAULT obbligatoriamente
    Config::writeNextStringLine(&filecpy,QString("%1,%2,%3,%4").arg("DEFPAD").arg((int)defPad.offset).arg((int)defPad.kF).arg((int)defPad.peso));

    // Si allinea con la fine della parte non sequenziale
    if(!lastLine.contains("DEFPAD"))
    {
        // Allinea i files per i prossimi campi sequenziali
        if(Config::alignFileWithTag(&file,QString("DEFPAD"))==FALSE) return FALSE;
    }

    // Scrittura coefficienti per la forza
    array[0] = config.F0;
    array[1] = config.KF0;
    array[2] = config.F1;
    array[3] = config.KF1;
    Config::writeNextArrayLine(&filecpy,&file,array,4);

    // Scrittura dati limitazione altezza
    array[0] = config.maxMechPosition;
    Config::writeNextArrayLine(&filecpy,&file,array,1);

    array[0] = config.maxPosition;
    Config::writeNextArrayLine(&filecpy,&file,array,1);

    array[0] = config.maxProtection;
    Config::writeNextArrayLine(&filecpy,&file,array,1);

    // Scrittura dati Ingranditore
    for(i=0; i<8; i++)
    {
        array[0] = config.sbalzoIngranditore[i];
        array[1] = config.fattoreIngranditore[i];
        Config::writeNextArrayLine(&filecpy,&file,array,2);
    }
    filecpy.flush();
    filecpy.close();
    file.close();

    pSysLog->log("CONFIG: COMPRESSOR CALIBRATION FILE");

    // Copia il file temp nel file definitivo
    filename =  QString(COMPRCFG);
    filenamecpy = "comprtemp.cnf";


    // Copia il file modificato nel file finale
    command = QString("cp comprtemp.cnf %1").arg(filename);
    system(command.toStdString().c_str());

    // SYNC
    command = QString("sync");
    system(command.toStdString().c_str());
    return true;
}



// Intercetta le notifiche dal Driver compressore
void Compressor::pcb215Notify(unsigned char id, unsigned char notifyCode, QByteArray buffer)
{
    // Funzionamento normale
    if(id!=1) return;


    switch(notifyCode)
    {
        case PCB215_NOTIFY_COMPR_DATA:
            comprFlags0 = buffer.at(COMPRESSORE_FLAG0);
            comprFlags1 = buffer.at(COMPRESSORE_FLAG1);

            // Il valore della compressione viene attributio solo sopra soglia
            if(isCompressed())
                comprStrenght = buffer.at(COMPRESSORE_FORZA);
            else
                comprStrenght = 0;
            ApplicationDatabase.setData(_DB_FORZA,(int) comprStrenght,0);

            target_compressione = buffer.at(COMPRESSORE_TARGET);
            ApplicationDatabase.setData(_DB_TARGET_FORCE,(int) target_compressione,0);

            // Aggiorna il campo spessore seno
            breastThick = buffer.at(COMPRESSORE_THICKL)+256*buffer.at(COMPRESSORE_THICKH);
            ApplicationDatabase.setData(_DB_SPESSORE,(int) breastThick,0);

            // Aggiorna il campo posizione, ma solo a studio chiuso
            posizione = buffer.at(COMPRESSORE_POSL)+256*buffer.at(COMPRESSORE_POSH);
            if(ApplicationDatabase.getDataU(_DB_STUDY_STAT)==_CLOSED_STUDY_STATUS)
                ApplicationDatabase.setData(_DB_COMPRESSOR_POSITION,(int) posizione,0);

            if(comprPad!=(Pad_Enum) buffer.at(COMPRESSORE_PAD))
            {                
                comprPad=(Pad_Enum) buffer.at(COMPRESSORE_PAD);
                ApplicationDatabase.setData(_DB_COMPRESSOR_PAD,getPadName(comprPad),0);
                ApplicationDatabase.setData(_DB_COMPRESSOR_PAD_CODE,(unsigned char) comprPad,0);

                padChanged();
                pCollimatore->updateColli(); // Aggiorna la collimazione
            }

            // Errore sicurezza compressore: errore non resettabile
            if(!safety_fault){
                if(comprFlags0&0x80){
                    PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_COMPRESSORE,_COMPR_SAFETY_FAULT,FALSE);
                    safety_fault = true; // Non resettabile
                }
            }

            /*
            // Se non in compressione e lo studio � chiuso allora riabilita l'allarme di compressione a studio chiuso
            if((!isCompressed()) && ( ApplicationDatabase.getDataU(_DB_STUDY_STAT)==_CLOSED_STUDY_STATUS)){
                enable_compressione_closed_study=false;
            }


            if((isCompressed())&&(!enable_compressione_closed_study)&&(ApplicationDatabase.getDataU(_DB_STUDY_STAT)==_CLOSED_STUDY_STATUS)){
                enable_compressione_closed_study=true;
                PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_PAD,_ALR_COMPR_CLOSED_STUDY,TRUE);
            }*/

        break;

        case PCB215_NOTIFY_ERRORS:
            if(buffer.at(0)==PCB215_ERROR_PEDALS_STARTUP)
                PageAlarms::activateNewAlarm(_DB_ALLARME_CMP_PUSH,COMPRESSOR_INVALID_PUSH,FALSE);
            else
                PageAlarms::activateNewAlarm(_DB_ALLARME_CMP_PUSH,0,FALSE);
        break;

    }



    return;
}


// Cambio Compressore riconosciuto: verifica se PAD corretto, ma solo quando serve
void Compressor::padChanged(void)
{

}

QString Compressor::getPadName(void)
{       
    if(comprPad >= PAD_ENUM_SIZE) return QString(QApplication::translate("COMPRESSORE","NON RICONOSCIUTO", 0, QApplication::UnicodeUTF8));
    if(comprPad>=padNames.size()) return QString(QApplication::translate("COMPRESSORE","CODICE ERRATO", 0, QApplication::UnicodeUTF8));
    return padNames.at(comprPad);
}

QString Compressor::getPadName(Pad_Enum code)
{
    if(code==PAD_UNLOCKED){ // Compressore non bloccato
        return QString(QApplication::translate("COMPRESSORE","COMPRESSORE NON BLOCCATO", 0, QApplication::UnicodeUTF8));
    }else if(code==PAD_UNMOUNTED){  // Nacchera non bloccata
        return QString(QApplication::translate("COMPRESSORE","NACCHERA NON BLOCCATA", 0, QApplication::UnicodeUTF8));
    }else if(code >= PAD_ENUM_SIZE) return QString(QApplication::translate("COMPRESSORE","NON RICONOSCIUTO", 0, QApplication::UnicodeUTF8));

    if(code>=padNames.size()) return QString(QApplication::translate("COMPRESSORE","CODICE ERRATO", 0, QApplication::UnicodeUTF8));
    return padNames.at(code);

}

// Restituisce il codice a cui è associato il Tag stringa
// Se non trova nulla restituisce -1
int Compressor::getPadCodeFromTag(QString tag)
{
    int i;

    for(i=0; i< padTags.size(); i++)
    {
        if(padTags.at(i)==tag) return i;
    }

    return -1;
}


// Restituisce il Tag associato al codice del PAD comnpressore in oggetto
QString Compressor::getPadTag(Pad_Enum code)
{
    if(code >= padTags.size()) return "";
    return padTags.at(code);
}


/////////////////////////////////////////////////////////////////////////////////////////////
// LETTURA CONFIGURAZIONE RICONOSCIMENTO PAD
bool Compressor::readPadCfg(void)
{
    QString filename;
    QList<QString> dati;

    // Imposta i valori di default
    config.thresholds[0] = _PAD_THRESHOLD_0;
    config.thresholds[1] = _PAD_THRESHOLD_1;
    config.thresholds[2] = _PAD_THRESHOLD_2;
    config.thresholds[3] = _PAD_THRESHOLD_3;
    config.thresholds[4] = _PAD_THRESHOLD_4;
    config.thresholds[5] = _PAD_THRESHOLD_5;
    config.thresholds[6] = _PAD_THRESHOLD_6;
    config.thresholds[7] = _PAD_THRESHOLD_7;
    config.thresholds[8] = _PAD_THRESHOLD_8;
    config.thresholds[9] = _PAD_THRESHOLD_9;

    // Apre automaticamente il file user
    filename =  QString("/resource/config/padcalib.cnf");

    QFile file(filename.toAscii());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {        
        // Si salva il file con i valori di default
        return storePadCfg();
    }


    while(1)
    {

        dati = pConfig->getNextArrayFields(&file);
        if(dati.isEmpty()) break;

        // Se il dato non è corretto non lo considera
        if(dati.size()!=2) continue;

        if(dati.at(0)=="0")  config.thresholds[0] = dati.at(1).toInt();
        else if(dati.at(0)=="1") config.thresholds[1] = dati.at(1).toInt();
        else if(dati.at(0)=="2") config.thresholds[2] = dati.at(1).toInt();
        else if(dati.at(0)=="3") config.thresholds[3] = dati.at(1).toInt();
        else if(dati.at(0)=="4") config.thresholds[4] = dati.at(1).toInt();
        else if(dati.at(0)=="5") config.thresholds[5] = dati.at(1).toInt();
        else if(dati.at(0)=="6") config.thresholds[6] = dati.at(1).toInt();
        else if(dati.at(0)=="7") config.thresholds[7] = dati.at(1).toInt();
        else if(dati.at(0)=="8") config.thresholds[8] = dati.at(1).toInt();
        else if(dati.at(0)=="9") config.thresholds[9] = dati.at(1).toInt();
    }


    file.close();

    return TRUE;
}

bool Compressor::storePadCfg(void)
{
    QString frame;
    QString filename ;
    int i;

    filename =  QString("/resource/config/padcalib.cnf");

    // Apre il file in scrittura
    QFile file(filename.toAscii());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() <<"IMPOSSIBILE SALVARE IL FILE:" << filename;
        return FALSE;
    }

    for(i=0; i<10;i++){
        frame = QString("<%1,%2>  \n").arg(i).arg(config.thresholds[i]);
        file.write(frame.toAscii().data());
    }

    file.close();
    file.flush();

    pSysLog->log("CONFIG: PAD CALIBRATION FILE");

    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());

    return TRUE;
}

// Calcola le soglie ideali dato il valore numerico
// con PAD aperto (idealmente 255)
void Compressor::calibrateThresholds(unsigned char ncc)
{
    float COEF[]={0,0.1,0.19,0.29,0.38,0.47,0.57,0.67,0.77,0.88,1};
    float levels[11];


    for(int i=0; i<11;i++){
        levels[i] = (float) ncc * COEF[i];
    }

    for(int i=0; i<10;i++){
        int val = (int) ((levels[i] + levels[i+1]) / 2) ;

        config.thresholds[i] = val;
    }
}
