#ifndef CALIBCOMPRESSORFORCE_H
#define CALIBCOMPRESSORFORCE_H

#include <QWidget>
#include <QGraphicsScene>
#include "lib/numericpad.h"

namespace Ui {
class CalibCompressorForce;
}

class CalibCompressorForce : public QWidget
{
    Q_OBJECT

public:
    explicit CalibCompressorForce(int rotview, QWidget *parent = 0);
    ~CalibCompressorForce();

    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject

public slots:
    // Funzione associata a GWindw
    void changePage(int pg,  int opt);
    void onExitButton(void);
    void valueChanged(int index,int opt);


    void onStartButton(void);
    void onCalibButton(void);
    void onStoreButton(void);


    void onCalibForceCancelButton(void);

    void onCalibComprEditF1(void);
    void onCalibComprEditF2(void);
    void calculatorSlot(bool state);    // Slot associato all'uscita del tool di calcolo
    void pcb215Notify(unsigned char id, unsigned char notifyCode, QByteArray buffer);
    void calibrate(void);
    void store(void);

    void warningBoxSignal(void);

private:
    Ui::CalibCompressorForce *ui;
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

    void hideAll(void);

    numericPad* pCalculator;
    QString     dataField; // Stringa di riferimento per i calcoli
    #define     CALIB_FORCE_OFFSET  1
    #define     CALIB_FORCE_F1      2
    #define     CALIB_FORCE_F2      3

    int rawOffset;
    int f1Force;
    int rawF1Force; // Sampled lower position
    int f2Force;
    int rawF2Force; // Sampled lower position

    int rawForce;
    int calibratedForce;

    // Valori calibrati
    bool calibrated;
    int F0 ;
    int KF0;
    int F1;
    int KF1;

};

#endif // CALIBCOMPRESSORFORCE_H
