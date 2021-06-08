// CALIBPOWER  CALIBPOWER
// calibpower  calibpower
// calibPowerUI  calibPowerUI

#ifndef CALIBPOWER_H
#define CALIBPOWER_H

#include <QWidget>
#include <QGraphicsScene>
#include "lib/numericpad.h"

// Include aggiuntivi __________

//______________________________

namespace Ui {
class calibPowerUI;
}

class calibpower : public QWidget
{
    Q_OBJECT

public:
    explicit calibpower(int rotview, QWidget *parent = 0);
    ~calibpower();

    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject

public slots:
    // Funzione associata a GWindw
    void changePage(int pg,  int opt);
    void onExitButton(void);
    void valueChanged(int index,int opt);
    void onValueEdit(void);
    void calculatorSlot(bool state);
    void onStoreButton(void);

private:
    Ui::calibPowerUI *ui;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsProxyWidget *proxy;
    QWidget *parent;
    int rotview;

    numericPad* pCalculator;
    QString     calcData; // Stringa di riferimento per i calicol

    // Timer per la disabilitazione a tempo del pulsante di ingresso
    int changePageTimer;
    int timerDisable;   // Disable a tempo pulsante next   
    void initPage(void);
    void exitPage(void);

    int timer_update;

};

#endif
