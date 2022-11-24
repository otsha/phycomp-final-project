int zone_perc[] = {30, 31, 38, 39};

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

float getZoneAverage(int pixels[], int indices[]) {
  float sum = 0;
  int size = sizeof(indices);
  for (int i = 0; i < size; i++) {
    sum += pixels[indices[i]];
  }
  return sum / size * 1.0;
}


