/*
 * Copyright (c) 2009 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mohamed Amine Ismail <amine.ismail@sophia.inria.fr>
 */

// Network topology
//
//       n0    n1
//       |     |
//       =======
//         LAN (CSMA)
//
// - UDP flow from n0 to n1 of 1024 byte packets at intervals of 50 ms
//   - maximum of 320 packets sent (or limited by simulation duration)
//   - option to use IPv4 or IPv6 addressing
//   - option to disable logging statements

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"

//#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("230TcpUdpCSMA");

int
main(int argc, char* argv[])
{
    // Declare variables used in command-line arguments
    bool useTcp = true;
    bool useV6 = false;
    bool logging = false;
    Address serverAddress;
    double startTime = 2.0;
    double endTime = 10.0;

    CommandLine cmd(__FILE__);
    cmd.AddValue("useTcp", "Use TCP", useTcp);
    cmd.AddValue("useIpv6", "Use Ipv6", useV6);
    cmd.AddValue("logging", "Enable logging", logging);
    cmd.Parse(argc, argv);

    if (logging)
    {
        LogComponentEnable("UdpClient", LOG_LEVEL_INFO);
        LogComponentEnable("UdpServer", LOG_LEVEL_INFO);
    }

if (useTcp)
{
    //Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpHighSpeed"));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1436));     // default 536
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(131072*2));   // default 131072
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(131072*2));   // default 131072
}
else
{
    //Config::SetDefault("ns3::UdpSocket::RcvBufSize", UintegerValue(1310720));   // default 131072
}

    NS_LOG_INFO("Create nodes in above topology.");
    NodeContainer n;
    n.Create(2);

    InternetStackHelper internet;
    internet.Install(n);

    NS_LOG_INFO("Create channel between the two nodes.");
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(100000000)));
    csma.SetChannelAttribute("Delay", TimeValue(MicroSeconds(50)));
    //csma.SetDeviceAttribute("Mtu", UintegerValue(1400));
    NetDeviceContainer d = csma.Install(n);

    NS_LOG_INFO("Assign IP Addresses.");
    Ipv4InterfaceContainer i;
    if (useV6 == false)
    {
        Ipv4AddressHelper ipv4;
        ipv4.SetBase("10.1.1.0", "255.255.255.0");
        i = ipv4.Assign(d);
        serverAddress = Address(i.GetAddress(1));
    }
    else
    {
        Ipv6AddressHelper ipv6;
        ipv6.SetBase("2001:0000:f00d:cafe::", Ipv6Prefix(64));
        Ipv6InterfaceContainer i6 = ipv6.Assign(d);
        serverAddress = Address(i6.GetAddress(1, 1));
    }

if (useTcp)
{
    //
    // Create a BulkSendApplication and install it on node 0
    //
    uint16_t port = 9; // well-known echo port number
    uint32_t maxBytes = 0;

    BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(i.GetAddress(1), port));
    // Set the amount of data to send in bytes.  Zero is unlimited.
    source.SetAttribute("MaxBytes", UintegerValue(maxBytes));
    ApplicationContainer sourceApps = source.Install(n.Get(0));
    sourceApps.Start(Seconds(startTime));
    sourceApps.Stop(Seconds(endTime));

    //
    // Create a PacketSinkApplication and install it on node 1
    //
    PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApps = sink.Install(n.Get(1));
    sinkApps.Start(Seconds(startTime - 1.0));
    sinkApps.Stop(Seconds(endTime));

    NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(Seconds(endTime+0.1));
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");

    Ptr<PacketSink> sink1 = DynamicCast<PacketSink>(sinkApps.Get(0));
    std::cout << "Total Bytes Received: " << sink1->GetTotalRx() << std::endl;
    std::cout << "Throughput (Mbps): " << sink1->GetTotalRx() * 8 / (endTime - startTime) / 1e6 << std::endl;
}
else
{
    NS_LOG_INFO("Create UdpServer application on node 1.");
    uint16_t port = 4000;
    UdpServerHelper server(port);
    ApplicationContainer serverapps = server.Install(n.Get(1));
    serverapps.Start(Seconds(startTime - 1.0));
    serverapps.Stop(Seconds(endTime));

    NS_LOG_INFO("Create UdpClient application on node 0 to send to node 1.");
    uint32_t MaxPacketSize = 1436;
    Time interPacketInterval = MicroSeconds(200);
    uint32_t maxPacketCount = 0;
    UdpClientHelper client(serverAddress, port);
    client.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
    client.SetAttribute("Interval", TimeValue(interPacketInterval));
    client.SetAttribute("PacketSize", UintegerValue(MaxPacketSize));
    ApplicationContainer clientapps = client.Install(n.Get(0));
    clientapps.Start(Seconds(startTime));
    clientapps.Stop(Seconds(endTime));

    NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(Seconds(endTime+0.1));
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");

    Ptr<UdpClient> cp = DynamicCast<UdpClient>(clientapps.Get(0));
    std::cout << "Total Tx: " << cp->GetTotalTx() << " bytes" << std::endl;
    std::cout << "Tx Rate: " << cp->GetTotalTx() * 8 / (endTime - startTime) / 1000000.0 << " Mbps" << std::endl;
    Ptr<UdpServer> sp = DynamicCast<UdpServer>(serverapps.Get(0));
    std::cout << "Received: " << sp->GetReceived() << std::endl;
    std::cout << "Rx Rate: " << sp->GetReceived() * MaxPacketSize * 8 / (endTime - startTime) / 1000000.0 << " Mbps" << std::endl;
}

    return 0;
}
