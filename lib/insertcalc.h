#ifndef INSERTCALC_H
#define INSERTCALC_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QGraphicsView>
#include <QGraphicsScene>

namespace Ui {
class InsertCalc;
}

class InsertCalc : public QWidget
{
    Q_OBJECT
    
public:
    explicit InsertCalc(QWidget *parent = 0);
    ~InsertCalc();

signals:
    void calcSgn(bool state);


public slots:
    void  activate(QString* field, QString name);
    void  activate(QLabel* field, QString name);
    void  activate(QLineEdit* field, QString name);
    void  activate(QString* field, QString name,int code);

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

    void timerEvent(QTimerEvent* ev);

    void hide();

private:    


    // Utilizzato per disabilitare i pulsanti all'ingresso ed evitare
    // fastidiose false attivazioni
    int disableTimer;
    bool disableButton;

public:
            QString*    risultato;
            QLabel*     rislabel;
            QLineEdit*  rislineedit;
            int activation_code;        // Codice di riferimento per operazione in corso
            bool validazione;
            QString valore;
};

#endif // INSERTCALC_H
