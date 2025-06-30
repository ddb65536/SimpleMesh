#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <uv.h>

typedef struct {
    uv_loop_t *loop;
    uv_pipe_t socket;
    char socket_path[256];
    int connected;
} system_monitor_t;

// 系统配置结构
typedef struct {
    char hostname[64];
    char model[32];
    char firmware_version[32];
    char lan_ip[16];
    char wan_ip[16];
    char mac_address[18];
} system_config_t;

// 系统统计结构
typedef struct {
    float cpu_usage;
    int memory_total;
    int memory_used;
    int disk_total;
    int disk_used;
    int uptime;
    int load_average[3];
} system_stats_t;

// 网络信息结构
typedef struct {
    char interface[16];
    char ip_address[16];
    char netmask[16];
    char gateway[16];
    int tx_bytes;
    int rx_bytes;
} network_info_t;

// 初始化系统监控
int system_monitor_init(system_monitor_t *monitor, uv_loop_t *loop);

// 清理系统监控
void system_monitor_cleanup(system_monitor_t *monitor);

// 设置系统数据回调
void system_monitor_set_callback(void (*callback)(const char *data, int len));

// 获取系统配置
int system_monitor_get_config(system_config_t *config);

// 获取系统统计
int system_monitor_get_stats(system_stats_t *stats);

// 获取网络信息
int system_monitor_get_network_info(network_info_t *info, int max_count);

#endif // SYSTEM_MONITOR_H 