// CALIB_FILTER  CALIB_FILTER
// calibfilter  calibfilter
// calibFilterUI  calibFilterUI

#ifndef CALIB_FILTER_H
#define CALIB_FILTER_H

#include <QWidget>
#include <QGraphicsScene>

// Include aggiuntivi __________

//______________________________

namespace Ui {
class calibFilterUI;
}

class calibfilter : public QWidget
{
    Q_OBJECT

public:
    explicit calibfilter(int rotview, QWidget *parent = 0);
    ~calibfilter();

    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject

public slots:
    // Funzione associata a GWindw
    void changePage(int pg,  int opt);
    void onExitButton(void);
    void valueChanged(int index,int opt);
    void guiNotifySlot(unsigned char,unsigned char,QByteArray);
    void onButton1(void);
    void onButton2(void);
    void onButton3(void);
    void onButton4(void);
    void onAdjustLeft(void);
    void onAdjustRight(void);
    void onStoreButton(void);
    void onTestButton(void);
    void onStopButton(void);




private:
    Ui::calibFilterUI *ui;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsProxyWidget *proxy;
    QWidget *parent;
    int rotview;

    // Timer per la disabilitazione a tempo del pulsante di ingresso
    int changePageTimer;
    int timerDisable;   // Disable a tempo pulsante next   
    void initPage(void);
    void exitPage(void);


    unsigned char posizioneIniziale[4];// Per ripristino valori di ingresso
    QString fname[4];           // Nome filtro associato alla posizione n
    int indiceCorrente;         // Posizione corrente


    // Esecuzione test ciclico
    QByteArray sequenza;
    int indice_sequenza;
    int timer_test;
    int errors;

};

#endif
