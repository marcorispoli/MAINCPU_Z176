#ifndef PANNELLOCOMANDI_H
#define PANNELLOCOMANDI_H

#include "../application.h"

#include <QWidget>
#include <QGraphicsScene>


class pannelloComandi : public QGraphicsScene
{
    Q_OBJECT

public:
    pannelloComandi(QGraphicsView* view);
    ~pannelloComandi();
     bool isOpen() {return open_flag;}

signals:

public slots:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void timerEvent(QTimerEvent* ev);
    void valueChanged(int index,int opt);

    void open(void);
    void exit();


    void setCampi(unsigned char ncampi);
    void setKv(int val);
    void setdmAs(int dmAs);
    void setProfileLabel(QString name);
    void setPlate(unsigned char plateType);
    void setOd(int od);
    void setTechnic(int tech);
    void setTechMode(int tm);
    void setProjectionPix(void);
    void setFuoco(int fuoco);
    void setFilterField(int filtro, int automode);
    void setAnode(QString anodo);
    void setReady(bool ready);
    void xrayPixActivation(bool stat);    
    void setArm(int angolo);


public:
    bool config_changed;
private:
    int timerDisable;
    QGraphicsView* parent;
    bool open_flag;

    QGraphicsPixmapItem* comandiPix;
    QGraphicsPixmapItem* readyNotReadyPix;
    QGraphicsPixmapItem* campiPix;
    QGraphicsPixmapItem* optionsPix;
    QGraphicsPixmapItem* platePix;
    QGraphicsPixmapItem* projectionPix;
    QGraphicsPixmapItem* wrongProjectionPix;
    QGraphicsPixmapItem* focusPix;
    QGraphicsPixmapItem* xrayPix;

    GLabel* kvLabel;
    GLabel* masLabel;
    GLabel* profileLabel;
    GLabel* odLabel;
    GLabel* technicLabel;
    GLabel* techModeLabel;
    GLabel* anodeLabel;
    GLabel* filterLabel;
    GLabel* autoFilterLabel;
    GLabel* doseLabel;
    GLabel* mAsXLabel;
    GLabel* kvXLabel;
    GLabel* angoloArm;

    QString projectionsFiles[17];

    void stepCampi(void); // Effettua uno step di selezione campi AEC

    int currentBackground; // 0 = Rot Manuale, 1 = Rot motorizzata
};


#endif
