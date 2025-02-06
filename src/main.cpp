#include <Arduino.h>

#ifndef DEVICE_BOARD_NAME
#  define DEVICE_BOARD_NAME "TechRSMQTTgateway"
#endif

#define DEVICE_HOSTNAME "RSmqttGW"

#include "controlWiFi.h"

#include "MQTT_task.h"

#include <esp_task_wdt.h>
#define WDT_TIMEOUT 60


//Port if Built-in LED
#define LED_BUILTIN 22

//Ports of pull-up transistors
#define RX_PULLUP 21
#define TX_PULLUP 22

hw_timer_t *Timer0_Cfg = NULL;

#include "CTechManager.h"
CTechManager techManager;

bool publishMQTT = false;
int mqtt_num_attempts = 0;
const int max_mqtt_attempts = 600;
ulong RSStamp = 0;
ulong RSDelay = 60 * 1000;

void readRS()
{
  Stream* response = nullptr;
  //techManager.SendCommand((CTechManager::ETechCommand)cmd, val);
  Serial.println("Execute readRS fucntion");
  techManager.GetStateJson(*response);
  //techManager.GetStatsJson(*response, CTechManager::EStatsType::co);
  //techManager.GetStatsJson(*response, CTechManager::EStatsType::cwu);
  //techManager.GetStatsJson(*response, CTechManager::EStatsType::ext);
  //Serial.print(response->readString());
}

void publishStateJsonMQTT() {
  digitalWrite(LED_BUILTIN, HIGH);
  if (MQTTMessageCallback())
  {
    mqtt_num_attempts = 0;
  } else {
    mqtt_num_attempts++;
  }
  digitalWrite(LED_BUILTIN, LOW);
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Start");

  pinMode(LED_BUILTIN, OUTPUT);

  esp_reset_reason_t reason = esp_reset_reason();
  switch (reason) {
    case ESP_RST_POWERON:
      Serial.println("Reset due to power-on event");
    break;
  
    case ESP_RST_SW:
      Serial.println("Software reset via esp_restart");
    break;

    case ESP_RST_WDT:
      Serial.println("Rebooted by Watchdog!");
    break;
  }

  //pinMode(rx_pin, INPUT);
  //pinMode(tx_pin, OUTPUT);
  pinMode(RX_PULLUP, OUTPUT);
  pinMode(TX_PULLUP, OUTPUT);
  digitalWrite(RX_PULLUP, HIGH);
  digitalWrite(TX_PULLUP, HIGH);

  //Set witchdog timeout for 32 seconds
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);

  while (esp_task_wdt_status(NULL) != ESP_OK) {
    // LED blinks indefinitely
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }

  Serial.print("Start WiFi on ");
  Serial.println(DEVICE_BOARD_NAME);
  
  initializeWiFi(DEVICE_HOSTNAME);
  
  establishWiFi();

  // you're connected now, so print out the data
  printWifiStatus();

  initMQTT();

  // Start tech manager.
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  techManager.SetStream(&Serial2);
  techManager.SetStatsDelay(30);
}

void loop()
{
  techManager.Update();
  MQTTLoop();
  if (millis() - RSStamp > RSDelay)
  {
    RSStamp = millis();
    readRS();
    //publishStateJsonMQTT();
  }
  //check if long time no mqtt publish
  if (mqtt_num_attempts < max_mqtt_attempts)
  {
    esp_task_wdt_reset();
  }
}