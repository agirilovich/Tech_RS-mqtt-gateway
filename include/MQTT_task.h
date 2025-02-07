#ifndef DEVICE_BOARD_NAME
#  define DEVICE_BOARD_NAME "TechRSMQTTgateway"
#endif

#ifndef MQTT_GENERAL_PREFIX
#  define MQTT_GENERAL_PREFIX "home"
#endif

#define MQTT_CONFIG_PREFIX "homeassistant"

//Define MQTT Topic for HomeAssistant Discovery
const char *MQTTConfigPrefix = MQTT_CONFIG_PREFIX;

//Define MQTT Topic for HomeAssistant Sensors state
const char *MQTTGeneralPrefix = MQTT_GENERAL_PREFIX;

#include <HaMqttEntities.h>

// Define Device in Home Assistant scope of integrations
HADevice ha_device(DEVICE_BOARD_NAME, DEVICE_BOARD_NAME, "1.0");

// Define Home Assistant sensors
HASensorNumeric device_time = HASensorNumeric("device_time", "Device Time", ha_device);
    device_time.addFeature(HA_FEATURE_DEVICE_CLASS, "TIMESTAMP");

HASensorNumeric device_day = HASensorNumeric("device_day", "Device Week Day", ha_device);
    device_time.addFeature(HA_FEATURE_DEVICE_CLASS, "TIMESTAMP");

HASensorNumeric ext_temp = HASensorNumeric("ext_temp", "Temperature outside", ha_device, "С");
    ext_temp.addFeature(HA_FEATURE_DEVICE_CLASS, "TEMPERATURE");
    ext_temp.addFeature(HA_FEATURE_ICON,"mdi:home-thermometer-outline");

HASensorNumeric co_temp = HASensorNumeric("co_temp", "Temperature CO", ha_device, "С");
    co_temp.addFeature(HA_FEATURE_DEVICE_CLASS, "TEMPERATURE");
    co_temp.addFeature(HA_FEATURE_ICON,"mdi:water-thermometer");

HASensorNumeric co_temp_ret = HASensorNumeric("co_temp_ret", "Temperature return CO", ha_device, "С");
    co_temp_ret.addFeature(HA_FEATURE_DEVICE_CLASS, "TEMPERATURE");
    co_temp_ret.addFeature(HA_FEATURE_ICON,"mdi:water-thermometer");


HASensorNumeric cwu_temp = HASensorNumeric("cwu_temp", "Temperature CWU", ha_device, "С");
    cwu_temp.addFeature(HA_FEATURE_DEVICE_CLASS, "TEMPERATURE");
    cwu_temp.addFeature(HA_FEATURE_ICON,"mdi:water-thermometer");

HASensorNumeric cwu_temp_ret = HASensorNumeric("cwu_temp_ret", "Temperature return CWU", ha_device, "С");
    cwu_temp_ret.addFeature(HA_FEATURE_DEVICE_CLASS, "TEMPERATURE");
    cwu_temp_ret.addFeature(HA_FEATURE_ICON,"mdi:water-thermometer");

HASensorNumeric cwu_temp_set = HASensorNumeric("cwu_temp_set", "Temperature set CWU", ha_device, "С");
    cwu_temp_set.addFeature(HA_FEATURE_DEVICE_CLASS, "TEMPERATURE");
    cwu_temp_set.addFeature(HA_FEATURE_ICON,"mdi:water-thermometer");

#define OPTIONS_COUNT 4
const char *pump_mode_options[4] PROGMEM = {
     "Grzanie domu",
     "Priorytet bojlera",
     "Pompy równoległe",
     "Tryb letni"
     };
HASensorNumeric pump_mode = HASelect("pump_mode", "Mode", ha_device, OPTIONS_COUNT, pump_mode_options);

HASensorBinary pump_state_co = HASensorBinary("pump_state_co", "CO pump state", ha_device);
    pump_state_co.addFeature(HA_FEATURE_DEVICE_CLASS, "HEAT");
    pump_state_co.addFeature(HA_FEATURE_ICON,"mdi:pump");

HASensorBinary pump_state_cwu = HASensorBinary("pump_state_cwu", "CWU pump state", ha_device);
    pump_state_cwu.addFeature(HA_FEATURE_DEVICE_CLASS, "HEAT");
    pump_state_cwu.addFeature(HA_FEATURE_ICON,"mdi:pump");

// Valve 1
HASensorBinary mix_valve1_state = HASensorBinary("mix_valve1_state", "Mix Valve 1 state", ha_device);
    mix_valve1_state.addFeature(HA_FEATURE_DEVICE_CLASS, "HEAT");
    mix_valve1_state.addFeature(HA_FEATURE_ICON,"mdi:pipe-valve");

HASensorNumeric mix_valve1_openLevel = HASensorNumeric("mix_valve1_openLevel", "Mix Valve 1 Open Level", ha_device, "%");
    mix_valve1_openLevel.addFeature(HA_FEATURE_DEVICE_CLASS, "POWER_FACTOR");
    mix_valve1_openLevel.addFeature(HA_FEATURE_ICON,"mdi:valve");

HASensorNumeric mix_valve1_type = HASensorNumeric("mix_valve1_type", "Mix Valve 1 Type", ha_device);

HASensorNumeric mix_valve1_temp_set = HASensorNumeric("mix_valve1_temp_set", "Mix Valve 1 Temperature set", ha_device, "С");
    mix_valve1_temp_set.addFeature(HA_FEATURE_DEVICE_CLASS, "TEMPERATURE");
    mix_valve1_temp_set.addFeature(HA_FEATURE_ICON,"mdi:water-thermometer");

HASensorNumeric mix_valve1_temp = HASensorNumeric("mix_valve1_temp_set", "Mix Valve 1 Temperature", ha_device, "С");
    mix_valve1_temp.addFeature(HA_FEATURE_DEVICE_CLASS, "TEMPERATURE");
    mix_valve1_temp.addFeature(HA_FEATURE_ICON,"mdi:water-thermometer");

// Valve 2
HASensorBinary mix_valve2_state = HASensorBinary("mix_valve2_state", "Mix Valve 2 state", ha_device);
    mix_valve2_state.addFeature(HA_FEATURE_DEVICE_CLASS, "HEAT");
    mix_valve2_state.addFeature(HA_FEATURE_ICON,"mdi:pipe-valve");

HASensorNumeric mix_valve2_openLevel = HASensorNumeric("mix_valve2_openLevel", "Mix Valve 2 Open Level", ha_device, "%");
    mix_valve1_openLevel.addFeature(HA_FEATURE_DEVICE_CLASS, "POWER_FACTOR");
    mix_valve1_openLevel.addFeature(HA_FEATURE_ICON,"mdi:valve");

HASensorNumeric mix_valve2_type = HASensorNumeric("mix_valve2_type", "Mix Valve 2 Type", ha_device);

HASensorNumeric mix_valve2_temp_set = HASensorNumeric("mix_valve2_temp_set", "Mix Valve 2 Temperature set", ha_device, "С");
    mix_valve2_temp_set.addFeature(HA_FEATURE_DEVICE_CLASS, "TEMPERATURE");
    mix_valve2_temp_set.addFeature(HA_FEATURE_ICON,"mdi:water-thermometer");

HASensorNumeric mix_valve2_temp = HASensorNumeric("mix_valve2_temp_set", "Mix Valve 2 Temperature", ha_device, "С");
    mix_valve2_temp.addFeature(HA_FEATURE_DEVICE_CLASS, "TEMPERATURE");
    mix_valve2_temp.addFeature(HA_FEATURE_ICON,"mdi:water-thermometer");


void initMQTT();

void publishMQTTPayload(const char *Topic, char *PayloadMessage);

bool MQTTMessageCallback(HAEntity *entity, char *topic, byte *payload, unsigned int length);

void MQTTLoop();


