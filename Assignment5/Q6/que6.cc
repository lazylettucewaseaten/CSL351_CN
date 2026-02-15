#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h" 
#include "ns3/mobility-module.h"

using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE ("Assignment5");
int main(int argc,char *argv[]){
bool trace=true;          

LogComponentEnable ("Assignment5", LOG_LEVEL_INFO);
string bandwidth="1Mbps";
string latency="1ms";
uint32_t packetsize=1024;
uint32_t maxipackets=10;
string interval="1s";
CommandLine cmd;
cmd.AddValue("bandwidth","link data rate",bandwidth);
cmd.AddValue("latency","latency of the Link",latency);
cmd.AddValue ("packetsize", "Size of packets", packetsize);
cmd.AddValue ("maxpackets", "Maximum number of packets", maxipackets);
cmd.AddValue ("interval", "Interval between packets", interval);

cmd.Parse(argc, argv);

Time::SetResolution (Time::NS);
NS_LOG_INFO ("Creating nodes.");
NodeContainer nodes;
nodes.Create (2);

NS_LOG_INFO ("Creating mobility forr nodes.");
MobilityHelper mobility;
mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
mobility.Install(nodes);


NS_LOG_INFO ("Creating channels.");
PointToPointHelper p2p;
p2p.SetDeviceAttribute ("DataRate", StringValue(bandwidth));
p2p.SetChannelAttribute ("Delay", StringValue(latency));

NetDeviceContainer devices;
devices = p2p.Install (nodes);

InternetStackHelper stack;
stack.Install (nodes);

NS_LOG_INFO ("assigning IP addr");
Ipv4AddressHelper address;
address.SetBase ("10.1.1.0", "255.255.255.0");
Ipv4InterfaceContainer interfaces = address.Assign (devices);

NS_LOG_INFO ("creating applications.");
uint16_t port=8790;
UdpServerHelper server(port);
ApplicationContainer appserver=server.Install(nodes.Get(1));
appserver.Start(Seconds(1.0));
appserver.Stop(Seconds(31.0)); 
UdpClientHelper client (interfaces.GetAddress (1), port); 
client.SetAttribute("MaxPackets", UintegerValue (maxipackets));
client.SetAttribute("Interval", TimeValue (Time(interval))); 
client.SetAttribute("PacketSize", UintegerValue (packetsize));
ApplicationContainer clientApp = client.Install (nodes.Get(0));
clientApp.Start(Seconds(1.0));
clientApp.Stop(Seconds(31.0));


NS_LOG_INFO ("creating applications with udp echo helper");
 uint16_t newport = 8793; 

  UdpEchoServerHelper echoServer(newport);
  ApplicationContainer echoServerApps = echoServer.Install(nodes.Get(1));
  echoServerApps.Start(Seconds(1.0));
  echoServerApps.Stop(Seconds(31.0));


  UdpEchoClientHelper echoClient(interfaces.GetAddress(1), newport);
  
  echoClient.SetAttribute("MaxPackets", UintegerValue(maxipackets));
  echoClient.SetAttribute("Interval", TimeValue(Time(interval)));
  echoClient.SetAttribute("PacketSize", UintegerValue(packetsize));
  ApplicationContainer echoapps = echoClient.Install(nodes.Get(0));
  echoapps.Start(Seconds(1.0));
  echoapps.Stop(Seconds(31.0));
if(trace){
  p2p.EnablePcapAll("p2p");   
}
  AnimationInterface anim("que6.xml");
  anim.SetConstantPosition(nodes.Get(0), 10.0, 20.0);
  anim.SetConstantPosition(nodes.Get(1), 60.0, 20.0);

Simulator::Stop(Seconds(31.0));
Simulator::Run();
Simulator::Destroy();
return 0;
}

