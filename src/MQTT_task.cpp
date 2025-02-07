#include "MQTT_task.h"
#include "controlWiFi.h"
#include "Credentials.h"

const char *mqtt_host = mqtt_server;
const int mqtt_port = 1883;
const char *mqtt_user = mqtt_username;
const char *mqtt_pass = mqtt_password;

WiFiClient client;

#include <PubSubClient.h>
PubSubClient mqtt(client);

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

void initMQTT() {
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

      //Define Sensors
      HAMQTT.addEntity(ha_switch);

      //Assign incoming message callback
      HAMQTT.setCallback(MQTTMessageCallback);
      
  } else {
    Serial.println("MQTT connection is not established, ignoring");
  }
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
