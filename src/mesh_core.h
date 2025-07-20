#ifndef MESH_CORE_H
#define MESH_CORE_H

#include <uv.h>
#include <mosquitto.h>
#include <stdint.h>
#include "mqtt_common.h"
#include "wifi_monitor.h"
#include "system_monitor.h"
#include "uthash.h"  // 添加uthash支持

// MAC地址结构体
typedef struct {
    uint8_t bytes[6];  // MAC地址的6个字节
} mac_addr_t;

// Mesh节点详细信息结构体
typedef struct {
    uint32_t hash_key;     // MAC地址字节相加的哈希值
    mac_addr_t mac;        // MAC地址原始数据
    char mesh_id[18];      // MAC地址字符串格式 (xx:xx:xx:xx:xx:xx)
    int band;              // 频段 (2.4G/5G)
    int rssi;              // 信号强度
    mesh_mode_t role;      // 节点角色
    char status[16];       // 节点状态 (online/offline)
    UT_hash_handle hh;     // uthash句柄
} mesh_node_info_t;

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
    struct mosquitto *mosq;          // MQTT客户端实例
    mesh_node_info_t *nodes;         // 节点哈希表（使用uthash）
    
    // 回调函数
    void (*wifi_data_callback)(const char *data, int len);
    void (*system_data_callback)(const char *data, int len);
    
    // 状态标志
    int initialized;
    int mqtt_connected;
    
} mesh_core_t;

// 节点管理函数
int mesh_node_add(mesh_core_t *core, const char *mesh_id, int band, int rssi);
mesh_node_info_t *mesh_node_find(mesh_core_t *core, const char *mesh_id);
void mesh_node_delete(mesh_core_t *core, const char *mesh_id);
void mesh_node_clear_all(mesh_core_t *core);

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