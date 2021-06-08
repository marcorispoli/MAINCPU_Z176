#ifndef TOOLSMENU_H
#define TOOLSMENU_H

#include <QWidget>
#include <QGraphicsScene>

namespace Ui {
class toolsMenuUI;
}

class toolsmenu : public QWidget
{
    Q_OBJECT
    
public:
    explicit toolsmenu(int rotview,QWidget *parent = 0);
    ~toolsmenu();
    
    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject

public slots:
    // Funzione associata a GWindw
    void changePage(int pg, int opt);
    void onExitButton(void);

    void onTiltingToolButton(void);
    void onArmToolButton(void);
    void onLenzeDriverButton(void);
    void onInverterButton(void);
    void onAudioSetupButton(void);
    void onManualAnalogXrayButton(void);
    void onPotterButton(void);


private:
    Ui::toolsMenuUI *ui;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsProxyWidget *proxy;

    void initPage(void);
    void exitPage(void);

    // Timer per la disabilitazione a tempo del pulsante di ingresso
    int changePageTimer;


};

#endif // SERVICEPANELMENU_H
