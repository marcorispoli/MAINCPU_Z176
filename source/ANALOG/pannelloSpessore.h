#ifndef PANNELLOSPESSORE_H
#define PANNELLOSPESSORE_H

#include "../application.h"

#include <QWidget>
#include <QGraphicsScene>


class pannelloSpessore : public QGraphicsScene
{
    Q_OBJECT

public:
    pannelloSpessore(QGraphicsView* view);
    ~pannelloSpessore();

public slots:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void timerEvent(QTimerEvent* ev);
    void databaseChanged(int index,int opt);
    void init();
    void exit();




private:
    int timerDisable;
    QGraphicsPixmapItem* spessorePix;
    GLabel* spessoreLabel;
    GLabel* magFactor;

    void updateSpessore();
};


#endif
