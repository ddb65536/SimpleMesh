#include "mqtt_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <uv.h>

// 全局消息回调函数
static void (*g_message_callback)(const char *topic, const char *payload, int payload_len) = NULL;

// MQTT socket 事件回调
static void mqtt_poll_cb(uv_poll_t* handle, int status, int events) {
    mqtt_client_t *client = (mqtt_client_t*)handle->data;
    
    if (events & UV_READABLE) {
        mosquitto_loop_read(client->mosq, 1);
    }
    if (events & UV_WRITABLE) {
        mosquitto_loop_write(client->mosq, 1);
    }
    mosquitto_loop_misc(client->mosq);
}

// MQTT 连接回调
static void mqtt_connect_cb(struct mosquitto *mosq, void *userdata, int rc) {
    mqtt_client_t *client = (mqtt_client_t*)userdata;
    if (rc == 0) {
        client->connected = 1;
        printf("MQTT connected to %s:%d\n", client->broker_host, client->broker_port);
        // 订阅相关主题
        mosquitto_subscribe(mosq, NULL, "mesh/+/status", 1);
        mosquitto_subscribe(mosq, NULL, "mesh/+/command", 1);
    } else {
        printf("MQTT connection failed, rc=%d\n", rc);
    }
}

// MQTT 消息回调
static void mqtt_message_cb(struct mosquitto *mosq, void *userdata, 
                           const struct mosquitto_message *msg) {
    mqtt_client_t *client = (mqtt_client_t*)userdata;
    
    // 调用全局消息回调
    if (g_message_callback) {
        g_message_callback(msg->topic, (char*)msg->payload, msg->payloadlen);
    }
    
    // 处理接收到的消息
    if (strstr(msg->topic, "mesh/") == msg->topic) {
        printf("Received mesh message: %.*s\n", msg->payloadlen, (char*)msg->payload);
    }
}

// 心跳定时器回调
static void heartbeat_timer_cb(uv_timer_t* handle) {
    mqtt_client_t *client = (mqtt_client_t*)handle->data;
    if (client->connected) {
        // 发送心跳消息
        char payload[128];
        snprintf(payload, sizeof(payload), "{\"node_id\":\"%s\",\"timestamp\":%ld}", 
                client->client_id, time(NULL));
        mosquitto_publish(client->mosq, NULL, "mesh/heartbeat", 
                         strlen(payload), payload, 1, false);
    }
}

// 初始化 MQTT 客户端
int mqtt_client_init(mqtt_client_t *client, uv_loop_t *loop) {
    client->loop = loop;
    client->connected = 0;
    
    // 设置默认配置
    strcpy(client->broker_host, "127.0.0.1");
    client->broker_port = 1883;
    strcpy(client->client_id, "mesh_controller");
    
    // 初始化 libmosquitto
    mosquitto_lib_init();
    client->mosq = mosquitto_new(client->client_id, true, client);
    if (!client->mosq) {
        printf("Failed to create mosquitto instance\n");
        return -1;
    }
    
    // 设置回调
    mosquitto_connect_callback_set(client->mosq, mqtt_connect_cb);
    mosquitto_message_callback_set(client->mosq, mqtt_message_cb);
    
    // 连接到 MQTT broker
    if (mosquitto_connect(client->mosq, client->broker_host, client->broker_port, 60) != MOSQ_ERR_SUCCESS) {
        printf("Failed to connect to MQTT broker\n");
        return -1;
    }
    
    // 注册到 libuv
    int mosq_fd = mosquitto_socket(client->mosq);
    if (uv_poll_init(loop, &client->mqtt_poll, mosq_fd) != 0) {
        printf("Failed to init MQTT poll\n");
        return -1;
    }
    client->mqtt_poll.data = client;
    if (uv_poll_start(&client->mqtt_poll, UV_READABLE | UV_WRITABLE, mqtt_poll_cb) != 0) {
        printf("Failed to start MQTT poll\n");
        return -1;
    }
    
    // 启动心跳定时器
    if (uv_timer_init(loop, &client->heartbeat_timer) != 0) {
        printf("Failed to init heartbeat timer\n");
        return -1;
    }
    client->heartbeat_timer.data = client;
    if (uv_timer_start(&client->heartbeat_timer, heartbeat_timer_cb, 30000, 30000) != 0) {
        printf("Failed to start heartbeat timer\n");
        return -1;
    }
    
    return 0;
}

// 清理 MQTT 客户端
void mqtt_client_cleanup(mqtt_client_t *client) {
    if (client->mosq) {
        uv_poll_stop(&client->mqtt_poll);
        uv_timer_stop(&client->heartbeat_timer);
        mosquitto_disconnect(client->mosq);
        mosquitto_destroy(client->mosq);
    }
    mosquitto_lib_cleanup();
}

// 发送消息
int mqtt_publish(mqtt_client_t *client, const char *topic, const char *payload, int qos) {
    if (!client->connected) {
        return -1;
    }
    return mosquitto_publish(client->mosq, NULL, topic, strlen(payload), payload, qos, false);
}

// 订阅主题
int mqtt_subscribe(mqtt_client_t *client, const char *topic, int qos) {
    if (!client->connected) {
        return -1;
    }
    return mosquitto_subscribe(client->mosq, NULL, topic, qos);
}

// 设置消息回调
void mqtt_set_message_callback(void (*callback)(const char *topic, const char *payload, int payload_len)) {
    g_message_callback = callback;
} 