#ifndef SERIAL_INTERFACE_H
#define SERIAL_INTERFACE_H

#include "application.h"
#include <QFile>

class SerialInterface {

public:


    explicit SerialInterface(void);

    void sendWelcomMessage(void);
    QByteArray format2(unsigned char val);
    QByteArray format4(unsigned short val);
    QByteArray format4(float val);

    void sendMessage(QByteArray data);
    QFile* device;
};

#endif // SERIAL_INTERFACE_H
