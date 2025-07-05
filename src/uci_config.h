#ifndef UCI_CONFIG_H
#define UCI_CONFIG_H

#include <stdint.h>

// UCI配置项结构
typedef struct {
    char name[64];
    char description[256];
    char cmd_get[512];
    char cmd_set[512];
} uci_config_item_t;

// UCI配置管理器结构
typedef struct {
    uci_config_item_t *items;
    int item_count;
    int max_items;
} uci_config_manager_t;

// 初始化UCI配置管理器
int uci_config_init(uci_config_manager_t *manager);

// 清理UCI配置管理器
void uci_config_cleanup(uci_config_manager_t *manager);

// 加载UCI配置项
int uci_config_load_items(uci_config_manager_t *manager);

// 获取配置项数量
int uci_config_get_item_count(uci_config_manager_t *manager);

// 获取配置项
uci_config_item_t* uci_config_get_item(uci_config_manager_t *manager, int index);

// 根据名称查找配置项
uci_config_item_t* uci_config_find_item(uci_config_manager_t *manager, const char *name);

// 执行获取命令
int uci_config_execute_get(uci_config_manager_t *manager, const char *name, char *result, size_t result_size);

// 执行设置命令
int uci_config_execute_set(uci_config_manager_t *manager, const char *name, const char *value);

// 获取所有配置项列表（JSON格式）
int uci_config_get_all_items_json(uci_config_manager_t *manager, char *json_buffer, size_t buffer_size);

// 获取单个配置项信息（JSON格式）
int uci_config_get_item_json(uci_config_manager_t *manager, const char *name, char *json_buffer, size_t buffer_size);

#endif // UCI_CONFIG_H 