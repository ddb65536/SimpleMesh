#include "system_monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// 全局系统数据回调函数
static void (*g_system_callback)(const char *data, int len) = NULL;

// 内存分配回调
static void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}

// 读取数据回调
static void system_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

// Unix socket 连接回调
static void system_connect_cb(uv_connect_t* req, int status) {
    system_monitor_t *monitor = (system_monitor_t*)req->data;
    if (status == 0) {
        monitor->connected = 1;
        printf("System monitor connected to %s\n", monitor->socket_path);
        // 开始读取数据
        uv_read_start((uv_stream_t*)&monitor->socket, alloc_buffer, system_read_cb);
    } else {
        printf("System monitor connection failed: %s\n", uv_strerror(status));
    }
    free(req);
}

// 读取数据回调
static void system_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    system_monitor_t *monitor = (system_monitor_t*)stream->data;
    
    if (nread > 0) {
        // 调用全局回调函数
        if (g_system_callback) {
            g_system_callback(buf->base, nread);
        }
        
        // 处理接收到的系统信息
        printf("System data received: %.*s\n", (int)nread, buf->base);
    } else if (nread < 0) {
        if (nread != UV_EOF) {
            printf("System read error: %s\n", uv_strerror(nread));
        }
        monitor->connected = 0;
    }
    
    free(buf->base);
}

// 初始化系统监控
int system_monitor_init(system_monitor_t *monitor, uv_loop_t *loop) {
    monitor->loop = loop;
    monitor->connected = 0;
    
    // 设置默认 socket 路径
    strcpy(monitor->socket_path, "/tmp/system_monitor.sock");
    
    // 初始化 Unix socket
    if (uv_pipe_init(loop, &monitor->socket, 0) != 0) {
        printf("Failed to init system monitor pipe\n");
        return -1;
    }
    monitor->socket.data = monitor;
    
    // 连接到系统 monitor 的 Unix socket
    uv_connect_t *connect_req = malloc(sizeof(uv_connect_t));
    connect_req->data = monitor;
    
    uv_pipe_connect(connect_req, &monitor->socket, monitor->socket_path, system_connect_cb);
    
    return 0;
}

// 清理系统监控
void system_monitor_cleanup(system_monitor_t *monitor) {
    if (monitor->connected) {
        uv_read_stop((uv_stream_t*)&monitor->socket);
        uv_close((uv_handle_t*)&monitor->socket, NULL);
    }
}

// 设置系统数据回调
void system_monitor_set_callback(void (*callback)(const char *data, int len)) {
    g_system_callback = callback;
}

// 获取系统配置（简化实现）
int system_monitor_get_config(system_config_t *config) {
    if (!config) return -1;
    
    // 这里应该从实际的数据源获取配置
    // 目前返回默认值
    strcpy(config->hostname, "router");
    strcpy(config->model, "OpenWrt");
    strcpy(config->firmware_version, "21.02.0");
    strcpy(config->lan_ip, "192.168.1.1");
    strcpy(config->wan_ip, "0.0.0.0");
    strcpy(config->mac_address, "00:11:22:33:44:55");
    
    return 0;
}

// 获取系统统计（简化实现）
int system_monitor_get_stats(system_stats_t *stats) {
    if (!stats) return -1;
    
    // 这里应该从实际的数据源获取统计信息
    // 目前返回默认值
    stats->cpu_usage = 25.5;
    stats->memory_total = 128 * 1024; // 128MB
    stats->memory_used = 64 * 1024;   // 64MB
    stats->disk_total = 1024 * 1024;  // 1GB
    stats->disk_used = 512 * 1024;    // 512MB
    stats->uptime = 3600;             // 1小时
    stats->load_average[0] = 1;
    stats->load_average[1] = 1;
    stats->load_average[2] = 1;
    
    return 0;
}

// 获取网络信息（简化实现）
int system_monitor_get_network_info(network_info_t *info, int max_count) {
    if (!info || max_count <= 0) return -1;
    
    // 这里应该从实际的数据源获取网络信息
    // 目前返回默认值
    if (max_count >= 1) {
        strcpy(info[0].interface, "eth0");
        strcpy(info[0].ip_address, "192.168.1.1");
        strcpy(info[0].netmask, "255.255.255.0");
        strcpy(info[0].gateway, "192.168.1.254");
        info[0].tx_bytes = 1024 * 1024;
        info[0].rx_bytes = 2048 * 1024;
        return 1;
    }
    
    return 0;
} 