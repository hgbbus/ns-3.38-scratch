# 200FTP Script

This script starts a simulation of multiple FTP flows between a number of 
FTP server nodes and LAN nodes.

There are these many shell scripts to run simulations using different 
queue disciplines:
 * go.sh            - for development run
 * goCoDel.sh
 * goFifo.sh
 * goFqCoDel.sh
 * goPie.sh

There are two Python scripts to analyze simulation results:
 * flowinfo.py      - process XML result from FlowMonitor
 * getflowinfo.py   - a nicer version of the flowinfo.py

Example output:

```
% ./flowinfo.py out/results.xml          
Reading flow monitor result file in XML format:
Done reading XML file.

FlowID: 1 (TCP 10.2.1.2/49153 --> 10.1.1.2/21)
        TX bitrate: 1791.55 kbit/s
        RX bitrate: 1780.25 kbit/s
        Mean Delay: 61.28 ms
        Packet Loss Ratio: 0.32 %
FlowID: 2 (TCP 10.2.2.2/49153 --> 10.1.1.3/21)
        TX bitrate: 1948.52 kbit/s
        RX bitrate: 1934.47 kbit/s
        Mean Delay: 60.77 ms
        Packet Loss Ratio: 0.27 %
FlowID: 3 (TCP 10.2.3.2/49153 --> 10.1.1.4/21)
        TX bitrate: 1697.25 kbit/s
        RX bitrate: 1682.59 kbit/s
        Mean Delay: 62.63 ms
        Packet Loss Ratio: 0.39 %
FlowID: 4 (TCP 10.2.4.2/49153 --> 10.1.1.5/21)
        TX bitrate: 1513.09 kbit/s
        RX bitrate: 1330.96 kbit/s
        Mean Delay: 168.72 ms
        Packet Loss Ratio: 11.77 %
FlowID: 5 (TCP 10.2.5.2/49153 --> 10.1.1.6/21)
        TX bitrate: 1687.89 kbit/s
        RX bitrate: 1548.94 kbit/s
        Mean Delay: 123.42 ms
        Packet Loss Ratio: 7.89 %
FlowID: 6 (TCP 10.1.1.2/21 --> 10.2.1.2/49153)
        TX bitrate: 87.42 kbit/s
        RX bitrate: 87.45 kbit/s
        Mean Delay: 6.07 ms
        Packet Loss Ratio: 0.00 %
FlowID: 7 (TCP 10.1.1.4/21 --> 10.2.3.2/49153)
        TX bitrate: 89.98 kbit/s
        RX bitrate: 90.06 kbit/s
        Mean Delay: 6.07 ms
        Packet Loss Ratio: 0.00 %
FlowID: 8 (TCP 10.1.1.3/21 --> 10.2.2.2/49153)
        TX bitrate: 93.61 kbit/s
        RX bitrate: 93.65 kbit/s
        Mean Delay: 6.07 ms
        Packet Loss Ratio: 0.00 %
FlowID: 9 (TCP 10.1.1.5/21 --> 10.2.4.2/49153)
        TX bitrate: 155.33 kbit/s
        RX bitrate: 155.46 kbit/s
        Mean Delay: 6.08 ms
        Packet Loss Ratio: 0.00 %
FlowID: 10 (TCP 10.1.1.6/21 --> 10.2.5.2/49153)
        TX bitrate: 133.82 kbit/s
        RX bitrate: 133.93 kbit/s
        Mean Delay: 6.08 ms
        Packet Loss Ratio: 0.00 %
```
```
% ./getflowinfo.py out/results.xml 10.1.1
Reading flow monitor result input file in XML format:
1 simulation(s) parsed.

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
1    DS  TCP  10.2.1.2:49153   10.1.1.2:21        1.79    10.0    1.78    10.0   0.06128   0.00164    0.316
2    DS  TCP  10.2.2.2:49153   10.1.1.3:21        1.95    10.0    1.93    10.0   0.06077   0.00162    0.267
3    DS  TCP  10.2.3.2:49153   10.1.1.4:21        1.70    10.0    1.68    10.0   0.06263   0.00156    0.390
4    DS  TCP  10.2.4.2:49153   10.1.1.5:21        1.51     9.9    1.33     9.9   0.16872   0.00246   11.772
5    DS  TCP  10.2.5.2:49153   10.1.1.6:21        1.69    10.0    1.55    10.0   0.12342   0.00191    7.893
Total TxRate =   8.64 RxRate =   8.64

Flow Dir Type Src              Dest             TxRate TxPeriod RxRate RxPeriod DelayMean JitterMean LossRatio(%)
6    US  TCP  10.1.1.2:21      10.2.1.2:49153     0.09    10.0    0.09    10.0   0.00607   0.00000    0.000
8    US  TCP  10.1.1.3:21      10.2.2.2:49153     0.09    10.0    0.09    10.0   0.00607   0.00000    0.000
7    US  TCP  10.1.1.4:21      10.2.3.2:49153     0.09    10.0    0.09    10.0   0.00607   0.00000    0.000
9    US  TCP  10.1.1.5:21      10.2.4.2:49153     0.16     9.6    0.16     9.6   0.00608   0.00000    0.000
10   US  TCP  10.1.1.6:21      10.2.5.2:49153     0.13    10.0    0.13    10.0   0.00608   0.00000    0.000
Total TxRate =   0.56 RxRate =   0.56

```

