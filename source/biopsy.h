#ifndef BIOPSY_H
#define BIOPSY_H

#include "application.h"


class biopsy : public QObject
{
    Q_OBJECT
public:
    explicit biopsy(QObject *parent = 0);
    void activateConnections(void);
    
    bool moveXYZ(unsigned short X, unsigned short Y, unsigned short Z); // Chiede il movimento sui tre assi
    bool moveZ(unsigned short Z); // Chiede il movimento su Z
    bool stepZ(int step); // Muove di un certo numero di steps
    bool moveX(unsigned short X); // Chiede il movimento su X
    bool moveY(unsigned short Y); // Chiede il movimento su Y
    bool updateTorretta(unsigned char id); // Effettua l'update dei dati precaricati da console
    void verifyAccessorio(void); // Verifica se deve attivare l'avviso di accessorio non corrispondente
    int  setBiopsyData(unsigned int x, unsigned int y, unsigned int z, // Posizione da raggiungere
                              unsigned int z_limit,     // Massima Z calcolata dalla AWS
                              unsigned int z_lesione,   // Posizione rilevata della lesione
                              unsigned int lAgo,        // Lunghezza dell'ago
                              unsigned int holder_code, // Codice holder utilizzato dalla AWS
                              QString  nome_accessorio,  // Nome dellk'accessorio montato da AWS
                              int cmd_id      // Id del comando richiesto
                              );

signals:
    
public slots:
    void mccStatNotify(unsigned char,unsigned char,QByteArray); // Notifica dal driver di gestione sul blocco STAT

public:
    bool connected;
    biopsyConf_Str config;          // Dati di configurazione
    bool openCfg(void);             // Funzione per l'apertura del file di configurazione
    bool storeConfig(void);         // Salva la configurazione
    bool updateConfig(void);        // Aggioirna M4 con i valori correnti

    unsigned char Lago;            // (mm) Lunghezza effettiva Ago
    unsigned char Zlimit;          // (mm) Posizione limite Z
    unsigned char Zlesione;        // (mm) Posizione limite Z
    QString        codiceAccessorio;// Nome accessorio selezionato
    QString        codiceAgo;       // Nome ago selezionato


    unsigned short curX;    // (0.1mm) Ultimo comando di movimento
    unsigned short curY;    // (0.1mm) Ultimo comando di movimento
    unsigned short curZ;    // (0.1mm) Ultimo comando di movimento
    int margZ;   // (mm)  Margine di spostamento della Z

    unsigned short maxZ;    // (mm) Massima escursione meccanica
    unsigned short targetX;    // (dam) Valore Target di movimento X
    unsigned short targetY;    // (dam) Valore Target di movimento Y
    unsigned short targetZ;    // (dam) Valore Target di movimento Z

    #define _BIOP_ACCESSORIO_ND 0
    #define _BIOP_ACCESSORIO_MAMMOTOME 1
    #define _BIOP_ACCESSORIO_AGO 2
    #define _BIOP_ACCESSORIO_GUN 3
    unsigned char accessorio; // Accessorio riconosciuto
    unsigned char accessorioSelezionato; // Accessorio impostato da Console

    unsigned char id_console;   // ID comando da console

    // Dati perifierica collegata
    unsigned char checksum_h;
    unsigned char checksum_l;
    unsigned char revisione;


};

#endif // BIOPSY_H
