#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// ===== PIN & THRESHOLD =====
#define LDR_PIN   35
#define SOIL_PIN  34
#define LED_PIN   26
#define PUMP_PIN  27
#define LIGHT_THRESHOLD  80  
#define SOIL_THRESHOLD   2300 

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct struct_message {
    int lightIntensity;
    int soilMoisture;
    bool isDark;
    bool isDry;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

volatile int lightIntensity = 0;
volatile int soilMoisture   = 0;
volatile bool isDark  = false;
volatile bool isDry   = false;

void taskLightSensor(void *parameter) {
  while (1) {
    lightIntensity = analogRead(LDR_PIN);
    isDark = (lightIntensity > LIGHT_THRESHOLD);
    Serial.print("[LIGHT] Intensity: ");
    Serial.println(lightIntensity);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void taskSoilSensor(void *parameter) {
  while (1) {
    soilMoisture = analogRead(SOIL_PIN);
    isDry = (soilMoisture > SOIL_THRESHOLD);
    Serial.print("[SOIL] Moisture: ");
    Serial.println(soilMoisture);
    vTaskDelay(700 / portTICK_PERIOD_MS);
  }
}

void taskControl(void *parameter) {
  while (1) {
    if (isDark) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }

    if (isDry) {
      digitalWrite(PUMP_PIN, LOW); 
    } else {
      digitalWrite(PUMP_PIN, HIGH); 
    }

    myData.lightIntensity = lightIntensity;
    myData.soilMoisture = soilMoisture;
    myData.isDark = isDark;
    myData.isDry = isDry;

    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  analogSetAttenuation(ADC_11db);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("#G4G4L menginisialisasi ESP-NOW!");
    return;
  }

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Gagal nambahin peer");
    return;
  }

  xTaskCreate(taskLightSensor, "Light Task", 2048, NULL, 1, NULL);
  xTaskCreate(taskSoilSensor,  "Soil Task",  2048, NULL, 1, NULL);
  xTaskCreate(taskControl,      "Control",    2048, NULL, 2, NULL);
}

void loop() {
}