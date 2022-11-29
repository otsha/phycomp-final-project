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
AudioSynthSimpleDrum     perc;          //xy=382,648
AudioEffectEnvelope      bassEnv;      //xy=390,543
AudioEffectDelay         bleepDel;         //xy=486,372
AudioSynthKarplusStrong  pluck;        //xy=564,723
AudioFilterStateVariable bassFlt;        //xy=606,501
AudioMixer4              bleepDelMix;         //xy=678,250
AudioMixer4              mixer1;         //xy=919,466
AudioOutputI2S           i2s1;           //xy=1116,369
AudioConnection          patchCord1(bleep, bleepEnv);
AudioConnection          patchCord2(bass, bassEnv);
AudioConnection          patchCord3(bleepEnv, bleepDel);
AudioConnection          patchCord4(bleepEnv, 0, bleepDelMix, 0);
AudioConnection          patchCord5(perc, 0, mixer1, 2);
AudioConnection          patchCord6(bassEnv, 0, bassFlt, 0);
AudioConnection          patchCord7(bleepDel, 0, bleepDelMix, 1);
AudioConnection          patchCord8(pluck, 0, mixer1, 3);
AudioConnection          patchCord9(bassFlt, 0, mixer1, 1);
AudioConnection          patchCord10(bleepDelMix, 0, mixer1, 0);
AudioConnection          patchCord11(mixer1, 0, i2s1, 0);
AudioConnection          patchCord12(mixer1, 0, i2s1, 1);
// GUItool: end automatically generated code




AudioControlSGTL5000 codec;

float currentTime;
float blinkPreviousTime;
float pluckPreviousTime;
float bassPreviousTime;
float percPreviousTime;

Note bleepNote;
Note pluckNote;
Note bassNote;
Note drumNote;

GridEYE sensor;
int pixels[64];

LFO bassLFO(4);
LFO percLFO(1);
LFO percLFO2(8);

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
  bassFlt.frequency(15000);
  bassFlt.resonance(0.7);

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

  blinkPreviousTime = millis();
  pluckPreviousTime = millis();
  bassPreviousTime = millis();
  percPreviousTime = millis();
  currentTime = millis();
}

int pentatonicMajor[5] = {C, D, E, G, A};
int pentatonicMinor[5] = {A, B, C, E, G};
int scale[5];

void setScale(int newScale[]) {
  for (int i = 0; i < 5; i++) {
    scale[i] = newScale[i];
  }
}

void loop() {
  currentTime = millis();

  if (getZoneAverage(pixels, zone_scale) > 25) {
    setScale(pentatonicMajor);
  } else {
    setScale(pentatonicMinor);
  }

  playBlink();
  playPluck();
  playBass();

  if (currentTime - percPreviousTime >= 250.0) {
    float avg = getZoneAverage(pixels, zone_perc);
    if (random(100) >= 50 && avg > 23) {
      playPerc();
    }
    percPreviousTime = currentTime;
    
    readSensor();
    //outputSerialData(pixels);
  }

  bassLFO.update();
  bassFlt.resonance(2 + (1.25 * bassLFO.getValue()));

  percLFO.update();
  percLFO2.update();
}

void playBlink() {
  if (currentTime - blinkPreviousTime >= 1000.0) {    
    bleep.frequency(bleepNote.getRandomNoteFromScale(scale, 4, 7));
    bleepEnv.decay(random(100, 1000));
    bleepEnv.noteOn();
    bleepDel.delay(0, random(100, 300));
    blinkPreviousTime = currentTime;
  }
}

void playPluck() {
  if (currentTime - pluckPreviousTime >= 500.0) {
    float avg = getZoneAverage(pixels, zone_pluck_vel);
    if (avg > 23) {
      pluck.noteOn(
        pluckNote.getRandomNoteFromScale(scale, 5, 6),
        map(avg, 23, 42, 0.0, 1.0)
      );
    }
    pluckPreviousTime = currentTime;
  }
}

void playBass() {
  if (currentTime - bassPreviousTime >= 3000.0) {
    bass.frequency(bassNote.getRandomNoteFromScale(scale, 2, 4));
    float avg = getZoneAverage(pixels, zone_bass);
    if (avg > 23) {
      bass.amplitude(0.75);
      bass.begin(WAVEFORM_SAWTOOTH);
      bassFlt.frequency(1300);
    } else {
      bass.amplitude(1);
      bass.begin(WAVEFORM_TRIANGLE);
      bassFlt.frequency(15000);
    }
    bassEnv.noteOn();
    bassPreviousTime = currentTime;
  }
}

void playPerc() {
  float freq = 300.0 + (percLFO.getValue() * 200);
  perc.frequency(freq);

  if (random(100) >= 75) {
    perc.pitchMod(abs(percLFO2.getValue()));
  } else {
    perc.pitchMod(0.65);
  }

  perc.noteOn();  
}

void readSensor() {
  for (int i = 0; i < 64; i++) {
    pixels[i] = sensor.getPixelTemperature(i);
  }
}

