#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include "includes.h"
#include "apu_config.h"

WiFiUDP Udp;
int isConfiging = 0;
param_t param;
static uint8_t macAddr[6];

void APU_Config_start();
void APUConfigInit();
int APU_Config_loop();
void saveParam();
void loadParam();
void copy_string(char *p1, char *p2);

void APU_Config_start()
{
    LOG_PRINTLN("启动配网");
    WiFi.disconnect();
    APUConfigInit();
    int connetRes = 0;
    unsigned long _times = millis();
    isConfiging = 1;
    while (1)
    {
        connetRes = APU_Config_loop();
        if (connetRes == 1)
        {
            LOG_PRINTLN("连接成功,退出循环");
            WiFi.softAPdisconnect(true);
            WiFi.mode(WIFI_STA);
            digitalWrite(APU_CONFIG_LED_PIN, HIGH);
            isConfiging = 0;
            delay(2000);
            ESP.restart();
            break;
        }
        else if (connetRes == 2)
        {
            break;
        }
        if (millis() - _times > 180000)
        {
            LOG_PRINTLN("配网超时重启");
            ESP.restart();
        }
    }
    if (connetRes == 2)
    {
        APU_Config_start();
    }
}

void APUConfigCennetWifi()
{
    WiFi.macAddress(macAddr);
    loadParam();
    LOG_PRINTF("ssid: %s pwd: %s  key: %s  s: %d \n", param.ssid, param.password, param.auth_key, param.check_sum);
    WiFi.mode(WIFI_STA);
    if (strlen(param.ssid))
    {
        WiFi.begin(param.ssid, param.password);
        WiFi.setAutoConnect(true);
        WiFi.setAutoReconnect(true);
        unsigned long previousMillis = millis();
        while (WiFi.waitForConnectResult() != WL_CONNECTED)
        {
            APUConfigKeyScan();
            if (millis() - previousMillis > 50000 && WiFi.status() != WL_CONNECTED)
            {
                APU_Config_start();
            }
        }
    }
    else
    {
        APU_Config_start();
    }
}

void APUConfigInit()
{
    digitalWrite(APU_CONFIG_LED_PIN, HIGH);
    pinMode(APU_CONFIG_LED_PIN, OUTPUT);
    pinMode(APU_CONFIG_KEY_PIN, INPUT);
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);
    WiFi.disconnect();

    // 设置wifi的ip地址(wifi的ip地址好像不能和网关地址在同一个网段)
    IPAddress local_IP(192, 168, 4, 1);
    // 设置网关
    IPAddress gateway(192, 168, 4, 1);
    // 设置子网掩码
    IPAddress subnet(255, 255, 255, 0);
    WiFi.mode(WIFI_AP_STA);
    // 设置IP地址网关和子网掩码
    WiFi.softAPConfig(local_IP, gateway, subnet);
    // 设置wifi的名字和密码
    WiFi.softAP(APU_CONFIG_SSID, APU_CONFIG_PWD, 3, 0, 4);

    Udp.begin(APU_CONFIG_UDPPORT);
}

void APUConfigKeyScan()
{
    if (isConfiging)
    {
        return;
    }
    if (digitalRead(APU_CONFIG_KEY_PIN) == 0)
    {
        delay(200);
        if (digitalRead(APU_CONFIG_KEY_PIN) == 0)
        {
            delay(3000);
            if (digitalRead(APU_CONFIG_KEY_PIN) == 0)
            {
                APU_Config_start();
            }
        }
    }
}

int APU_Config_loop()
{
    static unsigned char led_sta = 0;
    int packetSize = 0;
    packetSize = Udp.parsePacket();
    unsigned long currentMillis = millis();
    static unsigned long previousMillis = 0;
    if (packetSize)
    {
        String udpStringVal = Udp.readString();
        if (udpStringVal.length() < 2)
        {
            return 0;
        }
        LOG_PRINTF("收到来自IP：%s（端口：%d）的数据包字节数：%d\n", Udp.remoteIP().toString().c_str(), Udp.remotePort(), packetSize);
        LOG_PRINTLN(udpStringVal);
        StaticJsonDocument<255> UDPdata;
        DeserializationError error = deserializeJson(UDPdata, udpStringVal);
        if (error)
        {
            LOG_PRINT(F("deserializeJson() failed: "));
            LOG_PRINTLN(error.c_str());
            return 0;
        }
        const char *ssid = UDPdata["ssid"];
        const char *pwd = UDPdata["pwd"];
        const char *authKey = UDPdata["key"];
        LOG_PRINTF("Connecting to %s ", ssid);
        WiFi.begin(ssid, pwd);
        delay(1000);
        WiFi.setAutoConnect(true);
        WiFi.setAutoReconnect(true);
        int s = WiFi.waitForConnectResult(10000);
        if (s != WL_CONNECTED)
        {
            char res[255];
            sprintf(res, "{\"mac\":null,\"ip\":\"\"}");
            LOG_PRINTLN(res);
            Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
            Udp.write(res);
            Udp.endPacket();
            delay(1000);
            return 2;
        }
        else
        {
            char res[255];
            sprintf(res, "{\"mac\":\"%s\",\"ip\":\"%s\"}", WiFi.macAddress().c_str(), WiFi.localIP().toString().c_str());
            LOG_PRINTLN(res);
            Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
            Udp.write(res);
            Udp.endPacket();
            delay(1000);
            copy_string((char *)&param.ssid, (char *)ssid);
            copy_string((char *)&param.password, (char *)pwd);
            copy_string((char *)&param.auth_key, (char *)authKey);
            saveParam();
            LOG_PRINTLN("连接成功");
            return 1;
        }
    }
    if (currentMillis - previousMillis >= 500)
    {
        previousMillis = currentMillis;
        led_sta = !led_sta;
        digitalWrite(APU_CONFIG_LED_PIN, led_sta ? HIGH : LOW);
    }
    packetSize = 0;
    return 0;
}

int Get_Param_CheckSum()
{
    int tmp = 0;
    param_t t;
    memcpy(&t, &param, sizeof(param));
    t.check_sum = 0;
    uint8_t *p = (uint8_t *)(&t);
    for (int i = 0; i < sizeof(param); i++)
    {
        tmp += *(p + i);
    }
    return tmp;
}

void saveParam()
{
    EEPROM.begin(1024);
    param.check_sum = Get_Param_CheckSum();
    uint8_t *p = (uint8_t *)(&param);
    for (int i = 0; i < sizeof(param); i++)
    {
        EEPROM.write(i, *(p + i) ^ macAddr[i % 6]);
    }
    EEPROM.commit();
}

void loadParam()
{
    EEPROM.begin(1024);
    uint8_t *p = (uint8_t *)(&param);
    for (int i = 0; i < sizeof(param); i++)
    {
        *(p + i) = EEPROM.read(i) ^ macAddr[i % 6];
    }
    if (Get_Param_CheckSum() != param.check_sum)
    {
        memset(&param, 0x00, sizeof(param));
    }
}

void copy_string(char *p1, char *p2)
{
    while (*p2 != '\0')
    {
        *p1 = *p2;
        *p1++;
        *p2++;
    }
    *p1 = '\0';
}
