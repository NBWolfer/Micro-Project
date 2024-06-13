#include <SD.h>
#include <SPI.h>

const int chipSelect = 4; // SD kart modülünün CS pin numarası

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // Serial port açılana kadar bekle
  }

  Serial.print("SD kart başlatılıyor...");

  if (!SD.begin(chipSelect)) {
    Serial.println("Başarısız!");
    return;
  }
  Serial.println("Başarılı.");

  // CSV dosyasını aç
  File dataFile = SD.open("data.csv");

  if (dataFile) {
    Serial.println("data.csv dosyası okunuyor...");

    char line[100]; // Satır uzunluğunu kısıtlı tutmak için yeterli büyüklükte buffer
    bool found = false; // Aranan metal bulunup bulunmadığını takip etmek için

    while (dataFile.available() && !found) {
      int len = dataFile.readBytesUntil('\n', line, sizeof(line) - 1);
      line[len] = '\0'; // Null-terminator ekleyin

      // Satırı ayrıştırın
      char* amp = strtok(line, ",");
      char* phase = strtok(NULL, ",");
      char* metalType = strtok(NULL, ",");

      if (amp != NULL && phase != NULL && metalType != NULL) {
        // Arama yapmak için örnek: "iron" metal türünü aramak
        if (strcmp(metalType, "iron") == 0) {
          Serial.print("Aranan metal bulundu: ");
          Serial.print("Amp: ");
          Serial.print(amp);
          Serial.print(", Phase: ");
          Serial.print(phase);
          Serial.print(", Metal: ");
          Serial.println(metalType);
          found = true; // Aranan metal bulunduğunda döngüyü durdurmak için
        }
      }

      // line dizisini sıfırla
      memset(line, 0, sizeof(line));
    }

    dataFile.close();
  } else {
    Serial.println("data.csv dosyası açılamadı.");
  }
}

void loop() {
  // Ana döngüye gerek yok
}
