#ifndef SERVICEPANELMENU_H
#define SERVICEPANELMENU_H

#include <QWidget>
#include <QGraphicsScene>
#include "lib/numericpad.h"

namespace Ui {
class ServicePanelMenu;
}

class ServicePanelMenu : public QWidget
{
    Q_OBJECT
    
public:
    explicit ServicePanelMenu(int rotview,QWidget *parent = 0);
    ~ServicePanelMenu();
    
    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject


    QWidget *parentWidget;
public slots:
    // Funzione associata a GWindw
    void changePage(int pg, int opt);
    void valueChanged(int,int); // Link esterno alla fonte dei contenuti dei campi valore

    void onExitButton(void);

    void onToolsMenuButton(void);
    void onCalibMenuButton(void);
    void onPackagePanelButton(void);    
    void passwordSlot(bool result);

private:
    Ui::ServicePanelMenu *ui;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsProxyWidget *proxy;

    // Timer per la disabilitazione a tempo del pulsante di ingresso
    int changePageTimer;

    // utilizzato per la passsword
    #define PASSWORD    1
    numericPad* pCalculator;
    QString     calcData;
    void initPage(void);


};

#endif // SERVICEPANELMENU_H
