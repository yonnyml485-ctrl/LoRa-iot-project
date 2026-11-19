# ESP32 LoRa Kod Morse
# 📡 ESP32 LoRa Off-Grid Morse Code Communicator

Alat komunikasi **Kod Morse & Teks jarak jauh tanpa Internet** berasaskan **ESP32** dan **LoRa 433MHz (SX1278)**. Dilengkapi dengan **Skrin OLED 0.96"** untuk paparan simbol Morse secara langsung serta terjemahan automatik ke huruf (A–Z), **Buzzer**, **Butang Fizikal**, dan sokongan **Bluetooth ke Telefon Pintar**.

---

## ✨ Ciri-Ciri Utama
* **100% Tanpa Internet (*Off-Grid*):** Berkomunikasi terus dari peranti ke peranti menggunakan gelombang radio 433MHz (jarak hingga 2 km – 5 km+ bergantung pada antena & halangan).
* **Paparan Dwi-Baris OLED 0.96":** Memaparkan simbol titik (`.`) dan sempang (`-`) secara masa nyata sebelum ditukar (*convert*) secara automatik kepada huruf **A–Z**.
* **Saluran Peribadi (*Private Network*):** Menggunakan `SyncWord` dan `SECRET_ID` supaya peranti hanya menerima mesej daripada kumpulan anda dan menapis isyarat rawak dari luar.
* **Mod Hibrid Telefon (Bluetooth Serial):** Boleh menaip dan membaca mesej panjang terus dari telefon pintar menggunakan aplikasi *Serial Bluetooth Terminal*.

---

## 🛠️ Senarai Komponen (*Hardware*)
*(Sediakan 2 set untuk komunikasi dua hala)*
* **Board Mikropengawal:** ESP32 Dev Module (30-pin / 38-pin)
* **Modul Radio:** LoRa SX1278 Ra-02 / Ra-01 (433MHz)
* **Antena:** Antena 433MHz 5dBi/7dBi (SMA-Male) + Kabel *Pigtail* IPEX U.FL ke SMA-Female
* **Paparan:** Skrin OLED 0.96" I2C (SSD1306, 128×64)
* **Input & Audio:** 1× *Push Button* & 1× *Active Buzzer* 3.3V

---

## 🔌 Rajah Sambungan Pin (*Wiring*)

| Komponen | Pin Komponen | Pin ESP32 | Catatan |
| :--- | :--- | :--- | :--- |
| **LoRa Ra-02** | `3.3V` / `GND` | `3V3` / `GND` | **Wajib 3.3V** (Jangan sambung ke 5V) |
| **LoRa Ra-02** | `NSS` (CS) | `GPIO 5` | SPI Chip Select |
| **LoRa Ra-02** | `SCK` | `GPIO 18` | SPI Clock |
| **LoRa Ra-02** | `MOSI` | `GPIO 23` | SPI MOSI |
| **LoRa Ra-02** | `MISO` | `GPIO 19` | SPI MISO |
| **LoRa Ra-02** | `RST` | `GPIO 14` | Reset |
| **LoRa Ra-02** | `DIO0` | `GPIO 2` | Interrupt |
| **OLED 0.96"** | `VCC` / `GND` | `3V3` / `GND` | Kuasa I2C |
| **OLED 0.96"** | `SDA` / `SCL` | `GPIO 21` / `GPIO 22` | Data & Clock I2C |
| **Push Button** | Kaki 1 / Kaki 2 | `GPIO 4` / `GND` | Menggunakan `INPUT_PULLUP` |
| **Active Buzzer** | `+` / `-` | `GPIO 15` / `GND` | Nada *beep* masuk & keluar |

---

## 💻 Persediaan Perisian (Arduino IDE)

1. Pasang pakej papan **ESP32 by Espressif Systems** melalui *Boards Manager*.
2. Pasang 3 *library* berikut melalui *Library Manager*:
   * **LoRa** (oleh *Sandeep Mistry*)
   * **Adafruit SSD1306**
   * **Adafruit GFX Library**
3. Buka fail `LoRa_Morse.ino`, tetapkan kod saluran peribadi anda di bahagian atas kod (pastikan sama untuk semua ahli kumpulan):
   ```cpp
   const byte PRIVATE_SYNC_WORD = 0xA5;
   const String SECRET_ID       = "KUMPULAN1:";
   ```
4. Pilih **Board: ESP32 Dev Module** dan klik **Upload** ke kedua-dua unit ESP32.

---

## 📖 Cara Penggunaan
1. **Pastikan antena 433MHz dipasang** sebelum menghidupkan kuasa USB ESP32.
2. **Menaip Kod Morse:**
   * Tekan butang **pendek (< 250ms)** untuk **Titik (`.`)**.
   * Tekan butang **lama (≥ 250ms)** untuk **Sempang (`-`)**.
   * Berhenti **0.65 saat** untuk menukar simbol kepada **Huruf (A–Z)**.
   * Berhenti **1.4 saat** untuk menambah **Jarak (*Space*)**.
   * Berhenti **3.0 saat** untuk **menghantar mesej secara automatik** melalui LoRa.
3. **Mod Telefon Pintar:** Sambungkan Bluetooth telefon ke **`LoRa_Morse_Radio`** dan gunakan aplikasi *Serial Bluetooth Terminal* untuk menaip ayat penuh.

