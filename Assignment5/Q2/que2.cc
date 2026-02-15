#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h" 


using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE ("Assignment5");
int main(int argc,char *argv[]){
bool trace=true;          
bool anim=false;      

LogComponentEnable ("Assignment5", LOG_LEVEL_INFO);
string bandwidth="1Mbps";
string latency="1ms";
uint32_t packetsize=1024;
uint32_t maxipackets=100;
string interval="1s";
CommandLine cmd;
cmd.AddValue("bandwidth","link data rate",bandwidth);
cmd.AddValue("latency","latency of the Link",latency);
cmd.AddValue ("packetsize", "Size of packets", packetsize);
cmd.AddValue ("maxpackets", "Maximum number of packets", maxipackets);
cmd.AddValue ("interval", "Interval between packets", interval);

cmd.Parse(argc, argv);
NS_LOG_INFO ("Creating nodes.");
NodeContainer nodes;
nodes.Create (2);
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
appserver.Stop(Seconds(60.0)); 
UdpClientHelper client (interfaces.GetAddress (1), port); 
client.SetAttribute("MaxPackets", UintegerValue (maxipackets));
client.SetAttribute("Interval", TimeValue (Time(interval))); 
client.SetAttribute("PacketSize", UintegerValue (packetsize));
ApplicationContainer clientApp = client.Install (nodes.Get(0));
clientApp.Start (Seconds (2.0));
clientApp.Stop (Seconds (60.0));

if(trace){
  p2p.EnablePcapAll("p2p");   
}
if(anim){
  AnimationInterface anim("que1.xml");
  anim.SetConstantPosition(nodes.Get(0), 10.0, 20.0);
  anim.SetConstantPosition(nodes.Get(1), 60.0, 20.0);
}
Simulator::Stop(Seconds(2.0));
Simulator::Run();
Simulator::Destroy();
return 0;
}

