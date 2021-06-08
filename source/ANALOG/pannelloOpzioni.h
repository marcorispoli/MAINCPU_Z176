#ifndef PANNELLOOPZIONI_H
#define PANNELLOOPZIONI_H

#include "../application.h"

#include <QWidget>
#include <QGraphicsScene>


class pannelloOpzioni : public QGraphicsScene
{
    Q_OBJECT

public:
    pannelloOpzioni(QGraphicsView* view);
    ~pannelloOpzioni();
    bool isOpen() {return open_flag;}


    bool analog_conf_changed; // Parametri della configurazione generale cambiati
    bool profile_conf_changed;// Parametri del profilo cambiati

    void setExpositionRange(int options);

public slots:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void timerEvent(QTimerEvent* ev);
    void valueChanged(int index,int opt);

    void open();
    void exit();


private:
    int timerDisable;
    QGraphicsView* parent;
    bool open_flag;


    QGraphicsPixmapItem* backgroundPix;

    QGraphicsPixmapItem* disableOdPix;
    QGraphicsPixmapItem* disableTechPix;
    QGraphicsPixmapItem* disableFilmPix;

    QGraphicsPixmapItem* disableFiltroAutoPix;
    QGraphicsPixmapItem* disableFiltroMoPix;
    QGraphicsPixmapItem* disableFiltroRhPix;

    QGraphicsPixmapItem* selButtonTechModePix;
    QGraphicsPixmapItem* selButtonFilterModePix;
    QGraphicsPixmapItem* selButtonODPix;
    QGraphicsPixmapItem* selButtonTechPix;

    GLabel* profileLabel;
    void setProfileLabel(QString name);

    GLabel* indexLabel;
    void setProfileIndex(int index);


    QGraphicsPixmapItem* platePix;
    void setPlate(unsigned char plateType);


    void setEnables(void);
    void stepProfile(int direction);    // Richiesta di selezione nuovo profilo
    void setProfile(void); // Imposta la grafica per il profilo corrente
};


#endif
