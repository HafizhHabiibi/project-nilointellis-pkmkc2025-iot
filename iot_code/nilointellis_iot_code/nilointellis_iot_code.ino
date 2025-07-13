// ===== LIBRARY =====
#include <WiFi.h>            
#include <HTTPClient.h>      
#include <ArduinoJson.h> 
#include <OneWire.h>
#include <DallasTemperature.h>

// ===== KONFIGURASI WIFI & API =====
const char* ssid = "Bharata";             
const char* password = "orangerti";     
const char* API_SENSOR_URL = "http://192.168.18.4:5000/sensor";  

// ===== KONFIGURASI PIN =====
const int pinOneWire = 4;  // Pin data sensor DS18B20
const int pinpH = 34; // Pin data sensor pH

// ===== OBJEK SENSOR =====
OneWire oneWire(pinOneWire);
DallasTemperature sensors(&oneWire);

// ===== TIMER =====
unsigned long lastSuhuCheck = 0;
unsigned long lastpHCheck = 0;
float currentpH = 0.0;

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
void kirimData(float suhu, float ph) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(API_SENSOR_URL);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> doc;
    doc["suhu"] = suhu;
    doc["ph"] = ph;

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

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  konekWifi();        // menyambungkan ke WIFI
  sensors.begin();    // mulai sensor suhu DS18B20
}

// ===== LOOP =====
void loop() {
  cekWifi(); // cek wifi

  unsigned long now = millis();

  // Baca pH setiap 5 detik
  if (now - lastpHCheck > 5000) {
    int rawpH = analogRead(pinpH);
    float voltagepH = rawpH * (3.3 / 4095.0);
    currentpH = topH(voltagepH);
    Serial.print("pH: ");
    Serial.println(currentpH, 2);
    lastpHCheck = now;
  }

  // Baca suhu setiap 5 detik
  if (now - lastSuhuCheck > 5000) {
    float suhu = bacaSuhu();
    Serial.println("Suhu: " + String(suhu) + " °C");

    lastSuhuCheck = now;
    kirimData(suhu, currentpH);  // Kirim ke server
  }

  delay(1000);  
}