
#ifndef CALIBPARKING_H
#define CALIBPARKING_H

#include <QWidget>
#include <QGraphicsScene>



namespace Ui {
class calibParkUI;
}

class calibParking : public QWidget
{
    Q_OBJECT

public:
    explicit calibParking(int rotview, QWidget *parent = 0);
    ~calibParking();

    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject

public slots:
    // Funzione associata a GWindw
    void changePage(int pg,  int opt);
    void onExitButton(void);
    void valueChanged(int index,int opt);

    void onActivateButton(void);
    void onStoreButton(void);
    void onArm0Button(void);
    void onArm180Button(void);
    void slaveNotifySlot(unsigned char id,unsigned char mcc_code,QByteArray data);

private:
    Ui::calibParkUI *ui;
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

    int potenziometro;
    int timerCommand;
    int timerStoring;
    void requestLenzePot(void);
    void requestStartCalib(void);
    void requestStopCalib(void);
    void activateRot(int angolo);

};

#endif
