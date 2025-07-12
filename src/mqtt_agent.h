#ifndef MQTT_AGENT_H
#define MQTT_AGENT_H
#include "mqtt_common.h"
#include <uv.h>

int mqtt_agent_init(uv_loop_t *loop);

#endif // MQTT_AGENT_H 