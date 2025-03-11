#ifndef __APU_CONFIG_H__
#define __APU_CONFIG_H__

#define APU_CONFIG_LED_PIN 2 // 8266 LED连接引脚
#define APU_CONFIG_KEY_PIN 0 // 8266 配网按键连接引脚

#define APU_CONFIG_UDPPORT 88 // 配网UDP端口

#define APU_CONFIG_SSID "wifi_config_bind" // AP名称
#define APU_CONFIG_PWD "11223344"          // AP密码

struct param_t
{
    char ssid[128];
    char password[128];
    char auth_key[256];
    int check_sum;
};
extern param_t param;

void APUConfigCennetWifi();
void APUConfigKeyScan();
#endif
