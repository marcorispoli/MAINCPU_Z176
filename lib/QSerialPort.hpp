//////////////////////////////////////////////////////////
// Serial Port Driver Header
// Peter Fetterer
// Biovore@gmail.com
/////////////////////////////////////////////////////////
// Release under Creative Commons
// No warrenty given, use at your own risk.
////////////////////////////////////////////////////////
// This is a simple, cross platform example of a serial port driver for windows/linux.
// The code here can probably be improved with some help from some folks that know more then I do.
// This code only supports byte level read/writes.  We do not process based on "line terminators" or "timeout" at this time.
//
// How to use this thing:
// This project is setup to be a libaray to use in another project
// thus requires you include it into your .pro file as a external project
// you could also just take the cpp and hpp class files and put them in your project.
//
// Step 1:
//  Make an instance of the QSerialPort class
//      QSerialPort *MySerialPort = new QSerialPort();
//
// Step 2:
//  Make connections of signals to slots
//  This connection signals your use class when there is receive data to read
//      Must:
//          connect(MySerialPort, SIGNAL(hasData()), MainClass, SLOT(processData()));
//
//  This connections are nice because the can tell your code if the serial opened correctly or not.
//      Optional:
//          connect(MySerialPort, SIGNAL(openPortFailed()), MainClass, SLOT(openPortFailed()));
//          connect(MySerialPort, SIGNAL(openPortSuccess()), MainClas, SLOT(openPortSuccess()));
//
//      Maybe be nice to look at the buffer overflow signal.  Probably not much you can do if it happens.
//      But is nice to tell you that data is getting dropped when this happens.
//
// Step 3:
//  Setup comm params
//          MySerialPort->usePort("COM1", B9600, CS8, SB1, ParityNone);
//      OR:
//          MySerialPort->usePort("/dev/ttyUSB0", B9600, CS8, SB1, ParityNone);
//
// Step 4:
//  Start the serial polling thread
//  MySerialPort->start();
//
// Now when serial data is received, the processData() SLOT in your MainClass should get called.
// Method of use in your processes data slot.
//
// serialDevice->bytesAvailable()
//      return the number of byte in receive buffer to be processed
//
// byte = serialDevice->getNextByte()
//      returns the next byte in the buffer
//
// TODO:
//      Come up with a QSerialPort method to read a line at a time
//      given that a line ends in a line terminator like \n or \r
//
//////////////////////////////////////////////////////////////////////////

#ifndef QSERIALPORT_HPP
#define QSERIALPORT_HPP

// library linking info
#include "QSerialPort_Global.hpp"

// Common Stuff
#include <QThread>
#include <QMutex>
#include <QSemaphore>

// enables verbose qDebug messages
#define _SERIALTHREAD_DEBUG 0

#ifdef Q_OS_WIN32

///////////////////////////////////////////////////////
//  IF BUILDING ON WINDOWS, IMPLEMENT WINDOWS VERSION
//  THE SERIAL PORT CLASS.
///////////////////////////////////////////////////////

#include <windows.h>
#include <stdint.h>


// default defined baud rates
// custom ones could be set.  These are just clock dividers from some base serial clock.
#ifdef Q_OS_WIN32
// Use windows definitions
#define Baud300        CBR_300
#define Baud600        CBR_600
#define Baud1200       CBR_1200
#define Baud2400       CBR_2400
#define Baud4800       CBR_4800
#define Baud9600       CBR_9600
#define Baud19200      CBR_19200
#define Baud38400      CBR_38400
#define Baud57600      CBR_57600
#define Baud115200     CBR_115200
#else
// Use Posix definitions
#define Baud300        B300
#define Baud600        B600
#define Baud1200       B1200
#define Baud2400       B2400
#define Baud4800       B4800
#define Baud9600       B9600
#define Baud19200      B19200
#define Baud38400      B38400
#define Baud57600      B57600
#define Baud115200     B115200
#endif

// bytes sizes
#ifdef Q_OS_WIN32
// windows byte defines
#define CS8            8
#define CS7            7
#define CS6            6
#define CS5            5
#else
// posix is already CS8 CS7 CS6 CS5 defined
#endif

// parity
#ifdef Q_OS_WIN32
#define ParityEven      EVENPARITY
#define ParityOdd       ODDPARITY
#define ParityNone      NOPARITY
#else
#define ParityEven      PARENB
#define ParityOdd       PARENB | PARODD
#define ParityNone      0
#endif

// stop bit
#ifdef Q_OS_WIN32
#define SB1             0
#define SB2             CSTOPB
#else
#define SB1             ONESTOPBIT
#define SB2             TWOSTOPBIT
#endif


class QSerialPort : public QThread
{
    Q_OBJECT
public:
    explicit QSerialPort(QObject *parent = 0);
    ~QSerialPort();
    void usePort(QString *device_Name, int _buad, int _byteSize, int _stopBits, int _parity);
    void closePort();

    // data fetcher, get next byte from buffer
    uint8_t getNextByte();

    // return number of bytes in receive buffer
    uint32_t bytesAvailable();

    // write buffer
    int writeBuffer(QByteArray *buffer);

protected:
    // thread process, called with a start() defined in the base class type
    // This is our hardware receive buffer polling thread
    // when data is received, the hasData() signal is emitted.
    virtual void run();

signals:
    // asynchronous signal to notify there is receive data to process
    void hasData();
    // signal that we couldn't open the serial port
    void openPortFailed();
    // signal that we openned the port correct and are running
    void openPortSuccess();
    // RX buffer overflow signal
    void bufferOverflow();

public slots:
    // we don't need no sinking slots

private:
    // serial port settings
    int Buad;
    int ByteSize;
    int StopBits;
    int Parity;

    HANDLE hSerial;
    bool running;
    QByteArray *dataBuffer;
    QSemaphore *bufferSem;
    QString *deviceName;
    // windows uses a struct called DCB to hold serial port configuration information
    DCB dcbSerialParams;
};

#else // not Q_OS_WIN32

////////////////////////////////////////////////////////////////
//  IF USING A POSIX OS, ONE THAT UNDSTANDS THE NOTION        /
//  OF A TERMINAL DEVICE (Linux,BSD,Mac OSX, Solaris, etc)   /
/////////////////////////////////////////////////////////////

// Assuming posix compliant OS  (Tested on Linux, might work on Mac / BSD etc )

#include <inttypes.h>
#include <termios.h>

// Use Posix definitions
#define Baud300        B300
#define Baud600        B600
#define Baud1200       B1200
#define Baud2400       B2400
#define Baud4800       B4800
#define Baud9600       B9600
#define Baud19200      B19200
#define Baud38400      B38400
#define Baud57600      B57600
#define Baud115200     B115200

#define ParityEven      PARENB
#define ParityOdd       PARENB | PARODD
#define ParityNone      0

#define SB1             ONESTOPBIT
#define SB2             TWOSTOPBIT

class QSerialPort : public QThread
{
    Q_OBJECT
public:
    explicit QSerialPort(QObject *parent = 0);
    ~QSerialPort();
    void usePort(QString *device_Name, int _buad, int _byteSize, int _stopBits, int _parity);
    void usePort(QString *device_Name, int _buad, int _byteSize, int _stopBits, int _parity, int min_len);

    void closePort();

    // data fetcher, get next byte from buffer
    uint8_t getNextByte();

    // return number of bytes in receive buffer
    uint32_t bytesAvailable();

    void resetBuffer(); // Erase current buffer

    // write buffer
    int writeBuffer(QByteArray *buffer);

protected:
    // thread process, called with a start() defined in the base class type
    virtual void run();

signals:
    // asynchronous signal to notify there is receive data to process
    void hasData();
    // signal that we couldn't open the serial port
    void openPortFailed();
    // signal that we openned the port correct and are running
    void openPortSuccess();
    // RX buffer overflow signal
    void bufferOverflow();

public slots:
    // we don't need no sinking slots

private:
    // serial port settings
    int Buad;
    int ByteSize;
    int StopBits;
    int Parity;
    int MinLen;

    int sfd;
    bool running;
    QByteArray *dataBuffer;
    QMutex *bufferMutex;
    QString *deviceName;
    // termio structs
    struct termios oldtio, newtio;
};

#endif // OS Selection

#endif // QSERIALPORT_HPP
