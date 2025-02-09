#include "MQTT_task.h"
#include "controlWiFi.h"


const char *mqtt_host = mqtt_server;
const int mqtt_port = 1883;
const char *mqtt_user = mqtt_username;
const char *mqtt_pass = mqtt_password;

WiFiClient client;

#include <PubSubClient.h>
PubSubClient mqtt(client);

// Define Device in Home Assistant scope of integrations
HADevice ha_device(DEVICE_BOARD_NAME, DEVICE_BOARD_NAME, "1.0");

// Define Home Assistant sensors
HASensorText device_time = HASensorText("device_time", "Device Time", ha_device, 32);
HASensorNumeric device_state = HASensorNumeric("device_state", "Device State", ha_device);
HASensorNumeric ext_temp = HASensorNumeric("ext_temp", "Temperature outside", ha_device, "°C");
HASensorNumeric co_temp = HASensorNumeric("co_temp", "Temperature CO", ha_device, "°C");
HASensorNumeric co_temp_ret = HASensorNumeric("co_temp_ret", "Temperature return CO", ha_device, "°C");
HASensorNumeric cwu_temp = HASensorNumeric("cwu_temp", "Temperature CWU", ha_device, "°C");
HASensorNumeric cwu_temp_ret = HASensorNumeric("cwu_temp_ret", "Temperature return CWU", ha_device, "°C");
HASensorNumeric cwu_temp_set = HASensorNumeric("cwu_temp_set", "Temperature set CWU", ha_device, "°C");
HASensorBinary pump_state_co = HASensorBinary("pump_state_co", "CO pump state", ha_device);
HASensorBinary pump_state_cwu = HASensorBinary("pump_state_cwu", "CWU pump state", ha_device);

// Valve 1
HASensorBinary mix_valve1_state = HASensorBinary("mix_valve1_state", "Mix Valve 1 state", ha_device);
HASensorNumeric mix_valve1_openLevel = HASensorNumeric("mix_valve1_openLevel", "Mix Valve 1 Open Level", ha_device, "%");
HASensorNumeric mix_valve1_type = HASensorNumeric("mix_valve1_type", "Mix Valve 1 Type", ha_device);
HASensorNumeric mix_valve1_temp_set = HASensorNumeric("mix_valve1_temp_set", "Mix Valve 1 Temperature set", ha_device, "°C");
HASensorNumeric mix_valve1_temp = HASensorNumeric("mix_valve1_temp_set", "Mix Valve 1 Temperature", ha_device, "°C");
HASensorBinary mix_valve1_pump_state = HASensorBinary("mix_valve1_pump_state", "Mix Valve 1 pump state", ha_device);

// Valve 2
HASensorBinary mix_valve2_state = HASensorBinary("mix_valve2_state", "Mix Valve 2 state", ha_device);
HASensorNumeric mix_valve2_openLevel = HASensorNumeric("mix_valve2_openLevel", "Mix Valve 2 Open Level", ha_device, "%");
HASensorNumeric mix_valve2_type = HASensorNumeric("mix_valve2_type", "Mix Valve 2 Type", ha_device);
HASensorNumeric mix_valve2_temp_set = HASensorNumeric("mix_valve2_temp_set", "Mix Valve 2 Temperature set", ha_device, "°C");
HASensorNumeric mix_valve2_temp = HASensorNumeric("mix_valve2_temp_set", "Mix Valve 2 Temperature", ha_device, "°C");
HASensorBinary mix_valve2_pump_state = HASensorBinary("mix_valve2_pump_state", "Mix Valve 2 pump state", ha_device);

// Mode
#define OPTIONS_COUNT 4
const char *pump_mode_options[4] PROGMEM = {
     "Grzanie domu",
     "Priorytet bojlera",
     "Pompy równoległe",
     "Tryb letni"
     };
HASelect pump_mode = HASelect("pump_mode", "Mode", ha_device, OPTIONS_COUNT, pump_mode_options);


void initMQTT() {
  //Initialise MQTT autodiscovery topic and sensor
  mqtt.setServer(mqtt_host, mqtt_port);
  HAMQTT.begin(mqtt, 22);

  device_time.addFeature(HA_FEATURE_DEVICE_CLASS, "TIMESTAMP");
  device_time.addFeature(HA_FEATURE_DEVICE_CLASS, "TIMESTAMP");
  ext_temp.addFeature(HA_FEATURE_DEVICE_CLASS, "TEMPERATURE");
  ext_temp.addFeature(HA_FEATURE_ICON,"mdi:home-thermometer-outline");
  co_temp.addFeature(HA_FEATURE_DEVICE_CLASS, "TEMPERATURE");
  co_temp.addFeature(HA_FEATURE_ICON,"mdi:water-thermometer");
  co_temp_ret.addFeature(HA_FEATURE_DEVICE_CLASS, "TEMPERATURE");
  co_temp_ret.addFeature(HA_FEATURE_ICON,"mdi:water-thermometer");
  cwu_temp.addFeature(HA_FEATURE_DEVICE_CLASS, "TEMPERATURE");
  cwu_temp.addFeature(HA_FEATURE_ICON,"mdi:water-thermometer");
  cwu_temp_set.addFeature(HA_FEATURE_DEVICE_CLASS, "TEMPERATURE");
  cwu_temp_set.addFeature(HA_FEATURE_ICON,"mdi:water-thermometer");
  pump_state_co.addFeature(HA_FEATURE_DEVICE_CLASS, "HEAT");
  pump_state_co.addFeature(HA_FEATURE_ICON,"mdi:pump");
  pump_state_cwu.addFeature(HA_FEATURE_DEVICE_CLASS, "HEAT");
  pump_state_cwu.addFeature(HA_FEATURE_ICON,"mdi:pump");

  mix_valve1_state.addFeature(HA_FEATURE_DEVICE_CLASS, "HEAT");
  mix_valve1_state.addFeature(HA_FEATURE_ICON,"mdi:pipe-valve");
  mix_valve1_openLevel.addFeature(HA_FEATURE_DEVICE_CLASS, "POWER_FACTOR");
  mix_valve1_openLevel.addFeature(HA_FEATURE_ICON,"mdi:valve");
  mix_valve1_temp_set.addFeature(HA_FEATURE_DEVICE_CLASS, "TEMPERATURE");
  mix_valve1_temp_set.addFeature(HA_FEATURE_ICON,"mdi:water-thermometer");
  mix_valve1_temp.addFeature(HA_FEATURE_DEVICE_CLASS, "TEMPERATURE");
  mix_valve1_temp.addFeature(HA_FEATURE_ICON,"mdi:water-thermometer");
  mix_valve1_pump_state.addFeature(HA_FEATURE_DEVICE_CLASS, "HEAT");
  mix_valve1_pump_state.addFeature(HA_FEATURE_ICON,"mdi:pump");

  mix_valve2_state.addFeature(HA_FEATURE_DEVICE_CLASS, "HEAT");
  mix_valve2_state.addFeature(HA_FEATURE_ICON,"mdi:pipe-valve");
  mix_valve2_openLevel.addFeature(HA_FEATURE_DEVICE_CLASS, "POWER_FACTOR");
  mix_valve2_openLevel.addFeature(HA_FEATURE_ICON,"mdi:valve");
  mix_valve2_temp_set.addFeature(HA_FEATURE_DEVICE_CLASS, "TEMPERATURE");
  mix_valve2_temp_set.addFeature(HA_FEATURE_ICON,"mdi:water-thermometer");
  mix_valve2_temp.addFeature(HA_FEATURE_DEVICE_CLASS, "TEMPERATURE");
  mix_valve2_temp.addFeature(HA_FEATURE_ICON,"mdi:water-thermometer");
  mix_valve2_pump_state.addFeature(HA_FEATURE_DEVICE_CLASS, "HEAT");
  mix_valve2_pump_state.addFeature(HA_FEATURE_ICON,"mdi:pump");

  HAMQTT.addEntity(device_time);
  HAMQTT.addEntity(device_state);
  HAMQTT.addEntity(ext_temp);
  HAMQTT.addEntity(co_temp);
  HAMQTT.addEntity(co_temp_ret);
  HAMQTT.addEntity(cwu_temp);
  HAMQTT.addEntity(cwu_temp_set);
  HAMQTT.addEntity(pump_state_co);
  HAMQTT.addEntity(pump_state_cwu);

  HAMQTT.addEntity(mix_valve1_state);
  HAMQTT.addEntity(mix_valve1_openLevel);
  HAMQTT.addEntity(mix_valve1_type);
  HAMQTT.addEntity(mix_valve1_temp_set);
  HAMQTT.addEntity(mix_valve1_temp);
  HAMQTT.addEntity(mix_valve1_pump_state);

  HAMQTT.addEntity(mix_valve2_state);
  HAMQTT.addEntity(mix_valve2_openLevel);
  HAMQTT.addEntity(mix_valve2_type);
  HAMQTT.addEntity(mix_valve2_temp_set);
  HAMQTT.addEntity(mix_valve2_temp);
  HAMQTT.addEntity(mix_valve2_pump_state);

  HAMQTT.addEntity(pump_mode);
}

bool MQTTpublish(struct SensorsData* SensorsCurrentValues)
{
  if (WiFi.status() == WL_CONNECTED && !HAMQTT.connected())
  {
    if (HAMQTT.connect(DEVICE_BOARD_NAME, mqtt_user, mqtt_pass))
      Serial.println("Connected to MQTT");
    else
    {
      Serial.println("Failed to connect to MQTT");
      return(false);
    }
  }
  device_time.setState(SensorsCurrentValues->device_time);
  device_state.setState(SensorsCurrentValues->device_state);
  ext_temp.setState(SensorsCurrentValues->ext_temp);
  co_temp.setState(SensorsCurrentValues->co_temp);
  co_temp_ret.setState(SensorsCurrentValues->co_temp_ret);
  cwu_temp.setState(SensorsCurrentValues->cwu_temp);
  cwu_temp_set.setState(SensorsCurrentValues->cwu_temp_set);
  pump_state_co.setState(SensorsCurrentValues->pump_state_co);
  pump_state_cwu.setState(SensorsCurrentValues->pump_state_cwu);

  mix_valve1_state.setState(SensorsCurrentValues->valveData[0].mix_valve_state);
  mix_valve1_openLevel.setState(SensorsCurrentValues->valveData[0].mix_valve_openLevel);
  mix_valve1_type.setState(SensorsCurrentValues->valveData[0].mix_valve_type);
  mix_valve1_temp_set.setState(SensorsCurrentValues->valveData[0].mix_valve_temp_set);
  mix_valve1_temp.setState(SensorsCurrentValues->valveData[0].mix_valve_temp);
  mix_valve1_pump_state.setState(SensorsCurrentValues->valveData[0].mix_valve_pump);

  mix_valve2_state.setState(SensorsCurrentValues->valveData[1].mix_valve_state);
  mix_valve2_openLevel.setState(SensorsCurrentValues->valveData[1].mix_valve_openLevel);
  mix_valve2_type.setState(SensorsCurrentValues->valveData[1].mix_valve_type);
  mix_valve2_temp_set.setState(SensorsCurrentValues->valveData[1].mix_valve_temp_set);
  mix_valve2_temp.setState(SensorsCurrentValues->valveData[1].mix_valve_temp);
  mix_valve2_pump_state.setState(SensorsCurrentValues->valveData[0].mix_valve_pump);

  pump_mode.setState(pump_mode_options[(int)SensorsCurrentValues->pump_mode]);

  return(true);
}

/*
bool MQTTMessageCallback()
{
  char MessageBuf[16];
  //Publish MQTT messages
  Serial.println("Publishing MQTT messages...");
  //mqtt.connect(DEVICE_BOARD_NAME, mqtt_user, mqtt_pass);
  if (mqtt.connected()) {

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
*/

void MQTTLoop()
{
  HAMQTT.loop();
}