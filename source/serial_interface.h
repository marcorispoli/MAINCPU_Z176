#ifndef SERIAL_INTERFACE_H
#define SERIAL_INTERFACE_H

#include "application.h"
#include <QFile>

class SerialInterface: public QObject
{
    Q_OBJECT

public:
    explicit SerialInterface(void);

    typedef enum{
        SI_W = 0,
        SI_Rh,
        SI_Mo,
    }FilterT;
    bool sendExposureData(unsigned char kV, float mAs, bool large_focus, FilterT filter, unsigned short thick, unsigned short force,  unsigned short mA, unsigned short time, unsigned char mag_factor, int angle, unsigned char tech);


private:


    QByteArray format2(unsigned char val);
    QByteArray format3(unsigned short val);
    QByteArray format4(unsigned short val);
    QByteArray format4(float val);

    int fd_serialport;
    bool sendMessage(QByteArray data);
};

#endif // SERIAL_INTERFACE_H
