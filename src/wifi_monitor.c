#include "wifi_monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// 全局 WiFi 数据回调函数
static void (*g_wifi_callback)(const char *data, int len) = NULL;

// 内存分配回调
static void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}

// Unix socket 连接回调
static void wifi_connect_cb(uv_connect_t* req, int status) {
    wifi_monitor_t *monitor = (wifi_monitor_t*)req->data;
    if (status == 0) {
        monitor->connected = 1;
        printf("WiFi monitor connected to %s\n", monitor->socket_path);
        // 开始读取数据
        uv_read_start((uv_stream_t*)&monitor->socket, alloc_buffer, wifi_read_cb);
    } else {
        printf("WiFi monitor connection failed: %s\n", uv_strerror(status));
    }
    free(req);
}

// 读取数据回调
static void wifi_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    wifi_monitor_t *monitor = (wifi_monitor_t*)stream->data;
    
    if (nread > 0) {
        // 调用全局回调函数
        if (g_wifi_callback) {
            g_wifi_callback(buf->base, nread);
        }
        
        // 处理接收到的 WiFi 信息
        printf("WiFi data received: %.*s\n", (int)nread, buf->base);
    } else if (nread < 0) {
        if (nread != UV_EOF) {
            printf("WiFi read error: %s\n", uv_strerror(nread));
        }
        monitor->connected = 0;
    }
    
    free(buf->base);
}

// 初始化 WiFi 监控
int wifi_monitor_init(wifi_monitor_t *monitor, uv_loop_t *loop) {
    monitor->loop = loop;
    monitor->connected = 0;
    
    // 设置默认 socket 路径
    strcpy(monitor->socket_path, "/tmp/wifi_monitor.sock");
    
    // 初始化 Unix socket
    if (uv_pipe_init(loop, &monitor->socket, 0) != 0) {
        printf("Failed to init WiFi monitor pipe\n");
        return -1;
    }
    monitor->socket.data = monitor;
    
    // 连接到 WiFi monitor 的 Unix socket
    uv_connect_t *connect_req = malloc(sizeof(uv_connect_t));
    connect_req->data = monitor;
    
    if (uv_pipe_connect(connect_req, &monitor->socket, monitor->socket_path, wifi_connect_cb) != 0) {
        printf("Failed to connect to WiFi monitor socket\n");
        free(connect_req);
        return -1;
    }
    
    return 0;
}

// 清理 WiFi 监控
void wifi_monitor_cleanup(wifi_monitor_t *monitor) {
    if (monitor->connected) {
        uv_read_stop((uv_stream_t*)&monitor->socket);
        uv_close((uv_handle_t*)&monitor->socket, NULL);
    }
}

// 设置 WiFi 数据回调
void wifi_monitor_set_callback(void (*callback)(const char *data, int len)) {
    g_wifi_callback = callback;
}

// 获取 WiFi 状态（简化实现）
int wifi_monitor_get_status(wifi_status_t *status) {
    if (!status) return -1;
    
    // 这里应该从实际的数据源获取状态
    // 目前返回默认值
    strcpy(status->ssid, "default_ssid");
    strcpy(status->bssid, "00:00:00:00:00:00");
    status->channel = 1;
    status->signal_strength = -50;
    status->connected_clients = 0;
    status->is_up = 1;
    
    return 0;
}

// 获取连接的客户端（简化实现）
int wifi_monitor_get_stations(station_info_t *stations, int max_count) {
    if (!stations || max_count <= 0) return -1;
    
    // 这里应该从实际的数据源获取客户端信息
    // 目前返回空列表
    return 0;
} 