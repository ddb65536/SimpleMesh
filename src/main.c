#include <uv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mqtt_client.h"
#include "wifi_monitor.h"
#include "system_monitor.h"

/*
main的入参要支持设置为controler或者agent

1. 使用libuv实现，mqtt信息的接收和发送，接收本地 unix socket的wifi_monitor和system_monitor的信息
2. 维护一个mesh节点列表，每个节点的标识是mac地址，包括节点下连接的设备的信息，连接的设备要能区分lan/wifi


*/

// 全局变量
uv_loop_t *loop;
mqtt_client_t mqtt_client;
wifi_monitor_t wifi_monitor;
system_monitor_t system_monitor;

// 主程序入口
int main() {
    // 初始化 libuv 事件循环
    loop = uv_default_loop();
    
    // 初始化各模块
    if (mqtt_client_init(&mqtt_client, loop) != 0) {
        printf("Failed to init MQTT client\n");
        return 1;
    }
    
    if (wifi_monitor_init(&wifi_monitor, loop) != 0) {
        printf("Failed to init WiFi monitor\n");
        return 1;
    }
    
    if (system_monitor_init(&system_monitor, loop) != 0) {
        printf("Failed to init system monitor\n");
        return 1;
    }
    
    printf("Mesh controller started\n");
    
    // 启动事件循环
    uv_run(loop, UV_RUN_DEFAULT);
    
    // 清理资源
    mqtt_client_cleanup(&mqtt_client);
    wifi_monitor_cleanup(&wifi_monitor);
    system_monitor_cleanup(&system_monitor);
    
    return 0;
} 