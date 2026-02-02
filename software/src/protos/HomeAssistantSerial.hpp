#pragma once

#include <stdint.h>

#include "HomeAssistantSerial_bp.h"

enum class HaInTopicKey : uint8_t {
  FanModeCommandTopic = FAN_MODE_COMMAND_TOPIC,
  ModeCommandTopic = MODE_COMMAND_TOPIC,
  PowerCommandTopic = POWER_COMMAND_TOPIC,
  PresetModeCommandTopic = PRESET_MODE_COMMAND_TOPIC,
  TempCommandTopic = TEMP_COMMAND_TOPIC,
  MqttPing = MQTT_PING_TOPIC
};

enum class HaOutTopicKey : uint8_t {
  ActionTopic = ACTION_TOPIC,
  CurrentHumidityTopic = CURRENT_HUMIDITY_TOPIC,
  CurrentTempTopic = CURRENT_TEMP_TOPIC,
  FanModeStateTopic = FAN_MODE_STATE_TOPIC,
  HumidityStateTopic = HUMIDITY_STATE_TOPIC,
  ModeStateTopic = MODE_STATE_TOPIC,
  PresetModeStateTopic = PRESET_MODE_STATE_TOPIC,
  TempStateTopic = TEMP_STATE_TOPIC,
};

enum class HaActionKey : uint8_t {
  Off = HA_ACTION_OFF,
  Heating = HA_ACTION_HEATING,
  Cooling = HA_ACTION_COOLING,
  Idle = HA_ACTION_IDLE,
  Fan = HA_ACTION_FAN,
};
