#ifndef __MQTT_H__
#define __MQTT_H__

#include <mosquitto.h>

extern struct mosquitto *mosq;
void mqtt_init(struct mosquitto **mosq);


#endif