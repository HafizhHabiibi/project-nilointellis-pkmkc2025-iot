// ===== LIBRARY =====
#include <WiFi.h>            
#include <HTTPClient.h>      
#include <ArduinoJson.h> 
#include <OneWire.h>
#include <DallasTemperature.h>
#include <GravityTDS.h>

// ===== KONFIGURASI WIFI & API =====
const char* ssid = "UTY-Connect";             
const char* password = "";     
const char* API_SENSOR_URL = "http://192.168.40.181:5000/sensor";  

// ===== KONFIGURASI PIN =====
const int pinOneWire = 4;  // Pin data sensor DS18B20
const int pinpH = 34; // Pin data sensor pH
const int pintds = 35; // Pin data sensor TDS
const int pintur = 32; // Pin data sensor turbiity

// ===== OBJEK SENSOR =====
OneWire oneWire(pinOneWire);
DallasTemperature sensors(&oneWire);
GravityTDS gravityTds;

// ===== TIMER =====
unsigned long lastSuhuCheck = 0;
unsigned long lastpHCheck = 0;
unsigned long lastTdsCheck = 0;
unsigned long lastTurCheck = 0;
float currentpH = 0.0;
float currentTDS = 0.0;
float currentTur = 0.0;
float suhu = 0.0;

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

    Serial.println("Kirim data ke API, Response: " + String(httpResponseCode));
    http.end();
  }
}

// 4. Fungsi baca suhu DS18B20
float bacaSuhu() {
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);  
}

// 5. Fungsi konversi nilai pH
float topH(float voltage) {
  float m = (4.0 - 7.0) / (2.947 - 2.315);
  float b = 7.0 - (m * 2.315);
  return m * voltage + b;
}

// 6. Fungsi baca TDS
float bacaTDS(float suhu) {
  gravityTds.setTemperature(suhu);  // Kompensasi suhu
  gravityTds.update();              // Perbarui data TDS
  return gravityTds.getTdsValue();  // Ambil nilai TDS
}

// 7. Fungsi konversi nilai NTU
float toNTU(float voltage) {
  float m = -917.43;
  float b = 2370.69;
  float ntu = m * voltage + b;
  if (ntu < 0) ntu = 0;
  return ntu;
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  konekWifi();        // menyambungkan ke WIFI
  sensors.begin();    // mulai sensor suhu DS18B20
  gravityTds.setPin(pintds);       // Set pin sensor TDS
  gravityTds.setAref(3.3);         // Tegangan referensi
  gravityTds.begin();              // Inisialisasi sensor
}

// ===== LOOP =====
void loop() {
  cekWifi(); // cek wifi

  unsigned long now = millis();

  // Baca pH setiap 1 menit
  if (now - lastpHCheck > 5000) {
    int rawpH = analogRead(pinpH);
    float voltagepH = rawpH * (3.3 / 4095.0);
    currentpH = topH(voltagepH);
    Serial.print("pH: ");
    Serial.println(currentpH, 2);

    lastpHCheck = now;
  }

// Baca NTU setiap 1 menit
  if (now - lastTurCheck > 5000) {
    int rawNTU = analogRead(pintur);
    float voltageNTU = rawNTU * (3.3 / 4095.0);
    currentTur = toNTU(voltageNTU);

    Serial.print("NTU: ");
    Serial.println(currentTur, 2);

    lastTurCheck = now;
  }

// Baca suhu + TDS + kirim setiap 1 menit
  if (now - lastSuhuCheck > 5000) {
    suhu = bacaSuhu();
    Serial.println("Suhu: " + String(suhu) + " °C");

    currentTDS = bacaTDS(suhu);
    Serial.print("TDS: ");
    Serial.print(currentTDS, 0);
    Serial.println(" ppm");

    kirimData(suhu, currentpH, currentTDS, currentTur);

    lastSuhuCheck = now;
  }

  delay(1000);  
}