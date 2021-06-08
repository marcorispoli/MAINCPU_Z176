#ifndef CALIBCOMPRESSORPOSITION_H
#define CALIBCOMPRESSORPOSITION_H


#include <QWidget>
#include <QGraphicsScene>
#include "lib/numericpad.h"

namespace Ui {
class calibCompressorPosition;
}

class calibCompressorPosition : public QWidget
{
    Q_OBJECT

public:
    explicit calibCompressorPosition(int rotview, QWidget *parent = 0);
    ~calibCompressorPosition();

    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject

public slots:
    // Funzione associata a GWindw
    void changePage(int pg,int opt);
    void onExitButton(void);
    void valueChanged(int index,int opt);
    void onStartButton(void);
    void onCalibButton(void);
    void onStoreButton(void);

    //void onCalibPadNextButton(void); // Pulsante sequenziale
    void onCalibPadCancelButton(void);

    void onCalibPadEditUp(void);
    void onCalibPadEditDwn(void);
    void calculatorSlot(bool state);    // Slot associato all'uscita del tool di calcolo
    void pcb215Notify(unsigned char id, unsigned char notifyCode, QByteArray buffer);
    void calibrate(void);
    void store(void);

    void warningBoxSignal(void);

private:
    Ui::calibCompressorPosition *ui;
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
    #define     CALIB_POS_UP_FIELD 1
    #define     CALIB_POS_DWN_FIELD 2

    int upperPosition;
    int rawUpperPosition; // Sampled Upper position

    int lowerPosition;
    int rawLowerPosition; // Sampled lower position

    int rawPosition; // Current raw position
    int calibratedPosition;

    // Valori calibrati
    bool calibrated;
    int  calibPosOfs;
    int  calibPosK;
};

#endif // CALIBCOMPRESSORPOSITION_H
