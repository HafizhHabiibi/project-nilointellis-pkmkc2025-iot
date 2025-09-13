int sensorPin = 34;
const int samples = 800;

// voltage divider values (ohm)
const float R1 = 3300.0;
const float R2 = 8200.0;
const float ratio = R2 / (R1 + R2);

// ADC/Vref assumptions
const float ADC_MAX = 4095.0;
float VREF = 3.3; 

// correctionFactor = Vout_measured / Vout_calculated_from_ADC
float correctionFactor = 1.14;

// Persamaan NTU (ganti sesuai regresi kamu)
const float coef = -1982.9;
const float intercept = 5151.1;

void setup() {
  Serial.begin(115200);
  delay(100);

  #ifdef ESP32
    analogSetAttenuation(ADC_11db);
  #endif
  Serial.println("DEBUG: mulai pembacaan ADC");
  Serial.println("Jika ingin kalibrasi: ukur Vout (node antara R1 dan R2) dengan multimeter, lalu hitung correctionFactor.");
}

void loop() {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(sensorPin);
    delay(1);
  }
  int rawADC = sum / samples;

  // Konversi ADC ke Vout (yang masuk ADC, sebelum koreksi)
  float Vout_calc = (rawADC * VREF) / ADC_MAX; // Volt (tanpa koreksi)

  float Vout_corrected = Vout_calc * correctionFactor;

  // Rekonstruksi Vsensor (sebelum divider)
  float Vsensor = Vout_corrected / ratio;

  // Hitung NTU
  float NTU = coef * Vsensor + intercept;
  if (NTU < 0) NTU = 0;

  Serial.print("rawADC: "); Serial.print(rawADC);
  Serial.print(" | Vout_calc: "); Serial.print(Vout_calc, 4); Serial.print(" V");
  Serial.print(" | corrFactor: "); Serial.print(correctionFactor, 4);
  Serial.print(" | Vout_corr: "); Serial.print(Vout_corrected, 4); Serial.print(" V");
  Serial.print(" | Vsensor: "); Serial.print(Vsensor, 4); Serial.print(" V");
  Serial.print(" | NTU: "); Serial.println(NTU, 2);

  delay(1000);
}