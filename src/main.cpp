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

#include "CTechManager.h"
CTechManager techManager;

int mqtt_num_attempts = 0;
const int max_mqtt_attempts = 60;


ulong SendStamp = 0;
ulong SendDelay = 60 * 1000;

struct SensorsData readRS()
{
  struct SensorsData SensorsCurrentValues;

  //int dev_hours = ((int)techManager.GetState(CTechManager::ETechCommand::DEVICE_TIME) >> 8) & 0xFF;
  //int dev_minutes = ((int)techManager.GetState(CTechManager::ETechCommand::DEVICE_TIME) & 0xFF;
  //setTime(dev_hours, dev_minutes, 0, day(now()), month(now()), year(now()));

  SensorsCurrentValues.device_time = techManager.GetState(CTechManager::ETechCommand::DEVICE_TIME);
  SensorsCurrentValues.device_state = techManager.GetState(CTechManager::ETechCommand::DEVICE_STATE);
  SensorsCurrentValues.ext_temp = techManager.GetState(CTechManager::ETechCommand::EXTERNAL_TEMP);
  SensorsCurrentValues.co_temp = techManager.GetState(CTechManager::ETechCommand::CO_TEMP);
  SensorsCurrentValues.co_temp_ret = techManager.GetState(CTechManager::ETechCommand::CO_TEMP_RET);
  SensorsCurrentValues.cwu_temp = techManager.GetState(CTechManager::ETechCommand::CWU_TEMP);
  SensorsCurrentValues.cwu_temp_ret = techManager.GetState(CTechManager::ETechCommand::CWU_TEMP_RET);
  SensorsCurrentValues.cwu_temp_set = techManager.GetState(CTechManager::ETechCommand::CWU_TEMP_SET);
  SensorsCurrentValues.pump_state_co = techManager.GetState(CTechManager::ETechCommand::PUMP_STATE_CO);
  SensorsCurrentValues.pump_state_cwu = techManager.GetState(CTechManager::ETechCommand::PUMP_STATE_CWU);

  for (int i = 0; i <= 1; i++)
  {
    SensorsCurrentValues.valveData[i].mix_valve_state = techManager.GetState(CTechManager::ETechCommand::VALVE_STATE, i + 1);
    SensorsCurrentValues.valveData[i].mix_valve_openLevel = techManager.GetState(CTechManager::ETechCommand::VALVE_OPEN_LEVEL, i + 1);
    SensorsCurrentValues.valveData[i].mix_valve_type = techManager.GetState(CTechManager::ETechCommand::VALVE_TYPE, i + 1);
    SensorsCurrentValues.valveData[i].mix_valve_temp_set = techManager.GetState(CTechManager::ETechCommand::VALVE_TEMP_SET, i + 1);
    SensorsCurrentValues.valveData[i].mix_valve_temp = techManager.GetState(CTechManager::ETechCommand::VALVE_TEMP, i + 1);
    SensorsCurrentValues.valveData[i].mix_valve_pump = techManager.GetState(CTechManager::ETechCommand::VALVE_PUMP_STATE, i + 1);
  }
 
  SensorsCurrentValues.pump_mode = techManager.GetState(CTechManager::ETechCommand::PUMP_MODE);
    
  return SensorsCurrentValues;
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

  initMQTT();
  
  initializeWiFi(DEVICE_HOSTNAME);
  
  establishWiFi();

  // you're connected now, so print out the data
  printWifiStatus();

  // Start tech manager.
  Serial2.begin(9600, SERIAL_8N1, 16, 17, true); // Invers TTL for Serial2 port on some ESP32 boards
  while (!Serial2);
  Serial.println("HW port Serial2 - Ready");
  techManager.SetStream(&Serial2);
}

String readSerial()
{
  int inChar;
  String inStr = "";
  char buff[2];
  long startTime = millis();

  if (Serial2.available())
  {
    while (millis() - startTime < 1500)
    {
      inChar = -1;
      inChar = Serial2.read();
      if (inChar > -1)
      {
        sprintf(buff,"%02X",inChar);
        inStr = inStr + buff;
      }
    }
  }
  return inStr;
}

void loop()
{
  /*
  String fromSerial = readSerial();
  if (fromSerial.length() > 0)
  {
    Serial.println("dane z CO:");
    Serial.println(fromSerial);
    Serial.println("=============");
  }
  */
  techManager.Update();
  MQTTLoop();
  if (millis() - SendStamp > SendDelay)
  {
    SendStamp = millis();
    struct SensorsData SensorsCurrentValues;
    Serial.print("Read and Set sensors values...");
    SensorsCurrentValues = readRS();
    Serial.println("Done.");
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print("Publish sensors values via MQTT....");
    if (MQTTpublish(&SensorsCurrentValues))
    {
      mqtt_num_attempts = 0;
      Serial.println("Done");
    } else {
    mqtt_num_attempts++;
    Serial.println("Failed. Skip the cicle.");
    }
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("======================================================================");
  }
  //check if long time no mqtt publish
  if (mqtt_num_attempts < max_mqtt_attempts)
  {
    esp_task_wdt_reset();
  }
}