#ifndef CALIBMENU_H
#define CALIBMENU_H

#include <QWidget>
#include <QGraphicsScene>

namespace Ui {
class CalibMenuObject;
}

class CalibMenu : public QWidget
{
    Q_OBJECT

public:
    explicit CalibMenu(int rotview,QWidget *parent = 0);
    ~CalibMenu();

    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject

public slots:
    // Funzione associata a GWindw
    void changePage(int pg, int opt);
    void onExitButton(void);

     void onStarterCalibButton(void); // Attiva il menu setup
     void onHVCalibButton(void); // Attiva il menu setup
     void onFilterCalibButton(void); // Attiva il menu setup
     void onForceCalibButton(void); // Attiva il menu setup
     void onPositionCalibButton(void); // Attiva il menu setup
     void onTiltCalibButton(void); // Attiva il menu setup
     void onLenzePotCalibButton(void); // Calibrazione potenziometro lenze
     void onDetectorCalibButton(void);
     void onColliCalibButton(void); // Calibrazione collimazione
     void onConsoleButton(void);    // Calibrazione cursore console

private:
    Ui::CalibMenuObject *ui;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsProxyWidget *proxy;

    void initPage(void);
    void exitPage(void);

    // Timer per la disabilitazione a tempo del pulsante di ingresso
    int changePageTimer;

};

#endif // CALIBMENU_H
