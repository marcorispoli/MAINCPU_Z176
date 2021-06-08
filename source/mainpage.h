#ifndef MAINPAGE_H
#define MAINPAGE_H


#include "application.h"

class MainPage : public GWindow
{
    Q_OBJECT

public:
    MainPage(bool ls, QString bg,QString bgs ,bool showLogo, int w,int h, qreal angolo,QPainterPath pn, int pgpn, QPainterPath pp, int pgpp, int pg);
    virtual ~MainPage();
    void childStatusPage(bool stat,int param); // Override funzione della classe base GWindow
                                                // Al cambio pagina riporta lo stato di attivazione

    void timerEvent(QTimerEvent* ev); // Override della classe QObject
    void mousePressEvent(QGraphicsSceneMouseEvent* event); // Override funzione della classe base GWindow

    enum{
        _MAIN_PANEL = 0,
        _ROT_PANEL,
        _TILT_PANEL,
        _PWROFF_PANEL
    };



public slots:
    void valueChanged(int,int); // Link esterno alla fonte dei contenuti dei campi valore
    void buttonActivationNotify(int id,bool status,int opt);
    void languageChanged(); // Link esterno alla fonte dei contenuti dei campi valore


private:
    QString getArmString(void);
    QString getTiltString(void);
    void setWindowUpdate(void);
    void activateRot(int angolo);
    void activateTilt(int angolo);
    void setBattPix(unsigned char val);
    void setRotGroupEnaView(void);
    void changePannello(int newpanel);

private:
    bool disableTimedButtons; //  Disabilitazione a tempo dei bottoni per evitare rimbalzi
    int timerDisableButton;
    void disableButtons(int t){
        if(disableTimedButtons) return;
        disableTimedButtons=true;
        timerDisableButton = startTimer(t);
    }


    QColor studyColor;   // Colore relativo allo studio in corso
    int timerId; // Usato per la gestione del timer della data
    int timerPowerOffButton;

    // Testo per Intestazione
    GLabel* intestazioneValue;

    // Testo per la DATA DI SISTEMA
    QGraphicsTextItem* dateText;

    // Label campo accessorio
    GLabel* accessoryLabel;
    GLabel* accessoryValue;

    // Testo per campo copmressione
    GLabel* compressorLabel;
    GLabel* compressorValue;

    // Testo per campo posizione
    GLabel* positionLabel;
    GLabel* spessoreLabel;
    GLabel* positionValue;
    GLabel* spessoreValue;

    // Testo per campo forza
    GLabel* forceLabel;
    GLabel* targetLabel;
    GLabel* forceValue;
    GLabel* targetValue;



    GLabel* angoloValue; // Angolo C-ARM

    GLabel* tiltValue;   // Angolo Tilt


    QGraphicsPixmapItem* panelTool;
    QGraphicsPixmapItem* logoPix;       // Pixmap per il logo



    // Pulsante apertura pagina allarmi

    int pannello; // TIpo di pannello comandi attivo

    // PULSANTI PANNELLO _MAIN_PANEL________________________
    GPush* pulsanteAlarmOn;
    GLabel* alarmNum;   // numero di allarmi presenti

    GPush* pulsanteToolsOn;
    GPush* pulsanteRotazioni;
    GPush* pulsanteTilt;
    GPush* pulsantePowerOff;
    GPush* pulsanteOperatingMode; // Per Analogico
    GPush* pulsanteAudioMute; // Per Analogico

    QGraphicsPixmapItem* acPresent;
    GLabel* acLabel;

    QGraphicsPixmapItem* battPix;
    QGraphicsPixmapItem* awsPresent;


    QGraphicsPixmapItem* rotDisabledPix;
    QGraphicsPixmapItem* tiltDisabledPix;
    QGraphicsPixmapItem* deadmanPix;
    QGraphicsPixmapItem* demoPix;
    QGraphicsPixmapItem* closedDoorPix;

    // PULSANTI PANNELLO _ROT_PANEL________________________
    int selRotAngolo;
    GPush* pulsanteCancRot;
    GPush* pulsanteOkRot;
    GPush* pulsanteRot0;
    GPush* pulsanteRot45;
    GPush* pulsanteRot90;
    GPush* pulsanteRot135;
    GPush* pulsanteRot_45;
    GPush* pulsanteRot_90;
    GPush* pulsanteRot_135;
    GPush* pulsanteRotP;

    // PANNELLO TILT _______________________________________
    int selTiltAngolo;

    // PANNELLO POWER OFF _______________________________________
    GPush* pulsanteCancPowerOff;
    GPush* pulsanteOkPowerOff;
    GLabel* powerOffLabel;


};

#endif // MAINPAGE_H
