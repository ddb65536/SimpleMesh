#include "mqtt_client.h"
#include "mqtt_topics.h"
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
        if (client->mode == MODE_CONTROLLER) {
            // Controller监听登录和发现主题
            mosquitto_subscribe(mosq, NULL, MESH_TOPIC_LOGIN, 1);
            mosquitto_subscribe(mosq, NULL, MESH_TOPIC_DISCOVER, 1);
        } else if (client->mode == MODE_AGENT) {
            // Agent监听控制器通知主题
            mosquitto_subscribe(mosq, NULL, MESH_TOPIC_CONTROLLER_NOTIFY, 1);
        }
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
    if (strstr(msg->topic, "SIMPLE_MESH") == msg->topic) {
        printf("Received SimpleMesh message on topic '%s': %.*s\n", 
               msg->topic, msg->payloadlen, (char*)msg->payload);
        
        // Controller收到discover消息后回复
        if (client->mode == MODE_CONTROLLER && 
            strcmp(msg->topic, MESH_TOPIC_DISCOVER) == 0) {
            
            // 解析discover消息中的node_id
            char node_id[64] = {0};
            // 简单解析JSON获取node_id（这里简化处理）
            char *node_id_start = strstr((char*)msg->payload, "\"node_id\":\"");
            if (node_id_start) {
                node_id_start += 11; // 跳过 "node_id":"
                char *node_id_end = strchr(node_id_start, '"');
                if (node_id_end) {
                    int len = node_id_end - node_id_start;
                    if (len < sizeof(node_id)) {
                        strncpy(node_id, node_id_start, len);
                        node_id[len] = '\0';
                    }
                }
            }
            
            // 发送controller通知回复
            char reply_payload[256];
            snprintf(reply_payload, sizeof(reply_payload), 
                    "{\"controller_id\":\"%s\",\"timestamp\":%ld,\"action\":\"notify\",\"target_node\":\"%s\"}", 
                    client->client_id, time(NULL), node_id);
            
            mosquitto_publish(client->mosq, NULL, MESH_TOPIC_CONTROLLER_NOTIFY, 
                             strlen(reply_payload), reply_payload, 1, false);
            printf("Controller replied to discover from node: %s\n", node_id);
        }
    }
}

// 心跳定时器回调
static void heartbeat_timer_cb(uv_timer_t* handle) {
    mqtt_client_t *client = (mqtt_client_t*)handle->data;
    if (client->connected) {
        // 发送心跳消息
        char payload[128];
        snprintf(payload, sizeof(payload), "{\"node_id\":\"%s\",\"timestamp\":%ld,\"mode\":\"%s\"}", 
                client->client_id, time(NULL), 
                (client->mode == MODE_CONTROLLER) ? "controller" : "agent");
        
        // 根据模式发送到不同的topic
        if (client->mode == MODE_CONTROLLER) {
            mosquitto_publish(client->mosq, NULL, MESH_TOPIC_CONTROLLER_NOTIFY, 
                             strlen(payload), payload, 1, false);
        } else if (client->mode == MODE_AGENT) {
            mosquitto_publish(client->mosq, NULL, MESH_TOPIC_DISCOVER, 
                             strlen(payload), payload, 1, false);
        }
    }
}

// Discover定时器回调
static void discover_timer_cb(uv_timer_t* handle) {
    mqtt_client_t *client = (mqtt_client_t*)handle->data;
    if (client->connected && client->mode == MODE_AGENT) {
        // Agent周期性发送discover消息
        char payload[128];
        snprintf(payload, sizeof(payload), "{\"node_id\":\"%s\",\"timestamp\":%ld,\"action\":\"discover\"}", 
                client->client_id, time(NULL));
        
        mosquitto_publish(client->mosq, NULL, MESH_TOPIC_DISCOVER, 
                         strlen(payload), payload, 1, false);
        printf("Agent sent discover message\n");
    }
}

// 初始化 MQTT 客户端
int mqtt_client_init(mqtt_client_t *client, uv_loop_t *loop, mesh_mode_t mode) {
    client->loop = loop;
    client->connected = 0;
    client->mode = mode;
    
    // 设置默认配置
    strcpy(client->broker_host, "127.0.0.1");
    client->broker_port = 1883;
    
    // 根据模式设置client_id
    if (mode == MODE_CONTROLLER) {
        strcpy(client->client_id, "mesh_controller");
    } else if (mode == MODE_AGENT) {
        strcpy(client->client_id, "mesh_agent");
    } else {
        strcpy(client->client_id, "mesh_unknown");
    }
    
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
    
    // 启动discover定时器（仅Agent模式）
    if (uv_timer_init(loop, &client->discover_timer) != 0) {
        printf("Failed to init discover timer\n");
        return -1;
    }
    client->discover_timer.data = client;
    if (client->mode == MODE_AGENT) {
        // Agent每10秒发送一次discover
        if (uv_timer_start(&client->discover_timer, discover_timer_cb, 10000, 10000) != 0) {
            printf("Failed to start discover timer\n");
            return -1;
        }
    }
    
    return 0;
}

// 清理 MQTT 客户端
void mqtt_client_cleanup(mqtt_client_t *client) {
    if (client->mosq) {
        uv_poll_stop(&client->mqtt_poll);
        uv_timer_stop(&client->heartbeat_timer);
        uv_timer_stop(&client->discover_timer);
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