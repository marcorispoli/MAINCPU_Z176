#include "application.h"
#include "appinclude.h"
#include "globvar.h"

void Generatore::activateConnections(void){
    connect(this,SIGNAL(sigStopStarter()),this,SLOT(stopStarterSlot()),Qt::UniqueConnection);
    connect(pConsole,SIGNAL(mccPcb190Notify(unsigned char,unsigned char,QByteArray)),this,SLOT(pcb190Notify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);


}


Generatore::Generatore(QObject *parent) :
    QObject(parent)
{
    validated = FALSE;
    selectedDmAs = 0;
    selectedKv = 0;
    selectedFSize= FUOCO_SZ_ND;
    selectedAnodo="";
    confF1.clear();
    confF2.clear();
    tomoMode=FALSE;
    timerStarter = 0;
    timerUpdateAnodeHU=0;
    manualMode = false;
    templateRevision = 0;
    anodeHuInitOk = false;
    alarmAnodeHU= false;
    pAECprofiles = 0;
    pDose = 0;

    // Apertura dai di configurazione
    QString tubeDir = QString(_TUBEPATH) + QString("/") + pConfig->userCnf.tubeFileName + QString("/");
    pConfig->generator_configured = openTube(tubeDir);



    // Configurazione coefficienti di conversione per la lettura delle Tensioni / Correnti
    genCnf.iAnodeK = 200.0/255.0;  // parametri generici di gestione Generatore
    genCnf.kvK = 10.554 * 5.0 / 255.0; // Coefficiente di conversione kv Medi

    filOn=FALSE;

    hv_calibrated = false;
    hv_updated = false;

    // Conversione RAW-HV to HV
    genCnf.pcb190.HV_CONVERTION = 1000;

    hvval = 0;
    openHVcalibFile();

    warningHV = false;
    faultHV = false;
    levelHV = false;

    faultFilAmpTemp = false;

    faultGnd = false;          // Fault sulla connessione GND
    faultR16 = false;          // Guasto resistenza R16
    warningMas = false;        // Valore anomalo masmetro
    faultMas = false;
    warningITest = false;      // Valore anomalo corrente di test

    warningIFilamento = false;
    faultIFilamento = false;

    v32_warning = false;
    vm32_warning = false;
    v12_warning = false;
    vm12_warning = false;
    v15_warning = false;
    v15ext_warning = false;

    dgn_fault = false;
    starterHS = false;

}

#define FILE_HV_CALIB   "/resource/config/calib_hv.cnf"
#define HV_FILE_RELEASE 1
void Generatore::openHVcalibFile(void){
    QList<QString> dati;

    QFile file(FILE_HV_CALIB);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) return;

    while(1)
    {

        dati = Config::getNextArrayFields(&file);
        if(dati.isEmpty()) break;

        // Se il dato non √® corretto non lo considera
        if(dati.size()!=2) continue;

        // Calibrazione della tensione HV
        if(dati.at(0)=="HV_CALIB"){
            genCnf.pcb190.HV_CONVERTION = dati.at(1).toInt();
        }else if(dati.at(0)=="HV_VAC"){
            genCnf.pcb190.HV_VAC = dati.at(1).toInt();
        }else if(dati.at(0)=="VPRIMARIO"){
            genCnf.pcb190.HV_VPRIMARIO = dati.at(1).toInt();
        }else if(dati.at(0)=="RELEASE"){
            if(dati.at(1).toInt()==HV_FILE_RELEASE){
                hv_calibrated = true;
            }
        }
    }
    file.close();

    if(!hv_calibrated){
        genCnf.pcb190.HV_CONVERTION = 1000;
        genCnf.pcb190.HV_VAC =0;
        genCnf.pcb190.HV_VPRIMARIO = 0;
    }
    return;
}
void Generatore::storeHVcalibFile(void){

    QFile file(FILE_HV_CALIB);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) return;

    QString data = QString("<HV_CALIB,%1>  //  Calibrazione tensione HV\n").arg(genCnf.pcb190.HV_CONVERTION);
    file.write(data.toAscii().data());

    data = QString("<HV_VAC,%1>  //  Tensione di rete letta \n").arg(genCnf.pcb190.HV_VAC);
    file.write(data.toAscii().data());

    data = QString("<VPRIMARIO,%1>  //  Tensione di primario \n").arg(genCnf.pcb190.HV_VPRIMARIO);
    file.write(data.toAscii().data());

    data = QString("<RELEASE,%1>  //  Release file di configurazione \n").arg(HV_FILE_RELEASE);
    file.write(data.toAscii().data());


    file.close();
    file.flush();

    pSysLog->log("CONFIG: MAIN VOLTAGE CALIBRATION FILE");

    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());

    // Effettua la riconfigurazione della PCB190
    // per aggiornare il parametro di conversione
    // RAWHV-HV
    // pConfig->updatePCB190();

}

/* Verifica sulle condizioni del filamento
 *  - Se il filamento √® spento resetta tutti i flag di errore;
 *  - Se il filamento in IDLE supera il 10% allora viene generato un fault (se abilitato);
 *  - Se il filamento in IDLE supera il 5% allora viene generato un warning
 *
 *  La gestione del fault e del warning non dipende da questa funzione ma da specifiche di sistema
 *  return: TRUE se c'√® stato un cambio di stato
 */
void Generatore::testFilamento(void){

    // Con il filamento spento non √® verificare la corrente in IDLE
    if(!(flags0 & 8)){
        warningIFilamento = false;
        faultIFilamento = false;
        return;
    }


    int dfil = pConfig->userCnf.correnteFilamento * 5 / 100;

    if((iFilamento > pConfig->userCnf.correnteFilamento + dfil) ||(iFilamento < pConfig->userCnf.correnteFilamento - dfil) ){
       warningIFilamento = true;

       // Nel caso sia abilitato il controllo anche sull'errore allora verifica che non sia superato il 10%
       if(pConfig->userCnf.enableIFil){
           if((iFilamento > pConfig->userCnf.correnteFilamento + 2 * dfil) ||(iFilamento < pConfig->userCnf.correnteFilamento - 2 * dfil) )
              faultIFilamento = true;
           else faultIFilamento = false;
       }else{
           faultIFilamento = false;
       }
    }else {
        warningIFilamento = false;
        faultIFilamento = false;
    }

    return;

}

/**
 * Verifica se il livello dell'alimentazione di potenza pu√≤ determinare un allarme
 * - Viene identificato un livello tra 0 e 4;
 * - 0 = HV entro il 10% di variazione sul nominale di 560V;
 * - 1,2 ->   HV compreso tra 10% e 15% di variazione sul valore nominale (WARNING);
 * - 3,4 ->   HV compreso tra 15% e 20% di variazione sul valore nominale (FAULT CONDIZIONATO);
 * - 5,6 ->   HV oltre il 20% di variazione sul valore nominale (FAULT);
 */

void  Generatore::testHV(void){

    // Con la lettura non calibrata non procede
    if(!hv_calibrated){
        warningHV = false;
        faultHV = false;
        levelHV=0;
        return ;
    }


    // Calcola i livelli
    if(hvval>=_HV_POWER_H){
        levelHV = 6;
    }else if(hvval<=_HV_POWER_L){
        levelHV = 5;
    }else if(hvval>=_HV_WRN_POWER_H){
        levelHV = 4;
    }else if(hvval<=_HV_WRN_POWER_L){
        levelHV = 3;
    }else if(hvval>=_HV_STD_POWER_H){
        levelHV = 2;
    }else if(hvval<=_HV_STD_POWER_L){
        levelHV = 1;
    }else levelHV = 0;


    // Decide per il tipo di errore
    if(levelHV==0){
        warningHV = false;
        faultHV = false;
    }else{
        warningHV = true;
        if(levelHV>=5) faultHV = true; // FAULT SICURO
        else if((levelHV>=3) && (pConfig->userCnf.enableHVMonitor)) faultHV = true;
        else faultHV = false;
    }

    return ;

}

void Generatore::pcb190Notify(unsigned char id, unsigned char cmd, QByteArray data)
{

    if(cmd == PCB190_NOTIFY_DGN){

        // Ricava lo stato generale della potenza della macchina per non mischiare la diagnostica
        unsigned char powerstat = ApplicationDatabase.getDataU(_DB_POWER_STAT);

        // In demo annulla tutti gli allarmi
        if(pConfig->userCnf.demoMode){
            hv_calibrated = true;
            hvval = 560;
            iFilamento = pConfig->userCnf.correnteFilamento;
            tempAmplFil = 40;
            anodicaTest = 30;
            mAsTest = 30;
            v32 = data[PCB190_V32] * 0.2593;
            vm32= (data[PCB190_VM32] * 63 - 1222 * v32) / 1000 ;
            v12= data[PCB190_V12] * 0.096;
            vm12= (data[PCB190_VM12] * 75 - 1830 * v12) / 1000 ;
            v15= data[PCB190_V15] * 120 / 1000; // Circa
            v15ext= data[PCB190_V15EXT]*111 / 1000;
            unsigned char fault_code = 0;
            faultHV = false;
            faultIFilamento = false;
            faultFilAmpTemp = false;
            faultGnd = false;
            faultR16 = false;
            faultMas = false;
            dgn_fault=false;
            ApplicationDatabase.setData(_DB_FAULT_CODE_GEN,fault_code,0);
            return;
        }

        // Lettura conteggi lanci starter
        float ARjoule = (data[PCB190_ARLS] * 1560 + data[PCB190_ARHS] * 3560) * 1.33 / 1000;
        if(ARjoule>0) updateAnodeHU(ARjoule);


        hv_updated = true;
        hvraw = (int) data[PCB190_DGN_HV_IDLE];


        // Gestione della tensione di alimentazione in IDLE
        // Il controllo diagnostico viene effettuato solo se lo stato della gestione potenza Ë OK
        if(hv_calibrated) hvval = (int) ((float) hvraw * genCnf.pcb190.HV_CONVERTION /1000);
        else hvval = 0;
        if(powerstat==PWRMANAGEMENT_STAT_OK) testHV();
        else faultHV=false;


        // Controllo della corrente di filamento in IDLE

        iFilamento = (int) data[PCB190_I_FIL] * 48; // Lettura corrente di filamento in IDLE: 1A : 0.408V         
        testFilamento();

        // Lettura e verifica temperatura

        tempAmplFil = (int) (data[PCB190_DGN_VTEMP]* 500 / 255 - 273); // Gestione della temperatura dell'amplificatore di filamento
        if(tempAmplFil>90) faultFilAmpTemp = true;
        else faultFilAmpTemp = false;


        flags0 = (char) data[PCB190_FLAGS0];        // Lettura flags di stato

        anodicaTest = (data[PCB190_I_ANODICA] * 200) / 255 ; // lettura corrente anodica di test
        mAsTest = ((float)data[PCB190_MAS_TEST_L] + 256.0 * (float) data[PCB190_MAS_TEST_H] ) / 50.0; // Lettura masmetro (su 1 secondo)

        // Verifica diagnostica solo se abilitata
        if(pConfig->userCnf.enableTestGnd){
            // Controllo presenza ground
            if(anodicaTest<10) faultGnd = true;
            else faultGnd = false;
        }else faultGnd = false;

        // Controllo integrit√  resistenza R16: SEMPRE
        if(anodicaTest>250){
            faultR16 = true;
            faultITest = false;
            warningITest = false;
        }else {
            faultR16 = false;
            // Controllo valore anodica: solo warning se la variazione √® contenuta altrementi √® fault
            if((anodicaTest < pConfig->userCnf.correnteAnodicaTest-10)||(anodicaTest > pConfig->userCnf.correnteAnodicaTest+10)){
                faultITest = true;
                warningITest = true;
            }else{
                faultITest  = false;
                if((anodicaTest < pConfig->userCnf.correnteAnodicaTest-5)||(anodicaTest > pConfig->userCnf.correnteAnodicaTest+5)) warningITest=true;
                else warningITest = false;
            }
        }

        // Controllo mAs (solo se gnd √® ok)
        if(faultGnd==false){
            int dmas = pConfig->userCnf.correnteAnodicaTest * 5 / 100;
            if((mAsTest < pConfig->userCnf.correnteAnodicaTest-dmas)||(mAsTest > pConfig->userCnf.correnteAnodicaTest+dmas)) warningMas=true;
            else warningMas = false;
            if(pConfig->userCnf.enableTestMasmetro){
                if((mAsTest < pConfig->userCnf.correnteAnodicaTest-2*dmas)||(mAsTest > pConfig->userCnf.correnteAnodicaTest+2*dmas)) faultMas=true;
                else faultMas=false;
            }else faultMas = false;
        }


        // Tensioni di alimentazione scheda
        v32 = data[PCB190_V32] * 0.2593;
        vm32= (data[PCB190_VM32] * 63 - 1222 * v32) / 1000 ;
        v12= data[PCB190_V12] * 0.096;
        vm12= (data[PCB190_VM12] * 75 - 1830 * v12) / 1000 ;
        v15= data[PCB190_V15] * 120 / 1000; // Circa
        v15ext= data[PCB190_V15EXT]*111 / 1000;


        // Controllati con la logica della variazione del 10% e solo a livello di Warning
        if(v32 < 28.8) v32_warning = true;
        else v32_warning = false;
        if(vm32 > -28.8) vm32_warning = true;
        else vm32_warning = false;

        if(v12 < 10.8) v12_warning = true;
        else v12_warning = false;
        if(vm12 > -10.8) vm12_warning = true;
        else vm12_warning = false;

        // Questa non pu√≤ essere controllata con precisione poich√® non √® calibrata
        // deve essere controllata con la logica della presenza
        if(v15 < 10) v15_warning = true;
        else v15_warning = false;

        if(v15ext < 13.5) v15ext_warning = true;
        else v15ext_warning = false;

        // Controllo sullo starter
        if((flags0&0x2)&&(data[PCB190_FAULTS]>=INIT_ALR_STARTER) && (data[PCB190_FAULTS]<=END_ALR_STARTER)) fault_starter = true;
        else fault_starter = 0;

        // Controllo in ordine di importanza
        dgn_fault_code = 0;
        if(faultR16) dgn_fault_code = GEN_R16_FAULT;
        else if(faultGnd) dgn_fault_code = GEN_GND_FAULT;
        else if(hv_calibrated==false) dgn_fault_code=GEN_CALIB_HV;
        else if(faultHV) dgn_fault_code = GEN_HV_FAULT;
        else if((faultMas)||(faultITest)) dgn_fault_code = GEN_MAS_FAULT;  // Condividono lo stesso simbolo
        else if(faultIFilamento) dgn_fault_code = GEN_IFIL_FAULT;
        else if(faultFilAmpTemp) dgn_fault_code = GEN_AMPTEMP_FAULT;
        else if(fault_starter)  dgn_fault_code = data[PCB190_FAULTS];
        else dgn_fault_code=0;


        // Attivazione flag generale di fault per impedire l'attivazione raggi
        if(dgn_fault_code) dgn_fault=true;
        else dgn_fault=false;
        PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_GEN,dgn_fault_code);
        //ApplicationDatabase.setData(_DB_FAULT_CODE_GEN,fault_code,0);


    }
}

// Carica la revisione corrente del tubo passato come filename
int Generatore::getTubeRevision(QString filename){

    QFile file(filename.toAscii());
    if(!file.exists()) return 0;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return 0;

    QList<QString> dati;
    dati = pConfig->getNextArrayFields(&file);
    if(dati.size()!=2) return 0;
    if(dati.at(0)=="REV")  return dati.at(1).toInt();
    return 0;

}

// Salva la revisione corrente nel Tubo selezionato
void Generatore::setTubeRevision(QString tube){

    // Scrive la revisione del tubo
    QFile file(QString("%1%2").arg(tube).arg(_TUBE_REV_FILENAME).toAscii());
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        file.write(QString("<REV,%1>\n").arg((int) templateRevision).toAscii());
        file.close();
    }

}


// APERTURA DI DATI DI CONFIGURAZIONE DEL TUBO
// tubeDir √® la directory contenente i dati del tubo, comprensivo di "/"
bool Generatore::openTube(QString tubeDir)
{
    QString tubeConfFile;
    int i;


    // Inizializzazione della struttura dati
    for(i=0;i<_MAX_KV-_MIN_KV+1;i++)
    {
        tube[i].tomo.clear();
        tube[i].vRef.enable=FALSE; // Reset KV
        tube[i].mAs.clear();
        tube[i].starter.clear();
        tube[i].iTab.clear();
    }

    // Lettura della revisione corrente del template del tubo
    // IL codice revisione √® contenuto in un file revision.cnf.
    // Se il file non √® pesente nel tubo corrente si assegna 0
    // ATTENZIONE: il codice di revisione letto influenza
    // le modalit√  di lettura del tubo stesso
    templateRevision = getTubeRevision(QString("%1%2").arg(tubeDir).arg(_TUBE_REV_FILENAME));


    // Lettura file di configurazione generale del tubo
    tubeConfFile = QString("%1%2").arg(tubeDir).arg(_TUBE_CONFIG_NAME);
    QFile file(tubeConfFile.toAscii());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return FALSE;
    if(readTubeConfigFile(&file) == FALSE)
    {
        file.close();        
        return FALSE;
    }


    // Ciclo di lettura di tutti i file KV abilitati
    max_selectable_kv = _MIN_KV;
    for(i=_MIN_KV;i<_MAX_KV+1;i++)
    {
        if(tube[i-_MIN_KV].vRef.enable==FALSE) continue;
        if(readKvFile((unsigned char) i, tubeDir)==FALSE) return FALSE;
        if(i>max_selectable_kv) max_selectable_kv=i;
    }

    // Lettura file di calibrazione (se esistente)
    readTubeCalibFile(tubeDir);

    // Lettura offsets ma solo fuori dalla calibrazione dei KV
    readTubeKvOffsetFile(tubeDir);

    // Lettura dei coefficienti di calibrazione della lettura dei kV
    readTubeKvReadCalibrationFile();

    // Lettura statistiche tubo
    readTubeStatisticsFile(tubeDir);

    // Lettura file di gestione del filamento
    readTubeFilamentFile(tubeDir);

    // Lettura dati di calibrazione starter
    readStarterFile();

    // Apertura eventuali profili ma solo se il tubo Ë configurato e il sistema Ë analogico
    if(pAECprofiles!=0){
        delete pAECprofiles;
        pAECprofiles = 0;
    }
    pAECprofiles = new AEC(tubeDir);
    if(pDose!=0){
        delete pDose;
        pDose = 0;
    }
    pDose = new DOSE(tubeDir);

    pConfig->aec_configured      = pAECprofiles->aec_configured;
    pConfig->kerma_mo_configured = pDose->kerma_mo_configured;
    pConfig->kerma_rh_configured = pDose->kerma_rh_configured;

    // Inizializza la Temperatura Anodica
    initAnodeHU();

    return TRUE;
}

// Rinfresca i dati del tubo correntemente selezionato
bool Generatore::openCurrentTube(){

    QString tubeDir = QString(_TUBEPATH) + QString("/") + pConfig->userCnf.tubeFileName + QString("/");
    return openTube(tubeDir);

}

bool Generatore::readTubeCalibFile(QString tubeDir)
{
    QString fileName;
    int i;
    QList<QString> param;
    int kv,In,Ine;
    unsigned char fsize;

    // Lettura file di configurazione generale del tubo
    fileName = QString("%1%2").arg(tubeDir).arg(_TUBE_CALIB_NAME);
    QFile file(fileName.toAscii());
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) return FALSE;

    // Legge tutto il contenuto del file e riempie i campi della struttura con le correnti
    // appena calibrate
    while(!file.atEnd())
    {
        param = Config::getNextArrayFields(&file);
        if(param.size()!=6) continue;

        // Controllo sui parametri
        if((param[1].toInt()<_MIN_KV) || (param[1].toInt()>_MAX_KV)) continue;
        if((param[2]!="W") && (param[2]!="Mo")) continue;
        if((param[2]!=confF1) && (param[2]!=confF2)) continue;
        if((param[3]!="G") && (param[3]!="P")) continue;

        kv = param[1].toInt()-_MIN_KV;
        In = param[4].toInt();
        Ine = param[5].toInt();
        if(param[3]=="G") fsize = (unsigned char) Generatore::FUOCO_LARGE;
        else fsize = (unsigned char) Generatore::FUOCO_SMALL;

        if(param.at(0)=="T")
        {// Tomo
            // Cerca il valore nominale corrispondente al parametro
            for(i=0;i<tube[kv].tomo.size();i++)
            {
                if((tube[kv].tomo[i].In == In) && (tube[kv].tomo[i].anode == param[2]))
                {
                    tube[kv].tomo[i].INcalib = Ine;
                    break;
                }
            }

        }else if(param.at(0)=="S")
        {// Standard
            // Cerca il valore nominale corrispondente al parametro
            for(i=0;i<tube[kv].iTab.size();i++)
            {
                if((tube[kv].iTab[i].In == In) && (tube[kv].iTab[i].anode == param[2])&& (tube[kv].iTab[i].fsize == fsize))
                {
                    tube[kv].iTab[i].INcalib = Ine;
                    break;
                }
            }

        }
    }


    file.close();
    return TRUE;
}

bool Generatore::readTubeConfigFile(QFile* fp)
{
    int i;
    QByteArray array,ftag, mtag;

    // Legge Array di Tensioni disponibili
    array = Config::getNextArrayLine(fp);
    if(array.isEmpty()) return FALSE;

    // Scansione data per abilitazione tensioni
    for(i=0;i<array.size();i++)
    {
        if(array.at(i)<_MIN_KV) continue;
        if(array.at(i)>_MAX_KV) continue;
        tube[array.at(i)-_MIN_KV].vRef.enable=TRUE;
        tube[array.at(i)-_MIN_KV].vRef.Voffset=0;
    }

    // Lettura Presenza obbligatoria starter
    array = Config::getNextValidLine(fp);
    if(array.isEmpty()) return FALSE;
    i=0;
    ftag = Config::getNextTag(&array,&i,false);
    if(ftag!="HS") return FALSE;
    mtag = Config::getNextTag(&array,&i,false);
    if( (mtag=="1") && (!pConfig->sys.highSpeedStarter)) return FALSE;

    // Lettura Materiali disponibili
    array = Config::getNextValidLine(fp);
    if(array.isEmpty()) return FALSE;
    i=0;

    confF1.clear();
    confF2.clear();
    // Lettura dati per Fuoco 1 (obbligatorio)
    ftag = Config::getNextTag(&array,&i,false);
    if((ftag!="F1")&&(ftag!="F2")) return FALSE;

    mtag = Config::getNextTag(&array,&i,false);
    if(ftag=="F1") confF1.append(mtag);
    else confF2.append(mtag);

    // Lettura dati per Fuoco 2 (obbligatorio)
    ftag = Config::getNextTag(&array,&i,false);
    if((ftag!="F1")&&(ftag!="F2")) return FALSE;
    mtag = Config::getNextTag(&array,&i,TRUE);
    if(ftag=="F1") confF1.append(mtag);
    else confF2.append(mtag);

    // Lettura impostazione di default
    array = Config::getNextValidLine(fp);
    if(array.isEmpty()) return FALSE;
    i=0;

    mtag = Config::getNextTag(&array,&i,false);
    ftag = Config::getNextTag(&array,&i,true);

    if((mtag!=confF1)&&(mtag!=confF2))
    {
        startupSelectedAnodo="";
        selectedAnodo ="";
    }else
    {
        startupSelectedAnodo = mtag;
        selectedAnodo = mtag;
    }

    if(ftag=="P") selectedFSize = FUOCO_SMALL;
    else if(ftag=="G") selectedFSize = FUOCO_LARGE;
    else selectedFSize = FUOCO_SZ_ND;
    startupSelectedFSize = selectedFSize;  // Dimensione Fuoco selezionato alla partenza
    return TRUE;

}

// Aggiornamento statistiche ad ogni shot con dose
void Generatore::notifyStatisticData(int datakv, unsigned int datamAs, bool isStandard){
   statChanged = true;
   numShots++;

   long shotJ = (long) datakv *  (long) (datamAs/50);
   cumulatedJ += shotJ;
   if(isStandard) nStandard++;
   else nTomo++;

   // Aggiornamento degli HU accumulati sull'Anodo
   updateAnodeHU((float) shotJ * 1.33 / 1000.0);

   return;
}


/*
 * Lettura dei dati statistici sull'utilizzo del tubo
 *
 */
void Generatore::readTubeStatisticsFile(QString tubeDir){
    QList<QString> dati;

    numShots = 0;
    cumulatedJ = 0;
    nTomo = 0;
    nStandard = 0;
    anodeHUSaved = 0;
    anodeHuTime = QDateTime::currentDateTime();

    QString tubeName = tubeDir + QString("statistics.dat");

    // Lettura file di configurazione generale del tubo
    QFile file(tubeName.toAscii());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    while(1)
    {

        dati = Config::getNextArrayFields(&file);
        if(dati.isEmpty()) break;

        // Se il dato non √® corretto non lo considera
        if(dati.size()==2){
            // Sblocco/Blocco compressore
            if(dati.at(0)=="SHOTS"){
                numShots = dati.at(1).toLong();
            }else if(dati.at(0)=="CUM_J"){
                cumulatedJ = dati.at(1).toLong();
            }else if(dati.at(0)=="NTOMO"){
                nTomo = dati.at(1).toLong();
            }else if(dati.at(0)=="NSTANDARD"){
                nStandard = dati.at(1).toLong();
            }
        }else if(dati.size()==8){
            if(dati.at(0)=="ANODE_HU"){
                anodeHUSaved = dati.at(1).toInt();
                int year = dati.at(2).toInt();
                int month = dati.at(3).toInt();
                int day = dati.at(4).toInt();
                int hr = dati.at(5).toInt();
                int mn = dati.at(6).toInt();
                int sec = dati.at(7).toInt();

                anodeHuTime = QDateTime(QDate(year,month,day),QTime(hr,mn,sec));
            }
        }

    }

    // Se la data di sistema Ë gi‡ stata inizializzata allora Ë possibili
    // effettuare l'init del controllo di temperatura dell'Anodo.

    if(QDateTime::currentDateTime().date().year()>=2018){
        systemTimeUpdated = true;
        initAnodeHU();
    }

    file.close();
    statChanged = false;
}

/*
 * Lettura parametri per gestione filamento
 *
 */
void Generatore::readTubeFilamentFile(QString tubeDir){
    QList<QString> dati;

    float Iwarm=0;
    float Ilimit=0;
    float Imax=0;
    unsigned short Idac=0;

    genCnf.filData.IFILwarm = 2.02; // Corrente nominale di riscaldamento
    genCnf.filData.IFILdac = 1300;  // Taratura corrente di riscaldamento
    genCnf.filData.IFILlimit = 4.5; // Valore assoluto corrente di filamento
    genCnf.filData.IFILmax = 4.0;   // Massimo valore impostabile corrente di filamento

    QString tubeName = tubeDir + QString("filament.dat");


    // Lettura file di configurazione generale del tubo
    QFile file(tubeName.toAscii());
    if(!file.exists()){
        saveTubeFilamentFile(tubeDir); // Salva i valori di default
        return;
    }

    // Se il file non si dovesse aprire (perch√®??) allora  parte con i valori di default
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    while(1)
    {

        dati = Config::getNextArrayFields(&file);
        if(dati.isEmpty()) break;

        // Se il dato non √® corretto non lo considera
        if(dati.size()!=2) continue;

        // Sblocco/Blocco compressore
        if(dati.at(0)=="ILIM"){              // Ilimit in (A)
            Ilimit = dati.at(1).toFloat();
        }else if(dati.at(0)=="IMAX"){       // Max selectable in (A)
            Imax = dati.at(1).toFloat();
        }else if(dati.at(0)=="IWARM"){      // Warming Up in (A)
            Iwarm = dati.at(1).toFloat();
        }else if(dati.at(0)=="IDAC"){       // Warming Up in DAC
            Idac = (unsigned short) dati.at(1).toLong();
        }
    }
    file.close();

    if((Ilimit!=0) && (Imax!=0) && (Ilimit<=6) && (Idac!=0) && (Iwarm!=0)){
            genCnf.filData.IFILwarm = Iwarm; // Corrente nominale di riscaldamento
            genCnf.filData.IFILdac = Idac;  // Taratura corrente di riscaldamento
            genCnf.filData.IFILlimit = Ilimit; // Valore assoluto corrente di filamento
            genCnf.filData.IFILmax = Imax;   // Massimo valore impostabile corrente di filamento
    }

    // Calcolo dei parametri
    genCnf.pcb190.IFIL_DAC_WARM = genCnf.filData.IFILdac;
    genCnf.pcb190.IFIL_LIMIT = genCnf.filData.IFILlimit * 20.838; // 0.4086 * I * 255/5
    genCnf.pcb190.IFIL_MAX_SET = (unsigned short) (genCnf.filData.IFILmax * (float) genCnf.filData.IFILdac / genCnf.filData.IFILwarm);

    // Aggiunta configurazione dello starter    
    genCnf.pcb190.starter_off_after_exposure = pConfig->userCnf.starter_off_after_exposure;
    genCnf.pcb190.starter_off_with_brake = pConfig->userCnf.starter_brake;

    return;
}


void Generatore::saveStarterFile(void){

    // Apre automaticamente il file user
    QString filename =  QString(_CFGPATH);
    filename.append("/");
    filename.append("starter.cnf");
    QFile file(filename.toAscii());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QString frame = QString("<MAIN_OFF,%1>  \n").arg(genCnf.pcb190.cal_main_off);
    file.write(frame.toAscii().data());
    frame = QString("<MAIN_RUN,%1>  \n").arg(genCnf.pcb190.cal_main_run);
    file.write(frame.toAscii().data());
    frame = QString("<MAIN_KEEP,%1>  \n").arg(genCnf.pcb190.cal_main_keep);
    file.write(frame.toAscii().data());
    frame = QString("<MAIN_BRK,%1>  \n").arg(genCnf.pcb190.cal_main_brk);
    file.write(frame.toAscii().data());
    frame = QString("<SHIFT_OFF,%1>  \n").arg(genCnf.pcb190.cal_shift_off);
    file.write(frame.toAscii().data());
    frame = QString("<SHIFT_RUN,%1>  \n").arg(genCnf.pcb190.cal_shift_run);
    file.write(frame.toAscii().data());
    frame = QString("<SHIFT_KEEP,%1>  \n").arg(genCnf.pcb190.cal_shift_keep);
    file.write(frame.toAscii().data());
    frame = QString("<SHIFT_BRK,%1>  \n").arg(genCnf.pcb190.cal_shift_brk);
    file.write(frame.toAscii().data());

    frame = QString("<MAX_MAIN_OFF,%1>  \n").arg(genCnf.pcb190.cal_max_main_off);
    file.write(frame.toAscii().data());
    frame = QString("<MAX_MAIN_RUN,%1>  \n").arg(genCnf.pcb190.cal_max_main_run);
    file.write(frame.toAscii().data());
    frame = QString("<MAX_MAIN_KEEP,%1>  \n").arg(genCnf.pcb190.cal_max_main_keep);
    file.write(frame.toAscii().data());
    frame = QString("<MAX_MAIN_BRK,%1>  \n").arg(genCnf.pcb190.cal_max_main_brk);
    file.write(frame.toAscii().data());
    frame = QString("<MAX_SHIFT_OFF,%1>  \n").arg(genCnf.pcb190.cal_max_shift_off);
    file.write(frame.toAscii().data());
    frame = QString("<MAX_SHIFT_RUN,%1>  \n").arg(genCnf.pcb190.cal_max_shift_run);
    file.write(frame.toAscii().data());
    frame = QString("<MAX_SHIFT_KEEP,%1>  \n").arg(genCnf.pcb190.cal_max_shift_keep);
    file.write(frame.toAscii().data());
    frame = QString("<MAX_SHIFT_BRK,%1>  \n").arg(genCnf.pcb190.cal_max_shift_brk);
    file.write(frame.toAscii().data());
    frame = QString("<MIN_MAIN_RUN,%1>  \n").arg(genCnf.pcb190.cal_min_main_run);
    file.write(frame.toAscii().data());
    frame = QString("<MIN_MAIN_KEEP,%1>  \n").arg(genCnf.pcb190.cal_min_main_keep);
    file.write(frame.toAscii().data());
    frame = QString("<MIN_MAIN_BRK,%1>  \n").arg(genCnf.pcb190.cal_min_main_brk);
    file.write(frame.toAscii().data());
    frame = QString("<MIN_SHIFT_RUN,%1>  \n").arg(genCnf.pcb190.cal_min_shift_run);
    file.write(frame.toAscii().data());
    frame = QString("<MIN_SHIFT_KEEP,%1>  \n").arg(genCnf.pcb190.cal_min_shift_keep);
    file.write(frame.toAscii().data());
    frame = QString("<MIN_SHIFT_BRK,%1>  \n").arg(genCnf.pcb190.cal_min_shift_brk);
    file.write(frame.toAscii().data());

    file.close();
    file.flush();

    pSysLog->log("CONFIG: STARTER CONFIGURATION FILE");

    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());


}

bool Generatore::isStarterCalibrated(){
    if(genCnf.pcb190.cal_main_run != 0) return true;
    if(genCnf.pcb190.cal_shift_run != 0) return true;
    if(genCnf.pcb190.cal_main_keep != 0) return true;
    if(genCnf.pcb190.cal_shift_keep != 0) return true;
    if(genCnf.pcb190.cal_main_off != 0) return true;
    if(genCnf.pcb190.cal_shift_off != 0) return true;
    if(genCnf.pcb190.cal_main_brk != 0) return true;
    if(genCnf.pcb190.cal_shift_brk != 0) return true;
    return false;

}

/*
 * Lettura parametri per gestione filamento
 *
 */
void Generatore::readStarterFile(void){
    QList<QString> dati;

    // La lettura di questo file ha senso solo in presenza dello starter a bassa velocit‡
    if(pConfig->sys.highSpeedStarter) return;

    genCnf.pcb190.cal_main_run = 0;
    genCnf.pcb190.cal_shift_run = 0;
    genCnf.pcb190.cal_main_keep = 0;
    genCnf.pcb190.cal_shift_keep = 0;
    genCnf.pcb190.cal_main_off = 0;
    genCnf.pcb190.cal_shift_off = 0;
    genCnf.pcb190.cal_main_brk = 0;
    genCnf.pcb190.cal_shift_brk = 0;

    genCnf.pcb190.cal_max_main_off = 255;
    genCnf.pcb190.cal_max_shift_off = 255;
    genCnf.pcb190.cal_max_main_run = 255;
    genCnf.pcb190.cal_max_shift_run = 255;
    genCnf.pcb190.cal_max_main_keep = 255;
    genCnf.pcb190.cal_max_shift_keep = 255;
    genCnf.pcb190.cal_max_main_brk = 255;
    genCnf.pcb190.cal_max_shift_brk = 255;


    genCnf.pcb190.cal_min_main_run = 0;
    genCnf.pcb190.cal_min_shift_run = 0;
    genCnf.pcb190.cal_min_main_keep = 0;
    genCnf.pcb190.cal_min_shift_keep = 0;
    genCnf.pcb190.cal_min_main_brk = 0;
    genCnf.pcb190.cal_min_shift_brk = 0;

    // Apre automaticamente il file user
    QString filename =  QString(_CFGPATH);
    filename.append("/");
    filename.append("starter.cnf");

    QFile file(filename.toAscii());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() <<"IMPOSSIBILE APRIRE IL FILE USER:" << filename;
        return ;
    }

    while(1)
    {

        dati = Config::getNextArrayFields(&file);
        if(dati.isEmpty()) break;

        // Se il dato non √® corretto non lo considera
        if(dati.size()!=2) continue;


        if(dati.at(0)=="MAIN_OFF"){
            genCnf.pcb190.cal_main_off = dati.at(1).toInt();
        }else if(dati.at(0)=="MAIN_RUN"){
            genCnf.pcb190.cal_main_run = dati.at(1).toInt();
        }else if(dati.at(0)=="MAIN_KEEP"){
            genCnf.pcb190.cal_main_keep = dati.at(1).toInt();
        }else if(dati.at(0)=="MAIN_BRK"){
            genCnf.pcb190.cal_main_brk = dati.at(1).toInt();
        }else if(dati.at(0)=="SHIFT_OFF"){
            genCnf.pcb190.cal_shift_off = dati.at(1).toInt();
        }else if(dati.at(0)=="SHIFT_RUN"){
            genCnf.pcb190.cal_shift_run = dati.at(1).toInt();
        }else if(dati.at(0)=="SHIFT_KEEP"){
            genCnf.pcb190.cal_shift_keep = dati.at(1).toInt();
        }else if(dati.at(0)=="SHIFT_BRK"){
            genCnf.pcb190.cal_shift_brk = dati.at(1).toInt();
        }else if(dati.at(0)=="MAX_MAIN_OFF"){
            genCnf.pcb190.cal_max_main_off = dati.at(1).toInt();
        }else if(dati.at(0)=="MAX_MAIN_RUN"){
            genCnf.pcb190.cal_max_main_run = dati.at(1).toInt();
        }else if(dati.at(0)=="MAX_MAIN_KEEP"){
            genCnf.pcb190.cal_max_main_keep = dati.at(1).toInt();
        }else if(dati.at(0)=="MAX_MAIN_BRK"){
            genCnf.pcb190.cal_max_main_brk = dati.at(1).toInt();
        }else if(dati.at(0)=="MAX_SHIFT_OFF"){
            genCnf.pcb190.cal_max_shift_off = dati.at(1).toInt();
        }else if(dati.at(0)=="MAX_SHIFT_RUN"){
            genCnf.pcb190.cal_max_shift_run = dati.at(1).toInt();
        }else if(dati.at(0)=="MAX_SHIFT_KEEP"){
            genCnf.pcb190.cal_max_shift_keep = dati.at(1).toInt();
        }else if(dati.at(0)=="MAX_SHIFT_BRK"){
            genCnf.pcb190.cal_max_shift_brk = dati.at(1).toInt();
        }else if(dati.at(0)=="MIN_MAIN_RUN"){
            genCnf.pcb190.cal_min_main_run = dati.at(1).toInt();
        }else if(dati.at(0)=="MIN_MAIN_KEEP"){
            genCnf.pcb190.cal_min_main_keep = dati.at(1).toInt();
        }else if(dati.at(0)=="MIN_MAIN_BRK"){
            genCnf.pcb190.cal_min_main_brk = dati.at(1).toInt();
        }else if(dati.at(0)=="MIN_SHIFT_RUN"){
            genCnf.pcb190.cal_min_shift_run = dati.at(1).toInt();
        }else if(dati.at(0)=="MIN_SHIFT_KEEP"){
            genCnf.pcb190.cal_min_shift_keep = dati.at(1).toInt();
        }else if(dati.at(0)=="MIN_SHIFT_BRK"){
            genCnf.pcb190.cal_min_shift_brk = dati.at(1).toInt();
        }

    }
    file.close();


    return;
}

// Lettura coefficienti di calibrazione rilettura dei kV
// Questo files √® esterno al tubo perch√® non dipende dal tubo selezionato
// ma dalla PCB190 e dalle caratteristiche delk generatore stesso
void Generatore::readTubeKvReadCalibrationFile(){
    QList<QString> dati;

    genCnf.pcb190.kV_CALIB = 204;
    genCnf.pcb190.kV_OFS = -1420;

    QString tubeName = QString("/resource/config/kvcalib.dat");

    // Lettura file di configurazione generale del tubo
    QFile file(tubeName.toAscii());
    if(!file.exists()){
        saveTubeKvReadCalibrationFile(); // Salva i valori di default
        return;
    }

    // Se il file non si dovesse aprire (perch√®??) allora  parte con i valori di default
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    while(1)
    {
        dati = Config::getNextArrayFields(&file);
        if(dati.isEmpty()) break;

        // Se il dato non √® corretto non lo considera
        if(dati.size()!=2) continue;

        // Sblocco/Blocco compressore
        if(dati.at(0)=="kV_CALIB"){
            genCnf.pcb190.kV_CALIB = dati.at(1).toUShort();
        }else if(dati.at(0)=="kV_OFS"){
            genCnf.pcb190.kV_OFS = dati.at(1).toShort();
        }
    }
    file.close();
    return;
}

// Salvataggio coefficienti di calibrazione rilettura dei kV
void Generatore::saveTubeKvReadCalibrationFile(){

    QString tubeName = QString("/resource/config/kvcalib.dat");

    QFile file(tubeName.toAscii());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QString frame = QString("<kV_CALIB,%1>  //  coeff. * 1000 \n").arg(genCnf.pcb190.kV_CALIB);
    file.write(frame.toAscii().data());

    frame = QString("<kV_OFS,%1>  //  Offset * 1000 \n").arg(genCnf.pcb190.kV_OFS);
    file.write(frame.toAscii().data());

    file.close();
    file.flush();

    pSysLog->log("CONFIG: KV READ CALIBRATION FILE ");
    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());

    return ;

}


void Generatore::saveTubeFilamentFile(QString tubeDir){

    // Non √® possibile salvare i dati nel template
    if(tubeDir.contains("TEMPLATE")) return;


    QString tubeName = tubeDir + QString("filament.dat");

    QFile file(tubeName.toAscii());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QString frame = QString("<ILIM,%1>  //  Corrente limite (A)\n").arg(genCnf.filData.IFILlimit);
    file.write(frame.toAscii().data());

    frame = QString("<IMAX,%1>  //  Corrente max impostabile (A)\n").arg(genCnf.filData.IFILmax);
    file.write(frame.toAscii().data());

    frame = QString("<IWARM,%1>  //  Corrente riscaldamento (A)\n").arg(genCnf.filData.IFILwarm);
    file.write(frame.toAscii().data());

    frame = QString("<IDAC,%1>  //  DAC calibrato corrente di riscaldamento\n").arg(genCnf.filData.IFILdac);
    file.write(frame.toAscii().data());

    file.close();
    file.flush();

    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());

    // Calcolo dei parametri
    genCnf.pcb190.IFIL_DAC_WARM = genCnf.filData.IFILdac;
    genCnf.pcb190.IFIL_LIMIT = genCnf.filData.IFILlimit * 20.838; // 0.4086 * I * 255/5
    genCnf.pcb190.IFIL_MAX_SET = (unsigned short) (genCnf.filData.IFILmax * (float) genCnf.filData.IFILdac / genCnf.filData.IFILwarm);

    pSysLog->log("CONFIG: TUBE FILAMENT FILE - "+ tubeDir);
    return ;
}


void Generatore::saveTubeStatisticsFile(QString tubeDir){

    if(!statChanged) return;

    QString tubeName = tubeDir + QString("statistics.dat");

    QFile file(tubeName.toAscii());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QString frame = QString("<SHOTS,%1>  //  Numero totali shots\n").arg(numShots);
    file.write(frame.toAscii().data());

    frame = QString("<CUM_J,%1>  //  Energia accumulata\n").arg(cumulatedJ);
    file.write(frame.toAscii().data());

    frame = QString("<NTOMO,%1>  //  Numero esami in Tomo\n").arg(nTomo);
    file.write(frame.toAscii().data());

    frame = QString("<NSTANDARD,%1>  //  Numero esami in 2D\n").arg(nStandard);
    file.write(frame.toAscii().data());

    // Salva il valore corrente dell'Anodo HU se necessario
    if(anodeHU>0){
        int year = anodeHuTime.date().year();
        int month = anodeHuTime.date().month();
        int day = anodeHuTime.date().day();
        int sec = anodeHuTime.time().second();
        int mn = anodeHuTime.time().minute();
        int hr = anodeHuTime.time().hour();

        frame = QString("<ANODE_HU,%1,%2,%3,%4,%5,%6,%7>  //  Ultima registrazione HU anode\n").arg(anodeHUSaved).arg(year).arg(month).arg(day).arg(hr).arg(mn).arg(sec);
        file.write(frame.toAscii().data());
    }

    file.close();
    file.flush();

    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());

    statChanged = false;
    return ;
}


/*
 *  Lettura Offsets sui KV
 *  Il valore degli offset deve essere espresso in percentuale rispetto
 *  all'incremento DAC di un KV.
 *  Il valore 1 corrisponde a 1% di Delta DAC per avere un KV (tipicamente 80)
 *  Il valore deve essere natuaralmente approssimato all'intero pi√π prossimo
 */
bool Generatore::readTubeKvOffsetFile(QString tubeDir)
{
    int i;
    QByteArray array;

    QString tubeName = tubeDir + QString(_TUBE_KV_OFFSET_NAME);

    // Lettura file di configurazione generale del tubo
    QFile file(tubeName.toAscii());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return FALSE;

    // Legge Array di Tensioni disponibili
    array = Config::getNextArrayLine(&file);
    if(array.size() != 30) return FALSE;

    // Scansione data per acquisizione offset
    int resto;
    for(i=0;i<array.size();i++)
    {        
        int val = array.at(i);
        if(val<-150) val=0;
        else if(val>150) val = 0;

        // Solo valori interi multipli di 10
        resto = val % 10;
        tube[i].vRef.Voffset= val -resto;
    }

    file.close();

    return TRUE;

}


// Salva i dati per gli offset di un tubo generico
void Generatore::saveTubeKvOffsetFile(QString tubeDir)
{
    int i;
    QByteArray riga;

    // Non √® possibile salvare i dati nel template
    if(tubeDir.contains("TEMPLATE")) return;

    QString tubeName = tubeDir + _TUBE_KV_OFFSET_NAME;

    // Lettura file di configurazione generale del tubo
    QFile file(tubeName.toAscii());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return ;


    file.write(QByteArray("Configurazione Offsets sui KV\n"));
    riga.clear();
    riga.append("<");
    for(i=0; i<_MAX_KV-_MIN_KV+1; i++)
    {
        if(riga.size()==1) riga.append(QString("%1").arg(tube[i].vRef.Voffset).toAscii());
        else riga.append(QString(",%1").arg(tube[i].vRef.Voffset).toAscii());
    }
    riga.append(">\n\n");
    file.write(riga);
    file.close();

    pSysLog->log("CONFIG: TUBE KV OFFSET FILE - " + tubeDir);
    // SYNC
    QString command = QString("sync");
    system(command.toStdString().c_str());
    return ;


}

/* Salva i dati di configurazione del tubo in un Tubo
 * il cui nome √® <filename>
 * Se il file √® gi√  esistente semplicemente lo sovreascrive
 */
bool Generatore::saveTube(QString tubename)
{
    QDir tubeDir(_TUBEPATH);
    QString filename;
    QString sval;
    QString command;
    QByteArray riga;
    int i,ii;

    // Non Ë possibile salvare i dati nel template
    if(tubename.contains("TEMPLATE")) return false;

    // Se il file attualmente selezionato Ë un TEMPLATE
    // allora viene fatta la copia pari pari del contenuto del Template nel nuovo tubo
    if(pConfig->userCnf.tubeFileName.contains("TEMPLATE")){
        // Se il file esiste lo cancella
        // copia tutto il contenuto
        command = QString("rm -r %3/%4").arg(_TUBEPATH).arg(tubename);
        system(command.toStdString().c_str());

        // copia tutto il contenuto
        command = QString("cp -r %1/%2 %3/%4").arg(_TUBEPATH).arg(pConfig->userCnf.tubeFileName).arg(_TUBEPATH).arg(tubename);
        system(command.toStdString().c_str());

        command = QString("sync");
        system(command.toStdString().c_str());
        return true;
    }

    // Se il tubo non esiste allora crea la directory
    if(tubeDir.exists(tubename)==FALSE)
    {
        // Se il file non esiste viene creato ex-novo
        tubeDir.mkdir(tubename);
    }

    // Entra nella directory del tubo
    tubeDir.cd(tubename);

    // Eliminazione di tutto il contenuto della directory
    //command = QString("rm %1/%2/*.*").arg(_TUBEPATH).arg(tubename);
    //system(command.toStdString().c_str());

    //____________________________________________________________
    // Creazione/modifica file tube.cnf
    filename = QString("%1/%2/tube.cnf").arg(_TUBEPATH).arg(tubename);
    QFile file(filename.toAscii());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() <<"IMPOSSIBILE APRIRE IL FILE IN SCRITTURA:" << "tube.cnf";
        return FALSE;
    }

    file.write(QByteArray("Configurazione Tensioni selezionabili\n"));
    riga.clear();
    riga.append("<");
    for(i=0; i<_MAX_KV-_MIN_KV+1; i++)
    {
        if(tube[i].vRef.enable)
        {
            if(riga.size()==1) riga.append(QString("%1").arg(i+_MIN_KV).toAscii());
            else riga.append(QString(",%1").arg(i+_MIN_KV).toAscii());
        }
    }
    riga.append(">\n\n");
    file.write(riga);

    // Configurazione Starter
    if(pConfig->sys.highSpeedStarter){
        file.write(QByteArray("<\"HS\",\"1\">\n"));
    }else{
        file.write(QByteArray("<\"HS\",\"0\">\n"));
    }

    file.write(QByteArray("Configurazione materiali associati ai fuochi\n"));
    riga.clear();
    riga.append(QString("<\"F1\",\"%1\",\"F2\",\"%2\">\n\n").arg(confF1).arg(confF2));
    file.write(riga);

    file.write(QByteArray("Selezione fuoco allo startup\n"));
    if(startupSelectedFSize==FUOCO_SMALL)  sval=QString("P");
    else if(startupSelectedFSize==FUOCO_LARGE) sval=QString("G");
    else sval=QString("");

    riga.clear();
    riga.append(QString("<\"%1\",\"%2\">\n\n").arg(startupSelectedAnodo).arg(sval));
    file.write(riga);
    file.flush();
    file.close();

    //____________________________________________________________________________________
    // CREAZIONE FILES KVXX PER OGNI KV
    for(i=0; i<_MAX_KV-_MIN_KV+1; i++)
    {
            file.setFileName(QString("%1/%2/KV%3.cnf").arg(_TUBEPATH).arg(tubename).arg(i+_MIN_KV).toAscii());
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                qDebug() <<"IMPOSSIBILE APRIRE IL FILE KVXX" ;
                return FALSE;
            }

            file.write(QByteArray("VDAC + Combinazione scambio primari\n"));
            riga.clear();
            riga.append(QString("<\"V\",%1,%2,%3>\n\n").arg(tube[i].vRef.Vdac).arg(tube[i].vRef.SWA).arg(tube[i].vRef.SWB));
            file.write(riga);

            file.write(QByteArray("Definizioni correnti utilizzate nel file\n"));
            for(ii=0; ii<tube[i].iTab.count();ii++)
            {
                // <"I","A","W","G",105,2465>
                riga.clear();
                if(tube[i].iTab[ii].fsize==FUOCO_SMALL)   sval=QString("P");
                else   sval=QString("G");
                riga.append(QString("<\"I\",\"%1\",\"%2\",\"%3\",%4,%5,%6>\n").arg(tube[i].iTab[ii].sym).arg(tube[i].iTab[ii].anode).arg(sval).arg(tube[i].iTab[ii].In).arg(tube[i].iTab[ii].Idac).arg(tube[i].iTab[ii].derivata));
                file.write(riga);
            }

            //<"T","W",100,2450>
            file.write(QByteArray("\nCorrente dedicata alla Tomografia\n"));
            for(ii=0; ii< tube[i].tomo.count(); ii++)
            {
                riga.clear();
                riga.append(QString("<\"T\",\"%1\",%2,%3,%4>\n").arg(tube[i].tomo[ii].anode).arg(tube[i].tomo[ii].In).arg(tube[i].tomo[ii].Idac).arg(tube[i].tomo[ii].derivata));
                file.write(riga);
            }

            file.write(QByteArray("Definizioni Intervalli Fuoco Grande\n"));
            for(ii=0;ii<tube[i].mAs.count();ii++)
            {
                if(tube[i].mAs[ii].fsize==FUOCO_LARGE)
                {
                    riga.clear();
                    riga.append(QString("<\"M\",\"%1\",\"G\",%2,%3,\"%4\">\n").arg(tube[i].mAs[ii].anode).arg(tube[i].mAs[ii].da).arg(tube[i].mAs[ii].a).arg(tube[i].mAs[ii].sym));
                    file.write(riga);
                }
            }

            file.write(QByteArray("\nDefinizioni Intervalli Fuoco Piccolo\n"));
            for(ii=0;ii<tube[i].mAs.count();ii++)
            {
                if(tube[i].mAs[ii].fsize==FUOCO_SMALL)
                {
                    riga.clear();
                    riga.append(QString("<\"M\",\"%1\",\"P\",%2,%3,\"%4\">\n").arg(tube[i].mAs[ii].anode).arg(tube[i].mAs[ii].da).arg(tube[i].mAs[ii].a).arg(tube[i].mAs[ii].sym));
                    file.write(riga);
                }
            }

            file.write(QByteArray("\nDefinizioni Intervalli Starter\n"));
            for(ii=0;ii<tube[i].starter.count();ii++)
            {
                riga.clear();
                if(tube[i].starter[ii].fsize == FUOCO_LARGE) sval = QString("G");
                else sval = QString("P");
                riga.append(QString("<\"S\",\"%1\",\"%2\",%3,%4>\n").arg(tube[i].starter[ii].anode).arg(sval).arg(tube[i].starter[ii].da).arg(tube[i].starter[ii].a));
                file.write(riga);
            }


            file.write("\n");
            file.flush();
            file.close();
    }

    //____________________________________________________________
    // Creazione/modifica file calibrated.cnf
    filename = QString("%1/%2/%3").arg(_TUBEPATH).arg(tubename).arg(_TUBE_CALIB_NAME);
    file.setFileName(filename.toAscii());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() <<"IMPOSSIBILE APRIRE IL FILE IN SCRITTURA:" << _TUBE_CALIB_NAME;
        return FALSE;
    }

    // Ripassa tutti i KV
    char fsize;
    for(i=0; i<_MAX_KV-_MIN_KV+1; i++)
    {
        if(tube[i].vRef.enable)
        {
            for(ii=0; ii<tube[i].iTab.count();ii++)
            {
                if(tube[i].iTab[ii].INcalib!=-1)
                {
                   if(tube[i].iTab[ii].fsize==Generatore::FUOCO_LARGE) fsize='G';
                   else fsize='P';
                   riga = QString("<S,%1,%2,%3,%4,%5>\n").arg((int) i+_MIN_KV).arg(tube[i].iTab[ii].anode).arg(fsize).arg(tube[i].iTab[ii].In).arg(tube[i].iTab[ii].INcalib).toAscii();
                   file.write(riga);
                }
            }

            for(ii=0; ii<tube[i].tomo.count();ii++)
            {
                if(tube[i].tomo[ii].INcalib!=-1)
                {
                   riga = QString("<T,%1,%2,G,%3,%4>\n").arg((int) i+_MIN_KV).arg(tube[i].tomo[ii].anode).arg(tube[i].tomo[ii].In).arg(tube[i].tomo[ii].INcalib).toAscii();
                   file.write(riga);
                }
            }
        }
    }

    file.flush();
    file.close();

    // Salvataggio di tutti gli altri files necessari
    QString tube = QString(_TUBEPATH) + QString("/") + tubename +  QString("/");

    // Salva il file di offset ma NON durante la calibrazione dei KV
    // laddove gli offset devono essere azzerati.
    if((pConsole!=NULL) &&(!pConsole->isCalibKvMode()))   saveTubeKvOffsetFile(tube);

    saveTubeFilamentFile(tube);

    // Scrive la revisione del tubo
    setTubeRevision(tube);

    pSysLog->log("CONFIG: TUBE CONFIGURATION FILE - " + tubename);

    // SYNC
    command = QString("sync");
    system(command.toStdString().c_str());
    return TRUE;

}

/*
 *  Rinfresca il timer per mantenere vivo lo starter
 */
void Generatore::refreshStarter(void)
{
    if(timerStarter==0) timerStarter = startTimer(pConfig->userCnf.starterTimeout*1000);
    else
    {
        killTimer(timerStarter);
        timerStarter = startTimer(pConfig->userCnf.starterTimeout*1000);
    }
}

void Generatore::timerEvent(QTimerEvent* ev)
{
    // E' scaduto il timer, viene fermata la rotazione del tubo
    if(ev->timerId()==timerStarter)
    {
        emit sigStopStarter();
        killTimer(timerStarter);
        timerStarter = 0;
        return;
    }else if(ev->timerId()==timerUpdateAnodeHU) // Evento di UpdateHU
    {
        getCurrentHU();

        // UpdateHU ogni 10 secondi se necessario
        if(anodeHU==0){
            killTimer(timerUpdateAnodeHU);
            timerUpdateAnodeHU=0;
        }

        return;
    }

}

void Generatore::stopStarterSlot(void)
{
unsigned char data;

    // Azzera la condizione di starter ad alta velocit‡
    starterHS = false;

    if(timerStarter){
        killTimer(timerStarter);
        timerStarter = 0;
    }

    // Comando a M4 per gestione Starter
    if(pConfig->userCnf.starter_brake) data = 0;
    else data = 3;
    pConsole->pGuiMcc->sendFrame(MCC_STARTER,1,&data, 1);

}

bool Generatore::readKvFile(unsigned char kV, QString path)
{
    QByteArray risultato;
    QString    tag;
    QString kvName;
    int j;

    kvName=QString("%1%2%3%4").arg(path).arg(_KVPREFIX).arg(kV).arg(_KVSUFFIX);

    // Apertura file di configurazione kV
    QFile file(kvName.toAscii());
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) return FALSE;

    // Scansione righe
    while(!file.atEnd())
    {
        risultato = Config::getNextValidLine(&file);
        if(risultato.isEmpty()) break;

        // Ricerca il Tag sulla posizione corrente
        j=0;tag=Config::getNextTag(&risultato,&j,false);


        if(tag=="V")
        {
            // Impostazione dati relativi ai KV
            if(getVLine(&risultato, &j, kV)==FALSE)
            {
                qDebug() << "ERRORE LETTURA V";
                return FALSE;
            }
        }else if(tag=="I")
        {
            // Dati relativi alle correnti di riferimento
            if(getILine(&risultato, &j, kV)==FALSE)
            {
                qDebug() << "ERRORE LETTURA I";
                return FALSE;
            }
        }else if(tag=="M")
        {
            // Dati relativi ai carichi
            if(getMLine(&risultato, &j, kV)==FALSE)
            {
                qDebug() << "ERRORE LETTURA M";
                return FALSE;
            }
        }else if(tag=="S")
        {
            // Dati relativi all'uso dello Starter
            if(getSLine(&risultato, &j, kV)==FALSE)
            {
                qDebug() << "ERRORE LETTURA S";
                return FALSE;
            }
        }else if(tag=="T")
        {
            // Dati relativi alla corrente Tomo
            if(getTLine(&risultato, &j, kV)==FALSE)
            {
                qDebug() << "ERRORE LETTURA T";
                return FALSE;
            }
        }else
        {
            qDebug() << "ERRORE INVALID TAG";
            return FALSE;
        }

    }


    return TRUE;
}


// Legge la riga relativa al tag V
bool Generatore::getVLine(QByteArray* dato, int* index, unsigned char kV)
{
    QByteArray qval;
    int par1,par2,par3;

    // Legge primo dato fino alla virgola
    qval=Config::getNextParam(dato,index,false);
    if(qval.isEmpty()) return FALSE;
    par1=qval.toInt();

    qval=Config::getNextParam(dato,index,false);
    if(qval.isEmpty()) return FALSE;
    par2=qval.toInt();

    qval=Config::getNextParam(dato,index,true);
    if(qval.isEmpty()) return FALSE;
    par3=qval.toInt();

    // Aggiunge i dati

    tube[kV-_MIN_KV].vRef.Vdac = par1;
    tube[kV-_MIN_KV].vRef.SWA  = par2;
    tube[kV-_MIN_KV].vRef.SWB  = par3;

    return TRUE;
}

// Legge la riga relativa al tag I
//<"I","A","[materiale]","+"[G/P]",105,0,0.xx>
bool Generatore::getILine(QByteArray* dato, int* index, unsigned char kV)
{
    QChar sym;
    QByteArray qval;
    _ITabStr newItem;

    // Legge Simbolo di un carattere max
    qval=Config::getNextTag(dato,index,false);
    if(qval.isEmpty()) return FALSE;
    if(qval.size()>1) return FALSE;
    newItem.sym = qval.at(0);

    // Legge Anodo
    newItem.anode=Config::getNextTag(dato,index,false);
    if((newItem.anode!=W_STR)&&(newItem.anode!=Mo_STR)) return FALSE;

    // Se il Materiale non √® configurato per il tubo
    // esce ma senza dare errore
    if(!isValidAnode(newItem.anode)) return TRUE;

    // Legge fuoco
    qval=Config::getNextTag(dato,index,false);
    if(qval.isEmpty()) return FALSE;
    if(qval=="G") newItem.fsize = FUOCO_LARGE;
    else if(qval=="P") newItem.fsize = FUOCO_SMALL;
    else return FALSE;

    // Legge corrente nominale
    qval=Config::getNextParam(dato,index,false);
    if(qval.isEmpty()) return FALSE;
    newItem.In=qval.toInt();

    // Modifiche relative alla transizione tra revisione 0 ed 1
    if(templateRevision==0){

        // Vecchio tipo: non c'√® la derivata
        qval=Config::getNextParam(dato,index,true);
        if(qval.isEmpty()) return FALSE;
        newItem.Idac=qval.toInt();

        newItem.derivata= 0.3;

    }else{
        // Revisione 1: derivata delle curve di corrente
        qval=Config::getNextParam(dato,index,false);
        if(qval.isEmpty()) return FALSE;
        newItem.Idac=qval.toInt();

        // Legge derivata fino alla fine
        qval=Config::getNextParam(dato,index,true);
        if(qval.isEmpty()) return FALSE;
        newItem.derivata=qval.toFloat();
    }


    // Mette a zero la corrente di calibrazione (letta successivamente)
    newItem.INcalib=-1;

    // Aggiorna i dati
    tube[kV-_MIN_KV].iTab.append(newItem);




    return TRUE;
}

// Legge la riga relativa al tag M
// <"M","Anode","P/G",0,3200,"A">
bool Generatore::getMLine(QByteArray* dato, int* index, unsigned char kV)
{
    QByteArray qval;
    _mAsStr newItem;

    // Legge Anodo
    newItem.anode=Config::getNextTag(dato,index,false);
    if((newItem.anode!=W_STR)&&(newItem.anode!=Mo_STR)) return FALSE;

    // Se il Materiale non √® configurato per il tubo  non procede
    if(!isValidAnode(newItem.anode)) return TRUE;

    // Legge fuoco
    qval=Config::getNextTag(dato,index,false);
    if(qval.isEmpty()) return FALSE;
    if(qval=="G") newItem.fsize = FUOCO_LARGE;
    else if(qval=="P") newItem.fsize = FUOCO_SMALL;
    else return FALSE;

    // Legge intervallo da
    qval=Config::getNextParam(dato,index,false);
    if(qval.isEmpty()) return FALSE;
    newItem.da=qval.toInt();

    // Legge intervallo a
    qval=Config::getNextParam(dato,index,false);
    if(qval.isEmpty()) return FALSE;
    newItem.a=qval.toInt();

    // Legge il simbolo
    qval=Config::getNextTag(dato,index,true);
    if(qval.isEmpty()) return FALSE;
    if(qval.size()>1) return FALSE;
    newItem.sym = qval.at(0);

    // Aggiorna dati
    tube[kV-_MIN_KV].mAs.append(newItem);
    return TRUE;
}

// Legge la riga relativa al tag S
// <"S","Anode","P/G",0,6400>
bool Generatore::getSLine(QByteArray* dato, int* index, unsigned char kV)
{
    QByteArray qval;
    _starterStr newItem;

    // Legge Anodo
    newItem.anode=Config::getNextTag(dato,index,false);
    if((newItem.anode!=W_STR)&&(newItem.anode!=Mo_STR)) return FALSE;

    // Se il Materiale non √® configurato per il tubo  non procede
    if(!isValidAnode(newItem.anode)) return TRUE;

    // Legge fuoco
    qval=Config::getNextTag(dato,index,false);
    if(qval.isEmpty()) return FALSE;
    if(qval=="G") newItem.fsize = FUOCO_LARGE;
    else if(qval=="P") newItem.fsize = FUOCO_SMALL;
    else return FALSE;

    // Legge intervallo da
    qval=Config::getNextParam(dato,index,false);
    if(qval.isEmpty()) return FALSE;
    newItem.da=qval.toInt();

    // Legge intervallo a
    qval=Config::getNextParam(dato,index,true);
    if(qval.isEmpty()) return FALSE;
    newItem.a=qval.toInt();

    // Aggiunge i dati
    tube[kV-_MIN_KV].starter.append(newItem);
    return TRUE;
}

// Legge la riga relativa al tag T
// <"T","Anodo", 150,2234,0.x>
bool Generatore::getTLine(QByteArray* dato, int* index, unsigned char kV)
{
    QByteArray qval;
    QString anode;
    int In,Idac;
    float Iderivata;

    // Legge Anodo
    anode=Config::getNextTag(dato,index,false);
    if((anode!=W_STR)&&(anode!=Mo_STR)) return FALSE;

    // Legge In
    qval=Config::getNextParam(dato,index,false);
    if(qval.isEmpty()) return FALSE;
    In=qval.toInt();


    // Modifiche relative alla transizione tra revisione 0 ed 1
    if(templateRevision==0){

        // Vecchio tipo: non c'√® la derivata
        qval=Config::getNextParam(dato,index,true);
        if(qval.isEmpty()) return FALSE;
        Idac=qval.toInt();

         Iderivata = 0.3;

    }else{

        qval=Config::getNextParam(dato,index,false);
        if(qval.isEmpty()) return FALSE;
        Idac=qval.toInt();

        // Legge derivata
        qval=Config::getNextParam(dato,index,true);
        if(qval.isEmpty()) return FALSE;
        Iderivata=qval.toFloat();
    }

    // Aggiunge i dati ma li abilita solo se l'anodo √® configurato
    _TomoStr tomoblk;
    tomoblk.anode = anode;
    tomoblk.In = In;
    tomoblk.Idac = Idac;
    tomoblk.derivata = Iderivata;

    // Mette a zero la corrente di calibrazione (letta successivamente)
    tomoblk.INcalib=-1;


    if((confF1==anode)||(confF2==anode)) tomoblk.enabled = TRUE;
    else tomoblk.enabled = FALSE;
    tube[kV-_MIN_KV].tomo.append(tomoblk);




    return TRUE;
}


/*
 *  La funzione determina se, in funzione del fuoco + anodo + mas + kv selezionati
 *  lo starter debba essere ad alta velocit√ 
 *
 */
bool Generatore::isStarterHS(unsigned char kV, int mAs)
{
    int i;

    if(tube[kV-_MIN_KV].starter.isEmpty()) return FALSE; // Nessun intervallo disponibile

    for(i=0; i<tube[kV-_MIN_KV].starter.size(); i++)
    {
        // Seleziona solo gli intervalli associati al fuoco e anodo selezionati
        if(tube[kV-_MIN_KV].starter.at(i).anode!=selectedAnodo) continue;
        if(tube[kV-_MIN_KV].starter.at(i).fsize!=selectedFSize) continue;
        if(mAs<=tube[kV-_MIN_KV].starter.at(i).da) continue;
        if(mAs>tube[kV-_MIN_KV].starter.at(i).a) continue;
        return TRUE;
    }
    return FALSE; // Nessun intervallo disponibile per questi mAs

}

// Restituisce l'indice della tabella delle correnti associato
// Alla selkezione corrente di mas, kv, fuoco, anodo
// Prima si verifica dati i kV e mAs e Fuoco e Anodo
// se esiste un intervallo compatibile. A Quel punto tramite la lettera
// associata all'intervallo si trova l'indice della tabella iTab..
int Generatore::getITabIndex(unsigned char kV)
{
    int i;
    char sym;

    if(tube[kV-_MIN_KV].mAs.isEmpty()) return -1; // Nessun intervallo disponibile

    // Rilevazione TAG associato all'intervallo
    for(i=0; i<tube[kV-_MIN_KV].mAs.size(); i++)
    {
        // Seleziona solo gli intervalli associati al fuoco e anodo selezionati
        if(tube[kV-_MIN_KV].mAs.at(i).anode!=selectedAnodo) continue;
        if(tube[kV-_MIN_KV].mAs.at(i).fsize!=selectedFSize) continue;
        if(selectedDmAs<=tube[kV-_MIN_KV].mAs.at(i).da) continue;
        if(selectedDmAs>tube[kV-_MIN_KV].mAs.at(i).a) continue;
        sym = tube[kV-_MIN_KV].mAs.at(i).sym;
        break;
    }

    // Si scorre la tabella delle correnti per scoprire quale di queste Ë associata al simbolo
    for(i=0; i<tube[kV-_MIN_KV].iTab.size(); i++){
        if(tube[kV-_MIN_KV].iTab.at(i).sym == sym) return i;
    }

    return -1; // Nessun intervallo disponibile per questi mAs

}



// Restituisce il valore di VDAC come interpolazione lineare tra i valore pi√π prossimi
// disponibili nel file di configurazione
// val rappresenta la selezione corrente;
// *kv conterr√  il valore da utilizzare per reperire il resto dei parametri
// ( Correnti / Mas) con la regola che si utilizzer√  se possibile i valori dei KV
// pi√π vicini alla selezione corrente
// Al valore DAC calibrato viene inoltre aggiunto il valore dell'offset assegnato
// per compensare eventuali variazioni introdotte oltre la calibrazione standard
bool  Generatore::getValidKv(float val, unsigned char* kv, unsigned short* vdac)
{
    unsigned short valL, valH;
    float indice;

    // Controllo coerenza
    if(kv==NULL) return FALSE;
    if((val>(float)_MAX_KV)||(val<(float)_MIN_KV))  return FALSE;


    // Controllo abilitazioni
    valL = (unsigned short) (val);
    valH = valL + 1;

    // Calcola l'indice di vicinanza [0:1]
    if((tube[valL-_MIN_KV].vRef.enable==FALSE)&&(tube[valH -_MIN_KV].vRef.enable==FALSE)) return FALSE;
    if((tube[valL-_MIN_KV].vRef.enable==FALSE)||(tube[valL-_MIN_KV].vRef.Vdac==0)) indice=1.0;
    if((tube[valH-_MIN_KV].vRef.enable==FALSE)||(tube[valH-_MIN_KV].vRef.Vdac==0)) indice=0.0;
    else indice = val - valL;

    if(indice==0)
    {
       *kv = valL;
       *vdac = (unsigned short) (((long) tube[*kv-_MIN_KV].vRef.Vdac) + tube[*kv-_MIN_KV].vRef.Voffset * 80 / 100);
    }else if(indice==1)
    {
        *kv = valH;
        *vdac = (unsigned short) (((long) tube[*kv-_MIN_KV].vRef.Vdac) + tube[*kv-_MIN_KV].vRef.Voffset * 80 / 100);
    }else
    {        
        float dacH = (float) (((float) tube[valH-_MIN_KV].vRef.Vdac) + (float) tube[valH-_MIN_KV].vRef.Voffset * 8 / 10);
        float dacL = (float) (((float) tube[valL-_MIN_KV].vRef.Vdac) + (float) tube[valL-_MIN_KV].vRef.Voffset * 8 / 10);
        float dac = (dacH * indice + dacL * (1-indice));
        if(dac < 0) return FALSE;
        *vdac = (unsigned short) (dac);
        //*vdac = (unsigned short) ((float) tube[valH-_MIN_KV].vRef.Vdac * (indice) + (float) tube[valL-_MIN_KV].vRef.Vdac * (1-indice));

        if(indice>=0.5) *kv = valH;
        else *kv = valL;
    }

    return TRUE;
}



// Restituisce Idac da utilizzare per esposizione di Kv nominali
// BUG-KV
bool Generatore::getIdacForKvCalibration(int kV, QString anodo, int* Idac, int* Inom){
    int i;

    // Azzera inizialmente
    *Idac = 0;
    *Inom = 0;


    i=getIminIndex(kV,anodo,FUOCO_LARGE); // Corrente low speed
    if(i<0) return false; // Nessun intervallo disponibile
    *Idac = tube[kV-_MIN_KV].iTab.at(i).Idac;
    *Inom =  tube[kV-_MIN_KV].iTab.at(i).In;
    return true;

}


// Questa funzione calcola i valori dei DAC sulla base delle
// impostazioni correnti.
// La funzione restituisce false in caso di dati non consentiti
// Restituisce un codice generale di errore
unsigned char Generatore::validateData(void)
{
    int i;
    unsigned char kV;

    if(pConfig->generator_configured == FALSE) return ERROR_TUBE_NOT_CONFIGURED;
    validated = FALSE;

    // In Tomo il fuoco  SEMPRE LARGE
    if((tomoMode)&&(selectedFSize==FUOCO_SMALL)) return ERROR_INVALID_FUOCO;
    if(!isValidFuoco()) return ERROR_INVALID_FUOCO;

    // Verifica mAs impostati
    if(tomoMode)
    {
        //if((selectedmAsTomo==0) || (selectedmAsTomo>_MAX_MAS*10)) return ERROR_INVALID_MAS;
        if((selectedmAsTomo==0) || (selectedmAsTomo>_MAX_MAS)) return ERROR_INVALID_MAS;

    }else
    {
        if((selectedDmAs==0) || (selectedDmAs>_MAX_MAS*10)) return ERROR_INVALID_MAS;
    }


    /*
     *  I KV selezionati possono essere a virgola mobile mentre quelli configurati sono a valori interi.
     *  La regola che verr√  adottata sar√  quella di impostare i VDAC come interpolazione tra i valori
     *  discreti pi√π prossimi.
     *  I valori di corrente saranno selezionati considerando i KV interi pi√π vicini.
     */
    // Cerca i Kv a partire dal valore float richiesto
    if(getValidKv(selectedKv, &kV, &selectedVdac)==FALSE) return ERROR_INVALID_KV;
    if(selectedVdac > 4095) return ERROR_INVALID_GEN_CONFIG;
    if(selectedVdac == 0) return ERROR_NOT_CALIBRATED_KV;

    // Impostazione scambio primari
    SWA = tube[kV-_MIN_KV].vRef.SWA;
    SWB = tube[kV-_MIN_KV].vRef.SWB;

    // Verifica dell'intervallo richiesto ma solo in 2D
    if(tomoMode)
    {
        // Non Ë possibile effettuare la Tomo senza starter
        if(pConfig->sys.highSpeedStarter == FALSE)
              return ERROR_INVALID_GEN_CONFIG;

        selectedIdac = 0;
        selectedIn = 0;
        for(i=0; i<tube[kV-_MIN_KV].tomo.count();i++)
        {
            if((tube[kV-_MIN_KV].tomo[i].anode == selectedAnodo) &&(tube[kV-_MIN_KV].tomo[i].enabled))
            {
                selectedIn = tube[kV-_MIN_KV].tomo[i].In;
                selectedIdac = tube[kV-_MIN_KV].tomo[i].Idac;
                break;
            }
        }

        if(selectedIn > _MAX_In) return ERROR_INVALID_GEN_CONFIG;
        if(selectedIdac > genCnf.pcb190.IFIL_MAX_SET) return ERROR_INVALID_GEN_CONFIG;
        if(selectedIdac == 0) return ERROR_NOT_CALIBRATED_I;

        starterHS=TRUE;

        // Conversione di dmAs per il driver PCB190 (50 unit√  per mAs)
        selectedmAsDacTomo = (unsigned short) (selectedmAsTomo * 50.0);//selectedmAsTomo * 5;

        // Timeout massimo finestra di integrazione
        timeoutExp = 3; // 300ms

        // In 3D il limite inferiore √® posto al 50% della corrente nominale
        // Ed il limite superiore non viene fissato. La protezione avviene grazie al timeout
        // massimo di 300ms
        minI = convertPcb190IanodeToRaw(selectedIn / 2);
        maxI = 255;

    }else
    {
        /*   I dati di corrente da utilizzare in funzione dei mas sono determinati dalla seguente funzione
         *   tube[kV-_MIN_KV].mas(anodo+fuoco+mas) -> sym   DETERMINA IL SYMBOLO
         *   iTab(sym) -> Idac,In                           DAL SIMBOLO SI OTTENGONO I VALORI DI CORRENTE
         *
         */
        i=getITabIndex(kV);
        if(i<0) return ERROR_INVALID_MAS; // Nessun intervallo disponibile

        // Carica i valori di corrente cercati e controllo valori
        float derivata;
        selectedIn = tube[kV-_MIN_KV].iTab.at(i).In;    // Corrente nominale
        selectedIdac = tube[kV-_MIN_KV].iTab.at(i).Idac;    // Corrente nominale
        if(selectedIn > _MAX_In) return ERROR_INVALID_GEN_CONFIG;
        if(selectedIdac > genCnf.pcb190.IFIL_MAX_SET) return ERROR_INVALID_GEN_CONFIG;
        if(selectedIdac == 0) return ERROR_NOT_CALIBRATED_I;
        derivata = tube[kV-_MIN_KV].iTab.at(i).derivata;

        // Carica le impostazioni relative allo starter:
        if(pConfig->sys.highSpeedStarter == FALSE) starterHS=FALSE;
        else{
            if(selectedFSize==FUOCO_SMALL) starterHS=TRUE;        // Alta Velocit√
            else if((!aecMode)&&(pGeneratore->selectedDmAs>=400*10)) starterHS=TRUE;    // Alta Velocit√
            else starterHS=FALSE;
        }

        // Conversione di dmAs per il driver PCB190 (50 unit√  per mAs)
        selectedmAsDac = selectedDmAs * 5;

        // Timeout dell'esposizione
        timeoutExp = 50;

        // In 2D il limite inferiore √® posto al 50% della corrente nominale
        // Ed il limite superiore ad una corrente che dipende dalla gamma di tensione
        minI = convertPcb190IanodeToRaw(selectedIn / 2);

        if(selectedFSize==FUOCO_SMALL){
                if(selectedKv<=35) maxI = convertPcb190IanodeToRaw(75);
                else if(selectedKv<=40) maxI = convertPcb190IanodeToRaw(70);
                else if(selectedKv<=45) maxI = convertPcb190IanodeToRaw(60);
                else maxI = convertPcb190IanodeToRaw(50);
        }else{
            // LIMITI FUOCO GRANDE
            if(starterHS){

                // Caso per gestire i 640 mAs: la corrente viene aumentata di 10mA
                // rispetto al valore calibrato e il timeout, per evitare errori antipatici
                // viene portato a 6 secondi
                if(selectedmAsDac>20000){
                    timeoutExp = 60;

                    // Aggiunta di circa 10ms alla corrente nominale
                    selectedIdac += (unsigned short)(10.0 / derivata);
                    if(selectedIdac > genCnf.pcb190.IFIL_MAX_SET)
                        selectedIdac = genCnf.pcb190.IFIL_MAX_SET;

                }

                if(selectedKv<=35) maxI = 255;
                else if(selectedKv<=40) maxI = convertPcb190IanodeToRaw(190);
                else if(selectedKv<=45) maxI = convertPcb190IanodeToRaw(170);
                else maxI = convertPcb190IanodeToRaw(150);
            }else{

                // Caso Manuale oltre i 400 mAs ma senza lo starter (la corrente nn puÚ essere maggiorata
                if((!aecMode)&&(pGeneratore->selectedDmAs>=400*10)) timeoutExp = 60;

                if(selectedKv<=32) maxI = convertPcb190IanodeToRaw(170);
                else if(selectedKv<=35) maxI = convertPcb190IanodeToRaw(150);
                else if(selectedKv<=40) maxI = convertPcb190IanodeToRaw(130);
                else if(selectedKv<=45) maxI = convertPcb190IanodeToRaw(110);
                else  maxI = convertPcb190IanodeToRaw(100);
            }
        }
    }

    // Caricamento valori limite
    maxV = convertPcb190KvToRaw(kV+3);
    minV = convertPcb190KvToRaw(kV-3);


    validated=TRUE;
    return 0; // Nessun errore rilevato
}

// Cerca l'indice corrispondente alla corrente minima
// per i parametri indicati
int Generatore::getIminIndex(int kV, QString anode, int fsize){

    int imin=255;
    int i=-1;
    for(int j=0; j<tube[kV-_MIN_KV].iTab.size(); j++){
        if(tube[kV-_MIN_KV].iTab.at(j).fsize != fsize) continue;
        if(tube[kV-_MIN_KV].iTab.at(j).anode!= anode) continue;
        if(tube[kV-_MIN_KV].iTab.at(j).In <= imin){
            imin = tube[kV-_MIN_KV].iTab.at(j).In;
            i = j;
        }
    }
    return i;
}

// Cerca l'indice corrispondente alla corrente massima
// per i parametri indicati
int Generatore::getImaxIndex(int kV, QString anode, int fsize){

    int imax=0;
    int i=-1;
    for(int j=0; j<tube[kV-_MIN_KV].iTab.size(); j++){
        if(tube[kV-_MIN_KV].iTab.at(j).fsize != fsize) continue;
        if(tube[kV-_MIN_KV].iTab.at(j).anode != anode) continue;
        if(tube[kV-_MIN_KV].iTab.at(j).In >= imax){
            imax = tube[kV-_MIN_KV].iTab.at(j).In;
            i = j;
        }
    }
    return i;
}

// Attenzione: con isPre==true bisogna considerare la corrente a bassa velocit‡
unsigned char Generatore::validateAnalogData(unsigned char modo, bool calibMode, bool isPre)
{
    int i;
    unsigned char kV;

    if(pConfig->generator_configured  == FALSE) return ERROR_TUBE_NOT_CONFIGURED;
    validated = FALSE;

    if(!isValidFuoco()) return ERROR_INVALID_FUOCO;
    if((selectedDmAs==0) || (selectedDmAs>_MAX_MAS*10)) return ERROR_INVALID_MAS;

    // Cerca i Kv a partire dal valore float richiesto
    if(getValidKv(selectedKv, &kV, &selectedVdac)==FALSE) return ERROR_INVALID_KV;
    if(selectedVdac > 4095) return ERROR_INVALID_GEN_CONFIG;
    if(selectedVdac == 0) return ERROR_NOT_CALIBRATED_KV;

    // Impostazione scambio primari
    SWA = tube[kV-_MIN_KV].vRef.SWA;
    SWB = tube[kV-_MIN_KV].vRef.SWB;

    // Impostazione Starter Alta velocit‡
    if(pConfig->sys.highSpeedStarter == FALSE) starterHS=FALSE;
    else{

        if(calibMode){
            // In calibrazione lo starter Ë sempre attivo
            starterHS = TRUE;
        }else{
            // In modalit‡ operativa lo starter si decide sulla base delle regole di attivazione
            if(selectedFSize==FUOCO_SMALL) starterHS=TRUE;
            else if(modo!=ANALOG_TECH_MODE_MANUAL){
                // La decisione sullo starter avviene solo nel pre impulso:
                // Se perÚ l'alta velocit‡ Ë gi‡ attiva, rimane in HS
                if((isPre)&&(!starterHS)){
                    // In Automatico lo starter viene selezionato sulla base dello spessore
                    if(!pCompressore->isCompressed()) starterHS=TRUE; // Se non Ë compresso allora Ë sempre attivo per sicurezza
                    else if(pCompressore->breastThick >= 50) starterHS=TRUE; // Per seni molto spessi
                    else starterHS=FALSE;
                }
            }else{
                // Modalit‡ manuale: se si trova gi‡ in alta velocit‡, ci rimane
                if(!starterHS){
                    if(pGeneratore->selectedDmAs >= 400*10) starterHS=TRUE;
                    else starterHS=FALSE;
                }
            }
        }
    }

    // Impostazione delle correnti di esercizio

    // Tubi a bassa velocit‡
    if(pConfig->sys.highSpeedStarter == FALSE){
        i=getITabIndex(kV); // Correnti determinate dall'intervallo mAs selezionato
    }else if(isPre){
        // Se si tratta di un pre impulso seleziona sempre la corrente a bassa velocit‡
        i=getIminIndex(kV,selectedAnodo,selectedFSize); // Corrente low speed
    }else if(calibMode){
        // In calibrazione per risparmiare il tubo meglio usare correnti basse
        i=getIminIndex(kV,selectedAnodo,selectedFSize); // Corrente low speed
    }else if(!starterHS){
        // Per una corrente di impulso, con bassa velocit‡ seleziona la corrente minore
        i=getIminIndex(kV,selectedAnodo,selectedFSize); // Corrente low speed
    }else{
        // Per una corrente di impulso, con alta velocit‡ seleziona la corrente maggiore
        i=getImaxIndex(kV,selectedAnodo,selectedFSize); // Corrente high speed
    }
    if(i<0) return ERROR_INVALID_MAS; // Nessun intervallo disponibile

    // Carica i valori di corrente cercati e controllo valori
    float derivata;
    selectedIn = tube[kV-_MIN_KV].iTab.at(i).In;    // Corrente nominale
    selectedIdac = tube[kV-_MIN_KV].iTab.at(i).Idac;    // Corrente nominale
    if(selectedIn > _MAX_In) return ERROR_INVALID_GEN_CONFIG;
    if(selectedIdac > genCnf.pcb190.IFIL_MAX_SET) return ERROR_INVALID_GEN_CONFIG;
    if(selectedIdac == 0) return ERROR_NOT_CALIBRATED_I;
    derivata = tube[kV-_MIN_KV].iTab.at(i).derivata;



    // Conversione di dmAs per il driver PCB190
    selectedmAsDac = selectedDmAs * 5;

    // Timeout dell'esposizione
    timeoutExp = 100;

    // Diagnostica corrente inferiore al 50% della corrente nominale
    // Diagnostica corrente superiore ad una corrente che dipende dalla gamma di tensione
    minI = convertPcb190IanodeToRaw(selectedIn / 2);
    if(selectedFSize==FUOCO_SMALL){
        maxI = convertPcb190IanodeToRaw(selectedIn+10);
    }else{
        maxI = convertPcb190IanodeToRaw(selectedIn+20);
    }


    // Caricamento valori limite
    maxV = convertPcb190KvToRaw(kV+3);
    minV = convertPcb190KvToRaw(kV-3);


    validated=TRUE;
    return 0; // Nessun errore rilevato
}

bool Generatore::setmAs(double mAs)
{
    validated = FALSE;
    selectedDmAs = 0;

    if(mAs > (double) _MAX_MAS ) return FALSE;

    selectedDmAs = (unsigned int) (mAs * 10);

    return TRUE;
}

bool Generatore::setMasTomo(unsigned short index, double mAs)
{
    validated = FALSE;
    selectedmAsTomo = 0;

    if(mAs > (float) _MAX_MAS ) return FALSE;

    selectedmAsTomo = mAs;//mAs * 10;

    return TRUE;
}


bool Generatore::setkV(float kV)
{
    validated = FALSE;
    selectedKv = 0;
    if(kV > (float) max_selectable_kv) return FALSE;
    if(kV > _MAX_KV ) return FALSE;
    if(kV < _MIN_KV ) return FALSE;
    selectedKv = kV;

    return TRUE;
}

/*
 *  Attivazione Starter IAE:
 *  stat = 0 -> OFF
 *  stat = 1 -> LS
 *  stat = 2 -> HS
 */
void Generatore::setStarter(unsigned char stat)
{
    unsigned char data;

    if(stat) refreshStarter();
    data = stat;
    pConsole->pGuiMcc->sendFrame(MCC_STARTER,1,&data, 1);

}


/*
  BUFFER[0] == Fuoco da attivare:
  - 0 = F1G
  - 1 = F2G
  - 2 = F1P
  - 3 = F2P

  NOTIFY: BUFFER[0] = RISULTATO
*/
bool Generatore::updateFuoco(void)
{
    unsigned char data;

    if(confF1==selectedAnodo)
    {
        if(selectedFSize==FUOCO_SMALL)  data = 2;
        else if(selectedFSize==FUOCO_LARGE) data = 0;
        else return FALSE;

    }if(confF2==selectedAnodo)
    {
        if(selectedFSize==FUOCO_SMALL)  data = 3;
        else if(selectedFSize==FUOCO_LARGE) data = 1;
        else return FALSE;
    }    

    // Invio comando
    if(pConsole->pGuiMcc->sendFrame(MCC_SET_FUOCO,1,&data, 1)==FALSE) return FALSE;


    //connect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(updateFuocoNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
    return TRUE;
}

/*
 *  Comanda lo spegnimento del fuoco
 */
void Generatore::fuocoOff(void)
{
    unsigned char data;
    data=100; // Codice di spegnimento
    if(pConsole->pGuiMcc->sendFrame(MCC_SET_FUOCO,1,&data, 1)==TRUE) filOn=FALSE;
}


/*
 *  ATTENZIONE! QUESTA FUNZIONE E' STATA DISMESSA
 */
void Generatore::updateFuocoNotify(unsigned char id, unsigned char mcccode, QByteArray buffer)
{
    /*
     *  buffer[0] =  Esito
     */
    if(id!=1) return;
    if(mcccode!=MCC_SET_FUOCO) return;
    QObject::disconnect(pConsole,SIGNAL(mccGuiNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(updateFuocoNotify(unsigned char,unsigned char,QByteArray)));

    if(buffer.at(0)==0)
    {
        // Notifica l'errore a monitor
        PageAlarms::activateNewAlarm(_DB_ALLARMI_ALR_GEN, GEN_SET_FUOCO, TRUE);
    }else
    {
        filOn=TRUE;
    }
}

unsigned char Generatore::convertPcb190Kv(unsigned char val)
{
    float raw;
    unsigned short lowRaw;
    float k  = (float) genCnf.pcb190.kV_CALIB / 1000.0;
    float cost  = (float) genCnf.pcb190.kV_OFS / 1000.0;

    raw = (float)val * k + cost;
    lowRaw = (unsigned short) raw;

    if((raw - (float) lowRaw) >0.5) return (unsigned char)lowRaw+1;
    else return (unsigned char)lowRaw;

}

float Generatore::convertPcb190Kv(unsigned char val, float k_corr)
{
    float k  = (float) genCnf.pcb190.kV_CALIB / 1000.0;
    float cost  = (float) genCnf.pcb190.kV_OFS / 1000.0;

    return  ((float)val * k + cost) *  k_corr;
}

// Restituisce il valore che la PCB190 dovrebbe leggere in corrispondenza dei kV nominali
// I valori limite sono 1 e 254, per poter agevolmente associare questi valori
// ai limiti di controllo
unsigned char Generatore::convertPcb190KvToRaw(unsigned short kV)
{
    float raw;
    unsigned short lowRaw;
    unsigned short risultato;
    float k  = (float) genCnf.pcb190.kV_CALIB / 1000.0;
    float cost  = (float) genCnf.pcb190.kV_OFS / 1000.0;

    raw = ((float) kV - cost) / k;
    lowRaw = (unsigned short) raw;

    if((raw - (float) lowRaw) >0.5) risultato = lowRaw+1;
    else risultato = lowRaw;

    if(risultato>255) risultato = 255;
    return (unsigned char) risultato;
}


unsigned char Generatore::convertPcb190Ianode(unsigned char val)
{
    float raw;
    unsigned short lowRaw;

    raw = (float)val * 200.0/255.0;
    lowRaw = (unsigned short) raw;

    if((raw - (float) lowRaw) >0.5) return (unsigned char) lowRaw+1;
    else return (unsigned char) lowRaw;
}

// Restituisce il valore che la PCB190 dovrebbe leggere in corrispondenza dell'anodica richiesta
// I valori limite sono 1 e 254, per poter agevolmente associare questi valori
// ai limiti di controllo
unsigned char Generatore::convertPcb190IanodeToRaw(unsigned short Ianode)
{
    float raw;
    unsigned short lowRaw;
    unsigned short risultato;

    raw = ((float) Ianode ) * 255.0 / 200.0;
    lowRaw = (unsigned short) raw;

    if((raw - (float) lowRaw) >0.5) risultato = lowRaw+1;
    else risultato = lowRaw;

    if(risultato>254) risultato = 254;
    else if(risultato==0) risultato = 1;

    return (unsigned char) risultato;
}

/*
 *  Questa funzione seleziona con un criterio definito
 *  quale sia l'anodo da utilizzare tra quelli configurati.
 *
 *  La regola generale √® che si sceglie confF1 oppure confF2
 */
QString Generatore::getKvCalibAnode(void)
{
    if((confF1=="Mo")||(confF1=="W")) return confF1;
    else return confF2;
}


int  Generatore::convertHvexp(unsigned char data) {
    if(hv_calibrated&&hv_updated){
        return (int) (genCnf.pcb190.HV_CONVERTION * (float) data /1000);
        //return (int) (1.25 * (khv_convert * (float) data) - 0.25 * (float) hvval);
    }else return 0;
}



// Funzione da chiamare quando si voglia aggiungere HU da qualsiasi fonte
// hu Ë espresso in kHu
float Generatore::updateAnodeHU(float hu){

    if(anodeHuInitOk==false) return 0;
    hutimer_elapsed = hutimer.elapsed();

    if(anodeHU!=0){
        if(T0+hutimer_elapsed>1000)
            anodeHU = -52.77 * log((T0+hutimer_elapsed)/1000)+430.9;
        else anodeHU=430.9;
    }

    if(anodeHU<0)  anodeHU = 0;
    anodeHU+=hu;

    // Ricalcola T0 sulla curva teorica
    T0 = 1000* exp((430.9 - anodeHU) / 52.77);

    // Restart del timer
    hutimer.start();

    // Controllo diagnostico
    if(anodeHU>270) alarmAnodeHU = true;
    else if(anodeHU<150) alarmAnodeHU = false;

    // Aggiunta dello status
    int ahu = anodeHU;
    if(ahu>270) ahu|=0x2000;
    else if(ahu>150) ahu|=0x1000;
    ApplicationDatabase.setData(_DB_HU_ANODE,ahu, 0);

    // Aggiorna le statistiche solo se la data Ë statra inizializzata
    anodeHuTime = QDateTime::currentDateTime();
    anodeHUSaved = anodeHU;
    statChanged = true;

    // UpdateHU ogni 10 secondi se necessario
    if(anodeHU>(300/20)){
        if(!timerUpdateAnodeHU) timerUpdateAnodeHU = startTimer(10000);
    }


    return anodeHU;

}

// Funzione che restituisce il valore corrente degli HU
float Generatore::getCurrentHU(void){

    if(anodeHuInitOk==false) return 0;
    if(anodeHU==0) return 0;
    hutimer_elapsed = hutimer.elapsed();

    if(T0+hutimer_elapsed>1000)
        anodeHU = -52.77 * log((T0+hutimer_elapsed)/1000)+430.9;
    else anodeHU=430.9;

    if(anodeHU>270) alarmAnodeHU = true;
    else if(anodeHU<150) alarmAnodeHU = false;

    // Aggiunta dello status
    int ahu = anodeHU;
    if(ahu>270) ahu|=0x2000;
    else if(ahu>150) ahu|=0x1000;
    ApplicationDatabase.setData(_DB_HU_ANODE,ahu, 0);


    return  anodeHU;
}



void Generatore::initAnodeHU(void){

    if(anodeHuInitOk) return;
    anodeHuInitOk = true;


    int Y = QDateTime::currentDateTime().date().year();
    int M = QDateTime::currentDateTime().date().month();
    int D = QDateTime::currentDateTime().date().day();
    int s = QDateTime::currentDateTime().time().second();
    int m = QDateTime::currentDateTime().time().minute();
    int h = QDateTime::currentDateTime().time().hour();

    // Lettura dell'ultima data di registrazione della temperatura
    unsigned int csec = h*3600+m*60 + s;
    unsigned int ssec = anodeHuTime.time().hour()*3600 + anodeHuTime.time().minute() * 60 + anodeHuTime.time().second();
    if(
            (anodeHUSaved==0)||
            (anodeHuTime.date().year()!=Y)  ||
            (anodeHuTime.date().month()!=M)  ||
            (anodeHuTime.date().day()!=D)  ||
            (csec>ssec+3600) ||
            (csec<=ssec)
            )
    {
        anodeHU=0;
        alarmAnodeHU = false;
        anodeHUSaved = anodeHU;
        anodeHuTime = QDateTime::currentDateTime();
        return;
    }

    // calcola T0 sulla curva teorica di quello salvato
    if(anodeHUSaved>430.9) anodeHUSaved=430.9;
    T0 = 1000 * exp((430.9 - anodeHUSaved) / 52.77);

    hutimer_elapsed = (csec-ssec)*1000; // Calcolo il tempo passato in ms

    if(T0+hutimer_elapsed>1000)
        anodeHU = -52.77 * log((T0+hutimer_elapsed)/1000)+430.9;
    else anodeHU=430.9;

    if(anodeHU<0)  anodeHU = 0;
    anodeHUSaved = anodeHU;
    anodeHuTime = QDateTime::currentDateTime();

    // Ricalcola T0 sulla curva teorica
    T0 = 1000* exp((430.9 - anodeHU) / 52.77);

    // Restart del timer
    hutimer.start();


    // Controllo diagnostico
    if(anodeHU>270) alarmAnodeHU = true;
    else if(anodeHU<150) alarmAnodeHU = false;

    // Aggiunta dello status
    int ahu = anodeHU;
    if(ahu>270) ahu|=0x2000;
    else if(ahu>150) ahu|=0x1000;
    ApplicationDatabase.setData(_DB_HU_ANODE,ahu, 0);

    // UpdateHU ogni 10 secondi
    if(anodeHU>300/20){
        if(!timerUpdateAnodeHU) timerUpdateAnodeHU = startTimer(10000);
    }
}



// ___________________________________________________________________________________
// Rerstituisce il valore massimo dei Mas selezionabili per quel dato kV e fuoco
// Restituisce 0 se non ci sono intervalli possibili
// ___________________________________________________________________________________
int Generatore::getMaxDMas(unsigned char kV, QString anodo, unsigned char fuoco)
{
    int i;
    int mas=0;

    if(kV<_MIN_KV) return 0; // kV non ancora selezionati!
    if(kV>_MAX_KV) return 0; // kV non ancora selezionati!

    if(tube[kV-_MIN_KV].mAs.isEmpty()) return 0; // Nessun intervallo disponibile

    for(i=0; i<tube[kV-_MIN_KV].mAs.size(); i++)
    {
        // Seleziona solo gli intervalli associati al fuoco e anodo selezionati
        if(tube[kV-_MIN_KV].mAs.at(i).anode!=anodo) continue;
        if(tube[kV-_MIN_KV].mAs.at(i).fsize!=fuoco) continue;

        if(mas<=tube[kV-_MIN_KV].mAs.at(i).a) mas = tube[kV-_MIN_KV].mAs.at(i).a; // Assegna il valore massimo possibile
    }
    return mas;

}

int Generatore::getMaxDMas(void)
{
    int i;
    int mas=0;
    int kv=(int) selectedKv;

    if(kv<_MIN_KV) return 0; // kV non ancora selezionati!
    if(kv>_MAX_KV) return 0; // kV non ancora selezionati!
    if(tube[kv-_MIN_KV].mAs.isEmpty()) return 0; // Nessun intervallo disponibile

    for(i=0; i<tube[kv-_MIN_KV].mAs.size(); i++)
    {
        // Seleziona solo gli intervalli associati al fuoco e anodo selezionati
        if(tube[kv-_MIN_KV].mAs.at(i).anode!=selectedAnodo) continue;
        if(tube[kv-_MIN_KV].mAs.at(i).fsize!=selectedFSize) continue;

        if(mas<=tube[kv-_MIN_KV].mAs.at(i).a) mas = tube[kv-_MIN_KV].mAs.at(i).a; // Assegna il valore massimo possibile
    }
    return mas;

}

// Acquisizione dei kV correntemente selezionati
// In caso essi siano negativi indica che la modalita corrente
// non consente la selezione degli stessi (modalit‡ automatica).
// Il valore negativo indica i kV che sono stati selezionati nell'ultima
// esposizione effettuata.
float Generatore::getCurrentAnalogKv(void){
    if(pConfig->analogCnf.tech_mode == ANALOG_TECH_MODE_AUTO) return -1 * selectedKv;
    else return selectedKv;
}

// Acquisizione dei mAs in decimi di mAs correntemente selezionati
// In caso essi siano negativi indica che la modalita corrente
// non consente la selezione degli stessi (modalit‡ automatica/semiautomatica).
// Il valore negativo indica in tal caso l'ultimo valore di mAs utilizzato
int Generatore::getCurrentAnalogDmAs(void){
    if(pConfig->analogCnf.tech_mode == ANALOG_TECH_MODE_MANUAL) return  selectedDmAs;
    else return -1 * selectedDmAs;

}


