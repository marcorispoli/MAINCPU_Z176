#include "appinclude.h"
#include "systemlog.h"

#define _MAX_LOG_LINES 10000
#define _MAX_LOG_BUFFER 1000

systemLog::systemLog(QString filename, QObject *parent) :
    QObject(parent)
{
    QList<QString> dati;
    QString frame;

    // Apertura file LOG
    this->filename = filename;
    file.setFileName(filename);
    enabled =false;
    fileLines = 0;

    // Se non esiste lo crea con l'intestazione
    if(!file.exists()){
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) return;
        frame = QString("%1\n").arg((int) 0);
        file.write(frame.toAscii().data());
        fileLines = 0;
        file.close();
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append)) return;
        enabled = true;
        return;
    }

    // Se esiste lo apre ed aggiorna il numero di linee
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) return;

    // Legge la prima riga per determinare il numero dil linee
    if(file.atEnd()) return;

    frame = file.readLine();
    fileLines =frame.toInt();
    file.close();
    enabled = true;

    // Controllo sul massimo numero di linee possibili
    if(fileLines >= _MAX_LOG_LINES) resizeFile();


    if (!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append)){enabled=false; return;}

    activated = true;
    return;


}
void systemLog::log(QString event){
    if(!activated) return;
    if(!enabled) return;
    QString stringa = QString("[%1] >%2\n").arg(QDateTime::currentDateTime().toString("dd.MM.yy hh.mm.ss ap")).arg(event);
    file.write(stringa.toAscii().data());
    fileLines++;
    changed = true;

}

// prefix==true attiva la stampa della data
void systemLog::log(QString event,bool prefix){
    if(!activated) return;
    if(!enabled) return;

    QString stringa;
    if(prefix) stringa = QString("[%1] >%2\n").arg(QDateTime::currentDateTime().toString("dd.MM.yy hh.mm.ss ap")).arg(event);
    else stringa = QString("%1\n").arg(event);

    file.write(stringa.toAscii().data());
    fileLines++;
    changed = true;

}

void systemLog::flush(void){
    if(!enabled) return;
    if(!changed) return;
    file.close();

    // Aggiorna le linee
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) return;
    QString frame = QString("%1\n").arg((int) fileLines);
    file.write(frame.toAscii().data());
    file.close();
    file.flush();

    // Effettua un sync
    QString command = QString("sync");
    system(command.toStdString().c_str());
    changed = false;

     if (!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append)){enabled=false; return;}
}

void systemLog::resizeFile(void){
    QList<QString> dati;
    QByteArray frame;
    QString command;

    QFile newfile("/newlog");
    if (!newfile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    // Aggiorna le linee
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;


    // Legge le prime linee per scartarle
    int j= fileLines -  _MAX_LOG_LINES +  _MAX_LOG_BUFFER +1;
    while(!file.atEnd())
    {
        file.readLine();
        j--;
        if(!j) break;
    }

    // Scrive il numero di righe tolte le prime
    fileLines = _MAX_LOG_LINES - _MAX_LOG_BUFFER;
    QString stringa = QString("%1\n").arg((int) fileLines);
    newfile.write(stringa.toAscii().data());

    // Copia le righe restanti
    while(!file.atEnd())
    {
        frame = file.readLine();
        newfile.write(frame.data());
    }

    file.close();
    newfile.close();
    file.flush();
    newfile.flush();

    command = QString("sync");
    system(command.toStdString().c_str());
    command = QString("mv /newlog ") + filename;
    system(command.toStdString().c_str());
    command = QString("sync");
    system(command.toStdString().c_str());
    changed = false;


}
