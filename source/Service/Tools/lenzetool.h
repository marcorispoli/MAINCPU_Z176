
#ifndef LENZETOOL_H
#define LENZETOOL_H

#include <QWidget>
#include <QGraphicsScene>

// Include aggiuntivi __________

//______________________________

namespace Ui {
class lenzeToolUI;
}

class lenzetool : public QWidget
{
    Q_OBJECT

public:
    explicit lenzetool(int rotview, QWidget *parent = 0);
    ~lenzetool();

    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject

public slots:
    // Funzione associata a GWindw
    void changePage(int pg, int opt);
    void onExitButton(void);
    void valueChanged(int index,int opt);
    void slaveNotifySlot(unsigned char id,unsigned char mcc_code,QByteArray data);

private:
    Ui::lenzeToolUI *ui;
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
};

#endif
