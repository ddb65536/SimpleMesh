#include <uv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mqtt_client.h"
#include "mqtt_topics.h"
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
mesh_mode_t current_mode = MODE_UNKNOWN;

// 打印使用说明
void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("Options:\n");
    printf("  -m, --mode MODE     Set mesh mode (controller|agent)\n");
    printf("  -h, --help          Show this help message\n");
    printf("  -v, --version       Show version information\n");
    printf("\nExamples:\n");
    printf("  %s -m controller    Run as mesh controller\n", program_name);
    printf("  %s -m agent         Run as mesh agent\n", program_name);
}

// 打印版本信息
void print_version() {
    printf("SimpleMesh v1.0.0\n");
    printf("A simple mesh network implementation for OpenWrt routers\n");
}

// 解析命令行参数
int parse_arguments(int argc, char *argv[]) {
    int opt;
    
    while ((opt = getopt(argc, argv, "m:hv")) != -1) {
        switch (opt) {
            case 'm':
                if (strcmp(optarg, "controller") == 0) {
                    current_mode = MODE_CONTROLLER;
                } else if (strcmp(optarg, "agent") == 0) {
                    current_mode = MODE_AGENT;
                } else {
                    printf("Error: Invalid mode '%s'. Use 'controller' or 'agent'\n", optarg);
                    return -1;
                }
                break;
            case 'h':
                print_usage(argv[0]);
                exit(0);
            case 'v':
                print_version();
                exit(0);
            case '?':
                print_usage(argv[0]);
                return -1;
        }
    }
    
    if (current_mode == MODE_UNKNOWN) {
        printf("Error: Mode not specified. Use -m controller or -m agent\n");
        print_usage(argv[0]);
        return -1;
    }
    
    return 0;
}

// 主程序入口
int main(int argc, char *argv[]) {
    // 解析命令行参数
    if (parse_arguments(argc, argv) != 0) {
        return 1;
    }
    
    // 初始化 libuv 事件循环
    loop = uv_default_loop();
    
    // 初始化各模块，传递模式参数
    if (mqtt_client_init(&mqtt_client, loop, current_mode) != 0) {
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
    
    // 根据模式打印启动信息
    if (current_mode == MODE_CONTROLLER) {
        printf("Mesh controller started\n");
    } else if (current_mode == MODE_AGENT) {
        printf("Mesh agent started\n");
    }
    
    // 启动事件循环
    uv_run(loop, UV_RUN_DEFAULT);
    
    // 清理资源
    mqtt_client_cleanup(&mqtt_client);
    wifi_monitor_cleanup(&wifi_monitor);
    system_monitor_cleanup(&system_monitor);
    
    return 0;
} 