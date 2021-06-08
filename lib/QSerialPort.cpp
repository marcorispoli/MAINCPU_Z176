#include "QSerialPort.hpp"
#include <QtDebug>

//////////////////////////////////////////
// Set header file for documentation    /
////////////////////////////////////////

//////////////////////////////////////////////////////
// Windows Version of the serial port driver Code
/////////////////////////////////////////////////////

#ifdef Q_OS_WIN32

#include <windows.h>

QSerialPort::QSerialPort(QObject *parent) :
    QThread(parent)
{
    // make everything in this thread, run in this thread. (Including signals/slots)
    QObject::moveToThread(this);
    // make our data buffer
    dataBuffer = new QByteArray();
    hSerial = INVALID_HANDLE_VALUE;
    running = true;
    deviceName=NULL;
    bufferSem = new QSemaphore(1); // control access to buffer
}

QSerialPort::~QSerialPort() {
    running = false;
    CloseHandle(hSerial);
}

//write data to serial port
int QSerialPort::writeBuffer(QByteArray *buffer) {
        int dwBytesRead = 0;
    if (hSerial != INVALID_HANDLE_VALUE) {
        // have a valid file discriptor
        WriteFile(hSerial, buffer->constData(), buffer->size(), (DWORD *)&dwBytesRead, NULL);
        return dwBytesRead;
    } else {
        return -1;
    }
}

// setup what device we should use
void QSerialPort::usePort(QString *device_Name, int _buad, int _byteSize, int _stopBits, int _parity) {
    deviceName = new QString(device_Name->toLatin1());
    // serial port settings
    Buad = _buad;
    ByteSize = _byteSize;
    StopBits = _stopBits;
    Parity = _parity;
}

// data fetcher, get next byte from buffer
uint8_t QSerialPort::getNextByte() {
    // mutex needed to make thread safe
    bufferSem->acquire(1); // lock access to resource, or wait untill lock is avaliable
    uint8_t byte = (uint8_t)dataBuffer->at(0); // get the top most byte
    dataBuffer->remove(0, 1); // remove top most byte
    bufferSem->release(1);
    return byte; // return top most byte
}

// return number of bytes in receive buffer
uint32_t QSerialPort::bytesAvailable() {
    // this is thread safe, read only operation
    return (uint32_t)dataBuffer->size();
}

// our main code thread
void QSerialPort::run() {
    bufferSem->release(1);      // not in a locked state

    // thread procedure
    if (_SERIALTHREAD_DEBUG) {
        qDebug() << "QSerialPort: QSerialPort Started..";
        qDebug() << "QSerialPort: Openning serial port " << deviceName->toLatin1();
    }

    // open selected device
    hSerial = CreateFile( (WCHAR *) deviceName->constData() , GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if ( hSerial == INVALID_HANDLE_VALUE ) {
        qDebug() << "QSerialPort: Failed to open serial port " << deviceName->toLatin1();
        emit openPortFailed();
        return;  // exit thread
    }

    // Yay we are able to open device as read/write
    qDebug() << "QSerialPort: Opened serial port " << deviceName->toLatin1() << " Sucessfully!";

    // now save current device/terminal settings
    dcbSerialParams.DCBlength=sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
                qDebug() << "QSerialPort: Failed to get com port paramters";
                emit openPortFailed();
                return;
        }

    if (_SERIALTHREAD_DEBUG) {
        qDebug() << "QSerialPort: Serial port setup and ready for use";
        qDebug() << "QSerialPort: Starting QSerialPort main loop";
    }
    dcbSerialParams.BaudRate=Buad;
    dcbSerialParams.ByteSize=ByteSize;
    dcbSerialParams.Parity=Parity;
    dcbSerialParams.StopBits=StopBits;

    if(!SetCommState(hSerial, &dcbSerialParams)) {
                qDebug() << "QSerialPort: Failed to set new com port paramters";
                emit openPortFailed();
                return;
    }

    // signal we are opened and running
    emit openPortSuccess();

    uint8_t byte; // temp storage byte
    int dwBytesRead;
    int state=0;    // state machine state

    // start polling loop
    while(running) {
        ReadFile(hSerial, (void *)&byte, 1, (DWORD *)&dwBytesRead, NULL); // reading 1 byte at a time..  only 2400 baud.
        // print what we received
        if (_SERIALTHREAD_DEBUG) {
            qDebug() << "QSerialPort: Received byte with value: " << byte;
        }
        if (dataBuffer->size() > 1023) {
            if ( state == 0 ) {
                qDebug() << "Local buffer overflow, dropping input serial port data";
                state = 1;  // over-flowed state
                emit bufferOverflow();
            }
        } else {
            if ( state == 1 ) {
                qDebug() << "Local buffer no-longer overflowing, back to normal";
                state = 0;;
            }
            // stick byte read from device into buffer
            // Mutex needed to make thread safe from buffer read operation
            bufferSem->acquire(1);
            dataBuffer->append(byte);
            bufferSem->release(1);
            emit hasData(); // signal our user that there is data to receive
        }
    }
    CloseHandle(hSerial);
}

#else


//////////////////////////////////////////////////////////
// Linux/Mac/BSD Version of the serial port driver Code
////////////////////////////////////////////////////////

// POSIX C stuff for accessing the serial port
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

// Constructor
QSerialPort::QSerialPort(QObject *parent) :
    QThread(parent)
{
    // make everything in this thread, run in this thread. (Including signals/slots)
    QObject::moveToThread(this);
    // make our data buffer
    dataBuffer = new QByteArray();
    running = true;
    deviceName=NULL;
    bufferMutex = new QMutex(); // control access to buffer
    bufferMutex->unlock();  // not in a locked state
}

QSerialPort::~QSerialPort() {
                running = false;
                close(sfd);
}

//write data to serial port
int QSerialPort::writeBuffer(QByteArray *buffer) {
    if (sfd != 0) {
        // have a valid file discriptor
        return write(sfd, buffer->constData(), buffer->size());
    } else {
        return -1;
    }
}

// setup what device we should use
void QSerialPort::usePort(QString *device_Name, int _buad, int _byteSize, int _stopBits, int _parity) {
    deviceName = new QString(device_Name->toLatin1());
    // serial port settings
    Buad = _buad;
    ByteSize = _byteSize;
    StopBits = _stopBits;
    Parity = _parity;
    MinLen = 1;
}

void QSerialPort::usePort(QString *device_Name, int _buad, int _byteSize, int _stopBits, int _parity, int ml) {
    deviceName = new QString(device_Name->toLatin1());
    // serial port settings
    Buad = _buad;
    ByteSize = _byteSize;
    StopBits = _stopBits;
    Parity = _parity;
    MinLen = ml;
}
// data fetcher, get next byte from buffer
uint8_t QSerialPort::getNextByte() {
    // mutex needed to make thread safe
    bufferMutex->lock(); // lock access to resource, or wait untill lock is avaliable
    uint8_t byte = (uint8_t)dataBuffer->at(0); // get the top most byte
    dataBuffer->remove(0, 1); // remove top most byte
    bufferMutex->unlock();
    return byte; // return top most byte
}

void QSerialPort::resetBuffer() {
    // mutex needed to make thread safe
    bufferMutex->lock(); // lock access to resource, or wait untill lock is avaliable
    dataBuffer->clear();
    bufferMutex->unlock();
    return ;
}

// return number of bytes in receive buffer
uint32_t QSerialPort::bytesAvailable() {
    // this is thread safe, read only operation
    return (uint32_t)dataBuffer->size();
}

// our main code thread
void QSerialPort::run() {
    // thread procedure
    if (_SERIALTHREAD_DEBUG) {
        qDebug() << "QSerialPort: QSerialPort Started..";
        qDebug() << "QSerialPort: Openning serial port " << deviceName->toLatin1();
    }

    // open selected device
    sfd = open( deviceName->toLatin1(), O_RDWR | O_NOCTTY);
    if ( sfd < 0 ) {
		#if _SERIALTHREAD_DEBUG == true
			qDebug() << "QSerialPort: Failed to open serial port " << deviceName->toLatin1();
		#endif

        emit openPortFailed();
        return;  // exit thread
    }

    // Yay we are able to open device as read/write
	#if _SERIALTHREAD_DEBUG == true
		qDebug() << "QSerialPort: Opened serial port " << deviceName->toLatin1() << " Sucessfully!";
	#endif

    // now save current device/terminal settings
    tcgetattr(sfd,&oldtio);
    // setup new terminal settings
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = Buad | ByteSize | StopBits | Parity | CREAD | CLOCAL; // enable rx, ignore flowcontrol
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME]    = 10;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = MinLen;   /* blocking read until atleast 1 charactors received */
    // flush device buffer
    tcflush(sfd, TCIFLUSH);
    // set new terminal settings to the device
    tcsetattr(sfd,TCSANOW,&newtio);
    // ok serial port setup and ready for use

    if (_SERIALTHREAD_DEBUG) {
        qDebug() << "QSerialPort: Serial port setup and ready for use";
        qDebug() << "QSerialPort: Starting QSerialPort main loop";
    }

    // signal we are opened and running
    emit openPortSuccess();

    uint8_t byte; // temp storage byte
    int state=0;    // state machine state

    // start polling loop
    while(running) {
        read(sfd, (void *)&byte, 1); // reading 1 byte at a time..  only 2400 baud.
        // print what we received
        if (_SERIALTHREAD_DEBUG) {
            qDebug() << "QSerialPort: Received byte with value: " << byte;
        }
        if (dataBuffer->size() > 1023) {
            if ( state == 0 ) {
                qDebug() << "Local buffer overflow, dropping input serial port data";
                state = 1;  // over-flowed state
                emit bufferOverflow();
            }
        } else {
            if ( state == 1 ) {
                qDebug() << "Local buffer no-longer overflowing, back to normal";
                state = 0;;
            }
            // stick byte read from device into buffer
            // Mutex needed to make thread safe from buffer read operation
            bufferMutex->lock();
            dataBuffer->append(byte);
            bufferMutex->unlock();
            if(dataBuffer->size()>=MinLen)
                emit hasData(); // signal our user that there is data to receive
        }
    }
    close(sfd);
}

#endif // OS Selection
