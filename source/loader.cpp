#define LOADER_C

#include "application.h"
#include "appinclude.h"
#include "globvar.h"


Loader::Loader(int rotation, QWidget *parent) :
    QWidget(parent)

{


    manualMode = false;

}

Loader::~Loader()
{

}


bool Loader::readHexRow(QFile* pFile, QByteArray* row, int* nbyte){
    QByteArray frame,val;
    int i;
    unsigned char checksum;
    bool ok;

    if(row==null) return FALSE;
    row->clear();

    while(!pFile->atEnd()){

        // Legge una riga e scarta quelle che non contengano i ':'
        frame = pFile->readLine();
        frame.replace(" ",""); // Elimina tutti gli spazi
        i=frame.indexOf(":");
        if(i==-1) continue;
        frame = frame.right(frame.size()-i-1);

        // Frame senza ":"
        if(frame.size()<8) return FALSE;
        *nbyte = frame.left(2).toInt(&ok,16);
        frame = frame.right(frame.size()-2);

        if(frame.size()<2* (*nbyte)+8) return FALSE;

        // Legge tutti i byte
        row->clear();
        checksum = (*nbyte);
        for(i=0;i<*nbyte*2+6;i+=2)
        {
            val.clear();
            val.append(frame.at(i));val.append(frame.at(i+1));
            row->append((char) val.toInt(&ok,16));
            if(!ok) return FALSE;
            checksum +=(unsigned char) val.toInt(&ok,16);
        }
        checksum = (-1 * checksum) ;

        // Legge il checksum
        val.clear();
        val.append(frame.at(i));val.append(frame.at(i+1));
        if((unsigned char) val.toInt(&ok,16)!=checksum) return FALSE;
        return TRUE;
    }

    return TRUE;
}

/*
 *  HEADER ASPETTATO NEL FILE .HEX
 *  <CRC,REV>
 *  CRC = HEX FORMAT
 *  return:
 *  - 0 = OK
 *  - 1 = Non c'è il file richiesto
 *  - 2 = Non c'è l'intestazione del file o non è corretta
 *  - 3 = Formato CRC non esadecimanle
 *  - 4 = Formato file hex errato
 *  - 5 = Revisione non corrispondente
 *  - 6 = Crc non corrispondente
 */
int  Loader::verifyHeader(QString filename, QString revCode, unsigned short* calc_crc, QString* file_rev){
    QList<QString> dati;
    unsigned short file_crc,loc_crc;
    bool ok;
    int i,nbyte;

    // Lettura del file hex
    QByteArray row;

    if(calc_crc) *calc_crc = 0;

    QFile file(filename.toAscii());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return 1;

    // Lettura Intestazione
    dati = pConfig->getNextArrayFields(&file);
    if(dati.isEmpty()) return 2;

    file_crc = (unsigned short) dati.at(0).toInt(&ok,16);
    if(!ok) return 3;

    if(file_rev) *file_rev = dati.at(1);

    // Verifica il checksum sulle sole righe valide
    loc_crc = 0;
    while(!file.atEnd())
    {
        // Legge una riga valida
        if(readHexRow(&file, &row, &nbyte) == FALSE) return 4;
        if(row.size()==0) break; // Finita lettura righe
        for(i=0; i<row.size();i++) loc_crc+=row.at(i);
    }
    file.close();

    // Verifica crc
    if(calc_crc!=NULL) *calc_crc = loc_crc;


    if(revCode!=*file_rev) return 5;
    if(loc_crc!=file_crc) return 6;

    return 0;
}


/*___________________________________________________________________________________________________________
                      CARICA IL CONTENUTO DI UN FILE HEX DEL PIC
 Formato HEX FILE
    :02  0000    05   HHLL HHLL ... CH
     NB  START  TYPE  --  bytes --  checksum

    TYPE: solo tipo 5
*/
bool Loader::openPic16FHexFile(QString filename)
{
    QByteArray row;
    unsigned short address;
    _addrStr dataVal;

    int i,nbyte;

    // Apertura file hex
    QFile file(filename.toAscii());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return FALSE;


    pic16Fhex.progSegment.clear();
    pic16Fhex.ID[0]=pic16Fhex.ID[1]=pic16Fhex.ID[2]=pic16Fhex.ID[3]=0;
    pic16Fhex.configWord=0;

    // Scorre tutto il file interpretando le sole righe di codice
    while(!file.atEnd())
    {
        // Legge una riga valida
        if(readHexRow(&file, &row, &nbyte) == FALSE) return FALSE;
        if(row.size()==0) break; // Finita lettura righe

        // Considera solo un Header DATI
        if(row.at(2)!=HEX_RECORD_DATA) continue;

        // Calcola l'indirizzo
        address = (row.at(0)*256 + row.at(1))/2;
        if((address>=0x2000)&&(address<0x2100))
        {
            // Salva la configuration Word
            if((address<=0x2007)&&(address+nbyte>=0x2007))
            {
               i = (0x2007-address);
               pic16Fhex.configWord=row.at(3+i*2) + row.at((i*2)+4)*256;
            }

            // Salva le ID word
            if((address<=0x2000)&&(address+nbyte>=0x2003))
            {
               i = (0x2000-address);
               pic16Fhex.ID[0]=row.at(3+i*2) + row.at((i*2)+4)*256;


               i = (0x2001-address);
               pic16Fhex.ID[1]=row.at(3+i*2) + row.at((i*2)+4)*256;


               i = (0x2002-address);
               pic16Fhex.ID[2]=row.at(3+i*2) + row.at((i*2)+4)*256;


               i = (0x2003-address);
               pic16Fhex.ID[3]=row.at(3+i*2) + row.at((i*2)+4)*256;

            }

        }else
        {
            dataVal.startAddr = address ;
            dataVal.len = nbyte/2;
            for(i=0;i<dataVal.len;i++)
                dataVal.val[i] = row.at(3+i*2) + row.at((i*2)+4)*256;
            pic16Fhex.progSegment.append(dataVal);
        }
    }
    file.close();

    if(pic16Fhex.configWord==0) return FALSE;
    return TRUE;
}

/*
 *  Funzion per impostare correttamente il comando verso MCC
 *  Il Loader ha una sotto-famiglia di comandi identificati
 *  dal comando MCC_LOADER e dal sotto comando scritto sul
 *  byte 0 del Buffer.A seguire nel buffer verranno copiati
 *  i Dati associati al sotto comando richiesto.
 *
 */
bool Loader::mccLoader(_MccLoaderNotify_Code cmd, QByteArray data)
{
    pendingAction = cmd;
    pendingData = data;
    pendingId++; if(!pendingId) pendingId=1;
    data.prepend((unsigned char) cmd);

    bool ris = pConsole->pGuiMcc->sendFrame(MCC_LOADER,pendingId,(unsigned char*) data.data(), data.size());
    return ris;
}

bool Loader::mccLoader(_MccLoaderNotify_Code cmd)
{
    pendingAction = cmd;
    pendingData.clear();
    pendingId++; if(!pendingId) pendingId=1;
    QByteArray data;
    data.append((unsigned char) cmd);
    return pConsole->pGuiMcc->sendFrame(MCC_LOADER,pendingId,(unsigned char*) data.data(), data.size());
}

// Utilizza gli stessi dati dell'ultima emissione mcc
bool Loader::mccLoader(void)
{
    QByteArray data = pendingData;
    data.prepend((unsigned char) pendingAction);

    bool ris = pConsole->pGuiMcc->sendFrame(MCC_LOADER,pendingId,(unsigned char*) data.data(), data.size());
    return ris;
}

/*
typedef struct
{
    unsigned short addr;
    unsigned short  val;
}_addrStr;

QList<_addrStr> dataContent;
*/

//------------------------- Invio a periferica del codice oggetto--------------------------------------------------------------------
bool Loader::startDownloadHexFile(_itemDownload item)
{
    QByteArray data;
    curItem = item;
    QString stringa1, stringa;

    // Verifica se esite il package da istallare
    if(manualMode) stringa1 = QString("MANUAL FIRMWARE PRROGRAMMING");
    else stringa1 = QString("INSTALLING PACKAGE:%1").arg(pConfig->swConf.rvPackage);
    stringa =  QString("Download device:%1 ...\n").arg(item.devTag);

    pWarningBox->activate(stringa1,stringa,100,msgBox::_PROGRESS_BAR);
    pWarningBox->setTextAlignment(Qt::AlignLeft);
    pWarningBox->setProgressBar(0,100,download_cur_perc);

    // Carica il file hex in memoria
    if(openPic16FHexFile(item.file) == false)
    {
        if(manualMode){
            manualMode=false;
            return false;
        }
        pConfig->updError = true;
        pConfig->updErrStr.append(QString("Loader: ERR %1, File:%2\n").arg(LOAD_HEX).arg(item.file));
        return false;
    }

    data.append((unsigned char) 1); // Attivazione
    data.append((unsigned char) item.loaderAddr); // indirizzo loader remoto
    data.append((unsigned char) item.uC); // indirizzo uC remoto

    if(mccLoader(LOADER_ACTIVATION, data)==FALSE)
    {
        if(manualMode){
            manualMode=false;
            return false;
        }
        pConfig->updError = true;
        pConfig->updErrStr.append(QString("Loader: ERR %1, File:%2\n").arg(MCC_COMMAND).arg(item.file));
        return false;
    }

    // Inizio sequenza di scaricamento verso M4: ATTIVAZIONE LOADER SU PERIFERICA
    connect(pConsole,SIGNAL(mccLoaderNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(loaderNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);

    return true;
}


// Notifica esito comando su loader
void Loader::loaderNotify(unsigned char id,unsigned char cmd,QByteArray data)
{

    int i;
    unsigned char buffer[sizeof(_addrStr)+1];


    pendingId++; if(!pendingId) pendingId=1;
    switch(cmd)
    {
        case LOADER_ACTIVATION: // Ricevuta notifica dal comando di attivazione

            if(data.at(0)==1)
            {
                // Attivazione OK: si procede con la cancellazione della FLASH
                // Il comando utilizza ID per identificare l'azione in corso
                if(mccLoader(LOADER_CHIP_ERASE)==FALSE)
                {
                    // COMANDO FALLITO. AGGIUNGERE INVIO NOTIFICA ERRORE
                    onDownloadErr(QString("Loader: ERR %1, File:%2\n").arg(MCC_COMMAND).arg(curItem.file));
                    return;
                }
                qDebug() << "LOADER: Chip Erasing ......";

            }else                
            {
                // Attivazione loader fallita
                onDownloadErr(QString("Loader: ERR %1, File:%2\n").arg(ERR_ACTIVATION).arg(curItem.file));
                return;
            }
        break;
        case LOADER_CHIP_ERASE: // Ricevuta notifica dal comando di cancellazione avvenuta
            if(data.at(0)==1)
            {                
                // Preparazione Blocco 0
                pendingBlock = 0;
                buffer[0] = LOADER_WRITE_BLK;

                _addrStr addrVal = pic16Fhex.progSegment.at(0);
                buffer[1] = (unsigned char) (addrVal.startAddr & 0x00ff);
                buffer[2] = (unsigned char) ((addrVal.startAddr>>8) & 0x00ff);
                buffer[3] = (unsigned char) addrVal.len;
                for(i=0; i< buffer[3]; i++){
                    buffer[4+i*2] = (unsigned char) (addrVal.val[i] & 0x00ff);
                    buffer[5+i*2] = (unsigned char) ((addrVal.val[i]>>8) & 0x00ff);
                }

                pendingId++; if(!pendingId) pendingId=1;
                if(pConsole->pGuiMcc->sendFrame(MCC_LOADER,pendingId,buffer, 4+2*buffer[3])==FALSE)
                {
                    onDownloadErr(QString("Loader: ERR %1, File:%2\n").arg(MCC_COMMAND).arg(curItem.file));
                    return;
                }

            }else
            {  // FALLITA CANCELLAZIONE
                onDownloadErr(QString("Loader: ERR %1, File:%2\n").arg(ERR_ERASE).arg(curItem.file));
                return;
            }
        break;

        case LOADER_WRITE_BLK: // Ricevuta notifica dal comando di scrittura
            if(data.at(0))
            {
                pendingBlock++;

                // Calcola la percentuale da inviare alla console con comando asincrono
                int perc = download_cur_perc +  ((download_delta_perc * pendingBlock) / pic16Fhex.progSegment.size());
                if(perc>=100) perc = 99;
                if(prev_cur_perc != perc){
                    prev_cur_perc = perc;
                    if(!manualMode) pToConsole->systemUpdateStatus(perc, curItem.file);
                    pWarningBox->setProgressBar(0,100,perc);
                }

                if(pendingBlock>=pic16Fhex.progSegment.size())
                {
                    // Scrittura ID e configuration WORD
                    QByteArray param;
                    param.append((char) pic16Fhex.ID[0]);
                    param.append((char) (pic16Fhex.ID[0]>>8));
                    param.append((char) pic16Fhex.ID[1]);
                    param.append((char) (pic16Fhex.ID[1]>>8));
                    param.append((char) pic16Fhex.ID[2]);
                    param.append((char) (pic16Fhex.ID[2]>>8));
                    param.append((char) pic16Fhex.ID[3]);
                    param.append((char) (pic16Fhex.ID[3]>>8));
                    param.append((char) pic16Fhex.configWord);
                    param.append((char) (pic16Fhex.configWord>>8));
                    if(mccLoader(LOADER_WRITE_CONFIG,param)==FALSE)
                    {
                        onDownloadErr(QString("Loader: ERR %1, File:%2\n").arg(MCC_COMMAND).arg(curItem.file));
                        return;
                    }
                    return;
                 }

                // Preparazione Blocco n-esimo
                buffer[0] = LOADER_WRITE_BLK;
                _addrStr addrVal = pic16Fhex.progSegment.at(pendingBlock);
                buffer[1] = (unsigned char) (addrVal.startAddr & 0x00ff);
                buffer[2] = (unsigned char) ((addrVal.startAddr>>8) & 0x00ff);
                buffer[3] = (unsigned char) addrVal.len;
                for(i=0; i< buffer[3]; i++){
                    buffer[4+i*2] = (unsigned char) (addrVal.val[i] & 0x00ff);
                    buffer[5+i*2] = (unsigned char) ((addrVal.val[i]>>8) & 0x00ff);
                }


                pendingId++; if(!pendingId) pendingId=1;
                if(pConsole->pGuiMcc->sendFrame(MCC_LOADER,pendingId,buffer, 4+2*buffer[3])==FALSE)
                {
                    onDownloadErr(QString("Loader: ERR %1, File:%2\n").arg(MCC_COMMAND).arg(curItem.file));
                    return;
                }

            }else
            {
                onDownloadErr(QString("Loader: ERR %1, File:%2\n").arg(ERR_BLOCK_WRITE).arg(curItem.file));
                return;
            }
        break;

        case LOADER_WRITE_CONFIG:
            if(data.at(0))
            {
                // TRASFERIMENTO COMPLETATO, PROSEGUE EVENTUALMENTE CON UNA NUOVA SEQUENZA
                if(mccLoader(LOADER_WRITE_COMPLETED)==FALSE)
                {
                    onDownloadErr(QString("Loader: ERR %1, File:%2\n").arg(MCC_COMMAND).arg(curItem.file));
                    return;
                }
                if(manualMode){
                    manualMode=false;
                    emit loaderCompletedSgn(true,QString(""));
                    return;
                }



                // Prosegue con la sequenza
                if(pendingDownload.size()!=0) {                    
                    download_cur_perc += download_delta_perc;
                    if(startDownloadHexFile(pendingDownload.at(0))==false){
                        pendingDownload.removeAt(0);
                        onDownloadErr("");
                    }else   pendingDownload.removeAt(0);
                    return ; // Iniziata sequenza di download
                }else
                {
                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // NOTIFICA FINE SEQUENZA RAGGIUNTA
                    emit loaderCompletedSgn(true,QString(""));
                    return;
                }

                return;

            }else
            {
                onDownloadErr(QString("Loader: ERR %1, File:%2\n").arg(ERR_CONFIG).arg(curItem.file));
                return;
            }

        break;

    default:
        break;
    }
}


//___________________________________________________________________________________________________________________________________

// Legge la configurazione ID+confWord da un Target preciso
// La funzione emette la readConfigSgn(_picConfigStr) con il contenuto richiesto
bool Loader::readConfig(unsigned char target, unsigned char uC)
{
    QByteArray data;
    data.append((char) target);
    data.append((char) uC);
    if(mccLoader(LOADER_READ_CONFIG,data)==TRUE)
    {
        connect(pConsole,SIGNAL(mccLoaderNotify(unsigned char,unsigned char,QByteArray)),this,SLOT(readConfigNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
        return TRUE;
    }

    return FALSE;
}


// Data.at(0:7) ->ID0..3
// Data(8,9) ->confWord
void Loader::readConfigNotify(unsigned char id,unsigned char cmd,QByteArray data)
{
    _picConfigStr conf;


    if(id!=pendingId) return;
    if(cmd!=LOADER_READ_CONFIG) return;


    if(data.at(0))
    {
        conf.ID[0] = data.at(1) + data.at(2)*256;
        conf.ID[1] = data.at(3) + data.at(4)*256;
        conf.ID[2] = data.at(5) + data.at(6)*256;
        conf.ID[3] = data.at(7) + data.at(8)*256;
        conf.configWord = data.at(9) + data.at(10)*256;
        emit readConfigSgn(conf);
    }else
    {
        qDebug() << "FALLITA LETTURA CONFIGURAZIONE";
    }
}



/*
 * Funzione di attivazione download firmwares verso le periferiche.
 *
 * @Param file_indice  File indice contenente i dati del package da aggiornare.
 *
 *  La sequenza di aggiornamento ìterminer�  con l'invio di una signal
 *  loaderCompletedSgn(bool result, QString errstr) a cui il chiamante deve
 *  connettersi per ricevere l'esito dell'operazione.
 *
 *  I files hex, formattati opportunamente, devono essere presenti nella HOME
 *  predefinita.
 *
 *  Attenzione, per poter aggiornare il potter è necessario che lo stesso sia
 *  collegato.
 */
void Loader::firmwareUpdate(void){

    QFile file;
    QString file_rev;
    unsigned short calc_crc;
    _itemDownload item;
    int codice;

    // Cancella la lista delle periferiche da caricare
    pendingDownload.clear();
    manualMode=false;


    // Discrimina il modello montato. Per compatibilit� con il passato9
    // i collimatori in ASSY 01 (MODELLO A) rimarranno fermi alla revisione 2.4 (U1) 1.4 (U2)
    // mentre i nuovi collimatori (ASSY 02) andranno avfanti a partire dalle revisioni 3.1 (U1) 2.1 (U2)
    // Il tipo di colllimatore viene impostato tramite parametro (pConfig->sys.collimator_type)
    QString rv249U1selected, rv249U2selected;
    QString fileU1, fileU2;
    if(pCollimatore->colli_model == _COLLI_TYPE_ASSY_01){
        rv249U1selected = "2.5";
        rv249U2selected = "1.4";
        fileU1 = FILE_249U1A;
        fileU2 = FILE_249U2A;

    }else{
        rv249U1selected = pConfig->swConf.rv249U1;
        rv249U2selected = pConfig->swConf.rv249U2;
        fileU1 = FILE_249U1;
        fileU2 = FILE_249U2;
    }

    if(pConfig->rv249U1 != rv249U1selected)
    {
        // Verifica la presenza del file
        file.setFileName(fileU1);
        if(file.exists()==FALSE){
            pConfig->updError = true;
             pConfig->updErrStr.append(QString("Loader: ERR %1, File:%2\n").arg(FIRMWARE_INESISTENTE).arg(fileU1));
        }else{
            // Test Checksum e revisione firmware dall'header del file
            codice = verifyHeader(fileU1, rv249U1selected, &calc_crc, &file_rev);
            if(codice)
            {
                 pConfig->updError = true;
                 pConfig->updErrStr.append(QString("Loader: ERR %1 SUB-ERR %2, File:%3\n").arg(FORMATO_HEX).arg(codice).arg(fileU1));
            }else{
                // Aggiunge alla lista
                item.loaderAddr = 0x1E;
                item.file = fileU1;
                item.uC = 1;
                item.devTag = "PCB249U1";
                pendingDownload.append(item);
            }
        }
    }

    if(pConfig->rv249U2 != rv249U2selected)
    {
        // Verifica la presenza del file
        file.setFileName(fileU2);
        if(file.exists()==FALSE){
             pConfig->updError = true;
             pConfig->updErrStr.append(QString("Loader: ERR %1, File:%2\n").arg(FIRMWARE_INESISTENTE).arg(fileU2));
        }else{

            // Test Checksum e revisione firmware dall'header del file
            codice = verifyHeader(fileU2, rv249U2selected, &calc_crc, &file_rev);
            if(codice)
            {
                 pConfig->updError = true;
                 pConfig->updErrStr.append(QString("Loader: ERR %1 SUB-ERR %2, File:%3\n").arg(FORMATO_HEX).arg(codice).arg(fileU2));
            }else{
                // Aggiunge alla lista
                item.loaderAddr = 0x1E;
                item.file = fileU2;
                item.uC = 2;
                item.devTag = "PCB249U2";
                pendingDownload.append(item);
            }
        }
    }

    if(pConfig->rv240 != pConfig->swConf.rv240)
    {
        // Verifica la presenza del file
        file.setFileName(FILE_240);
        if(file.exists()==FALSE){
             pConfig->updError = true;
             pConfig->updErrStr.append(QString("Loader: ERR %1, File:%2\n").arg(FIRMWARE_INESISTENTE).arg(FILE_240));
        }else{
            // Test Checksum e revisione firmware dall'header del file
            codice = verifyHeader(FILE_240, pConfig->swConf.rv240, &calc_crc, &file_rev);
            if(codice)
            {
                 pConfig->updError = true;
                 pConfig->updErrStr.append(QString("Loader: ERR %1 SUB-ERR %2, File:%3\n").arg(FORMATO_HEX).arg(codice).arg(FILE_240));
            }else{
                // Aggiunge alla lista
                item.loaderAddr = 0x0;
                item.file = FILE_240;
                item.uC = 1;
                item.devTag = "PCB240";
                pendingDownload.append(item);
            }
        }
    }

    if(pConfig->rv190 != pConfig->swConf.rv190)
     {
        // Verifica la presenza del file
        file.setFileName(FILE_190);
        if(file.exists()==FALSE){
             pConfig->updError = true;
             pConfig->updErrStr.append(QString("Loader: ERR %1, File:%2\n").arg(FIRMWARE_INESISTENTE).arg(FILE_190));
        }else{
            // Test Checksum e revisione firmware dall'header del file
            codice = verifyHeader(FILE_190, pConfig->swConf.rv190, &calc_crc, &file_rev);
            if(codice)
            {
                 pConfig->updError = true;
                 pConfig->updErrStr.append(QString("Loader: ERR %1 SUB-ERR %2, File:%3\n").arg(FORMATO_HEX).arg(codice).arg(FILE_190));
            }else{

                // Aggiunge alla lista
                item.loaderAddr = 0x1C;
                item.file = FILE_190;
                item.uC = 1;
                item.devTag = "PCB190";
                pendingDownload.append(item);
            }
        }
    }

    if(pConfig->rv244A != pConfig->swConf.rv244A)
    {

        // Verifica la presenza del file
        file.setFileName(FILE_244A);
        if(file.exists()==FALSE){
             pConfig->updError = true;
             pConfig->updErrStr.append(QString("Loader: ERR %1, File:%2\n").arg(FIRMWARE_INESISTENTE).arg(FILE_244A));
        }else{

            // Test Checksum e revisione firmware dall'header del file
            codice = verifyHeader(FILE_244A, pConfig->swConf.rv244A, &calc_crc, &file_rev);
            if(codice){
                 pConfig->updError = true;
                 pConfig->updErrStr.append(QString("Loader: ERR %1 SUB-ERR %2, File:%3\n").arg(FORMATO_HEX).arg(codice).arg(FILE_244A));
            }else{
                // Aggiunge alla lista
                item.loaderAddr = 0x1A;
                item.file = FILE_244A;
                item.uC = 1;
                item.devTag = "PCB244A";
                pendingDownload.append(item);
            }
        }
    }




    if(pConfig->rv269 != pConfig->swConf.rv269)
    {
        // Verifica la presenza del file
        file.setFileName(FILE_269);
        if(file.exists()==FALSE){
             pConfig->updError = true;
             pConfig->updErrStr.append(QString("Loader: ERR %1, File:%2\n").arg(FIRMWARE_INESISTENTE).arg(FILE_269));
        }else{

            // Test Checksum e revisione firmware dall'header del file
            codice = verifyHeader(FILE_269, pConfig->swConf.rv269, &calc_crc, &file_rev);
            if(codice){
                 pConfig->updError = true;
                 pConfig->updErrStr.append(QString("Loader: ERR %1 SUB-ERR %2, File:%3\n").arg(FORMATO_HEX).arg(codice).arg(FILE_269));
            }else{
                // Aggiunge alla lista
                item.loaderAddr = 0x1B;
                item.file = FILE_269;
                item.uC = 1;
                item.devTag = "PCB269";
                pendingDownload.append(item);
            }
        }
    }

    // Verifica se deve lanciare la procedura
    if(pendingDownload.size()!=0) {
        download_cur_perc = 0;
        download_delta_perc = 100 / pendingDownload.size();

        if(startDownloadHexFile(pendingDownload.at(0))==false){
            pendingDownload.removeAt(0);
            onDownloadErr("");
        }else{
            pendingDownload.removeAt(0);
        }
        return  ; // Iniziata sequenza di download
    }


    // Non ci sono siles da aggiornare. Termina la procedura con successo
    emit loaderCompletedSgn(true,QString(""));
    return ;
}

void Loader::manualFirmwareUpload(unsigned char target, unsigned char uC, QString file, QString tag){
    _itemDownload item;

    manualMode = true;

    item.loaderAddr = target;
    item.file = file;
    item.uC =  uC;
    item.devTag = tag;

    download_cur_perc = 0;
    download_delta_perc = 100;
    connect(this,SIGNAL(loaderCompletedSgn(bool,QString)),this,SLOT(manualFirmwareUploadNotify(bool,QString)));
    startDownloadHexFile(item);
}


void Loader::manualFirmwareUploadNotify(bool esito, QString errstr){
    QString stringa;

    manualMode = false;
    disconnect(this,SIGNAL(loaderCompletedSgn(bool,QString)),this,SLOT(manualFirmwareUploadNotify(bool,QString)));
    QString stringa1 = QString("MANUAL FIRMWARE PROGRAMMING");
    if(esito){
        stringa =  QString("INSTALLATION COMPLETED SUCCESSFULLY\nREBOOT THE MACHINE TO COMPLETE.");
        pWarningBox->setTimeout(5000);
    }
    else {
        stringa = QString("INSTALLATION ERROR:\n");
        stringa.append(errstr);
    }

    pWarningBox->activate(stringa1,stringa,100,msgBox::_BUTTON_CANC);
    pWarningBox->setTextAlignment(Qt::AlignLeft);


}

/*
 *  return TRUE: deve proseguire
 *  return FALSE: ha finito
 */
bool Loader::onDownloadErr(QString error){

    pConfig->updError = true;
    pConfig->updErrStr.append(error);

    if(manualMode){
        manualMode=false;
        emit loaderCompletedSgn(true,QString(""));
        return FALSE;
    }

    // Verifica se deve proseguire o meno
    if(pendingDownload.size()==0) {
        emit loaderCompletedSgn(true,QString(""));
        return FALSE;
    }

    // Ricalcola le percentuali
    download_cur_perc += download_delta_perc;
    while(!startDownloadHexFile(pendingDownload.at(0))){
        pendingDownload.removeAt(0);

        if(pendingDownload.size()==0) {
            emit loaderCompletedSgn(true,QString(""));
            return false;
        }

        // Riprova con uno nuovo
        download_cur_perc += download_delta_perc;
    }

    pendingDownload.removeAt(0);
    return true;

}
