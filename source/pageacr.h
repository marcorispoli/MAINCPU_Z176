#ifndef PAGEACR_H
#define PAGEACR_H
#include "application.h"

class PageACR : public GWindow
{
    Q_OBJECT
public:
    PageACR(bool ls, QString bg,QString bgs ,bool showLogo,int w,int h, qreal angolo,QPainterPath pn, int pgpn, QPainterPath pp, int pgpp, int pg);
    virtual ~PageACR();
    void childStatusPage(bool stat,int param); // Override funzione della classe base GWindow
                                                // Al cambio pagina riporta lo stato di attivazione
    void timerEvent(QTimerEvent* ev); // Override della classe QObject
    void mousePressEvent(QGraphicsSceneMouseEvent* event); // Override funzione della classe base GWindow


    enum ACR_VIEW
    {
        _ACR_CC_VIEW=0x1,
        _ACR_XCC_VIEW=0x101,
        _ACR_CCCV_VIEW=0x401,
        _ACR_SIO_VIEW=0x80,
        _ACR_LM_VIEW=0x40,
        _ACR_LMO_VIEW=0x20,
        _ACR_FB_VIEW=0x10,
        _ACR_ML_VIEW=0x4,
        _ACR_MLO_VIEW=0x2,
        _ACR_MLOAT_VIEW=0x802,
        _ACR_UNDEF_VIEW=0
    };

    enum ACR_SUFFIX
    {
        _ACR_ID=0x1,
        _ACR_TAN=0x2,
        _ACR_S=0x4,
        _ACR_RL=0x8,
        _ACR_RM=0x10,
         _ACR_UNDEF_SUFFIX=0
    };

    enum ACR_BREAST
    {
        _ACR_R_BREAST=0x1000,
        _ACR_L_BREAST=0,
        _ACR_UNDEF_BREAST=0
    };


signals:


public slots:
    void valueChanged(int,int); // Link esterno alla fonte dei contenuti dei campi valore
    void languageChanged(void); // Link esterno alla fonte dei contenuti dei campi valore
    void buttonActivationNotify(int id,bool status,int opt);
    void setView();
    void updateEnables();

public:
    unsigned short getAcrView();
    unsigned char getAcrSuffix();


private:
    int timerId; // Usato per la gestione del timer della data
    bool disableTimedButtons; //  Disabilitazione a tempo dei bottoni per evitare rimbalzi
    int timerDisableButton;
    void disableButtons(int t){
        if(disableTimedButtons) return;
        disableTimedButtons=true;
        timerDisableButton = startTimer(t);
    }

    QPixmap activationViewPix;
    QPixmap activationSuffixPix;
    QPixmap disableViewPix;
    QPixmap disableSuffixPix;

    // Testo per la DATA DI SISTEMA
    QGraphicsTextItem* dateText;

    // Testo per Intestazione
    GLabel* intestazioneValue;
    void setIntestazione();

    // Tasti selezione

    GPush* pulsanteXCC ;
    GPush* pulsanteCV ;
    GPush* pulsanteAT ;

    bool   suffixEna; // Abiilitazione pulsanti per suffisso
    GPush* pulsanteId;
    GPush* pulsanteS;
    GPush* pulsanteRM;
    GPush* pulsanteRL;
    GPush* pulsanteTAN;

    // Icone da attivare
    QGraphicsPixmapItem* memePix;
    GLabel* memeName;
    QGraphicsPixmapItem* latPix;

    QString currViewName;
    ACR_VIEW currentView;
    ACR_BREAST currentBreast;
    ACR_SUFFIX currentSuffix;

    QColor studyColor;

    void updateProjection(void);
};


#endif // PAGEACR_H
