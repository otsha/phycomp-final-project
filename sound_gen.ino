#include "Note.h"
#include "Freq.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       bleep;      //xy=160,256
AudioSynthWaveform       bass;      //xy=189,524
AudioEffectEnvelope      bleepEnv;      //xy=408,275
AudioEffectEnvelope      bassEnv;      //xy=459,500
AudioMixer4              mixer1;         //xy=812,374
AudioOutputI2S           i2s1;           //xy=1067,369
AudioConnection          patchCord1(bleep, bleepEnv);
AudioConnection          patchCord2(bass, bassEnv);
AudioConnection          patchCord3(bleepEnv, 0, mixer1, 0);
AudioConnection          patchCord4(bassEnv, 0, mixer1, 1);
AudioConnection          patchCord5(mixer1, 0, i2s1, 0);
AudioConnection          patchCord6(mixer1, 0, i2s1, 1);
// GUItool: end automatically generated code

AudioControlSGTL5000 codec;

float pTime;
float bpTime;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  bleep.frequency(440);
  bleep.amplitude(1);
  bleepEnv.attack(25);
  bleepEnv.sustain(0);
  bleepEnv.decay(100);

  bass.begin(WAVEFORM_SQUARE);
  bass.frequency(440);
  bass.amplitude(1);
  bassEnv.attack(1000);
  bassEnv.sustain(0);
  bassEnv.decay(1000);

  mixer1.gain(0, 0.20);
  mixer1.gain(1, 0.15);

  AudioMemory(18);

  codec.enable();
  codec.volume(0.25);

  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();

  pTime = millis();
  bpTime = millis();
}

int scale[] = {C, D, E, G, A};

void loop() {
  float cTime = millis();
  if (cTime - pTime >= 1000.0) {
    bleep.frequency(getRandomNote(4, 7));
    bleepEnv.decay(random(100, 1000));
    bleepEnv.noteOn();
    pTime = cTime;
  }

  if (cTime - bpTime >= 3000.0) {
    bass.frequency(getRandomNote(2, 4));
    bassEnv.noteOn();
    bpTime = cTime;
  }
}

float getRandomNote(int minOct, int maxOct) {
  int scaleIndex = round(random(0, 4));
  float noteFreq = scale[scaleIndex] / 1000.0;
  Note n(noteFreq);
  return n.oct(random(minOct, maxOct));
}


