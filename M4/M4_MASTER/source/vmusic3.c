#define _VMUSIC3_C
#include "dbt_m4.h"
#include "vmusic3.h"

// Imposta un ritardo di circa 10 us
void _10u_time_delay(uint32_t time){
    time = time * 1200;
    uint32_t hw;
    uint32_t oldhw = _time_get_hwticks();
    uint32_t diff = 0;

    while(time > diff){
        hw = _time_get_hwticks();
        diff = hw>oldhw?(hw-oldhw):((uint32_t) _time_get_hwticks_per_tick()-oldhw+hw);
    }
}



//////////////////////////////////////////////////////////////////////////////
/*

Autore: M. Rispoli
Data: 22/06/2015
*/
//////////////////////////////////////////////////////////////////////////////
void vmTask(uint32_t taskRegisters)
{
   printf("VMUSIC DRIVER STARTED \n");
   generalConfiguration.audioInitiated = 1;

   // Inizializzazione SPI
   spiInit();

   // Verifica presenza device
   if(verifyVmPresence() == false){
       generalConfiguration.audioInitiated =0;
       printf("VM3 INTERFACE NOT DETECTED!\n");
   }else{

       // Init Driver: da qui esce solo se il sistema è inizializzato
       int attempt = 15;
       while(!vmInit()){
           _time_delay(2000);
           attempt--;
           if(attempt==0){
               generalConfiguration.audioInitiated =0;
               printf("USB BOMS NOT AVAILABLE! VM3 DISABLED!\n");
               break;
           }
       }

   }

   if(generalConfiguration.audioInitiated){
        printf("USB BOMS FOUND! DIRECTORY CONTENT:\n");
        vmDir();
   }

   // Avverte la gui dell'avvenuta inizializzazione
   unsigned char buffer[1];
   if(generalConfiguration.audioInitiated) buffer[0] = 1;
   else buffer[0] = 0;
   while(1){
           if(mccGuiNotify(1,MCC_AUDIO,buffer,1)==true) break;
           _time_delay(1000);
   }


   // Azzeramento coda
   for(int i=0; i<MAXAUDIOMSG; i++)
       audioMessagesQueue[i]=0;

   _EVCLR(_EV0_VMUSIC_EVENT);
   while(1)
   {
        _EVWAIT_ALL(_EV0_VMUSIC_EVENT);

        // Analizza l'evento
        for(int i=0; i<MAXAUDIOMSG; ){
            if(audioMessagesQueue[i]!=0){
                unsigned char msg= audioMessagesQueue[i];
                unsigned char vol= audioVolumeQueue[i];
                printf("AUDIO MESSAGE:%d\n", msg);
                audioMessagesQueue[i] = 0;
                vmPlay(msg,vol);
                i=-1;
            }
            i++;
        }
        _EVCLR(_EV0_VMUSIC_EVENT);

   }
}






// ___________________________ SPI COMMANDS _________________________________________________________________________

#define ACTIVE      LWGPIO_VALUE_HIGH
#define NOT_ACTIVE  LWGPIO_VALUE_LOW

void CLK_ONOFF(void);
void CS_ON(void);
void spiDelay(void);

LWGPIO_STRUCT spics;
LWGPIO_STRUCT spiclk;
LWGPIO_STRUCT spimosi;
LWGPIO_STRUCT spimiso;

#define MOSI_1 {lwgpio_set_value(&spimosi, ACTIVE);spiDelay();}
#define MOSI_0 {lwgpio_set_value(&spimosi, NOT_ACTIVE);spiDelay();}
#define GETMISO lwgpio_get_value(&spimiso)

int GET_STATUS(void){
    int val;
    lwgpio_set_value(&spiclk, ACTIVE); spiDelay();
    val = lwgpio_get_value(&spimiso);
    lwgpio_set_value(&spiclk, NOT_ACTIVE); spiDelay();
    return val;
}

void CLK_ONOFF(void){
    lwgpio_set_value(&spiclk, ACTIVE); spiDelay();
    lwgpio_set_value(&spiclk, NOT_ACTIVE); spiDelay();
}

void CS_ON(void){
    lwgpio_set_value(&spics, ACTIVE); spiDelay();
}
void CS_OFF(void){
    lwgpio_set_value(&spics, NOT_ACTIVE); spiDelay();
}

void spiInit(void)
{
    printf("INIZIALIZZAZIONE GPIO\n");
    if (lwgpio_init(&spics,SPI_CS,LWGPIO_DIR_OUTPUT,NOT_ACTIVE) != TRUE)
     {
       printf("SPI CS ERROR");
       _mqx_exit(-1);
     }
    lwgpio_set_functionality(&spics, 0);
    lwgpio_set_attribute(&spics, LWGPIO_ATTR_DRIVE_STRENGTH,1);

    if (lwgpio_init(&spiclk,SPI_CLK,LWGPIO_DIR_OUTPUT,NOT_ACTIVE) != TRUE)
     {
       printf("SPI CLK ERROR");
       _mqx_exit(-1);
     }
    lwgpio_set_functionality(&spiclk, 0);
    lwgpio_set_attribute(&spiclk, LWGPIO_ATTR_DRIVE_STRENGTH,1);

    if (lwgpio_init(&spimosi,SPI_MOSI,LWGPIO_DIR_OUTPUT,NOT_ACTIVE) != TRUE)
     {
       printf("SPI CLK ERROR");
       _mqx_exit(-1);
     }

    lwgpio_set_functionality(&spimosi, 0);
    lwgpio_set_attribute(&spimosi, LWGPIO_ATTR_PULL_UP,1);

    // Impostazione MISO con Pulldown per poter identificare la presenza della scheda di interfaccia
    if (lwgpio_init(&spimiso,SPI_MISO,LWGPIO_DIR_INPUT,LWGPIO_VALUE_NOCHANGE) != TRUE)
     {
       printf("SPI MISO ERROR");
       _mqx_exit(-1);
     }

    lwgpio_set_functionality(&spimiso, 0); // GPIO MODE
    lwgpio_set_attribute(&spimiso, LWGPIO_ATTR_DRIVE_STRENGTH,1);
    lwgpio_set_attribute(&spimiso, LWGPIO_ATTR_PULL_DOWN,LWGPIO_AVAL_ENABLE); // 100k Pull down

    lwgpio_set_value(&spics, NOT_ACTIVE); // CS = LOW
    lwgpio_set_value(&spimosi, NOT_ACTIVE); // MOSI = LOW
    lwgpio_set_value(&spiclk, NOT_ACTIVE); // CLK = LOW

}


void spiDelay(void){
    _10u_time_delay(SPIDELAY);
}


//***************************************************************************
// Name: spiXfer
//
// Description: Read or write transfer on SPI bus.
//
// Parameters: spiDirection - DIR_SPIWRITE/DIR_SPIREAD
//             pSpiData - pointer to data byte to write or read to
//
// Returns: retData - XFER_OK for success, XFER_RETRY to retry.
//
// Comments: Bit bangs SPI interface pins.
//
//***************************************************************************
int spiRW(int spiDirection, char *pSpiData)
{
    unsigned char retData;
    unsigned char bitData;
    MOSI_0;

    // CS goes high to enable SPI communications
    CS_ON();

    // Clock 1 - Start State
    MOSI_1;
    CLK_ONOFF();

    // Clock 2 - Direction
    if(spiDirection) {MOSI_1;}
    else {MOSI_0; }
    CLK_ONOFF();

    // Clock 3 - Address
    MOSI_0;
    CLK_ONOFF();

    // Clocks 4..11 - Data Phase
    bitData = 0x80;
    if (spiDirection)
    {
        // read operation
        retData = 0;

        while (bitData)
        {
            spiDelay();
            if(GETMISO == ACTIVE) retData|=bitData;
            CLK_ONOFF();
            bitData = bitData >> 1;
        }

        *pSpiData = retData;
    }
    else
    {
        // write operation
        retData = *pSpiData;

        while (bitData)
            {
            spiDelay();
            if(retData & bitData) {MOSI_1}
            else {MOSI_0}
            CLK_ONOFF();
            bitData = bitData >> 1;
        }
    }

    // Clock 12 - Status bit
    spiDelay();
    if(GETMISO == ACTIVE) bitData = 1;
    else bitData = 0;
    CLK_ONOFF();

    // CS goes high to enable SPI communications
    CS_OFF();

    // Clock 13 - CS low
    CLK_ONOFF();

    MOSI_0;
    // Test of data trasfer success
    /*
    if(!bitData){
        if (spiDirection) printf("NEW DATA READ=%x\n", retData);
        else printf("WRITW DATA OK\n");
    }*/
    return bitData;
}




bool vmWrite(char spiData){
    int attempt = 1000;
    while(attempt--){
        if(spiRW(DIR_SPIWRITE, &spiData)==0) return true;
        else spiDelay();
    }

    return false;
}

// Svuota i dati non letti dal player
int vmFlush(char* buffer, int maxlen, bool format,int attempt){
    char byte;
    int idx=0;

    if(buffer==0) return 0;
    if(maxlen==0) return 0;

    int repeat=attempt;
    while(repeat--){
        if(spiRW(DIR_SPIREAD, &byte)){
            _time_delay(10);
        }else{
            repeat=attempt;
            buffer[idx] = byte;
            idx++;
            if(idx == maxlen) break;
        }
    }

    return idx;
}

void vmPrint(bool ascii,int attempt){
    char buffer[100];
    int len;
    int max_print = 10; // Massimo numero di blocchi leggibili

    while(max_print){

        len = vmFlush(buffer,90,ascii,attempt);
        int i=0;
        if(ascii)
            while(i < len){
                if(!((buffer[i] < 32)||( buffer[i] > 126))) printf("%c",buffer[i]);
                i++;
            }
        else
            while(i < len){
                if(!((buffer[i] < 32)||( buffer[i] > 126))) printf("%x ",buffer[i]);
                i++;
            }

        if(len<90) break;
        max_print--;
    }
    printf("\n");
}


bool vmDir(void){

    if(!vmWrite(1))     return false;
    if(!vmWrite(0xD))   return false;
    _time_delay(100);
    vmPrint(true,300);
    return true;
}

bool vmPrompt(void){
    int len;
    char buffer[10];

    if(!vmWrite(0xD))       return false;
    len = vmFlush(buffer,10,false,100);

    for(int j=0; j<len-1; j++){
        if((buffer[j]=='N')&&(buffer[j+1]=='D')) return false;
        if((buffer[j]=='>')&&(buffer[j+1]==0xD)) return true;
    }

    return false;
}

bool vmDiskLabel(void){

    if(!vmWrite(0x2E))      return false;
    if(!vmWrite(0xD))       return false;

    vmPrint(true,200);
    return true;
}

bool vmDiskInfo(void){

    if(!vmWrite(0x0F))      return false;
    if(!vmWrite(0xD))       return false;

    vmPrint(true,300);
    return true;
}

bool vmChar(void){

    if(!vmWrite(0x90))       return false;
    if(!vmWrite(0xD))       return false;
    vmPrint(false,100);
    return true;
}

bool vmQP2(void){

    if(!vmWrite(0x2C))       return false;
    if(!vmWrite(0xD))       return false;
    vmPrint(false,300);
    return true;
}


bool vmAnswer(char* chk, int dimchk, int attempt){
    char buffer[10];
    int len,i,j;

    len = vmFlush(buffer,10,false,attempt);

    if(len<dimchk) return false;

    for(i=0; i<len-dimchk+1; i++){
        for(j=0;j<dimchk;j++){
            if(buffer[i+j]!=chk[j]) break;
        }
        if(j==dimchk) return true;
    }
    return false;

}

bool vmSync(void){
    char chk[2];
    chk[0]='E';
    chk[1]=0xD;

    if(!vmWrite('E'))       return false;
    if(!vmWrite(0xD))       return false;
    _time_delay(100);
    return vmAnswer(chk,2,200);


}

// Messaggi in formato SHORT
bool vmShortMsg(void){
    char chk[2];
    chk[0]='>';
    chk[1]=0xD;

    if(!vmWrite(0x10))      return false;
    if(!vmWrite(0xD))       return false;
    _time_delay(100);
    return vmAnswer(chk,2,50);
}

bool vmPlay(unsigned char msgcode, unsigned char volume){

    // Imposta il volume
    if(!vmWrite(0x88))      return false;
    if(!vmWrite(0x20))       return false;
    if(!vmWrite(volume))       return false;
    if(!vmWrite(0x0D))       return false;
    vmPrint(false,10);

    // Attiva il comando
    if(!vmWrite(0x1D))      return false;
    if(!vmWrite(0x20))       return false;

    if(msgcode>9) {
        if(!vmWrite('0'+msgcode/10))       return false;
    }
    if(!vmWrite('0'+msgcode%10))       return false;
    if(!vmWrite('.'))       return false;
    if(!vmWrite('m'))       return false;
    if(!vmWrite('p'))       return false;
    if(!vmWrite('3'))       return false;
    if(!vmWrite(0x0D))      return false;
    vmPrint(true,100);
    _time_delay(5000);

    // Stop Playback
    //vmWrite(0x20);
    //vmWrite(0x0D);
    vmPrint(true,10);

    return true;
}

bool verifyVmPresence(void){
    CS_OFF();
    _time_delay(100);

    for(int i=0; i<10; i++){
        if(GETMISO == LWGPIO_VALUE_HIGH) return true;
        _time_delay(100);
    }

    return false;
}

bool vmInit(void){

    printf("VM3 INIZIALIZZATION \n");

    // Verifica la presenza della scheda di interfaccia
    // Se l'ingresso MISO risulta basso allora significa che non c'è
    // vmusic collegato.


    // Scarica le scritte varie
    vmPrint(true,100);

    if(!vmShortMsg()) printf("SHORT MSG MODE SETTING NOK\n");
    else  printf("SHORT MSG MODE SETTING OK\n");


    if(vmPrompt()) return true;
    else return false;
}

#define IOTEST LWGPIO_PIN_PTB19
LWGPIO_STRUCT pin;
void ioTestO(void)
{
    static bool init = false;

    if(!init){
        init = true;
        printf("INIZIALIZZAZIONE IOTEST\n");
        if (lwgpio_init(&pin,IOTEST,LWGPIO_DIR_OUTPUT,ACTIVE) != TRUE)
         {
           printf("INIT IO ERR");
           _mqx_exit(-1);
         }
        lwgpio_set_functionality(&pin, 0);
        lwgpio_set_attribute(&pin, LWGPIO_ATTR_DRIVE_STRENGTH,1);
    }

    _time_delay(100);

    // 10 oscillazioni lente
    int i=10;
    while(i--){
        lwgpio_set_value(&pin, ACTIVE);
        _time_delay(20);
        lwgpio_set_value(&pin, NOT_ACTIVE);
        _time_delay(20);
    }
    return;
}

void ioTestI(void)
{
    static bool init = false;
    static bool stat=false;

    if(!init){
        init = true;
        printf("INIZIALIZZAZIONE IOTEST\n");
        if (lwgpio_init(&pin,IOTEST,LWGPIO_DIR_INPUT,LWGPIO_VALUE_NOCHANGE) != TRUE)
         {
            printf(" IN ERROR");
           _mqx_exit(-1);
         }

        lwgpio_set_functionality(&pin, 0); // GPIO MODE
        lwgpio_set_attribute(&pin, LWGPIO_ATTR_DRIVE_STRENGTH,1);
        if(lwgpio_get_value(&pin))   stat = true;
        else stat = false;
        printf("STAT = %d", stat);
    }

    _time_delay(100);


    bool locstat=stat;
    while(1){

        if(lwgpio_get_value(&pin))   locstat= true;
        else locstat= false;
        if(stat!=locstat){
            stat = locstat;
            printf("STAT = %d", stat);
        }

        _time_delay(500);
    }
    return;
}



void setMosi(unsigned char val){
    if(val){ MOSI_1;}
    else {   MOSI_0;}
}

void pulseMosi(void){
    MOSI_1;
    MOSI_0;
}


int spiRead(char *pSpiData)
{
    unsigned char retData;
    unsigned char bitData;

    MOSI_0;

    // CS goes high to enable SPI communications
    CS_ON();

    // Clock 1 - Start State
    MOSI_1;
    CLK_ONOFF();

    // Clock 2 -Read  Direction
    MOSI_1;
    CLK_ONOFF();

    // Clock 3 - Address
    MOSI_0; // MOSI_1
    CLK_ONOFF();

    // Clocks 4..11 - Data Phase
    bitData = 0x80;

    // read operation
    retData = 0;

    while (bitData)
    {
        spiDelay();
        if(GETMISO == ACTIVE) retData|=bitData;
        CLK_ONOFF();
        bitData = bitData >> 1;
    }

    *pSpiData = retData;

    // Clock 12 - Status bit
    spiDelay();
    if(GETMISO == ACTIVE) bitData = 1;
    else bitData = 0;
    CLK_ONOFF();

    // CS goes high to enable SPI communications
    CS_OFF();

    // Clock 13 - CS low
    CLK_ONOFF();

    MOSI_0;
    // Test of data trasfer success
    return bitData;
}

int spiWrite(char *pSpiData)
{
    unsigned char retData;
    unsigned char bitData;

    MOSI_0;

    // CS goes high to enable SPI communications
    CS_ON();

    // Clock 1 - Start State
    MOSI_1;
    CLK_ONOFF();

    // Clock 2 - Direction
    MOSI_0;
    CLK_ONOFF();

    // Clock 3 - Address
    MOSI_0;
    CLK_ONOFF();

    // Clocks 4..11 - Data Phase
    bitData = 0x80;

    // write operation
    retData = *pSpiData;

    while (bitData)
    {
        spiDelay();
        if(retData & bitData) {MOSI_1}
        else {MOSI_0}
        CLK_ONOFF();
        bitData = bitData >> 1;
    }
    MOSI_0;

    // Clock 12 - Status bit
    if(GET_STATUS() == ACTIVE) bitData = 1;
    else bitData =0;

    // CS goes high to enable SPI communications
    CS_OFF();

    // Clock 13 - CS low
    CLK_ONOFF();

    MOSI_0;
    CLK_ONOFF();


    return bitData;
}
