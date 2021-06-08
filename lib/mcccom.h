#ifndef MCCCOM_H
#define MCCCOM_H

#include <QObject>
#include <QThread>
#include <QRunnable>

extern "C" // Necessario per utilizzare simboli da libreria dinamica
{
#include <libmcc.h>
}

#define _MCC_DIM 150
#define MAX_MCC_QUEUE 5

typedef struct
{
  unsigned char cmd;
  unsigned char id;  // Id di riconoscimento inviato da chiamante e restituito nelle notifiche
  unsigned char len;
  unsigned char checksum;
  unsigned char buffer[_MCC_DIM];
}_MccFrame_Str;

////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *  La classe in oggetto permette di gestire la comunicazione con M4 Core sia in trasmissione
 *  che in ricezione (se viene aperta la comunicazione).
 *  L'istanza della classe permette di aprire un canale in trasmissione. L'indirizzo
 *  dell'endpoint viene passato al costruttore della classe come segue:
 *          pointer = new mccCom(core, nodo, porta);
 *
 *  Per inviare un comando con il protocollo mcc bisogna prima settare il comando
 *  con una delle due versioni disponibili di funzioni:
 *
 *      pointer->setFrame(comando, char* buffer, uchar lunghezza)
 *      pointer->setFrame(comando, QByteArray buffer)
 *
 *  l'invio del comando avviene con la fuinzione
 *      pointer->sendFrame()
 *
 *  La ricezione da MCC avviene istanziando la classe ed aprendo un endpoint di
 *  ricezione come segue:
 *          pointer = new mccCom(core, nodo, porta); Istanza la classe e assegna un ep di trasmissione
 *          pointer->open(core, nodo, porta); Apre un endpoint di ricezione
 *
 *  Se la classe non viene re-implementata, in seguito alla ricezione di un buffer corretto, vengono
 *  emessi due segnali:
 *
 *  ---> SIGNAL  mccRxSignal(QByteArray data)
 *  ---> SIGNAL  mccRxFrameSignal(_MccFrame_Str* mccframe)
 *
 *  Tale segnale può essere utilizzato associato ad uno slot per gestire il buffer mcc ricevuto.
 *
 *  Per velocizzare il processo di ricezionoe senza passare dal meccanismo SIGNAL/SLOTS è
 *  possibile reimplementare la classe e reimplementare la funzione virtuale
 *      void mccRxHandler(_MccFrame_Str mccframe);
 *
 *  gestendo direttamente il messaggio ricevuto
 *
 */
////////////////////////////////////////////////////////////////////////////////////////////////////////


class mccCom : public QObject
{
    Q_OBJECT
public:

    // mode==FALSE -> Tx; mode==TRUE-> Rx
    explicit mccCom(unsigned char core, unsigned char node, unsigned char port,bool mode) ;
    //void openRx(unsigned char core, unsigned char node, unsigned char port);

    bool sendCmd(unsigned char cmd,unsigned char id);
    bool sendFrame(void);  // Invia il Frame verso M4 all'EP indicato
    bool sendFrame(unsigned char cmd, unsigned char id, unsigned char* buffer, unsigned char len); // Crea il frame da spedire
    bool setFrame(unsigned char cmd, unsigned char id, unsigned char* buffer, unsigned char len); // Crea il frame da spedire
    bool setFrame(unsigned char cmd, unsigned char id, QByteArray* buffer); // Crea il frame da spedire
    void setChecksum(void); // Imposta il checksum internamente
    static void setChecksum(_MccFrame_Str* mccframe); // IMposta il checksum nella struttura passata

    static _MccFrame_Str fromQByteArray(QByteArray* data); // Restituisce un mcc frame
    static QByteArray toQByteArray(unsigned char cmd, unsigned char id, unsigned char* buffer, int len);

signals:
    // Segnali generati in caso di ricezione frame
    void mccRxSignal(QByteArray data);
    void mccRxFrameSignal(_MccFrame_Str* mccframe);

public slots:
    // Se non viene reimplementata la funzione emette la mccRxSignal
    virtual void mccRxHandler(_MccFrame_Str mccframe);
    void sendBuffer(QByteArray frame); // Utilizzato per effettuare tunnel su sockets

public:
    static unsigned char mccID;
    _MccFrame_Str mcc_cmd;

    unsigned short max_len;    
    QString mccRev;

private:
    MCC_ENDPOINT tx_ep;   


};

// Oggetto di gestione di una thread di ricezione
class mccComRx : public QObject
 {
    Q_OBJECT

signals:
    void mccRxSgn(_MccFrame_Str  mccframe);  // Segnale di ricezione del pacchetto mcc da inviare a gui

public slots:
     void mccThreadRx(void); // Funzione di inizio thread

public:
     bool isFrameCorrect(_MccFrame_Str* mccframe); // Verifica se il frame è corretto
     _MccFrame_Str mcc_cmd;
     MCC_ENDPOINT rx_ep;  // End point di ricezione

     QThread *pThread;
};

class mccSendThread : public QRunnable
 {
public:
    mccSendThread(MCC_ENDPOINT ep,_MccFrame_Str mcc){
        mcc_cmd = mcc;
        tx_ep = ep;
    }

     void run()
     {
         mcc_send(&tx_ep,(void*)&mcc_cmd,sizeof(_MccFrame_Str),0);
         return;
     }

     _MccFrame_Str mcc_cmd;
     MCC_ENDPOINT tx_ep;
 };

#endif // MCCCOM_H
