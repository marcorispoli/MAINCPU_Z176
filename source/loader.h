#ifndef LOADER_H
#define LOADER_H

#include "application.h"
#include <QWidget>

namespace Ui {
class Loader;
}

class Loader : public QWidget
{
    Q_OBJECT
    
public:
    explicit Loader(int rotation, QWidget *parent = 0);
    ~Loader();

    // Tipi di record gestiti nell'HEX FILE
    #define HEX_RECORD_DATA             0x0 // DATA
    #define HEX_RECORD_HEADER           0x4 //
    #define HEX_RECORD_HEADER_SPECIAL   0x5
    #define HEX_RECORD_END              0x1



    bool readConfig(unsigned char target, unsigned char uC); // Legge la Congig area di un target

    // BLOCCHI DA 16 WORD
    typedef struct
    {
        unsigned short startAddr;       // Indirizzo di partenza
        unsigned short val[16];         // Blocchi da 16 word
        unsigned short len;             // Numero di word validi
    }_addrStr;



    typedef struct{

        QString file;
        unsigned char loaderAddr;
        unsigned char uC;
        QString devTag;
    }_itemDownload;

    typedef struct
    {
      QString filename;  // Nome del file aperto
      unsigned short   crc; // Crc calcolato come somma di tutti i byte scritti
      QList<_addrStr> progSegment;
      unsigned short configWord;
      unsigned short ID[4];
      QString intestazioneFile;
      QString crcString; // CRC file in formato stringa
    }_pic16FhexFileStr;

    bool openPic16FHexFile(QString filename);
    void uploadPic16FHexFile(unsigned char indirizzo, unsigned char uC, QString file);


signals:
    void readConfigSgn(_picConfigStr); // Ricevuto risultato lettura config area
    void loaderCompletedSgn(bool esito, QString errstr);

public slots:
    void loaderNotify(unsigned char,unsigned char,QByteArray);

    void readConfigNotify(unsigned char id,unsigned char cmd,QByteArray data);
    bool startDownloadHexFile(_itemDownload item);
    void manualFirmwareUpload(unsigned char target, unsigned char uC, QString file, QString tag);


    // Funzioni di interfaccia per l'aggiornamento del sistema
    void firmwareUpdate(void);
    bool readHexRow(QFile* pFile, QByteArray* row, int* nbyte);
    int  verifyHeader(QString filename, QString revCode, unsigned short* calc_crc, QString* file_rev);
    void manualFirmwareUploadNotify(bool esito, QString errstr);

private:
    bool mccLoader(_MccLoaderNotify_Code cmd, QByteArray data); // Compone i comandi per il sotto gruppo MCC_LOADER
    bool mccLoader(_MccLoaderNotify_Code cmd);// Comando senza buffer dati
    bool mccLoader(void);// Utilizza i dati dell'ultima emissione
    bool onDownloadErr(QString error);

    Ui::Loader *ui;

    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsProxyWidget *proxy;
    _pic16FhexFileStr pic16Fhex;

    // Sezione di variabili per il trasferimento dati
    int dataIndex;

    // LOADER: AGIORNAMENTO IN CORSO
    _MccLoaderNotify_Code pendingAction; // Azione in corso
    QByteArray pendingData;
    unsigned char pendingId;
    unsigned short pendingBlock;

    unsigned char pendingAddress;
    unsigned char pendinguC;

    QList<_itemDownload> pendingDownload;
    _itemDownload curItem;    // Dati download in corso

    int download_cur_perc;    // Totale percentuale di aggiornamento
    int download_delta_perc;  // Delta aggiornamento per unità di download
    int prev_cur_perc;        // utilizzato per inviare solo a cambio numero

    bool manualMode;          // download manuale di un singolo file
public:


};

#endif // LOADER_H
