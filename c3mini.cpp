#include <Arduino.h>

#define pump      4 
#define safety    5  
#define sensor          A0 

volatile int currentSensorValue = 0;
volatile bool emergencyShutdownActive = false;

TaskHandle_t SafetyTaskHandle = NULL;
TaskHandle_t SerialTaskHandle = NULL;


void SafetyLoop(void * pvParameters) {
  pinMode(safety, OUTPUT);
  digitalWrite(safety, HIGH);

  for(;;) {
    currentSensorValue = analogRead(sensor);
    if (currentSensorValue > 800) {
      digitalWrite(safety, LOW);
      digitalWrite(pump, LOW);
      emergencyShutdownActive = true;
    } else {
      emergencyShutdownActive = false;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void SerialLoop(void * pvParameters) {
  Serial.begin(115200);
  pinMode(pump, OUTPUT);

  for(;;) {
    if (Serial.available() > 0) {
      char cmd = Serial.read();
      
      switch(cmd) {
        case 'P':
          if (!emergencyShutdownActive) {
            digitalWrite(pump, HIGH);
            Serial.println("ACK_PUMP_ON");
          } else {
            Serial.println("ERR_SAFETY_TRIPPED");
          }
          break;

        case 'p':
          digitalWrite(pump, LOW);
          Serial.println("ACK_PUMP_OFF");
          break;

        case 'R':
          Serial.print("DATA:");
          Serial.print(currentSensorValue);
          Serial.print(",");
          Serial.println(emergencyShutdownActive ? "FAULT" : "OK");
          break;
          
        default:
          Serial.println("ERR_UNKNOWN_CMD");
          break;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void setup() {
  xTaskCreatePinnedToCore(
    SafetyLoop,           
    "SafetyTask",         
    2048,                 /* stack size*/
    NULL,                 /* input parameter */
    3,                    /* priority(3 = highest) */
    &SafetyTaskHandle,    /* task handle */
    0                     /* core 0 */
  );

  xTaskCreatePinnedToCore(
    SerialLoop,
    "SerialTask",
    2048,
    NULL,
    1,                    /* lower priority, can't block safety loop */
    &SerialTaskHandle,
    0
  );
}

void loop() {
  // leave empty FreeRTOS handles execution
}
