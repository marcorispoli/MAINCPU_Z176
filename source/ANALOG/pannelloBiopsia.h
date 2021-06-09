#ifndef PANNELLOBIOPSIA_H
#define PANNELLOBIOPSIA_H

#include "../application.h"

#include <QWidget>
#include <QGraphicsScene>


class pannelloBiopsia : public QGraphicsScene
{
    Q_OBJECT

public:
    pannelloBiopsia(QGraphicsView* view);
    ~pannelloBiopsia();
    bool isOpen() {return open_flag;}

public slots:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void timerEvent(QTimerEvent* ev);
    void valueChanged(int index,int opt);

    void open();
    void exit();
    void xrayPixActivation(bool stat);

private:
    int timerStatus;
    int timerDisable;
    QGraphicsView* parent;
    bool open_flag;
    QGraphicsPixmapItem* backgroundPix;

    QGraphicsPixmapItem* xrayPix;

    // Messaggio di stato
    GLabel* statusMsgLabel;
    void setStatusMessage(void);

    // Impostazione kV e mAs
    void setKv(int val);
    void setdmAs(int dmAs);
    GLabel* kvLabel; // Impostazione kV
    GLabel* masLabel;// Impostazione mAs

    // Ready field
    void setReady(bool ready);
    QGraphicsPixmapItem* readyNotReadyPix;

    // Visualizzazione posizione Tubo
    void setTrxPicture(void);
    QGraphicsPixmapItem* trxPix;
    void setTrx(int angolo);

    // Visualizzazione pulsante rotazioni
    void setEnableRot(void);
    QGraphicsPixmapItem* armEnabledPix;
    GLabel* angoloArm;


    // Stringe Informative lato lesione
    void setLesionPosition(void);
    GLabel* lesionlabel;
    GLabel* lesionPosition;

    GLabel* holderLabel;

    // Finestra di attivazione movimento Torretta
    GLabel* targetPosition;

    // Finestra di check position e aggiustamento
    GLabel* zPosition; // Posizione corrente Z
    GLabel* margPosition; // Margini disponibili
    void setCurrentNeedlePosition(void);

    // Pulsanti vari
    #define _BIOPSY_PUSH_SELECT_NEEDLE 1

    // Stringe Informative lato Ago
    void setNeedleLength(void);
    GLabel* needleLengthLabel;
    GLabel* needleMinLengthLabel;
    GLabel* needleMaxLengthLabel;
    GLabel* needleLength;

    // Label per il pannello di selezione della lunghezza ago
    GLabel* needleSelection;
    int     last_increment;

    // Pannello di errore calcolo
    GLabel* wrongCalcLabel;

    // Dosimetria
    GLabel* doseLabel;
    GLabel* mAsXLabel;
    GLabel* kvXLabel;

public:
    //_______________ GESTIONE WORKFLOW ___________________
    void changeBiopsyWorkflowStatus(unsigned char workflow, bool force);
    #define _BIOPSY_INIT                 0
    #define _BIOPSY_SHOT_RIGHT           1
    #define _BIOPSY_SHOT_LEFT            2
    #define _BIOPSY_POINT_LESION         3
    #define _BIOPSY_WAIT_REFERENCE_P15   4
    #define _BIOPSY_WAIT_LESION_P15      5
    #define _BIOPSY_WAIT_REFERENCE_M15   6
    #define _BIOPSY_WAIT_LESION_M15      7
    #define _BIOPSY_WRONG_LESION_CALC    8

    #define _BIOPSY_SELECTION_NEEDLE     9
    #define _BIOPSY_MOVE_BYM             10
    #define _BIOPSY_MOVING               11

    #define _BIOPSY_CHECK_POSITION       12

    #define _BIOPSY_NEEDLE_LENGTH        20
    #define _BIOPSY_NO_STATUS            255


    unsigned char workflow; // Indice di stato del workflow


    // Imposta un'azione da eseguire all'uscita da un messaggio di allarme
    void manageAfterAlarmActions(void);
    unsigned char actionAfterAlarm;
    #define BIOPSY_AFTER_ALARM_NO_ACTION        0
    #define BIOPSY_AFTER_ALARM_RESET_SEQUENCE   1

    void manageConsoleButtons(int buttons);

    // IMposta l'angolo nel riquadro relativo
    void setArm(int angolo);
};


#endif
