# WDS 4ADDESS MULTI-AP backhaul TEST

#### Mesh Node and Mesh Node connection is WDS 4address Multi-ap connection

#### Test WDS 4address Multi-AP
* Enable Multi-ap on Mesh Main Router
    multi_ap = (0->Disabled, 1->Only BH Support, 2->Only FH Support, 3->FH+BH Support)
```
wireless.wlan1=wifi-iface
wireless.wlan1.device='radio1'
wireless.wlan1.ifname='wlan1'
wireless.wlan1.network='lan'
wireless.wlan1.mode='ap'
wireless.wlan1.ssid='OpenWrt_test'
wireless.wlan1.encryption='psk2+ccmp'
wireless.wlan1.key='12345678'
wireless.wlan1.ieee80211w='0'
wireless.wlan1.ieee80211k='1'
wireless.wlan1.multi_ap='1'
```

```
cat /var/run/hostapd-phy1.conf 
driver=nl80211
logger_syslog=127
logger_syslog_level=2
logger_stdout=127
logger_stdout_level=2
country_code=CN
ieee80211d=1
ieee80211h=1
hw_mode=a
beacon_int=100
channel=149
chanlist=149
noscan=1
tx_queue_data2_burst=2.0
ieee80211n=1
ht_coex=0
ht_capab=[HT40+][LDPC][SHORT-GI-20][SHORT-GI-40][TX-STBC][RX-STBC1]
ieee80211ac=1
vht_oper_chwidth=1
vht_oper_centr_freq_seg0_idx=155
vht_capab=[RXLDPC][SHORT-GI-80][SHORT-GI-160][TX-STBC-2BY1][RX-ANTENNA-PATTERN][TX-ANTENNA-PATTERN][RX-STBC-1][VHT160-80PLUS80][MAX-A-MPDU-LEN-EXP7]

radio_config_id=8db41946e813f08ebc28cb07dfe0d512
interface=wlan1
ctrl_interface=/var/run/hostapd
ap_isolate=1
bss_load_update_period=60
chan_util_avg_period=600
disassoc_low_ack=1
skip_inactivity_poll=0
preamble=1
wmm_enabled=1
ignore_broadcast_ssid=0
uapsd_advertisement_enabled=1
utf8_ssid=1
multi_ap=1
nas_identifier=aa5af112004b
wpa_passphrase=12345678
wpa_psk_file=/var/run/hostapd-wlan1.psk
auth_algs=1
wpa=2
wpa_pairwise=CCMP
ssid=OpenWrt_test
bridge=br-lan
wds_bridge=
snoop_iface=br-lan
rrm_neighbor_report=1
rrm_beacon_report=1
wpa_disable_eapol_key_retries=0
wpa_key_mgmt=WPA-PSK
okc=0
disable_pmksa_caching=1
ieee80211w=0
dynamic_vlan=0
vlan_naming=1
vlan_no_bridge=1
vlan_file=/var/run/hostapd-wlan1.vlan
qos_map_set=0,0,2,16,1,1,255,255,18,22,24,38,40,40,44,46,48,56
config_id=61e898f46cadeabf7fe5185f971061db
bssid=aa:5a:f1:12:00:4b
```

Enable Multi-ap in Mesh Extender Node
```
wireless.wclinet_sta=wifi-iface
wireless.wclinet_sta.network='lan'
wireless.wclinet_sta.ifname='wlan1-sta'
wireless.wclinet_sta.device='radio1'
wireless.wclinet_sta.mode='sta'
wireless.wclinet_sta.encryption='psk2+ccmp'
wireless.wclinet_sta.ssid='OpenWrt_test'
wireless.wclinet_sta.key='12345678'
wireless.wclinet_sta.multi_ap='1'
```

Check Result at main router
```
wifi driver will create new interface for Multi-ap 

wlan1.sta1 Link encap:Ethernet  HWaddr **:**:**:12:00:4B  
          inet6 addr: fe80::a85a:f1ff:fe12:4b/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:5840 errors:0 dropped:0 overruns:0 frame:0
          TX packets:3053 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:393898 (384.6 KiB)  TX bytes:302043 (294.9 KiB)
```
Check Result at Mesh Extender Node
```
phy#1
        Interface wlan1
                ifindex 47
                wdev 0x100000015
                addr **:**:**:12:00:63
                ssid OpenWrt-extender
                type AP
                channel 149 (5745 MHz), width: 80 MHz, center1: 5775 MHz
                txpower 25.00 dBm
                multicast TXQ:
                        qsz-byt qsz-pkt flows   drops   marks   overlmt hashcol tx-bytes        tx-packets
                        0       0       0       0       0       0       0       0               0
        Interface wlan1-sta
                ifindex 40
                wdev 0x100000012
                addr **:**:**:12:00:63
                ssid OpenWrt_test
                type managed
                channel 149 (5745 MHz), width: 20 MHz (no HT), center1: 5745 MHz
                txpower 25.00 dBm
                multicast TXQ:
                        qsz-byt qsz-pkt flows   drops   marks   overlmt hashcol tx-bytes        tx-packets
                        0       0       0       0       0       0       0       0               0
                4addr: on
phy#0
```

Check the PC mac is replaced or not 
PC(40:c2:ba:40:85:b0 )--Lan->Mesh Extender Node ----WIFI backhaul---> Mesh Router
```
Mesh Extender Node:
root@OpenWrt:/# arp
IP address       HW type     Flags       HW address            Mask     Device
192.168.21.100   0x1         0x2         40:c2:ba:40:85:b0     *        br-lan
```
```
Mesh Router Node:
root@OpenWrt:~# arp
IP address       HW type     Flags       HW address            Mask     Device
192.168.21.100   0x1         0x2         40:c2:ba:40:85:b0     *        br-lan
```