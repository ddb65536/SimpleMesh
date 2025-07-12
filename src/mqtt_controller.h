#ifndef MQTT_CONTROLLER_H
#define MQTT_CONTROLLER_H
#include "mqtt_common.h"
#include <uv.h>

int mqtt_controller_init(uv_loop_t *loop);

#endif // MQTT_CONTROLLER_H 