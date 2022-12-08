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
AudioSynthWaveform       bleepFMShape;      //xy=103,134
AudioSynthWaveform       bass;      //xy=108,425
AudioSynthWaveform       bleepFM;      //xy=158,47
AudioSynthWaveformModulated bleep;   //xy=264,127
AudioEffectEnvelope      bassEnv;      //xy=342,425
AudioEffectEnvelope      bleepEnv;      //xy=353,205
AudioSynthKarplusStrong  pluck;        //xy=470,512
AudioEffectDelay         bleepDel;         //xy=488,318
AudioSynthNoiseWhite     hat;         //xy=520,746
AudioFilterStateVariable bassFlt;        //xy=558,423
AudioSynthSimpleDrum     perc;          //xy=567,673
AudioMixer4              bleepDelMix;         //xy=678,250
AudioEffectGranular      pluckGranularFX;      //xy=680,480
AudioEffectEnvelope      hatEnv;      //xy=693,739
AudioMixer4              percMixer;         //xy=884,628
AudioMixer4              melodicMixer;         //xy=886,321
AudioMixer4              mainMixer;         //xy=1064,464
AudioEffectBitcrusher    bitcrusher1;    //xy=1210,593
AudioOutputI2S           i2s1;           //xy=1325,723
AudioConnection          patchCord1(bleepFMShape, 0, bleep, 1);
AudioConnection          patchCord2(bass, bassEnv);
AudioConnection          patchCord3(bleepFM, 0, bleep, 0);
AudioConnection          patchCord4(bleep, bleepEnv);
AudioConnection          patchCord5(bassEnv, 0, bassFlt, 0);
AudioConnection          patchCord6(bleepEnv, bleepDel);
AudioConnection          patchCord7(bleepEnv, 0, bleepDelMix, 0);
AudioConnection          patchCord8(pluck, pluckGranularFX);
AudioConnection          patchCord9(bleepDel, 0, bleepDelMix, 1);
AudioConnection          patchCord10(hat, hatEnv);
AudioConnection          patchCord11(bassFlt, 0, melodicMixer, 1);
AudioConnection          patchCord12(perc, 0, percMixer, 0);
AudioConnection          patchCord13(bleepDelMix, 0, melodicMixer, 0);
AudioConnection          patchCord14(pluckGranularFX, 0, melodicMixer, 2);
AudioConnection          patchCord15(hatEnv, 0, percMixer, 1);
AudioConnection          patchCord16(percMixer, 0, mainMixer, 1);
AudioConnection          patchCord17(melodicMixer, 0, mainMixer, 0);
AudioConnection          patchCord18(mainMixer, bitcrusher1);
AudioConnection          patchCord19(bitcrusher1, 0, i2s1, 0);
AudioConnection          patchCord20(bitcrusher1, 0, i2s1, 1);
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
LFO hatLFO(4);


#define GRANULAR_MEMORY_SIZE 12800  // (from example) enough for 290 ms at 44.1 kHz
int16_t grains[GRANULAR_MEMORY_SIZE];

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Instrument setup

  bleepFM.begin(WAVEFORM_TRIANGLE_VARIABLE);
  bleepFM.amplitude(0.05);

  bleepFMShape.begin(WAVEFORM_SINE);
  bleepFMShape.frequency(A);
  bleepFMShape.amplitude(1);

  bleep.begin(WAVEFORM_SINE);
  bleep.phaseModulation(180);
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

  pluckGranularFX.begin(grains, GRANULAR_MEMORY_SIZE);
  pluckGranularFX.setSpeed(0.5);

  perc.frequency(200);
  perc.length(50);
  perc.pitchMod(0.65);

  hat.amplitude(1);
  hatEnv.attack(1.0);
  hatEnv.sustain(0);
  hatEnv.decay(10.0);

  // Set clock intervals
  clk.setInterval(0, 1000.0); // bleep
  clk.setInterval(1, 3000.0); // bass
  clk.setInterval(2, 250.0); // perc
  clk.setInterval(3, 250.0); // pluck
  clk.setInterval(4, 125.0); // hat
  clk.setInterval(7, 250.0); // IR sensor read interval

  // Set mixer levels
  melodicMixer.gain(0, 0.15); // bleep
  melodicMixer.gain(1, 0.20); // bass
  melodicMixer.gain(2, 1); // pluck

  percMixer.gain(0, 0.25); // perc
  percMixer.gain(1, 0.05); // hat

  mainMixer.gain(0, 1);
  mainMixer.gain(1, 1);

  // No bitcrush by default
  bitcrusher1.bits(16);

  // Teensy setup
  AudioMemory(127);

  codec.enable();
  codec.volume(0.25);

  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();

  // Start IR grid
  sensor.begin();
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
  // Update clock
  clk.update();
  
  // Scale shifting
  setScale(pentatonicMajor);

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
  playBleep();
  playPluck();
  playBass();
  playPerc();
  playHat();

  // Read (and output) sensor
  if (clk.counterOver(7)) {
    readSensor();
    // DEBUG: 
    outputSerialData(pixels);
    clk.setPrevious(7);
  }

  // LFO Stuff
  bassLFO.update();
  bassFlt.frequency(2000 + (1000 * bassLFO.getValue()));

  percLFO.update();
  percLFO2.update();
  hatLFO.update();
}

/**
* Play the "bleep" instrument
* - Note frequency betweeen C4-B6
* - FM amplitude is set based on temperature in a sensor zone
* - Set the interval to 1000ms or 333.33ms based on whether a zone is active
* - Notes are not played if the hat instrument is playing (sensor pixel 32)
* - If active, probability that a note is played is 56%
* - If active, delay is set randomly between 100-300ms
*/
void playBleep() {
  if (clk.counterOver(0) == true) {    
    float freq = bleepNote.getRandomNoteFromScale(scale, 4, 6);
    bleep.frequency(freq);
    bleepFM.frequency(freq * 4.0);
    bleepFM.amplitude(map(getZoneAverage(pixels, zone_bleep_fm), 18, 35, 0, 0.5));
    
    if (getZoneAverage(pixels, zone_bleep_interval) > threshold_warm) {
      clk.setInterval(0, 333.33);
      bleepEnv.attack(10);
      bleepEnv.decay(50);
    } else {
      clk.setInterval(0, 1000.0);
      bleepEnv.attack(5);
      bleepEnv.decay(random(50, 2000));
    }

    if (random(100) > 40 && pixels[32] <= threshold_warm) {
      bleepDel.delay(0, random(100, 300));
      bleepEnv.noteOn();
    }
    
    clk.setPrevious(0);
  }
}

/**
* Play the bass instrument
* - Interval 3000ms
* - Note frequency between C2-B4
* - If zone_bass is active, change waveform from a triangle to a sawtooth
* - If the waveform is a sawtooth, filter frequencies >1300Hz out
*/
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

/**
* Play the "perc" instrument (drum / laser / percussion)
* - Interval 250ms
* - Only played 50% of the time zone_perc is active
* - Base frequency set based on an LFO value
* - 25% chance when playing to set the starting frequency (resulting in glissando up/down)
* to a value from an LFO
* - If sensor pixel 60 is active, randomize length between 100ms and 150ms
*/
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

/**
* Play the "pluck" instrument (string instrument)
* - Note frequencies between C5-B7
* - Default interval 250ms
* - If sensor pixel 14 is active, apply granular "freeze" effect
* - If sensor zone zone_pluck_interval is active, set the interval to 62.5ms
* - Notes are played 15% of the time zone_pluck is active
* - Note velocity is determined by the average warmth in zone_pluck_vel
*/
void playPluck() {
  if (clk.counterOver(3) == true) {

    if (pixels[14] > threshold_warm) {
      pluckGranularFX.beginPitchShift(4000);
    } else {
      pluckGranularFX.stop();
    }

    if (getZoneAverage(pixels, zone_pluck_interval) > threshold_warm) {
      clk.setInterval(3, 62.5);
    } else {
      clk.setInterval(3, 250.0);
    }

    float activationAvg = getZoneAverage(pixels, zone_pluck);
    float velocityAvg = getZoneAverage(pixels, zone_pluck_vel);

    if (activationAvg > threshold_warm && random(100) >= 85) {
      pluck.noteOn(
        pluckNote.getRandomNoteFromScale(scale, 5, 7),
        map(velocityAvg, threshold_warm, 42, 0.25, 1.0)
      );
    }
    
    clk.setPrevious(3);
  }
}

/**
* Play the hat (white noise) instrument
* - Interval 125ms
* - Notes are played 34% of the time when sensor pixel 32 is active
*/
void playHat() {
  if (clk.counterOver(4) == true) {
    hatEnv.decay(15.0 + (hatLFO.getValue() * 10.0));
    
    if (pixels[32] > threshold_warm && random(100) >= 66) {
      hatEnv.noteOn();
    }
    
    clk.setPrevious(4);
  }
}

void readSensor() {
  for (int i = 0; i < 64; i++) {
    pixels[i] = sensor.getPixelTemperature(i);
  }
}

