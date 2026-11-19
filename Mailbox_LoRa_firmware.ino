#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// Tetapan Skrin OLED 0.96" (128x64)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Tetapan Pin LoRa Ra-02 / Ra-01
#define LORA_SS   5
#define LORA_RST  14
#define LORA_DIO0 2

// Tetapan Pin Butang & Buzzer
#define BTN_PIN    4
#define BUZZER_PIN 15

// --- TETAPAN PRIVASI SALURAN (MESTI SAMA UNTUK KEDUA-DUA UNIT) ---
const byte PRIVATE_SYNC_WORD = YOUR_PRIVATE_WORD;         // Kod saluran radio (0x00 hingga 0xFF)
const String SECRET_ID       = "YOUR_ID:";                // Kata kunci rahsia pengesahan mesej

// Tetapan Masa Kod Morse (milisaat)
const unsigned long DOT_MAX_MS    = 250;  // Tekan < 250ms = Titik (.)
const unsigned long LETTER_GAP_MS = 650;  // Lepas butang 650ms = Convert ke Huruf
const unsigned long WORD_GAP_MS   = 1400; // Lepas butang 1.4s = Tambah Jarak (Space)
const unsigned long SEND_GAP_MS   = 3000; // Berhenti menaip 3.0s = Terus Hantar LoRa

String currentLetterMorse = ""; 
String fullMorseHistory   = ""; 
String convertedText      = ""; 
String rxBuffer           = "Tiada mesej";

unsigned long pressStartTime  = 0;
unsigned long lastReleaseTime = 0;
bool isPressing               = false;
bool letterConverted          = true;
bool spaceAdded               = true;
bool messageSent              = true;

// Kamus Penterjemah Kod Morse (Huruf A-Z Sahaja)
char decodeMorse(String code) {
  if (code == ".-") return 'A';
  if (code == "-...") return 'B';
  if (code == "-.-.") return 'C';
  if (code == "-..") return 'D';
  if (code == ".") return 'E';
  if (code == "..-.") return 'F';
  if (code == "--.") return 'G';
  if (code == "....") return 'H';
  if (code == "..") return 'I';
  if (code == ".---") return 'J';
  if (code == "-.-") return 'K';
  if (code == ".-..") return 'L';
  if (code == "--") return 'M';
  if (code == "-.") return 'N';
  if (code == "---") return 'O';
  if (code == ".--.") return 'P';
  if (code == "--.-") return 'Q';
  if (code == ".-.") return 'R';
  if (code == "...") return 'S';
  if (code == "-") return 'T';
  if (code == "..-") return 'U';
  if (code == "...-") return 'V';
  if (code == ".--") return 'W';
  if (code == "-..-") return 'X';
  if (code == "-.--") return 'Y';
  if (code == "--..") return 'Z';
  return '?';
}

void updateDisplay(String statusTag) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Bahagian 1: Papar Kod Morse yang ditulis
  display.setCursor(0, 0);
  display.print("MORSE (");
  display.print(statusTag);
  display.println("):");
  display.setCursor(0, 10);
  display.println(fullMorseHistory);

  display.drawLine(0, 21, 128, 21, SSD1306_WHITE);

  // Bahagian 2: Papar Hasil Convert ke Huruf (A-Z)
  display.setCursor(0, 24);
  display.println("HURUF:");
  display.setCursor(0, 34);
  display.println(convertedText);

  display.drawLine(0, 44, 128, 44, SSD1306_WHITE);

  // Bahagian 3: Papar Mesej Masuk dari Rakan
  display.setCursor(0, 47);
  display.print("IN: ");
  display.println(rxBuffer);

  display.display();
}

void sendLoRaMessage(String msg) {
  updateDisplay("HANTAR");
  LoRa.beginPacket();
  LoRa.print(SECRET_ID + msg); // Selitkan kod pengesahan sebelum hantar
  LoRa.endPacket();
  SerialBT.println("Hantar: " + msg);
  updateDisplay("TERHANTAR");
}

void beepIncoming() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(80);
    digitalWrite(BUZZER_PIN, LOW);
    delay(80);
  }
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("LoRa_Morse_Radio");

  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true);
  }
  updateDisplay("MULA");

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    updateDisplay("LORA ERR");
    while (true);
  }

  // Talaan Jarak Jauh & Saluran Peribadi
  LoRa.setTxPower(20);
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(62.5E3);
  LoRa.setCodingRate4(8);
  LoRa.setSyncWord(PRIVATE_SYNC_WORD); // Aktifkan saluran peribadi

  updateDisplay("SEDIA");
}

void loop() {
  unsigned long now = millis();
  int btnState = digitalRead(BTN_PIN);

  // 1. Apabila Butang Ditekan
  if (btnState == LOW && !isPressing) {
    isPressing = true;
    pressStartTime = now;
    digitalWrite(BUZZER_PIN, HIGH);

    if (messageSent) {
      fullMorseHistory = "";
      convertedText    = "";
      messageSent      = false;
    }
  }

  // 2. Apabila Butang Dilepaskan -> Papar . atau - serta-merta
  if (btnState == HIGH && isPressing) {
    isPressing = false;
    digitalWrite(BUZZER_PIN, LOW);
    unsigned long duration = now - pressStartTime;
    lastReleaseTime = now;

    if (duration > 25) {
      if (duration < DOT_MAX_MS) {
        currentLetterMorse += ".";
        fullMorseHistory   += ".";
      } else {
        currentLetterMorse += "-";
        fullMorseHistory   += "-";
      }
      letterConverted = false;
      spaceAdded      = false;
      updateDisplay("TAIP");
    }
  }

  // 3. Convert Kod Morse ke Huruf (A-Z) selepas jeda 650ms
  if (!isPressing && !letterConverted && (now - lastReleaseTime > LETTER_GAP_MS)) {
    char letter = decodeMorse(currentLetterMorse);
    convertedText      += letter;
    fullMorseHistory   += " ";
    currentLetterMorse  = "";
    letterConverted     = true;
    updateDisplay("CONVERT");
  }

  // 4. Tambah jarak (Space) antara perkataan selepas jeda 1.4 saat
  if (!isPressing && letterConverted && !spaceAdded && (now - lastReleaseTime > WORD_GAP_MS)) {
    convertedText    += " ";
    fullMorseHistory += "/ ";
    spaceAdded        = true;
    updateDisplay("JARAK");
  }

  // 5. Hantar mesej secara automatik selepas berhenti menaip 3 saat
  if (!isPressing && letterConverted && !messageSent && (now - lastReleaseTime > SEND_GAP_MS)) {
    convertedText.trim();
    if (convertedText.length() > 0) {
      sendLoRaMessage(convertedText);
      messageSent = true;
    }
  }

  // 6. Terima & tapis mesej dari rakan melalui LoRa
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }

    // Semak jika mesej mengandungi kod pengesahan SECRET_ID
    if (incoming.startsWith(SECRET_ID)) {
      incoming.remove(0, SECRET_ID.length()); // Buang tag rahsia supaya skrin papar teks bersih
      rxBuffer = incoming;
      SerialBT.println("Terima: " + rxBuffer);
      beepIncoming();
      updateDisplay("BARU");
    }
    // Jika isyarat rawak dari orang luar (tiada SECRET_ID), kod akan terus mengabaikannya
  }
}
