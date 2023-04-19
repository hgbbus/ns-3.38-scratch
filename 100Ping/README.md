# 100Ping Script

This script starts a simulation that pings from a local LAN node to 
a server node.

As seen in the `go.sh` shell script, the pinging is enabled through the command
line argument `pingOn`.  By default, the shell script uses the command line 
argument `nFTPs` to create five server nodes.  As many as five local LAN nodes 
will then be automatically created by the C++ code.

The simulation C++ code, by default, establishes a ping from the first local LAN 
node with the IP address `10.1.1.2` to the server code with the IP address `10.2.1.2`:

* The payload size is 1024 and the interval is set to 1.0 second.
* It starts at 1.0 second and stops at 101.0 second.

Here is a sample run output (and the interpretation of the result is too at
the end):
```
% ./go.sh
PING 10.2.1.2 - 1024 bytes of data - 1052 bytes including ICMP and IPv4 headers.
1032 bytes from 10.2.1.2: icmp_seq=0 ttl=61 time=+18.2401ms
1032 bytes from 10.2.1.2: icmp_seq=1 ttl=61 time=+12.1894ms
... ...
1032 bytes from 10.2.1.2: icmp_seq=98 ttl=61 time=+12.1894ms
1032 bytes from 10.2.1.2: icmp_seq=99 ttl=61 time=+12.1894ms
--- 10.2.1.2 ping statistics ---
100 packets transmitted, 100 received, 0% packet loss, time +1e+05ms
rtt min/avg/max/mdev = 12/12.06/18/0.6 ms
```

As shown, the average RTT is 12 ms.  If you look at the propogation 
delays on the path, it is roughly 2 ms + 2 ms + 6 ms (max) = 10 ms (max).
Note the 6 ms delay of the LAN is the max delay.  Nodes closer to
the router have shorter propogation delays (can be as short as 0).
Then, on a CSMA type of network, other types of protocol related 
delays such as sensing and backoff can be significant.

(End)
