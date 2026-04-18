#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/tcp-congestion-ops.h" 
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector> 
#include <map>    

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpHystartEvaluation");

class TcpAshish : public TcpNewReno {
public:
  static TypeId GetTypeId (void) {
    static TypeId tid = TypeId ("ns3::TcpAshish")
      .SetParent<TcpNewReno> ()
      .SetGroupName ("Internet")
      .AddConstructor<TcpAshish> ();
    return tid;
  }

  TcpAshish () : TcpNewReno (), m_delayMin (Time::Max ()), m_thresholdFound (false) {}
  
  TcpAshish (const TcpAshish& sock) 
    : TcpNewReno (sock), 
      m_delayMin (sock.m_delayMin), 
      m_currRtt (sock.m_currRtt), 
      m_thresholdFound (sock.m_thresholdFound) {}

  virtual ~TcpAshish () {}

  virtual std::string GetName () const override { return "TcpAshish"; }
  
  virtual Ptr<TcpCongestionOps> Fork () override {
    return CreateObject<TcpAshish> (*this);
  }

  virtual void PktsAcked (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked, const Time& rtt) override {
    if (rtt.IsStrictlyPositive ()) {
      if (m_delayMin == Time::Max () || rtt < m_delayMin) {
        m_delayMin = rtt; 
      }
      m_currRtt = rtt;
    }
    TcpNewReno::PktsAcked(tcb, segmentsAcked, rtt);
  }

  virtual void IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked) override {
    if (tcb->m_cWnd < tcb->m_ssThresh) {
      if (!m_thresholdFound && m_delayMin != Time::Max ()) {
        Time threshold = MilliSeconds(4); 
        if (m_currRtt > m_delayMin + threshold) {
          m_thresholdFound = true; 
          tcb->m_ssThresh = tcb->m_cWnd; 
        }
      }
      
      if (!m_thresholdFound) {
        tcb->m_cWnd += tcb->m_segmentSize;
      }
    } else {
      TcpNewReno::IncreaseWindow(tcb, segmentsAcked);
    }
  }

private:
  Time m_delayMin;
  Time m_currRtt;
  bool m_thresholdFound; 
};

NS_OBJECT_ENSURE_REGISTERED (TcpAshish);

static void CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd) {
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}

static void RttChange (Ptr<OutputStreamWrapper> stream, Time oldRtt, Time newRtt) {
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldRtt.GetSeconds () << "\t" << newRtt.GetSeconds () << std::endl;
}

static void SsthreshChange (Ptr<OutputStreamWrapper> stream, uint32_t oldSsthresh, uint32_t newSsthresh) {
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldSsthresh << "\t" << newSsthresh << std::endl;
}

void AttachTraces (std::string prefix, uint32_t node0Id, uint32_t nodeCustomId, bool mix) {
  AsciiTraceHelper ascii;
  Config::ConnectWithoutContextFailSafe ("/NodeList/" + std::to_string(node0Id) + "/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", 
                                         MakeBoundCallback (&CwndChange, ascii.CreateFileStream (prefix + "_NewReno_cwnd.txt")));
  Config::ConnectWithoutContextFailSafe ("/NodeList/" + std::to_string(node0Id) + "/$ns3::TcpL4Protocol/SocketList/0/RTT", 
                                         MakeBoundCallback (&RttChange, ascii.CreateFileStream (prefix + "_NewReno_rtt.txt")));
  
  if (mix) {
    Config::ConnectWithoutContextFailSafe ("/NodeList/" + std::to_string(nodeCustomId) + "/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", 
                                           MakeBoundCallback (&CwndChange, ascii.CreateFileStream (prefix + "_TcpAshish_cwnd.txt")));
    Config::ConnectWithoutContextFailSafe ("/NodeList/" + std::to_string(nodeCustomId) + "/$ns3::TcpL4Protocol/SocketList/0/SlowStartThreshold", 
                                           MakeBoundCallback (&SsthreshChange, ascii.CreateFileStream (prefix + "_TcpAshish_ssthresh.txt")));
    Config::ConnectWithoutContextFailSafe ("/NodeList/" + std::to_string(nodeCustomId) + "/$ns3::TcpL4Protocol/SocketList/0/RTT", 
                                           MakeBoundCallback (&RttChange, ascii.CreateFileStream (prefix + "_TcpAshish_rtt.txt")));                                      
  }
}

int main (int argc, char *argv[]) {
  uint32_t totflow = 4;
  bool mix = false; 

  CommandLine cmd;
  cmd.AddValue ("nFlows", "Total number of TCP flows", totflow);
  cmd.AddValue ("isMixed", "If true, half flows use TcpAshish", mix);
  cmd.Parse (argc, argv);

  NodeContainer leftNodes, rightNodes, routers;
  leftNodes.Create (totflow);
  rightNodes.Create (totflow);
  routers.Create (2);

  PointToPointHelper p2pleaf, p2pbotn;
  p2pleaf.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2pleaf.SetChannelAttribute ("Delay", StringValue ("2ms"));
  
  p2pbotn.SetDeviceAttribute ("DataRate", StringValue ("2Mbps"));
  p2pbotn.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer routerDevices = p2pbotn.Install (routers.Get (0), routers.Get (1));
  std::vector<NetDeviceContainer> leftLeafDevices, rightLeafDevices;
  for (uint32_t i = 0; i < totflow; ++i) {
    leftLeafDevices.push_back(p2pleaf.Install (leftNodes.Get (i), routers.Get (0)));
    rightLeafDevices.push_back(p2pleaf.Install (rightNodes.Get (i), routers.Get (1)));
  }

  InternetStackHelper stack;
  stack.Install (leftNodes);
  stack.Install (rightNodes);
  stack.Install (routers);

  for (uint32_t i = 0; i < totflow; ++i) {
    std::string algo = (mix && i >= totflow / 2) ? "ns3::TcpAshish" : "ns3::TcpNewReno";
    Config::Set ("/NodeList/" + std::to_string(leftNodes.Get(i)->GetId()) + "/$ns3::TcpL4Protocol/SocketType", 
                 TypeIdValue(TypeId::LookupByName(algo)));
  }

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  address.Assign (routerDevices);

  std::vector<Ipv4InterfaceContainer> rightInterfaces;
  for (uint32_t i = 0; i < totflow; ++i) {
    std::ostringstream leftSubnet, rightSubnet;
    leftSubnet << "10.2." << i + 1 << ".0";
    rightSubnet << "10.3." << i + 1 << ".0";
    
    address.SetBase (Ipv4Address(leftSubnet.str().c_str()), "255.255.255.0");
    address.Assign (leftLeafDevices[i]);
    
    address.SetBase (Ipv4Address(rightSubnet.str().c_str()), "255.255.255.0");
    rightInterfaces.push_back(address.Assign (rightLeafDevices[i]));
  }
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  uint16_t sinkPort = 9000; 
  for (uint32_t i = 0; i < totflow; ++i) {
    Address sinkAddress (InetSocketAddress (rightInterfaces[i].GetAddress (0), sinkPort));
    PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkAddress);
    ApplicationContainer sinkApp = sinkHelper.Install (rightNodes.Get (i));
    sinkApp.Start (Seconds (1.0));
    sinkApp.Stop (Seconds (10.0)); 

    OnOffHelper clientHelper ("ns3::TcpSocketFactory", sinkAddress);
    clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    clientHelper.SetAttribute ("DataRate", DataRateValue (DataRate ("5Mbps"))); 
    clientHelper.SetAttribute ("PacketSize", UintegerValue (1024));
    
    ApplicationContainer clientApp = clientHelper.Install (leftNodes.Get (i));
    clientApp.Start (Seconds (2.0));
    clientApp.Stop (Seconds (10.0)); 
  }

  std::string prefix = mix ? "Mixed_" + std::to_string(totflow) : "Baseline_" + std::to_string(totflow);
  Simulator::Schedule (Seconds (2.0001), &AttachTraces, prefix, leftNodes.Get(0)->GetId(), leftNodes.Get(totflow - 1)->GetId(), mix);

  FlowMonitorHelper flowmonHelper;
  Ptr<FlowMonitor> flowmon = flowmonHelper.InstallAll ();

  Simulator::Stop (Seconds (10.0)); 
  Simulator::Run ();

  flowmon->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = flowmon->GetFlowStats ();

  double sthroghput = 0, sthroghputSq = 0;
  int validFlows = 0;

  for (auto const &i : stats) {
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i.first);
    std::ostringstream destStream; destStream << t.destinationAddress;
    
    if (destStream.str().find("10.3.") == 0 && t.destinationPort == sinkPort) {
      double throughput = i.second.rxBytes * 8.0 / (10.0 - 2.0) / 1000000.0;
      if (throughput > 0) {
        sthroghput += throughput;
        sthroghputSq += (throughput * throughput);
        validFlows++;
      }
    }
  }

  if (validFlows > 0 && sthroghputSq > 0) {
    double jainsIndex = (sthroghput * sthroghput) / (validFlows * sthroghputSq);
    std::cout << jainsIndex << std::endl;
  } else {
    std::cout << 0.0000 << std::endl;
  }

  Simulator::Destroy ();
  return 0;
}