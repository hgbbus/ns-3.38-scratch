#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

#define THIS_SCRIPT     "000Echo"

NS_LOG_COMPONENT_DEFINE(THIS_SCRIPT);

int
main(int argc, char* argv[])
{
  // default parameters (can be overwritten via command line arguments)
  bool verbose = false;
  bool pcapOn = false;
  uint32_t nPackets = 1;
  uint32_t nFTPs = 1;
  uint32_t nLanNodes = 3;

  // NS log is disabled by default (can be turned on via command line)
  NS_LOG_INFO(THIS_SCRIPT " starts ...");
  //printf(__FILE__ "\n");

  // associate command line options with default simulation parameters
  CommandLine cmd;
  //cmd.Usage("This simulation script supports the following command line options:");
  cmd.AddValue("verbose", "Tell echo application to log if true", verbose);
  cmd.AddValue("pcapOn", "Capture PCAP traces if true", pcapOn);
  cmd.AddValue("nPackets", "Number of packets to echo", nPackets);
  cmd.AddValue("nFTPs", "Number of FTP server nodes", nFTPs);
  cmd.AddValue("nLanNodes", "Number of LAN nodes on local net", nLanNodes);
  cmd.Parse(argc, argv);  // This causes the modification of simulation
                          // parameters via command line options

  // sanity check -- adjust nLanNodes to be at least nFTPs
  nLanNodes = (nLanNodes == 0) ? 1 : nLanNodes;
  if (nLanNodes < nFTPs) nLanNodes = nFTPs;

  // do you have other logs to enable?
  // NS3 defined a number of log components with NS_LOG_COMPONENT_DEFINE's
  // https://www.nsnam.org/docs/release/3.37/doxygen/d1/df4/_log_component_list.html

  // so this is where you say which of those you want to turn on.
  if (verbose) {
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
  }

  Time::SetResolution(Time::NS);	// it is the default time resolution

  //
  // Build nodes with devices and channels
  //

  NodeContainer coreNodes;              // core nodes
  coreNodes.Create(4);
  Ptr<Node> rtNode = coreNodes.Get(0);  // root node
  Ptr<Node> lrNode = coreNodes.Get(1);  // local net router node
  Ptr<Node> frNode = coreNodes.Get(2);  // ftp link router node
  Ptr<Node> orNode = coreNodes.Get(3);  // other link router node

  NodeContainer lanNodes;
  lanNodes.Add(lrNode);         // gateway node for the local net
  lanNodes.Create(nLanNodes);   // LAN nodes on the local net

  NodeContainer ftpNodes;       // running FTP servers?
  ftpNodes.Create(nFTPs);

  // P2P networks are used to connect those router nodes using the code below
  PointToPointHelper p2pHelper;   // p2p network connection (helper)
  p2pHelper.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
  p2pHelper.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer rlNodesP2PNetDevices;	// root to local net
  rlNodesP2PNetDevices = p2pHelper.Install(rtNode, lrNode);
  // at this moment, the two nodes are connected by an unnamed PointToPointChannel
  // attached to two PointToPointNetDevice's that are put in the device container

  // change p2p helper data rate for other two links if needed
  p2pHelper.SetDeviceAttribute("DataRate", StringValue("10Gbps"));

  NetDeviceContainer roNodesP2PNetDevices;	// root to other router
  roNodesP2PNetDevices = p2pHelper.Install(rtNode, orNode);

  NetDeviceContainer rfNodesP2PNetDevices;	// root to FTP router
  rfNodesP2PNetDevices = p2pHelper.Install(rtNode, frNode);

  // change p2p helper data rate for other non-trunk links
  //p2pHelper.SetDeviceAttribute("DataRate", StringValue("10Gbps"));

  // P2P star topology for FTP server nodes with FTP router as the hub
  std::vector<NetDeviceContainer> ftpNodesP2PNetDevicesAL(nFTPs);
  for (uint32_t i=0; i<nFTPs; i++) {
    ftpNodesP2PNetDevicesAL[i] = p2pHelper.Install(frNode, ftpNodes.Get(i));
  }

  // use CSMA for local net topology
  // local net set to CSMA
  CsmaHelper csmaHelper;
  csmaHelper.SetChannelAttribute("DataRate", StringValue("1Gbps"));
  csmaHelper.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));  // this is 6.56us
  // propagation delay is set to 20ns per foot for a 100 meter cable
  // this delay is low but CSMA delay (sensing, back-off, etc.) can be much higher
  NetDeviceContainer lanNodesNetDevices;
  lanNodesNetDevices = csmaHelper.Install(lanNodes);

  //
  // Configure them with Internet Stack and addresses
  // Set global routing for each node
  //

  InternetStackHelper inetStackHelper;
  inetStackHelper.Install(rtNode);
  inetStackHelper.Install(orNode);
  inetStackHelper.Install(frNode);
  inetStackHelper.Install(ftpNodes);
  inetStackHelper.Install(lanNodes);  // remember the lrNode is in lanNodes

  Ipv4AddressHelper ipv4AddrHelper;

  ipv4AddrHelper.SetBase("10.1.0.0", "255.255.0.0");
  Ipv4InterfaceContainer rlNodesInetInterfaces;		// root to local net
  rlNodesInetInterfaces = ipv4AddrHelper.Assign(rlNodesP2PNetDevices);

  ipv4AddrHelper.SetBase("10.3.0.0", "255.255.0.0");
  Ipv4InterfaceContainer roNodesInetInterfaces;		// root to other router
  roNodesInetInterfaces = ipv4AddrHelper.Assign(roNodesP2PNetDevices);

  ipv4AddrHelper.SetBase("10.2.0.0", "255.255.0.0");
  Ipv4InterfaceContainer rfNodesInetInterfaces;		// root to ftp router
  rfNodesInetInterfaces = ipv4AddrHelper.Assign(rfNodesP2PNetDevices);
    
  ipv4AddrHelper.SetBase("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer lanNodesCSMANetInterfaces;	// local net
  lanNodesCSMANetInterfaces = ipv4AddrHelper.Assign(lanNodesNetDevices);

  const char* ipv4NetAddrPattern = "172.16.%d.0";		// star topology for FTPs
  std::vector<Ipv4InterfaceContainer> ftpNodesInetInterfacesAL(nFTPs);
  for (int i=0; i<nFTPs; i++) {
    char ipv4NetAddr[16];
    snprintf(ipv4NetAddr, 16, ipv4NetAddrPattern, i+1);
    ipv4AddrHelper.SetBase(ipv4NetAddr, "255.255.255.0");
    ftpNodesInetInterfacesAL[i] = ipv4AddrHelper.Assign(ftpNodesP2PNetDevicesAL[i]);
  }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  //
  // Install apps on the nodes
  // Schedule applications events (start and stop events)
  //

  UdpEchoServerHelper echoServerHelper(9);	// port number 9
  //ApplicationContainer serverApps = echoServerHelper.Install(lanNodes.Get(1));
  ApplicationContainer serverApps = echoServerHelper.Install(lanNodes.Get(nLanNodes));
  serverApps.Start(Seconds(1.0));
  serverApps.Stop(Seconds(10.0));

  // pointing the client helper to a server (addr + port)
  UdpEchoClientHelper echoClientHelper(lanNodesCSMANetInterfaces.GetAddress(nLanNodes), 9);
  // Following sets AttributeValue's (TimeValue is an AttributeValue)
  echoClientHelper.SetAttribute("MaxPackets", UintegerValue(nPackets));
  echoClientHelper.SetAttribute("Interval", TimeValue(Seconds(1.0)));
  echoClientHelper.SetAttribute("PacketSize", UintegerValue(1024));
  //ApplicationContainer clientApps = echoClientHelper.Install(frNode);
  ApplicationContainer clientApps = echoClientHelper.Install(ftpNodes.Get(0));
  clientApps.Start(Seconds(2.0));
  clientApps.Stop(Seconds(10.0));

  //
  // PCAP tracing code
  //

  if (pcapOn) {
    p2pHelper.EnablePcapAll(THIS_SCRIPT "-p2p");	// two (both) P2P net devices
    //p2pHelper.EnablePcap(THIS_SCRIPT "p2p", lanNodes.Get(0)->GetId(), 0);
    csmaHelper.EnablePcap(THIS_SCRIPT "-csma", lanNodesNetDevices.Get(1), true);
    //csmaHelper.EnablePcap(THIS_SCRIPT "csma", lanNodes.Get(nLanNodes)->GetId(), 0, false);
    //csmaHelper.EnablePcap(THIS_SCRIPT "csma", lanNodes.Get(nLanNodes-1)->GetId(), 0, false);
    // for CSMA net, only one net device is set to 'promiscuous' to capture
    // display pcap: tcpdump -# -tt -r abbp2p-0-0.pcap
    // also display pcap with wireshark
  }

  //
  // Let the scheduler start
  //

  // schedule a simulator stop event
  // this is needed only if the simulation generates self-sustaining events
  //Simulator::Stop(Seconds(11.0));

  Simulator::Run();
  Simulator::Destroy();

  NS_LOG_INFO(THIS_SCRIPT " ends now.");
  return 0;
}