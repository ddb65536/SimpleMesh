#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <mosquitto.h>
#include <uv.h>
#include <time.h>

typedef struct {
    struct mosquitto *mosq;
    uv_loop_t *loop;
    uv_poll_t mqtt_poll;
    uv_timer_t heartbeat_timer;
    int connected;
    char broker_host[128];
    int broker_port;
    char client_id[64];
} mqtt_client_t;

// 初始化 MQTT 客户端
int mqtt_client_init(mqtt_client_t *client, uv_loop_t *loop);

// 清理 MQTT 客户端
void mqtt_client_cleanup(mqtt_client_t *client);

// 发送消息
int mqtt_publish(mqtt_client_t *client, const char *topic, const char *payload, int qos);

// 订阅主题
int mqtt_subscribe(mqtt_client_t *client, const char *topic, int qos);

// 设置消息回调
void mqtt_set_message_callback(void (*callback)(const char *topic, const char *payload, int payload_len));

#endif // MQTT_CLIENT_H 