// ===== LIBRARY =====
#include <WiFi.h>            
#include <HTTPClient.h>      
#include <ArduinoJson.h> 
#include <OneWire.h>
#include <DallasTemperature.h>
#include <GravityTDS.h>
#include "DFRobot_ESP_PH.h"
#include "EEPROM.h"

// ===== KONFIGURASI WIFI & API =====
const char* ssid = "Bharata";             
const char* password = "orangerti";     
const char* API_SENSOR_URL = "http://192.168.18.4:5000/sensor";  

// ===== KONFIGURASI PIN =====
const int pinOneWire = 13;  // Pin data sensor DS18B20
const int pinpH = 32; // Pin data sensor pH
const int pintds = 35; // Pin data sensor TDS
const int pintur = 34; // Pin data sensor turbiity

// ===== KONFIGURASI Turbidity =====
const int samples = 800;

// Referensi tegangan
const float ADC_MAX = 4095.0;
float VREF = 3.3;

// correctionFactor perbandingan ADS ESP32 & Multimeter
float correctionFactor = 1.14;

// Persamaan linear NTU
const float coef = -1982.9;
const float intercept = 5151.1;

// ===== OBJEK SENSOR =====
OneWire oneWire(pinOneWire);
DallasTemperature sensors(&oneWire);
GravityTDS gravityTds;
DFRobot_ESP_PH ph;

// ===== TIMER =====
unsigned long lastSuhuTDSCheck = 0;
unsigned long lastpHCheck = 0;
unsigned long lastTurCheck = 0;
unsigned long lastKirimCheck = 0;
float currentpH = 0.0;
float currentTDS = 0.0;
float currentTur = 0.0;
float suhu = 0.0;

// ===== RATA RATA TDS, SUHU, PH =====
float sumPH = 0, sumTDS = 0, sumSuhu = 0;
int sampleCount = 0;

// ===== FUNGSI =====

// 1. Fungsi koneksi WIFI
void konekWifi() {
  WiFi.begin(ssid, password);
  Serial.print("Menyambungkan ke WIFI");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[INFO] WIFI Connected!");
}

// 2. Fungsi Reconnect WIFI
void cekWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WARNING] WIFI terputus! Mencoba reconnect...");

    WiFi.disconnect();
    WiFi.begin(ssid, password);

    unsigned long startAttemptTime = millis();

    // Coba koneksi maksimal 10 detik
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n[INFO] WIFI kembali terhubung!");
    } else {
      Serial.println("\n[ERROR] Gagal menghubungkan kembali ke WIFI setelah 10 detik!");
    }
  }
}

// 3. Fungsi kirim data ke API
void kirimData(float suhu, float ph, float tds, float turbidity) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(API_SENSOR_URL);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> doc;
    doc["suhu"] = suhu;
    doc["ph"] = ph;
    doc["tds"] = tds;
    doc["turbidity"] = turbidity;
 
    String body;
    serializeJson(doc, body);
    int httpResponseCode = http.POST(body);

    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("Response code: "+ String(httpResponseCode));
      Serial.println("Response body: "+ payload);
    }

    else {
      Serial.println("[ERROR] saat mengirim data, code: "+ String(httpResponseCode));
    }
    http.end();
  }
}

// 4. Fungsi baca suhu DS18B20
float bacaSuhu() {
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);  
}

// 5. Fungsi baca TDS
float bacaTDS(float suhu) {
  gravityTds.setTemperature(suhu);  // Kompensasi suhu
  gravityTds.update();              // Perbarui data TDS
  return gravityTds.getTdsValue();  // Ambil nilai TDS
}

// 6. Fungsi konversi nilai NTU
float bacaNTU() {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pintur);
    delay(1);
  }
  int rawADC = sum / samples;

  // Konversi ADC ke tegangan (Vout langsung dari pin ESP32)
  float Vout_calc = (rawADC * VREF) / ADC_MAX;

  // Koreksi dengan correctionFactor
  float Vout_corrected = Vout_calc * correctionFactor;

  // Hitung NTU langsung (tanpa rekonstruksi divider)
  float NTU = coef * Vout_corrected + intercept;

  // Batasi range hasil
  if (NTU < 0) NTU = 0;
  if (NTU > 3000) NTU = 3000;

  return NTU;
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  EEPROM.begin(32);

  konekWifi();        // menyambungkan ke WIFI
  sensors.begin();    // mulai sensor suhu DS18B20
  gravityTds.setPin(pintds);       // Set pin sensor TDS
  gravityTds.setAref(3.3);         // Tegangan referensi
  gravityTds.begin();              // Inisialisasi TDS
  ph.begin(); //Inisiasi PH
  
}

// ===== LOOP =====
void loop() {
  cekWifi(); // cek wifi

  unsigned long now = millis();

  // Baca pH setiap 1 menit
  if (now - lastpHCheck > 5000) {
    int rawpH = analogRead(pinpH);
    float voltagepH = rawpH * (3.3 / 4095.0 * 1000);
    currentpH = ph.readPH(voltagepH, suhu);
    Serial.print("pH: ");
    Serial.println(currentpH, 2);

    sumPH += currentpH;
    sampleCount++;

    lastpHCheck = now;
  }

// Baca NTU setiap 1 menit
  if (now - lastTurCheck > 60000) {
    currentTur = bacaNTU();

    Serial.print("NTU: ");
    Serial.println(currentTur, 2);

    lastTurCheck = now;
  }

// Baca suhu + TDS + kirim setiap 1 menit
  if (now - lastSuhuTDSCheck > 5000) {
    suhu = bacaSuhu();
    Serial.println("Suhu: " + String(suhu) + " °C");

    currentTDS = bacaTDS(suhu);
    Serial.print("TDS: ");
    Serial.print(currentTDS, 0);
    Serial.println(" ppm");

    sumTDS += currentTDS;
    sumSuhu += suhu;

    lastSuhuTDSCheck = now;
  }

// Kirim Data Sensor Setiap 1 Menit
if (now - lastKirimCheck > 60000 && sampleCount > 0){


  float avgPH = sumPH / sampleCount;
  float avgTDS = sumTDS / sampleCount;
  float avgSuhu = sumSuhu / sampleCount;

  kirimData(avgSuhu, avgPH, avgTDS, currentTur);

  // Reset Value
  sumPH = sumTDS = sumSuhu = 0;
  sampleCount = 0;

  lastKirimCheck = now;
}

  delay(1000);  
}