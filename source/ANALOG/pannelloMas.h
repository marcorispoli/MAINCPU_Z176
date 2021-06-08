#ifndef PANNELLOMAS_H
#define PANNELLOMAS_H

#include "../application.h"

#include <QWidget>
#include <QGraphicsScene>


class pannelloMas : public QGraphicsScene
{
    Q_OBJECT

public:
    pannelloMas(QGraphicsView* view);
    ~pannelloMas();
    bool isOpen() {return open_flag;}

    void getNearestR20DMas(unsigned short dmas, unsigned char* tab, unsigned char* index);
    void setCurrentTab(unsigned char tab, unsigned short dmin, unsigned short dmax);

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
    QGraphicsPixmapItem* disabledPix[16];
    bool                 masEnabled[16];

    GLabel* masSelezionati;


    float entryMas;
    float selectedMas;
    unsigned short maxdMas;
    unsigned short mindMas;

};


#endif
