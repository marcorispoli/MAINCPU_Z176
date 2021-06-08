#include "mcccom.h"
#include <QDebug>
#include <QFile>
#include <QRunnable>
#include <QThreadPool>

unsigned char mccCom::mccID=0;      // Inizializza il contatore degli Id sulla classe base


mccCom::mccCom(unsigned char core, unsigned char node, unsigned char port,bool mode) :
    QObject(0)
{
    mccComRx *mccThObj;
    MCC_INFO_STRUCT mccDriverInfo; // Info Driver MCC

    qRegisterMetaType<_MccFrame_Str>("_MccFrame_Str");

    // Inizializzazione del nodo di comunicazione con Core M4
    if(mccID==0)
    {
        // Apertura del driver
        mcc_initialize(0);

        // Acquisizione della revisione
        if(mcc_get_info(0,&mccDriverInfo)!=MCC_SUCCESS)
        {
            // ERRORE DI SISTEMA
            return;
        }
        int maj = QString(mccDriverInfo.version_string).toInt() / 10;
        int min = QString(mccDriverInfo.version_string).toInt() % 10;
        mccRev = QString("%1.%2").arg(maj).arg(min);

    }
    mccID++;

    if(mode==FALSE)
    {// Modo Trasmissione
        // Assegna end point di trasmissione
        tx_ep.core =core;
        tx_ep.node =node;
        tx_ep.port =port;
    }else
    {// Modo ricezione
        mccThObj = new mccComRx;      // Crea oggetto contenente il codice per la thread

        // Assegna end point di trasmissione
        mccThObj->rx_ep.core =core;
        mccThObj->rx_ep.node =node;
        mccThObj->rx_ep.port =port;

        // Crea la thread e cambia l'affinit√  dell'oggetto mccThObj nella nuova thread
        QThread *Thread = new QThread(this);    // Crea la thread
        connect(Thread, SIGNAL(started()), mccThObj, SLOT(mccThreadRx()),Qt::QueuedConnection);
        connect(Thread, SIGNAL(finished()), mccThObj, SLOT(deleteLater()),Qt::QueuedConnection);
        mccThObj->moveToThread(Thread);

        // Effettua la connect tra threads differenti per poter comunicare l'evento di ricezione
        connect(mccThObj,SIGNAL(mccRxSgn(_MccFrame_Str)),this,SLOT(mccRxHandler(_MccFrame_Str)),Qt::BlockingQueuedConnection);

        mccThObj->pThread = Thread;

        // Starts an event loop, and emits workerThread->started()
        Thread->start();
    }

    max_len = 0;
    return;
}


////////////////////////////////////////////////////////////////////
/*
 *  Aggiunge il checksum al buffer del messaggio
 *
 */
////////////////////////////////////////////////////////////////////
void mccCom::setChecksum(void)
{
    int i;
    mcc_cmd.checksum=0;
    for(i=0;i<mcc_cmd.len;i++)
        mcc_cmd.checksum ^= mcc_cmd.buffer[i];

    return;
}

void mccCom::setChecksum(_MccFrame_Str* mccframe)
{
    int i;
    mccframe->checksum=0;
    for(i=0;i<mccframe->len;i++)
        mccframe->checksum ^= mccframe->buffer[i];

    return;
}

////////////////////////////////////////////////////////////////////
/*
 *  Prepara il messaggio da inviare a M4 secondo il protocollo
 *  interno
 *
 */
////////////////////////////////////////////////////////////////////
bool mccCom::setFrame(unsigned char cmd, unsigned char id, unsigned char* buffer, unsigned char len)
{
    int i;

    // Controllo sulla dimensione del messaggio
    if (len > _MCC_DIM) return FALSE;


    mcc_cmd.cmd = cmd;
    mcc_cmd.id = id;
    mcc_cmd.len = len;
    mcc_cmd.checksum = 0;
    for(i=0; i<len; i++)
    {
        mcc_cmd.buffer[i] = buffer[i];
        mcc_cmd.checksum ^= buffer[i];
    }
    return TRUE;
}
////////////////////////////////////////////////////////////////////
/*
 *  Prepara il messaggio da inviare a M4 secondo il protocollo
 *  interno
 *
 */
////////////////////////////////////////////////////////////////////
bool mccCom::sendFrame(unsigned char cmd, unsigned char id, unsigned char* buffer, unsigned char len)
{
    if(setFrame(cmd,id,buffer,len)==FALSE)
    {
        return FALSE;
    }
    return sendFrame();
}

////////////////////////////////////////////////////////////////////
/*
 *  Invia messaggio senza buffer
 *
 */
////////////////////////////////////////////////////////////////////
bool mccCom::sendCmd(unsigned char cmd,unsigned char id)
{

    mcc_cmd.cmd = cmd;
    mcc_cmd.id = id;
    mcc_cmd.len=0;
    mcc_cmd.checksum = 0;
    return sendFrame();
}
////////////////////////////////////////////////////////////////////
/*
 *  Prepara il messaggio da inviare a M4 secondo il protocollo
 *  interno
 *
 */
////////////////////////////////////////////////////////////////////
bool mccCom::setFrame(unsigned char cmd, unsigned char id, QByteArray* buffer)
{
    int i;

    // Controllo sulla dimensione del messaggio
    if (buffer->size() > _MCC_DIM) return FALSE;

    mcc_cmd.cmd = cmd;
    mcc_cmd.id=id;
    mcc_cmd.len = buffer->size();
    mcc_cmd.checksum = 0;
    for(i=0; i<mcc_cmd.len; i++)
    {
        mcc_cmd.buffer[i] = buffer->at(i);
        mcc_cmd.checksum ^= buffer->at(i);
    }
    return TRUE;

}




////////////////////////////////////////////////////////////////////
/*
 *  Invia il comando verso M4 con protocollo MCC (interno)
 *  Il comando deve gi√  essere stato pre-caricato con l'opportuna
 *  funzione di interfaccia
 */
////////////////////////////////////////////////////////////////////
bool mccCom::sendFrame(void)
{
    unsigned int len;

    // Verifica se busy
    mcc_msgs_available(&tx_ep,&len);

    if(len>MAX_MCC_QUEUE) return FALSE ; // C'√® gi√  un messaggio in coda


    //mccSendThread *st = new mccSendThread(tx_ep,mcc_cmd);
    //QThreadPool::globalInstance()->start(st);

    // Invia a M4
    if(mcc_send(&tx_ep,(void*)&mcc_cmd,sizeof(_MccFrame_Str),0)!=MCC_SUCCESS) return FALSE;
    return TRUE;
}

void mccCom::sendBuffer(QByteArray frame)
{
    int i;
    unsigned int len;
    _MccFrame_Str mcc_cmd;


    mcc_msgs_available(&tx_ep,&len);
    if(len>MAX_MCC_QUEUE) return;

    mcc_cmd = this->fromQByteArray(&frame);


   //mccSendThread *st = new mccSendThread(tx_ep,mcc_cmd);
   // QThreadPool::globalInstance()->start(st);

    // Invia a M4
    mcc_send(&tx_ep,(void*)&mcc_cmd,sizeof(_MccFrame_Str),0);

    return ;
}



////////////////////////////////////////////////////////////////////
/*
 *  Funzione per la gestione dei messaggi ricevuti da M4
 *  La funzione √® virtuale e pu√≤ essere reimplementata
 *  per rendere pi√π veloce la gestione dei messaggi
 *  senza dover utilizzare le signals
 */
////////////////////////////////////////////////////////////////////
void mccCom::mccRxHandler(_MccFrame_Str mccframe)
{
    int i;

    QByteArray data;
    data.resize(sizeof(_MccFrame_Str));
    for(i=0;i<data.size();i++)
        data[i] = ((unsigned char*) &mccframe)[i];

    emit mccRxSignal(data);
    //emit mccRxFrameSignal(mccframe);
}

////////////////////////////////////////////////////////////////////
/*
 *  La funzione restituisce un mcc frame da un QByteArray
 */
////////////////////////////////////////////////////////////////////
_MccFrame_Str mccCom::fromQByteArray(QByteArray* data)
{
    _MccFrame_Str mccframe;
    int i;

    for(i=0; i<data->size(); i++)
            ((unsigned char*)&mccframe)[i] = data->at(i);
    return mccframe;
}


QByteArray mccCom::toQByteArray(unsigned char cmd, unsigned char id, unsigned char* buffer, int len)
{
    QByteArray data;
    _MccFrame_Str mccframe;
    int i;

    mccframe.cmd = cmd;
    mccframe.id = id;
    mccframe.len = len;
    for(i=0;i<len;i++)
        mccframe.buffer[i] = buffer[i];
    mccCom::setChecksum(&mccframe);

    data.resize(sizeof(_MccFrame_Str));
    for(i=0;i<data.size();i++)
        data[i] = ((unsigned char*) &mccframe)[i];

    return data;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
// THread di gestione dei comandi da MASTER M4
////////////////////////////////////////////////////////////////////
/*
 *  Verifica se il messaggio √® corretto
 */
////////////////////////////////////////////////////////////////////
bool mccComRx::isFrameCorrect(_MccFrame_Str* mccframe)
{
    int i;
    unsigned char chk=0;

    if(mccframe->len>_MCC_DIM) return FALSE;

    for(i=0;i<mccframe->len;i++)
        chk^=mccframe->buffer[i];

    if(chk!=mccframe->checksum) return FALSE;
    return TRUE;
}

void mccComRx::mccThreadRx(void)
{
    int mcc_len;
    int i;
    unsigned char chk;

    // Creazione end-point di ricezione
    if(mcc_create_endpoint(&rx_ep,rx_ep.port)!=MCC_SUCCESS) exit(-1);

    // Ciclo di attesa da M4

    while(1)
    {
        mcc_len = 0;
        if((mcc_recv_copy(&rx_ep,&mcc_cmd,sizeof(mcc_cmd),(MCC_MEM_SIZE*) &mcc_len,0xFFFFFFFF)==MCC_SUCCESS) && (mcc_len))
        {
            // Controlla il pacchetto e chiama la funzione della classe mccParent di gestione del comando
            if(isFrameCorrect(&mcc_cmd)) emit mccRxSgn(mcc_cmd);
        }

    }

}
