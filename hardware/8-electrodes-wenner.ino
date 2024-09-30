const int electrodePins[8] = {12, 14, 27, 26, 25, 33, 32, 15};

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
const int layerSizes[] = {5, 4, 3, 2, 1};  // Number of measurements in each layer

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

  Serial.println("Enter to start");
  waitForInput();
}

void loop() {
  int configIndex = 0;
  for (int layer = 0; layer < 5; layer++) {
    Serial.printf("Layer %d\n", layer + 1);
    
    for (int measurement = 0; measurement < layerSizes[layer]; measurement++) {
      bool repeat;
      do {
        repeat = false;
        int current1 = wennerConfig[configIndex][0];
        int voltage1 = wennerConfig[configIndex][1];
        int voltage2 = wennerConfig[configIndex][2];
        int current2 = wennerConfig[configIndex][3];

        setCurrent(current1, current2);
        delay(10);  // Allow current to stabilize

        float voltage = measureVoltage(voltage1, voltage2);
        float impedance = calculateImpedance(voltage);
        
        measurements[configIndex] = impedance;  // Store the impedance value
        
        Serial.printf("Current injection: Pin %d (HIGH) - Pin %d (LOW)\n", 
                      electrodePins[current1], electrodePins[current2]);
        Serial.printf("Voltage measurement: Pin %d (V1) - Pin %d (V2)\n", 
                      electrodePins[voltage1], electrodePins[voltage2]);
        Serial.printf("V%d = %.4f\n", measurement + 1, voltage);
        Serial.printf("I%d = %.2f\n", measurement + 1, impedance);
        Serial.println("Press Enter for next measurement or 'q' to repeat this measurement");
        
        char input = waitForInput();
        if (input == 'q') {
          repeat = true;
          Serial.println("Repeating measurement...");
        }
      } while (repeat);
      
      configIndex++;
    }
    
    if (layer < 4) {  // Don't print dashes after the last layer
      Serial.println("------");
    }
  }
  
  // Print all measurements in a list format
  Serial.println("\nAll measurements (impedance values in ohms):");
  Serial.println("[");
  for (int i = 0; i < numConfigurations; i++) {
    Serial.print(measurements[i]);
    if (i < numConfigurations - 1) {
      Serial.print(", ");
    }
    if ((i + 1) % 5 == 0) {
      Serial.println();
    }
  }
  Serial.println("\n]");
  
  Serial.println("Enter to restart");
  waitForInput();
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
  float voltage = voltage1 - voltage2;
  
  Serial.printf("  ADC1: %d, ADC2: %d\n", adc1, adc2);
  Serial.printf("  V1: %.4f, V2: %.4f\n", voltage1, voltage2);
  
  return voltage;
}

float calculateImpedance(float voltage) {
  // Calculate impedance using Ohm's law: Z = V / I
  float impedance = voltage / ACTUAL_CURRENT;
  return impedance;
}

char waitForInput() {
  while (!Serial.available()) {
    // Wait for input
  }
  char input = Serial.read();
  while (Serial.available()) {
    Serial.read(); // Clear the input buffer
  }
  return input;
}
