#include "serial_interface.h"
#include "appinclude.h"
#include "globvar.h"
#include "systemlog.h"
extern systemLog* pSysLog;

#include <QIODevice>

static unsigned char vect[] = {
        0x30, 0x3a, 0x32 , 0x36 , 0x3b , 0x31 , 0x3a , 0x30 , 0x31 , 0x36 , 0x30 , 0x3b , 0x32 , 0x3a , 0x30
        , 0x30 , 0x39 , 0x3b , 0x33 , 0x3a , 0x30 , 0x30 , 0x30 , 0x3b , 0x34 , 0x3a , 0x4d , 0x6f , 0x3b , 0x35, 0x3a
        , 0x41 , 0x74 , 0x3b , 0x36 , 0x3a , 0x30 , 0x39 , 0x36 , 0x3b , 0x37 , 0x3a , 0x31 , 0x36 , 0x36 , 0x30 , 0x3b
        , 0x38 , 0x3a , 0x4d , 0x75 , 0x3b , 0x39 , 0x3a , 0x2b , 0x30 , 0x3b , 0x31 , 0x30 , 0x3a , 0x4c , 0x46 , 0x3b
        , 0x31 , 0x31 , 0x3a , 0x2d , 0x3b , 0x31 , 0x32 , 0x3a , 0x30 , 0x30 , 0x30 , 0x3b , 0x31 , 0x33 , 0x3a , 0x2b
        , 0x30 , 0x30 , 0x30 , 0x3b , 0x31 , 0x34 , 0x3a , 0x2d , 0x3b , 0x31 , 0x35 , 0x3a , 0x30 , 0x34 , 0x2e , 0x34
        , 0x35 , 0x3b
};

SerialInterface::SerialInterface(void){

    // Initializes the serial driver in the linux system
    QString command = QString("stty /dev/ttymxc3 9600");
    system(command.toStdString().c_str());
    command = QString("stty /dev/ttymxc3 -crtscts");
    system(command.toStdString().c_str());

    device = new QFile("/dev/ttymxc3");
}


void SerialInterface::sendWelcomMessage(void){


    try{
        device->open(QIODevice::WriteOnly);
        QString message = "Welcome to the drtech interface\n";
        device->write(message.toAscii().data());
        device->close();
    }catch (...){}


}

QByteArray SerialInterface::format2(unsigned char val){
    QByteArray result;
    result.append('0');
    result.append('0');


    if(val >= 10){
        unsigned char v = (unsigned char) (val/10);
        result[0] ='0'+v;
        val = val-10*v;
    }

    result[1] = '0'+val;
    return result;
}

QByteArray SerialInterface::format4(unsigned short val){

    QByteArray result;
    result.append('0');
    result.append('0');
    result.append('0');
    result.append('0');

    if(val >= 1000){
        unsigned char v = (unsigned char) (val/1000);
        result[0] ='0'+v;
        val = val-1000*v;
    }
    if(val >= 100){
        unsigned char v = (unsigned char) (val/100);
        result[1] ='0'+v;
        val = val-100*v;
    }

    if(val >= 10){
        unsigned char v = (unsigned char) (val/10);
        result[2] ='0'+v;
        val = val-10*v;
    }

    result[3] = '0'+val;
    return result;
}

QByteArray SerialInterface::format4(float val){

    if(val >= 100) return format4((unsigned short) val);

    QByteArray result =  QString("%1").arg(val,-4,'f',2,'0').toAscii();
    if(result.size() > 4) result.resize(4);
    return result;
}

void SerialInterface::sendMessage(QByteArray data){

    QByteArray frame;
    unsigned char chs = 0;


    // Calculates the LRC checksum
    int ichs = 0;
    for(int i=0; i< data.length(); i++){
        ichs -= (int) data[i];
    }
    chs = *((unsigned char*) &ichs);




    frame.append(0x2); // STX
    //_______________________


    //_______________________
    frame.append(chs); // CHECKSUM

    //_______________________
    frame.append(0x3); // ETX




    try{
        device->open(QIODevice::WriteOnly);
        device->write(frame.data());
        device->close();
    }catch (...){}

}
