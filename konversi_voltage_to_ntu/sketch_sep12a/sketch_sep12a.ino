int sensorPin = 34; // pin ADC ESP32, contoh GPIO34 (input only)
const int samples = 800; 

// Faktor koreksi hasil pengukuran multimeter vs ADC
// ganti sesuai hasil pengukuranmu sendiri
const float correctionFactor = 1.14;  

// Persamaan regresi hasil kalibrasi percobaan ke-2
const float coef = -1982.9;
const float intercept = 5151.1;

void setup() {
  Serial.begin(115200); 
  analogSetAttenuation(ADC_11db); // pastikan ADC bisa baca sampai 3.3V
}

void loop() {
  float tegangan = 0;

  // ambil sampel untuk meredam noise
  for (int i = 0; i < samples; i++) {
    int rawADC = analogRead(sensorPin);          
    float volt = (rawADC * 3.3 / 4095.0) * correctionFactor;  // konversi + koreksi
    tegangan += volt;
  }
  tegangan = tegangan / samples; // rata-rata tegangan

  // Hitung NTU sesuai regresi percobaan ke-2
  float ntu = (coef * tegangan) + intercept;

  // Batasi hasil supaya tidak keluar range
  if (ntu < 0) ntu = 0;          
  if (ntu > 3000) ntu = 3000;    

  // tampilkan hasil
  Serial.print("Tegangan (koreksi): ");
  Serial.print(tegangan, 3);
  Serial.print(" V | NTU: ");
  Serial.println(ntu, 1);

  delay(1000); 
}