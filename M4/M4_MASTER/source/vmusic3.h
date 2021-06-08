/*

Aut: M. Rispoli
Data di Creazione: 22/09/2014

Data Ultima Modifica:22/09/2014
*/
#ifndef _VMUSIC3_H
#define _VMUSIC3_H

#ifdef ext
#undef ext
#undef extrd
#endif
#ifdef _VMUSIC3_C
  #define ext 
  #define extrd 
#else
  #define ext extern
  #define extrd extern const
#endif



//______________________ SPI ________________________________
#define DIR_SPIWRITE 0
#define DIR_SPIREAD  1
/*
 *  _CS  = J4,19  -> VYB:14 -> PTA18
 *  CLK  = J4,14  -> VYB:169 -> PTB22
 *  MOSI = J4,20  -> VYB:15 -> PTA19
 *  MISO = J4,15  -> VYB:168 -> PTB20
 */
#define SPI_CS   LWGPIO_PIN_PTA18
#define SPI_CLK  LWGPIO_PIN_PTB22
#define SPI_MOSI LWGPIO_PIN_PTA19
#define SPI_MISO LWGPIO_PIN_PTB20
#define SPIDELAY 4 // 10us delay between pulses
ext void spiInit(void);

// __________________ VMUSIC _____________________________________

#define MAXAUDIOMSG 5

ext unsigned char audioMessagesQueue[MAXAUDIOMSG];
ext unsigned char audioVolumeQueue[MAXAUDIOMSG];


ext void vmTask(uint32_t initial_data);
ext int  vmFlush(char* buffer, int maxlen, bool format,int attempt);
ext void vmPrint(bool ascii,int attempt);
ext bool vmSync(void);
ext bool vmShortMsg(void);
ext bool vmPlay(unsigned char msgcode, unsigned char volume);
ext bool vmInit(void);
ext bool verifyVmPresence(void);
ext bool vmDir(void);
ext bool vmChar(void);
ext bool vmPrompt(void);
ext bool vmQP2(void);
ext bool vmDiskLabel(void);
ext bool vmDiskInfo(void);
ext void ioTestO(void);
ext void ioTestI(void);
ext void setMosi(unsigned char val);
ext void pulseMosi(void);
ext int spiRead(char *pSpiData);
ext int spiWrite(char *pSpiData);
#endif
