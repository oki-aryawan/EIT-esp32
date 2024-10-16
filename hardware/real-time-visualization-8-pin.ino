const int electrodePins[8] = {13, 12, 14, 27, 26, 25, 32, 15}; // pin configuration

// Wenner configuration for 8 electrodes
const int wennerConfig[][4] = {
  // Layer 1 (5 measurements)
  {0, 1, 2, 3}, {1, 2, 3, 4}, {2, 3, 4, 5}, {3, 4, 5, 6}, {4, 5, 6, 7},
  // Layer 2 (4 measurements)
  {0, 1, 3, 4}, {1, 2, 4, 5}, {2, 3, 5, 6}, {3, 4, 6, 7},
  // Layer 3 (3 measurements)
  {0, 1, 4, 5}, {1, 2, 5, 6}, {2, 3, 6, 7},
  // Layer 4 (2 measurements)
  {0, 1, 5, 6}, {1, 2, 6, 7},
  // Layer 5 (1 measurement)
  {0, 1, 6, 7}
};
const int numConfigurations = 15;  // Total number of measurements

// Constants for current injection and impedance calculation
const float CURRENT_LIMIT_RESISTOR = 2000.0; // Using 2k ohm resistor
const float ESP32_VOLTAGE = 3.3; // ESP32 output voltage
const float ACTUAL_CURRENT = ESP32_VOLTAGE / CURRENT_LIMIT_RESISTOR;

float measurements[15];  // Array to store all measurements

void setup() {
  Serial.begin(115200);
  
  for (int i = 0; i < 8; i++) {
    pinMode(electrodePins[i], INPUT); // Set all electrodes to high impedance initially
  }
}

void loop() {
  performMeasurements();
  printMeasurements();
  delay(5000); // Wait for 5 seconds before the next measurement cycle
}

void performMeasurements() {
  for (int i = 0; i < numConfigurations; i++) {
    int current1 = wennerConfig[i][0];
    int voltage1 = wennerConfig[i][1];
    int voltage2 = wennerConfig[i][2];
    int current2 = wennerConfig[i][3];

    setCurrent(current1, current2);
    delay(10);  // Allow current to stabilize

    float voltage = measureVoltage(voltage1, voltage2);
    measurements[i] = calculateImpedance(voltage);
  }
}

void printMeasurements() {
  Serial.print("[");
  for (int i = 0; i < numConfigurations; i++) {
    Serial.print(measurements[i], 2); // Print with 2 decimal places
    if (i < numConfigurations - 1) {
      Serial.print(", ");
    }
  }
  Serial.println("]");
}

void setCurrent(int electrode1, int electrode2) {
  // Reset all electrodes to input (high impedance)
  for (int i = 0; i < 8; i++) {
    pinMode(electrodePins[i], INPUT);
  }
  
  // Set current electrodes
  pinMode(electrodePins[electrode1], OUTPUT);
  pinMode(electrodePins[electrode2], OUTPUT);
  digitalWrite(electrodePins[electrode1], HIGH);
  digitalWrite(electrodePins[electrode2], LOW);
}

float measureVoltage(int electrode1, int electrode2) {
  pinMode(electrodePins[electrode1], INPUT);
  pinMode(electrodePins[electrode2], INPUT);
  
  int adc1 = analogRead(electrodePins[electrode1]);
  int adc2 = analogRead(electrodePins[electrode2]);
  float voltage1 = adc1 * (3.3 / 4095.0);
  float voltage2 = adc2 * (3.3 / 4095.0);
  return voltage1 - voltage2;
}

float calculateImpedance(float voltage) {
  return voltage / ACTUAL_CURRENT;
}
