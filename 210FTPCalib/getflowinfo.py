#!/usr/bin/env python3
#
# This script processes the data generated by Flow Monitor. The data file
# is an XML file. Its detailed format is documented in a PDF file
# called "xml-results-format.pdf" in my Google Drive.
#
# Written by following example script:
#       ns-3.30.1/src/flow-monitor/examples/flowmon-parse-results.py
#
# Usage: ./getflowinfo.py out/results.xml 192.168.1
#

import sys
#import os
import xml.etree.ElementTree as ET


def parse_time_ns(tm):
    if tm.endswith('ns'):
        if tm.find('e') > 0:
            return int(float(tm[:-2]))
        return int(tm[:-2])
    raise ValueError(tm)


## FiveTuple
class FiveTuple(object):
    ## class variables
    ## @var sourceAddress
    #  source address
    ## @var destinationAddress
    #  destination address
    ## @var protocol
    #  network protocol
    ## @var sourcePort
    #  source port
    ## @var destinationPort
    #  destination port
    ## @var __slots__
    #  class variable list
    __slots__ = ['sourceAddress', 'destinationAddress',
                 'protocol', 'sourcePort', 'destinationPort']
    def __init__(self, el):
        '''The initializer.
        @param self The object pointer.
        @param el The flow element of a FlowClassifier.
        '''
        self.sourceAddress = el.get('sourceAddress')
        self.destinationAddress = el.get('destinationAddress')
        self.sourcePort = int(el.get('sourcePort'))
        self.destinationPort = int(el.get('destinationPort'))
        self.protocol = int(el.get('protocol'))


## Histogram
class Histogram(object):
    ## class variables
    ## @var bins
    #  histogram bins
    ## @var nbins
    #  number of bins
    ## @var number_of_flows
    #  number of flows
    ## @var __slots__
    #  class variable list
    __slots__ = ['bins', 'nbins', 'number_of_flows']
    def __init__(self, el=None):
        ''' The initializer.
        @param self The object pointer.
        @param el The element.
        '''
        self.bins = []
        self.nbins = 0
        if el is not None:
            self.nbins = int(el.get('nBins'))
            for bin in el.findall('bin'):
                self.bins.append( (float(bin.get("start")),
                                   float(bin.get("width")),
                                   int(bin.get("count"))) )

## Flow
class Flow(object):
    ## class variables
    ## @var flowId
    #  flow ID
    ## @var delayMean
    #  mean delay
    ## @var packetLossRatio
    #  packet loss ratio
    ## @var rxBitrate
    #  receive bit rate
    ## @var txBitrate
    #  transmit bit rate
    ## @var fiveTuple
    #  five tuple
    ## @var packetSizeMean
    #  packet size mean
    ## @var probe_stats_unsorted
    #  unsorted probe stats
    ## @var hopCount
    #  hop count
    ## @var flowInterruptionsHistogram
    #  flow interruption histogram
    ## @var rx_duration
    #  receive duration
    ## @var tx_duration
    #  transmit duration
    ## @var __slots__
    #  class variable list
    __slots__ = ['flowId', 'delayMean', 'jitterMean', 'packetLossRatio',
                 'rxBitrate', 'txBitrate',
                 'fiveTuple',
                 'packetSizeMean', 'probe_stats_unsorted',
                 'hopCount', 'flowInterruptionsHistogram',
                 'rx_duration', 'tx_duration']
    def __init__(self, flow_el):
        ''' The initializer.
        @param self The object pointer.
        @param flow_el The element.
        '''
        self.flowId = int(flow_el.get('flowId'))
        
        rxPackets = int(flow_el.get('rxPackets'))
        txPackets = int(flow_el.get('txPackets'))
        
        tx_duration = float(parse_time_ns(flow_el.get('timeLastTxPacket')) -
                        parse_time_ns(flow_el.get('timeFirstTxPacket')))*1e-9
        rx_duration = float(parse_time_ns(flow_el.get('timeLastRxPacket')) -
                        parse_time_ns(flow_el.get('timeFirstRxPacket')))*1e-9
        #print(tx_duration, rx_duration)
        self.rx_duration = rx_duration
        self.tx_duration = tx_duration
        
        self.probe_stats_unsorted = []
        
        if rxPackets:
            self.hopCount = float(flow_el.get('timesForwarded')) / rxPackets + 1
        else:
            self.hopCount = -1000
        
        if rxPackets:
            self.delayMean = float(parse_time_ns(flow_el.get('delaySum'))) \
                                / rxPackets * 1e-9
            self.jitterMean = float(parse_time_ns(flow_el.get('jitterSum'))) \
                                / (rxPackets - 1) * 1e-9
            self.packetSizeMean = float(flow_el.get('rxBytes')) / rxPackets
        else:
            self.delayMean = None
            self.jitterMean = None
            self.packetSizeMean = None
        
        if rx_duration > 0:
            self.rxBitrate = int(flow_el.get('rxBytes'))*8 / rx_duration
        else:
            self.rxBitrate = None
        
        if tx_duration > 0:
            self.txBitrate = int(flow_el.get('txBytes'))*8 / tx_duration
        else:
            self.txBitrate = None
            
        lost = float(flow_el.get('lostPackets'))
        if rxPackets == 0:
            self.packetLossRatio = None
        else:
            self.packetLossRatio = (lost / (rxPackets + lost))

        interrupt_hist_elem = flow_el.find("flowInterruptionsHistogram")
        if interrupt_hist_elem is None:
            self.flowInterruptionsHistogram = None
        else:
            self.flowInterruptionsHistogram = Histogram(interrupt_hist_elem)

    def setFiveTuple(self, ft):
        ''' The initializer.
        @param self The object pointer.
        @param ft The FiveTuple.
        '''
        self.fiveTuple = ft


## ProbeFlowStats
class ProbeFlowStats(object):
    ## class variables
    ## @var probeIndex
    #  probe index
    ## @var flowId
    #  flow ID
    ## @var packets
    #  network packets
    ## @var bytes
    #  bytes
    ## @var delayFromFirstProbe
    #  delay from first probe
    ## @var __slots__
    #  class variable list
    __slots__ = ['probeIndex', 'flowId', 'packets', 'bytes',
                 'delayFromFirstProbe']
    def __init__(self, probeIndex, stats_el):
        self.probeIndex = probeIndex
        self.flowId = int(stats_el.get('flowId'))
        self.packets = int(stats_el.get('packets'))
        self.bytes = int(stats_el.get('bytes'))
        if self.packets > 0:
            self.delayFromFirstProbe = \
                    parse_time_ns(stats_el.get('delayFromFirstProbeSum')) \
                            / float(self.packets)
        else:
            self.delayFromFirstProbe = 0

## Simulation
class Simulation(object):
    ## class variables
    ## @var flows
    #  list of flows
    __slots__ = ['flows']
    def __init__(self, simulation_el):
        ''' The initializer.
        @param self The object pointer.
        @param simulation_el The element.
        '''
        self.flows = []
        
        # handle 'Flow' elements under 'FlowStats'
        flow_map = {}
        for flow_el in simulation_el.findall("FlowStats/Flow"):
            flow = Flow(flow_el)    # from 'Flow' element to Flow object
            flow_map[flow.flowId] = flow
            self.flows.append(flow)

        # handle flow classifier to attach each flow with 5-tuple id
        # (we expect only one FlowClassifier element)
        FlowClassifier_el, = simulation_el.findall("Ipv4FlowClassifier")
        for flow_cls in FlowClassifier_el.findall("Flow"):
            flowId = int(flow_cls.get('flowId'))
            flow_map[flowId].setFiveTuple(FiveTuple(flow_cls))

        for probe_elem in simulation_el.findall("FlowProbes/FlowProbe"):
            probeIndex = int(probe_elem.get('index'))
            for stats in probe_elem.findall("FlowStats"):
                flowId = int(stats.get('flowId'))
                s = ProbeFlowStats(probeIndex, stats)
                flow_map[flowId].probe_stats_unsorted.append(s)

def sort_flow_by_dest(flow):
    return flow.fiveTuple.destinationAddress

def sort_flow_by_src(flow):
    return flow.fiveTuple.sourceAddress

def main(argv):
    if len(argv) != 3:
        print('Usage: ./getflowinfo.py results.xml sink_ipaddr_prefix (e.g., 192.168.1)')
        return
    
    infile = open(argv[1])  # input flow info result file
    print('Reading flow monitor result input file in XML format:')
    
    xml_tag_level = 0
    sim_result_list = []    # simulation result list
                            # (normally the input file contains the
                            # result for one simulation only)
    for event, elem in ET.iterparse(infile, events=('start', 'end')):
        if event == 'start':
            xml_tag_level += 1
        if event == 'end':
            xml_tag_level -= 1
            
            # when it is top level tag 'FlowMonitor' ...
            if xml_tag_level == 0 and elem.tag == 'FlowMonitor':
                sim = Simulation(elem)
                sim_result_list.append(sim)
                elem.clear()

    print(len(sim_result_list), 'simulation(s) parsed.')
    
    sink_prefix = argv[2]   # to separate flows into ...
    ds_flows = []
    us_flows = []
    for sim in sim_result_list:
        for flow in sim.flows:
            t = flow.fiveTuple
            if t.sourceAddress.startswith(sink_prefix):
                us_flows.append(flow)
            if t.destinationAddress.startswith(sink_prefix):
                ds_flows.append(flow)
            if not (t.sourceAddress.startswith(sink_prefix) 
                    or t.destinationAddress.startswith(sink_prefix)):
                print("Unknown flow from", t.sourceAddress, "to", t.destinationAddress)

    header = 'Flow Dir Type Src              Dest            '
    header += ' TxRate TxPeriod RxRate RxPeriod'
    header += ' DelayMean JitterMean LossRatio(%)'
    proto_type = {6: 'TCP', 17: 'UDP'}

    print()
    print(header)
    totTxRate = 0
    totRxRate = 0
    ds_flows.sort(key=sort_flow_by_dest)
    for flow in ds_flows:
        t = flow.fiveTuple
        proto = proto_type[t.protocol]
        flowinfo = f'{flow.flowId:<4d} DS  {proto}  '
        src = t.sourceAddress + ':' + str(t.sourcePort)
        dst = t.destinationAddress + ':' + str(t.destinationPort)
        flowinfo += f'{src:16s} {dst:16s} '
        if flow.txBitrate is None:
            flowinfo += '------ '
        else:
            flowinfo += f'{flow.txBitrate*1e-6:6.2f} '
            totTxRate += flow.txBitrate
        flowinfo += f'{flow.tx_duration:7.1f}  '
        if flow.rxBitrate is None:
            flowinfo += '------ '
        else:
            flowinfo += f'{flow.rxBitrate*1e-6:6.2f} '
            totRxRate += flow.rxBitrate
        flowinfo += f'{flow.rx_duration:7.1f}  '
        if flow.delayMean is None:
            flowinfo = '--------  '
        else:
            flowinfo += f'{flow.delayMean:8.5f}  '
        if flow.jitterMean is None:
            flowinfo += '--------   '
        else:
            flowinfo += f'{flow.jitterMean:8.5f}   '
        if flow.packetLossRatio is None:
            flowinfo += '------'
        else:
            flowinfo += f'{flow.packetLossRatio*100:6.3f}'
        print(flowinfo)
    print("Total " + "TxRate = " + f'{totTxRate*1.0e-6:6.2f} ' + "RxRate = " + f'{totRxRate*1.0e-6:6.2f}' + " Mbps")

    print()
    print(header)
    totTxRate = 0
    totRxRate = 0
    us_flows.sort(key=sort_flow_by_src)
    for flow in us_flows:
        t = flow.fiveTuple
        proto = proto_type[t.protocol]
        flowinfo = f'{flow.flowId:<4d} US  {proto}  '
        src = t.sourceAddress + ':' + str(t.sourcePort)
        dst = t.destinationAddress + ':' + str(t.destinationPort)
        flowinfo += f'{src:16s} {dst:16s} '
        if flow.txBitrate is None:
            flowinfo += '------ '
        else:
            flowinfo += f'{flow.txBitrate*1e-6:6.2f} '
            totTxRate += flow.txBitrate
        flowinfo += f'{flow.tx_duration:7.1f}  '
        if flow.rxBitrate is None:
            flowinfo += '------ '
        else:
            flowinfo += f'{flow.rxBitrate*1e-6:6.2f} '
            totRxRate += flow.rxBitrate
        flowinfo += f'{flow.rx_duration:7.1f}  '
        if flow.delayMean is None:
            flowinfo = '--------  '
        else:
            flowinfo += f'{flow.delayMean:8.5f}  '
        if flow.jitterMean is None:
            flowinfo += '--------   '
        else:
            flowinfo += f'{flow.jitterMean:8.5f}   '
        if flow.packetLossRatio is None:
            flowinfo += '------'
        else:
            flowinfo += f'{flow.packetLossRatio*100:6.3f}'
        print(flowinfo)
    print("Total " + "TxRate = " + f'{totTxRate*1.0e-6:6.2f} ' + "RxRate = " + f'{totRxRate*1.0e-6:6.2f}' + " Mbps")

    print()

# Only run the main function when this script runs
# as the main script
if __name__ == '__main__':
    main(sys.argv)
