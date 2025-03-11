#include "apu_config.h"
#include "includes.h"
#include "mqtt.h"
#include <ESP8266WiFi.h>
#include "pc.h"

uint8_t delay_upload_flag = 0;
unsigned long delay_upload_time = 0;

uint8_t pc_get_sta()
{
    static uint8_t last = 0;
    if (digitalRead(POWER_STATUS_PIN) == HIGH)
    {
        delay(10);
        if (digitalRead(POWER_STATUS_PIN) == HIGH)
        {
            last = 1;
            return 1;
        }
    }
    else
    {
        delay(10);
        if (digitalRead(POWER_STATUS_PIN) == LOW)
        {
            last = 0;
            return 0;
        }
    }
    return last;
}

void pc_gpio_init()
{
    digitalWrite(SWITCH_PIN, LOW);
    pinMode(SWITCH_PIN, OUTPUT);
    digitalWrite(SWITCH_PIN, LOW);

    digitalWrite(RESTART_PIN, LOW);
    pinMode(RESTART_PIN, OUTPUT);
    digitalWrite(RESTART_PIN, LOW);

    pinMode(POWER_STATUS_PIN, INPUT);
}

void pc_power_ctrl(uint8_t sta)
{
    if (sta)
    {
        if (pc_get_sta())
            return;
    }
    else
    {
        if (!pc_get_sta())
            return;
    }
    digitalWrite(SWITCH_PIN, HIGH);
    delay(500);
    digitalWrite(SWITCH_PIN, LOW);
    delay_upload_time = millis();
    delay_upload_flag = 1;
}

void pc_restart_ctrl(void)
{
    if (!pc_get_sta())
        return;
    digitalWrite(RESTART_PIN, HIGH);
    delay(500);
    digitalWrite(RESTART_PIN, LOW);
    delay_upload_time = millis();
    delay_upload_flag = 1;
}

void pc_sta_scan()
{
    static int sta = 99;
    static unsigned long t = 0;
    if (millis() - t > 1000)
    {
        t = millis();
        if (mqtt_get_connected() && pc_get_sta() != sta)
        {
            sta = pc_get_sta();
            mqtt_upload_status();
        }
    }
    if (delay_upload_flag && millis() - delay_upload_time > 15000)
    {
        mqtt_upload_status();
        delay_upload_flag = 0;
    }
}
