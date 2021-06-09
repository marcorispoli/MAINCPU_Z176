#ifndef ANALOGPAGEOPEN_H
#define ANALOGPAGEOPEN_H

#include "../application.h"
#include "pannelloComandi.h"
#include "pannelloCompressione.h"
#include "pannelloSpessore.h"
#include "pannelloKv.h"
#include "pannelloMas.h"
#include "pannelloOpzioni.h"
#include "pannelloProiezioni.h"
#include "pannelloColli.h"
#include "pannelloBiopsia.h"
#include <QWidget>
#include <QGraphicsScene>

namespace Ui {
class analogUI;
}


class AnalogPageOpen : public QWidget
{
    Q_OBJECT

public:
    explicit AnalogPageOpen(int rotview,QWidget *parent = 0);
    ~AnalogPageOpen();

    // Timer per gestire il pulsante
    void timerEvent(QTimerEvent* ev); // Override della classe QObject

signals:
    void queuedExecution(int code, int val, QString str); // Esecuzione nella coda dei messaggi

public slots:

    #define QUEUED_SELECTED_FILTER      1
    #define QUEUED_SELECTED_FUOCO       2
    #define QUEUED_UPDATE_COLLI         3
    #define QUEUED_START_XRAY_SEQ       4
    #define QUEUED_LOG                  5
    #define QUEUED_LOG_FLUSH            6
    #define QUEUED_INIT_PAGE            7



    bool openPageRequest(void);
    bool closePageRequest(void);
    void queuedExecutionSlot(int code, int val, QString str);
    // Per i moduli esterni alla classe corrente
    void emitQueuedExecution(int code, int val, QString str){
        emit queuedExecution(code, val,str);
    }

    // Funzione associata a GWindw
    void changePage(int pg, int opt);
    void onExitButton(void);
    void valueChanged(int index,int opt);
    void onSblocco_compressore(void);
    void onManualColliButt(void);
    void onAlarmButt(void);
    void guiNotify(unsigned char id, unsigned char mcccode, QByteArray data);


    void setHuAnode(int khu);
    void setTempCuffia(int temp);


private:
    int timerReady;
    int timerDisable;
    Ui::analogUI *ui;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsProxyWidget *proxy;

    void initPage(void);
    void exitPage(void);


    void rotView(int angolo);

    int timer_attesa_dati;
    bool data_ready;
    void startAttesaDati(int time);
    void stopAttesaDati(void);

    // Timer per la disabilitazione a tempo del pulsante di ingresso
    int changePageTimer;
    int  getArm(void);
    int getCurrentProjection(int angolo);
    void setKvRange(unsigned char opt);     // Impostazione del range di kV utilizzabile
    int  getMinKv(void) ;
    int  getMaxKv(void) ;


    int currentAngolo;

    pannelloComandi* commandPanel;
    pannelloCompressione* compressionPanel;
    pannelloSpessore* thicknessPanel;
    pannelloKv* kvPanel;
    pannelloMas* masPanel;
    pannelloOpzioni* optionPanel;
    pannelloProiezioni* projPanel;
    pannelloColli* colliPanel;
    pannelloBiopsia* biopsyPanel;

    // Vettore per agevolare la gestione delle proiezioni
    int currentProjection;
    unsigned char currentBreast;

    int projectionsAngoli[17];

    void changePanel(int panel);
    void manageCallbacks(int opt);
    void saveOptions(void);

    void  activateProjection(void) ;
    void  setCurrentFuoco(void);
    void  setPad(void);
    void  setSbloccoCompressore(void);
    void  verifyBiopsyReady(void);
    void  verifyStandardReady(void);

    void  updateAlarmField(void);
    void  setCurrentCollimation(void);

    void  startXraySequence(void);
    void  xrayManualSequence(void);
    void  xraySemiAutoSequence(void);
    void  xrayFullAutoSequence(void);
    void  xrayReleasePushButton(void);
    void  xrayErrorInCommand(unsigned char code);

    // Inizializzazione pagina relkativa (solo master esegue!)
    void  initializeBiopsyPage(void);
    void  initializeStandardPage(void);
    void  setInfoReadyFields(int flags);


    // Dati di esposizione
    QString Xprofile;
    int     XPlog;
    int     XRad;
    float   Xpre_selectedkV;   // kv utilizzate per il pre impulso
    int     Xpre_selectedDmAs; // mAs utilizzati per il pre impulso
    float   XselectedkV;
    int     XselectedDmAs;
    int     XselectedIa;
    int     XselectedFiltro;
    int     XselectedFuoco;
    int     XspessoreSeno;
    float   Xdose;
    float   XmAs;
    int     XThick;
    int     XForce;

    float   cumulativeXdose; // Dose accumulata nello studio in uG


};



#endif
