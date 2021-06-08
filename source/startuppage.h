#ifndef STARTUPPAGE_H
#define STARTUPPAGE_H

#include <QWidget>
#include <QGraphicsScene>

namespace Ui {
class StartupPage;
}

class StartupPage : public QWidget
{
    Q_OBJECT

public:
    explicit StartupPage(int rotview,QWidget *parent = 0);
    ~StartupPage();

    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject

public slots:

    void changePage(int pg, int opt);       // Handler messaggi di cambio pagina da Grafica
    void valueChanged(int index,int opt);   // Handler messaggi da database
    void startupNotifySlot(unsigned char id, unsigned char mcccode, QByteArray buffer); // Handler messaggi di configurazione da M4

    void onExitButton(void);


private:
    Ui::StartupPage *ui;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsProxyWidget *proxy;

    // Timer per la disabilitazione a tempo del pulsante di ingresso
    int changePageTimer;
    void initWindow(void);

    int syncToSlave;
    bool hardwareConfigured;
    bool deviceConnected;
    bool deviceConfigured;

};

#endif // STARTUPPAGE_H
