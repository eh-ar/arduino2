#include <math.h>

// Function prototypes for calculating statistics
float calculateMin(int data[], int size);
float calculateMax(int data[], int size);
float calculateAvg(int data[], int size);
float calculateStdDev(int data[], int size);

// Function to calculate statistics for sensor data
String calculateStatistics(SensorData dataEntries[], int dataSize) {
  // Extract sensor data arrays
  int s1RhData[dataSize], s1TData[dataSize], s1EcData[dataSize], s1PhData[dataSize], s1SaData[dataSize];
  int s2RhData[dataSize], s2TData[dataSize], s2EcData[dataSize], s2PhData[dataSize], s2SaData[dataSize];
  int s3RhData[dataSize], s3TData[dataSize], s3EcData[dataSize], s3PhData[dataSize], s3SaData[dataSize];

  for (int i = 0; i < dataSize; i++) {
    s1RhData[i] = dataEntries[i].s1Rh;
    s1TData[i] = dataEntries[i].s1T;
    s1EcData[i] = dataEntries[i].s1Ec;
    s1PhData[i] = dataEntries[i].s1Ph;
    s1SaData[i] = dataEntries[i].s1Sa;
    s2RhData[i] = dataEntries[i].s2Rh;
    s2TData[i] = dataEntries[i].s2T;
    s2EcData[i] = dataEntries[i].s2Ec;
    s2PhData[i] = dataEntries[i].s2Ph;
    s2SaData[i] = dataEntries[i].s2Sa;
    s3RhData[i] = dataEntries[i].s3Rh;
    s3TData[i] = dataEntries[i].s3T;
    s3EcData[i] = dataEntries[i].s3Ec;
    s3PhData[i] = dataEntries[i].s3Ph;
    s3SaData[i] = dataEntries[i].s3Sa;
  }

  // Calculate statistics for each sensor
  float s1RhMin = calculateMin(s1RhData, dataSize);
  float s1RhMax = calculateMax(s1RhData, dataSize);
  float s1RhAvg = calculateAvg(s1RhData, dataSize);
  float s1RhStdDev = calculateStdDev(s1RhData, dataSize);

  float s1TMin = calculateMin(s1TData, dataSize);
  float s1TMax = calculateMax(s1TData, dataSize);
  float s1TAvg = calculateAvg(s1TData, dataSize);
  float s1TStdDev = calculateStdDev(s1TData, dataSize);

  float s1EcMin = calculateMin(s1EcData, dataSize);
  float s1EcMax = calculateMax(s1EcData, dataSize);
  float s1EcAvg = calculateAvg(s1EcData, dataSize);
  float s1EcStdDev = calculateStdDev(s1EcData, dataSize);

  float s1PhMin = calculateMin(s1PhData, dataSize);
  float s1PhMax = calculateMax(s1PhData, dataSize);
  float s1PhAvg = calculateAvg(s1PhData, dataSize);
  float s1PhStdDev = calculateStdDev(s1PhData, dataSize);

  float s1SaMin = calculateMin(s1SaData, dataSize);
  float s1SaMax = calculateMax(s1SaData, dataSize);
  float s1SaAvg = calculateAvg(s1SaData, dataSize);
  float s1SaStdDev = calculateStdDev(s1SaData, dataSize);

  //sensor 2

  float s2RhMin = calculateMin(s2RhData, dataSize);
  float s2RhMax = calculateMax(s2RhData, dataSize);
  float s2RhAvg = calculateAvg(s2RhData, dataSize);
  float s2RhStdDev = calculateStdDev(s2RhData, dataSize);

  float s2TMin = calculateMin(s2TData, dataSize);
  float s2TMax = calculateMax(s2TData, dataSize);
  float s2TAvg = calculateAvg(s2TData, dataSize);
  float s2TStdDev = calculateStdDev(s2TData, dataSize);

  float s2EcMin = calculateMin(s2EcData, dataSize);
  float s2EcMax = calculateMax(s2EcData, dataSize);
  float s2EcAvg = calculateAvg(s2EcData, dataSize);
  float s2EcStdDev = calculateStdDev(s2EcData, dataSize);

  float s2PhMin = calculateMin(s2PhData, dataSize);
  float s2PhMax = calculateMax(s2PhData, dataSize);
  float s2PhAvg = calculateAvg(s2PhData, dataSize);
  float s2PhStdDev = calculateStdDev(s2PhData, dataSize);

  float s2SaMin = calculateMin(s2SaData, dataSize);
  float s2SaMax = calculateMax(s2SaData, dataSize);
  float s2SaAvg = calculateAvg(s2SaData, dataSize);
  float s2SaStdDev = calculateStdDev(s2SaData, dataSize);


  //sensor 3

  float s3RhMin = calculateMin(s3RhData, dataSize);
  float s3RhMax = calculateMax(s3RhData, dataSize);
  float s3RhAvg = calculateAvg(s3RhData, dataSize);
  float s3RhStdDev = calculateStdDev(s3RhData, dataSize);

  float s3TMin = calculateMin(s3TData, dataSize);
  float s3TMax = calculateMax(s3TData, dataSize);
  float s3TAvg = calculateAvg(s3TData, dataSize);
  float s3TStdDev = calculateStdDev(s3TData, dataSize);

  float s3EcMin = calculateMin(s3EcData, dataSize);
  float s3EcMax = calculateMax(s3EcData, dataSize);
  float s3EcAvg = calculateAvg(s3EcData, dataSize);
  float s3EcStdDev = calculateStdDev(s3EcData, dataSize);

  float s3PhMin = calculateMin(s3PhData, dataSize);
  float s3PhMax = calculateMax(s3PhData, dataSize);
  float s3PhAvg = calculateAvg(s3PhData, dataSize);
  float s3PhStdDev = calculateStdDev(s3PhData, dataSize);

  float s3SaMin = calculateMin(s3SaData, dataSize);
  float s3SaMax = calculateMax(s3SaData, dataSize);
  float s3SaAvg = calculateAvg(s3SaData, dataSize);
  float s3SaStdDev = calculateStdDev(s1SaData, dataSize);

  // Print or send the calculated statistics
  Serial.print("Sensor 1 RH - Min: ");
  Serial.print(s1RhMin);
  Serial.print(", Max: ");
  Serial.print(s1RhMax);
  Serial.print(", Avg: ");
  Serial.print(s1RhAvg);
  Serial.print(", StdDev: ");
  Serial.println(s1RhStdDev);

  // Repeat for other statistics...
    String message = id + ",Da,";
  message += String(s1RhMin) + "," + String(s1RhMax) + "," + String(s1RhAvg) + "," + String(s1RhStdDev);
  message += String(s1TMin) + "," + String(s1TMax) + "," + String(s1TAvg) + "," + String(s1TStdDev);
  message += String(s1EcMin) + "," + String(s1EcMax) + "," + String(s1EcAvg) + "," + String(s1EcStdDev);
  message += String(s1PhMin) + "," + String(s1PhMax) + "," + String(s1PhAvg) + "," + String(s1PhStdDev);
  message += String(s1SaMin) + "," + String(s1SaMax) + "," + String(s1SaAvg) + "," + String(s1SaStdDev);

  message += String(s2RhMin) + "," + String(s2RhMax) + "," + String(s2RhAvg) + "," + String(s2RhStdDev);
  message += String(s2TMin) + "," + String(s2TMax) + "," + String(s2TAvg) + "," + String(s2TStdDev);
  message += String(s2EcMin) + "," + String(s2EcMax) + "," + String(s2EcAvg) + "," + String(s2EcStdDev);
  message += String(s2PhMin) + "," + String(s2PhMax) + "," + String(s2PhAvg) + "," + String(s2PhStdDev);
  message += String(s2SaMin) + "," + String(s2SaMax) + "," + String(s2SaAvg) + "," + String(s2SaStdDev);

  message += String(s3RhMin) + "," + String(s3RhMax) + "," + String(s3RhAvg) + "," + String(s3RhStdDev);
  message += String(s3TMin) + "," + String(s3TMax) + "," + String(s3TAvg) + "," + String(s3TStdDev);
  message += String(s3EcMin) + "," + String(s3EcMax) + "," + String(s3EcAvg) + "," + String(s3EcStdDev);
  message += String(s3PhMin) + "," + String(s3PhMax) + "," + String(s3PhAvg) + "," + String(s3PhStdDev);
  message += String(s3SaMin) + "," + String(s3SaMax) + "," + String(s3SaAvg) + "," + String(s3SaStdDev);

  return message;
}

float calculateMin(int data[], int size) {
  int minVal = data[0];
  for (int i = 1; i < size; i++) {
    if (data[i] < minVal) {
      minVal = data[i];
    }
  }
  return minVal;
}

float calculateMax(int data[], int size) {
  int maxVal = data[0];
  for (int i = 1; i < size; i++) {
    if (data[i] > maxVal) {
      maxVal = data[i];
    }
  }
  return maxVal;
}

float calculateAvg(int data[], int size) {
  int sum = 0;
  for (int i = 0; i < size; i++) {
    sum += data[i];
  }
  return sum / (float)size;
}

float calculateStdDev(int data[], int size) {
  float avg = calculateAvg(data, size);
  float variance = 0;
  for (int i = 0; i < size; i++) {
    variance += pow(data[i] - avg, 2);
  }
  variance /= size;
  return sqrt(variance);
}
