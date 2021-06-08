#ifndef BIOPSYPAGE_H
#define BIOPSYPAGE_H

#include "application.h"

#include <QObject>

class BiopsyPage : public GWindow
{
    Q_OBJECT

public:
    BiopsyPage(bool ls, QString bg,QString bgs ,bool showLogo, int w,int h, qreal angolo,QPainterPath pn, int pgpn, QPainterPath pp, int pgpp, int pg);
    virtual ~BiopsyPage();
    void childStatusPage(bool stat,int param); // Override funzione della classe base GWindow
                                                // Al cambio pagina riporta lo stato di attivazione

    void timerEvent(QTimerEvent* ev); // Override della classe QObject
    void mousePressEvent(QGraphicsSceneMouseEvent* event); // Override funzione della classe base GWindow


    void setXrayOn(bool stat); // Attiva il simbolo raggi in corso
    void setCalibOn(bool stat); // Attivazione simboli modo Calibrazione

    /* DA VERIFICARE L'UTILIZZO
    void setBiopsyMode(void); // Impostazione modalit√  Biopsia
    void setBiopsyMoveMode(void);
    void setStandardMode(void); // Impostazione modalit√  Biopsia
    void changeMode(void);      // Uso interno
    */

    //void resetBiopMoveButtons(void);
    //void setEnableMovePage(bool state);
    //void mainInitBiopVar(void);


public slots:
    void InitBiopsyPage(void); // Funzione per entrare nella pagina di Biopsia
    void valueChanged(int,int); // Link esterno alla fonte dei contenuti dei campi valore
    void buttonActivationNotify(int id,bool status,int opt);
    void languageChanged(); // Link esterno alla fonte dei contenuti dei campi valore

    void setOpenStudy(void);
    void setCloseStudy(void);

private:
    bool disableTimedButtons; //  Disabilitazione a tempo dei bottoni per evitare rimbalzi
    int timerDisableButton;
    int timerId; // Usato per la gestione del timer della data

    void disableButtons(int t){
        if(disableTimedButtons) return;
        disableTimedButtons=true;
        timerDisableButton = startTimer(t);
    }

    bool enableBiopMoveButtons;  // Abilitazione pagina movimenti manuali
    bool localStudy;    // Definisce se lo studio √® locale o con PC
    QColor studyColor;   // Colore relativo allo studio in corso




    // Testo per Intestazione
    GLabel* intestazioneValue;
    void setIntestazione();

    // Testo per la DATA DI SISTEMA
    QGraphicsTextItem* dateText;


    // Testo per l'angolo di inclinazione braccio
    GLabel* armValue;
    void setArmAngolo(void);

    // Testo per l'angolo di inclinazione TRX
    GLabel* trxValue;
    void setTrxAngolo(void);

    // Pulsante apertura pagina immagine presente
    GPush* pulsanteImgOn;


    // Campo Compressione
    GLabel* targetValue;
    GLabel* compressioneValue;
    void setCompressione(void);


    // Campo Spessore
    GLabel* spessoreValue;
    void setSpessore(void);


    // Pulsante apertura pagina allarmi
    GPush* pulsanteAlarmOn;

    // Testo per campo XYZ
    GLabel* biopXValue;
    GLabel* biopYValue;
    GLabel* biopZValue;
    GLabel* biopMargineValue;
    GLabel* biopMaxZValue;
    GLabel* biopHolderValue;

    void setBiopXYZ(void);
    void updateHolder(void);
    void updateManualActivation(void);

    // Testo per campo accessorio biopsia
    //GLabel* biopAgoLabel;
    //GLabel* biopAgoValue;
    //void setBiopAgo(void);

   // bool warningBiopAdapterVisible;
   // QGraphicsPixmapItem* warningBiopAdapter_Pix;
   // void setWarningBiopAdapter(bool stat);


    // Pulsanti movimento manuale
    GPush* pulsanteBiopStepUp;
    GPush* pulsanteBiopStepDown;
    GPush* pulsanteBiopHome;
    QGraphicsPixmapItem* buttonsPix;

    // Labels
    /*
    GLabel* funcBiopMoveLabel;
    GLabel* margBiopMoveLabel;
    GLabel* zBiopMoveLabel;
    GLabel* margBiopMoveValue;
    GLabel* zBiopMoveValue;
    void setBiopMoveMargValue(void);
    void setBiopMoveZValue(void);
*/

    QGraphicsPixmapItem* xrayPix;
    QGraphicsPixmapItem* calibPix;
    QGraphicsPixmapItem* demoPix;
    QGraphicsPixmapItem* rotPix;

    bool xRayStat;
    bool isOpen;
};

#endif // BIOPSYPAGE_H
