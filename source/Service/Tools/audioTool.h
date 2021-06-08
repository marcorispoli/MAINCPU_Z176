
#ifndef AUDIOTOOL_H
#define AUDIOTOOL_H

#include <QWidget>
#include <QGraphicsScene>

// Include aggiuntivi __________
#include "lib/numericpad.h"
//______________________________

namespace Ui {
class audioToolUI;
}

class audiotool : public QWidget
{
    Q_OBJECT

public:
    explicit audiotool(int rotview, QWidget *parent = 0);
    ~audiotool();

    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject

public slots:
    // Funzione associata a GWindw
    void changePage(int pg, int opt);
    void onExitButton(void);
    void valueChanged(int index,int opt);

    void onVolMButton(void);
    void onVolPButton(void);
    void onEnabButton(void);
    void onTestButton(void);

    void onMsgNum(void);
    void calculatorSlot(bool state);

private:
    Ui::audioToolUI *ui;
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

    bool configChanged;
    QString dataField;
    numericPad* pCalculator;
};

#endif
