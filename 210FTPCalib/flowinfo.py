#!/usr/bin/env python3
#
# This is the slightly "beautified" Python 3 version of the:
#   ns-3.30.1/src/flow-monitor/examples/flowmon-parse-results.py
#
# Usage: ./flowinfo.py out/results.xml
#

import sys
import os
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


def main(argv):
    #print(argv)
    if len(argv) != 2:
        print('Usage: ./flowinfo.py results.xml')
        return
        
    file_obj = open(argv[1])
    print("Reading flow monitor result file in XML format:")
    
    level = 0
    sim_list = []
    for event, elem in ET.iterparse(file_obj, events=("start", "end")):
        if event == "start":
            #print('    ' * level, end='')
            #print(elem)
            level += 1
        if event == "end":
            level -= 1
            #print('    ' * level, end='')
            #print(elem)
            if level == 0 and elem.tag == 'FlowMonitor':
                sim = Simulation(elem)
                sim_list.append(sim)
                elem.clear()            # won't need this any more
    
    print("Done reading XML file.")
    print()
    
    for sim in sim_list:
        for flow in sim.flows:
            t = flow.fiveTuple
            proto = {6: 'TCP', 17: 'UDP'}[t.protocol]
            print("FlowID: %i (%s %s/%s --> %s/%s)" % (flow.flowId, proto, t.sourceAddress, t.sourcePort, t.destinationAddress, t.destinationPort))

            if flow.txBitrate is None:
                print("\tTX bitrate: None")
            else:
                print("\tTX bitrate: %.2f kbit/s" % (flow.txBitrate*1e-3))

            if flow.rxBitrate is None:
                print("\tRX bitrate: None")
            else:
                print("\tRX bitrate: %.2f kbit/s" % (flow.rxBitrate*1e-3))

            if flow.delayMean is None:
                print("\tMean Delay: None")
            else:
                print("\tMean Delay: %.2f ms" % (flow.delayMean*1e3))

            if flow.packetLossRatio is None:
                print("\tPacket Loss Ratio: None")
            else:
                print("\tPacket Loss Ratio: %.2f %%" % (flow.packetLossRatio*100))


if __name__ == '__main__':
    main(sys.argv)
