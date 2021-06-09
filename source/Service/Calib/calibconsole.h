
#ifndef CALIBCONSOLE_H
#define CALIBCONSOLE_H

#include "../../application.h"
#include <QWidget>
#include <QGraphicsScene>

// Include aggiuntivi __________
#include "lib/numericpad.h"
//______________________________

namespace Ui {
class consoleUI;
}

class calibconsole: public QWidget
{
    Q_OBJECT

public:
    explicit calibconsole(int rotview, QWidget *parent = 0);
    ~calibconsole();

    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject

public slots:
    // Funzione associata a GWindw
    void changePage(int pg,  int opt);
    void onExitButton(void);
    void valueChanged(int index,int opt);
    void calculatorSlot(bool state);    // Slot associato all'uscita del tool di calcolo
    bool checkConfigChange(void);
    void refreshStatus(void);

    void onExitPageEvent(void);
    void showCalibFrame(void);
    void showConfigChangeFrame(void);
    void showMoveFrame(void);
    void showLoopFrame(void);

    void updateFase(int fase);
    void updateConsoleButtonShadow(void);
    void updateConsoleButtonFunction(void);

    void onConfirmExitButton(void);
    void onStoreButton(void);
    void onCancelButton(void);
    void onDxSetup(void);
    void onDySetup(void);
    void onXOffset(void);
    void onYOffset(void);
    void onZOffset(void);
    void onFOffset(void);
    void onLoopButton(void);

    void onXMove(void);
    void onYMove(void);
    void onZMove(void);
    void onHomeButton(void);

    void onXloop(void);
    void onYloop(void);
    void onZloop(void);

    void onOpenMoveButton(void);
    void onMoveXYZButton(void);
    void onCancelMoveButton(void);
    void onStartLoop(void);
    void onStopLoop(void);


private:
    Ui::consoleUI *ui;
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


    numericPad* pCalculator;
    QString     dataField; // Stringa di riferimento per i calcoli
    QGraphicsPixmapItem* buttonSelector;
    QGraphicsPixmapItem* readerSelector;

    int Xhome;
    int Yhome;
    int Xref;
    int Yref;

    bool loopState;
    int  loopTimer;
    int  loopNtime;

    // Dati di calibrazione in ingresso
    biopsyConf_Str config;


    bool Calibrate(void);
};

#endif
