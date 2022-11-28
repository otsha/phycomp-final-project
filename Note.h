class Note {
  public:
    Note();
    float getRandomNoteFromScale(int scale[], int minOct, int maxOct);
    void setFrequency(float freq, int octave);
    float getFrequency();
 
  private:
    float _frequency;
};

Note::Note() {
  _frequency = 0;
};

float Note::getRandomNoteFromScale(int scale[], int minOct, int maxOct) {
  int scaleIndex = round(random(0, sizeof(scale)));
  float noteFreq = scale[scaleIndex] / 1000.0;
  int octave = random(minOct, maxOct);
  setFrequency(noteFreq, octave);
  return getFrequency();
}

void Note::setFrequency(float freq, int octave) {
  _frequency = freq * pow(2, octave);
}

float Note::getFrequency() {
  return _frequency;
}