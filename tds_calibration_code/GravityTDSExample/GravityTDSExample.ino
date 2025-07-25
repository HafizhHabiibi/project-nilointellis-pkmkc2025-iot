#include <OneWire.h>
#include <DallasTemperature.h>
#include <GravityTDS.h>

// --- DS18B20 Setup ---
#define ONE_WIRE_BUS 4  // Sesuaikan dengan pin data DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// --- TDS Setup ---
#define TDS_PIN 35  // Pin ADC untuk TDS
GravityTDS gravityTds;

float temperature = 25.0;
float tdsValue;

void setup() {
  Serial.begin(115200);

  // Inisialisasi sensor suhu DS18B20
  sensors.begin();

  // Inisialisasi sensor TDS
  gravityTds.setPin(TDS_PIN);
  gravityTds.setAref(3.3);       // Tegangan referensi ADC (ESP32 = 3.3V)
  gravityTds.setAdcRange(4096);  // ADC ESP32 12-bit: 0–4095
  gravityTds.begin();
}

void loop() {
  // Baca suhu dari sensor DS18B20
  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);

  // Atur suhu untuk kompensasi di library TDS
  gravityTds.setTemperature(temperature);

  // Baca data TDS
  int rawADC = analogRead(TDS_PIN);
  gravityTds.update();
  tdsValue = gravityTds.getTdsValue();

  // Output ke Serial Monitor
  Serial.print("Suhu: ");
  Serial.print(temperature, 2);
  Serial.print(" °C | ADC Raw: ");
  Serial.print(rawADC);
  Serial.print(" | TDS: ");
  Serial.print(tdsValue, 0);
  Serial.println(" ppm");

  delay(1000);
}