#include <uv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mesh_core.h"
#include "mqtt_topics.h"

/*
main的入参要支持设置为controler或者agent

1. 使用libuv实现，mqtt信息的接收和发送，接收本地 unix socket的wifi_monitor和system_monitor的信息
2. 维护一个mesh节点列表，每个节点的标识是mac地址，包括节点下连接的设备的信息，连接的设备要能区分lan/wifi


*/

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
int parse_arguments(int argc, char *argv[], mesh_mode_t *mode) {
    int opt;
    
    while ((opt = getopt(argc, argv, "m:hv")) != -1) {
        switch (opt) {
            case 'm':
                if (strcmp(optarg, "controller") == 0) {
                    *mode = MODE_CONTROLLER;
                } else if (strcmp(optarg, "agent") == 0) {
                    *mode = MODE_AGENT;
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
    
    if (*mode == MODE_UNKNOWN) {
        printf("Error: Mode not specified. Use -m controller or -m agent\n");
        print_usage(argv[0]);
        return -1;
    }
    
    return 0;
}

// WiFi数据回调
static void main_wifi_data_cb(const char *data, int len) {
    printf("[Main] WiFi monitor data: %.*s\n", len, data);
}

// 系统数据回调
static void main_system_data_cb(const char *data, int len) {
    printf("[Main] System monitor data: %.*s\n", len, data);
}

// 主程序入口
int main(int argc, char *argv[]) {
    mesh_mode_t mode = MODE_UNKNOWN;
    mesh_core_t *core;
    
    // 解析命令行参数
    if (parse_arguments(argc, argv, &mode) != 0) {
        return 1;
    }
    
    // 获取mesh核心实例
    core = mesh_core_get_instance();
    
    // 初始化mesh核心
    if (mesh_core_init(core, mode) != 0) {
        printf("Failed to initialize mesh core\n");
        return 1;
    }
    
    // 设置回调函数
    mesh_core_set_wifi_callback(core, main_wifi_data_cb);
    mesh_core_set_system_callback(core, main_system_data_cb);
    
    // 启动mesh核心
    if (mesh_core_start(core) != 0) {
        printf("Failed to start mesh core\n");
        mesh_core_cleanup(core);
        return 1;
    }
    
    printf("Mesh %s started\n", (mode == MODE_CONTROLLER) ? "controller" : "agent");
    
    // 启动事件循环
    uv_run(core->loop, UV_RUN_DEFAULT);
    
    // 清理资源
    mesh_core_cleanup(core);
    
    return 0;
} 