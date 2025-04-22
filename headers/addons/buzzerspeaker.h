#ifndef BUZZER_H_
#define BUZZER_H_

#include <vector>
#include <string>
#include "gpaddon.h"

#ifndef BUZZER_ENABLED
#define BUZZER_ENABLED 0
#endif

#ifndef BUZZER_PIN
#define BUZZER_PIN -1
#endif

#ifndef BUZZER_ENABLE_PIN
#define BUZZER_ENABLE_PIN -1
#endif

#ifndef BUZZER_VOLUME
#define BUZZER_VOLUME 100
#endif

// Buzzer Speaker Module
#define BuzzerSpeakerName "BuzzerSpeaker"

enum Tone {
    B0 = 31,
    C1 = 33,
    CS1 = 35,
    DS1 = 39,
    E1 = 41,
    F1 = 44,
    FS1 = 46,
    G1 = 49,
    GS1 = 52,
    A1 = 55,
    AS1 = 58,
    B1 = 62,
    C2 = 65,
    CS2 = 69,
    D2 = 73,
    DS2 = 78,
    E2 = 82,
    F2 = 87,
    FS2 = 93,
    G2 = 98,
    GS2 = 104,
    A2 = 110,
    AS2 = 117,
    B2 = 123,
    C3 = 131,
    CS3 = 139,
    D3 = 147,
    DS3 = 156,
    E3 = 165,
    F3 = 175,
    FS3 = 185,
    G3 = 196,
    GS3 = 208,
    A3 = 220,
    AS3 = 233,
    B3 = 247,
    C4 = 262,
    CS4 = 277,
    D4 = 294,
    DS4 = 311,
    E4 = 330,
    F4 = 349,
    FS4 = 370,
    G4 = 392,
    GS4 = 415,
    A4 = 440,
    AS4 = 466,
    B4 = 494,
    C5 = 523,
    CS5 = 554,
    D5 = 587,
    DS5 = 622,
    E5 = 659,
    F5 = 698,
    FS5 = 740,
    G5 = 784,
    GS5 = 831,
    A5 = 880,
    AS5 = 932,
    B5 = 988,
    C6 = 1047,
    CS6 = 1109,
    D6 = 1175,
    DS6 = 1245,
    E6 = 1319,
    F6 = 1397,
    FS6 = 1480,
    G6 = 1568,
    GS6 = 1661,
    A6 = 1760,
    AS6 = 1865,
    B6 = 1976,
    C7 = 2093,
    CS7 = 2217,
    D7 = 2349,
    DS7 = 2489,
    E7 = 2637,
    F7 = 2794,
    FS7 = 2960,
    G7 = 3136,
    GS7 = 3322,
    A7 = 3520,
    AS7 = 3729,
    B7 = 3951,
    C8 = 4186,
    CS8 = 4435,
    D8 = 4699,
    DS8 = 4978,
    PAUSE = 0
};

struct Song {
    uint16_t toneDuration;
    std::vector<Tone> song;
    Song(uint16_t t, std::vector<Tone> s){
        toneDuration = t;
        song = s;
    };
};

// Buzzer Speaker
class BuzzerSpeakerAddon : public GPAddon
{
public:
    virtual bool available();
    virtual void setup();
    virtual void preprocess() {}
    virtual void process();
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return BuzzerSpeakerName; }
private:
    void processBuzzer();
    void play(Song *song);
    void playIntro();
    void stop();
    uint32_t pwmSetFreqDuty(uint slice, uint channel, uint32_t frequency, float duty);
    uint8_t buzzerPin;
    uint8_t buzzerEnablePin;
    uint8_t buzzerPinSlice;
    uint8_t buzzerPinChannel;
    uint8_t buzzerVolume;
    uint32_t startedSongMils;
    Song *currentSong;
    bool introPlayed;
    bool isSpeakerOn = false;
};

#endif