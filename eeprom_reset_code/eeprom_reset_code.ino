/*
 * Reset EEPROM kalibrasi DFRobot Gravity pH Meter V2 (SEN0161-V2)
 * Gunakan ini jika ingin menghapus data kalibrasi lama.
 * Setelah upload, buka Serial Monitor (115200) dan pastikan semua value = 255.
 */

#include <EEPROM.h>

#define PHADDR 0x00     // alamat awal EEPROM untuk kalibrasi pH
#define EEPROM_SIZE 64  // cukup besar untuk pH + TDS dsb.

void setup() {
  Serial.begin(115200);

  // Inisialisasi EEPROM
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("Gagal inisialisasi EEPROM");
    while (1);
  }

  // Reset 8 byte (dua float) ke 0xFF
  for (int i = 0; i < 8; i++) {
    EEPROM.write(PHADDR + i, 0xFF);
  }
  EEPROM.commit();  // simpan ke flash
  delay(100);

  Serial.println("EEPROM pH sudah di-reset ke 255:");
  
  // Baca ulang hasil reset
  for (int i = 0; i < 8; i++) {
    int value = EEPROM.read(PHADDR + i);
    Serial.print("0x");
    Serial.print(PHADDR + i, HEX);
    Serial.print(": ");
    Serial.println(value);  // seharusnya 255
  }
}

void loop() {
  // tidak ada loop
}