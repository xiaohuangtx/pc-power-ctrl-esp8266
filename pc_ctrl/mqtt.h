#ifndef _MATT_H
#define _MATT_H

#include <ESP8266WiFi.h>

void mqtt_upload_status();
uint8_t mqtt_get_connected();
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();
void mqtt_setup();
void mqtt_loop();
uint8_t create_topic();
uint8_t check_topic();
void ota_update_handle();
#endif
