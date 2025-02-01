#ifndef DEVICE_BOARD_NAME
#  define DEVICE_BOARD_NAME "TechRSMQTTgateway"
#endif

#ifndef MQTT_GENERAL_PREFIX
#  define MQTT_GENERAL_PREFIX "home"
#endif

#define MQTT_SENSORS_NAME "/sensor/techrsgw"
#define MQTT_BINARYSENSORS_NAME "/binary_sensor/techrsgw"
#define MQTT_CONFIG_PREFIX "homeassistant"
#define SENSOR_NAME "TechRSgateway"

#define LIMIT_MQTT_FAILURE 10

#define MQTT_TOPIC_TSET_CONFIG MQTT_CONFIG_PREFIX MQTT_SENSORS_NAME "/TSet/config"
#define MQTT_TOPIC_FLAMEON_CONFIG MQTT_CONFIG_PREFIX MQTT_BINARYSENSORS_NAME "/FlameOn/config"
#define MQTT_TOPIC_MAXMODLEVEL_CONFIG MQTT_CONFIG_PREFIX MQTT_SENSORS_NAME "/MaxRelModLevelSetting/config"
#define MQTT_TOPIC_TRSET_CONFIG MQTT_CONFIG_PREFIX MQTT_SENSORS_NAME "/TrSet/config"
#define MQTT_TOPIC_ROOMTEMP_CONFIG MQTT_CONFIG_PREFIX MQTT_SENSORS_NAME "/Tr/config"

#define MQTT_TOPIC_STATE MQTT_GENERAL_PREFIX "/" DEVICE_BOARD_NAME

#define MQTT_TSET_TOPIC_STATE MQTT_TOPIC_STATE "/TSet/state"
#define MQTT_FLAMEON_TOPIC_STATE MQTT_TOPIC_STATE "/FlameOn/state"
#define MQTT_MAXMODLEVEL_TOPIC_STATE MQTT_TOPIC_STATE "/MaxRelModLevelSetting/state"
#define MQTT_TRSET_TOPIC_STATE MQTT_TOPIC_STATE "/TrSet/state"
#define MQTT_ROOMTEMP_TOPIC_STATE MQTT_TOPIC_STATE "/Tr/state"

void initMQTT();

void initializeMQTTTopic(const char *Topic, char *SensorConfig);

void publishMQTTPayload(const char *Topic, char *PayloadMessage);

bool MQTTMessageCallback();

void MQTTLoop();


