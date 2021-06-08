#ifndef PROJECTIONPAGE_H
#define PROJECTIONPAGE_H


#include "application.h"


class ProjectionPage : public GWindow
{
    Q_OBJECT

public:
    ProjectionPage(bool ls, QString bg,QString bgs ,bool showLogo, int w,int h, qreal angolo,QPainterPath pn, int pgpn, QPainterPath pp, int pgpp, int pg);
    virtual ~ProjectionPage();
    void childStatusPage(bool stat,int param); // Override funzione della classe base GWindow
                                                // Al cambio pagina riporta lo stato di attivazione

    void timerEvent(QTimerEvent* ev); // Override della classe QObject
    void mousePressEvent(QGraphicsSceneMouseEvent* event); // Override funzione della classe base GWindow
    void nextPageHandler(void);
    void prevPageHandler(void);

    QString getPixFile(QString name);
    int getListSize() {return proiezioni.size();}
    QList<QString> getList(QString stringa);
    void setProiezioni(void);

public slots:
    void valueChanged(int,int); // Link esterno alla fonte dei contenuti dei campi valore
    void buttonActivationNotify(int id,bool status,int opt);
    void languageChanged(); // Link esterno alla fonte dei contenuti dei campi valore

    void onOkSelection();
    void onCancSelection();


private:
    bool disableTimedButtons; //  Disabilitazione a tempo dei bottoni per evitare rimbalzi
    int timerDisableButton;
    void disableButtons(int t){
        if(disableTimedButtons) return;
        disableTimedButtons=true;
        timerDisableButton = startTimer(t);
    }

    void initWindow(void);

    QColor studyColor;   // Colore relativo allo studio in corso
    int timerId; // Usato per la gestione del timer della data

    // Testo per Intestazione
    GLabel* intestazioneValue;
    void setIntestazione();

    // Testo per la DATA DI SISTEMA
    QGraphicsTextItem* dateText;

    // Lista proiezioni disponibili
    QList<QString> proiezioni;

    GPush* Meme[8];

    // Testo per campo spessore
    GLabel* MemeLabel[8];


};

#endif // PROJECTIONPAGE_H
