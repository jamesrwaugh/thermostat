#pragma once

#include <stdint.h>

#include "HomeAssistantSerial_bp.h"

enum class HaInTopicKey : uint8_t {
  FanModeCommandTopic = FAN_MODE_COMMAND_TOPIC,
  ModeCommandTopic = MODE_COMMAND_TOPIC,
  PowerCommandTopic = POWER_COMMAND_TOPIC,
  PresetMoreCommandTopic = PRESET_MODE_COMMAND_TOPIC,
  TempCommandTopic = TEMP_COMMAND_TOPIC,
  TempHighCommandTopic = TEMP_HIGH_COMMAND_TOPIC,
  TempLowCommandTopic = TEMP_LOW_COMMAND_TOPIC,
};

enum class HaOutTopicKey : uint8_t {
  ActionTopic = ACTION_TOPIC,
  CurrentHumidityTopic = CURRENT_HUMIDITY_TOPIC,
  CurrentTempTopic = CURRENT_TEMP_TOPIC,
  FanModeStateTopic = FAN_MODE_STATE_TOPIC,
  HumidityStateTopic = HUMIDITY_STATE_TOPIC,
  ModeStateTopic = MODE_STATE_TOPIC,
  PresetModeStateTopic = PRESET_MODE_STATE_TOPIC,
  TempHighStateTopic = TEMP_HIGH_STATE_TOPIC,
  TempLowStateTopic = TEMP_LOW_STATE_TOPIC,
  TempStateTopic = TEMP_STATE_TOPIC,
};
