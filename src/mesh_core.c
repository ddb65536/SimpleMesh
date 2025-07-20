#include "mesh_core.h"
#include "mqtt_controller.h"
#include "mqtt_agent.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <uv.h>
#include <mosquitto.h>

// 全局单例实例
static mesh_core_t g_mesh_core = {0};

// MQTT事件处理相关
static uv_poll_t mqtt_poll_handle;

static void mqtt_uv_poll_cb(uv_poll_t* handle, int status, int events) {
    if (status < 0) {
        printf("[MeshCore] Poll error: %s\n", uv_strerror(status));
        return;
    }

    struct mosquitto *mosq = (struct mosquitto*)handle->data;
    if (events & UV_READABLE) {
        mosquitto_loop_read(mosq, 1);
    }
    if (events & UV_WRITABLE) {
        mosquitto_loop_write(mosq, 1);
    }
    mosquitto_loop_misc(mosq);
}

static void setup_mqtt_uv_poll(uv_loop_t* loop, struct mosquitto* mosq) {
    int sock = mosquitto_socket(mosq);
    if (sock < 0) {
        printf("[MeshCore] Failed to get mosquitto socket\n");
        return;
    }

    uv_poll_init(loop, &mqtt_poll_handle, sock);
    mqtt_poll_handle.data = mosq;

    // 监听可读可写事件
    uv_poll_start(&mqtt_poll_handle, UV_READABLE | UV_WRITABLE, mqtt_uv_poll_cb);
    printf("[MeshCore] MQTT socket added to libuv event loop\n");
}

// 将MAC地址字符串解析为字节数组
int parse_mac_addr(const char *mac_str, mac_addr_t *mac) {
    if (!mac_str || !mac) {
        return -1;
    }

    unsigned int bytes[6];
    int matched = sscanf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x",
                        &bytes[0], &bytes[1], &bytes[2],
                        &bytes[3], &bytes[4], &bytes[5]);
    
    if (matched != 6) {
        return -1;
    }

    for (int i = 0; i < 6; i++) {
        mac->bytes[i] = (uint8_t)bytes[i];
    }

    return 0;
}

// 将MAC地址转换为哈希值（6个字节相加）
uint32_t mac_addr_to_hash(const char *mac_str) {
    mac_addr_t mac;
    if (parse_mac_addr(mac_str, &mac) != 0) {
        return 0;
    }

    uint32_t hash = 0;
    for (int i = 0; i < 6; i++) {
        hash += mac.bytes[i];
    }
    return hash;
}

// 将MAC地址转换为字符串
void mac_addr_to_str(const mac_addr_t *mac, char *str, size_t size) {
    if (!mac || !str || size < 18) {
        if (str && size > 0) {
            str[0] = '\0';
        }
        return;
    }

    snprintf(str, size, "%02x:%02x:%02x:%02x:%02x:%02x",
             mac->bytes[0], mac->bytes[1], mac->bytes[2],
             mac->bytes[3], mac->bytes[4], mac->bytes[5]);
}

// 添加节点到哈希表
int mesh_node_add(mesh_core_t *core, const char *mac_str, int band, int rssi) {
    if (!core || !mac_str) {
        return -1;
    }

    // 计算哈希值
    uint32_t hash_key = mac_addr_to_hash(mac_str);
    if (hash_key == 0) {
        return -1;
    }

    mesh_node_info_t *node;
    HASH_FIND_INT(core->nodes, &hash_key, node);
    
    if (node == NULL) {
        // 新建节点
        node = (mesh_node_info_t *)malloc(sizeof(mesh_node_info_t));
        if (!node) {
            return -1;
        }
        
        // 初始化节点
        node->hash_key = hash_key;
        if (parse_mac_addr(mac_str, &node->mac) != 0) {
            free(node);
            return -1;
        }
        strncpy(node->mesh_id, mac_str, sizeof(node->mesh_id) - 1);
        node->mesh_id[sizeof(node->mesh_id) - 1] = '\0';
        
        // 添加到哈希表
        HASH_ADD_INT(core->nodes, hash_key, node);
    }
    
    // 更新节点信息
    node->band = band;
    node->rssi = rssi;
    strcpy(node->status, "online");
    
    return 0;
}

// 查找节点
mesh_node_info_t *mesh_node_find(mesh_core_t *core, const char *mac_str) {
    if (!core || !mac_str) {
        return NULL;
    }

    uint32_t hash_key = mac_addr_to_hash(mac_str);
    if (hash_key == 0) {
        return NULL;
    }

    mesh_node_info_t *node;
    HASH_FIND_INT(core->nodes, &hash_key, node);
    return node;
}

// 删除节点
void mesh_node_delete(mesh_core_t *core, const char *mac_str) {
    if (!core || !mac_str) {
        return;
    }

    uint32_t hash_key = mac_addr_to_hash(mac_str);
    if (hash_key == 0) {
        return;
    }

    mesh_node_info_t *node;
    HASH_FIND_INT(core->nodes, &hash_key, node);
    if (node) {
        HASH_DEL(core->nodes, node);
        free(node);
    }
}

// 清除所有节点
void mesh_node_clear_all(mesh_core_t *core) {
    if (!core) {
        return;
    }

    mesh_node_info_t *current, *tmp;
    HASH_ITER(hh, core->nodes, current, tmp) {
        HASH_DEL(core->nodes, current);
        free(current);
    }
}

// 初始化mesh核心
int mesh_core_init(mesh_core_t *core, mesh_mode_t mode) {
    if (!core) {
        return -1;
    }
    
    // 清零结构体
    memset(core, 0, sizeof(mesh_core_t));
    
    // 初始化节点哈希表
    core->nodes = NULL;
    
    // 设置模式
    core->mode = mode;
    
    // 初始化libuv事件循环
    core->loop = uv_default_loop();
    if (!core->loop) {
        printf("[MeshCore] Failed to get default loop\n");
        return -1;
    }
    
    // 初始化监控模块
   /* if (wifi_monitor_init(&core->wifi_monitor, core->loop) != 0) {
        printf("[MeshCore] Failed to init WiFi monitor\n");
        return -1;
    }
    
    if (system_monitor_init(&core->system_monitor, core->loop) != 0) {
        printf("[MeshCore] Failed to init system monitor\n");
        wifi_monitor_cleanup(&core->wifi_monitor);
        return -1;
    }*/
    
    // 设置默认回调
    // mesh_core_set_wifi_callback(core, mesh_core_wifi_data_cb);
    // mesh_core_set_system_callback(core, mesh_core_system_data_cb);
    
    // 初始化MQTT
    mosquitto_lib_init();
    const char *client_id = (mode == MODE_CONTROLLER) ? "mesh_controller" : "mesh_agent";
    core->mosq = mosquitto_new(client_id, true, core);
    if (!core->mosq) {
        printf("[MeshCore] Failed to create mosquitto instance\n");
        system_monitor_cleanup(&core->system_monitor);
        wifi_monitor_cleanup(&core->wifi_monitor);
        return -1;
    }
    
    core->initialized = 1;
    printf("[MeshCore] Initialized in %s mode\n", 
           (mode == MODE_CONTROLLER) ? "controller" : "agent");
    
    return 0;
}

// 清理mesh核心
void mesh_core_cleanup(mesh_core_t *core) {
    if (!core || !core->initialized) {
        return;
    }
    
    // 停止MQTT事件监听
    if (core->mqtt_connected) {
        uv_poll_stop(&mqtt_poll_handle);
    }

    // 清理节点哈希表
    mesh_node_clear_all(core);
    
    // 清理MQTT
    if (core->mosq) {
        mosquitto_disconnect(core->mosq);
        mosquitto_destroy(core->mosq);
        core->mosq = NULL;
    }
    mosquitto_lib_cleanup();
    
    // 清理监控模块
    system_monitor_cleanup(&core->system_monitor);
    wifi_monitor_cleanup(&core->wifi_monitor);
    
    // 清零结构体
    memset(core, 0, sizeof(mesh_core_t));
    
    printf("[MeshCore] Cleaned up\n");
}

// 设置WiFi数据回调
void mesh_core_set_wifi_callback(mesh_core_t *core, void (*callback)(const char *data, int len)) {
    if (core) {
        core->wifi_data_callback = callback;
        wifi_monitor_set_callback(callback);
    }
}

// 设置系统数据回调
void mesh_core_set_system_callback(mesh_core_t *core, void (*callback)(const char *data, int len)) {
    if (core) {
        core->system_data_callback = callback;
        system_monitor_set_callback(callback);
    }
}

// 获取核心实例（单例模式）
mesh_core_t* mesh_core_get_instance(void) {
    return &g_mesh_core;
}

// 启动mesh核心
int mesh_core_start(mesh_core_t *core) {
    if (!core || !core->initialized) {
        printf("[MeshCore] Core not initialized\n");
        return -1;
    }
    
    // 连接MQTT
    if (mosquitto_connect(core->mosq, "127.0.0.1", 1883, 60) != MOSQ_ERR_SUCCESS) {
        printf("[MeshCore] Failed to connect to MQTT broker\n");
        return -1;
    }
    core->mqtt_connected = 1;
    
    // 根据模式订阅不同主题
    if (core->mode == MODE_CONTROLLER) {
        mosquitto_subscribe(core->mosq, NULL, MESH_TOPIC_LOGIN, 1);
        mosquitto_subscribe(core->mosq, NULL, MESH_TOPIC_DISCOVER, 1);
        printf("[MeshCore] Controller subscribed to topics\n");
    } else if (core->mode == MODE_AGENT) {
        mosquitto_subscribe(core->mosq, NULL, MESH_TOPIC_CONTROLLER_NOTIFY, 1);
        printf("[MeshCore] Agent subscribed to topics\n");
    }
    
    // 将MQTT socket加入libuv事件循环
    setup_mqtt_uv_poll(core->loop, core->mosq);
    
    // 根据模式初始化对应的MQTT模块
    if (core->mode == MODE_CONTROLLER) {
        if (mqtt_controller_init(core->loop) != 0) {
            printf("[MeshCore] Failed to init MQTT controller\n");
            return -1;
        }
    } else if (core->mode == MODE_AGENT) {
        if (mqtt_agent_init(core->loop) != 0) {
            printf("[MeshCore] Failed to init MQTT agent\n");
            return -1;
        }
    }
    
    printf("[MeshCore] Started successfully\n");
    return 0;
}

// 停止mesh核心
void mesh_core_stop(mesh_core_t *core) {
    if (!core || !core->initialized) {
        return;
    }
    
    if (core->mosq && core->mqtt_connected) {
        mosquitto_loop_stop(core->mosq, true);
        mosquitto_disconnect(core->mosq);
        core->mqtt_connected = 0;
    }
    
    printf("[MeshCore] Stopped\n");
}

// 内部WiFi数据回调
static void mesh_core_wifi_data_cb(const char *data, int len) {
    printf("[MeshCore] WiFi monitor data: %.*s\n", len, data);
}

// 内部系统数据回调
static void mesh_core_system_data_cb(const char *data, int len) {
    printf("[MeshCore] System monitor data: %.*s\n", len, data);
} 