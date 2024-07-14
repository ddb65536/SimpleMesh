# 802.11v WNM

### WNM include:
* BSS Max idle period management
* BSS transition management(BTM)
* Channel usage
* Collocated interference reporting, Diagnostic reporting, Event * reporting, Multicast diagnostic reporting
* DMS (Directed Multicast Service)
* FMS (Flexible Multicast Service)
* Location services
* Multiple BSSID capability
* Proxy ARP
* QoS traffic capability
* SSID list
* Triggered TA statistics
* TIM broadcast
* Timing measurement
* Traffic filtering service
* WNM-Sleep mode



##### How to send BTM request via ubus+hostapd (test on mt7621+7603+7613)
1. Enable 11v in your wifi radio:
```
wireless.wlan1_1=wifi-iface
wireless.wlan1_1.device='radio1'
wireless.wlan1_1.network='lan'
wireless.wlan1_1.ifname='wlan1-1'
wireless.wlan1_1.mode='ap'
wireless.wlan1_1.ssid='OpenWrt_11v_test'
wireless.wlan1_1.encryption='psk2+ccmp'
wireless.wlan1_1.key='12345678'
wireless.wlan1_1.ieee80211w='0'
wireless.wlan1_1.ieee80211k='1'
wireless.wlan1_1.ieee80211v='1'
```
2. get BTM target neighour information
```
Smart connect to Mesh Router

call command at Mesh Extender Node
root@OpenFi:/# ubus call hostapd.wlan1-1 rrm_nr_get_own
{
        "value": [
                "aa:5a:f1:12:00:63",
                "OpenWrt_11v_test",
                "aa5af1120063ef190000809d090603029b00"
        ]
}
```

3. recv ubus hostapd event 
```
ubus subscribe hostapd.wlan1-1
```
4. send beacon request
```
root@OpenWrt:/# ubus -v list hostapd.wlan1
'hostapd.wlan1' @f0f6bff2
...
        "bss_transition_request":{"addr":"String","disassociation_imminent":"Boolean","disassociation_timer":"Integer","validity_period":"Integer","neighbors":"Array","abridged":"Boolean","dialog_token":"Integer","mbo_reason":"Integer","cell_pref":"Integer","reassoc_delay":"Integer"}
...

```

```
root@OpenFi:/# ubus call  hostapd.wlan1-2 bss_transition_request '{ "addr": "42:
48:16:6b:83:ee ", "disassociation_imminent": true, "disassociation_timer": 10000
, "validity_period": 30, "neighbors": ["aa5af1120063ef190000809d090603029b00"], 
"abridged": 1 }'

```

5. check the result
```
smart phone is steered to another Mesh Node.
but no BTM RESPONSE show in ubus


{ "auth": {"address":"42:48:16:6b:83:ee","target":"ae:5a:f1:12:00:4b","signal":-36,"freq":5745} }
{ "disassoc": {"address":"42:48:16:6b:83:ee"} }
{ "assoc": {"address":"42:48:16:6b:83:ee","target":"ae:5a:f1:12:00:4b","signal":-36,"freq":5745} }
{ "sta-authorized": {"address":"42:48:16:6b:83:ee","auth-alg":"open"} }
{ "probe": {"address":"42:48:16:6b:83:ee","target":"ff:ff:ff:ff:ff:ff","signal":-37,"freq":5745,"ht_capabilities":{"ht_capabilities_info":2543,"supported_mcs_set":{"a_mpdu_params":23,"ht_extended_capabilities":0,"tx_bf_capability_info":0,"asel_capabilities":0,"supported_mcs_set":[255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0]}},"vht_capabilities":{"vht_capabilities_info":865204726,"vht_supported_mcs_set":{"rx_map":-6,"rx_highest":780,"tx_map":-6,"tx_highest":8972}}} }
{ "probe": {"address":"42:48:16:6b:83:ee","target":"ff:ff:ff:ff:ff:ff","signal":-37,"freq":5745,"ht_capabilities":{"ht_capabilities_info":2543,"supported_mcs_set":{"a_mpdu_params":23,"ht_extended_capabilities":0,"tx_bf_capability_info":0,"asel_capabilities":0,"supported_mcs_set":[255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0]}},"vht_capabilities":{"vht_capabilities_info":865204726,"vht_supported_mcs_set":{"rx_map":-6,"rx_highest":780,"tx_map":-6,"tx_highest":8972}}} }
{ "probe": {"address":"42:48:16:6b:83:ee","target":"ff:ff:ff:ff:ff:ff","signal":-38,"freq":5745,"ht_capabilities":{"ht_capabilities_info":2543,"supported_mcs_set":{"a_mpdu_params":23,"ht_extended_capabilities":0,"tx_bf_capability_info":0,"asel_capabilities":0,"supported_mcs_set":[255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0]}},"vht_capabilities":{"vht_capabilities_info":865204726,"vht_supported_mcs_set":{"rx_map":-6,"rx_highest":780,"tx_map":-6,"tx_highest":8972}}} }
{ "probe": {"address":"42:48:16:6b:83:ee","target":"ff:ff:ff:ff:ff:ff","signal":-38,"freq":5745,"ht_capabilities":{"ht_capabilities_info":2543,"supported_mcs_set":{"a_mpdu_params":23,"ht_extended_capabilities":0,"tx_bf_capability_info":0,"asel_capabilities":0,"supported_mcs_set":[255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0]}},"vht_capabilities":{"vht_capabilities_info":865204726,"vht_supported_mcs_set":{"rx_map":-6,"rx_highest":780,"tx_map":-6,"tx_highest":8972}}} }
{ "disassoc": {"address":"42:48:16:6b:83:ee"} }
{ "inactive-deauth": {"address":"42:48:16:6b:83:ee"} }
```