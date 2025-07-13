#!/bin/sh

. /lib/functions.sh
. /usr/share/libubox/jshn.sh

json_init
json_add_object "sync_data"

get_data_cmd_val() {
    local section="$1"
    local cmd_get
    config_get cmd_get "$section" cmd_get
    if [ -n "$cmd_get" ]; then
        val=$(eval $cmd_get 2>/dev/null)
        json_add_string "$section" "$val"
    fi
}

config_load simple_mesh
config_foreach get_data_cmd_val data_cmd

json_close_object
json_dump
