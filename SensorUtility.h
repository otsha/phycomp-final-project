/**
* Define threshold values for sensing presence.
* Only the lowest threshold is really used within this project,
* as fabrics and distance both affect surface temperature detection.
*/
int threshold_warm = 20;
int threshold_warmer = 25;
int threshold_hot = 30;

/**
* Detection zones for changing instrument behavior.
* Refer to the project documentation for a visual representation.
*/
int zone_perc[] = {30, 31, 38, 39};
int zone_bass[] = {0, 1, 2, 3};
int zone_pluck[] = {56, 57, 58, 59};
int zone_pluck_vel[] = {26, 27, 34, 35};
int zone_pluck_interval[] = {9, 10, 17, 18};
int zone_scale[] = {6, 7, 62, 63};
int zone_bleep_interval[] = {12, 13, 20, 21};
int zone_bleep_fm[] = {41, 42, 59, 60};

/**
* Print out the data so that it can be accessed within Processing for debugging.
*/
void outputSerialData(int pixels[]) {
  for (int i = 0; i < 64; i++) {
    Serial.print(pixels[i]);
    Serial.print(",");
  }

  Serial.println("");
};

/**
* Get the average temperature within a zone
*/
float getZoneAverage(int pixels[], int indices[]) {
  float sum = 0;
  int size = sizeof(indices);
  for (int i = 0; i < size; i++) {
    sum += pixels[indices[i]];
  }
  return sum / size * 1.0;
}

/**
* Get the average temperature for the entire sensor
*/
float getSensorAverage(int pixels[]) {
  float sum = 0;
  for (int i = 0; i < 64; i++) {
    sum += pixels[i];
  }
  return sum / 64.0;
}


