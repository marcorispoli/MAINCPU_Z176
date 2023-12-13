#ifndef PANNELLOMAG_H
#define PANNELLOMAG_H

#include "../application.h"

#include <QWidget>
#include <QGraphicsScene>


class pannelloMag : public QGraphicsScene
{
    Q_OBJECT

public:
    pannelloMag(QGraphicsView* view,QWidget* widget);
    ~pannelloMag();
    bool isOpen() {return open_flag;}


public slots:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void timerEvent(QTimerEvent* ev);
    void valueChanged(int index,int opt);

    void open();
    void exit();

private:
    int timerDisable;
    int timerBlink;
    bool blinking;

    QGraphicsView*  parentView;
    QWidget*        parentWidget;
    bool open_flag;


    QGraphicsPixmapItem* backgroundPix;
    GLabel* formatLabel;

    void onScrollLeft(void);
    void onScrollRight(void);
    void onOk(void);
    QList<QString> formatList;
};


#endif
