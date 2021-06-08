#ifndef PANNELLOCOLLI_H
#define PANNELLOCOLLI_H

#include "../application.h"

#include <QWidget>
#include <QGraphicsScene>
#include "lib/numericpad.h"

class pannelloColli : public QGraphicsScene
{
    Q_OBJECT

public:
    pannelloColli(QGraphicsView* view,QWidget* widget);
    ~pannelloColli();
    bool isOpen() {return open_flag;}


public slots:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void timerEvent(QTimerEvent* ev);
    void valueChanged(int index,int opt);

    void open();
    void exit();

    void customCalcSlot(bool result);

private:
    int timerDisable;
    QGraphicsView*  parentView;
    QWidget*        parentWidget;
    bool open_flag;


    QGraphicsPixmapItem* backgroundPix;
    numericPad* pCalculator;
    QString dataField;

    GLabel* formatLabel;
    GLabel* LLabel;
    GLabel* RLabel;
    GLabel* BLabel;
    GLabel* FLabel;

    void onScrollLeft(void);
    void onScrollRight(void);
    void onLeft(void);
    void onRight(void);
    void onFront(void);
    void onBack(void);

    QList<QString> formatList;
};


#endif
