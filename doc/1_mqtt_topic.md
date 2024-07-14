# MQTT TOPIC

### controller publish
* Topology_query
* UNASSOC STA LINK METRICS QUERY
* UNASSOC STA LINK METRICS REPORT
* BTM REPORT
* STEER REQUEST
* RRM REQUEST
* RRM REPORT
* 802.11R PMKID REPORT
* Topology Member notify
* HEARTBEAT

### controller  subscribe
* Topology report
* UNASSOC STA LINK METRICS QUERY
* UNASSOC STA LINK METRICS REPORT
* RRM REQUEST
* RRM REPORT
* BTM REPORT
* 802.11R PMKID REPORT
* Topology Member notify


### AGENT publish
* Topology_query
* UNASSOC STA LINK METRICS QUERY
* UNASSOC STA LINK METRICS REPORT
* BTM REPORT
* STEER REQUEST
* RRM REQUEST
* RRM REPORT
* 802.11R PMKID REPORT
* Topology Member notify
  

### AGENT subscribe
* Topology report
* UNASSOC STA LINK METRICS QUERY
* UNASSOC STA LINK METRICS REPORT
* RRM REQUEST
* RRM REPORT
* BTM REPORT
* 802.11R PMKID REPORT
* Topology Member notify
* 

#### Topology_query: ap/topology_query
```
{
    "_msg_type":"topology_query",
    "_mesh_version":"1.0",
    "_query_mode":"boardcast"/"query_80:3f:5d:11:22:33"
}
```


#### Topology report: ap/topology_report
```
{
    "_msg_type":"topology_report",
    "_mesh_version":"1.0",
    "_msg_data":{
        "AL_MAC":"80:3f:5d:11:22:33", "MESH_ROLE":"agent", "WIFI_BACKHAUL_STA_MAC":"82:3f:5d:11:22:33", "BH_TYPE":"2G/5G/MLO/LAN", "RSSI":"-50/NONE", 
        "BAND_2G":[
        {
            "RADIO0": [{"MAC":"80:3f:5d:11:22:35", "stationList":[{"mac":"52:60:3b:b2:8b:48","MLO":"1","rssi":"-65", "rate":"866Mbps"}]}],
            "RADIO1": [{"MAC":"NONE"}],
            "RADIO2": [{"MAC":"NONE"}],
            "RADIO3": [{"MAC":"NONE"}],
        }
        ],
        "BAND_5G":[
        {
            "RADIO0": [{"MAC":"80:3f:5d:11:22:36", "stationList":[{"mac":"11:60:3b:b2:8b:48","MLO":"1","rssi":"-45", "rate":"866Mbps"}]}],
            "RADIO1": [{"MAC":"NONE"}],
            "RADIO2": [{"MAC":"NONE"}],
            "RADIO3": [{"MAC":"NONE"}],
        }
        ],
        "BAND_6G":[
        {
            "RADIO0": [{"MAC":"80:3f:5d:11:22:37", "stationList":[{"mac":"87:21:3b:b2:8b:48","MLO":"1","rssi":"-15", "rate":"2400Mbps"}]}],
            "RADIO1": [{"MAC":"NONE"}],
            "RADIO2": [{"MAC":"NONE"}],
            "RADIO3": [{"MAC":"NONE"}],
        }
        ],
    }
}

```

#### Topology Member notify: ap/topology_member_notify
```
{
    "_msg_type":"topology_member_notify",
    "_mesh_version":"1.0",
    "_msg_data":{
        "EVENT" : "NEW_AGENT_JOIN/AGENT_OFFLINE",
        "AL_MAC" : "80:3f:5d:11:22:37",
    }
}
```



#### STEER request: ap/steer_request : 
sent by agnet when agent need to do bandsteering or roaming
controller collect RRM from all node
```
{
    "_msg_type":"steer_request",
    "_mesh_version":"1.0",
    "_msg_data":{
        "STA_MAC" : "41:3f:5d:11:33:12",
        "TARGET_BSSID" : "80:3f:5d:11:22:37/FF:FF:FF:FF:FF:FF",//bandsteering or  roaming, 
        "channel" : "52",
        "rssi" : "-70",
    }
}
```
#### AIR MONITOR REQUEST: ap/air_monitor_request
```
{
    "_msg_type":"air_monitor_request",
    "_mesh_version":"1.0",
    "_msg_data":{
        "STA_MAC" : "41:3f:5d:11:33:12",
        "ACTION" : "JOIN/LEAVE",
    }
}

```



#### UNASSOC STA LINK METRICS QUERY: ap/unassoc_sta_query
```
{
    "_msg_type":"unassoc_sta_query",
    "_mesh_version":"1.0",
    "_msg_data":{
        "STA_MAC" : "41:3f:5d:11:33:12",
        "op_class" : "115",
        "channel" : "52",
    }
}

```

#### UNASSOC STA LINK METRICS REPORT: ap/unassoc_sta_report
```
{
    "_msg_type":"unassoc_sta_report",
    "_mesh_version":"1.0",
    "_msg_data":{
        "STA_MAC" : "41:3f:5d:11:33:12",
        "AL_MAC" : "80:3f:5d:11:22:33",
        "BSSID" : "82:3f:5d:11:22:33",
        "op_class" : "115",
        "channel" : "52",
        "RSSI" : "-28",
    }
}
```
#### RRM REQUEST ap/rrm_request
```
    "_msg_type":"rrm_request",
    "_mesh_version":"1.0",
    "_msg_data":{
        "STA_MAC" : "41:3f:5d:11:33:12",
        "TARGET_AL_MAC":"80:3f:5d:11:22:33",
        "op_class" : "115",
        "channel" : "52",
    }
```

#### RRM REQUEST ap/rrm_report
```
    "_msg_type":"rrm_request",
    "_mesh_version":"1.0",
    "_msg_data":{
        "STA_MAC" : "41:3f:5d:11:33:12",
        "BSSID":"82:3f:5d:11:22:33",
        "op_class" : "115",
        "channel" : "52",
        "RCPI":"128",

    }
```

#### BTM REPORT: ap/btm_report
```
{
    "_msg_type":"btm_report",
    "_mesh_version":"1.0",
    "_msg_data":{
        "STA_MAC" : "41:3f:5d:11:33:12",
        "CONNECTING_BSSID" : "82:3f:5d:11:22:33",
        "STEERTO_BSSID" : "82:3f:5d:01:22:33",
        "DISASSOC" : "1/0",
    }
}
```


#### 802.11R PMKID REPORT: ap/ieee80211r_pmkid_report
```
{
    "_msg_type":"ieee80211r_pmkid_report",
    "_mesh_version":"1.0",
    "_msg_data":{
        "STA_MAC" : "41:3f:5d:11:33:12",
        "PMKID" : "61:70:2e:6d:74:6b:2e:63:6f:6d:",
    }
}
```

#### HEARTBEAT: ap/sync_heartbeat
```
{
    "_msg_type":"sync_heartbeat",
    "_mesh_version":"1.0",
    "_msg_data":{
        "ap_settings_version_code" : "166982(timestamp)",
        "ap_member_version_code" : "166982(timestamp)",
    }
}
```


### AP SETTINGS REQUEST:
* ap/mesh/ap_settings_request
* ap/group1/ap_settings_request
* ap/group2/ap_settings_request
* ap/group3/ap_settings_request
```
{
    "_msg_type":"ap_settings_request",
    "_mesh_version":"1.0",
    "_settings_version":"166982(timestamp)",
    "_msg_data":{
        "AL_MAC":"80:3f:5d:11:22:33"
    }
}
```

### AP SETTINGS RESPONSE: 
* ap/mesh/ap_settings_response
* ap/group1/ap_settings_response
* ap/group2/ap_settings_response
* ap/group3/ap_settings_response
```
{
    "_msg_type":"ap_settings_response",
    "_mesh_version":"1.0",
    "_settings_version":"166982(timestamp)",
    "_msg_data":{
        "_wifi_settings": {
            "_2g_settings":{
                "_enable":"1",
                "_countrycode":"CN",
                "_htmode":"20M",
                "_radio0":{
                    "_enable":"1",
                    "_ssid":"OpenWrt",
                    "_encryption":"psk2+ccmp",
                    "_password":"12345678",
                    "_backhaul":"0",
                },
                "_radio1":{
                    "_enable":"1",
                    "_ssid":"OpenWrt1",
                    "_encryption":"psk2+ccmp",
                    "_password":"12345678",
                    "_backhaul":"0",
                },
                "_radio2":{
                    "_enable":"1",
                    "_ssid":"OpenWrt2",
                    "_encryption":"psk2+ccmp",
                    "_password":"12345678",
                    "_backhaul":"0",
                },
                "_radio3":{
                    "_enable":"1",
                    "_ssid":"OpenWrt3",
                    "_encryption":"psk2+ccmp",
                    "_password":"12345678",
                    "_backhaul":"0",
                }
            },
            "_5g_settings":{
                "_enable":"1",
                "_countrycode":"CN",
                "_htmode":"20M",
                "_radio0":{
                    "_enable":"1",
                    "_ssid":"OpenWrt",
                    "_encryption":"psk2+ccmp",
                    "_password":"12345678",
                    "_backhaul":"0",
                },
                "_radio1":{
                    "_enable":"1",
                    "_ssid":"OpenWrt1",
                    "_encryption":"psk2+ccmp",
                    "_password":"12345678",
                    "_backhaul":"0",
                },
                "_radio2":{
                    "_enable":"1",
                    "_ssid":"OpenWrt2",
                    "_encryption":"psk2+ccmp",
                    "_password":"12345678",
                    "_backhaul":"0",
                },
                "_radio3":{
                    "_enable":"1",
                    "_ssid":"OpenWrt3",
                    "_encryption":"psk2+ccmp",
                    "_password":"12345678",
                    "_backhaul":"1",
                }
            },
            "_6g_settings":{
                "_enable":"1",
                "_countrycode":"CN",
                "_htmode":"320M",
                "_radio0":{
                    "_enable":"1",
                    "_ssid":"OpenWrt",
                    "_encryption":"psk2+ccmp",
                    "_password":"12345678",
                    "_backhaul":"0",
                },
                "_radio1":{
                    "_enable":"1",
                    "_ssid":"OpenWrt1",
                    "_encryption":"psk2+ccmp",
                    "_password":"12345678",
                    "_backhaul":"0",
                },
                "_radio2":{
                    "_enable":"1",
                    "_ssid":"OpenWrt2",
                    "_encryption":"psk2+ccmp",
                    "_password":"12345678",
                    "_backhaul":"0",
                },
                "_radio3":{
                    "_enable":"1",
                    "_ssid":"OpenWrt3",
                    "_encryption":"psk2+ccmp",
                    "_password":"12345678",
                    "_backhaul":"0",
                }
            },

        },
        "_system_settings": {
            "_timezone":"UTC+8:00",
        }
    }
}
```