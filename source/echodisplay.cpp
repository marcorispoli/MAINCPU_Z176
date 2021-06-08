#include "application.h"
#include "appinclude.h"
#include "globvar.h"

EchoDisplay::EchoDisplay()
{
    connectionStatus = false;
}

// CODICE PACCHETTO
#define _ECHO_DB_ALL      1
#define _ECHO_PG          2
#define _ECHO_DB_ITEM     3
#define _ECHO_BUTTON      4
#define _ECHO_DATE        5


void EchoDisplay::rxData(QByteArray data)
{
    QString sval;
    int ival,iival;
    int opt;
    unsigned char cval;
    unsigned char packtype;
    int index;
    bool status;
    QString dd,mm,yy,hh,min,ss;


    QDataStream stream(&data, QIODevice::ReadWrite);

    loop_rx:


    // Riconoscimento Init Paccehtto
    stream >> cval; // (unsigned char) 0xAA;
    if(cval!=0xAA) return;

    stream >> cval; // (unsigned char) 0xBB;
    if(cval!=0xBB) return;
    stream >> cval; // (unsigned char) 0xCC;
    if(cval!=0xCC) return;
    stream >> cval; // (unsigned char) 0xDD;
    if(cval!=0xDD) return;


    stream >> packtype;

    switch(packtype)
    {
    ////////////////// ECHO PAGE //////////////////////////
    case _ECHO_PG:
        stream >> ival;
        stream >> iival;
        stream >> opt;
        GWindow::setPage(ival,iival,opt|DBase::_DB_NO_ECHO); // Cambia pagina disabilitando il contro-echo
        break;

    ////////////////// ECHO ALL //////////////////////////
    case _ECHO_DB_ALL:
        for(int ciclo=0; ciclo<ApplicationDatabase.database.size(); ciclo++)
        {
            switch(ApplicationDatabase.database[ciclo].type)
            {
            case DBase::_DB_T_INT:
                 stream >> ival;
                 ApplicationDatabase.setData(ciclo,ival,DBase::_DB_NO_ECHO);
            break;
            case DBase::_DB_T_STR:
                stream >> sval;
                ApplicationDatabase.setData(ciclo,sval,DBase::_DB_NO_ECHO);
                break;
            case DBase::_DB_T_UC:
                stream >> cval;
                ApplicationDatabase.setData(ciclo,cval,DBase::_DB_NO_ECHO);
                break;
            }
        }
        break;
    ////////////////// ECHO ITEM //////////////////////////
    case _ECHO_DB_ITEM:
        stream >> index; // indice database
        stream >> opt;   // Options propagate
        switch(ApplicationDatabase.database[index].type)
        {
        case DBase::_DB_T_INT:
             stream >> ival;
             ApplicationDatabase.setData(index,ival,opt|DBase::_DB_NO_ECHO);
        break;
        case DBase::_DB_T_STR:
            stream >> sval;
            ApplicationDatabase.setData(index,sval,opt|DBase::_DB_NO_ECHO);
            break;
        case DBase::_DB_T_UC:
            stream >> cval;
            ApplicationDatabase.setData(index,cval,opt|DBase::_DB_NO_ECHO);
            break;
        default:
            qDebug("DB campo non riconosciuto");
            return;
        }
        break;

    ////////////////// ECHO BUTTON //////////////////////////
    case _ECHO_BUTTON:
        stream >> index;
        stream >> status;
        GPush::pushActivate(index,status,DBase::_DB_NO_ECHO);
        break;

    case _ECHO_DATE:

        stream >>dd;
        stream>> mm;
        stream >> yy;
        stream >>hh;
        stream >> min;
        stream >> ss;
        setDate(dd,mm,yy,hh,min,ss);
        break;

    default:
        qDebug("PACCHETTO NON RICONOSCIUTO");
        return;
    }

    goto loop_rx;

return;


}





void EchoDisplay::connection(bool status)
{
    connectionStatus = status;
    if(status==false) return;

    // Operazioni in fase di connessione
    if(isMaster) masterDatabaseInitialization();
    else slaveDatabaseInitialization();



    return;
}

// Aggiornamento cambio pagina
void  EchoDisplay::setNewPageEcho(int index, int pagePrev, int opt)
{
    QByteArray datagram;
    unsigned char cval;
    int    ival;

    if (connectionStatus==false) return;
    if (opt&DBase::_DB_NO_ECHO) return; // Il cambiamento è stato generato da un segnale di echo e non da modifiche interne

    datagram.clear();
    QDataStream stream(&datagram, QIODevice::ReadWrite);

    // Init Messaggio
    stream << (unsigned char) 0xAA;
    stream << (unsigned char) 0xBB;
    stream << (unsigned char) 0xCC;
    stream << (unsigned char) 0xDD;


    // Inserisce la codifica del messaggio
    cval=_ECHO_PG;
    stream << cval ;

    ival = index;
    stream << ival ;

    ival = pagePrev;
    stream << ival ;

    ival = opt;
    stream << ival ;

    emit txData(datagram);
    return;
}

// Aggiornamento singolo campo
void  EchoDisplay::dbChanged(int index,int opt)
{
    QByteArray datagram;
    unsigned char cval;
    if (connectionStatus==false) return;
    if (opt&DBase::_DB_NO_ECHO) return; // Il cambiamento è stato generato da un segnale di echo e non da modifiche interne

    datagram.clear();
    QDataStream stream(&datagram, QIODevice::ReadWrite);
    stream << (unsigned char) 0xAA;
    stream << (unsigned char) 0xBB;
    stream << (unsigned char) 0xCC;
    stream << (unsigned char) 0xDD;

    // Inserisce la codifica del messaggio
    cval=_ECHO_DB_ITEM;
    stream << cval ;
    stream<< index;
    stream<< opt; // Propaga le options

    // Costruisce lo stream con il contenuto del database
    switch(ApplicationDatabase.getType(index))
    {
    case DBase::_DB_T_INT:
        stream << ApplicationDatabase.getDataI(index);
        break;
    case DBase::_DB_T_STR:
        stream << ApplicationDatabase.getDataS(index);
        break;
    case DBase::_DB_T_UC:
        stream << ApplicationDatabase.getDataU(index);
        break;
    }

    // Trasmette
    emit txData(datagram);    
    return;
}

void  EchoDisplay::buttonChanged(int id, bool status, int opt)
{
    QByteArray datagram;
    unsigned char cval;
    if (connectionStatus==false) return;
    if (opt&DBase::_DB_NO_ECHO) return; // Il cambiamento è stato generato da un segnale di echo e non da modifiche interne

    QDataStream stream(&datagram, QIODevice::ReadWrite);
    stream << (unsigned char) 0xAA;
    stream << (unsigned char) 0xBB;
    stream << (unsigned char) 0xCC;
    stream << (unsigned char) 0xDD;

    // Inserisce la codifica del messaggio
    cval=_ECHO_BUTTON;
    stream << cval ;
    stream<< id;
    stream<< status;


    // Trasmette
    emit txData(datagram);
    return;
}

void  EchoDisplay::echoDate(QString dd, QString mm, QString yy,QString hh, QString min, QString ss,int opt)
{
    QByteArray datagram;
    unsigned char cval;

    if (connectionStatus==false) return;
    if (opt&DBase::_DB_NO_ECHO) return; // Il cambiamento è stato generato da un segnale di echo e non da modifiche interne

    QDataStream stream(&datagram, QIODevice::ReadWrite);
    stream << (unsigned char) 0xAA;
    stream << (unsigned char) 0xBB;
    stream << (unsigned char) 0xCC;
    stream << (unsigned char) 0xDD;

    // Inserisce la codifica del messaggio
    cval=_ECHO_DATE;
    stream << cval ;

    // Inserisce i dati del messaggio
    stream << dd;
    stream<< mm;
    stream << yy;
    stream<< hh;
    stream << min;
    stream<< ss;
    // Trasmette
    emit txData(datagram);
    return;
}

void  EchoDisplay::setDate(QString dd, QString mm, QString yy,QString hh, QString min, QString ss)
{
    QString command;

    command = QString("date -u %1%2%3%4%5.%6").arg(mm).arg(dd).arg(hh).arg(min).arg(yy).arg(ss);
    system(command.toStdString().c_str());

    command = QString("hwclock -w");
    system(command.toStdString().c_str());

    systemTimeUpdated = TRUE;
    return;
}

// Inizializzazioni alla connessione delle parti di Database che dipendono dal Master
void  EchoDisplay::masterDatabaseInitialization(void)
{
    if(pConfig->userCnf.deadman) ApplicationDatabase.setData(_DB_DEAD_MEN,(unsigned char) 1 , DBase::_DB_FORCE_SGN);
    else ApplicationDatabase.setData(_DB_DEAD_MEN,(unsigned char) 0, DBase::_DB_FORCE_SGN);

    if(pConfig->userCnf.demoMode) ApplicationDatabase.setData(_DB_DEMO_MODE,(unsigned char) 1, DBase::_DB_FORCE_SGN);
    else ApplicationDatabase.setData(_DB_DEMO_MODE,(unsigned char) 0, DBase::_DB_FORCE_SGN);

    ApplicationDatabase.setData(_DB_COMPRESSOR_PAD,pCompressore->getPadName(PAD_ND),DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_COLLIMAZIONE,QString(QApplication::translate("COLLIMATORE","NON DEFINITA", 0, QApplication::UnicodeUTF8)),DBase::_DB_FORCE_SGN);

    // Linguaggio applicazione
    ApplicationDatabase.setData(_DB_LINGUA,pConfig->userCnf.languageInterface , DBase::_DB_FORCE_SGN);


    // Configurazione di sistema
    unsigned char conf=0;
    if(pConfig->sys.highSpeedStarter) conf|=_ARCH_HIGH_SPEED_STARTER;
    if(pConfig->sys.trxMotor) conf|=_ARCH_TRX_MOTOR ;
    if(pConfig->sys.armMotor) conf|=_ARCH_ARM_MOTOR ;    
    ApplicationDatabase.setData(_DB_SYSTEM_CONFIGURATION, (unsigned char) conf , DBase::_DB_FORCE_SGN);

    ApplicationDatabase.setData(_DB_VPRIMARIO, (int) pGeneratore->genCnf.pcb190.HV_VPRIMARIO, DBase::_DB_FORCE_SGN);
    ApplicationDatabase.setData(_DB_ACCESSORY_NAME,QString(QApplication::translate("POTTER","ACCESSORIO NON DEFINITO", 0, QApplication::UnicodeUTF8)), DBase::_DB_FORCE_SGN);
}

// Inizializzazioni alla connessione delle parti di Database che dipendono dallo Slave
void  EchoDisplay::slaveDatabaseInitialization(void)
{

}
