class LFO {
  public:
    LFO(float rate);
    float getValue();
    void update();
  private:
    float _arc;
    float _rate;
};

LFO::LFO(float rate) {
  _rate = rate;
  _arc = 0;
};

void LFO::update() {
  _arc += _rate;

  if (_arc > 1.0) {
    _arc = 0;
  }
};

float LFO::getValue() {
  return sin(_arc);
};

