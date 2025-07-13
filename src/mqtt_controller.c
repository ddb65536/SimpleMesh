#include "mqtt_controller.h"
#include "mesh_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <uv.h>
/*
 * ===== Controller处理Agent上线逻辑详细说明 =====
 * 
 * Controller处理Agent上线流程：
 * 1. 监听DISCOVER消息：当Agent发送discover消息时，Controller会收到此消息
 * 2. 解析Agent的node_id：从JSON消息中提取node_id字段
 * 3. 回复CONTROLLER_NOTIFY：向Agent发送控制器信息，包含controller_id
 * 4. 监听LOGIN消息：当Agent发送login消息时，处理Agent的登录请求
 * 5. 记录Agent信息：将登录的Agent信息存储到本地管理结构中
 * 6. 发送确认消息：向Agent发送登录确认消息
 * 
 * 关键数据结构：
 * - 需要类似mesh_node的结构来管理连接的Agent
 * - 每个Agent包含：agent_id、role、status、连接时间等信息
 * 
 * 状态管理：
 * - 新连接的Agent状态设置为"online"
 * - 可通过心跳检查更新Agent状态
 * - 支持Agent断开连接的状态更新
 * 
 * 消息处理：
 * - DISCOVER消息：回复CONTROLLER_NOTIFY，让Agent发现控制器
 * - LOGIN消息：处理Agent登录，记录Agent信息
 * - 心跳消息：更新Agent状态，维护连接
 * 
 * 扩展性设计：
 * - 支持多个Agent同时连接
 * - 为Agent状态监控预留接口
 * - 支持Agent的认证和权限管理
 */
static void controller_message_cb(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg) {
    mesh_core_t *core = (mesh_core_t *)userdata;
    
    if (strstr(msg->topic, "SIMPLE_MESH") == msg->topic) {
        printf("[Controller] Received message on topic '%s': %.*s\n", msg->topic, msg->payloadlen, (char*)msg->payload);
        
        // ===== 处理DISCOVER消息 =====
        // Agent发送discover消息，Controller回复notify让Agent发现控制器
        if (strcmp(msg->topic, MESH_TOPIC_DISCOVER) == 0) {
            // 步骤1: 解析Agent的node_id
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
            
            // 步骤2: 回复CONTROLLER_NOTIFY消息
            // 让Agent发现控制器，包含controller_id和网络状态
            char reply_payload[256];
            snprintf(reply_payload, sizeof(reply_payload),
                "{\"controller_id\":\"mesh_controller\",\"network_status\":\"unknown\"}");
            mosquitto_publish(mosq, NULL, MESH_TOPIC_CONTROLLER_NOTIFY,
                strlen(reply_payload), reply_payload, 1, false);
            printf("[Controller] Replied notify to node: %s\n", node_id);
        }
        
        // ===== 处理LOGIN消息 =====
        // Agent发送login消息，Controller处理Agent登录请求
        else if (strcmp(msg->topic, MESH_TOPIC_LOGIN) == 0) {
            // 步骤1: 解析Agent登录信息
            char agent_id[64] = {0};
            char target_controller[64] = {0};
            char *agent_start = strstr((char*)msg->payload, "\"node_id\":\"");
            char *target_start = strstr((char*)msg->payload, "\"target_controller\":\"");
            
            // 解析agent_id
            if (agent_start) {
                agent_start += 11;
                char *agent_end = strchr(agent_start, '"');
                if (agent_end) {
                    int len = agent_end - agent_start;
                    if (len < sizeof(agent_id)) {
                        strncpy(agent_id, agent_start, len);
                        agent_id[len] = '\0';
                    }
                }
            }
            
            // 解析target_controller
            if (target_start) {
                target_start += 19;
                char *target_end = strchr(target_start, '"');
                if (target_end) {
                    int len = target_end - target_start;
                    if (len < sizeof(target_controller)) {
                        strncpy(target_controller, target_start, len);
                        target_controller[len] = '\0';
                    }
                }
            }
            
            // 步骤2: 验证目标控制器
            // 检查Agent是否要登录到当前控制器
            if (strcmp(target_controller, "mesh_controller") == 0 || strcmp(target_controller, "") == 0) {
                // 步骤3: 记录Agent信息到本地存储
                // 这里可以扩展为类似mesh_node的结构来管理Agent
                printf("[Controller] Agent %s logged in successfully\n", agent_id);
                
                // 步骤4: 发送登录确认消息
                // 向Agent发送登录成功的确认消息
                char confirm_payload[256];
                snprintf(confirm_payload, sizeof(confirm_payload),
                    "{\"status\":\"success\",\"message\":\"Login confirmed\",\"agent_id\":\"%s\"}",
                    agent_id);
                
                // 发布确认消息到Agent状态主题
                mosquitto_publish(mosq, NULL, MESH_TOPIC_AGENT_STATUS,
                    strlen(confirm_payload), confirm_payload, 1, false);
                
                printf("[Controller] Sent login confirmation to agent: %s\n", agent_id);
            } else {
                // 如果目标控制器不是当前控制器，记录日志
                printf("[Controller] Agent %s tried to login to wrong controller: %s\n", 
                       agent_id, target_controller);
            }
        }
    }
}

int mqtt_controller_init(uv_loop_t *loop) {
    // 获取mesh核心实例（单例模式）
    mesh_core_t *core = mesh_core_get_instance();
    
    // 检查核心是否已初始化且MQTT客户端可用
    if (!core || !core->mosq) {
        printf("[Controller] Core not initialized or MQTT not available\n");
        return -1;
    }
    
    // 设置MQTT消息回调函数
    // 当收到MQTT消息时，会调用controller_message_cb函数处理
    mosquitto_message_callback_set(core->mosq, controller_message_cb);
    
    printf("[Controller] MQTT controller initialized.\n");
    return 0;
} 