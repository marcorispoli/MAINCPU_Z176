#include "dbase.h"
#include "../source/print.h"
extern bool isMaster;

DBase::DBase()
{
    database.clear();
    dbIndex=0;
    dataProtection = new QMutex(QMutex::Recursive) ;//RecursionMode=QMutex::Recursive;
}
DBase::DBase(DBaseItem item)
{
    database.clear();
    dbIndex=1;
    item.itemIndex=0;
    database.append(item);
}

void DBase::append(QString str)
{
    DBaseItem item=DBaseItem(str);
    item.itemIndex=dbIndex++;
    item.type = DBase::_DB_T_STR;
    this->database.append(item);

    return;
}
void DBase::append(unsigned char data)
{
    DBaseItem item=DBaseItem(data);
    item.itemIndex=dbIndex++;
    item.type = DBase::_DB_T_UC;

    this->database.append(item);
    return;
}
void DBase::append(int data)
{
    DBaseItem item=DBaseItem(data);
    item.itemIndex=dbIndex++;
    item.type = DBase::_DB_T_INT;
    this->database.append(item);
    return ;
}

void DBase::setData(int index,QString data, int opt)
{

    dataProtection->lock();


    if(( database.at(index).sData != data) || (opt & _DB_FORCE_SGN))
    {
        database[index].sData = data;
        if(!(opt&_DB_NO_CHG_SGN))  emit dbDataChanged(index,opt);
        if(!(opt&_DB_NO_ECHO))  emit dbEchoSignal(index,opt);
    }
    dataProtection->unlock();

}

void DBase::setData(int index,unsigned char data,int opt)
{

    dataProtection->lock();


    if(( database.at(index).cData != data)|| (opt & _DB_FORCE_SGN))
    {
        database[index].cData = data;
        if(!(opt&_DB_NO_CHG_SGN))  emit dbDataChanged(index,opt);
        if(!(opt&_DB_NO_ECHO))  emit dbEchoSignal(index,opt);
    }
    dataProtection->unlock();

}

void DBase::setData(int index,int data,int opt)
{

    dataProtection->lock();

    if((database.at(index).iData != data)|| (opt & _DB_FORCE_SGN))
    {
        database[index].iData = data;
        if(!(opt&_DB_NO_CHG_SGN))  emit dbDataChanged(index,opt);
        if(!(opt&_DB_NO_ECHO))  emit dbEchoSignal(index,opt);
    }
    dataProtection->unlock();

}

QString DBase::getDataS(int index)
{
    return database.at(index).sData;
}

unsigned char DBase::getDataU(int index)
{
    return database.at(index).cData;
}

int DBase::getDataI(int index)
{
    return database.at(index).iData;
}

int DBase::getType(int index)
{
    return database.at(index).type;
}
