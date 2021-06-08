#ifndef PANNELLOKV_H
#define PANNELLOKV_H

#include "../application.h"

#include <QWidget>
#include <QGraphicsScene>


class pannelloKv : public QGraphicsScene
{
    Q_OBJECT

public:
    pannelloKv(QGraphicsView* view);
    ~pannelloKv();
    bool isOpen() {return open_flag;}

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
    GLabel* kvSelezionati;

    int entry_dkV;
    int selected_dkV;
    int max_dKv;
    int min_dKv;

};


#endif
