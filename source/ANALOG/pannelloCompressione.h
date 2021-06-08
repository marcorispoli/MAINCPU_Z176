#ifndef PANNELLOCOMPRESSIONE_H
#define PANNELLOCOMPRESSIONE_H

#include "../application.h"

#include <QWidget>
#include <QGraphicsScene>


class pannelloCompressione : public QGraphicsScene
{
    Q_OBJECT

public:
    pannelloCompressione(QGraphicsView* view);
    ~pannelloCompressione();

public slots:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void timerEvent(QTimerEvent* ev);
    void databaseChanged(int index,int opt);
    void init();
    void exit();




private:
    int timerDisable;
    QGraphicsPixmapItem* forzaPix;
    GLabel* forzaLabel;
    GLabel* targetLabel;

    void updateForza();

};


#endif
