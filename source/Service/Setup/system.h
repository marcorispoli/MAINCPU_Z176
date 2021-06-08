// SYSTEM_SETUP  SYSTEM_SETUP
// system  system
// systemUI  systemUI

#ifndef SYSTEM_SETUP_H
#define SYSTEM_SETUP_H

#include <QWidget>
#include <QGraphicsScene>
#include "lib/numericpad.h"
// Include aggiuntivi __________

//______________________________

namespace Ui {
class systemUI;
}

class systemPage : public QWidget
{
    Q_OBJECT

public:
    explicit systemPage(int rotview, QWidget *parent = 0);
    ~systemPage();

    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject


#define _DB_SYS_HS   _DB_SERVICE1_INT
#define _DB_SYS_ARM  _DB_SERVICE2_INT
#define _DB_SYS_TRX  _DB_SERVICE3_INT

#define _DB_SYS_DATA_Y  _DB_SERVICE4_INT
#define _DB_SYS_DATA_M  _DB_SERVICE5_INT
#define _DB_SYS_DATA_D  _DB_SERVICE6_INT
#define _DB_SYS_DATA_h  _DB_SERVICE7_INT
#define _DB_SYS_DATA_m  _DB_SERVICE8_INT
#define _DB_SYS_DATA_s  _DB_SERVICE9_INT
#define _DB_SYS_EXIT    _DB_SERVICE10_INT
#define _DB_SYS_CLOCK_CHG    _DB_SERVICE11_INT

public slots:
    // Funzione associata a GWindw
    void changePage(int pg, int opt);
    void onExitButton(void);
    void valueChanged(int index,int opt);



    void onLeftStarter(void);

    void onLeftRotation(void);

    void onRightStarter(void);

    void onRightRotation(void);
    void onRightTilt(void);
    void onLeftTilt(void);
    void clockSlot(bool);
    void onYedit(void);
    void onMedit(void);
    void onDedit(void);
    void onhedit(void);
    void onmedit(void);
    void onsedit(void);

    void exitFunction(void);
private:
    Ui::systemUI *ui;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsProxyWidget *proxy;
    QWidget *parent;
    int rotview;

    // Timer per la disabilitazione a tempo del pulsante di ingresso
    int changePageTimer;
    int timerDisable;   // Disable a tempo pulsante next   
    void initServicePage(void);
    void initFactoryPage(void);
    void exitPage(void);


    QString getGantryStr(unsigned char val);
    QString getStarterStr(unsigned char val);
    QString getRotationStr(unsigned char val);
    QString getTiltStr(unsigned char val);
    QString getDetectorStr(unsigned char val);

    bool serviceMode;
    numericPad* pCalculator;
    QString dataField;
    bool clockChg;
};

#endif
