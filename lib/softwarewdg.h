#ifndef SOFTWAREWDG_H
#define SOFTWAREWDG_H

#include "../source/application.h"

class softwareWDG : public QObject
{
    Q_OBJECT
public:
    explicit softwareWDG(QObject *parent = 0);
    
    bool refresh;
    QMutex mutexWDG;
signals:
    
public slots:
    void watchdogThread(void);

};

#endif // SOFTWAREWDG_H
