#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;

#define THIS_SCRIPT "200FTP"

NS_LOG_COMPONENT_DEFINE(THIS_SCRIPT);

// ABB qdisc requires a flow table that specifies flow ids and weights
struct FlowTableEntry
{
    /// Flow id (for human eyes only)
    uint32_t m_id;
    // - better match it with the same id given by flow monitor

    // We will only deal with IPv4 address for now
    uint32_t m_srcAddr;
    uint32_t m_dstAddr;

    /// The flow weight
    double m_weight;
};

// default script settings
uint32_t simTime   = 10;        // simulation time in seconds
bool simParamsPrint = true;     // simulation parameters displayed
bool heartBeatOn    = true;     // heart beat (message) enabled
bool pingOn         = false;    // pinging disabled
bool tracing        = false;    // tracing disabled
uint32_t nFTPs      = 1;        // number of FTP flows
uint32_t nLanNodes  = 3;        // number of LAN nodes
std::string qdType;             // queue disc type (NS3 default to pfifo_fast)
uint32_t qdQSize    = 1000;     // queue disc queue size in packets (NS3 default to 1000)
std::string devQType;           // device queue type (NS3 default to DropTail)
uint32_t devQSize   = 100;      // device queue size in packets (NS3 default to 100 packets)
uint32_t holdTime   = 5;        // hold time for device queue (in unit of ms)
std::string flowTableFileName = "abb-flow-table.csv";   // for ABB queue disc only
std::vector<FlowTableEntry> flowTable;                  // for ABB queue disc only

// supported names of queue disciplines
//  To use these qdiscs, you must specifiy proper qdisc queue
//  size via command line arguments
//
//      ""          -- use ns3::PfifoFastQueueDisc (ns3 default)
//      "Fifo"      -- use ns3::FifoQueueDisc
//      "Codel"     -- use ns3::CoDelQueueDisc
//      "Pie"       -- use ns3::PieQueueDis
//      "FqCodel"   -- use ns3::FqCoDelQueueDisc
//      "ABBCoDel"  -- use ns3::ABBCoDelQueueDisc
//      

static void
heartbeat()
{
    Time now = Simulator::Now();
    NS_LOG_INFO("Heart Beat: " << now);
    if (now <= Seconds(simTime - 1))
    {
        Simulator::Schedule(Seconds(1), &heartbeat);
    }
}

// Callback for ABBCoDelQueueDisc to find flow ID
static uint32_t
GetFlowId(Ptr<QueueDiscItem> item)
{
    // Is it an ArpQueueDiscItem?
    Ptr<ArpQueueDiscItem> arpItem = DynamicCast<ArpQueueDiscItem>(item);
    if (arpItem != nullptr) {
        NS_LOG_DEBUG("ARP packet");
        // treat ARP as from flow 0
        return 0;
    }

    // Is it an Ipv4QueueDiscItem?
    Ptr<Ipv4QueueDiscItem> ipv4Item = DynamicCast<Ipv4QueueDiscItem>(item);
    if (ipv4Item != nullptr) {
        NS_LOG_DEBUG("IPv4 packet");
        const Ipv4Header& hdr = ipv4Item->GetHeader();
        uint32_t srcIpAddr = hdr.GetSource().Get();
        uint32_t dstIpAddr = hdr.GetDestination().Get();

        for (FlowTableEntry& e : flowTable) {
            if (e.m_srcAddr == srcIpAddr && e.m_dstAddr == dstIpAddr) {
                return e.m_id;
            }
        }

        NS_FATAL_ERROR("cannot find flow id");
    }

    // This script doesn't use IPv6
    NS_ABORT_MSG("unexpected types of queue disc item");

    return 0;
}

// Callback for ABBCoDelQueueDisc to find flow weight
static double
GetFlowWeight(Ptr<QueueDiscItem> item)
{
    // Is it an ArpQueueDiscItem?
    Ptr<ArpQueueDiscItem> arpItem = DynamicCast<ArpQueueDiscItem>(item);
    if (arpItem != nullptr) {
        // treat ARP as from flow 0 and set its weight to 0.1 (very low)
        return 0.1;
    }

    // Is it an Ipv4QueueDiscItem?
    Ptr<Ipv4QueueDiscItem> ipv4Item = DynamicCast<Ipv4QueueDiscItem>(item);
    if (ipv4Item != nullptr) {
        const Ipv4Header& hdr = ipv4Item->GetHeader();
        uint32_t srcIpAddr = hdr.GetSource().Get();
        uint32_t dstIpAddr = hdr.GetDestination().Get();

        for (FlowTableEntry& e : flowTable) {
            if (e.m_srcAddr == srcIpAddr && e.m_dstAddr == dstIpAddr) {
                return e.m_weight;
            }
        }
        
        NS_FATAL_ERROR("cannot find flow id");
    }

    // This script doesn't use IPv6
    NS_LOG_WARN("unexpected types of queue disc item");

    return 1.0;
}

int
main(int argc, char* argv[])
{
    // NS log is disabled by default
    NS_LOG_INFO(THIS_SCRIPT " starts ...");

    CommandLine cmd;
    cmd.AddValue("simTime", "Simulation time in unit of seconds", simTime);
    cmd.AddValue("simParamsPrint", "Printing simulation parameters", simParamsPrint);
    cmd.AddValue("heartBeatOn", "Enable heart beat (message) if true", heartBeatOn);
    cmd.AddValue("pingOn", "Capture PCAP traces if true", pingOn);
    cmd.AddValue("tracing", "Turn on tracing if true", tracing);
    cmd.AddValue("nFTPs", "Number of FTP flows", nFTPs);
    cmd.AddValue("nLanNodes", "Number of LAN nodes on local net", nLanNodes);
    cmd.AddValue("qdType", "Qdisc type (set to DropTail for now or not set at all)", qdType);
    cmd.AddValue("qdQSize", "Qdisc queue size", qdQSize);
    cmd.AddValue("devQType", "Device queue type", devQType);
    cmd.AddValue("devQSize", "Device queue size", devQSize);
    cmd.Parse(argc, argv);

    // adjust nLanNodes to be at least nFTPs
    nLanNodes = (nLanNodes == 0) ? 1 : nLanNodes;   // cannot be zero
    if (nLanNodes < nFTPs) nLanNodes = nFTPs;       // at least equal

    // Print out simulation parameters (maybe changed by CommandLine arguments)
    if (simParamsPrint) {
        std::cout << "Simulation parameters" << std::endl;
        std::cout << "simTime = " << simTime << std::endl;
        std::cout << "heartBeatOn = " << heartBeatOn << std::endl;
        std::cout << "pingOn = " << pingOn << std::endl;
        std::cout << "tracing = " << tracing << std::endl;
        std::cout << "nFTPs = " << nFTPs << std::endl;
        std::cout << "nLanNodes = " << nLanNodes << std::endl;
        std::cout << "qdType = " << qdType << std::endl;
        std::cout << "qdQSize = " << qdQSize << std::endl;
        std::cout << "devQType = " << devQType << std::endl;
        std::cout << "devQSize = " << devQSize << std::endl;
        std::cout << "holdTime = " << holdTime << std::endl;
    }

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

    NodeContainer ftpNodes;         // running FTP sources
    ftpNodes.Create(nFTPs);

    PointToPointHelper p2pHelper;	// p2p network connection (helper)
    p2pHelper.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
    p2pHelper.SetChannelAttribute("Delay", StringValue("2ms"));
    // This helper sets a default DropTail device queue of 100 packets
    // To change it, uncomment below line of code and modify it
    //p2pHelper.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1000p"));

    NetDeviceContainer rlNodesP2PNetDevices;	// root to local net
    rlNodesP2PNetDevices = p2pHelper.Install(rtNode, lrNode);
    // at this moment, the two nodes are connected by an unnamed
    // PointToPointChannel attached to two PointToPointNetDevice's
    // that are put in the device container

    p2pHelper.SetDeviceAttribute("DataRate", StringValue("10Gbps"));

    NetDeviceContainer roNodesP2PNetDevices;	// root to other router
    roNodesP2PNetDevices = p2pHelper.Install(rtNode, orNode);

    NetDeviceContainer rfNodesP2PNetDevices;	// root to FTP router
    rfNodesP2PNetDevices = p2pHelper.Install(rtNode, frNode);

    //p2pHelper.SetDeviceAttribute("DataRate", StringValue("1Gbps"));

    // P2P star topology for FTP servers with FTP router as the hub
    std::vector<NetDeviceContainer> ftpNodesP2PNetDevicesAL(nFTPs); // list of containers
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
    if (devQType.length() == 0 || devQType == "DropTail") {
        csmaHelper.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(std::to_string(devQSize)+"p"));
    } else {
        NS_LOG_ERROR(THIS_SCRIPT " Only the default device queue type (DropTail) is supported.");
        std::cout << "Only the default device queue type (DropTail) is supported." << std::endl;
        exit(-1);
    }
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
    inetStackHelper.Install(lanNodes);

    TrafficControlHelper tch;
    if (qdType.length() != 0) {
        // Different qdisc may require different parameters
        // The SetRootQueueDisc method can accept all sorts, varying number of config parameters!!
        if (qdType == "FqCoDel") {
            tch.SetRootQueueDisc("ns3::FqCoDelQueueDisc", 
                                "MaxSize", StringValue(std::to_string(qdQSize)+"p"),
                                "DropBatchSize", UintegerValue(16));
            // TODO: should we turn off ECN?
        } else if (qdType == "ABBCoDel") {
            // read flow table from a flow table file
            std::cout << "flowTable = " << flowTableFileName << std::endl;
            std::ifstream fin(flowTableFileName);
            char temp[255];
            while (fin.getline(temp, 255)) {
                if (temp[0] < '0' || temp[0] > '9')     // skip header/comment lines
                    continue;
                uint32_t flowId;
                uint32_t a1,a2,a3,a4;
                uint32_t b1,b2,b3,b4;
                double flowWeight;
                sscanf(temp, "%u,%u.%u.%u.%u,%u.%u.%u.%u,%lf", 
                        &flowId, &a1, &a2, &a3, &a4,
                        &b1, &b2, &b3, &b4, &flowWeight);

                FlowTableEntry entry;
                entry.m_id = flowId;
                entry.m_srcAddr = ((a1*256 + a2)*256 + a3)*256 + a4;
                entry.m_dstAddr = ((b1*256 + b2)*256 + b3)*256 + b4;
                entry.m_weight = flowWeight;
                flowTable.push_back(entry);
            }
            //for (FlowTableEntry& e : flowTable) {
            //    std::cout << e.m_id << "," << e.m_srcAddr << "," << e.m_dstAddr 
            //                << "," << e.m_weight << std::endl;
            //}
            std::cout << "Finished loading flow table" << std::endl;

            tch.SetRootQueueDisc("ns3::ABBCoDelQueueDisc", 
                                "MaxSize", StringValue(std::to_string(qdQSize)+"p"),
                                "DropBatchSize", UintegerValue(16),
                                "FlowIdCb", CallbackValue(MakeCallback<uint32_t,Ptr<QueueDiscItem>>(&GetFlowId)),
                                "FlowWeightCb", CallbackValue(MakeCallback<double,Ptr<QueueDiscItem>>(&GetFlowWeight)));
            // TODO: should we turn off ECN?
            // TODO: we need to set channel bandwidth (default 1000 mbps)
            // TODO: set number of bins (default 5), reclassification interval (default 1s)
        } else {
            tch.SetRootQueueDisc("ns3::"+qdType+"QueueDisc", 
                                "MaxSize", StringValue(std::to_string(qdQSize)+"p"));
        }

        // Enable BQL on netdevice queue
        tch.SetQueueLimits("ns3::DynamicQueueLimits", "HoldTime", StringValue(std::to_string(holdTime)+"ms"));
    }
    tch.Install(lanNodesNetDevices.Get(0));     // Only installed on LAN router node
    //tch.Install(lanNodesNetDevices);          // This will install on all LAN nodes

    // When an IP address is assigned to an interface, default queue discipline
    // (pfifo_fast) gets installed for the traffic control layer of netdevices
    // unless ... you specify your own

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
        pingApps.Stop(Seconds(simTime));
    }

    uint16_t port = 21;
    uint32_t maxBytes = 0;	// unlimited
    ApplicationContainer allSinkApps;
    for (uint32_t i = 0; i < nFTPs; i++) {
        // FTP source
        BulkSendHelper ftpSourceHelper("ns3::TcpSocketFactory",
                InetSocketAddress(lanNodesInetInterfaces.GetAddress(i+1), port));
        ftpSourceHelper.SetAttribute("MaxBytes", UintegerValue(maxBytes));
        //ftpSourceHelper.SetAttribute("SendSize", UintegerValue(1400));  // by default, 512
        ApplicationContainer ftpSourceApps = ftpSourceHelper.Install(ftpNodes.Get(i));
        ftpSourceApps.Start(Seconds(0.1));
        ftpSourceApps.Stop(Seconds(simTime));

        // FTP sink
        PacketSinkHelper ftpSinkHelper("ns3::TcpSocketFactory",
                InetSocketAddress(Ipv4Address::GetAny(), port));
        ApplicationContainer ftpSinkApps = ftpSinkHelper.Install(lanNodes.Get(i+1));
        ftpSinkApps.Start(Seconds(0.0));
        ftpSinkApps.Stop(Seconds(simTime+0.1));

        allSinkApps.Add(ftpSinkApps);
    }

    //
    // Enable flow monitor
    //
    Ptr<FlowMonitor> flowMonitor;
    // FlowMonitorHelper: can only be instantiated once
    FlowMonitorHelper flowMonitorHelper;
    //flowMonitor = flowMonitorHelper.InstallAll();
    flowMonitor = flowMonitorHelper.Install(ftpNodes);
    flowMonitor = flowMonitorHelper.Install(lanNodes);

	//
	// Enable traces
	//
	if (tracing) {
		AsciiTraceHelper asciiTraceHelper;
		csmaHelper.EnableAsciiAll(asciiTraceHelper.CreateFileStream("ftp.tr"));
		csmaHelper.EnablePcapAll("ftp", false);
	}
    
	//
	// Let the scheduler start
	//
    
    if (heartBeatOn)
        Simulator::Schedule(Seconds(1), &heartbeat);

	// schedule a simulator stop event
	// this is needed only if the simulation generates self-sustaining events
	Simulator::Stop(Seconds(simTime+0.1));

	Simulator::Run();
    
    flowMonitor->SerializeToXmlFile("results.xml", true, true);
    
	Simulator::Destroy();

	NS_LOG_INFO(THIS_SCRIPT " ends now.");

    return 0;
}
