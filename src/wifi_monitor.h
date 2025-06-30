#ifndef WIFI_MONITOR_H
#define WIFI_MONITOR_H

#include <uv.h>

typedef struct {
    uv_loop_t *loop;
    uv_pipe_t socket;
    char socket_path[256];
    int connected;
} wifi_monitor_t;

// WiFi 状态结构
typedef struct {
    char ssid[32];
    char bssid[18];
    int channel;
    int signal_strength;
    int connected_clients;
    int is_up;
} wifi_status_t;

// 连接的客户端信息
typedef struct {
    char mac[18];
    char ip[16];
    int signal_strength;
    time_t connected_time;
} station_info_t;

// 初始化 WiFi 监控
int wifi_monitor_init(wifi_monitor_t *monitor, uv_loop_t *loop);

// 清理 WiFi 监控
void wifi_monitor_cleanup(wifi_monitor_t *monitor);

// 设置 WiFi 数据回调
void wifi_monitor_set_callback(void (*callback)(const char *data, int len));

// 获取 WiFi 状态
int wifi_monitor_get_status(wifi_status_t *status);

// 获取连接的客户端
int wifi_monitor_get_stations(station_info_t *stations, int max_count);

#endif // WIFI_MONITOR_H 