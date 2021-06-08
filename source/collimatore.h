#ifndef COLLIMATORE_H
#define COLLIMATORE_H


#include "application.h"

//#define COLLI_DBG

class Collimatore : public QObject
{
    Q_OBJECT
public:
    explicit Collimatore(QObject *parent = 0);
    void activateConnections(void);


typedef enum
{
    MIRROR_HOME= 0,
    MIRROR_OUT,
    MIRROR_ND
}_MirrorCmd_Enum;

typedef enum
{
   LAMP_OFF=0,
   LAMP_ON,
   LAMPMIRR_OFF,
   LAMPMIRR_ON
}_LampCmd_Enum;

typedef enum
{
    FILTRO_Rh=0,
    FILTRO_Al,
    FILTRO_Ag,
    FILTRO_Cu,
    FILTRO_Mo,
    FILTRO_ND
}_FilterCmd_Enum;

signals:
    

public:
    #define _COLLI_FORMAT_UNDEFINED     0
    #define _COLLI_FORMAT_24x30         1
    #define _COLLI_FORMAT_18x24         2
    #define _COLLI_FORMAT_BIOPSY        3
    #define _COLLI_FORMAT_MAGNIFIER     4
    #define _COLLI_FORMAT_MANUAL        5

    #define _FACTORY_MIRROR_STEPS_ASSY01 1530
    #define _FACTORY_MIRROR_STEPS_ASSY02 1760

    // Definizione dei valory di FACTORY
    #define _FACTORY_FRONT_LARGE_FOCUS  30
    #define _FACTORY_FRONT_SMALL_FOCUS  20
    #define _FACTORY_LEFT_24x30         70
    #define _FACTORY_LEFT_18x24         120
    #define _FACTORY_LEFT_MAGNIFIER     230
    #define _FACTORY_LEFT_BIOPSY        230
    #define _FACTORY_RIGHT_24x30        65
    #define _FACTORY_RIGHT_18x24        120
    #define _FACTORY_RIGHT_MAGNIFIER    230
    #define _FACTORY_RIGHT_BIOPSY       230
    #define _FACTORY_BACK_24x30         35
    #define _FACTORY_BACK_18x24         115
    #define _FACTORY_BACK_MAGNIFIER     190
    #define _FACTORY_BACK_BIOPSY        190





    static unsigned char colliFormatFromPad(int pad);

    // Azionamento Specchio
    void setMirror(_MirrorCmd_Enum cmd);                    // Imposta lo stato dello Specchio
    void setToggleMirrorLamp(unsigned short steps);         // Effettua un toggle dello specchio con il numero adeguato di steps
    void setLamp(_LampCmd_Enum param, unsigned char tmo);   // Attivazione Lampada centratore
    bool updateColli(void);                                 // Effettua l'update del collimatore
    bool manualColliUpdate(void);                           // Modalit√  di collimazione manuale

    bool setFiltro(void);                                   // Impostazione filtro predefinito a filter_Cmd
    bool setFiltro(_FilterCmd_Enum cmd, bool update);         // Impostazione del filtro richiesto
    bool manualSetFiltro(void);                             // Impostazione filtro in modo manuale
    bool manualSetFiltro(unsigned char index);

    QString getFiltroTag(unsigned char code);               // Restituisce il tag simbolico per il dato codice filtro
    _FilterCmd_Enum getFiltroStat(void){return filtroStat;} // Restituisce lo stato del filtro

    int getColli2DIndex(int pad);              // Restituisce l'indice della lista dei dati di collimazione 2D

    void startColliTest(unsigned char nseq);
    void timerEvent(QTimerEvent* ev); // Override della classe QObject


public slots:
    void guiNotifySlot(unsigned char id, unsigned char mcccode, QByteArray buffer); // Notifica esecuzione azionamento specchio
    void changedPadNotify(void);
    void pcb249U1Notify(unsigned char id, unsigned char notifyCode, QByteArray buffer);


private:
    _MirrorCmd_Enum mirrorStat;   // 0=fuori campo, 1=in campo, 2=indeterminato    
    _FilterCmd_Enum filtroStat; // Stato corrente del filtro

    int colliTestNumber;
    int colliTestTimer;
    bool colliTestStatus;

public:
    unsigned char colli_model;

    bool     alrCuffia;     // Allarme temperatura cuffia
    bool     alrSensCuffia; // Allarme temperatura cuffia
    bool     lampStat;      // Stato Lampada centratore: TRUE = ACCESA

    unsigned char accessorio; // Accessorio riconosciuto

    // Gestione file di configurazione
    // Hotfix 11C
    #define COLLI_CNF_REV   2   // Revisione file di configurazione del collimatore

    bool colliConfUpdated;      // Configurazione aggiornata
    colliConf_Str colliConf;    // Configurazione collimatore

    // Lettura/Scrittura files di configurazione: il file di configurazione dipente dal Gantry Type
    bool readConfigFile(void) ;
    bool storeConfigFile(void);



    // Ultimi comandi attivati
    _FilterCmd_Enum filtroCmd;  // Richiesta stato filtro durante updateColli

    // DATI PER COLLIMAZIONE MANUALE
    bool manualCollimation;     // Attiva l'impostazione manuale della collimazione
                                // sganciandola dalla selezione del PAD corrente
    bool manualFiltroCollimation;     // Attiva l'impostazione manuale della collimazione



    unsigned char manualFilter; // Impostazione manuale del filtro
    int manualL;
    int manualR;
    int manualT;
    int manualB;
    int manualF;

    // Dati per collimazione custom salvata
    int customL;
    int customR;
    int customT;
    int customB;
    int customF;


    QString getFilterTag(unsigned char code)
    {
        if(code==FILTRO_Rh) return QString("Rh");
        else if(code==FILTRO_Al) return QString("Al");
        else if(code==FILTRO_Ag) return QString("Ag");
        else if(code==FILTRO_Cu) return QString("Cu");
        else if(code==FILTRO_Mo) return QString("Mo");
        return QString("UNDEFINED");
    }

    // Imposta il comando filtro utilizzando un TAG e ne verifica l'esattezza
    bool setFilterTag(QString tag)
    {
        if(getFilterTag(FILTRO_Rh) == tag) filtroCmd = FILTRO_Rh;
        else if(getFilterTag(FILTRO_Al) == tag) filtroCmd = FILTRO_Al;
        else if(getFilterTag(FILTRO_Ag) == tag) filtroCmd = FILTRO_Ag;
        else if(getFilterTag(FILTRO_Cu) == tag) filtroCmd = FILTRO_Cu;
        else if(getFilterTag(FILTRO_Mo) == tag) filtroCmd = FILTRO_Mo;
        else return FALSE;

        return TRUE;

    }

private:
#ifdef COLLI_DBG
    void printCnf(void);
#endif
};

#endif // COLLIMATORE_H
