#include "appinclude.h"
#include "globvar.h"
#include "audio.h"




audio::audio(QObject *parent) :
    QObject(0)
{

}


void audio::playAudio(unsigned char msg){
    unsigned char buffer[3];

#ifdef __NO_AUDIO_MSG
        return ;
#endif
    if(!pConfig->userCnf.audioEnable) return;
    if(muteOn) return;

    if(ApplicationDatabase.getDataU(_DB_AUDIO_PRESENT)==0) return;

    buffer[0] = 1; // Codice x riproduzione messaggi audio
    buffer[1] = (unsigned char) msg; // Codice messaggio
    buffer[2] = (unsigned char) pConfig->userCnf.volumeAudio; // Volume
    pConsole->pGuiMcc->sendFrame(MCC_AUDIO,1,buffer,3);
    return;
}

void audio::setMute(bool state){
    muteOn = state;
    if(muteOn) ApplicationDatabase.setData(_DB_AUDIO_MUTE, (unsigned char) 1);
    else       ApplicationDatabase.setData(_DB_AUDIO_MUTE, (unsigned char) 0);

}
