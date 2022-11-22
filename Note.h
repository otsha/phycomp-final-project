class Note {
  public:
    Note(float freq);
    float oct(int octave); 

  private:
    float _frequency;
};

Note::Note(float freq) {
  _frequency = freq;
};
  
float Note::oct(int octave) {
  return _frequency * pow(2, octave); 
};