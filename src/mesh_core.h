#ifndef MESH_CORE_H
#define MESH_CORE_H

#include <uv.h>
#include <mosquitto.h>
#include "mqtt_common.h"
#include "wifi_monitor.h"
#include "system_monitor.h"

// Mesh节点结构体
typedef struct {
    char mesh_id[18];  // MAC地址格式 (xx:xx:xx:xx:xx:xx)
    mesh_mode_t role;  // 节点角色
    char status[16];   // 节点状态 (online/offline)
} mesh_node_t;

// Mesh核心结构体，整合所有全局变量
typedef struct {
    // libuv事件循环
    uv_loop_t *loop;
    
    // 运行模式
    mesh_mode_t mode;
    
    // 监控模块
    wifi_monitor_t wifi_monitor;
    system_monitor_t system_monitor;
    
    // MQTT相关
    struct mosquitto *mosq;  // MQTT客户端实例
    struct mesh_node_t mesh_node[64];  // Mesh节点数组（仅agent模式使用）
    
    // 回调函数
    void (*wifi_data_callback)(const char *data, int len);
    void (*system_data_callback)(const char *data, int len);
    
    // 状态标志
    int initialized;
    int mqtt_connected;
    
} mesh_core_t;

// 初始化mesh核心
int mesh_core_init(mesh_core_t *core, mesh_mode_t mode);

// 清理mesh核心
void mesh_core_cleanup(mesh_core_t *core);

// 设置回调函数
void mesh_core_set_wifi_callback(mesh_core_t *core, void (*callback)(const char *data, int len));
void mesh_core_set_system_callback(mesh_core_t *core, void (*callback)(const char *data, int len));

// 获取核心实例（单例模式）
mesh_core_t* mesh_core_get_instance(void);

// 启动mesh核心
int mesh_core_start(mesh_core_t *core);

// 停止mesh核心
void mesh_core_stop(mesh_core_t *core);

#endif // MESH_CORE_H 