#ifndef MSGBOX_H
#define MSGBOX_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QGraphicsView>
#include <QGraphicsScene>
#include  <QThread>

class sleepThread : public QThread
{
public:
static void msleep(unsigned long delay)
    {
        QThread::msleep(delay);
    }
};

namespace Ui {
class msgBox;
}

class msgBox : public QWidget
{
    Q_OBJECT
    
public:
    explicit msgBox(int rotation, QWidget *parent = 0);
    ~msgBox();

    enum{

 _BUTTON_OK=      0x1,
 _BUTTON_CANC=    0x2,
 _BUTTON_FUNC=    0x4,
 _PROGRESS_BAR=   0x8,
 _EDITABLE=       0x10,
 _COLOR_DICOM=    0x20,
 _COLOR_LOCAL=    0x40,
 _COLOR_DEFAULT=  0x80,
 _WARNING_TYPE=   0x100,
 _ERROR_TYPE=     0x200
};

   void  activate(QString title, QString message, int flags=_BUTTON_OK);
   void  activate(QString title, QString message, int height, int flags);

   void  setFuncLabel(QString lab); // Imposta il nome del bottone funzione
   void  setOKLabel(QString lab); // Imposta il nome del bottone funzione
   void  setCANCLabel(QString lab); // Imposta il nome del bottone funzione
   void  setTitleLabel(QString title); // Imposta il titolo
   void  setEditable(QString def, QString title); // Imposta il contenuto del campo editable
   QString getEditable();
   void  setTextLabel(QString title); // Imposta il messaggio
   void  setTextAlignment(Qt::Alignment align);
   void  keepAlive() {terminate = false;} // Funzione da lanciare nella callback dei pulsanti per non far chiudere la finestra

   void  setSuffix(QString def);

signals:
    void buttonOkSgn(void);
    void buttonCancSgn(void);
    void buttonFuncSgn(void);
    void editableFuncSgn(void); // Funzione lanciata al termine della digitazione

public slots:
     void  onOkButtonPressed();   // OK button
     void  onCancButtonPressed(); // CANC button
     void  onFuncButtonPressed(); // FUNC button
     void  onEditableSelection(void);
     void  onEditableSelectionNotify(bool result);
     void  externalHide(void); // Forza chiusura interfaccia da esterno senza segnalazione

     void timerEvent(QTimerEvent* ev); // Override della classe QObject

public:
     void setProgressBar(int min, int max, int val); // Imposta i valori di lavoro
     void setProgressBar(QString style); // IMposta lo staylesheet
     void setTimeout(long timeout);

     QString userData; // Stringa utilizzabile per trasferire informazioni alle funzioni dei pulsanti

private:
     Ui::msgBox *ui;

    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsProxyWidget *proxy;

    unsigned int flags;

    int hText; // Altezza area testo
    int timerTimeout;
    int tic100msTimeout;
    int maxTimeout;
    bool terminate;

    QString editableTitle; // Titolo da associare alla finestra di inserimento dati nella editable
    QString messaggio;      // Contenuto dati

    void setWindow(int num); // Imposta la dimensione della finestra in funzione degli oggetti da visualizzare
    int rotation;
};

#endif // MSGBOX_H
