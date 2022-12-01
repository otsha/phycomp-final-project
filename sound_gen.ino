#include "Note.h"
#include "Freq.h"
#include "SensorUtility.h"
#include "LFO.h"
#include "Clock.h"
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
AudioFilterStateVariable bassFlt;        //xy=677,384
AudioMixer4              bleepDelMix;         //xy=678,250
AudioMixer4              mixer1;         //xy=903,408
AudioEffectBitcrusher    bitcrusher1;    //xy=1077,406
AudioOutputI2S           i2s1;           //xy=1281,409
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
AudioConnection          patchCord11(mixer1, bitcrusher1);
AudioConnection          patchCord12(bitcrusher1, 0, i2s1, 0);
AudioConnection          patchCord13(bitcrusher1, 0, i2s1, 1);
// GUItool: end automatically generated code


AudioControlSGTL5000 codec;
GridEYE sensor;
int pixels[64];

Clock clk;

Note bleepNote;
Note pluckNote;
Note bassNote;
Note drumNote;

LFO bassLFO(4);
LFO percLFO(1);
LFO percLFO2(8);

void setup() {
  Serial.begin(9600);
  Wire.begin();

  bleep.begin(WAVEFORM_SINE);
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
  bassFlt.resonance(2.5);

  perc.frequency(200);
  perc.length(50);
  perc.pitchMod(0.65);

  // Set clock intervals
  clk.setInterval(0, 1000.0); // bleep
  clk.setInterval(1, 3000.0); // bass
  clk.setInterval(2, 250.0); // perc
  clk.setInterval(3, 250.0); // pluck
  clk.setInterval(7, 250.0); // IR sensor read interval

  mixer1.gain(0, 0.15);
  mixer1.gain(1, 0.20);
  mixer1.gain(2, 0.25);
  mixer1.gain(3, 1);

  bitcrusher1.bits(16);

  AudioMemory(127);

  codec.enable();
  codec.volume(0.25);

  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();

  sensor.begin();
}

int pentatonicMajor[5] = {C, D, E, G, A};
int pentatonicMinor[5] = {A, B, C, E, G};
int chaos[5] = {C, CSharp, E, F, G};
int scale[5];

void setScale(int newScale[]) {
  for (int i = 0; i < 5; i++) {
    scale[i] = newScale[i];
  }
}

void loop() {  
  clk.update();

  setScale(pentatonicMajor);
  
  // Scale shifting
  if (getZoneAverage(pixels, zone_scale) > threshold_warm) {
    setScale(pentatonicMajor);
  } else {
    setScale(pentatonicMinor);
  }

  // Bit crushing
  float sensorAvg = getSensorAverage(pixels);
  float crushFactor = 16 - map(sensorAvg, threshold_warmer, 35, 0.0, 1.0) * 14;
  bitcrusher1.bits(max(4, crushFactor));

  // Play instruments
  playBlink();
  playPluck();
  playBass();
  playPerc();

  // Read (and output) sensor
  if (clk.counterOver(7)) {
    readSensor();
    outputSerialData(pixels);
    clk.setPrevious(7);
  }

  // LFO Stuff
  bassLFO.update();
  bassFlt.frequency(2000 + (1000 * bassLFO.getValue()));

  percLFO.update();
  percLFO2.update();
}

void playBlink() {
  if (clk.counterOver(0) == true) {    
    bleep.frequency(bleepNote.getRandomNoteFromScale(scale, 4, 6));
    
    if (getZoneAverage(pixels, zone_bleep_interval) > threshold_warm) {
      clk.setInterval(0, 333.33);
      bleepEnv.attack(10);
      bleepEnv.decay(50);
    } else {
      clk.setInterval(0, 1000.0);
      bleepEnv.attack(random(10, 500));
      bleepEnv.decay(random(100, 2000));
    }

    if (random(100) > 40) {
      bleepDel.delay(0, random(100, 300));
      bleepEnv.noteOn();
    }
    clk.setPrevious(0);
  }
}

void playBass() {
  if (clk.counterOver(1) == true) {
    bass.frequency(bassNote.getRandomNoteFromScale(scale, 2, 4));
    float avg = getZoneAverage(pixels, zone_bass);
    if (avg > threshold_warm) {
      bass.amplitude(0.75);
      bass.begin(WAVEFORM_SAWTOOTH);
      bassFlt.frequency(1300);
    } else {
      bass.amplitude(1);
      bass.begin(WAVEFORM_TRIANGLE);
      bassFlt.frequency(15000);
    }
    bassEnv.noteOn();
    clk.setPrevious(1);
  }
}

void playPerc() {
  if (clk.counterOver(2)) {
    if (random(100) >= 50 && getZoneAverage(pixels, zone_perc) > threshold_warm) {
      float freq = 300.0 + (percLFO.getValue() * 200);
      perc.frequency(freq);

      if (random(100) >= 75) {
        perc.pitchMod(abs(percLFO2.getValue()));
      } else {
        perc.pitchMod(0.65);
      }

      if (pixels[60] > threshold_warmer) {
        perc.length(random(100, 150));
      } else {
        perc.length(50);
      }

      perc.noteOn();  

    }
    clk.setPrevious(2);
  }
}

void playPluck() {
  if (clk.counterOver(3) == true) {
    if (getZoneAverage(pixels, zone_pluck_interval) > threshold_warm) {
      clk.setInterval(3, 62.5);
    } else {
      clk.setInterval(3, 250.0);
    }

    float avg = getZoneAverage(pixels, zone_pluck_vel);
    if (avg > threshold_warm && random(100) >= 85) {
      pluck.noteOn(
        pluckNote.getRandomNoteFromScale(scale, 5, 7),
        map(avg, threshold_warm, 42, 0.0, 1.0)
      );
    }
    clk.setPrevious(3);
  }
}

void readSensor() {
  for (int i = 0; i < 64; i++) {
    pixels[i] = sensor.getPixelTemperature(i);
  }
}

