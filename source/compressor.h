#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include "application.h"

class Compressor : public QObject
{
    Q_OBJECT
public:
    explicit Compressor(QObject *parent = 0);
    void activateConnections(void);

    typedef enum
    {
        COMPR_DISABLED=0, // Nessun movimento possibile
        COMPR_ZERO_MODE,  // Solo movimenti senza compressione
        COMPR_MANUALE,    // Solo attivazioni con Encoder (no pedali)
        COMPR_AUTO,       // Solo attivazioni con Pedali
        COMPR_FULL,       // Tutti i movimenti sono possibili
        COMPR_ND          // Stato non definito
    }comprStat_Enum;

    // Satus bit registro SYS_FLAGS1
    #define STARTUP_FLG 0x1     // Startup mode in corso
    #define COMPR_FLG 0x2       // Compressione rilevata


signals:
    
public slots:
    void pcb215Notify(unsigned char id, unsigned char notifyCode, QByteArray buffer);


public:
    void padChanged(void);

    Pad_Enum getPad() {return comprPad;}
    unsigned char getForce(){return comprStrenght;}

    // Configurazione
    bool readConfigFile(void);
    bool storeConfigFile(void);
    bool readPadCfg(void);      // Lettura file di configurazione livelli di riconoscimento del pad
    bool storePadCfg(void);     // Salvataggio dei dati di configurazione
    void calibrateThresholds(unsigned char ncc); // Calcola le soglie corrette per il sistema vero

    bool configUpdate; // Configurazione compressore aggiornata
    compressoreCnf_Str config;



    bool isCompressed() {return (comprFlags1 & COMPR_FLG);} // TRUE se il carrello è sotto compressione
    bool isValidPad()   {return (comprPad<PAD_ENUM_SIZE);} // TRUE se il carrello è sotto compressione
    bool isValidPad(Pad_Enum code)   {return (code<PAD_ENUM_SIZE);} // TRUE se il carrello è sotto compressione
    QString getPadName(void);                           // Restituisce il nome simboloico associato al Pad
    QString getPadName(Pad_Enum code);           // Restituisce il nome simbolico associato al Pad
    QString getPadTag(Pad_Enum code);            // Restituisce il TAG associato al codice numerico PAD

    int getPadCodeFromTag(QString tag);          // Restituisce il codice numerico assocciato al Tag stringa

    comprStat_Enum comprStat; // Modalit�  d'uso del compressore
    unsigned char  comprFlags0;   // Flags interni compressore
    unsigned char  comprFlags1;   // Flags interni compressore
    unsigned char  target_compressione;
    unsigned short posizione;

    unsigned char  comprStrenght; // Livello di compressione corrente
    Pad_Enum       comprPad;     // Pad detected (vedi collimatore)
    unsigned char  comprMagnifier; // FAttore di ingrandimento

    int            comprOffset;  // Offset relativo al PAD corrente
    int            breastThick;  // Spessore seno calcolato


    QList<QString> padTags;             // Tags per il collimatore
    QList<QString> padNames;            // Nomi da visualizzare a display
    pad_Str        defPad;              // Carica i valori di default da assegnare ai compressori non configurati

    // Valore di tensione della batteria
    float battery;

    bool enable_compressione_closed_study;

    // Errori attivi:
    bool battery_low;
    bool battery_fault;
    bool safety_fault;
    bool fault;

};

#endif // COMPRESSOR_H
