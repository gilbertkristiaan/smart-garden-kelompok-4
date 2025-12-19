# Embedded Systems Project – Soil Moisture & Smart Garden

* Abhiseka Susanto
* Gilbert Kristian
* Calvin Joy Tarigan
* Ivan Jehuda Angi

## Daftar Proyek

* [**AVR XMEGA A3BU (Soil Moisture System)**](#avr-xmega-a3bu-soil-moisture-system)
* [**ESP32 (Soil Moisture & Lighting System)**](#esp32-soil-moisture--lighting-system)

---

## AVR XMEGA A3BU (Soil Moisture System)

### Deskripsi

Proyek ini berfokus papada sistem penyiraman tanaman otomatis berbasis AVR XMEGA A3BU. Sistem ini berfokus pada pembacaan kelembaban tanah dan kontrol pompa air menggunakan _relay_.

Sistem bekerja secara _standalone_ (tanpa konektivitas jaringan) dan menampilkan informasi secara real-time melalui _LCD onboard_.

### Tujuan

* Mengotomatisasi penyiraman tanaman berdasarkan kelembaban tanah
* Menampilkan status sistem secara langsung

### Komponen Utama

* AVR XMEGA A3BU Xplained
* Sensor kelembaban tanah (resistif, ADC PA2)
* _Relay_ 5V + pompa air DC
* Transistor _level shifter_ (3.3V ke 5V)
* _LCD onboard_

### Cara Kerja

* Sensor membaca nilai ADC (12-bit)
* Nilai dikonversi menjadi persentase kelembaban
* Jika < 50%, pompa akan ON
* Jika >= 50%, pompa akan OFF
* Data ditampilkan pada LCD
* Metode: _Super-loop_ + ADC _Averaging_ + EMA Filter

---

## ESP32 (Soil Moisture & Lighting System)

### Deskripsi

Proyek ini lanjutan dari proyek UTS dengan menggunakan ESP32. Sistem ini akan mengatur kelembaban tanah dan pencahayaan tanaman (_grow light_).

Sistem menggunakan FreeRTOS untuk _multitasking_ dan ESP-NOW untuk komunikasi nirkabel sehingga berfungsi sebagai _node_ IoT.

### Tujuan

* Mengelola air dan cahaya tanaman secara bersamaan
* Menghindari _blocking_ dengan RTOS
* Mengirim data kondisi tanaman secara nirkabel

### Komponen Utama

* ESP32
* Sensor kelembaban tanah (GPIO 34)
* Sensor cahaya LDR (GPIO 35)
* _Relay_ & pompa air
* LED Grow Light (GPIO 26)
* _Transistor level shifter_

### Arsitektur Sistem

Sistem dibagi menjadi beberapa Task FreeRTOS:

1. _Task Soil Sensor_ - membaca kelembaban tanah
2. _Task Light Sensor_ - membaca intensitas cahaya
3. _Task Control & Communication_ - kontrol aktuator & kirim data ESP-NOW

### Fitur Utama

* Multitasking berbasis FreeRTOS
* Kontrol pompa & grow light otomatis
* Komunikasi nirkabel ESP-NOW (broadcast)
* Sistem siap dikembangkan menjadi Smart Garden
* RTOS: FreeRTOS (built-in ESP32)
* Protokol: ESP-NOW

---
