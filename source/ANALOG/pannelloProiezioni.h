#ifndef pannelloProiezioni_H
#define pannelloProiezioni_H

#include "../application.h"

#include <QWidget>
#include <QGraphicsScene>


class pannelloProiezioni : public QGraphicsScene
{
    Q_OBJECT

public:
    pannelloProiezioni(QGraphicsView* view);
    ~pannelloProiezioni();
    bool isOpen() {return open_flag;}

    void setLat(int lat); // Imposta la grafica del pannello in funzione della lateralità corrente

signals:



public slots:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void timerEvent(QTimerEvent* ev);
    void open(void);
    void exit();



private:
    int timerDisable;
    QGraphicsView* parent;
    bool open_flag;

    QGraphicsPixmapItem* backgroundPix;

    QRectF latRect;
    QRectF exitRect;

};


#endif
