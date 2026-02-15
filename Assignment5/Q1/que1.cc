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
CommandLine cmd;
cmd.AddValue("bandwidth","link data rate",bandwidth);
cmd.AddValue("latency","latency of the Link",latency);
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

