const int electrodePins[16] = {15, 2, 4, 16, 17, 5, 18, 19, 21, 22, 23, 13, 12, 14, 27, 26};

// Wenner configuration for 16 electrodes
const int wennerConfig[][4] = {
  // Layer 1 (13 measurements)
  {0, 1, 2, 3}, {1, 2, 3, 4}, {2, 3, 4, 5}, {3, 4, 5, 6}, {4, 5, 6, 7},
  {5, 6, 7, 8}, {6, 7, 8, 9}, {7, 8, 9, 10}, {8, 9, 10, 11}, {9, 10, 11, 12},
  {10, 11, 12, 13}, {11, 12, 13, 14}, {12, 13, 14, 15},
  // Layer 2 (11 measurements)
  {0, 2, 4, 6}, {1, 3, 5, 7}, {2, 4, 6, 8}, {3, 5, 7, 9}, {4, 6, 8, 10},
  {5, 7, 9, 11}, {6, 8, 10, 12}, {7, 9, 11, 13}, {8, 10, 12, 14}, {9, 11, 13, 15},
  {10, 12, 14, 15},
  // Layer 3 (9 measurements)
  {0, 3, 6, 9}, {1, 4, 7, 10}, {2, 5, 8, 11}, {3, 6, 9, 12}, {4, 7, 10, 13},
  {5, 8, 11, 14}, {6, 9, 12, 15}, {7, 10, 13, 15}, {8, 11, 14, 15},
  // Layer 4 (7 measurements)
  {0, 4, 8, 12}, {1, 5, 9, 13}, {2, 6, 10, 14}, {3, 7, 11, 15},
  {4, 8, 12, 15}, {5, 9, 13, 15}, {6, 10, 14, 15},
  // Layer 5 (5 measurements)
  {0, 5, 10, 15}, {1, 6, 11, 15}, {2, 7, 12, 15}, {3, 8, 13, 15}, {4, 9, 14, 15},
  // Layer 6 (3 measurements)
  {0, 6, 12, 15}, {1, 7, 13, 15}, {2, 8, 14, 15},
  // Layer 7 (1 measurement)
  {0, 7, 14, 15}
};
const int numConfigurations = 49;  // Total number of measurements
const int layerSizes[] = {13, 11, 9, 7, 5, 3, 1};  // Number of measurements in each layer

// Constants for current injection and impedance calculation
const float CURRENT_LIMIT_RESISTOR = 2000.0; // Using 2k ohm resistor
const float ESP32_VOLTAGE = 3.3; // ESP32 output voltage
const float ACTUAL_CURRENT = ESP32_VOLTAGE / CURRENT_LIMIT_RESISTOR;

float measurements[49];  // Array to store all measurements

void setup() {
  Serial.begin(115200);
  
  for (int i = 0; i < 16; i++) {
    pinMode(electrodePins[i], INPUT); // Set all electrodes to high impedance initially
  }

  Serial.println("Enter to start");
  waitForInput();
}

void loop() {
  int configIndex = 0;
  for (int layer = 0; layer < 7; layer++) {
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
    
    if (layer < 6) {  // Don't print dashes after the last layer
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
  for (int i = 0; i < 16; i++) {
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
