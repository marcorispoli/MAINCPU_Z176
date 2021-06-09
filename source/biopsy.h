#ifndef BIOPSY_H
#define BIOPSY_H

#include "application.h"


class biopsy : public QObject
{
    Q_OBJECT
public:
    explicit biopsy(QObject *parent = 0);
    void activateConnections(void);
    
    // Movimenti su tre assi
    bool moveXYZ(unsigned short X, unsigned short Y, unsigned short Z); // Chiede il movimento sui tre assi
    bool moveHome(void); // Chiede il movimento verso home

    // Movimenti per step
    bool setStepVal(unsigned char step);
    bool moveDecZ(void);
    bool moveIncZ(void);
    bool moveDecX(void);
    bool moveIncX(void);
    bool moveDecY(void);
    bool moveIncY(void);

    // Impostazione lunghezza Ago
    bool setLunghezzaAgo(unsigned char len);

    // Calcolo lesione
    bool calcLesionPosition(void); // True = lesione correttamente calcolata

signals:
    
public slots:
    void mccStatNotify(unsigned char,unsigned char,QByteArray); // Notifica dal driver di gestione sul blocco STAT

public:
    bool connected;
    biopsyConf_Str config;          // Dati di configurazione
    bool openCfg(void);             // Funzione per l'apertura del file di configurazione
    bool storeConfig(void);         // Salva la configurazione
    void defaultConfigData(void);

    bool updateConfig(void);        // Aggioirna M4 con i valori correnti

    // Comandi in corso
    int movingCommand;
    #define _BIOPSY_MOVING_NO_COMMAND       0
    #define _BIOPSY_MOVING_COMPLETED        1
    #define _BIOPSY_MOVING_XYZ              2
    #define _BIOPSY_MOVING_HOME             3
    #define _BIOPSY_MOVING_DECZ             4
    #define _BIOPSY_MOVING_INCZ             5
    #define _BIOPSY_MOVING_DECX             6
    #define _BIOPSY_MOVING_INCX             7
    #define _BIOPSY_MOVING_DECY             8
    #define _BIOPSY_MOVING_INCY             9

    int  movingError;
    #define _BIOPSY_MOVING_NO_ERROR         0
    #define _BIOPSY_MOVING_ERROR_MCC        1
    #define _BIOPSY_MOVING_ERROR_TIMEOUT    2
    #define _BIOPSY_MOVING_ERROR_TARGET     3
    #define _BIOPSY_MOVING_ERROR_BUSY       6

    // Posizione del joystic
    int dmmJX;
    int dmmJY;

    // References acquisiti
    float dmm_ref_p15_JX;
    float dmm_ref_p15_JY;
    float dmm_les_p15_JX;
    float dmm_les_p15_JY;
    float dmm_ref_m15_JX;
    float dmm_ref_m15_JY;
    float dmm_les_m15_JX;
    float dmm_les_m15_JY;



    // Calcolo rispetto al fuoco del tubo
    float Xbio,Ybio,Zbio;

    // Coordinate calcolate della lesione
    int Xlesione_dmm;    // X lesione rispetto allo zero torretta
    int Ylesione_dmm;    // Y lesione rispetto allo zero torretta
    int Zlesione_dmm;    // Z lesione rispetto allo 0 torretta
    int Zfibra_dmm;      // Z lesione rispetto alla fibra di carbonio

    bool           lesioneValida;   // il dato sulla posizione della lesione è valido
    unsigned char  calcError;       // Codice di errore in caso di calcolo impossibile


    // Posizione corrente della torretta
    unsigned short curX_dmm;    // (0.1mm) Ultimo comando di movimento
    unsigned short curY_dmm;    // (0.1mm) Ultimo comando di movimento
    unsigned short curZ_dmm;    // (0.1mm) Ultimo comando di movimento

    // Limiti di movimento
    unsigned short margZ_mm;     // (mm)  Margine di spostamento della Z con l'ago inserito per non toccare la fibra
    unsigned short maxZ_mm;      // (mm)  Massima escursione meccanica prima di toccare il compressore
    unsigned char  zlim_mm;      // Massima escursione possibile di Z calcolata dal driver in funzione della lunghezza ago
    unsigned char  lunghezzaAgo; // Lunghezza corrente dell'ago
    unsigned char  stepVal;      // STEPVAL corrente


    // Target di movimento impostato
    unsigned short targetX_dmm;    // (dam) Valore Target di movimento X
    unsigned short targetY_dmm;    // (dam) Valore Target di movimento Y
    unsigned short targetZ_dmm;    // (dam) Valore Target di movimento Z

    // Accessorio riconosciuto
    unsigned char adapterId;       // adapterId riconosciuto

    // Dati perifierica collegata
    unsigned char checksum_h;
    unsigned char checksum_l;
    unsigned char revisione;

};

#endif // BIOPSY_H
