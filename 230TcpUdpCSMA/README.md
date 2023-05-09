# 230TcpUdpCSMA Script

This script is based on ns-3 `examples/udp-client-server/udp-client-server.cc` script.
It is used to assess UDP / TCP throughput in CSMA net.

Use the following shell script to run it:
 * goUDP.sh             - test run using UDP
 * goTCP.sh             - test run using TCP

Example output:

```
% ./goUDP.sh
Total Tx: 2048000 bytes
Tx Rate: 2.048 Mbps
Received: 1978
Rx Rate: 2.02547 Mbps
```

```
% ./goTCP.sh 
Total Bytes Received: 3146240
Throughput (Mbps): 3.14624
```

# Results

## Experiment 1: Link speed = 5Mbps, propagation delay = 2ms, default MTU = 1500.

### UDP run: UDP packet size = 1024 bytes, interpacket interval = 4ms.

__Note: the interpacket interval is set to two times of the 
propagation delay for CSMA.__

```
% ./goUDP.sh
Total Tx: 2048000 bytes
Tx Rate: 2.048 Mbps
Received: 1978
Rx Rate: 2.02547 Mbps
```

__The Rx throughput amounts to 40% efficiency of theoretical bandwidth.__
Also note that the Rx rate is close to the Tx rate.  It is hard 
to further increase the Rx rate due to the propagation delay.
__Increasing the link speed without reducing propagation delay 
won't be able to improve the Rx rate.__

### TCP Run

Under default setting: TCP segment size = 536, both 
TCP send and receive buffer sizes are 131072.

```
% ./goTCP.sh 
Total Bytes Received: 969624
Throughput (Mbps): 0.969624
```

__The Rx throughput amounts to 19% efficiency of theoretical bandwidth.__

After:
```C++
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1024));     // default 536
```
The throughput is improved.  See below:
```
% ./goTCP.sh
Total Bytes Received: 1517568
Throughput (Mbps): 1.51757
```

Further increase:
```C++
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1436));     // default 536
```
We have:
```
% ./goTCP.sh
Total Bytes Received: 1872656
Throughput (Mbps): 1.87266
```

After increasing send/recv buffer:
```C++
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(1310720));   // default 131072
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1310720));   // default 131072
```
We have:
```
% ./goTCP.sh
Total Bytes Received: 1944344
Throughput (Mbps): 1.94434
```
__This result cannot be further improved.__ The minimum send/recv buffer to yield 
such throughput result is slightly above 2 times of the default send/recv buffer size.

## Experiment 2: Link speed = 10Mbps, propagation delay = 1ms, default MTU = 1500.

### UDP run: UDP packet size = 1024 bytes, interpacket interval = 2ms.

__Note: the interpacket interval is set to two times of the 
propagation delay for CSMA.__

```
% ./goUDP.sh
Total Tx: 4096000 bytes
Tx Rate: 4.096 Mbps
Received: 3652
Rx Rate: 3.73965 Mbps
```

__The Rx throughput amounts to 37% efficiency of theoretical bandwidth.__
Also note that the Rx rate is close to the Tx rate.  It is hard 
to further increase the Rx rate due to the propagation delay.

Increasing packet size to 1436:
```
% ./goUDP.sh
Total Tx: 5744000 bytes
Tx Rate: 5.744 Mbps
Received: 3174
Rx Rate: 4.55786 Mbps
```

### TCP Run

Under default setting: TCP segment size = 536, both 
TCP send and receive buffer sizes are 131072.

```
% ./goTCP.sh
Total Bytes Received: 1804712
Throughput (Mbps): 1.80471
```

__The Rx throughput amounts to 18% efficiency of theoretical bandwidth.__

After:
```C++
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1024));     // default 536
```
The throughput is improved.  See below:
```
% ./goTCP.sh
Total Bytes Received: 2586624
Throughput (Mbps): 2.58662
```

Further increase:
```C++
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1436));     // default 536
```
We have:
```
% ./goTCP.sh
Total Bytes Received: 3500136
Throughput (Mbps): 3.50014
```

After increasing send/recv buffer:
```C++
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(131072*2));   // default 131072
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(131072*2));   // default 131072
```
We have:
```
% ./goTCP.sh
Total Bytes Received: 3693392
Throughput (Mbps): 3.69339
```
__This result cannot be further improved.__ The minimum send/recv buffer to yield 
such throughput result is slightly below 2 times of the default send/recv buffer size.

## Experiment 3: Link speed = 100Mbps, propagation delay = 50us, default MTU = 1500.

### UDP run: UDP packet size = 1024 bytes, interpacket interval = 100us.

__Note: the interpacket interval is set to two times of the 
propagation delay for CSMA.__

```
% ./goUDP.sh
Total Tx: 81920000 bytes
Tx Rate: 81.92 Mbps
Received: 46528
Rx Rate: 47.6447 Mbps
```

__The Rx throughput amounts to 47.6% efficiency of theoretical bandwidth.__
It is hard to further increase the Rx rate due to the propagation delay.

Here, if we increase the interpacket interval to 150us, the throughput 
remains the same. See below:

```
% ./goUDP.sh
Total Tx: 54614016 bytes
Tx Rate: 54.614 Mbps
Received: 46528
Rx Rate: 47.6447 Mbps
```

It shows that reducing the interpacket interval won't always work. The 
propagation delay plays a big role in the efficiency of a CSMA network.

Increasing packet size to 1436 (helps with the throughput):
```
% ./goUDP.sh
Total Tx: 76587624 bytes
Tx Rate: 76.5876 Mbps
Received: 39026
Rx Rate: 56.0413 Mbps
```
The above result was from interpacket packet interval being 150us. Reducing 
the interpacket interval to 100us does not help improve the Rx rate (but only 
increase the Tx rate furthermore).  If the interpacket interval is increased 
to 200us, the throughput remains the same.

### TCP Run

Under default setting: TCP segment size = 536, both 
TCP send and receive buffer sizes are 131072.

```
% ./goTCP.sh
Total Bytes Received: 23658496
Throughput (Mbps): 23.6585
```

__The Rx throughput amounts to 23.6% efficiency of theoretical bandwidth.__

After:
```C++
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1024));     // default 536
```
The throughput is improved.  See below:
```
% ./goTCP.sh
Total Bytes Received: 36723712
Throughput (Mbps): 36.7237
```

Further increase:
```C++
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1436));     // default 536
```
We have:
```
% ./goTCP.sh
Total Bytes Received: 39145472
Throughput (Mbps): 39.1455
```

After increasing send/recv buffer:
```C++
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(131072*2));   // default 131072
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(131072*2));   // default 131072
```
We have:
```
% ./goTCP.sh
Total Bytes Received: 44945920
Throughput (Mbps): 44.9459
```
__This result can only be slightly improved if the buffer size is further increased.__

## Experiment 4: Link speed = 1Gbps, propagation delay = 503ns, default MTU = 1500.

### UDP run: UDP packet size = 1024 bytes, interpacket interval = 8us.

__Note: the interpacket interval is set to a value so that the Tx rate 
is slightly higher than the link speed.__

```
% ./goUDP.sh
Total Tx: 1024000000 bytes
Tx Rate: 1024 Mbps
Received: 827671
Rx Rate: 847.535 Mbps
```

__The Rx throughput amounts to 85% efficiency of theoretical bandwidth.__
It is hard to further increase the Rx rate due to the propagation delay.
Decreasing the interpacket interval (increasing the Tx rate) does not 
improve the throughput.

__It is clear that the smaller the propagation delay, the better the efficiency.__
It shows that reducing the interpacket interval won't always work. The 
propagation delay plays a big role in the efficiency of a CSMA network.

Increasing packet size to 1436 (helps with the throughput):
```
% ./goUDP.sh
Total Tx: 1436000000 bytes
Tx Rate: 1436 Mbps
Received: 617047
Rx Rate: 886.079 Mbps
```
The above result was from interpacket packet interval being 8us. If the 
interpacket interval is increased to 10us, the throughput remains the same.

### TCP Run

Under default setting: TCP segment size = 536, both 
TCP send and receive buffer sizes are 131072.

```
% ./goTCP.sh 
Total Bytes Received: 469734400
Throughput (Mbps): 469.734
```

__The Rx throughput amounts to 47% efficiency of theoretical bandwidth.__

After:
```C++
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1024));     // default 536
```
The throughput is improved.  See below:
```
% ./goTCP.sh
Total Bytes Received: 552956928
Throughput (Mbps): 552.957
```

Further increase:
```C++
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1436));     // default 536
```
We have:
```
% ./goTCP.sh
Total Bytes Received: 562828800
Throughput (Mbps): 562.829
```

After increasing send/recv buffer:
```C++
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(1310720));   // default 131072
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1310720));   // default 131072
```
We have:
```
% ./goTCP.sh
Total Bytes Received: 680455680
Throughput (Mbps): 680.456
```
__This result can be further improved if the buffer size is further increased.__
When the buffers are increased by a factor of 5, we have:
```
% ./goTCP.sh
Total Bytes Received: 718802944
Throughput (Mbps): 718.803
```
This may still be further improved if the buffer sizes are increased (but the 
simulation becomes very slow).

Increasing the SendSize of the BulkSendApplication also helps with improving 
the throughput:
```C++
    Config::SetDefault("ns3::BulkSendApplication::SendSize", UintegerValue(1436));    // default 512
```
```
% ./goTCP.sh
Total Bytes Received: 745200712
Throughput (Mbps): 745.201
```
The above result uses Socket buffer sizes of 1310720*5 with BulkSendApplication SendSize 
set to 1436.  If I increase the SendSize to 1436*2, the result is the same.

(End)