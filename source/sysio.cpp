#include "application.h"
#include "appinclude.h"
#include "globvar.h"

sysIO::sysIO(bool master)
{
    isMaster=master;

    if(isMaster) openMaster();
    else openSlave();
}



///////////////////////////////////////////////////////////////////////////
/*
 *  GESTIONE DELLA COMUNICAZIONE IN TUNNEL TRA MASTER E SLAVE
 *
 */
///////////////////////////////////////////////////////////////////////////
void sysIO::openSlave()
{
    int i;

    // Inizializza gli IO in possesso dello Slave che li controlla totalmente
    for(i=0; i<sizeof(_SystemInputs_Str); i++)
        ((unsigned char*) &inputs)[i]=0;

    for(i=0; i<sizeof(_SystemOutputs_Str); i++)
        ((unsigned char*) &outputs)[i]=0;

}

void sysIO::openMaster()
{
    int i;

    for(i=0; i<sizeof(_SystemInputs_Str); i++)
        ((unsigned char*) &inputs)[i]=0;

    for(i=0; i<sizeof(_SystemOutputs_Str); i++)
        ((unsigned char*) &outputs)[i]=0;

    timerOut1 = 0;
    timerOut2 = 0;
    timerOut3 = 0;
    timerOut4 = 0;

}

bool sysIO::setOutputs(ioOutputs out)
{
    unsigned int i;
    unsigned char dato[2*sizeof(_SystemOutputs_Str)];

    // Solo il Master puÃ² aggiornare gli Outputs
    if(!isMaster) return FALSE;

    // Prepara il buffer da spedire
    for(i=0; i<sizeof(_SystemOutputs_Str);i++)
    {
        dato[i] = ((unsigned char*)&out.outputs)[i];
        dato[i+sizeof(_SystemOutputs_Str)] = ((unsigned char*)&out.mask)[i];
    }

    // Manda il pacchetto verso M4
    return pConsole->pGuiMcc->sendFrame(MCC_SET_OUTPUTS, 0, dato, 2*sizeof(_SystemOutputs_Str));

}


// Al momento l'unica uscita impostabile è quella di BURNING!
void sysIO::setSpareOutputPulse(int nout, long time){
    ioOutputs out;

    switch(nout){
        case 1:   // Output BURNING.
            if(timerOut1) killTimer(timerOut1);
            timerOut1 = startTimer(time);
            out.mask.CPU_BURNING=1;
            out.outputs.CPU_BURNING=1;
            break;

    }

    setOutputs(out);

}

void sysIO::setSpareOutputClr(int nout){
    ioOutputs out;

    switch(nout){
        case 1:

            if(timerOut1) killTimer(timerOut1);
            timerOut1 = 0;
            out.mask.CPU_BURNING=1;
            out.outputs.CPU_BURNING=0;
            break;
     }

    setOutputs(out);

}

void sysIO::setXrayLamp(bool stat)
{
    ioOutputs out;
    out.mask.CPU_LMP_SW1=1;

    if(stat) out.outputs.CPU_LMP_SW1=1;
    else out.outputs.CPU_LMP_SW1=0;

    io->setOutputs(out);
    return;
}



void sysIO::timerEvent(QTimerEvent* ev)
{
    ioOutputs out;


    // Estingue la finestra dopo un timeout impostato dall'esterno
    if(ev->timerId()==timerOut1){

        killTimer(timerOut1);
        timerOut1 = 0;
        out.mask.CPU_BURNING=1;
        out.outputs.CPU_BURNING=0;
        setOutputs(out);
        return;
    }


}
