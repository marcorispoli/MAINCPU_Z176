#include "serial_interface.h"
#include "appinclude.h"
#include "globvar.h"
#include "systemlog.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>


/**
 * @brief SerialInterface::SerialInterface
 *
 * This class implements the serial communication protocol with DRTECH device.
 *
 * The Protocol is based on Serial Com RS232, 9600, 8n1
 *
 * See the DRTECH documentation for the protocol description
 */
SerialInterface::SerialInterface(void){


    struct termios options;

    // Create the COM port connection, setting the port with the requested characteristics
    tcgetattr(fd_serialport, &options);
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    //options.c_cflag |= PARODD;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_iflag &= ~ (INPCK | ISTRIP);
    tcsetattr(fd_serialport, TCSANOW, &options);

    fcntl(fd_serialport, F_SETFL, FNDELAY);

    fd_serialport = open("/dev/ttymxc3", O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd_serialport == -1){
        printf("Unable to open /dev/ttymxc3");
    }
}

/**
 * @brief SerialInterface::format2
 *
 * This function encode an unsigned char data into 2 byte strings
 * @param val: is the value to be coded
 * @return
 */
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

/**
 * @brief SerialInterface::format3
 * This function encode an unsigned short data into 3 byte strings
 *
 * @param val this is the input data
 * @return
 */
QByteArray SerialInterface::format3(unsigned short val){
    QByteArray result;
    result.append('0');
    result.append('0');
    result.append('0');

    if(val >= 100){
        unsigned char v = (unsigned char) (val/100);
        result[0] ='0'+v;
        val = val-100*v;
    }

    if(val >= 10){
        unsigned char v = (unsigned char) (val/10);
        result[1] ='0'+v;
        val = val-10*v;
    }

    result[2] = '0'+val;
    return result;
}

/**
 * @brief SerialInterface::format4
 * This function encode an unsigned short data into 4 byte strings
 *
 * @param val
 * @return
 */
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

/**
 * @brief SerialInterface::format4
 * This function encode a float data into 4 byte strings
 *
 * @param val
 * @return
 */
QByteArray SerialInterface::format4(float val){

    if(val >= 100) return format4((unsigned short) val);

    QByteArray result =  QString("%1").arg(val,-4,'f',2,'0').toAscii();
    if(result.size() > 4) result.resize(4);
    return result;
}


/**
 * @brief SerialInterface::sendMessage
 * This function encode the data fields into a protocol frame.
 *
 * The data field (properly encoded) are preppended with the start character,
 * and postpended with the end charcater and the checksum.
 *
 * @param data
 * @return
 */
bool SerialInterface::sendMessage(QByteArray data){

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

    frame.append(data);

    //_______________________
    frame.append(chs); // CHECKSUM

    //_______________________
    frame.append(0x3); // ETX

    int n;
    unsigned char buffer[10];

    // Repeats two times the frame if no ACK is received
    for(int rpt=0; rpt < 2; rpt++){
        int tmo = 100;
        write(fd_serialport, frame.data(),frame.length());
        usleep(5000);

        for(tmo=0; tmo<100; tmo++){
            usleep(1000);
            n = read(fd_serialport, buffer,sizeof(buffer));
            if(n < 0) continue;

            for(int i=0; i<n; i++){
                 if(buffer[i] == 0x06) return true;
            }

        }
    }

    return false;

}
/**
 * @brief SerialInterface::sendExposureData
 * This is the interface function.
 *
 * Application calls this function to encode the imput parameters into a protocol frame sent on RS232.
 *
 * @param kV
 * @param mAs
 * @param large_focus
 * @param filter
 * @param thick
 * @param force
 * @param mA
 * @param time
 * @param mag_factor
 * @param angle
 * @return
 */
bool SerialInterface::sendExposureData(
        unsigned char kV,
        float mAs,
        bool large_focus,
        FilterT filter,
        unsigned short thick,
        unsigned short force,
        unsigned short mA,
        unsigned short time,
        unsigned char mag_factor,
        int angle,
        unsigned char tech){

    QByteArray data;


    // kV
    data.append('0');data.append(':');
    data.append(format2(kV));data.append(';');

    // mAs
    data.append('1');data.append(':');
    data.append(format4(mAs));data.append(';');

    // Thickness (mm)
    data.append('2');data.append(':');
    data.append(format3(thick));data.append(';');

    // Force in N
    data.append('3');data.append(':');
    data.append(format3(force));data.append(';');

    // Tube
    data.append('4');data.append(':');
    if(pConfig->userCnf.tubeFileName.contains("16T")){
        data.append('W');data.append('+');data.append(';');
    }else{
        data.append('M');data.append('o');data.append(';');
    }

    // Filter W, Mo, Rh
    data.append('5');data.append(':');
    if(filter == SI_W){
        data.append('W');data.append(' ');
    }else if(filter == SI_Rh){
        data.append('R');data.append('h');
    }else{
        data.append('M');data.append('o');
    }
    data.append(';');


    // mA
    data.append('6');data.append(':');
    data.append(format3(mA));data.append(';');


    // mS
    data.append('7');data.append(':');
    data.append(format4(time));data.append(';');

    // AEC TECH
    data.append('8');data.append(':');
    if(tech == 0){
        data.append('M');data.append('u');data.append(';');
    }else if(tech == 1){
        data.append('S');data.append('m');data.append(';');

    }else{
        data.append('F');data.append('l');data.append(';');
    }

    // DEN
    data.append('9');data.append(':');
    data.append('+');data.append('0');data.append(';');

    // Focus
    data.append('1');data.append('0');data.append(':');
    if(large_focus){
        data.append('L');data.append('F');
    }else{
        data.append('S');data.append('F');
    }
    data.append(';');

    // Grid
    data.append('1');data.append('1');data.append(':');
    data.append('+');data.append(';');

    // Mag
    unsigned short mag = (unsigned short) mag_factor * 10;
    if(mag == 100) mag = 0;
    data.append('1');data.append('2');data.append(':');
    data.append(format3(mag));data.append(';');

    // Angolo
    data.append('1');data.append('3');data.append(':');
    // if(angle < 0) angle = -1*angle;
    if(angle < 0) data.append('-');
    else data.append('+');
    data.append(format3((unsigned short) abs(angle)));data.append(';');

    // Bucky
    data.append('1');data.append('4');data.append(':');
    data.append('+');data.append(';');

    // Peepok
    data.append('1');data.append('5');data.append(':');
    data.append('0');data.append('0');data.append('.'); data.append('0');data.append('0');data.append(';');

    return sendMessage(data);
}
