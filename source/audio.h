#ifndef AUDIO_H
#define AUDIO_H

#include "application.h"


class audio : public QObject
{
    Q_OBJECT
public:
    explicit audio(QObject *parent = 0);



signals:

public:
    #define AUDIO_READY_FOR_EXPOSURE                                    1

    #define AUDIO_NOT_READY_FOR_EXPOSURE_MISSING_CASSETTE               2
    #define AUDIO_NOT_READY_FOR_EXPOSURE_REMOVE_CASSETTE                3
    #define AUDIO_NOT_READY_FOR_EXPOSURE_CASSETTE_ALREADY_EXPOSED       4
    #define AUDIO_NOT_READY_UNDETECTED_COMPRESSOR                       5
    #define AUDIO_NOT_READY_UNLOCKED_COMPRESSOR                         6
    #define AUDIO_NOT_READY_REMOVE_COMPRESSOR                           7  
    #define AUDIO_NOT_READY_APPLY_COMPRESSION                           8
    #define AUDIO_NOT_READY_UNDETECTED_POTTER                           9
    #define AUDIO_NOT_READY_REMOVE_POTTER                               10
    #define AUDIO_NOT_READY_OPEN_DOOR                                   11
    #define AUDIO_NOT_READY_SYSTEM_ALARM                                12
    #define AUDIO_NOT_READY_PC_NOT_READY                                13
    #define AUDIO_NOT_READY_INVALID_POTTER                              14
    #define AUDIO_NOT_READY_INVALID_COMPRESSOR                          15
    #define AUDIO_NOT_READY_DETECTOR_NOT_CALIBRATED                     16

    #define AUDIO_NOT_READY_TUBE_TEMP_ALARM                             17
    #define AUDIO_MAX_NUM_MESSAGES                                      17
public slots:
    void playAudio(unsigned char msg);
    void setMute(bool state);
private:
    bool muteOn; // Funzione mute attivA
};

#endif // AUDIO_H
