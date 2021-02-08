# Application Protocols Comparison in Constrained Device Applications
Bench testing and comparison of application protocols for mesh networks using ESP8266, PC and Wireshark as part of the Bachelor of Electronics and ICT course of Engineering Experience 5, count toward the bachelor thesis, at KU Leuven 2019-2020.
The report detailing the background information, setup, tests, results and comparison of MQTT, CoAP and HTTP application protocols for constrained devices in mesh networks is attached to the repository as a PDF file.

Generic testbench layout:
![Diagram of logical layout for the bench tests](https://github.com/maximyudayev/ee5-application-and-session-protocols/blob/master/images/Picture%201.png)

MQTT working principle:
![Diagram of logical topology for MQTT applications](https://github.com/maximyudayev/ee5-application-and-session-protocols/blob/master/images/Picture%202.png)

CoAP and HTTP working principle:
![Diagram of logical topology for CoAP and HTTP applications](https://github.com/maximyudayev/ee5-application-and-session-protocols/blob/master/images/Picture%203.png)

## Test Setup
MQTT test setup:
![Diagram of MQTT test setup](https://github.com/maximyudayev/ee5-application-and-session-protocols/blob/master/images/Picture%204.png)

CoAP and HTTP test setup (no proxy was used):
![Diagram of CoAP and HTTP test setup](https://github.com/maximyudayev/ee5-application-and-session-protocols/blob/master/images/Picture%205.png)

## Brief Conclusions - Graphs
Throughput comparison of studied protocols:
![Chart of throughput comparison of MQTT, CoAP and HTTP in constrained devices](https://github.com/maximyudayev/ee5-application-and-session-protocols/blob/master/images/Picture%2011.png)

Roundtrip comparison of studied protocols:
![Chart of roundtrip comparison of MQTT, CoAP and HTTP in constrained devices](https://github.com/maximyudayev/ee5-application-and-session-protocols/blob/master/images/Picture%2012.png)

Frameloss comparison of studied protocols due to constrained device limitations:
![Chart of frameloss comparison of MQTT, CoAP and HTTP in constrained devices](https://github.com/maximyudayev/ee5-application-and-session-protocols/blob/master/images/Picture%2010.png)

Retransmission comparison of studied protocols:
![Chart of retransmission comparison of MQTT, CoAP and HTTP in constrained devices](https://github.com/maximyudayev/ee5-application-and-session-protocols/blob/master/images/Picture%2013.png)

HTTP packet loss due to network delays:
![Chart of HTTP packet loss due to network delays in constrained devices](https://github.com/maximyudayev/ee5-application-and-session-protocols/blob/master/images/Picture%2014.png)

MQTT packet loss comparison between different Quality of Service due to constrained device limitations:
![Chart of retransmission comparison of different MQTT QoS in constrained devices](https://github.com/maximyudayev/ee5-application-and-session-protocols/blob/master/images/Picture%206.png)

MQTT retransmission comparison between different Quality of Service:
![Chart of retransmission comparison of different MQTT QoS in constrained devices](https://github.com/maximyudayev/ee5-application-and-session-protocols/blob/master/images/Picture%209.png)

MQTT roundtrip comparison between different Quality of Service:
![Chart of roundtrip comparison of different MQTT QoS in constrained devices](https://github.com/maximyudayev/ee5-application-and-session-protocols/blob/master/images/Picture%208.png)

MQTT throughput comparison between different Quality of Service:
![Chart of throughput comparison of different MQTT QoS in constrained devices](https://github.com/maximyudayev/ee5-application-and-session-protocols/blob/master/images/Picture%207.png)
