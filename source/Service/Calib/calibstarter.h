// CALIBSTARTER  CALIBSTARTER
// calibstarter  calibstarter
// calibStarterUI  calibStarterUI

#ifndef CALIBSTARTER_H
#define CALIBSTARTER_H

#include <QWidget>
#include <QGraphicsScene>

// Include aggiuntivi __________

//______________________________

namespace Ui {
class calibStarterUI;
}

class calibstarter : public QWidget
{
    Q_OBJECT

public:
    explicit calibstarter(int rotview, QWidget *parent = 0);
    ~calibstarter();

    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject

public slots:
    // Funzione associata a GWindw
    void changePage(int pg, int opt);
    void onExitButton(void);
    void valueChanged(int index,int opt);

    void onStartButton(void);
    void onStoreButton(void);
    void activateTest(void);
    void storeData(void);
    void onServiceNotify(unsigned char id, unsigned char mcccode, QByteArray buffer);
    void onAfterExposure(void);
    void onBrakes(void);
    void noStoreSlot(void);
    void storeSlot(void);


private:
    Ui::calibStarterUI *ui;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsProxyWidget *proxy;
    QWidget *parent;
    int rotview;

    // Timer per la disabilitazione a tempo del pulsante di ingresso
    int changePageTimer;
    int timerDisable;   // Disable a tempo pulsante next   
    int timerStored;   // Disable a tempo pulsante next

    void initPage(void);
    void exitPage(void);

    unsigned char main_run;
    unsigned char shift_run;
    unsigned char main_keep;
    unsigned char shift_keep;
    unsigned char main_off;
    unsigned char shift_off;

    unsigned char cal_max_main_off;
    unsigned char cal_max_shift_off;
    unsigned char cal_max_main_run;
    unsigned char cal_max_shift_run;
    unsigned char cal_max_main_keep;
    unsigned char cal_max_shift_keep;
    unsigned char cal_min_main_run;
    unsigned char cal_min_shift_run;
    unsigned char cal_min_main_keep;
    unsigned char cal_min_shift_keep ;


};

#endif
