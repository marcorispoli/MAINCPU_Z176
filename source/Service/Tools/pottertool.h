
#ifndef POTTERTOOL_H
#define POTTERTOOL_H

#include <QWidget>
#include <QGraphicsScene>

// Include aggiuntivi __________
#include "lib/numericpad.h"
//______________________________

namespace Ui {
class potterToolUI;
}

class pottertool : public QWidget
{
    Q_OBJECT

public:
    explicit pottertool(int rotview, QWidget *parent = 0);
    ~pottertool();

    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject

public slots:
    // Funzione associata a GWindw
    void changePage(int pg, int opt);
    void onExitButton(void);
    void valueChanged(int index,int opt);

    void onGridCycles(void);
    void calculatorSlot(bool state);
    void onStartGrid(void);

private:
    Ui::potterToolUI *ui;
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


    int timerRequest;
    QString dataField;
    numericPad* pCalculator;
};

#endif
