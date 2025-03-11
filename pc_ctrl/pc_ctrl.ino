
#include "includes.h"
#include "apu_config.h"
#include "includes.h"
#include "mqtt.h"
#include "pc.h"

void setup()
{
#ifdef DEBUG_LOG
    Serial.begin(115200);
    LOG_PRINTLN(" ");
    LOG_PRINTLN("固件启动");
#endif
    pc_gpio_init();
    APUConfigCennetWifi();
    mqtt_setup();
}

void loop()
{
    APUConfigKeyScan();
    mqtt_loop();
    pc_sta_scan();
}
