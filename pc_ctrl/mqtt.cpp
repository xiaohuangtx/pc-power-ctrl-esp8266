#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "PubSubClient.h"
#include "includes.h"
#include "apu_config.h"
#include "mqtt.h"
#include "pc.h"
#include "ota.h"

const char *user_agent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/136.0.0.0 Safari/537.36";
const char *mqtt_server = "bemfa.com";
const int mqtt_server_port = 9501;
char topic[64];

static uint8_t macAddr[6];
StaticJsonDocument<1024> json_data;

HTTPClient http;
WiFiClient espClient;
PubSubClient client(espClient);

void mqtt_upload_status()
{
    char topic_temp[64];
    sprintf(topic_temp, "%s/up", topic);
    if (client.connected())
    {
        client.publish(topic_temp, pc_get_sta() ? "on" : "off");
    }
}

uint8_t mqtt_get_connected()
{
    return client.connected();
}

void callback(char *topic, byte *payload, unsigned int length)
{
    LOG_PRINT("Message arrived [");
    LOG_PRINT(topic);
    LOG_PRINT("] ");
    String Mqtt_Buff = "";
    for (int i = 0; i < length; i++)
    {
        Mqtt_Buff += (char)payload[i];
    }
    LOG_PRINT(Mqtt_Buff);
    LOG_PRINTLN();
    if (Mqtt_Buff == "on")
    {
        pc_power_ctrl(1);
    }
    else if (Mqtt_Buff == "off")
    {
        pc_power_ctrl(0);
    }
    else if (Mqtt_Buff == "restart")
    {
        pc_restart_ctrl();
    }
    else if (Mqtt_Buff == "update_bin")
    {
        ota_update_handle();
    }
}

void mqtt_setup()
{
    char topic_temp[64];
    WiFi.macAddress(macAddr);
    sprintf(topic_temp, "pc%02x%02x%02x%02x%02x%02x006", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
    LOG_PRINTLN(topic_temp);
    strcpy(topic, topic_temp);
    check_topic();
    client.setServer(mqtt_server, mqtt_server_port);
    client.setCallback(callback);
}

void mqtt_loop()
{
    static unsigned long last = 0;
    if (!client.connected())
    {
        unsigned long now = millis();
        if (now - last > 5000)
        {
            LOG_PRINTLN("MQTT connection...");
            last = now;
            if (client.connect(param.auth_key))
            {
                LOG_PRINTLN("connected");
                client.subscribe(topic);
                mqtt_upload_status();
            }
        }
    }
    else
    {
        client.loop();
    }
}

uint8_t create_topic()
{
    uint8_t res = 0;
    char post_data[256];
    sprintf(post_data, "{\"uid\":\"%s\",\"type\":1,\"topic\":\"%s\",\"name\":\"电脑\"}", param.auth_key, topic);
    http.begin(espClient, "http://pro.bemfa.com/v1/createTopic");
    http.setUserAgent(user_agent);
    http.addHeader("Content-Type", "Content-Type: application/json");
    int code = http.POST(post_data);
    if (code == HTTP_CODE_OK)
    {
        String html = http.getString();
        LOG_PRINTLN(html);
        DeserializationError error = deserializeJson(json_data, html);
        if (error)
        {
            LOG_PRINT(F("deserializeJson() failed: "));
            LOG_PRINTLN(error.c_str());
            res = 0;
            goto EXIT;
        }
        int code = json_data["code"];
        if (code == 0 || code == 40006)
        {
            res = 1;
            goto EXIT;
        }
    }
EXIT:
    http.end();
    return res;
}

uint8_t check_topic()
{
    uint8_t res = 0;
    char url[256];
    sprintf(url, "http://apis.bemfa.com/vb/api/v1/topicInfo?openID=%s&type=1&topic=%s", param.auth_key, topic);
    http.begin(espClient, url);
    http.setUserAgent(user_agent);
    int code = http.GET();
    if (code == HTTP_CODE_OK)
    {
        String html = http.getString();
        LOG_PRINTLN(html);
        DeserializationError error = deserializeJson(json_data, html);
        if (error)
        {
            res = 0;
            goto EXIT;
        }
        int code = json_data["code"];
        if (code == 0)
        {
            res = 1;
            goto EXIT;
        }
        else if (code == -1)
        {
            res = create_topic();
        }
    }
EXIT:
    http.end();
    return res;
}

void ota_update_handle()
{
    char url[256];
    sprintf(url, "http://api.bemfa.com/api/device/v1/bin/?uid=%s&type=1&topic=%s", param.auth_key, topic);
    http.begin(espClient, url);
    http.setUserAgent(user_agent);
    int code = http.GET();
    LOG_PRINTLN(code);
    if (code == HTTP_CODE_OK)
    {
        String html = http.getString();
        LOG_PRINTLN(html);
        DeserializationError error = deserializeJson(json_data, html);
        if (error)
        {
            goto EXIT;
        }
        int code = json_data["code"];
        if (code == 5723007)
        {
            String ota_url = json_data["data"]["url"];
            updateBin(ota_url);
        }
    }
EXIT:
    http.end();
}
