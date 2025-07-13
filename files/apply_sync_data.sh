#!/bin/sh

. /usr/share/libubox/jshn.sh

# 载入JSON（假设作为第一个参数传入）
json_load "$1"

# 先进入 sync_data 这个对象
json_select sync_data

# 获取 sync_data 里的所有 key
json_get_keys keys

for key in $keys; do
    json_get_var value "$key"
    echo "$key: $value"
    cmd_set=$(uci -q get simple_mesh.$key.cmd_set)
    export sync_data="$value"
    eval "$cmd_set"
done

# 回到上一级（可选）
json_select ..