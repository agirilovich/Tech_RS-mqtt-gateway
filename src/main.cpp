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
#define RX_PULLUP 16
#define TX_PULLUP 17

//Global variables in Master<>Slave communication
float SetPoint = 0;
bool FlameOn = false;
float MaxModulationLevel = 0;
float RoomSetPoint = 0;
float RoomTemperature = 0;

hw_timer_t *Timer0_Cfg = NULL;


#include <SoftwareSerial.h>
int rx_pin = 2;
int tx_pin = 5;
SoftwareSerial COSerial(rx_pin, tx_pin);

#include "CTechManager.h"
CTechManager techManager;

void IRAM_ATTR handleInterrupt() {
    ot.handleInterrupt();
}

bool publishMQTT = false;
int mqtt_num_attempts = 0;
const int max_mqtt_attempts = 600;
void IRAM_ATTR Timer0_ISR()
{
  readRS;

  publishMQTT = true;
}

String readRS() {
  /*
  int inChar;
  String inStr = "";
  char buff[2];
  long startTime = millis();

  if (COSerial.available()) {
    while (millis() - startTime < 1500) {
      inChar = -1;
      inChar = COSerial.read();
      if (inChar > -1) {
        sprintf(buff,"%02X",inChar);
        inStr = inStr + buff;
      }
    }
  }
  return inStr;
  */
  techManager.SendCommand((CTechManager::ETechCommand)cmd, val);
  techManager.GetStateJson(*response);
  publishMQTT(response);
}

void (publishMQTT) {
  digitalWrite(LED_BUILTIN, HIGH);
  if (MQTTMessageCallback(SetPoint, FlameOn, MaxModulationLevel, RoomSetPoint, RoomTemperature))
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

  pinMode(rx_pin, INPUT);
  pinMode(tx_pin, OUTPUT);
  pinMode(RX_PULLUP, OUTPUT);
  pinMode(TX_PULLUP, OUTPUT);
  digitalWrite(RX_PULLUP, HIGH);
  digitalWrite(TX_PULLUP, HIGH);

  //Set the software port for communication with Tech controller
  COSerial.begin(9600);
  while (!COSerial);
  Serial.println("SW Serial - Ready");

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

  Setup Hardware Timer for MQTT publish
  Timer0_Cfg = timerBegin(0, 300, true);
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
  timerAlarmWrite(Timer0_Cfg, 10000000, true);
  timerAlarmEnable(Timer0_Cfg);

}


void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);
  techManager.Update();
  digitalWrite(LED_BUILTIN, LOW);
  MQTTLoop();
  //check if long time no mqtt publish
  if (mqtt_num_attempts < max_mqtt_attempts)
  {
    esp_task_wdt_reset();
  }
}