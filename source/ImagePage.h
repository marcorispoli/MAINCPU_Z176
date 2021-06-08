#ifndef IMAGEPAGE_H
#define IMAGEPAGE_H


#include "application.h"



class ImagePage : public GWindow
{
    Q_OBJECT

public:
    ImagePage(QString bg, int w,int h, qreal angolo,QPainterPath pn, int pgpn, QPainterPath pp, int pgpp, int pg);
    virtual ~ImagePage();
    void childStatusPage(bool stat,int param); // Override funzione della classe base GWindow
                                                // Al cambio pagina riporta lo stato di attivazione

    void timerEvent(QTimerEvent* ev); // Override della classe QObject
    void mousePressEvent(QGraphicsSceneMouseEvent* event); // Override funzione della classe base GWindow
    // void nextPageHandler(void);
    void prevPageHandler(void);



signals:


public slots:
    void valueChanged(int,int); // Link esterno alla fonte dei contenuti dei campi valore
    void buttonActivationNotify(int id,bool status,int opt);


public:
    void OpenPage();
    void ClosePage(void);
    void showPage(void);
    void eraseImage(void);
    bool isImageActive(void){  if(imageName!=""){ return true;} else {return false;} }
    static bool existImage(QString filename);

    bool activateTimer; // Attiva il timer di auto chiusura pagina

private:

    bool disableTimedButtons; //  Disabilitazione a tempo dei bottoni per evitare rimbalzi
    int timerDisableButton;
    void disableButtons(int t){
        if(disableTimedButtons) return;
        disableTimedButtons=true;
        timerDisableButton = startTimer(t);
    }

    QGraphicsPixmapItem* Pix;
    QGraphicsPixmapItem* ExitPix;

    int timerId; // Usato per la gestione del timer della data
    int timerOn; // Usato per la gestione del timer della data

    QString imageName;

    int findTimer;
    int findAttempt;

};

#endif // IMAGEPAGE_H
