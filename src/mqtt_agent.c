#include "mqtt_agent.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <uv.h>

static struct mosquitto *g_agent_mosq = NULL;
static char g_logged_in_controller[64] = "";

static void agent_message_cb(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg) {
    if (strstr(msg->topic, "SIMPLE_MESH") == msg->topic) {
        printf("[Agent] Received message on topic '%s': %.*s\n", msg->topic, msg->payloadlen, (char*)msg->payload);
        // 只处理CONTROLLER_NOTIFY
        if (strcmp(msg->topic, MESH_TOPIC_CONTROLLER_NOTIFY) == 0) {
            // 解析controller_id
            char controller_id[64] = {0};
            char *id_start = strstr((char*)msg->payload, "\"controller_id\":\"");
            if (id_start) {
                id_start += strlen("\"controller_id\":\"");
                char *id_end = strchr(id_start, '"');
                if (id_end) {
                    int len = id_end - id_start;
                    if (len < sizeof(controller_id)) {
                        strncpy(controller_id, id_start, len);
                        controller_id[len] = '\0';
                    }
                }
            } else {
                strcpy(controller_id, "unknown_controller");
            }
            // 只向未登录的controller login
            if (strcmp(g_logged_in_controller, controller_id) != 0) {
                strncpy(g_logged_in_controller, controller_id, sizeof(g_logged_in_controller)-1);
                char login_payload[256];
                snprintf(login_payload, sizeof(login_payload),
                    "{\"node_id\":\"mesh_agent\",\"timestamp\":%ld,\"target_controller\":\"%s\"}",
                    time(NULL), controller_id);
                mosquitto_publish(mosq, NULL, MESH_TOPIC_LOGIN,
                    strlen(login_payload), login_payload, 1, false);
                printf("[Agent] Sent login to controller: %s\n", controller_id);
            } else {
                printf("[Agent] Already logged in to controller: %s, skip login.\n", controller_id);
            }
        }
    }
}

int mqtt_agent_init(uv_loop_t *loop) {
    mosquitto_lib_init();
    g_agent_mosq = mosquitto_new("mesh_agent", true, NULL);
    if (!g_agent_mosq) {
        printf("[Agent] Failed to create mosquitto instance\n");
        return -1;
    }
    mosquitto_message_callback_set(g_agent_mosq, agent_message_cb);
    if (mosquitto_connect(g_agent_mosq, "127.0.0.1", 1883, 60) != MOSQ_ERR_SUCCESS) {
        printf("[Agent] Failed to connect to MQTT broker\n");
        return -1;
    }
    mosquitto_subscribe(g_agent_mosq, NULL, MESH_TOPIC_CONTROLLER_NOTIFY, 1);
    // 启动mosquitto循环（简化，实际应集成到libuv）
    mosquitto_loop_start(g_agent_mosq);
    printf("[Agent] MQTT initialized and subscribed.\n");
    return 0;
} 