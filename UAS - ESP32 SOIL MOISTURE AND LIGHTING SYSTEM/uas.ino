#include <Arduino.h>

// ===== PIN =====
#define LDR_PIN   35
#define SOIL_PIN  34
#define LED_PIN   26
#define PUMP_PIN  27

// ===== THRESHOLD =====
#define LIGHT_THRESHOLD  80  
#define SOIL_THRESHOLD   2300 

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

// ===== TASK KONTROL AKTUATOR =====
void taskControl(void *parameter) {
  while (1) {
    if (isDark) {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("[CTRL] GELAP -> LED ON");
    } else {
      digitalWrite(LED_PIN, LOW);
      Serial.println("[CTRL] TERANG -> LED OFF");
    }

    if (isDry) {
      digitalWrite(PUMP_PIN, LOW);
      Serial.println("[CTRL] TANAH KERING -> POMPA ON");
    } else {
      digitalWrite(PUMP_PIN, HIGH);
      Serial.println("[CTRL] TANAH LEMBAB -> POMPA OFF");
    }

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);

  analogSetAttenuation(ADC_11db);

  xTaskCreate(taskLightSensor, "Light Task", 2048, NULL, 1, NULL);
  xTaskCreate(taskSoilSensor,  "Soil Task",  2048, NULL, 1, NULL);
  xTaskCreate(taskControl,     "Control",    2048, NULL, 2, NULL);
}

void loop() {
}
