
#ifndef ARMTOOL_H
#define ARMTOOL_H

#include <QWidget>
#include <QGraphicsScene>

// Include aggiuntivi __________

//______________________________

namespace Ui {
class armToolUI;
}

class armtool : public QWidget
{
    Q_OBJECT

public:
    explicit armtool(int rotview, QWidget *parent = 0);
    ~armtool();

    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject

public slots:
    // Funzione associata a GWindw
    void changePage(int pg, int opt);
    void onExitButton(void);
    void valueChanged(int index,int opt);
    void slaveNotifySlot(unsigned char id,unsigned char mcc_code,QByteArray data);

private:
    Ui::armToolUI *ui;
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

    QString getNanotecErrorCodeStr(unsigned long code);
    QString getNanotecErrorNumberStr(unsigned long code);
    int timerRequest;
};

#endif
