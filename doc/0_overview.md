# SimpleMesh 
SimpleMesh
*  sm_controller: Mesh controller , subscribe agent topic, submit controller topic
*  sm_agent: Mesh agent, subscribe agent topic and controller topic, submit agent topic
*  sm_wsta_m: wireless STA monitor, monitor wireless STA status, get/send wifi management pkt to STA.
*  sm_sys_m: system monitor, get/set router settings/monitor lan network

# Communication
```mermaid
graph LR;
    A["sm_wsta_m(local socket)"] --> C["sm_controller(mqtt clinet)"] 
    B["sm_sys_m(local socket)"] --> C;

    D["sm_wsta_m(local socket)"] --> F["sm_agent(mqtt clinet)"];
    E["sm_sys_m(local socket)"] --> F;

    G["MQTT Broker"]
    F<-->G
    C<-->G
```

# Login

```mermaid
sequenceDiagram
    participant controller
    participant agentA

    agentA->>controller: sm_login SSDP discover(0.0.0.0 -> 255.255.255.255:19011)
    controller->>agentA: sm_login offer(controller_ip->255.255.255.255:19012) distribute IP to agentB
    agentA->>controller: sm_login req (agent_ip:19011 -> controller_ip:19011)
    controller->>agentA: sm_login response (controller_ip:19011 -> controller_ip:19012)
    
```