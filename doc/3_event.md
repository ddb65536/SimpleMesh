#### NEW agent connect to controller

```mermaid
sequenceDiagram
    participant controller
    participant agentA
    participant agentB

    agentA->>controller: Publish Topic(Topology report)
    agentA->>agentB: Publish Topic(Topology report)
    controller->>agentA: Publish Topic(Topology Member notify)
    controller->>agentB:  Publish Topic(Topology Member notify)

    
```

#### Auto-Configuration
```mermaid
sequenceDiagram
    participant controller
    participant agentA

    controller->>agentA: Publish Topic(ap/sync_heartbeat)
    controller->>agentA: compare settings version code and Publish Topic(ap/mesh/ap_settings_request)
    controller->>agentA:  Publish Topic(ap/mesh/ap_settings_response)

```

#### 11R 
```mermaid
sequenceDiagram
    participant ALL_MESH_NODE
    participant agentA
    participant IPHONE

    IPHONE->>agentA: get 11R PMKID from hostapd
    agentA->>ALL_MESH_NODE: Publish Topic(ap/ieee80211r_pmkid_report)

```

#### BANDSTEERING 2G-->5G
```mermaid
sequenceDiagram
    participant agentA
    participant IPHONE

    IPHONE->>agentA: iphone Connect to agentA 2G and 2G rssi is low
    agentA->>IPHONE: send beacon request to STA(target BSSID is agentA's 5G)
    IPHONE->>agentA:  receive beacon report, send BTM to steer iphone if 5G rssi is good.

```

#### BANDSTEERING 5G-->2G
```mermaid
sequenceDiagram
    participant agentA
    participant IPHONE

    IPHONE->>agentA: iphone Connect to agentA 5G and 2G rssi is low
    agentA->>IPHONE: send beacon request to STA(target BSSID is agentA's 2G)
    IPHONE->>agentA:  receive beacon report, send BTM to steer iphone if 2G rssi is better than 2G.

```


#### ROAMING & AIR Monitor

Case 1: roaming then rssi is terrible
```mermaid
sequenceDiagram
    participant controller
    participant agentA
    participant agentB
    participant IPHONE

    IPHONE->>agentA: iphone Connect to agentA
    agentA->>controller: Publish Topic(ap/air_monitor_request) , start to monitor this iphone's probe request
    agentA->>agentB: Publish Topic(ap/air_monitor_request) , start to monitor this iphone's probe request
    agentA->>controller: Publish Topic(ap/steer_request) when sta rssi is lower than roaming threshold(default -70dbm)
    agentA->>controller: Publish multi Topic(ap/rrm_request), target BSSID is every mesh_node's 5G/2G mac.
    controller->>agentA: Publish Topic(ap/unassoc_sta_report) if recvice probe request from iphone
    agentB->>agentA: Publish Topic(ap/unassoc_sta_report) if recvice probe request from iphone
    agentA->>IPHONE : compare unassoc_sta_report and select best mesh node
    agentA->>controller:  Publish Topic(ap/btm_report), tell controller i will steer iphone to another mesh node.

```

case 2: Auto-roaming trigger by air monitor
```mermaid
sequenceDiagram
    participant OTHER_MESH_NODE
    participant agentA
    participant IPHONE

    IPHONE->>agentA: iphone Connect to agentA
    agentA->>OTHER_MESH_NODE: Publish Topic(ap/air_monitor_request) , start to monitor this iphone's probe request
    OTHER_MESH_NODE->>agentA: Publish Topic(ap/unassoc_sta_report) when receive iphone's probe request
    agentA->>IPHONE: Send BTM request if target bssid rssi is better than current mesh node
    agentA->>OTHER_MESH_NODE:  Publish Topic(ap/btm_report), tell control i will steer iphone to another mesh node.ler

```