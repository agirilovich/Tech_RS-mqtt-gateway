#include "MQTT_task.h"
#include "controlWiFi.h"
#include "Credentials.h"

//Define MQTT Topic for HomeAssistant Discovery
const char *MQTTTSetTopicConfig = MQTT_TOPIC_TSET_CONFIG;
const char *MQTTFlameOnTopicConfig = MQTT_TOPIC_FLAMEON_CONFIG;
const char *MQTTMaxRelModLevelSettingTopicConfig = MQTT_TOPIC_MAXMODLEVEL_CONFIG;
const char *MQTTTrSetTopicConfig = MQTT_TOPIC_TRSET_CONFIG;
const char *MQTTTrTopicConfig = MQTT_TOPIC_ROOMTEMP_CONFIG;


//Define MQTT Topic for HomeAssistant Sensor state
const char *MQTTTSetTopicState = MQTT_TSET_TOPIC_STATE;
const char *MQTTFlameOnTopicState = MQTT_FLAMEON_TOPIC_STATE;
const char *MQTTMaxRelModLevelSettingTopicState = MQTT_MAXMODLEVEL_TOPIC_STATE;
const char *MQTTTrSetTopicState = MQTT_TRSET_TOPIC_STATE;
const char *MQTTTrTopicState = MQTT_ROOMTEMP_TOPIC_STATE;

const char *mqtt_host = mqtt_server;
const int mqtt_port = 1883;
const char *mqtt_user = mqtt_username;
const char *mqtt_pass = mqtt_password;

//Define objects for MQTT messages in JSON format
#include <ArduinoJson.h>
StaticJsonDocument<512> JsonSensorConfig;

char Buffer[1024];

WiFiClient client;

#include <PubSubClient.h>
PubSubClient mqtt(client);

float OutsideTemperature = 0;

void CallbackMQTTmessage(char* topic, byte* payload, unsigned int length)
{
  StaticJsonDocument<512> JsonTemperaturePayload;
  deserializeJson(JsonTemperaturePayload, (const byte*)payload, length);
  float temp = JsonTemperaturePayload["temperature_C"];
  if (temp != 0) {
    Serial.print("Outdoor Temperature Message arrived [ ");
    OutsideTemperature = temp;
    Serial.print(temp);
    Serial.println(" C ] ");
  }
}

void initMQTT()
{
  Serial.print("Connecting to MQTT broker host: ");
  Serial.println(mqtt_host);
  if (!client.connect(mqtt_host, mqtt_port))
  {
    Serial.println("Connected to MQTT host!");
  
    //Initialise MQTT autodiscovery topic and sensor
    mqtt.setServer(mqtt_host, mqtt_port);

    Serial.print("Testing connection to mqtt broker...");
    if (mqtt.connect(DEVICE_BOARD_NAME, mqtt_user, mqtt_pass))
    {
      Serial.println(" connected!");

      //CH water temp. setpoint
      JsonSensorConfig["name"] = "CH water temp. setpoint";
      JsonSensorConfig["device_class"] = "temperature";
      JsonSensorConfig["state_class"] = "measurement";
      JsonSensorConfig["unit_of_measurement"] = "C";
      JsonSensorConfig["uniq_id"] = "chwatersetpointC";
      JsonSensorConfig["state_topic"] = MQTTTSetTopicState;

      JsonObject device  = JsonSensorConfig.createNestedObject("device");
      device["identifiers"][0] = "techrsgw000";
      device["connections"][0][0] = "mac";
      device["connections"][0][1] = "08:B6:1F:33:7D:AC";
      device["model"] = "TECH-RS-GW-01";
      device["name"] = SENSOR_NAME;
      device["manufacturer"] = "handmade"; 
      device["sw_version"] = "1.0";  
      serializeJson(JsonSensorConfig, Buffer);
      initializeMQTTTopic(MQTTTSetTopicConfig, Buffer);

      //Flame sensor
      JsonSensorConfig["name"] = "Flame On";
      JsonSensorConfig["device_class"] = "heat";
      JsonSensorConfig["state_class"] = "";
      JsonSensorConfig["unit_of_measurement"] = "";
      JsonSensorConfig["uniq_id"] = "FlameOnstate";
      JsonSensorConfig["state_topic"] = MQTTFlameOnTopicState;

      serializeJson(JsonSensorConfig, Buffer);
      initializeMQTTTopic(MQTTFlameOnTopicConfig, Buffer);

      //Maximum relative modulation level setting
      JsonSensorConfig["name"] = "Max modulation level";
      JsonSensorConfig["device_class"] = "power_factor";
      JsonSensorConfig["state_class"] = "measurement";
      JsonSensorConfig["unit_of_measurement"] = "%";
      JsonSensorConfig["uniq_id"] = "chmaxmodulationlvl";
      JsonSensorConfig["state_topic"] = MQTTMaxRelModLevelSettingTopicState;

      serializeJson(JsonSensorConfig, Buffer);
      initializeMQTTTopic(MQTTMaxRelModLevelSettingTopicConfig, Buffer);

      //Room Setpoint sensor
      JsonSensorConfig["name"] = "Room setpoint";
      JsonSensorConfig["device_class"] = "temperature";
      JsonSensorConfig["state_class"] = "measurement";
      JsonSensorConfig["unit_of_measurement"] = "C";
      JsonSensorConfig["uniq_id"] = "chroomsetpoint";
      JsonSensorConfig["state_topic"] = MQTTTrSetTopicState;

      serializeJson(JsonSensorConfig, Buffer);
      initializeMQTTTopic(MQTTTrSetTopicConfig, Buffer);

      //Room Temperature
      JsonSensorConfig["name"] = "Room Temperature";
      JsonSensorConfig["device_class"] = "temperature";
      JsonSensorConfig["state_class"] = "measurement";
      JsonSensorConfig["unit_of_measurement"] = "C";
      JsonSensorConfig["uniq_id"] = "chroomtemperature";
      JsonSensorConfig["state_topic"] = MQTTTrTopicState;

      serializeJson(JsonSensorConfig, Buffer);
      initializeMQTTTopic(MQTTTrTopicConfig, Buffer);

      //Subscribe on Outside temperature sensor state
      //mqtt.setCallback(CallbackMQTTmessage);
      //mqtt.subscribe(MQTTOutsideTemperatureTopicState);
    }
  } else {
    Serial.println("MQTT connection is not established, ignoring");
  }
}

void initializeMQTTTopic(const char *Topic, char *SensorConfig)
{
  
  Serial.println("Initialise MQTT autodiscovery topics and sensors...");
  Serial.println(Topic);
  //Serial.println(SensorConfig);

  //Publish message to AutoDiscovery topic
  if (mqtt.publish(Topic, SensorConfig, true)) {
    Serial.println("Done");
  }
    
  //Gracefully close connection to MQTT broker
}

bool MQTTMessageCallback()
{
  char MessageBuf[16];
  //Publish MQTT messages
  Serial.println("Publishing MQTT messages...");
  //mqtt.connect(DEVICE_BOARD_NAME, mqtt_user, mqtt_pass);
  if (mqtt.connected()) {
/*
    sprintf(MessageBuf, "%d", int(SetPoint));
    mqtt.publish(MQTTTSetTopicState, MessageBuf, false);

    sprintf(MessageBuf, "%s", FlameOn?"ON":"OFF");
    mqtt.publish(MQTTFlameOnTopicState, MessageBuf, false);

    sprintf(MessageBuf, "%d", int(MaxModulationLevel));
    mqtt.publish(MQTTMaxRelModLevelSettingTopicState, MessageBuf, false);

    sprintf(MessageBuf, "%d", int(RoomSetPoint));
    mqtt.publish(MQTTTrSetTopicState, MessageBuf, false);

    sprintf(MessageBuf, "%d", int(RoomTemperature));
    mqtt.publish(MQTTTrTopicState, MessageBuf, false);
*/
    Serial.println("Done");
  
  }
  else {
    Serial.println("Unable to connect to MQTT broker");
    Serial.println("Cycle is skipped");
    Serial.println("Trying to reconnect");
    initMQTT();
    return(false);

  }
  //mqtt.disconnect();
  return(true);
}

void MQTTLoop()
{
  mqtt.loop();
}
