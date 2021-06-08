#ifndef DBASE_H
#define DBASE_H

#include <QtGui>
#include <QApplication>

class DBaseItem
{

public:
    explicit DBaseItem(QString sdata){sData=sdata;}
    explicit DBaseItem(unsigned char cdata){cData=cdata;}
    explicit DBaseItem(int idata){iData=idata;}

public:
    QString sData;
    unsigned char cData;
    int iData;
    int itemIndex;
    int type;
};

class DBase : public QObject
{
    Q_OBJECT

public:
    DBase(); // Crea il database vuoto
    DBase(DBaseItem item); // Crea il database con un elemento
    enum {
        _DB_T_INT  = 0,
        _DB_T_UC   = 1,
        _DB_T_STR  = 2
    } DATA_TYPE;

    enum {
        _DB_NO_OPT      = 0,            // Nessuna option
        _DB_NO_ECHO     = 1,            // Il modulo echo non invia l'eco relativo
        _DB_NO_WIN_UPD  = 2,            // (nessun automatismo) Non bisogna effettuare update grafico
        _DB_NO_CHG_SGN  = 4,            // Il Database non genera il segnale di dato cambiato
        _DB_FORCE_SGN   = 8,            // Forza invio del segnale
        _DB_NO_ACTION   = 0x10,         // ValueChange non deve essere eseguito
        _DB_ONLY_MASTER_ACTION = 0x20,  // ValueChange deve essere eseguito solo dal MASTER
        _DB_ONLY_SLAVE_ACTION  = 0x40,  // ValueChange deve essere eseguito solo dal SLAVE
        _DB_INIT_PAGE  = 0x80,          // Chiede l'inizializzazione paina (se applicabile)
        _DB_EXIT_PAGE  = 0x100          // Chiede l'uscita da pagina (se applicabile)
    } OPTION_TYPE;


    // Costruzione database
    void append(QString);
    void append(int);
    void append(unsigned char);

    void setData(int index,QString data,int opt=0);
    void setData(int index,unsigned char data,int opt=0);
    void setData(int index,int data,int opt=0);

    QString getDataS(int index);
    unsigned char getDataU(int index);
    int getDataI(int index);
    int getType(int index);

signals:
    void dbDataChanged(int,int); // Per il cambio valori del database
    void dbEchoSignal(int,int);  // Per l'invio echo

public slots:

// Data
public:
    QList<DBaseItem> database;  // Lista di items nel database

private:
    QMutex* dataProtection;
    int dbIndex;
};

#endif // DBASE_H
