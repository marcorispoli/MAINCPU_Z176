
#ifndef INVERTERTOOL_H
#define INVERTERTOOL_H

#include <QWidget>
#include <QGraphicsScene>

// Include aggiuntivi __________

//______________________________

namespace Ui {
class inverterUI;
}

class invertertool : public QWidget
{
    Q_OBJECT

public:
    explicit invertertool(int rotview, QWidget *parent = 0);
    ~invertertool();

    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject

public slots:
    // Funzione associata a GWindw
    void changePage(int pg, int opt);
    void onExitButton(void);
    void valueChanged(int index,int opt);
    void onHSButton(void);
    void onStopButton(void);
    void onLSButton(void);

    //void masterNotifySlot(unsigned char id,unsigned char mcc_code,QByteArray data);

private:
    Ui::inverterUI *ui;
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

    void mainPowerUpdate(void);

};

#endif
