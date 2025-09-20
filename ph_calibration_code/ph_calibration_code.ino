#include "DFRobot_PH.h"
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>


#define PH_PIN 34       
#define ONE_WIRE_BUS 13   

// --- Objek sensor ---
DFRobot_PH ph;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float voltage;
float phValue;
float temperature;

void setup() {
  Serial.begin(115200);

  sensors.begin();  
  ph.begin();       // inisialisasi pH sensor (load kalibrasi dari EEPROM)
}

void loop() {
  static unsigned long timepoint = millis();
  if (millis() - timepoint > 1000U) {   
    timepoint = millis();

    // --- Baca suhu dari DS18B20 ---
    sensors.requestTemperatures();
    temperature = sensors.getTempCByIndex(0);

    // --- Baca tegangan dari sensor pH ---
    int adcValue = analogRead(PH_PIN);                
    voltage = (adcValue / 4095.0) * 3300;             

    // --- Hitung nilai pH dengan kompensasi suhu ---
    phValue = ph.readPH(voltage, temperature);

    // --- Tampilkan ke Serial Monitor ---
    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.print(" °C   pH: ");
    Serial.println(phValue, 2);
  }

  // --- Proses kalibrasi lewat Serial Monitor ---
  ph.calibration(voltage, temperature);
}