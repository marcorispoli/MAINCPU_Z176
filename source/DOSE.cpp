#include "application.h"
#include "appinclude.h"
#include "globvar.h"



DOSE::DOSE(QString tubedir)
{
    tubeDir = tubedir;

    kerma_mo_configured = readKermaConfig("Mo");
    kerma_rh_configured = readKermaConfig("Rh");
    return;
}


// Restituisce il puntatore alla struttura associata al filtro selezionato
DOSE::kermaCnf_Str* DOSE::getKermaPtr(int filtro){
    if(filtro==Collimatore::FILTRO_Mo) return &dose.MoKerma;
    else if(filtro==Collimatore::FILTRO_Rh) return &dose.RhKerma;
    else return 0;
}
DOSE::kermaCnf_Str* DOSE::getKermaPtr(QString filtro){
    if(filtro=="Mo") return &dose.MoKerma;
    else if(filtro=="Rh") return &dose.RhKerma;
    else return 0;
}


/*
    Lettura della configurazione dei parametri CG
    da tabella di sistema
*/
// Lettura file di configurazione per calcolatore dose
bool DOSE::readCgsConfig(DOSE::kermaCnf_Str* pKerma, QString filename){
    QList<QString> dati;

    // Lettura file di configurazione generale del tubo
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    while(1)
    {
        dati = Config::getNextArrayFields(&file);
        if(dati.isEmpty()) break;

        // Se il dato non Ã¨ corretto non lo considera
        if(dati.size()==_LEN_CGS_MM + 1){
            int kv_index = ((dati.at(0).toInt() - _MIN_CGS_DKV) / _D_CGS_DKV);
            if((kv_index >= _LEN_CGS_KV)||(kv_index<0)) continue;
            for(int mm_index=0; mm_index<_LEN_CGS_MM; mm_index++){
                pKerma->cgs[kv_index][mm_index] = dati.at(1+mm_index).toFloat();
            }
        }
    }


    file.close();
    return true;
}

/*
    Lettura delle tabelle del Kerma e dei valori S
    rispetto al tubo selezionato
*/
// Lettura file di configurazione per calcolatore dose
bool DOSE::readKermaConfig(QString filtro){
    QList<QString> dati;
    kermaCnf_Str *pKerma;
    QString kerma_filename;
    QString cgs_filename;

    pKerma = getKermaPtr(filtro);
    if(pKerma==0) return false;

    if(filtro=="Mo"){
        kerma_filename = tubeDir + QString("kerma_Mo.cnf");
        cgs_filename = tubeDir + QString("cgs_Mo.cnf");
    }else if(filtro=="Rh"){
        kerma_filename = tubeDir + QString("kerma_Rh.cnf");
        cgs_filename = tubeDir + QString("cgs_Rh.cnf");
    }else return false;

    pKerma->configured = false;

    // Lettura file di configurazione generale del tubo
    QFile file(kerma_filename.toAscii());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    while(1)
    {
        dati = Config::getNextArrayFields(&file);
        if(dati.isEmpty()) break;

        if(dati.at(0)=="D0"){
            pKerma->D0 = dati.at(1).toInt();
        }else if(dati.at(0)=="D1"){
            pKerma->D1 = dati.at(1).toInt();
        }else if(dati.at(0)=="KVH"){
            pKerma->KVH = dati.at(1).toInt();
        }else if(dati.at(0)=="KVM"){
            pKerma->KVM = dati.at(1).toInt();
        }else if(dati.at(0)=="KVL"){
            pKerma->KVL = dati.at(1).toInt();
        }else if(dati.at(0)=="AKH"){
            pKerma->AKH = dati.at(1).toFloat();
        }else if(dati.at(0)=="AKM"){
            pKerma->AKM = dati.at(1).toFloat();
        }else if(dati.at(0)=="AKL"){
            pKerma->AKL = dati.at(1).toFloat();
        }
    }

    pKerma->configured=true;
    file.close();

    // Legge il file CGS associato
    return readCgsConfig(pKerma, cgs_filename);
}

bool DOSE::storeKermaConfig(QString filtro){
    kermaCnf_Str *pKerma;
    QString filename;
    QString frame;

    pKerma = getKermaPtr(filtro);

    if(filtro=="Mo") filename = tubeDir + QString("kerma_Mo.cnf");
    else filename = tubeDir + QString("kerma_Rh.cnf");



    // Lettura file di configurazione generale del tubo
    QFile file(filename.toAscii());
    if (!file.open(QIODevice::WriteOnly| QIODevice::Text)) return false;


    frame = QString("<D0,%1>  \n").arg(pKerma->D0);
    file.write(frame.toAscii().data());

    frame = QString("<D1,%1>  \n").arg(pKerma->D1);
    file.write(frame.toAscii().data());

    frame = QString("<KVH,%1>  \n").arg(pKerma->KVH);
    file.write(frame.toAscii().data());

    frame = QString("<AKH,%1>  \n").arg(pKerma->AKH);
    file.write(frame.toAscii().data());

    frame = QString("<KVM,%1>  \n").arg(pKerma->KVM);
    file.write(frame.toAscii().data());

    frame = QString("<AKM,%1>  \n").arg(pKerma->AKM);
    file.write(frame.toAscii().data());

    frame = QString("<KVL,%1>  \n").arg(pKerma->KVL);
    file.write(frame.toAscii().data());

    frame = QString("<AKL,%1>  \n").arg(pKerma->AKL);
    file.write(frame.toAscii().data());

    file.close();
    file.flush();

    pSysLog->log("CONFIG: KERMA CONFIGURATION FILE");

    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());

    return true;
}

bool DOSE::storeKermaConfig(QString filtro, int D0, int D1, int KVH, float AKH, int KVM, float AKM, int KVL, float AKL){
    kermaCnf_Str *pKerma;

    pKerma = getKermaPtr(filtro);

    pKerma->D0 = D0;
    pKerma->D1 = D1;
    pKerma->KVH = KVH;
    pKerma->KVM = KVM;
    pKerma->KVL = KVL;
    pKerma->AKH = AKH;
    pKerma->AKM = AKM;
    pKerma->AKL = AKL;

    return storeKermaConfig(filtro);

}


// kV espresso in 10 * kV per gestire anche i decimi
float DOSE::getKerma(kermaCnf_Str *pKerma, int dkV){
    float kV = (float) dkV/10;


    float AK21 = pKerma->AKL;
    float AK25 = pKerma->AKM;
    float AK35 = pKerma->AKH;
    float AK23 = (AK21+AK25)*0.96/2;
    float AK30 = (AK25+AK35)*0.96/2;

    float km20_23 = (AK23 - AK21) / 2;
    float km23_25 = (AK25 - AK23) / 2;
    float km25_30 = (AK30 - AK25) / 5;
    float km30_35 = (AK35 - AK30) / 5;

    if(kV<=23){
        return AK21 + (kV-21)*km20_23;
    }else if(kV<=25){
        return AK23 + (kV-23)*km23_25;
    }else if(kV<=30){
        return AK25 + (kV-25)*km25_30;
    }else if(kV<=35){
        return AK30 + (kV-30)*km30_35;
    }else if(kV<=40){
            return AK35 + (kV-35)*km30_35;
    }else return 0;


    //PRINT(QString("AKH:%1 AKM:%2 AKL:%3 \n").arg(pKerma->AKH ).arg(pKerma->AKM ).arg(pKerma->AKL));
    //PRINT(QString("KVH:%1 KVM:%2 KVL:%3 \n").arg(pKerma->KVH ).arg(pKerma->KVM ).arg(pKerma->KVL));
    //PRINT(QString("kml:%1 kmh:%2 \n").arg(kml).arg(khm));
    /*
    float kml = (pKerma->AKM - pKerma->AKL) / (pKerma->KVM - pKerma->KVL);
    float khm = (pKerma->AKH - pKerma->AKM) / (pKerma->KVH - pKerma->KVM);

    if(kV <= pKerma->KVM )  return pKerma->AKM - kml * (float) ((float) pKerma->KVM  - kV );
    else return pKerma->AKM + khm * (float) (kV - pKerma->KVM);
*/

}

float DOSE::getConvertedUgKerma(int dkv, int dmas, int mm, int filtro){
    float airKerma550;
    kermaCnf_Str *pKerma;

    // Preleva il puntatore alla struttura cgs e airKerma relativo al filtro
    pKerma = getKermaPtr(filtro);
    if(!pKerma) return 0;
    if(!pKerma->configured) return 0;

    // Determina il puntatore dei kV
    // Se i kv eccedono i 35, il puntatore imposterà 35
    // Se i kV scendono sotto i 20 allora sarà impostato 20
    int kv_index = (dkv - _MIN_CGS_DKV) / _D_CGS_DKV;
    if(kv_index < 0) kv_index = 0;
    if(kv_index >= _LEN_CGS_KV) kv_index = _LEN_CGS_KV-1;

    // Calcolo dell'Entrance Dose
    airKerma550 = getKerma(pKerma,dkv);
    float dd = ((float) pKerma->D0) / ((float) pKerma->D1  - (float) mm);
    return (airKerma550 *  dd * dd * (float) dmas)/10;

}


// Restituisce la dose relativa ai dati in oggetto espressa in uG
// Il calcolo viene fatto interpolando rispetto ai mm, dato che la tabella
// dei CG è campionata ogni 5mm
// I CGS vengono calcolati sulla base dei mm derivati dalla misura dello spessore del seno.
// La formula di conversione spessore(pmmi) = 0.004 p^2 + 0.9905 p
// PARAMETRI:
// - mm: spessore seno in mm
// - offset_mm: offset piano di compressione sopra il potter (ingranditore)
// - dmAs: decimi di mAs
// -
float DOSE::getDoseUg(int mm, int offset_mm, int dmAs, int dkv, int filtro){
    float airKerma;
    float entrance_kerma;
    kermaCnf_Str *pKerma;

    // Preleva il puntatore alla struttura cgs e airKerma relativo al filtro
    pKerma = getKermaPtr(filtro);
    if(!pKerma) return -1;
    if(!pKerma->configured) return -1;

    // Verifica dei mm di seno che siano nel range di applicabilità
    if(mm<_MIN_CGS_MM) mm = _MIN_CGS_MM;
    if(mm >_MAX_CGS_MM) mm = _MAX_CGS_MM;
    int mm_index_low = (mm - _MIN_CGS_MM) / _D_CGS_MM;
    float delta_dose = ((float) mm - (float) (_MIN_CGS_MM + mm_index_low * _D_CGS_MM)) / ( (float)_D_CGS_MM);

    // Determina il puntatore dei kV
    // Se i kv eccedono i 35, il puntatore imposterà 35
    // Se i kV scendono sotto i 20 allora sarà impostato 20
    int kv_index = (dkv - _MIN_CGS_DKV) / _D_CGS_DKV;
    if(kv_index < 0) kv_index = 0;
    if(kv_index >= _LEN_CGS_KV) kv_index = _LEN_CGS_KV-1;


    // Calcolo dell'Entrance Dose
    airKerma = getKerma(pKerma,dkv);
    float dd = ((float) pKerma->D0) / ((float) pKerma->D1 - (float) offset_mm - (float) mm);
    entrance_kerma =  airKerma * ( dd * dd) ;
    //PRINT(QString("AIR-KERMA:%1 D0:%2 D1:%3 S:%4\n").arg(airKerma).arg(pKerma->D0).arg(pKerma->D1).arg(pKerma->S));

    // Calcolo della dose finale
    float cgs_mean = pKerma->cgs[kv_index][mm_index_low] * (1-delta_dose) +  pKerma->cgs[kv_index][mm_index_low+1] * (delta_dose);
    float dosecalc = entrance_kerma * cgs_mean * (float) dmAs/10;

    //PRINT(QString("kvindex:%1\n\r pmmi: %2 mmil=%3\n\r delta=%4\n\r kerma:%5 entrance=%6\n\r cg=%7\n dose=%8 \n\r").arg(kv_index).arg(pmmi).arg(pmmi_index_low).arg(delta_dose).arg(airKerma).arg(entrance_kerma).arg(cgs_mean).arg(dosecalc));

    return dosecalc;
}

QString DOSE::getDoseUgDebug(int mm, int offset_mm, int dmAs, int dkv, int filtro){
    float airKerma;
    float entrance_kerma;
    kermaCnf_Str *pKerma;
    QString risultato;

    // Preleva il puntatore alla struttura cgs e airKerma relativo al filtro
    pKerma = getKermaPtr(filtro);
    if(!pKerma) return "";
    if(!pKerma->configured) return "";

    // Verifica dei mm di seno che siano nel range di applicabilità
    if(mm<_MIN_CGS_MM) mm = _MIN_CGS_MM;
    if(mm >_MAX_CGS_MM) mm = _MAX_CGS_MM;
    int mm_index_low = (mm - _MIN_CGS_MM) / _D_CGS_MM;
    float delta_dose = ((float) mm - (float) (_MIN_CGS_MM + mm_index_low * _D_CGS_MM)) / ( (float)_D_CGS_MM);

    // Determina il puntatore dei kV
    // Se i kv eccedono i 35, il puntatore imposterà 35
    // Se i kV scendono sotto i 20 allora sarà impostato 20
    int kv_index = (dkv - _MIN_CGS_DKV) / _D_CGS_DKV;
    if(kv_index < 0) kv_index = 0;
    if(kv_index >= _LEN_CGS_KV) kv_index = _LEN_CGS_KV-1;


    // Calcolo dell'Entrance Dose
    airKerma = getKerma(pKerma,dkv);
    float dd = ((float) pKerma->D0) / ((float) pKerma->D1 - (float) offset_mm - (float) mm);
    entrance_kerma =  airKerma * ( dd * dd) ;
    //PRINT(QString("AIR-KERMA:%1 D0:%2 D1:%3 S:%4\n").arg(airKerma).arg(pKerma->D0).arg(pKerma->D1).arg(pKerma->S));

    // Calcolo della dose finale
    float cgs_mean = pKerma->cgs[kv_index][mm_index_low] * (1-delta_dose) +  pKerma->cgs[kv_index][mm_index_low+1] * (delta_dose);
    float dosecalc = entrance_kerma * cgs_mean * (float) dmAs/10;
    risultato = QString("dm=%1 AK[%2mm]=%3 EAK=%4 CGS=%5 DOSE=%6").arg(delta_dose).arg(pKerma->D0).arg(airKerma).arg(entrance_kerma).arg(cgs_mean).arg(dosecalc);

    //PRINT(QString("kvindex:%1\n\r pmmi: %2 mmil=%3\n\r delta=%4\n\r kerma:%5 entrance=%6\n\r cg=%7\n dose=%8 \n\r").arg(kv_index).arg(pmmi).arg(pmmi_index_low).arg(delta_dose).arg(airKerma).arg(entrance_kerma).arg(cgs_mean).arg(dosecalc));

    return risultato;
}

