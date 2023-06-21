# 210FTPCalib Script

This script is used to find out the right set of parameter for the 
simulations involving FTP-type flows in a CSMA net environment.

Use the following shell script to run it:
 * `go.sh`                - test run for development (default ns-3 queue discipline)
 * `goFifo.sh`            - test run using FIFO queue discipline
 * `goFqCoDel.sh`         - test run using FqCoDel queue discipline
 * `goABBCoDel.sh`        - test run using ABBCoDel queue discipline

Example Run:

```
% ./go.sh                                   
Simulation parameters
simTime = 10
heartBeatOn = 0
pingOn = 0
tracing = 0
nFTPs = 1
nLanNodes = 3
qdType = 
qdQSize = 1000
devQType = 
devQSize = 1000
holdTime = 5
Total Bytes Received: 107187768
Throughput (Mbps): 85.7502

% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21    88.95    10.0   89.01    10.0   0.00604   0.00002    0.000
Total TxRate =  88.95 RxRate =  89.01 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
2    US  TCP  192.168.1.2:21   172.16.1.2:49153   1.58    10.0    1.58    10.0   0.00601   0.00000    0.000
Total TxRate =   1.58 RxRate =   1.58 Mbps

```

# Results from go.sh (with default queue discipline)

All simulation configuration parameters are default ones:

```C++
    //Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpHighSpeed"));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1436));       // default 536
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(131072));      // default 131072
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(131072));      // default 131072
    Config::SetDefault("ns3::BulkSendApplication::SendSize", UintegerValue(1412));    // default 512
```

The propagation delay for the CSMA net is set to:

```C++
    CsmaHelper csmaHelper;
    csmaHelper.SetChannelAttribute("DataRate", StringValue("1Gbps"));
    csmaHelper.SetChannelAttribute("Delay", TimeValue(NanoSeconds(656)));
```

Simulation run result:

```
% ./go.sh                                   
Simulation parameters
simTime = 10
heartBeatOn = 0
pingOn = 0
tracing = 0
nFTPs = 1
nLanNodes = 3
qdType = 
qdQSize = 1000
devQType = 
devQSize = 100
holdTime = 5
Total Bytes Received: 99900412
Throughput (Mbps): 79.9203

% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21    82.97    10.0   83.01    10.0   0.00633   0.00002    0.000
Total TxRate =  82.97 RxRate =  83.01 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
2    US  TCP  192.168.1.2:21   172.16.1.2:49153   1.47    10.0    1.47    10.0   0.00660   0.00006    0.000
Total TxRate =   1.47 RxRate =   1.47 Mbps

```

After increasing TCP buffer size by 10:

```C++
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(1310720));      // default 131072
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1310720));      // default 131072
```

We have:

```
% ./go.sh
Simulation parameters
simTime = 10
heartBeatOn = 0
pingOn = 0
tracing = 0
nFTPs = 1
nLanNodes = 3
qdType = 
qdQSize = 1000
devQType = 
devQSize = 100
holdTime = 5
Total Bytes Received: 553341620
Throughput (Mbps): 442.673

% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21   458.98    10.0  459.52    10.0   0.01125   0.00002    0.000
Total TxRate = 458.98 RxRate = 459.52 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
2    US  TCP  192.168.1.2:21   172.16.1.2:49153   8.16    10.0    8.16    10.0   0.01220   0.00005    0.000
Total TxRate =   8.16 RxRate =   8.16 Mbps

```

This setting achieves __an efficiency of 46%__ of the total available bandwidth. 
Temporarily increasing the queue discipline queue size to 10000 and device queue size to 1000 
doesn't help improve the flow throughput.

Running more than flows improve the efficiency due to statistical multiplexing.  Below is 
the result of running two flows:

```
% ./go.sh                                   
Simulation parameters
simTime = 10
heartBeatOn = 0
pingOn = 0
tracing = 0
nFTPs = 2
nLanNodes = 3
qdType = 
qdQSize = 1000
devQType = 
devQSize = 100
holdTime = 5
Total Bytes Received: 382472676
Throughput (Mbps): 305.978
[M1] 23-05-17 11:02 ~/workspace/NS3/ns-allinone-3.38/ns-3.38/scratch/210FTPCalib
% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21   315.91    10.0  315.25    10.0   0.01611   0.00004    0.000
2    DS  TCP  172.16.2.2:49153 192.168.1.3:21   318.14    10.0  317.85    10.0   0.01614   0.00004    0.000
Total TxRate = 634.05 RxRate = 633.10 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
3    US  TCP  192.168.1.2:21   172.16.1.2:49153   5.60    10.0    5.60    10.0   0.01802   0.00010    0.000
4    US  TCP  192.168.1.3:21   172.16.2.2:49153   5.64    10.0    5.64    10.0   0.01774   0.00009    0.000
Total TxRate =  11.24 RxRate =  11.25 Mbps

```

That increases efficiency from 46% to 63%.

The following is the result from running 3 flows:

```
% ./go.sh                                   
Simulation parameters
simTime = 10
heartBeatOn = 0
pingOn = 0
tracing = 0
nFTPs = 3
nLanNodes = 3
qdType = 
qdQSize = 1000
devQType = 
devQSize = 100
holdTime = 5
Total Bytes Received: 288545048
Throughput (Mbps): 230.836
[M1] 23-05-17 11:08 ~/workspace/NS3/ns-allinone-3.38/ns-3.38/scratch/210FTPCalib
% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21   245.18    10.0  245.33    10.0   0.02078   0.00007    0.000
2    DS  TCP  172.16.2.2:49153 192.168.1.3:21   241.57    10.0  241.79    10.0   0.02108   0.00007    0.000
3    DS  TCP  172.16.3.2:49153 192.168.1.4:21   239.71    10.0  239.48    10.0   0.02123   0.00007    0.000
Total TxRate = 726.46 RxRate = 726.60 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
6    US  TCP  192.168.1.2:21   172.16.1.2:49153   4.36    10.0    4.35    10.0   0.02315   0.00014    0.000
5    US  TCP  192.168.1.3:21   172.16.2.2:49153   4.29    10.0    4.29     9.9   0.02313   0.00015    0.000
4    US  TCP  192.168.1.4:21   172.16.3.2:49153   4.25    10.0    4.24    10.0   0.02374   0.00015    0.000
Total TxRate =  12.90 RxRate =  12.89 Mbps

```

The efficiency further increases.

# Results from goFifo.sh

All simulation configuration parameters are default ones:

```C++
    //Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpHighSpeed"));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1436));       // default 536
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(131072));      // default 131072
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(131072));      // default 131072
    Config::SetDefault("ns3::BulkSendApplication::SendSize", UintegerValue(1412));    // default 512

    // Turn off ECN for now
    Config::SetDefault ("ns3::CoDelQueueDisc::UseEcn", BooleanValue (false));
    Config::SetDefault ("ns3::FqCoDelQueueDisc::UseEcn", BooleanValue (false));
    Config::SetDefault ("ns3::TcpSocketBase::UseEcn", StringValue ("Off"));
```

The propagation delay for the CSMA net is set to:

```C++
    CsmaHelper csmaHelper;
    csmaHelper.SetChannelAttribute("DataRate", StringValue("1Gbps"));
    csmaHelper.SetChannelAttribute("Delay", TimeValue(NanoSeconds(656)));
```

Simulation run result:

```
% ./goFifo.sh
Simulation parameters
simTime = 10
heartBeatOn = 0
pingOn = 0
tracing = 0
nFTPs = 1
nLanNodes = 3
qdType = Fifo
qdQSize = 2000
devQType = 
devQSize = 200
holdTime = 1
Total Bytes Received: 99900412
Throughput (Mbps): 79.9203

% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21    82.97    10.0   83.01    10.0   0.00633   0.00002    0.000
Total TxRate =  82.97 RxRate =  83.01 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
2    US  TCP  192.168.1.2:21   172.16.1.2:49153   1.47    10.0    1.47    10.0   0.00660   0.00006    0.000
Total TxRate =   1.47 RxRate =   1.47 Mbps

```

After increasing TCP buffer size by 10:

```C++
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(1310720));      // default 131072
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1310720));      // default 131072
```

We have:

```
% ./goFifo.sh
Simulation parameters
simTime = 10
heartBeatOn = 0
pingOn = 0
tracing = 0
nFTPs = 1
nLanNodes = 3
qdType = Fifo
qdQSize = 2000
devQType = 
devQSize = 200
holdTime = 1
Total Bytes Received: 553341620
Throughput (Mbps): 442.673

% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21   458.98    10.0  459.52    10.0   0.01125   0.00002    0.000
Total TxRate = 458.98 RxRate = 459.52 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
2    US  TCP  192.168.1.2:21   172.16.1.2:49153   8.16    10.0    8.16    10.0   0.01220   0.00005    0.000
Total TxRate =   8.16 RxRate =   8.16 Mbps

```

This setting achieves __an efficiency of 46%__ of the total available bandwidth. 
Temporarily increasing the queue discipline queue size to 10000 and device queue size to 1000 
doesn't help improve the flow throughput.

Running more flows improve the efficiency due to statistical multiplexing. Below is the result of running three flows:

```
% ./goFifo.sh
Simulation parameters
simTime = 10
heartBeatOn = 0
pingOn = 0
tracing = 0
nFTPs = 3
nLanNodes = 3
qdType = Fifo
qdQSize = 2000
devQType = 
devQSize = 200
holdTime = 1
Total Bytes Received: 349957140
Throughput (Mbps): 279.966

% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21   238.96    10.0  238.28    10.0   0.01780   0.00004    0.174
2    DS  TCP  172.16.2.2:49153 192.168.1.3:21   144.92    10.0  143.50    10.0   0.01622   0.00006    0.848
3    DS  TCP  172.16.3.2:49153 192.168.1.4:21   292.05    10.0  291.30    10.0   0.01759   0.00004    0.132
Total TxRate = 675.94 RxRate = 673.07 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
6    US  TCP  192.168.1.2:21   172.16.1.2:49153   4.34    10.0    4.34    10.0   0.01982   0.00010    0.000
5    US  TCP  192.168.1.3:21   172.16.2.2:49153   2.66    10.0    2.67    10.0   0.01795   0.00015    0.000
4    US  TCP  192.168.1.4:21   172.16.3.2:49153   5.25    10.0    5.25    10.0   0.01923   0.00009    0.000
Total TxRate =  12.25 RxRate =  12.25 Mbps
```

Increasing the hold time to 5 milliseconds, the result is shown below:

```
% ./goFifo.sh                               
Simulation parameters
simTime = 10
heartBeatOn = 0
pingOn = 0
tracing = 0
nFTPs = 3
nLanNodes = 3
qdType = Fifo
qdQSize = 2000
devQType = 
devQSize = 200
holdTime = 5
Total Bytes Received: 291946164
Throughput (Mbps): 233.557

% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21   212.90    10.0  212.12    10.0   0.01708   0.00005    0.240
2    DS  TCP  172.16.2.2:49153 192.168.1.3:21   214.75    10.0  213.40    10.0   0.01758   0.00005    0.358
3    DS  TCP  172.16.3.2:49153 192.168.1.4:21   245.99    10.0  244.26    10.0   0.01779   0.00004    0.461
Total TxRate = 673.65 RxRate = 669.78 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
6    US  TCP  192.168.1.2:21   172.16.1.2:49153   3.85    10.0    3.84    10.0   0.02035   0.00011    0.000
5    US  TCP  192.168.1.3:21   172.16.2.2:49153   3.86    10.0    3.86    10.0   0.01901   0.00011    0.000
4    US  TCP  192.168.1.4:21   172.16.3.2:49153   4.45    10.0    4.45    10.0   0.01937   0.00010    0.000
Total TxRate =  12.16 RxRate =  12.16 Mbps

```

The above hold time of 5ms requires more device queue space to realize, I suppose. 
Below result is from increasing device queue from 200 packets to 500 packets:

```
% ./goFifo.sh                               
Simulation parameters
simTime = 10
heartBeatOn = 0
pingOn = 0
tracing = 0
nFTPs = 3
nLanNodes = 3
qdType = Fifo
qdQSize = 2000
devQType = 
devQSize = 500
holdTime = 5
Total Bytes Received: 338959072
Throughput (Mbps): 271.167
[M1] 23-05-19 14:11 ~/workspace/NS3/ns-allinone-3.38/ns-3.38/scratch/210FTPCalib
% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21   162.82    10.0  162.49    10.0   0.01696   0.00006    0.389
2    DS  TCP  172.16.2.2:49153 192.168.1.3:21   231.18    10.0  230.06    10.0   0.01751   0.00005    0.399
3    DS  TCP  172.16.3.2:49153 192.168.1.4:21   283.56    10.0  282.91    10.0   0.01818   0.00004    0.124
Total TxRate = 677.57 RxRate = 675.46 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
6    US  TCP  192.168.1.2:21   172.16.1.2:49153   2.98    10.0    2.97    10.0   0.01933   0.00015    0.000
5    US  TCP  192.168.1.3:21   172.16.2.2:49153   4.10    10.0    4.10    10.0   0.01950   0.00011    0.000
4    US  TCP  192.168.1.4:21   172.16.3.2:49153   5.14    10.0    5.13    10.0   0.01967   0.00009    0.000
Total TxRate =  12.22 RxRate =  12.20 Mbps

```

Increasing the flows to 5, here is the result:

```
% ./goFifo.sh                               
Simulation parameters
simTime = 10
heartBeatOn = 0
pingOn = 0
tracing = 0
nFTPs = 5
nLanNodes = 5
qdType = Fifo
qdQSize = 2000
devQType = 
devQSize = 500
holdTime = 5
Total Bytes Received: 189346196
Throughput (Mbps): 151.477

% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21   108.82    10.0  107.84    10.0   0.01716   0.00008    0.806
2    DS  TCP  172.16.2.2:49153 192.168.1.3:21   186.16    10.0  184.55    10.0   0.01769   0.00005    0.478
3    DS  TCP  172.16.3.2:49153 192.168.1.4:21   153.96    10.0  152.85    10.0   0.01724   0.00006    0.560
4    DS  TCP  172.16.4.2:49153 192.168.1.5:21    95.56    10.0   94.99    10.0   0.01664   0.00009    0.228
5    DS  TCP  172.16.5.2:49153 192.168.1.6:21   158.08    10.0  157.73    10.0   0.01672   0.00006    0.176
Total TxRate = 702.57 RxRate = 697.96 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
7    US  TCP  192.168.1.2:21   172.16.1.2:49153   2.04    10.0    2.04    10.0   0.01899   0.00019    0.000
8    US  TCP  192.168.1.3:21   172.16.2.2:49153   3.38    10.0    3.39    10.0   0.01993   0.00012    0.000
6    US  TCP  192.168.1.4:21   172.16.3.2:49153   2.81    10.0    2.81    10.0   0.01927   0.00015    0.000
10   US  TCP  192.168.1.5:21   172.16.4.2:49153   1.71    10.0    1.71    10.0   0.01813   0.00023    0.000
9    US  TCP  192.168.1.6:21   172.16.5.2:49153   2.82    10.0    2.82    10.0   0.02001   0.00015    0.000
Total TxRate =  12.76 RxRate =  12.77 Mbps

```

Reducing the device queue size to 200 and hold time to 1ms, the result is below:

```
% ./goFifo.sh                               
Simulation parameters
simTime = 10
heartBeatOn = 0
pingOn = 0
tracing = 0
nFTPs = 5
nLanNodes = 5
qdType = Fifo
qdQSize = 2000
devQType = 
devQSize = 200
holdTime = 1
Total Bytes Received: 59368548
Throughput (Mbps): 47.4948
[M1] 23-05-19 14:20 ~/workspace/NS3/ns-allinone-3.38/ns-3.38/scratch/210FTPCalib
% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21   188.75    10.0  188.21    10.0   0.01814   0.00005    0.375
2    DS  TCP  172.16.2.2:49153 192.168.1.3:21   137.04    10.0  136.33    10.0   0.01630   0.00007    0.471
3    DS  TCP  172.16.3.2:49153 192.168.1.4:21   191.56    10.0  190.28    10.0   0.01665   0.00005    0.451
4    DS  TCP  172.16.4.2:49153 192.168.1.5:21   128.95    10.0  128.45    10.0   0.01568   0.00007    0.302
5    DS  TCP  172.16.5.2:49153 192.168.1.6:21    50.01    10.0   49.33    10.0   0.01557   0.00014    1.270
Total TxRate = 696.31 RxRate = 692.60 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
7    US  TCP  192.168.1.2:21   172.16.1.2:49153   3.58    10.0    3.56    10.0   0.01939   0.00011    0.000
8    US  TCP  192.168.1.3:21   172.16.2.2:49153   2.54    10.0    2.55    10.0   0.01727   0.00016    0.000
6    US  TCP  192.168.1.4:21   172.16.3.2:49153   3.49    10.0    3.49    10.0   0.01987   0.00012    0.000
10   US  TCP  192.168.1.5:21   172.16.4.2:49153   2.31    10.0    2.31     9.9   0.01842   0.00018    0.000
9    US  TCP  192.168.1.6:21   172.16.5.2:49153   0.94    10.0    0.94     9.9   0.01688   0.00034    0.000
Total TxRate =  12.85 RxRate =  12.85 Mbps

```

# Results from goCoDel.sh

All simulation configuration parameters are default ones:

```C++
    //Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpHighSpeed"));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1436));       // default 536
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(131072));      // default 131072
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(131072));      // default 131072
    Config::SetDefault("ns3::BulkSendApplication::SendSize", UintegerValue(1412));    // default 512

    // Turn off ECN for now
    Config::SetDefault ("ns3::CoDelQueueDisc::UseEcn", BooleanValue (false));
    Config::SetDefault ("ns3::FqCoDelQueueDisc::UseEcn", BooleanValue (false));
    Config::SetDefault ("ns3::TcpSocketBase::UseEcn", StringValue ("Off"));
```

The propagation delay for the CSMA net is set to:

```C++
    CsmaHelper csmaHelper;
    csmaHelper.SetChannelAttribute("DataRate", StringValue("1Gbps"));
    csmaHelper.SetChannelAttribute("Delay", TimeValue(NanoSeconds(656)));
```

Simulation run result:

```
% ./goCoDel.sh 
Simulation parameters
simTime = 10
heartBeatOn = 0
pingOn = 0
tracing = 0
nFTPs = 1
nLanNodes = 3
qdType = CoDel
qdQSize = 2000
devQType = 
devQSize = 200
holdTime = 1
Total Bytes Received: 99900412
Throughput (Mbps): 79.9203

% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21    82.97    10.0   83.01    10.0   0.00633   0.00002    0.000
Total TxRate =  82.97 RxRate =  83.01 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
2    US  TCP  192.168.1.2:21   172.16.1.2:49153   1.47    10.0    1.47    10.0   0.00660   0.00006    0.000
Total TxRate =   1.47 RxRate =   1.47 Mbps

```

The above result is identical to that of Fifo. ns-3 has changed the default queue discipline 
for all to FqCoDel. Changing the queue discipline at the CSMA router won't affect the 
result that much.

After increasing TCP buffer size by 10:

```C++
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(1310720));      // default 131072
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1310720));      // default 131072
```

We have:

```
% ./goCoDel.sh
Simulation parameters
simTime = 10
heartBeatOn = 0
pingOn = 0
tracing = 0
nFTPs = 1
nLanNodes = 3
qdType = CoDel
qdQSize = 2000
devQType = 
devQSize = 200
holdTime = 1
Total Bytes Received: 553341620
Throughput (Mbps): 442.673

% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21   458.98    10.0  459.52    10.0   0.01125   0.00002    0.000
Total TxRate = 458.98 RxRate = 459.52 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
2    US  TCP  192.168.1.2:21   172.16.1.2:49153   8.16    10.0    8.16    10.0   0.01220   0.00005    0.000
Total TxRate =   8.16 RxRate =   8.16 Mbps

```

This setting achieves __an efficiency of 46%__ of the total available bandwidth. 
Temporarily increasing the queue discipline queue size to 10000 and device queue size to 1000 
doesn't help improve the flow throughput.

Running more flows improve the efficiency due to statistical multiplexing. Below is the result of running three flows:

```
% ./goCoDel.sh                              
Simulation parameters
simTime = 10
heartBeatOn = 0
pingOn = 0
tracing = 0
nFTPs = 3
nLanNodes = 3
qdType = CoDel
qdQSize = 2000
devQType = 
devQSize = 200
holdTime = 1
Total Bytes Received: 349957140
Throughput (Mbps): 279.966

% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21   238.96    10.0  238.28    10.0   0.01780   0.00004    0.174
2    DS  TCP  172.16.2.2:49153 192.168.1.3:21   144.92    10.0  143.50    10.0   0.01622   0.00006    0.848
3    DS  TCP  172.16.3.2:49153 192.168.1.4:21   292.05    10.0  291.30    10.0   0.01759   0.00004    0.132
Total TxRate = 675.94 RxRate = 673.07 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
6    US  TCP  192.168.1.2:21   172.16.1.2:49153   4.34    10.0    4.34    10.0   0.01982   0.00010    0.000
5    US  TCP  192.168.1.3:21   172.16.2.2:49153   2.66    10.0    2.67    10.0   0.01795   0.00015    0.000
4    US  TCP  192.168.1.4:21   172.16.3.2:49153   5.25    10.0    5.25    10.0   0.01923   0.00009    0.000
Total TxRate =  12.25 RxRate =  12.25 Mbps

```

Increasing the flows to 5, the result is below:

```
% ./goCoDel.sh                              
Simulation parameters
simTime = 10
heartBeatOn = 0
pingOn = 0
tracing = 0
nFTPs = 5
nLanNodes = 5
qdType = CoDel
qdQSize = 2000
devQType = 
devQSize = 200
holdTime = 1
Total Bytes Received: 59368548
Throughput (Mbps): 47.4948

% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21   188.75    10.0  188.21    10.0   0.01814   0.00005    0.375
2    DS  TCP  172.16.2.2:49153 192.168.1.3:21   137.04    10.0  136.33    10.0   0.01630   0.00007    0.471
3    DS  TCP  172.16.3.2:49153 192.168.1.4:21   191.56    10.0  190.28    10.0   0.01665   0.00005    0.451
4    DS  TCP  172.16.4.2:49153 192.168.1.5:21   128.95    10.0  128.45    10.0   0.01568   0.00007    0.302
5    DS  TCP  172.16.5.2:49153 192.168.1.6:21    50.01    10.0   49.33    10.0   0.01557   0.00014    1.270
Total TxRate = 696.31 RxRate = 692.60 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
7    US  TCP  192.168.1.2:21   172.16.1.2:49153   3.58    10.0    3.56    10.0   0.01939   0.00011    0.000
8    US  TCP  192.168.1.3:21   172.16.2.2:49153   2.54    10.0    2.55    10.0   0.01727   0.00016    0.000
6    US  TCP  192.168.1.4:21   172.16.3.2:49153   3.49    10.0    3.49    10.0   0.01987   0.00012    0.000
10   US  TCP  192.168.1.5:21   172.16.4.2:49153   2.31    10.0    2.31     9.9   0.01842   0.00018    0.000
9    US  TCP  192.168.1.6:21   172.16.5.2:49153   0.94    10.0    0.94     9.9   0.01688   0.00034    0.000
Total TxRate =  12.85 RxRate =  12.85 Mbps

```

# Results from goFqCoDel.sh

Simulation configuration parameters set to:

```C++
    //Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpHighSpeed"));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1436));       // default 536
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(1310720));      // default 131072
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1310720));      // default 131072
    Config::SetDefault("ns3::BulkSendApplication::SendSize", UintegerValue(1412));    // default 512

    // Turn off ECN for now
    Config::SetDefault ("ns3::CoDelQueueDisc::UseEcn", BooleanValue (false));
    Config::SetDefault ("ns3::FqCoDelQueueDisc::UseEcn", BooleanValue (false));
    Config::SetDefault ("ns3::TcpSocketBase::UseEcn", StringValue ("Off"));
```

The propagation delay for the CSMA net is set to:

```C++
    CsmaHelper csmaHelper;
    csmaHelper.SetChannelAttribute("DataRate", StringValue("1Gbps"));
    csmaHelper.SetChannelAttribute("Delay", TimeValue(NanoSeconds(656)));
```

Simulation run result:

```
% ./goFqCoDel.sh
Simulation parameters
simTime = 10
heartBeatOn = 0
pingOn = 0
tracing = 0
nFTPs = 5
nLanNodes = 5
qdType = FqCoDel
qdQSize = 2000
devQType = 
devQSize = 200
holdTime = 1
Total Bytes Received: 174169508
Throughput (Mbps): 139.336

% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21   145.47    10.0  145.29    10.0   0.01777   0.00011    0.131
2    DS  TCP  172.16.2.2:49153 192.168.1.3:21   132.80    10.0  132.53    10.0   0.01545   0.00011    0.158
3    DS  TCP  172.16.3.2:49153 192.168.1.4:21   135.71    10.0  135.18    10.0   0.01622   0.00011    0.183
4    DS  TCP  172.16.4.2:49153 192.168.1.5:21   156.60    10.0  156.78    10.0   0.01816   0.00010    0.061
5    DS  TCP  172.16.5.2:49153 192.168.1.6:21   145.16    10.0  144.73    10.0   0.01671   0.00011    0.158
Total TxRate = 715.75 RxRate = 714.51 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
7    US  TCP  192.168.1.2:21   172.16.1.2:49153   2.59    10.0    2.59    10.0   0.01932   0.00023    0.000
8    US  TCP  192.168.1.3:21   172.16.2.2:49153   2.35    10.0    2.35    10.0   0.01922   0.00025    0.000
6    US  TCP  192.168.1.4:21   172.16.3.2:49153   2.41    10.0    2.41     9.9   0.01919   0.00025    0.000
10   US  TCP  192.168.1.5:21   172.16.4.2:49153   2.79    10.0    2.79    10.0   0.01825   0.00022    0.000
9    US  TCP  192.168.1.6:21   172.16.5.2:49153   2.57    10.0    2.57    10.0   0.01845   0.00023    0.000
Total TxRate =  12.72 RxRate =  12.71 Mbps

```

Increasing the simulation time to 100s, the result is as follows:

```
% NS_LOG=210FTPCalib ./goFqCoDel.sh
210FTPCalib:main(): [INFO ] 210FTPCalib starts ...
Simulation parameters
simTime = 100s
heartBeatOn = 1
pingOn = 0
tracing = 0
nFTPs = 5
nLanNodes = 5
qdType = FqCoDel
qdQSize = 2000p
devQType = 
devQSize = 200p
holdTime = 1ms
+1.000000000s -1 210FTPCalib:heartbeat(): [INFO ] Heart Beat: +1e+09ns
+2.000000000s -1 210FTPCalib:heartbeat(): [INFO ] Heart Beat: +2e+09ns
+3.000000000s -1 210FTPCalib:heartbeat(): [INFO ] Heart Beat: +3e+09ns
... ...
+99.000000000s -1 210FTPCalib:heartbeat(): [INFO ] Heart Beat: +9.9e+10ns
+100.000000000s -1 210FTPCalib:heartbeat(): [INFO ] Heart Beat: +1e+11ns
Total Bytes Received: 1747158164
Throughput (Mbps): 139.773
210FTPCalib:main(): [INFO ] 210FTPCalib ends now.

% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21   145.48   100.0  145.37   100.0   0.01694   0.00011    0.034
2    DS  TCP  172.16.2.2:49153 192.168.1.3:21   139.76   100.0  139.70   100.0   0.01639   0.00011    0.034
3    DS  TCP  172.16.3.2:49153 192.168.1.4:21   141.84   100.0  141.76   100.0   0.01669   0.00011    0.038
4    DS  TCP  172.16.4.2:49153 192.168.1.5:21   147.61   100.0  147.55   100.0   0.01720   0.00011    0.031
5    DS  TCP  172.16.5.2:49153 192.168.1.6:21   144.95   100.0  144.86   100.0   0.01690   0.00011    0.035
Total TxRate = 719.64 RxRate = 719.25 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
7    US  TCP  192.168.1.2:21   172.16.1.2:49153   2.56   100.0    2.56   100.0   0.01884   0.00023    0.000
8    US  TCP  192.168.1.3:21   172.16.2.2:49153   2.46   100.0    2.46   100.0   0.01887   0.00024    0.000
6    US  TCP  192.168.1.4:21   172.16.3.2:49153   2.50   100.0    2.50   100.0   0.01892   0.00024    0.000
10   US  TCP  192.168.1.5:21   172.16.4.2:49153   2.60   100.0    2.60   100.0   0.01852   0.00023    0.000
9    US  TCP  192.168.1.6:21   172.16.5.2:49153   2.55   100.0    2.55   100.0   0.01870   0.00024    0.000
Total TxRate =  12.67 RxRate =  12.67 Mbps

```

# Results from goABBCoDel.sh

Simulation configuration parameters set to:

```C++
    //Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpHighSpeed"));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1436));       // default 536
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(1310720));      // default 131072
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1310720));      // default 131072
    Config::SetDefault("ns3::BulkSendApplication::SendSize", UintegerValue(1412));    // default 512

    // Turn off ECN for now
    Config::SetDefault ("ns3::CoDelQueueDisc::UseEcn", BooleanValue (false));
    Config::SetDefault ("ns3::FqCoDelQueueDisc::UseEcn", BooleanValue (false));
    Config::SetDefault ("ns3::TcpSocketBase::UseEcn", StringValue ("Off"));
```

The propagation delay for the CSMA net is set to:

```C++
    CsmaHelper csmaHelper;
    csmaHelper.SetChannelAttribute("DataRate", StringValue("1Gbps"));
    csmaHelper.SetChannelAttribute("Delay", TimeValue(NanoSeconds(656)));
```

Simulation run result:

```
% NS_LOG=210FTPCalib=info ./goABBCoDel.sh
210FTPCalib starts ...
Simulation parameters
simTime = 10s
heartBeatOn = 1
pingOn = 0
tracing = 0
nFTPs = 5
nLanNodes = 5
qdType = ABBCoDel
qdQSize = 2000p
devQType = 
devQSize = 200p
holdTime = 1ms
flowTable = abb-flow-table.csv
Finished loading flow table
Heart Beat: +1e+09ns
Heart Beat: +2e+09ns
... ...
Heart Beat: +9e+09ns
Heart Beat: +1e+10ns
Total Bytes Received: 79359104
Throughput (Mbps): 63.4873
210FTPCalib ends now.

% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21   174.75    10.0  174.57    10.0   0.01813   0.00013    0.153
2    DS  TCP  172.16.2.2:49153 192.168.1.3:21   218.35    10.0  218.11    10.0   0.02083   0.00008    0.206
3    DS  TCP  172.16.3.2:49153 192.168.1.4:21   158.85    10.0  158.02    10.0   0.01658   0.00006    0.717
4    DS  TCP  172.16.4.2:49153 192.168.1.5:21    91.49    10.0   91.04    10.0   0.01130   0.00007    0.457
5    DS  TCP  172.16.5.2:49153 192.168.1.6:21    66.48    10.0   65.93    10.0   0.01097   0.00008    0.751
Total TxRate = 709.93 RxRate = 707.66 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
7    US  TCP  192.168.1.2:21   172.16.1.2:49153   3.17    10.0    3.16    10.0   0.01941   0.00017    0.000
8    US  TCP  192.168.1.3:21   172.16.2.2:49153   3.89    10.0    3.89    10.0   0.01704   0.00015    0.000
6    US  TCP  192.168.1.4:21   172.16.3.2:49153   2.87    10.0    2.86    10.0   0.02000   0.00015    0.000
10   US  TCP  192.168.1.5:21   172.16.4.2:49153   1.66    10.0    1.66    10.0   0.02099   0.00029    0.000
9    US  TCP  192.168.1.6:21   172.16.5.2:49153   1.22    10.0    1.22    10.0   0.01943   0.00035    0.000
Total TxRate =  12.81 RxRate =  12.78 Mbps

```

Increasing the simulation time to 100s, the result is as follows:

```
% NS_LOG=210FTPCalib=info ./goABBCoDel.sh   
210FTPCalib starts ...
Simulation parameters
simTime = 100s
heartBeatOn = 1
pingOn = 0
tracing = 0
nFTPs = 5
nLanNodes = 5
qdType = ABBCoDel
qdQSize = 2000p
devQType = 
devQSize = 200p
holdTime = 1ms
flowTable = abb-flow-table.csv
Finished loading flow table
Heart Beat: +1e+09ns
Heart Beat: +2e+09ns
... ...
Heart Beat: +9.9e+10ns
Heart Beat: +1e+11ns
Total Bytes Received: 1639281688
Throughput (Mbps): 131.143
210FTPCalib ends now.
[M1] 23-06-20 15:59 ~/workspace/NS3/ns-allinone-3.38/ns-3.38/scratch/210FTPCalib
% ./getflowinfo.py out/results.xml 192.168.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  172.16.1.2:49153 192.168.1.2:21   127.62   100.0  127.54   100.0   0.01619   0.00010    0.048
2    DS  TCP  172.16.2.2:49153 192.168.1.3:21   176.04   100.0  175.97   100.0   0.01919   0.00009    0.040
3    DS  TCP  172.16.3.2:49153 192.168.1.4:21   160.91   100.0  160.74   100.0   0.01764   0.00009    0.082
4    DS  TCP  172.16.4.2:49153 192.168.1.5:21   109.89   100.0  109.83   100.0   0.01307   0.00008    0.073
5    DS  TCP  172.16.5.2:49153 192.168.1.6:21   136.07   100.0  136.03   100.0   0.01582   0.00010    0.049
Total TxRate = 710.53 RxRate = 710.11 Mbps

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
7    US  TCP  192.168.1.2:21   172.16.1.2:49153   2.26   100.0    2.26   100.0   0.01896   0.00022    0.000
8    US  TCP  192.168.1.3:21   172.16.2.2:49153   3.11   100.0    3.11   100.0   0.01755   0.00019    0.000
6    US  TCP  192.168.1.4:21   172.16.3.2:49153   2.84   100.0    2.84   100.0   0.01858   0.00019    0.000
10   US  TCP  192.168.1.5:21   172.16.4.2:49153   1.95   100.0    1.94   100.0   0.02012   0.00024    0.000
9    US  TCP  192.168.1.6:21   172.16.5.2:49153   2.40   100.0    2.40   100.0   0.01907   0.00021    0.000
Total TxRate =  12.56 RxRate =  12.56 Mbps

```

(End)