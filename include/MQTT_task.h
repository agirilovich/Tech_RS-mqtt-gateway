#include <HaMqttEntities.h>
#include "Credentials.h"

#ifndef DEVICE_BOARD_NAME
#  define DEVICE_BOARD_NAME "TechRSMQTTgateway"
#endif

#ifndef MQTT_GENERAL_PREFIX
#  define MQTT_GENERAL_PREFIX "home"
#endif

#undef HA_TOPIC_HEAD
#define HA_TOPIC_HEAD MQTT_GENERAL_PREFIX

struct ValveData
{
    float mix_valve_state;
    float mix_valve_openLevel;
    float mix_valve_type;
    float mix_valve_temp_set;
    float mix_valve_temp;
    float mix_valve_pump;
};

struct SensorsData
{
  float device_time;
  float device_state;
  float ext_temp;
  float co_temp;
  float co_temp_ret;
  float cwu_temp;
  float cwu_temp_set;
  float pump_state_co;
  float pump_state_cwu;

  ValveData valveData[2];

  float pump_mode;
};

void initMQTT();

bool MQTTpublish(struct SensorsData* SensorsCurrentValues);

void MQTTMessageCallback(HAEntity *entity, char *topic, byte *payload, unsigned int length);

void MQTTLoop();


