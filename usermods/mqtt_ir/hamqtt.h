#pragma once

#include "wled.h"

class HaMqtt
{
private:
    char haTopicPrefix[40]; // "wled/mac/ha/"

public:
    // TODO: use the following array for getting the climate modes
    // const char *climateModes[6] = {"off", "auto", "cool", "heat", "fan", "dry"};

    void begin();
    void MQTTnumber(const char *name, const char *friendlyName, const char *topic, uint8_t min = 0, uint8_t max = 10, double step = 1);
    void MQTTclimate(const char *name, const char *friendlyName, const char *topic, uint8_t minTemp = 16, uint8_t maxTemp = 30, double step = 0.5);
    void MQTTselect(const char *name, const char *friendlyName, const char *topic, const char *options[], uint8_t optionsCount);
    void MQTTswitch(const char *name, const char *friendlyName, const char *topic);
    void publishHaState(const char *postFixTopic, const char *output);
};

void HaMqtt::begin()
{
    // mqttDeviceTopic + "/ha/"
    strcpy(haTopicPrefix, mqttDeviceTopic);
    strcat(haTopicPrefix, "/ha/");
}

void HaMqtt::MQTTnumber(const char *name, const char *friendlyName, const char *topic, uint8_t min, uint8_t max, double step)
{
    StaticJsonDocument<520> doc;

    doc["name"] = friendlyName;
    doc["unique_id"] = String(mqttClientID) + "/" + name;
    doc["state_topic"] = String(haTopicPrefix) + topic;
    doc["command_topic"] = String(haTopicPrefix) + topic + "_cmd";
    doc["min"] = min;
    doc["max"] = max;
    doc["step"] = step;

    doc["availability_topic"] = String(mqttDeviceTopic) + "/status";
    doc["payload_available"] = "online";
    doc["payload_not_available"] = "offline";

    JsonObject device = doc.createNestedObject("device"); // attach the sensor to the same device
    device["name"] = String(serverDescription);
    device["model"] = F(WLED_PRODUCT_NAME);
    device["manufacturer"] = F(WLED_BRAND);
    device["identifiers"] = String(mqttClientID);
    device["sw_version"] = VERSION;

    String output;
    serializeJson(doc, output);
    String adv_topic = String("homeassistant/number/") + mqttClientID + "/" + name + "/config";
    mqtt->publish(adv_topic.c_str(), 0, true, output.c_str());
}

void HaMqtt::MQTTclimate(const char *name, const char *friendlyName, const char *topic, uint8_t minTemp, uint8_t maxTemp, double step)
{
    StaticJsonDocument<520> doc;

    doc["name"] = friendlyName;
    doc["unique_id"] = String(mqttClientID) + "/" + name;
    doc["mode_state_topic"] = String(haTopicPrefix) + topic + "_mode_state";        // wled/mac/ha/<topic>_mode_state
    doc["mode_command_topic"] = String(haTopicPrefix) + topic + "_mode_cmd";        // wled/mac/ha/<topic>_mode_cmd
    doc["temperature_state_topic"] = String(haTopicPrefix) + topic + "_temp_state"; // wled/mac/ha/<topic>_temp_state
    doc["temperature_command_topic"] = String(haTopicPrefix) + topic + "_temp_cmd"; // wled/mac/ha/<topic>_temp_cmd
    doc["min_temp"] = minTemp;
    doc["max_temp"] = maxTemp;
    doc["temp_step"] = step;

    doc["availability_topic"] = String(mqttDeviceTopic) + "/status";
    doc["payload_available"] = "online";
    doc["payload_not_available"] = "offline";

    JsonObject device = doc.createNestedObject("device"); // attach the sensor to the same device
    device["name"] = String(serverDescription);
    device["model"] = F(WLED_PRODUCT_NAME);
    device["manufacturer"] = F(WLED_BRAND);
    device["identifiers"] = String(mqttClientID);
    device["sw_version"] = VERSION;

    String output;
    serializeJson(doc, output);
    String adv_topic = String("homeassistant/climate/") + mqttClientID + "/" + name + "/config";
    mqtt->publish(adv_topic.c_str(), 0, true, output.c_str());
}

void HaMqtt::MQTTselect(const char *name, const char *friendlyName, const char *topic, const char *options[], uint8_t optionsCount)
{
    StaticJsonDocument<400> doc;

    doc["name"] = friendlyName;
    doc["unique_id"] = String(mqttClientID) + "/" + name;
    doc["state_topic"] = String(haTopicPrefix) + topic;
    doc["command_topic"] = String(haTopicPrefix) + topic + "_cmd";
    doc["options"] = JsonArray(doc.createNestedArray("options"));

    for (uint8_t i = 0; i < optionsCount; i++)
    {
        doc["options"].add(options[i]);
    }

    doc["availability_topic"] = String(mqttDeviceTopic) + "/status";
    doc["payload_available"] = "online";
    doc["payload_not_available"] = "offline";

    JsonObject device = doc.createNestedObject("device"); // attach the sensor to the same device
    device["name"] = String(serverDescription);
    device["model"] = F(WLED_PRODUCT_NAME);
    device["manufacturer"] = F(WLED_BRAND);
    device["identifiers"] = String(mqttClientID);
    device["sw_version"] = VERSION;

    String output;
    serializeJson(doc, output);
    String adv_topic = String("homeassistant/select/") + mqttClientID + "/" + name + "/config";
    mqtt->publish(adv_topic.c_str(), 0, true, output.c_str());
}

void HaMqtt::MQTTswitch(const char *name, const char *friendlyName, const char *topic)
{
    StaticJsonDocument<400> doc;

    doc["name"] = friendlyName;
    doc["unique_id"] = String(mqttClientID) + "/" + name;
    doc["state_topic"] = String(haTopicPrefix) + topic;
    doc["command_topic"] = String(haTopicPrefix) + topic + "_cmd";

    doc["availability_topic"] = String(mqttDeviceTopic) + "/status";
    doc["payload_available"] = "online";
    doc["payload_not_available"] = "offline";

    JsonObject device = doc.createNestedObject("device"); // attach the sensor to the same device
    device["name"] = String(serverDescription);
    device["model"] = F(WLED_PRODUCT_NAME);
    device["manufacturer"] = F(WLED_BRAND);
    device["identifiers"] = String(mqttClientID);
    device["sw_version"] = VERSION;

    String output;
    serializeJson(doc, output);
    String adv_topic = String("homeassistant/switch/") + mqttClientID + "/" + name + "/config";
    mqtt->publish(adv_topic.c_str(), 0, true, output.c_str());
}

void HaMqtt::publishHaState(const char *postFixTopic, const char *output)
{
    String adv_topic = String(haTopicPrefix) + postFixTopic;
    mqtt->publish(adv_topic.c_str(), 0, true, output);
}

static HaMqtt haMqtt; // Create a static instance of HaMqtt to use its methods