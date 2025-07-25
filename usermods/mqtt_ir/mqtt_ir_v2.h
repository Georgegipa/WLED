#pragma once

#include "wled.h"
#include "hamqtt.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "ir_Tcl.h"

IRTcl112Ac ac(IR_LED);
class MqttIR : public Usermod
{
private:
  enum fanModes
  {
    FAN_AUTO = kTcl112AcFanAuto,
    FAN_MIN = kTcl112AcFanMin,
    FAN_LOW = kTcl112AcFanLow,
    FAN_MEDIUM = kTcl112AcFanMed,
    FAN_HIGH = kTcl112AcFanHigh
  };

  enum Modes
  {
    MODE_OFF = 0,
    MODE_AUTO = kTcl112AcAuto,
    MODE_COOL = kTcl112AcCool,
    MODE_DRY = kTcl112AcDry,
    MODE_FAN = kTcl112AcFan,
    MODE_HEAT = kTcl112AcHeat
  };

  enum Presets
  {
    PRESET_ECO = 0,
    PRESET_QUIET = 1,
    PRESET_TURBO = 2,
    PRESET_HEALTH = 3
  };

  int minutes = 0;
  double temperature = 24.0;
  bool displayOn = true;
  uint8_t fan = FAN_AUTO;
  uint8_t mode = MODE_OFF;

  void generateIRpayload();
  void publishAcState();
  void updateClimate();
  void updateFan();
  void updateDisplay();
  void updateTimer();
  // TODO: implement presets
  // const char *presets[4] = {"eco", "quiet", "turbo", "health"};
  const char *fanSpeeds[5] = {"silent", "auto", "low", "medium", "high"};
  bool firstInit = true;

public:
  void setup() override
  {
    haMqtt.begin();
    ac.begin();
  }

  /*
   * loop() is called continuously. Here you can check for events, read sensors, etc.
   *
   * Tips:
   * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
   *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
   *
   * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
   *    Instead, use a timer check as shown here.
   */
  void loop() override {}

#ifndef WLED_DISABLE_MQTT
  /**
   * handling of MQTT message
   * topic only contains stripped topic (part after /wled/MAC)
   */
  bool onMqttMessage(char *topic, char *payload) override
  {
    bool payloadChanged = false;
    // set timer
    // /ha/timer_cmd
    if (strcmp(&topic[4], "timer_cmd") == 0)
    {
      int time = atoi(payload);
      minutes = time * 60;
      payloadChanged = true;
      updateTimer();
    }

    // set fan speed
    // /ha/select_cmd
    if (strcmp(&topic[4], "fan_speed_cmd") == 0) // compare after the /ha/
    {
      if (strcmp(payload, fanSpeeds[0]) == 0)
        fan = FAN_MIN;
      else if (strcmp(payload, fanSpeeds[1]) == 0)
        fan = FAN_AUTO;
      else if (strcmp(payload, fanSpeeds[2]) == 0)
        fan = FAN_LOW;
      else if (strcmp(payload, fanSpeeds[3]) == 0)
        fan = FAN_MEDIUM;
      else if (strcmp(payload, fanSpeeds[4]) == 0)
        fan = FAN_HIGH;
      else
        return false;
      payloadChanged = true;
      updateFan();
    }

    // set climate mode
    // /ha/climate_mode_cmd
    if (strcmp(&topic[4], "tcl_climate_mode_cmd") == 0)
    {
      if (strcmp(payload, "auto") == 0)
        mode = MODE_AUTO;
      else if (strcmp(payload, "cool") == 0)
        mode = MODE_COOL;
      else if (strcmp(payload, "dry") == 0)
        mode = MODE_DRY;
      else if (strcmp(payload, "fan") == 0)
        mode = MODE_FAN;
      else if (strcmp(payload, "heat") == 0)
        mode = MODE_HEAT;
      else if (strcmp(payload, "off") == 0)
        mode = MODE_OFF;
      else
        return false;
      payloadChanged = true;
      updateClimate();
    }

    // receive temperature changes
    // /ha/climate_temp_cmd

    if (strcmp(&topic[4], "tcl_climate_temp_cmd") == 0)
    {
      temperature = atof(payload);
      payloadChanged = true;
      updateClimate();
    }

    // receive display on/off command
    // /ha/display_cmd

    if (strcmp(&topic[4], "display_cmd") == 0)
    {

      if (strcmp(payload, "OFF") == 0)
        displayOn = false;
      else
        displayOn = true;
      payloadChanged = true;
      updateDisplay();
    }

    if (payloadChanged)
    {
      generateIRpayload();
      return true; // message handled
    }
    return false;
  }

  void onMqttConnect(bool sessionPresent) override
  {
    char subuf[64];
    if (mqttDeviceTopic[0] != 0)
    {
      strcpy(subuf, mqttDeviceTopic);
      strcat_P(subuf, PSTR("/ha/#"));
      mqtt->subscribe(subuf, 0);
      haMqtt.MQTTnumber("timer", "AC Timer", "timer");
      haMqtt.MQTTclimate("ac", "TCL AC", "tcl_climate");
      haMqtt.MQTTselect("fan", "Fan Speed", "fan_speed", fanSpeeds, 5);
      haMqtt.MQTTswitch("display", "AC Display", "display");
    }
    if (firstInit)
    {
      publishAcState();
      firstInit = false;
    }
  }
#endif
};

void MqttIR::generateIRpayload()
{
  Serial.printf("Generating IR payload with temperature: %.1f, mode: %d, fan: %d, displayOn: %s\n", temperature, mode, fan, displayOn ? "true" : "false");
  if (mode == MODE_OFF)
    ac.setPower(false);
  else
    ac.setPower(true);
  ac.setOffTimer(minutes);
  ac.setFan(fan);
  ac.setMode(mode);
  ac.setTemp(temperature);
  ac.setLight(displayOn);
  ac.send();
}

void MqttIR::publishAcState()
{
  updateClimate();
  updateFan();
  updateDisplay();
  updateTimer();
}

void MqttIR::updateFan()
{
  switch (fan)
  {
  case FAN_MIN:
    haMqtt.publishHaState("fan_speed", fanSpeeds[0]); // "silent"
    break;
  case FAN_LOW:
    haMqtt.publishHaState("fan_speed", fanSpeeds[2]); // "low"
    break;
  case FAN_MEDIUM:
    haMqtt.publishHaState("fan_speed", fanSpeeds[3]); // "medium"
    break;
  case FAN_HIGH:
    haMqtt.publishHaState("fan_speed", fanSpeeds[4]); // "high"
    break;
  default:
    haMqtt.publishHaState("fan_speed", fanSpeeds[1]); // "auto"
    break;
  }
}

void MqttIR::updateDisplay()
{
  haMqtt.publishHaState("display", displayOn ? "ON" : "OFF");
}

void MqttIR::updateTimer()
{

  haMqtt.publishHaState("timer", String(minutes / 60.0).c_str());
}

void MqttIR::updateClimate()
{
  // publish the initial state of the ac
  // set the temp of the climate
  String climateTopicPostFix = "tcl_climate";
  String temperature_state_topic = climateTopicPostFix + "_temp_state";
  String mode_state_topic = climateTopicPostFix + "_mode_state";

  switch (mode)
  {
  case MODE_AUTO:
    haMqtt.publishHaState(mode_state_topic.c_str(), "auto");
    break;
  case MODE_COOL:
    haMqtt.publishHaState(mode_state_topic.c_str(), "cool");
    break;
  case MODE_DRY:
    haMqtt.publishHaState(mode_state_topic.c_str(), "dry");
    break;
  case MODE_FAN:
    haMqtt.publishHaState(mode_state_topic.c_str(), "fan");
    break;
  case MODE_HEAT:
    haMqtt.publishHaState(mode_state_topic.c_str(), "heat");
    break;
  default:
    haMqtt.publishHaState(mode_state_topic.c_str(), "off");
    break;
  }

  // publish the current temperature
  haMqtt.publishHaState(temperature_state_topic.c_str(), String(temperature).c_str());
}