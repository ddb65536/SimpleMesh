#include "mqtt_controller.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <uv.h>

static struct mosquitto *g_controller_mosq = NULL;

static void controller_message_cb(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg) {
    if (strstr(msg->topic, "SIMPLE_MESH") == msg->topic) {
        printf("[Controller] Received message on topic '%s': %.*s\n", msg->topic, msg->payloadlen, (char*)msg->payload);
        // 只处理DISCOVER
        if (strcmp(msg->topic, MESH_TOPIC_DISCOVER) == 0) {
            // 解析node_id
            char node_id[64] = {0};
            char *node_id_start = strstr((char*)msg->payload, "\"node_id\":\"");
            if (node_id_start) {
                node_id_start += 11;
                char *node_id_end = strchr(node_id_start, '"');
                if (node_id_end) {
                    int len = node_id_end - node_id_start;
                    if (len < sizeof(node_id)) {
                        strncpy(node_id, node_id_start, len);
                        node_id[len] = '\0';
                    }
                }
            }
            // 回复notify，带controller_id
            char reply_payload[256];
            snprintf(reply_payload, sizeof(reply_payload),
                "{\"controller_id\":\"mesh_controller\",\"network_status\":\"unknown\"}");
            mosquitto_publish(mosq, NULL, MESH_TOPIC_CONTROLLER_NOTIFY,
                strlen(reply_payload), reply_payload, 1, false);
            printf("[Controller] Replied notify to node: %s\n", node_id);
        }
    }
}

int mqtt_controller_init(uv_loop_t *loop) {
    mosquitto_lib_init();
    g_controller_mosq = mosquitto_new("mesh_controller", true, NULL);
    if (!g_controller_mosq) {
        printf("[Controller] Failed to create mosquitto instance\n");
        return -1;
    }
    mosquitto_message_callback_set(g_controller_mosq, controller_message_cb);
    if (mosquitto_connect(g_controller_mosq, "127.0.0.1", 1883, 60) != MOSQ_ERR_SUCCESS) {
        printf("[Controller] Failed to connect to MQTT broker\n");
        return -1;
    }
    mosquitto_subscribe(g_controller_mosq, NULL, MESH_TOPIC_LOGIN, 1);
    mosquitto_subscribe(g_controller_mosq, NULL, MESH_TOPIC_DISCOVER, 1);
    // 启动mosquitto循环（简化，实际应集成到libuv）
    mosquitto_loop_start(g_controller_mosq);
    printf("[Controller] MQTT initialized and subscribed.\n");
    return 0;
} 