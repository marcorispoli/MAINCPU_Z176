#include "./source/application.h"
#include "./source/appinclude.h"
#include "./source/globvar.h"


softwareWDG::softwareWDG(QObject *parent) :
    QObject(parent)
{
    QThread *Thread = new QThread(this);    // Crea la thread

    connect(Thread, SIGNAL(started()), this, SLOT(watchdogThread()));
    connect(Thread, SIGNAL(finished()), this, SLOT(deleteLater()));
    this->moveToThread(Thread);
    Thread->start();
}

void softwareWDG::watchdogThread(void)
{
    QWaitCondition attesa;
    QString command;

    qDebug() << "WATCHDOG PARTITO";
    //timerStart(2000);
    command=QString("./tst");
    while(1)
    {
        mutexWDG.lock();
        attesa.wait(&mutexWDG,3000);
        mutexWDG.unlock();
        if(refresh==FALSE)
        {
            qDebug() << "FALLITO";
            system(command.toStdString().c_str());
        }
        else
        {

            qDebug() << "TUTTO OK";
        }

        refresh = FALSE;
    }
}


