# 220TcpBulkSend Script

This script is based on ns-3 `examples/tcp/tcp-bulk-send.cc` script.
It is used to duplicate a TCP throughput issue and to find a solution 
to it.  This issue has been resolved.  See below for details.

Use the following shell script to run it:
 * go.sh            - for development run

Example output:

```
% ./go.sh
Total Bytes Received: 1164596
Throughput (Mbps): 0.931677
```

With __default ns-3 TCP configuration parameters__, running the script with 
respect to different channel capacity gives us the following result:

|  Link Capacity  |  TCP throughput   |  Percentage  |
|  -------------  |  --------------   |  ----------  |
|      500Kbps    |    443Kbps        |        88.6% |
|      1Mbps      |    907Kbps        |        90.7% |
|      10Mbps     |    9.06Mbps       |        90.6% |
|      100Mbps    |    90Mbps         |        90.0% |
|      1Gbps      |    104Mbps        |        10.4% |

Clearly, when the channel capacity reaches 1Gbps, we have a 
TCP throughput issue.

To overcome the problem, one must increase the send and receive buffer sizes:
```C++
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1436));     // default 536
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(1310720));   // default 131072
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1310720));   // default 131072
```
The `SegmentSize` plays a minor role as well.

|  Link Capacity  |  TCP throughput   |  Percentage  |
|  -------------  |  --------------   |  ----------  |
|      1Gbps      |    946Mbps        |        94.6% |
|      10Gbps     |    1.04Gbps       |        10.4% |

We see the same throughput issue.  To resolve the issue, we have to further increase 
the send/receive buffer size by 10 times.

|  Link Capacity  |  TCP throughput   |  Percentage  |
|  -------------  |  --------------   |  ----------  |
|      10Gbps     |    9.43Gbps       |        94.3% |

(End)