
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include "Note.h"
#include "Freq.h"

// GUItool: begin automatically generated code
AudioSynthWaveformSine   sine1;          //xy=158,203
AudioEffectEnvelope      envelope1;      //xy=338,239
AudioMixer4              mixer1;         //xy=742,338
AudioOutputI2S           i2s1;           //xy=997,333
AudioConnection          patchCord1(sine1, envelope1);
AudioConnection          patchCord2(envelope1, 0, mixer1, 0);
AudioConnection          patchCord3(mixer1, 0, i2s1, 0);
AudioConnection          patchCord4(mixer1, 0, i2s1, 1);
// GUItool: end automatically generated code

AudioControlSGTL5000 codec;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  sine1.frequency(440);
  sine1.amplitude(1);
  envelope1.attack(10);
  envelope1.hold(0);
  envelope1.decay(100);
  envelope1.sustain(0);
  envelope1.release(100);

  mixer1.gain(0, 0.25);

  AudioMemory(18);

  codec.enable();
  codec.volume(0.25);

  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();
}

float scale[] = {C, D, E, G, A};

void loop() {

  // Random Synth
  int scaleIndex = round(random(0, 4));
  float noteFreq = scale[scaleIndex] / 1000.0;
  Note n(noteFreq);
  sine1.frequency(n.oct(random(4, 7)));
  envelope1.decay(random(100, 1000));

  // Play note
  envelope1.noteOn();
  
  delay(500);
}



