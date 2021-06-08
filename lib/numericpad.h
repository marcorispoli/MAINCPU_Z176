#ifndef NUMERICPAD_H
#define NUMERICPAD_H

#include <QWidget>
#include <QGraphicsScene>
#include "lib/insertcalc.h"

namespace Ui {
class numericPad;
}

class numericPad : public QWidget
{
    Q_OBJECT
    
public:
    explicit numericPad(int rotview, QGraphicsView *view, QWidget *parent);
    ~numericPad();
    
    void  activate(QString* field, QString name);
    void  activate(QLabel* field, QString name);
    void  activate(QLineEdit* field, QString name);
    void  activate(QString* field, QString name,int code);
    void setCripto(void);

signals:
    void calcSgn(bool state);

public slots:

    void  on0Pressed();
    void  on1Pressed();
    void  on2Pressed();
    void  on3Pressed();
    void  on4Pressed();
    void  on5Pressed();
    void  on6Pressed();
    void  on7Pressed();
    void  on8Pressed();
    void  on9Pressed();
    void  onDotPressed();
    void  onSgnPressed();
    void  onOKPressed();
    void  onCancelPressed();
    void  onBackPressed();
    void  onClsPressed();
    void  onAPressed();
    void  onBPressed();
    void  onCPressed();
    void  onDPressed();
    void  onEPressed();
    void  onFPressed();
    void  onOXPressed();

    void timerEvent(QTimerEvent* ev);
    void hide();

private:
    Ui::numericPad *ui;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsProxyWidget *proxy;
    QWidget *parent;

    QGraphicsView *parentView;

    // Utilizzato per disabilitare i pulsanti all'ingresso ed evitare
    // fastidiose false attivazioni
    int disableTimer;
    bool disableButton;
    bool hexMode;
    bool criptato;

public:
            QString*    risultato;
            QLabel*     rislabel;
            QLineEdit*  rislineedit;
            int activation_code;        // Codice di riferimento per operazione in corso
            bool validazione;
            QString valore;
            QString criptoText;

};

#endif // NUMERICPAD_H
