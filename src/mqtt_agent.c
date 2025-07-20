#include "mqtt_agent.h"
#include "mesh_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <uv.h>
#include <time.h>
/*
 * ===== Agent上线逻辑详细说明 =====
 * 
 * Agent上线流程：
 * 1. 监听CONTROLLER_NOTIFY消息：当控制器广播通知时，Agent会收到此消息
 * 2. 解析控制器ID：从JSON消息中提取controller_id字段
 * 3. 检查重复登录：在哈希表中查找是否已存在该控制器
 * 4. 记录控制器信息：将新发现的控制器信息存储到哈希表中
 * 5. 发送登录消息：向控制器发送login消息，建立连接
 * 
 * 关键数据结构：
 * - nodes：哈希表存储所有节点信息
 * - mesh_node_info_t结构体包含：mesh_id、role、status等信息
 * 
 * 状态管理：
 * - 新登录的节点状态设置为"online"
 * - 后续可通过心跳检查更新状态为"offline"
 * 
 * 防重复机制：
 * - 通过哈希表快速查找是否已存在相同mesh_id的节点
 * - 避免向同一控制器重复发送login消息
 * 
 * 扩展性设计：
 * - 动态内存分配，不限制节点数量
 * - mesh_id预留18字节，可存储br-lan的MAC地址
 * - 为后续心跳检查功能预留接口
 */
static uv_timer_t discover_timer;
static int discover_stopped = 0;

static void on_discover_timer(uv_timer_t* handle) {
    mesh_core_t *core = mesh_core_get_instance();
    if (!core || !core->mosq) return;
    const char *payload = "{\"type\":\"discover\"}";
    mosquitto_publish(core->mosq, NULL, MESH_TOPIC_DISCOVER, strlen(payload), payload, 1, false);
    printf("[Agent] Sent discover message\n");
}

static void agent_message_cb(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg) {
    mesh_core_t *core = (mesh_core_t *)userdata;
    
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
            
            // 检查是否已经登录到该控制器
            mesh_node_info_t *existing_node = mesh_node_find(core, controller_id);
            
            // 只向未登录的controller login
            if (!existing_node) {
                // 添加新的控制器节点
                if (mesh_node_add(core, controller_id, 0, 0) == 0) {  // band和rssi暂时设为0
                    // 更新节点角色
                    mesh_node_info_t *node = mesh_node_find(core, controller_id);
                    if (node) {
                        node->role = MODE_CONTROLLER;
                    }
                    
                    // 发送登录消息
                    char login_payload[256];
                    snprintf(login_payload, sizeof(login_payload),
                        "{\"node_id\":\"mesh_agent\",\"timestamp\":%ld,\"target_controller\":\"%s\"}",
                        time(NULL), controller_id);
                    mosquitto_publish(mosq, NULL, MESH_TOPIC_LOGIN,
                        strlen(login_payload), login_payload, 1, false);
                    printf("[Agent] Sent login to controller: %s\n", controller_id);
                }
            } else {
                printf("[Agent] Already logged in to controller: %s, skip login.\n", controller_id);
            }
            // 停止定时器
            if (!discover_stopped) {
                uv_timer_stop(&discover_timer);
                discover_stopped = 1;
                printf("[Agent] Stopped discover timer after receiving CONTROLLER_NOTIFY\n");
            }
        }
    }
}

int mqtt_agent_init(uv_loop_t *loop) {
    mesh_core_t *core = mesh_core_get_instance();
    
    if (!core || !core->mosq) {
        printf("[Agent] Core not initialized or MQTT not available\n");
        return -1;
    }
    
    // 设置消息回调
    mosquitto_message_callback_set(core->mosq, agent_message_cb);
    
    // 启动定时器，每10秒发一次discover
    uv_timer_init(loop, &discover_timer);
    uv_timer_start(&discover_timer, on_discover_timer, 0, 10000);

    printf("[Agent] MQTT agent initialized.\n");
    return 0;
} 