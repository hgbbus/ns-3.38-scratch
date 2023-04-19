# 000Echo Script

This script is a simulation that starts an echo process between two nodes.

As seen in the `go.sh` shell script, by default, the shell script uses the command 
line option nFTPs to create five server nodes. As many as five local LAN nodes 
will then be automatically created by the C++ code. Besides, the shell script uses
the command line option nPackets to set the total number of echo packets to 3. 

In addition, the shell script sets the log level to __verbose__ and PCAP tracing is
turned on. 

The simulation C++ code, by default, runs an echo server on the last local LAN
node with an IP address 10.1.1.6.  Then, it runs an echo client on the first 
FTP node with an IP address 10.2.1.2. The following are the parameters used:

* Max number of echo packets default to 1 (but overwritten by shell script)
* Packet payload size is 1024 and the interval is set to 1.0 second.
* Echo server starts at 1.0 second and stops at 10.0 second.
* Echo client starts at 2.0 second and stops at 10.0 second.

Here is a sample run output (and the interpretation of the result is at the end):

```
% ./go.sh 
At time +2s client sent 1024 bytes to 10.1.1.6 port 9
At time +2.0082s server received 1024 bytes from 10.2.1.2 port 49153
At time +2.0082s server sent 1024 bytes to 10.2.1.2 port 49153
At time +2.02241s client received 1024 bytes from 10.1.1.6 port 9
At time +3s client sent 1024 bytes to 10.1.1.6 port 9
At time +3.00618s server received 1024 bytes from 10.2.1.2 port 49153
At time +3.00618s server sent 1024 bytes to 10.2.1.2 port 49153
At time +3.01236s client received 1024 bytes from 10.1.1.6 port 9
At time +4s client sent 1024 bytes to 10.1.1.6 port 9
At time +4.00618s server received 1024 bytes from 10.2.1.2 port 49153
At time +4.00618s server sent 1024 bytes to 10.2.1.2 port 49153
At time +4.01236s client received 1024 bytes from 10.1.1.6 port 9
```
As shown, three echo packets were sent and received in a time span from 
2-4 seconds.

(End)
