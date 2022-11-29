int zone_perc[] = {30, 31, 38, 39};
int zone_bass[] = {0, 1, 2, 3};
int zone_pluck_vel[] = {56, 57, 58, 59};
int zone_pluck_freq[] = {26, 27, 34, 35};
int zone_scale[] = {6, 7, 62, 63};

int threshold_warm = 20;
int threshold_warmer = 25;
int threshold_hot = 30;

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

float getSensorAverage(int pixels[]) {
  float sum = 0;
  for (int i = 0; i < 64; i++) {
    sum += pixels[i];
  }
  return sum / 64.0;
}


