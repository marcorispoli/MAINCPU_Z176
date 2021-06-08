#ifndef SYSTEMLOG_H
#define SYSTEMLOG_H

#include <QObject>
#include <QFile>

class systemLog : public QObject
{
    Q_OBJECT
public:
    explicit systemLog(QString filename, QObject *parent = 0);
    void log(QString event);
    void log(QString event,bool prefix);
    void flush(void);
    void resizeFile(void);
    void activate(bool status){
        if(status) activated=true;
        else{
            activated = false;
            flush();
        }
    }

signals:
    
public slots:
    
private:
    QFile file;
    QString filename;
    int fileLines;
    bool enabled;
    bool changed;

    bool activated;

};

#endif // SYSTEMLOG_H
