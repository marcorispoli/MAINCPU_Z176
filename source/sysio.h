#ifndef SYSIO_H
#define SYSIO_H

#include "application.h"

class ioOutputs
{
public:
    ioOutputs(void) { memset((unsigned char*) &outputs,0,sizeof(_SystemOutputs_Str));memset((unsigned char*) &mask,0,sizeof(_SystemOutputs_Str));}

    _SystemOutputs_Str outputs;
    _SystemOutputs_Str mask;

};


class sysIO: public QObject
{
    Q_OBJECT

public:
    sysIO(bool master);


signals:

    // Segnali emessi sia su master che su slave alla ricezione di Inputs cambiati
//    void xrayReqSig(bool stat);                 // Attivazione pulsante raggi
//    void mainsOnSgn(bool stat);                 // Segnale di MAINS ON
//    void pcb240ErrorSig(unsigned char errno);   // Errore da PCB240
//    void pcb240FlagsSig(unsigned char flags);   // Flags da PCB240
//    void ioUpdated(void);                       // Segnale di avvenuto update
//    void inputsChanged(void);                   // Segnale di ricezione input cambiati da M$ (solo quelli previsti utili per GUI)


public slots:
 //   void manageInputs(QByteArray dato);             // Slot di ricezione Input da M4 MASTER
 //   void manageIO(QByteArray dato);                 // Slot per ricevere glio IO cambiati
    void timerEvent(QTimerEvent* ev);               // Override della classe QObject

public:
    void openMaster();
    void openSlave();
//    void updateOutputs(void); // Invia a m4 SLAVE il contenuto degli outputs
    bool setOutputs(ioOutputs out);
    void setXrayLamp(bool stat);
//    bool updateIo(void);

    void setSpareOutputPulse(int nout, long time);
    void setSpareOutputClr(int nout);

    _SystemInputs_Str    inputs;
    _SystemOutputs_Str   outputs;

 //   bool pcb240PwrMonitorEna; // Abilitazione al controllo delle tensioni di BUS

  private:
    int timerOut1;
    int timerOut2;
    int timerOut3;
    int timerOut4;

    bool isMaster;
};

#endif // SYSIO_H
