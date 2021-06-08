#ifndef _I2C_H
#define _I2C_H

#ifdef ext
    #undef ext
    #undef extrd
#endif
#ifdef _I2C_C
  #define ext
  #define extrd
#else
  #define ext extern
  #define extrd extern const
#endif


ext void i2cInit();
ext bool i2cPing(unsigned char addr);
ext bool i2cRecvMessage(unsigned char addr, unsigned char reg, unsigned char* data, int size);
ext bool i2cSendMessage(unsigned char addr, unsigned char reg, unsigned char* data, int size);


#endif // _I2C_H
