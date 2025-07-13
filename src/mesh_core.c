#include "mesh_core.h"
#include "mqtt_controller.h"
#include "mqtt_agent.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 全局单例实例
static mesh_core_t g_mesh_core = {0};

// 内部函数声明
static void mesh_core_wifi_data_cb(const char *data, int len);
static void mesh_core_system_data_cb(const char *data, int len);

// 初始化mesh核心
int mesh_core_init(mesh_core_t *core, mesh_mode_t mode) {
    if (!core) {
        return -1;
    }
    
    // 清零结构体
    memset(core, 0, sizeof(mesh_core_t));
    
    // 设置模式
    core->mode = mode;
    
    // 初始化libuv事件循环
    core->loop = uv_default_loop();
    if (!core->loop) {
        printf("[MeshCore] Failed to get default loop\n");
        return -1;
    }
    
    // 初始化监控模块
    if (wifi_monitor_init(&core->wifi_monitor, core->loop) != 0) {
        printf("[MeshCore] Failed to init WiFi monitor\n");
        return -1;
    }
    
    if (system_monitor_init(&core->system_monitor, core->loop) != 0) {
        printf("[MeshCore] Failed to init system monitor\n");
        wifi_monitor_cleanup(&core->wifi_monitor);
        return -1;
    }
    
    // 设置默认回调
    mesh_core_set_wifi_callback(core, mesh_core_wifi_data_cb);
    mesh_core_set_system_callback(core, mesh_core_system_data_cb);
    
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
    
    // 启动mosquitto循环
    mosquitto_loop_start(core->mosq);
    
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