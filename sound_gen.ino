#include "Note.h"
#include "Freq.h"
#include "SensorUtility.h"
#include "LFO.h"
#include <SparkFun_GridEYE_Arduino_Library.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       bleep;      //xy=125,146
AudioSynthWaveform       bass;      //xy=140,488
AudioEffectEnvelope      bleepEnv;      //xy=347,181
AudioEffectEnvelope      bassEnv;      //xy=390,543
AudioEffectDelay         bleepDel;         //xy=486,372
AudioSynthSimpleDrum     perc;          //xy=559,707
AudioMixer4              bleepDelMix;         //xy=678,250
AudioMixer4              mixer1;         //xy=919,466
AudioOutputI2S           i2s1;           //xy=1067,369
AudioConnection          patchCord1(bleep, bleepEnv);
AudioConnection          patchCord2(bass, bassEnv);
AudioConnection          patchCord3(bleepEnv, bleepDel);
AudioConnection          patchCord4(bleepEnv, 0, bleepDelMix, 0);
AudioConnection          patchCord5(bassEnv, 0, mixer1, 1);
AudioConnection          patchCord6(bleepDel, 0, bleepDelMix, 1);
AudioConnection          patchCord7(perc, 0, mixer1, 2);
AudioConnection          patchCord8(bleepDelMix, 0, mixer1, 0);
AudioConnection          patchCord9(mixer1, 0, i2s1, 0);
AudioConnection          patchCord10(mixer1, 0, i2s1, 1);
// GUItool: end automatically generated code


AudioControlSGTL5000 codec;

float pTime;
float bpTime;
float dTime;

Note bleepNote;
Note bassNote;
Note drumNote;

GridEYE sensor;
int pixels[64];

LFO lfo(1);
LFO lfo2(8);

void setup() {
  Serial.begin(9600);
  Wire.begin();

  bleep.frequency(440);
  bleep.amplitude(1);
  bleepEnv.attack(25);
  bleepEnv.sustain(0);
  bleepEnv.decay(100);
  bleepDel.delay(0, 250);
  bleepDelMix.gain(0, 1);
  bleepDelMix.gain(1, 0.25);

  bass.begin(WAVEFORM_TRIANGLE);
  bass.frequency(440);
  bass.amplitude(1);
  bassEnv.attack(1000);
  bassEnv.sustain(0);
  bassEnv.decay(1000);

  perc.frequency(200);
  perc.length(50);
  perc.pitchMod(0.65);

  mixer1.gain(0, 0.20);
  mixer1.gain(1, 0.20);
  mixer1.gain(2, 0.15);

  AudioMemory(127);

  codec.enable();
  codec.volume(0.25);

  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();

  sensor.begin();

  pTime = millis();
  bpTime = millis();
  dTime = millis();
}

int scale[] = {C, D, E, G, A};

void loop() {
  float cTime = millis();
  if (cTime - pTime >= 1000.0) {    
    bleep.frequency(bleepNote.getRandomNoteFromScale(scale, 4, 7));
    bleepEnv.decay(random(100, 1000));
    bleepEnv.noteOn();
    bleepDel.delay(0, random(100, 300));
    pTime = cTime;
  }

  if (cTime - bpTime >= 3000.0) {
    bass.frequency(bassNote.getRandomNoteFromScale(scale, 2, 4));
    bassEnv.noteOn();
    bpTime = cTime;
  }

  if (cTime - dTime >= 250.0) {
    float avg = getZoneAverage(pixels, zone_perc);
    if (random(100) >= 50 && avg > 23) {

      float freq = 300.0 + (lfo.getValue() * 200);
      perc.frequency(freq);

      if (random(100) >= 75) {
        perc.pitchMod(abs(lfo2.getValue()));
      } else {
        perc.pitchMod(0.65);
      }

      perc.noteOn();
    }
    dTime = cTime;
    
    readSensor();
    outputSerialData(pixels);
  }

  lfo.update();
  lfo2.update();
}

void readSensor() {
  for (int i = 0; i < 64; i++) {
    pixels[i] = sensor.getPixelTemperature(i);
  }
}

