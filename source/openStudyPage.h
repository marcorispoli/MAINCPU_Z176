#ifndef OPENSTUDYPAGE_H
#define OPENSTUDYPAGE_H


#include "application.h"



class OpenStudyPage : public GWindow
{
    Q_OBJECT

public:
    OpenStudyPage(bool ls, QString bg,QString bgs ,bool showLogo, int w,int h, qreal angolo,QPainterPath pn, int pgpn, QPainterPath pp, int pgpp, int pg);
    virtual ~OpenStudyPage();
    void childStatusPage(bool stat,int param); // Override funzione della classe base GWindow
                                                // Al cambio pagina riporta lo stato di attivazione

    void timerEvent(QTimerEvent* ev); // Override della classe QObject
    void mousePressEvent(QGraphicsSceneMouseEvent* event); // Override funzione della classe base GWindow


public slots:
    void valueChanged(int,int); // Link esterno alla fonte dei contenuti dei campi valore
    void buttonActivationNotify(int id,bool status,int opt);
    void languageChanged(); // Link esterno alla fonte dei contenuti dei campi valore


private:
    void openStudyEvent(void);
    void setProiezione(QString name);
    void setReady(bool stat);
    void setXrayOn(bool stat); // Attiva il simbolo raggi in corso


public:
    void setOpenStudy(void);
    void setCloseStudy(void);

    bool disableTimedButtons; //  Disabilitazione a tempo dei bottoni per evitare rimbalzi
    int timerDisableButton;
    void disableButtons(int t){
        if(disableTimedButtons) return;
        disableTimedButtons=true;
        timerDisableButton = startTimer(t);
    }

    QColor studyColor;   // Colore relativo allo studio in corso


    int timerId; // Usato per la gestione del timer della data
    int timerWDG; // Usato per la gestione del timer della data

    // Testo per campo copmressione
    GLabel* cuffiaTLabel;
    GLabel* HuAnodeLabel;
    GPush* pulsanteTcuffia;
    unsigned char cuffiaViewMode; // 0 = HU%, 1=HU, 2 =Â°C
    void setTempCuffia(int temp);
    void setHuAnode(int khu);

    // Testo per la DATA DI SISTEMA
    QGraphicsTextItem* dateText;

    // Testo per campo spessore
    GLabel* spessoreLabel;

    // Testo per campo copmressione
    GLabel* compressioneLabel;


    // Testo per campo spessore
    GLabel* colliLabel;

    // Testo per campo copmressione
    GLabel* ingrLabel;


    // Testo per l'angolo di inclinazione
    //GLabel* angoloValue;
    //void setAngolo(int val);


    // Testo per Intestazione
    GLabel* intestazioneValue;
    void setIntestazione();

    // Campo Collimazione
    GLabel* collimazioneValue;
    void setCollimazione(QString str);

    // Campo Collimazione
    GLabel* ingrandimentoValue;
    void setIngrandimento(QString str);

    // Campo Compressione
    GLabel* compressioneValue;
    GLabel* targetValue;
    void setCompressione(void);


    // Campo Spessore
    GLabel* spessoreValue;
    void setSpessore(void);

    // Testo per l'angolo di inclinazione
    GLabel* angoloValue;
    void setAngolo(int val);

    // Pulsante sblocco compressore disabled
    GPush* pulsanteSbloccoDis;

    // Pulsante apertura pagina allarmi
    GPush* pulsanteAlarmOn;
    GLabel* alarmNum;   // numero di allarmi presenti


    // Pulsante apertura pagina immagine presente
    GPush* pulsanteImgOn;



    QGraphicsPixmapItem* unlockCompressorPix;
    QGraphicsPixmapItem* calibPix;


    QGraphicsPixmapItem* readyPix;
    GLabel* readyValue;
    QGraphicsPixmapItem* selProiezioniPix;
    QGraphicsPixmapItem* xRay_Pix;
    GPush* pulsanteSelezioneProiezioni;
    GPush* pulsanteAbortProiezioni;

    // Pulsante di attivazione ACR
    GPush* pulsanteAcr;
    QPixmap acrPix;


    QGraphicsPixmapItem* selectedProjPix;
    GLabel* selectedProjVal;

    bool xRayStat;


    QString selectedProjection;

private:
    bool isOpen; // Indica che la pagina è già stata aperta

};

#endif // OPENSTUDYPAGE_H
