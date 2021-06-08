#ifndef CALIBZEROSETTING_H
#define CALIBZEROSETTING_H

#include <QWidget>
#include <QGraphicsScene>
#include "lib/numericpad.h"


namespace Ui {
class calibzerosetting;
}

class calibzerosetting : public QWidget
{
    Q_OBJECT

public:
    explicit calibzerosetting(int rotview, QWidget *parent = 0);
    ~calibzerosetting();

    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject

public slots:
    // Funzione associata a GWindw
    void changePage(int pg, int opt);
    void onExitButton(void);
    void valueChanged(int index,int opt);

    //_____________________________________
    void onManualModeButton(void);
    void onTrxZero(void);

    void onGonioZero(void);
    void guiNotify(unsigned char id,unsigned char cmd, QByteArray buffer);



private:
    Ui::calibzerosetting *ui;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsProxyWidget *proxy;
    QWidget *parent;
    int rotview;

    // Timer per la disabilitazione a tempo del pulsante di ingresso
    int changePageTimer;
    int timerDisable;   // Disable a tempo pulsante next

    int seqindex;
    void initPage(void);
    void exitPage(void);

    //___________________________
    void updateManualMode(void);
    void activateTrxZeroSetting(void);

    void activateGonioZeroSetting(void);

    unsigned char rotButtonsMode;


};

#endif // CALIBZEROSETTING_H
