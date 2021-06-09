#include "application.h"
#include "appinclude.h"
#include "globvar.h"

/*______________________________________________________________________
 *
 *  LIMITI DI POSIZIONAMENTO: MAXZ = OFFZ + OFSPAD - margPos - pos + 8
 *  ATTENZIONE: Il Pad BIOP_3D ha la posizione del Top p8 mm più
 *  bassa di quella del Pad 2D. Di conseguenza la posizione calibrata
 *  del compressore è falsata (più alta) di 8mm.
 *  Per compensare questo fattore si è corretto l'ofPad da 42 a 50mm
 *  e nel file di configurazione del compressore, l'offset di compressione
 *  è stato aumentato di 8 mm per aggiustare la misura del seno compresso
 *           _
 *          | |
 *          | |____
 *          |_|____|
 * POS...
 *  |8mm __              |  ---
 *      |  | ofspad=42   |  --- margPos = 5
 *      |  |__________   |
 *      |_____________|  |
 *                       |
 *                       !offz=189
 *_______________________|___________
 *///////////////////////////////////
 /*_______________________________________________________________________*/

void biopsy::activateConnections(void){
    connect(pConsole, SIGNAL(mccBiopsyNotify(unsigned char,unsigned char,QByteArray)), this, SLOT(mccStatNotify(unsigned char,unsigned char,QByteArray)),Qt::UniqueConnection);
}


biopsy::biopsy(QObject *parent) :
    QObject(parent)
{
    connected = FALSE;
    checksum_h=0;
    checksum_l=0;
    revisione=0;

    // Nel caso in cui ci fosse un errore nel file o il file non
    // esiste vengono caricati dei valori di default
    if(openCfg() == FALSE)  pSysLog->log("BIOPSY: UNABLE TO CREATE CONFIG FILE, USE DEFAULTS");

}

void biopsy::defaultConfigData(void){

    // Offset di puntamento
    config.offsetX = 0;         // (dmm) offset di calibrazione X
    config.offsetY = 0;         // (dmm) offset di calibrazione Y
    config.offsetZ = 0;         // (dmm) offset di calibrazione Z
    config.offsetFibra = 170;   // (mm) distanza zero torretta - fibra di carbonio

    // Gestion Movimento Pad
    config.offsetPad = 50;              // Offset linea di calibrazione posizione - superficie staffe metalliche
    config.margineRisalita = 15;        // Margine di sicurezza per impatto con il compressore in risalita
    config.marginePosizionamento = 5;   // Margine di sicurezza impatto con il compressore in puntamento

    // Calibrazione reader
    config.dmm_DXReader = 1200;                // Dimensione in dmm Fantoccio di calibrazione Reader
    config.dmm_DYReader = 305;
    config.readerKX = 1.883;                   // Fattore di conversione
    config.readerKY = 0.973;                   // Fattore di conversione
}


bool biopsy::openCfg(void)
{
    QString filename;
    QList<QString> dati;

    // Default File di configurazione
    defaultConfigData();

    filename =  QString("/resource/config/biopsy.cnf");

    // Se nn esiste lo crea con i default
    QFile file(filename.toAscii());
    if(!file.exists()){
        return storeConfig();
    }

    // Se è corrotto lo crea di default
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return storeConfig();
    }

    // Procede con la lettura del formato corrente
    while(1){

        dati = Config::getNextArrayFields(&file);
        if(dati.isEmpty()) break;

        // Assegnazione tipologia filtri
        if(dati.at(0)=="OFFSET_X"){
            config.offsetX = dati.at(1).toInt();

        }else if(dati.at(0)=="OFFSET_Y"){
            config.offsetY = dati.at(1).toInt();

        }else if(dati.at(0)=="OFFSET_Z"){
            config.offsetZ = dati.at(1).toInt();

        }else if(dati.at(0)=="OFFSET_FIBRA"){
            config.offsetFibra = dati.at(1).toInt();

        }else if(dati.at(0)=="OFFSET_PAD"){
            config.offsetPad = dati.at(1).toInt();

        }else  if(dati.at(0)=="MARGINE_RISALITA"){
            config.margineRisalita = dati.at(1).toInt();

        }else  if(dati.at(0)=="MARGINE_POSIZIONAMENTO"){
            config.marginePosizionamento = dati.at(1).toInt();
        }else  if(dati.at(0)=="READER_DX"){
            config.dmm_DXReader = dati.at(1).toInt();
        }else  if(dati.at(0)=="READER_DY"){
            config.dmm_DYReader = dati.at(1).toInt();
        }else  if(dati.at(0)=="READER_KX"){
            config.readerKX = dati.at(1).toFloat();
        }else  if(dati.at(0)=="READER_KY"){
            config.readerKY = dati.at(1).toFloat();
        }

    }

    file.close();
    return true;
}


/*
 *
 *  Salva il file di configurazione della biopsia
 */
bool biopsy::storeConfig(void)
{
    QString filename;

    filename =  QString("/resource/config/biopsy.cnf");
    QFile file(filename.toAscii());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return FALSE;

    file.write( QString("<OFFSET_X, %1>\n").arg((int) config.offsetX).toAscii());
    file.write( QString("<OFFSET_Y, %1>\n").arg((int) config.offsetY).toAscii());
    file.write( QString("<OFFSET_Z, %1>\n").arg((int) config.offsetZ).toAscii());
    file.write( QString("<OFFSET_FIBRA, %1>\n").arg((int) config.offsetFibra).toAscii());

    file.write( QString("<OFFSET_PAD,%1>\n").arg((int) config.offsetPad).toAscii());
    file.write( QString("<MARGINE_RISALITA,%1>\n").arg((int) config.margineRisalita).toAscii());
    file.write( QString("<MARGINE_POSIZIONAMENTO,%1>\n").arg((int) config.marginePosizionamento).toAscii());

    file.write( QString("<READER_DX,%1>\n").arg((unsigned short) config.dmm_DXReader).toAscii());
    file.write( QString("<READER_DY,%1>\n").arg((unsigned short) config.dmm_DYReader).toAscii());
    file.write( QString("<READER_KX,%1>\n").arg((float) config.readerKX).toAscii());
    file.write( QString("<READER_KY,%1>\n").arg((float) config.readerKY).toAscii());

    file.flush();
    file.close();

    pSysLog->log("BIOPSY: STORED CALIBRATION FILE");
    return true;
}



/*


 */
void biopsy::mccStatNotify(unsigned char id_notify,unsigned char cmd, QByteArray data)
{
    unsigned char errore;
    static unsigned char bp_motion=0;
    static int bp_movecommand=0;

    if(id_notify!=1) return;
    //if(pConfig->startupCompleted == false) return;

    errore=0;
    if(cmd!=BIOP_NOTIFY_STAT) return;


    // Se il sistema risulta NON connesso non fa altro..
    if(data.at(_BP_CONNESSIONE)==_BP_CONNESSIONE_DISCONNECTED)
    {
        connected = FALSE;
        if(movingCommand > _BIOPSY_MOVING_COMPLETED){
            movingCommand =_BIOPSY_MOVING_COMPLETED;
            movingError = _BIOPSY_MOVING_ERROR_TIMEOUT;
        }
        return;
    }

    // Cambio stato da Non connesso a connesso
    if(!connected){
        // Riconoscimento torretta di Biopsia inserita
        connected = TRUE;
        checksum_h=data[_BP_CHKH];
        checksum_l=data[_BP_CHKL];
        revisione=data[_BP_REVIS];

        movingCommand =_BIOPSY_MOVING_NO_COMMAND;
        movingError = _BIOPSY_MOVING_NO_ERROR;

        // Aggiorna le pagine con nil riconoscimento della Biopsia
        ApplicationDatabase.setData(_DB_ACCESSORIO, (unsigned char) BIOPSY_DEVICE,0);
        ApplicationDatabase.setData(_DB_ACCESSORY_NAME,QString(QApplication::translate("BIOPSY","NOME ACCESSORIO", 0, QApplication::UnicodeUTF8)),0);
        ApplicationDatabase.setData(_DB_BIOP_Z_FIBRA, (int) pBiopsy->config.offsetFibra,0);
        // Inizializzazioni sul cambio di stato se necessario
    }

    // Acquisizione del campionamento Joystic corrente
    dmmJX = data[_BP_JXL] + 256 * data[_BP_JXH];
    dmmJY = data[_BP_JYL] + 256 * data[_BP_JYH];

    // Pulsante di sblocco Braccio e funzioni biopsia
    ApplicationDatabase.setData(_DB_BIOP_UNLOCK_BUTTON,(int) data.at(_BP_PUSH_SBLOCCO),0);
    ApplicationDatabase.setData(_DB_BIOP_CONSOLE_BUTTON,(int) data.at(_BP_CONSOLE_PUSH),0);

    //  Riconoscimento dell'accessorio
    ApplicationDatabase.setData(_DB_BIOP_HOLDER,(int) data.at(_BP_ADAPTER_ID),0);

    // Posizione attuale cursore
    curX_dmm = data.at(_BP_XL) + 256 * data.at(_BP_XH) ;
    curY_dmm = data.at(_BP_YL) + 256 * data.at(_BP_YH) ;
    curZ_dmm = data.at(_BP_ZL) + 256 * data.at(_BP_ZH) ;
    ApplicationDatabase.setData(_DB_BIOP_X,(int) curX_dmm,0);
    ApplicationDatabase.setData(_DB_BIOP_Y,(int) curY_dmm,0);
    ApplicationDatabase.setData(_DB_BIOP_Z,(int) curZ_dmm,0);

    maxZ_mm  = data.at(_BP_MAX_Z); // Questo dato tiene conto solo del compressore e non dell'ago
    zlim_mm  = data.at(_BP_ZLIMIT); // Questo è il massimo spostamento possibile considerando l'Ago

    // Questa è la distanza della punta dell'ago dalla fibra di carbonio
    margZ_mm = pBiopsy->config.offsetFibra - (curZ_dmm/10) - ApplicationDatabase.getDataI(_DB_BIOP_AGO);


    ApplicationDatabase.setData(_DB_BIOP_ZLIMIT,(int) maxZ_mm,0);
    ApplicationDatabase.setData(_DB_BIOP_MARG,(int) margZ_mm,0);

    if(bp_motion!=data.at(_BP_MOTION)){
        PRINT(QString("BIOPSIA: BP_MOTION=%1").arg((int) data.at(_BP_MOTION)));
        bp_motion = data.at(_BP_MOTION);
    }

    if(bp_movecommand!=movingCommand){
        PRINT(QString("BIOPSIA: MOVE CMD=%1").arg((int) movingCommand));
        bp_movecommand = movingCommand;
    }


    // ______________________________ Aggiornamento stato di posizionamento in corso _________________________________________________
    switch(data.at(_BP_MOTION))
    {

        case _BP_MOTION_ON:

        break;

        case _BP_MOTION_TERMINATED:

            // Valutazione del risultato del movimento
            if(data.at(_BP_MOTION_END) == _BP_ERROR_POSITIONINIG)
            {                
                movingError = _BIOPSY_MOVING_ERROR_TARGET;
            }else if(data.at(_BP_MOTION_END) == _BP_TIMEOUT_COMANDO)
            {
                movingError = _BIOPSY_MOVING_ERROR_TIMEOUT;
            }else  movingError = _BIOPSY_MOVING_NO_ERROR;

            movingCommand =_BIOPSY_MOVING_COMPLETED;

        break;


    } // data.at(_DB_MOTION)

}

bool biopsy::moveXYZ(unsigned short X, unsigned short Y, unsigned short Z)
{
    unsigned char data[15];

    if(movingCommand > _BIOPSY_MOVING_COMPLETED)
    {
        movingError = _BIOPSY_MOVING_ERROR_BUSY;
        movingCommand = _BIOPSY_MOVING_COMPLETED;
        return false;
    }

    data[0]=_MCC_BIOPSY_CMD_MOVE_XYZ; // Codice comando
    data[1]=(unsigned char) (X & 0x00FF);
    data[2]=(unsigned char) (X >>8);
    data[3]=(unsigned char) (Y & 0x00FF);
    data[4]=(unsigned char) (Y >>8);
    data[5]=(unsigned char) (Z & 0x00FF);
    data[6]=(unsigned char) (Z >>8);

    if(pConsole->pGuiMcc->sendFrame(MCC_BIOPSY_CMD,1,data,7)==FALSE)
    {
        movingCommand =_BIOPSY_MOVING_COMPLETED;
        movingError = _BIOPSY_MOVING_ERROR_MCC;
        return FALSE;
    }

    movingCommand =_BIOPSY_MOVING_XYZ;
    movingError = _BIOPSY_MOVING_NO_ERROR;
    return TRUE;
}

bool biopsy::moveHome(void)
{
    unsigned char data[1];

    if(movingCommand > _BIOPSY_MOVING_COMPLETED)
    {
        movingError = _BIOPSY_MOVING_ERROR_BUSY;
        movingCommand = _BIOPSY_MOVING_COMPLETED;
        return false;
    }

    data[0]=_MCC_BIOPSY_CMD_MOVE_HOME; // Codice comando
    if(pConsole->pGuiMcc->sendFrame(MCC_BIOPSY_CMD,1,data,1)==FALSE)
    {
        movingCommand =_BIOPSY_MOVING_COMPLETED;
        movingError = _BIOPSY_MOVING_ERROR_MCC;
        return FALSE;
    }

    movingCommand =_BIOPSY_MOVING_HOME;
    movingError = _BIOPSY_MOVING_NO_ERROR;
    return TRUE;
}


bool biopsy::moveDecZ(void)
{
    unsigned char data[1];
    if(movingCommand > _BIOPSY_MOVING_COMPLETED)
    {
        movingError = _BIOPSY_MOVING_ERROR_BUSY;
        movingCommand = _BIOPSY_MOVING_COMPLETED;
        return false;
    }


    data[0]=_MCC_BIOPSY_CMD_MOVE_DECZ; // Codice comando
    if(pConsole->pGuiMcc->sendFrame(MCC_BIOPSY_CMD,1,data,1)==FALSE)
    {        
        movingCommand =_BIOPSY_MOVING_COMPLETED;
        movingError = _BIOPSY_MOVING_ERROR_MCC;
        return FALSE;
    }

    movingCommand = _BIOPSY_MOVING_DECZ;
    movingError   = _BIOPSY_MOVING_NO_ERROR;
    return TRUE;
}


bool biopsy::moveIncZ(void)
{
    unsigned char data[1];
    if(movingCommand > _BIOPSY_MOVING_COMPLETED)
    {
        movingError = _BIOPSY_MOVING_ERROR_BUSY;
        movingCommand = _BIOPSY_MOVING_COMPLETED;
        return false;
    }


    data[0]=_MCC_BIOPSY_CMD_MOVE_INCZ; // Codice comando
    if(pConsole->pGuiMcc->sendFrame(MCC_BIOPSY_CMD,1,data,1)==FALSE)
    {
        movingCommand =_BIOPSY_MOVING_COMPLETED;
        movingError = _BIOPSY_MOVING_ERROR_MCC;
        return FALSE;
    }

    movingCommand = _BIOPSY_MOVING_INCZ;
    movingError   = _BIOPSY_MOVING_NO_ERROR;
    return TRUE;
}

bool biopsy::moveDecX(void)
{
    unsigned char data[1];
    if(movingCommand > _BIOPSY_MOVING_COMPLETED)
    {
        movingError = _BIOPSY_MOVING_ERROR_BUSY;
        movingCommand = _BIOPSY_MOVING_COMPLETED;
        return false;
    }


    data[0]=_MCC_BIOPSY_CMD_MOVE_DECX; // Codice comando
    if(pConsole->pGuiMcc->sendFrame(MCC_BIOPSY_CMD,1,data,1)==FALSE)
    {
        movingCommand =_BIOPSY_MOVING_COMPLETED;
        movingError = _BIOPSY_MOVING_ERROR_MCC;
        return FALSE;
    }

    movingCommand = _BIOPSY_MOVING_DECX;
    movingError   = _BIOPSY_MOVING_NO_ERROR;
    return TRUE;
}

bool biopsy::moveIncX(void)
{
    unsigned char data[1];
    if(movingCommand > _BIOPSY_MOVING_COMPLETED)
    {
        movingError = _BIOPSY_MOVING_ERROR_BUSY;
        movingCommand = _BIOPSY_MOVING_COMPLETED;
        return false;
    }


    data[0]=_MCC_BIOPSY_CMD_MOVE_INCX; // Codice comando
    if(pConsole->pGuiMcc->sendFrame(MCC_BIOPSY_CMD,1,data,1)==FALSE)
    {
        movingCommand =_BIOPSY_MOVING_COMPLETED;
        movingError = _BIOPSY_MOVING_ERROR_MCC;
        return FALSE;
    }

    movingCommand = _BIOPSY_MOVING_INCX;
    movingError   = _BIOPSY_MOVING_NO_ERROR;
    return TRUE;
}

bool biopsy::moveDecY(void)
{
    unsigned char data[1];
    if(movingCommand > _BIOPSY_MOVING_COMPLETED)
    {
        movingError = _BIOPSY_MOVING_ERROR_BUSY;
        movingCommand = _BIOPSY_MOVING_COMPLETED;
        return false;
    }


    data[0]=_MCC_BIOPSY_CMD_MOVE_DECY; // Codice comando
    if(pConsole->pGuiMcc->sendFrame(MCC_BIOPSY_CMD,1,data,1)==FALSE)
    {
        movingCommand =_BIOPSY_MOVING_COMPLETED;
        movingError = _BIOPSY_MOVING_ERROR_MCC;
        return FALSE;
    }

    movingCommand = _BIOPSY_MOVING_DECY;
    movingError   = _BIOPSY_MOVING_NO_ERROR;
    return TRUE;
}
bool biopsy::moveIncY(void)
{
    unsigned char data[1];
    if(movingCommand > _BIOPSY_MOVING_COMPLETED)
    {
        movingError = _BIOPSY_MOVING_ERROR_BUSY;
        movingCommand = _BIOPSY_MOVING_COMPLETED;
        return false;
    }


    data[0]=_MCC_BIOPSY_CMD_MOVE_INCY; // Codice comando
    if(pConsole->pGuiMcc->sendFrame(MCC_BIOPSY_CMD,1,data,1)==FALSE)
    {
        movingCommand =_BIOPSY_MOVING_COMPLETED;
        movingError = _BIOPSY_MOVING_ERROR_MCC;
        return FALSE;
    }

    movingCommand = _BIOPSY_MOVING_INCY;
    movingError   = _BIOPSY_MOVING_NO_ERROR;
    return TRUE;
}

bool biopsy::setStepVal(unsigned char step)
{
    unsigned char data[2];
    data[0]=_MCC_BIOPSY_CMD_SET_STEPVAL; // Codice comando
    data[1] = step;
    if(pConsole->pGuiMcc->sendFrame(MCC_BIOPSY_CMD,1,data,2)==FALSE) return false;
    return TRUE;
}

bool biopsy::setLunghezzaAgo(unsigned char len)
{
    unsigned char data[2];
    data[0]=_MCC_BIOPSY_CMD_SET_LAGO; // Codice comando
    data[1] = len;
    if(pConsole->pGuiMcc->sendFrame(MCC_BIOPSY_CMD,1,data,2)==FALSE) return false;
    return TRUE;
}



bool biopsy::updateConfig(void)
{
    // Invia la configurazione al driver per aggiornarlo in diretta
    return pConfig->sendMccConfigCommand(CONFIG_BIOPSY);

}


/* ____________________________________________________________________________________________________
 *  CALCOLO DELLA LESIONE
 *
 *  Vedere specifiche software
 *
 * _____________________________________________________________________________________________________*/


#define _FUOCO_TO_ROTAZIONE 6127.5 // dmm
#define _ROTAZIONE_TO_PELLICOLA 0 // dmm
#define _COS15 0.96592583
#define _SIN15 0.25881904
#define _BIO_REFY_OFFSET (470)  // Distanza Fuoco Reference
#define _BIO_REFX_OFFSET (0)    // Distanza Fuoco reference

#define _X0_BYM_TO_BIO_X0 260    // dmm Distanza X0 torretta to BIO-X
#define _Y0_BYM_TO_BIO_Y0 (470)  // Distanza Y0 torretta to BIO-Y
#define _Z0_BYM_TO_BIO_Y0 (1730) // Distanza Z0 torretta to BIO-Z (1930 - 200)

bool biopsy::calcLesionPosition(void){


    // Conversione delle coordinate con il coefficiente di calibrazione del reader
    // pBiopsy->readerKX
    // pBiopsy->readerKY
    float F_dmm_ref_p15_JX = (float) dmm_ref_p15_JX * config.readerKX;
    float F_dmm_ref_p15_JY = (float) dmm_ref_p15_JY * config.readerKY;
    float F_dmm_les_p15_JX = (float) dmm_les_p15_JX * config.readerKX;
    float F_dmm_les_p15_JY = (float) dmm_les_p15_JY * config.readerKY;
    float F_dmm_ref_m15_JX = (float) dmm_ref_m15_JX * config.readerKX;
    float F_dmm_ref_m15_JY = (float) dmm_ref_m15_JY * config.readerKY;
    float F_dmm_les_m15_JX = (float) dmm_les_m15_JX * config.readerKX;
    float F_dmm_les_m15_JY = (float) dmm_les_m15_JY * config.readerKY;

    PRINT(QString("[   P(15) -- REF-X:%1 REF-Y:%2 LES-X:%3 LES-Y:%4  ]").arg(F_dmm_ref_p15_JX).arg(F_dmm_ref_p15_JY).arg(F_dmm_les_p15_JX).arg(F_dmm_les_p15_JY));
    PRINT(QString("[   M(15) -- REF-X:%1 REF-Y:%2 LES-X:%3 LES-Y:%4  ]").arg(F_dmm_ref_m15_JX).arg(F_dmm_ref_m15_JY).arg(F_dmm_les_m15_JX).arg(F_dmm_les_m15_JY));



    // Controllo sui parametri ottenuti
    if( (F_dmm_ref_p15_JX==0) ||(F_dmm_ref_p15_JY==0) || (F_dmm_les_p15_JX==0) ||(F_dmm_les_p15_JY==0) ||
        (F_dmm_ref_m15_JX==0) ||(F_dmm_ref_m15_JY==0) || (F_dmm_les_m15_JX==0) ||(F_dmm_les_m15_JY==0)
      )
    {
        lesioneValida = false;
        calcError = ERROR_BIOP_INVALID_REFERENCES;
        Xlesione_dmm = (int) 0;
        Ylesione_dmm = (int) 0;
        Zlesione_dmm = (int) 0;
        Zfibra_dmm  = (int) 0;
        return false;
    }

    float L = _FUOCO_TO_ROTAZIONE * _SIN15;
    float H = _FUOCO_TO_ROTAZIONE * _COS15 + _ROTAZIONE_TO_PELLICOLA;

    // Coordinata a sinistra, generato dal tubo a +15
    float Xp = _BIO_REFX_OFFSET + F_dmm_ref_p15_JX - F_dmm_les_p15_JX ;
    float Yp = _BIO_REFY_OFFSET + F_dmm_ref_p15_JY - F_dmm_les_p15_JY;

    // Coordinata a destra, generato dal tubo a -15
    float Xm = _BIO_REFX_OFFSET + F_dmm_ref_m15_JX - F_dmm_les_m15_JX ;
    float Ym = _BIO_REFY_OFFSET + F_dmm_ref_m15_JY - F_dmm_les_m15_JY;

    float Xmp = abs(Xm-Xp);
    float Ymp = abs(Ym+Yp)/2;

    // Calcolo finale della lesione rispetto al BIO-REF
    Zbio = Xmp * H / (2*L + Xmp);
    Xbio = (2*L * Xp + L * Xmp)/( Xmp + 2*L);
    Ybio = (2*L * Ymp) / (Xmp + 2*L);

    PRINT(QString("[  CALC X:%1 Y:%2 Z:%3  ]").arg(Xbio).arg(Ybio).arg(Zbio));

    // Spostamento in coordinate torretta + offset di correzione da calibrazione
    Xlesione_dmm =  _X0_BYM_TO_BIO_X0 - (int) Xbio + pBiopsy->config.offsetX;
    Ylesione_dmm =  _Y0_BYM_TO_BIO_Y0 - (int) Ybio + pBiopsy->config.offsetY;
    Zlesione_dmm =  _Z0_BYM_TO_BIO_Y0 - (int) Zbio + pBiopsy->config.offsetZ;
    Zfibra_dmm   =  pBiopsy->config.offsetFibra * 10 - Zlesione_dmm;

    PRINT(QString("[  LES X:%1 Y:%2 Z:%3 ZF:%4 ]").arg(Xlesione_dmm).arg(Ylesione_dmm).arg(Zlesione_dmm).arg(Zfibra_dmm));

    // Controllo sullla raggiungibilità della lesione
    if(Zfibra_dmm <=40){
        lesioneValida = false;
        calcError = ERROR_BIOP_LESION_TOO_LOWER;
        PRINT("LESIONE Z < 4mm");
        return false;
    }

    // Controllo impatto con il compressore
    if(Zfibra_dmm >= pCompressore->breastThick *10){
        lesioneValida = false;
        calcError = ERROR_BIOP_LESION_TOO_HIGH;
        PRINT("LESIONE > THICKNESS");
        return false;
    }

    lesioneValida = true;
    calcError = 0;
    return true;

}
