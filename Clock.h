class Clock {
  public:
    Clock();
    void setInterval(int index, float value);
    void setPrevious(int index);
    void update();
    bool counterOver(int index);

  private:
    float _cTime;
    float _pTimes[8];
    float _intervals[8];
};

Clock::Clock() {
  for (int i = 0; i < 8; i++) {
    _pTimes[i] = _cTime;
    _intervals[i] = 1000.0;
  }
};

void Clock::setInterval(int index, float value) {
  _intervals[index] = value;
};

void Clock::setPrevious(int index) {
  _pTimes[index] = _cTime;
}

void Clock::update() {
  _cTime = millis();
};

bool Clock::counterOver(int index) {
  if (_cTime - _pTimes[index] > _intervals[index]) {
    return true;
  }

  return false;
}