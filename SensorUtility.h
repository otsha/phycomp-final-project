#include <iterator>

/**
* Print out the data so that it can be accessed within Processing.
*/
void outputSerialData(int pixels[]) {
  for (int i = 0; i < 64; i++) {
    Serial.print(pixels[i]);
    Serial.print(",");
  }

  Serial.println("");
};


