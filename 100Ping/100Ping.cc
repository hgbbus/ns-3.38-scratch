#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

#define THIS_SCRIPT     "100Ping"

NS_LOG_COMPONENT_DEFINE(THIS_SCRIPT);

int main(int argc, char *argv[])
{
  // default parameters (can be overwriten via command line)
  bool      pingOn = false;   // default to be false
  uint32_t  nFTPs = 1;        // default to one FTP server
  uint32_t  nLanNodes = 3;    // default to three LAN nodes

  // NS log is disabled by default (can be turned on via command line)
  NS_LOG_INFO(THIS_SCRIPT " starts ...");

  // associate command line options with default simulation parameters
  CommandLine cmd;
  //cmd.Usage("This simulation script supports the following command line options:");
  cmd.AddValue("pingOn", "Capture PCAP traces if true", pingOn);
  cmd.AddValue("nFTPs", "Number of FTP server nodes", nFTPs);
  cmd.AddValue("nLanNodes", "Number of LAN nodes on local net", nLanNodes);
  cmd.Parse(argc, argv);  // this triggers the modification of simulation
                          // parameters via command line options

  // sanity check -- adjust nLanNodes to be at least nFTPs
  nLanNodes = (nLanNodes == 0) ? 1 : nLanNodes;
  if (nLanNodes < nFTPs) nLanNodes = nFTPs;

  //
  // Build nodes with devices and channels
  //

  NodeContainer coreNodes;				// core nodes
  coreNodes.Create(4);
  Ptr<Node> rtNode = coreNodes.Get(0);	// root node
  Ptr<Node> lrNode = coreNodes.Get(1);	// local net router node
  Ptr<Node> frNode = coreNodes.Get(2);	// ftp link router node
  Ptr<Node> orNode = coreNodes.Get(3);	// other link router node

  NodeContainer lanNodes;
  lanNodes.Add(lrNode);			// gateway node for the local net
  lanNodes.Create(nLanNodes);		// LAN nodes on the local net

  NodeContainer ftpNodes;
  ftpNodes.Create(nFTPs);

  // P2P networks are used to connect those router nodes using the code below

  PointToPointHelper p2pHelper;	// p2p network connection (helper)
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

  //p2pHelper.SetDeviceAttribute("DataRate", StringValue("1Gbps"));

  // P2P star topology for FTP server nodes with FTP router as the hub
  std::vector<NetDeviceContainer> ftpNodesP2PNetDevicesAL(nFTPs);
  for (uint32_t i=0; i<nFTPs; i++) {
    ftpNodesP2PNetDevicesAL[i] = p2pHelper.Install(frNode, ftpNodes.Get(i));
  }

  // use CSMA for local net topology
  // local net set to CSMA
  CsmaHelper csmaHelper;
  csmaHelper.SetChannelAttribute("DataRate", StringValue("1Gbps"));
  csmaHelper.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
  // propagation delay is set to 20ns per foot for a 100 meter cable
  //  (100m = 328ft, 328 x 20ns = 6560ns or 6.56us)
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
  Ipv4InterfaceContainer lanNodesInetInterfaces;	    // local net
  lanNodesInetInterfaces = ipv4AddrHelper.Assign(lanNodesNetDevices);

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

  if (pingOn) {
    Ipv4Address remote = Ipv4Address("172.16.1.2");
    Ipv4Address local = Ipv4Address("192.168.1.2");
    PingHelper pingHelper(remote, local);
    pingHelper.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    pingHelper.SetAttribute("Size", UintegerValue(1024));
    //pingHelper.SetAttribute("Verbose", BooleanValue(true)); // not available

    ApplicationContainer pingApps = pingHelper.Install(lanNodes.Get(1));
    pingApps.Start(Seconds(1.0));
    pingApps.Stop(Seconds(101.0));
  }

  //
  // Let the scheduler start
  //

  // schedule a simulator stop event
  // this is needed only if the simulation generates self-sustaining events
  //Simulator::Stop(Seconds(101.0));

  Simulator::Run();
  Simulator::Destroy();

  NS_LOG_INFO(THIS_SCRIPT " ends now.");
  
  return 0;
}
