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
    padTags.append(QString("PADPROSTHESIS"));
    padTags.append(QString("PADD75_MAG"));
    padTags.append(QString("PADBIOP_2D"));
    padTags.append(QString("PADBIOP_3D"));
    padTags.append(QString("PADTOMO"));
    padTags.append(QString("PAD9x9_MAG"));

    padNames.append(QString("24x30"));
    padNames.append(QString("18x24 CENTER"));
    padNames.append(QString("18x24 LEFT"));
    padNames.append(QString("18x24 RIGHT"));
    padNames.append(QString("9x21 (MAG)"));
    padNames.append(QString("10x24"));
    padNames.append(QString("PROSTHESIS"));
    padNames.append(QString("D75 (MAG)"));
    padNames.append(QString("BIOPSY 2D"));
    padNames.append(QString("BIOPSY STEREO"));
    padNames.append(QString("TOMO 24x30"));
    padNames.append(QString("9x9 (MAG)"));

    comprStat = COMPR_ND;
    comprMagnifier = 1;
    comprPad = PAD_ND;    

    // Imposta la configurazione di default
    setConfigDefault();

    // Verifica se deve compensare id ati di calibrazione del vecchio file di configurazione
    // s il file esiste ne carica solo i dati di calibrazione e poi cancella il vecchioo file
    switchCalibFileVersion();

    // Carica il nuovo file di calibrazione
    readCompressorConfigFile();
    configUpdate = true;
    pConfig->compressor_configured = true;


    // Inizializzazione della diagnostica
    fault = false;
    battery_fault = false;
    battery_low = false;
    safety_fault = false;

    enable_compressione_closed_study = true;


}

// Se trova il vecchio file di configurazione lo legge e lo cancella
// Restituisce true se l'ha trovato
void Compressor::switchCalibFileVersion(void){
    QString command;

    // Verifica esistenza file : /resource/config/compressore.cnf
    QFile file("/resource/config/compressore.cnf");
    if (file.exists())
    {
        // Legge il file e lo cancella
        readOldConfigFile();

        // Cancella il file
        command = QString("rm /resource/config/compressore.cnf");
        system(command.toStdString().c_str());

        command = QString("sync");
        system(command.toStdString().c_str());
    }

    // Verifica esistenza file : /resource/config/padcalib.cnf
    QFile file1("/resource/config/padcalib.cnf");
    if (file1.exists())
    {
        // Cancella il file
        command = QString("rm /resource/config/padcalib.cnf");
        system(command.toStdString().c_str());

        command = QString("sync");
        system(command.toStdString().c_str());
    }
    return ;
}

void Compressor::setConfigDefault(void){

    // Imposta calibrazione di default per la posizione
    config.calibPosOfs = 217;
    config.calibPosK = 151;

    // Imposta calibrazione di default per la forza
    config.F0  = 35;
    config.KF0 = 814;
    config.F1  = 70;
    config.KF1 = 80;
    config.max_compression_force=200;

    // Impostazione di default per limiti meccanici
    config.maxMechPosition = 280;
    config.maxPosition = 400;
    config.maxProtection = 280;

    // Impostazione Sbalzo ingranditore
    config.sbalzoIngranditore[0] = 0;
    config.fattoreIngranditore[0] = 0;
    config.sbalzoIngranditore[1] = 299;
    config.fattoreIngranditore[1] = 20; // 2x
    config.sbalzoIngranditore[2] = 259;
    config.fattoreIngranditore[2] = 18; // 1.8x
    config.sbalzoIngranditore[3] = 0;
    config.fattoreIngranditore[3] = 0;
    config.sbalzoIngranditore[4] = 200;
    config.fattoreIngranditore[4] = 15; // 1.5x
    config.sbalzoIngranditore[5] = 0;
    config.fattoreIngranditore[5] = 0;
    config.sbalzoIngranditore[6] = 0;
    config.fattoreIngranditore[6] = 0;
    config.sbalzoIngranditore[7] = 0;
    config.fattoreIngranditore[7] = 0;

    // Impostazione dei livelli di riconoscimento pad di default
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


    // Imposta i valori di default per le caratteristiche morfologiche
    config.pads[PAD_24x30].offset = -110;
    config.pads[PAD_24x30].kF = 22;
    config.pads[PAD_24x30].peso = 50;

    config.pads[PAD_TOMO_24x30].offset = -110;
    config.pads[PAD_TOMO_24x30].kF = 15;
    config.pads[PAD_TOMO_24x30].peso = 50;

    config.pads[PAD_18x24].offset = -110;
    config.pads[PAD_18x24].kF = 22;
    config.pads[PAD_18x24].peso = 50;

    config.pads[PAD_18x24_LEFT].offset = -110;
    config.pads[PAD_18x24_LEFT].kF = 22;
    config.pads[PAD_18x24_LEFT].peso = 50;

    config.pads[PAD_18x24_RIGHT].offset = -110;
    config.pads[PAD_18x24_RIGHT].kF = 22;
    config.pads[PAD_18x24_RIGHT].peso = 50;

    config.pads[PAD_9x21].offset = 89;
    config.pads[PAD_9x21].kF = 17;
    config.pads[PAD_9x21].peso = 30;

    config.pads[PAD_10x24].offset = -106;
    config.pads[PAD_10x24].kF = 12;
    config.pads[PAD_10x24].peso = 30;

    config.pads[PAD_PROSTHESIS].offset = -106;
    config.pads[PAD_PROSTHESIS].kF = 12;
    config.pads[PAD_PROSTHESIS].peso = 30;

    config.pads[PAD_D75_MAG].offset = 89;
    config.pads[PAD_D75_MAG].kF = 17;
    config.pads[PAD_D75_MAG].peso = 30;


    config.pads[PAD_9x9_MAG].offset = 89;
    config.pads[PAD_9x9_MAG].kF = 17;
    config.pads[PAD_9x9_MAG].peso = 30;

    config.pads[PAD_BIOP_2D].offset = -110;
    config.pads[PAD_BIOP_2D].kF = 22;
    config.pads[PAD_BIOP_2D].peso = 50;

    config.pads[PAD_BIOP_3D].offset = -100;
    config.pads[PAD_BIOP_3D].kF = 10;
    config.pads[PAD_BIOP_3D].peso = 30;

}


// Rilegge tutto il vecchio file di configurazione
// ma carica solo i file di calibrazione di posizione e forza
void Compressor::readOldConfigFile(void)
{
    QString filename;
    int i=0;
    int array[10];

    filename =  QString(COMPRCFG);
    QFile file(filename.toAscii());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    Config::getNextArrayLine(&file,array, 2);
    config.calibPosOfs = array[0];
    config.calibPosK = array[1];

    // Lettura configurazione Pads fino a DEFPAD
    QList<QString> params;
    int offset,kF,peso; // default
    offset=kF=peso=0;
    for(i=0;i<PAD_ENUM_SIZE+1;i++)
    {
        params = Config::getNextArrayFields(&file);
        if(params.at(0)=="DEFPAD") break;
    }

    // Lettura coefficienti per la forza
    Config::getNextArrayLine(&file,array, 4);

    // Coefficienti per la forza
    config.F0 = array[0];
    config.KF0 = array[1];
    config.F1 = array[2];
    config.KF1=array[3];

    file.close();
    return ;
}

#define COMPRESSOR_CONFIG_FILE_REVISION 1
#define COMPRESSOR_CONFIG_FILE "/resource/config/pad.cnf"
void Compressor::readCompressorConfigFile(void){
    QList<QString> dati;

    // Se il file non esiste lo crea con i dati di default
    QFile filetest("/resource/config/pad.cnf");
    if (!filetest.exists()){
        storeConfigFile();
        return;
    }

    // prova ad aprirlo e se non riesce lo ricrea con dati di default
    QFile file(COMPRESSOR_CONFIG_FILE);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        storeConfigFile();
        return;
    }

    // Cicla sugli items
    while(1)
    {

        dati = Config::getNextArrayFields(&file);
        if(dati.isEmpty()) break;

        if(dati.at(0)=="REVISION"){
            // COMPRESSOR_CONFIG_FILE_RELEASE

        }else  if(dati.at(0)=="POSITION"){
            config.calibPosOfs = dati.at(1).toUShort();
            config.calibPosK = dati.at(2).toUShort();
        }else  if(dati.at(0)=="FORCE"){
            config.F0   = dati.at(1).toUShort();
            config.KF0  = dati.at(2).toUShort();
            config.F1   = dati.at(3).toUShort();
            config.KF1  = dati.at(4).toUShort();
            config.max_compression_force = dati.at(5).toUShort();
            if(config.max_compression_force > 200) config.max_compression_force=200;
            else if(config.max_compression_force < 70) config.max_compression_force=70;

        }else  if(dati.at(0)=="PAD24x30"){
            config.pads[PAD_24x30].offset = dati.at(1).toInt();
            config.pads[PAD_24x30].kF     = dati.at(2).toInt();
            config.pads[PAD_24x30].peso   = dati.at(3).toInt();
        }else  if(dati.at(0)=="PAD18x24_C"){
            config.pads[PAD_18x24].offset = dati.at(1).toInt();
            config.pads[PAD_18x24].kF     = dati.at(2).toInt();
            config.pads[PAD_18x24].peso   = dati.at(3).toInt();
        }else  if(dati.at(0)=="PAD18x24_L"){
            config.pads[PAD_18x24_LEFT].offset = dati.at(1).toInt();
            config.pads[PAD_18x24_LEFT].kF     = dati.at(2).toInt();
            config.pads[PAD_18x24_LEFT].peso   = dati.at(3).toInt();
        }else  if(dati.at(0)=="PAD18x24_R"){
            config.pads[PAD_18x24_RIGHT].offset = dati.at(1).toInt();
            config.pads[PAD_18x24_RIGHT].kF     = dati.at(2).toInt();
            config.pads[PAD_18x24_RIGHT].peso   = dati.at(3).toInt();
        }else  if(dati.at(0)=="PAD9x21"){
            config.pads[PAD_9x21].offset = dati.at(1).toInt();
            config.pads[PAD_9x21].kF     = dati.at(2).toInt();
            config.pads[PAD_9x21].peso   = dati.at(3).toInt();
        }else  if(dati.at(0)=="PAD10x24"){
            config.pads[PAD_10x24].offset = dati.at(1).toInt();
            config.pads[PAD_10x24].kF     = dati.at(2).toInt();
            config.pads[PAD_10x24].peso   = dati.at(3).toInt();
        }else  if(dati.at(0)=="PADPROSTHESIS"){
            config.pads[PAD_PROSTHESIS].offset = dati.at(1).toInt();
            config.pads[PAD_PROSTHESIS].kF     = dati.at(2).toInt();
            config.pads[PAD_PROSTHESIS].peso   = dati.at(3).toInt();
        }else  if(dati.at(0)=="PADD75_MAG"){
            config.pads[PAD_D75_MAG].offset = dati.at(1).toInt();
            config.pads[PAD_D75_MAG].kF     = dati.at(2).toInt();
            config.pads[PAD_D75_MAG].peso   = dati.at(3).toInt();
        }
        else  if(dati.at(0)=="PAD9x9_MAG"){
            config.pads[PAD_9x9_MAG].offset = dati.at(1).toInt();
            config.pads[PAD_9x9_MAG].kF     = dati.at(2).toInt();
            config.pads[PAD_9x9_MAG].peso   = dati.at(3).toInt();
        }
        else  if(dati.at(0)=="PADBIOP_2D"){
            config.pads[PAD_BIOP_2D].offset = dati.at(1).toInt();
            config.pads[PAD_BIOP_2D].kF     = dati.at(2).toInt();
            config.pads[PAD_BIOP_2D].peso   = dati.at(3).toInt();
        }else  if(dati.at(0)=="PADBIOP_3D"){
            config.pads[PAD_BIOP_3D].offset = dati.at(1).toInt();
            config.pads[PAD_BIOP_3D].kF     = dati.at(2).toInt();
            config.pads[PAD_BIOP_3D].peso   = dati.at(3).toInt();
        }else  if(dati.at(0)=="PADTOMO"){
            config.pads[PAD_TOMO_24x30].offset = dati.at(1).toInt();
            config.pads[PAD_TOMO_24x30].kF     = dati.at(2).toInt();
            config.pads[PAD_TOMO_24x30].peso   = dati.at(3).toInt();
        }else  if(dati.at(0)=="LIMITS"){
            config.maxMechPosition = dati.at(1).toUShort();
            config.maxPosition = dati.at(2).toUShort();
            config.maxProtection = dati.at(3).toUShort();
        }else  if(dati.at(0)=="MAGNIFIER"){
            config.sbalzoIngranditore[0] = dati.at(1).toUShort();
            config.fattoreIngranditore[0] = dati.at(2).toUShort();
            config.sbalzoIngranditore[1] = dati.at(3).toUShort();
            config.fattoreIngranditore[1] = dati.at(4).toUShort();
            config.sbalzoIngranditore[2] = dati.at(5).toUShort();
            config.fattoreIngranditore[2] = dati.at(6).toUShort();
            config.sbalzoIngranditore[3] = dati.at(7).toUShort();
            config.fattoreIngranditore[3] = dati.at(8).toUShort();
            config.sbalzoIngranditore[4] = dati.at(9).toUShort();
            config.fattoreIngranditore[4] = dati.at(10).toUShort();
            config.sbalzoIngranditore[5] = dati.at(11).toUShort();
            config.fattoreIngranditore[5] = dati.at(12).toUShort();
            config.sbalzoIngranditore[6] = dati.at(13).toUShort();
            config.fattoreIngranditore[6] = dati.at(14).toUShort();
            config.sbalzoIngranditore[7] = dati.at(15).toUShort();
            config.fattoreIngranditore[7] = dati.at(16).toUShort();
        }else  if(dati.at(0)=="THRESHOLDS"){
            config.thresholds[0] = dati.at(1).toInt();
            config.thresholds[1] = dati.at(2).toInt();
            config.thresholds[2] = dati.at(3).toInt();
            config.thresholds[3] = dati.at(4).toInt();
            config.thresholds[4] = dati.at(5).toInt();
            config.thresholds[5] = dati.at(6).toInt();
            config.thresholds[6] = dati.at(7).toInt();
            config.thresholds[7] = dati.at(8).toInt();
            config.thresholds[8] = dati.at(9).toInt();
            config.thresholds[9] = dati.at(10).toInt();
        }
    }
}

void Compressor::storeConfigFile(void){
    QString frame;
    QFile file(COMPRESSOR_CONFIG_FILE);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;


    frame = QString("<REVISION,%1>\n").arg((int) COMPRESSOR_CONFIG_FILE_REVISION);
    file.write(frame.toAscii().data());

    frame = QString("<POSITION,%1,%2>\n").arg(config.calibPosOfs).arg(config.calibPosK);
    file.write(frame.toAscii().data());

    frame = QString("<FORCE,%1,%2,%3,%4,%5>\n").arg(config.F0).arg(config.KF0).arg(config.F1).arg(config.KF1).arg(config.max_compression_force);
    file.write(frame.toAscii().data());

    int padcode = PAD_24x30;
    frame = QString("<PAD24x30,%1,%2,%3>\n").arg(config.pads[padcode].offset).arg(config.pads[padcode].kF).arg(config.pads[padcode].peso);
    file.write(frame.toAscii().data());

    padcode = PAD_18x24;
    frame = QString("<PAD_18x24,%1,%2,%3>\n").arg(config.pads[padcode].offset).arg(config.pads[padcode].kF).arg(config.pads[padcode].peso);
    file.write(frame.toAscii().data());

    padcode = PAD_18x24_LEFT;
    frame = QString("<PAD18x24_L,%1,%2,%3>\n").arg(config.pads[padcode].offset).arg(config.pads[padcode].kF).arg(config.pads[padcode].peso);
    file.write(frame.toAscii().data());

    padcode = PAD_18x24_RIGHT;
    frame = QString("<PAD18x24_R,%1,%2,%3>\n").arg(config.pads[padcode].offset).arg(config.pads[padcode].kF).arg(config.pads[padcode].peso);
    file.write(frame.toAscii().data());

    padcode = PAD_9x21;
    frame = QString("<PAD_9x21,%1,%2,%3>\n").arg(config.pads[padcode].offset).arg(config.pads[padcode].kF).arg(config.pads[padcode].peso);
    file.write(frame.toAscii().data());

    padcode = PAD_10x24;
    frame = QString("<PAD10x24,%1,%2,%3>\n").arg(config.pads[padcode].offset).arg(config.pads[padcode].kF).arg(config.pads[padcode].peso);
    file.write(frame.toAscii().data());

    padcode = PAD_PROSTHESIS;
    frame = QString("<PADPROSTHESIS,%1,%2,%3>\n").arg(config.pads[padcode].offset).arg(config.pads[padcode].kF).arg(config.pads[padcode].peso);
    file.write(frame.toAscii().data());

    padcode = PAD_D75_MAG;
    frame = QString("<PADD75_MAG,%1,%2,%3>\n").arg(config.pads[padcode].offset).arg(config.pads[padcode].kF).arg(config.pads[padcode].peso);
    file.write(frame.toAscii().data());

    padcode = PAD_9x9_MAG;
    frame = QString("<PAD9x9_MAG,%1,%2,%3>\n").arg(config.pads[padcode].offset).arg(config.pads[padcode].kF).arg(config.pads[padcode].peso);
    file.write(frame.toAscii().data());

    padcode = PAD_BIOP_2D;
    frame = QString("<PADBIOP_2D,%1,%2,%3>\n").arg(config.pads[padcode].offset).arg(config.pads[padcode].kF).arg(config.pads[padcode].peso);
    file.write(frame.toAscii().data());

    padcode = PAD_BIOP_3D;
    frame = QString("<PADBIOP_3D,%1,%2,%3>\n").arg(config.pads[padcode].offset).arg(config.pads[padcode].kF).arg(config.pads[padcode].peso);
    file.write(frame.toAscii().data());

    padcode = PAD_TOMO_24x30;
    frame = QString("<PADTOMO,%1,%2,%3>\n").arg(config.pads[padcode].offset).arg(config.pads[padcode].kF).arg(config.pads[padcode].peso);
    file.write(frame.toAscii().data());

    frame = QString("<LIMITS,%1,%2,%3>\n").arg(config.maxMechPosition).arg(config.maxPosition).arg(config.maxProtection);
    file.write(frame.toAscii().data());

    frame = QString("<MAGNIFIER,%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16>\n")\
            .arg(config.sbalzoIngranditore[0]).arg(config.fattoreIngranditore[0])\
            .arg(config.sbalzoIngranditore[1]).arg(config.fattoreIngranditore[1])\
            .arg(config.sbalzoIngranditore[2]).arg(config.fattoreIngranditore[2])\
            .arg(config.sbalzoIngranditore[3]).arg(config.fattoreIngranditore[3])\
            .arg(config.sbalzoIngranditore[4]).arg(config.fattoreIngranditore[4])\
            .arg(config.sbalzoIngranditore[5]).arg(config.fattoreIngranditore[5])\
            .arg(config.sbalzoIngranditore[6]).arg(config.fattoreIngranditore[6])\
            .arg(config.sbalzoIngranditore[7]).arg(config.fattoreIngranditore[7]);
    file.write(frame.toAscii().data());

    frame = QString("<THRESHOLDS,%1,%2,%3,%4,%5,%6,%7,%8,%9,%10>\n")\
            .arg(config.thresholds[0]).arg(config.thresholds[1])\
            .arg(config.thresholds[2]).arg(config.thresholds[3])\
            .arg(config.thresholds[4]).arg(config.thresholds[5])\
            .arg(config.thresholds[6]).arg(config.thresholds[7])\
            .arg(config.thresholds[8]).arg(config.thresholds[9]);
    file.write(frame.toAscii().data());


    file.close();
    file.flush();

    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());

    return ;
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
            // Se non in compressione e lo studio è chiuso allora riabilita l'allarme di compressione a studio chiuso
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

// Restituisce il codice a cui Ã¨ associato il Tag stringa
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
