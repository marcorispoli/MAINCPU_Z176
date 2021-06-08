// CALIBPOT  CALIBPOT
// calibpot  calibpot
// calibPotUI  calibPotUI

#ifndef CALIBPOT_H
#define CALIBPOT_H

#include <QWidget>
#include <QGraphicsScene>

// Include aggiuntivi __________
#include "lib/numericpad.h"
//______________________________

namespace Ui {
class calibPotUI;
}

class calibpot : public QWidget
{
    Q_OBJECT

public:
    explicit calibpot(int rotview, QWidget *parent = 0);
    ~calibpot();

    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject

public slots:
    // Funzione associata a GWindw
    void changePage(int pg,  int opt);
    void onExitButton(void);
    void valueChanged(int index,int opt);

    void onStartButton(void);
    void onPrevButton(void);
    void onNextButton(void);
    void onStoreButton(void);
    void onCalibPositionLow(void);
    void onCalibPositionHigh(void);

    void slaveNotifySlot(unsigned char id,unsigned char mcc_code,QByteArray data);

    void calculatorSlot(bool state);    // Slot associato all'uscita del tool di calcolo

private:
    Ui::calibPotUI *ui;
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

    #define CALIB_POT_LOW_THRESHOLD 1
    #define CALIB_POT_HIGH_THRESHOLD 2
    numericPad* pCalculator;
    QString     dataField; // Stringa di riferimento per i calcoli

};

#endif
