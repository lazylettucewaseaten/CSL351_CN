#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/error-model.h"

using namespace ns3;

int main(int argc, char* argv[]){
    double errorRate = 0.0;
    uint32_t seedr = 70; 
    std::string errorUnit = "Packet";

    CommandLine cmd;
    cmd.AddValue("seedRun", "", seedr);
    cmd.AddValue("errorUnit", "", errorUnit);
    cmd.AddValue("errorRate", "", errorRate);
    cmd.Parse(argc, argv);

    RngSeedManager::SetSeed(1);
    RngSeedManager::SetRun(seedr);
    Time::SetResolution(Time::NS);

    NodeContainer termi;
    termi.Create(8);

    NodeContainer routers;
    routers.Create(3);
    
    PointToPointHelper alinks;



    alinks.SetChannelAttribute("Delay", StringValue("2ms"));
    alinks.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    
    PointToPointHelper rout0_1;
    PointToPointHelper rout0_2;


    rout0_1.SetChannelAttribute("Delay", StringValue("5ms"));
    rout0_1.SetDeviceAttribute("DataRate", StringValue("5Mbps"));

    rout0_2.SetChannelAttribute("Delay", StringValue("10ms"));
    rout0_2.SetDeviceAttribute("DataRate", StringValue("3Mbps"));

    NetDeviceContainer acsdevice;

    for(uint32_t i=0;i<3;i++){
        NetDeviceContainer link=alinks.Install(termi.Get(i), routers.Get(0));
        acsdevice.Add(link);
    }

    NetDeviceContainer d_rout0_1=rout0_1.Install(routers.Get(0), routers.Get(1));
    
    
    NetDeviceContainer d_r0r2=rout0_2.Install(routers.Get(0), routers.Get(2));

    for (uint32_t i=3;i<=5;i++){
        NetDeviceContainer link=alinks.Install(termi.Get(i), routers.Get(1));
        acsdevice.Add(link);
    }

    for (uint32_t i=6;i<=7;i++){
        NetDeviceContainer link=alinks.Install(termi.Get(i), routers.Get(2));
        acsdevice.Add(link);
    }
    
    std::string unitStr = "ERROR_UNIT_PACKET";
    if (errorUnit == "Bit") unitStr = "ERROR_UNIT_BIT";
    else if (errorUnit == "Byte") unitStr = "ERROR_UNIT_BYTE";

    NetDeviceContainer allErrorDevices;
    allErrorDevices.Add(acsdevice);
    allErrorDevices.Add(d_rout0_1);
    allErrorDevices.Add(d_r0r2);

    for (uint32_t i = 0; i < allErrorDevices.GetN(); ++i) {
        Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
        em->SetAttribute("ErrorRate", DoubleValue(errorRate));
        em->SetAttribute("ErrorUnit", StringValue(unitStr));
        allErrorDevices.Get(i)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    }

    InternetStackHelper stack;
    stack.Install(termi);
    stack.Install(routers);

    Ipv4AddressHelper address;

    Ipv4InterfaceContainer acsinterfaces;
    Ipv4InterfaceContainer rout0_1_interfaces;
    Ipv4InterfaceContainer rout0_2_interfaces;

    for (uint32_t i=0;i<acsdevice.GetN();i+=2){
        std::ostringstream subnet;
        subnet<<"10.1."<<(i/2+1)<<".0";

        address.SetBase(Ipv4Address(subnet.str().c_str()), "255.255.255.0");

        acsinterfaces.Add(address.Assign(acsdevice.Get(i)));
        address.Assign(acsdevice.Get(i+1));
    }

    address.SetBase("10.1.10.0","255.255.255.0");
    rout0_1_interfaces = address.Assign(d_rout0_1);

    address.SetBase("10.1.20.0","255.255.255.0");
    rout0_2_interfaces = address.Assign(d_r0r2);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    uint32_t packetCount=100;

    int snodes[] = {4,5,6,3};
    int cnodes[] = {7,1,2,0};

    uint16_t port1=8770;
    UdpEchoServerHelper echoServer1(port1);
    ApplicationContainer sinkapp1 = echoServer1.Install(termi.Get(snodes[0]));
    sinkapp1.Start(Seconds(1.0));
    sinkapp1.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient1(acsinterfaces.GetAddress(snodes[0]), port1);
    echoClient1.SetAttribute("MaxPackets", UintegerValue(packetCount));
    echoClient1.SetAttribute("PacketSize", UintegerValue(1024));
    echoClient1.SetAttribute("Interval", TimeValue(Seconds(0.05))); 
    ApplicationContainer app1 = echoClient1.Install(termi.Get(cnodes[0]));
    app1.Start(Seconds(2.0));
    app1.Stop(Seconds(10.0));

    uint16_t port2=8780;
    UdpEchoServerHelper echoServer2(port2);
    ApplicationContainer sinkapp2 = echoServer2.Install(termi.Get(snodes[1]));
    sinkapp2.Start(Seconds(1.0));
    sinkapp2.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient2(acsinterfaces.GetAddress(snodes[1]), port2);
    echoClient2.SetAttribute("MaxPackets", UintegerValue(packetCount));
    echoClient2.SetAttribute("PacketSize", UintegerValue(1024));
    echoClient2.SetAttribute("Interval", TimeValue(Seconds(0.05))); 
    ApplicationContainer app2 = echoClient2.Install(termi.Get(cnodes[1]));
    app2.Start(Seconds(2.0));
    app2.Stop(Seconds(10.0));

    uint16_t port3=8790;


    UdpEchoServerHelper echoServer3(port3);
    ApplicationContainer sinkapp3 = echoServer3.Install(termi.Get(snodes[2]));
    sinkapp3.Start(Seconds(1.0));
    sinkapp3.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient3(acsinterfaces.GetAddress(snodes[2]), port3);
    echoClient3.SetAttribute("MaxPackets", UintegerValue(packetCount));
    echoClient3.SetAttribute("PacketSize", UintegerValue(1024));
    echoClient3.SetAttribute("Interval", TimeValue(Seconds(0.05)));
    
    
    ApplicationContainer app3 = echoClient3.Install(termi.Get(cnodes[2]));
    app3.Start(Seconds(2.0));
    app3.Stop(Seconds(10.0));

    uint16_t port4=8800;


    UdpEchoServerHelper echoServer4(port4);
    ApplicationContainer sinkapp4 = echoServer4.Install(termi.Get(snodes[3]));
    sinkapp4.Start(Seconds(1.0));
    sinkapp4.Stop(Seconds(10.0));



    UdpEchoClientHelper echoClient4(acsinterfaces.GetAddress(snodes[3]), port4);
    echoClient4.SetAttribute("MaxPackets", UintegerValue(packetCount));
    echoClient4.SetAttribute("PacketSize", UintegerValue(1024));
    echoClient4.SetAttribute("Interval", TimeValue(Seconds(0.05))); 
    ApplicationContainer app4=echoClient4.Install(termi.Get(cnodes[3]));
    app4.Start(Seconds(2.0));
    app4.Stop(Seconds(10.0));

    FlowMonitorHelper monitorflow;
    Ptr<FlowMonitor> monitor=monitorflow.InstallAll();

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();

    monitor->CheckForLostPackets();
    std::map<FlowId,FlowMonitor::FlowStats> stats=monitor->GetFlowStats();
    
    uint32_t totaltx=0;

    uint32_t totalrx=0;

    for(const auto& flow:stats){
        totaltx+=flow.second.txPackets;
        totalrx+=flow.second.rxPackets;
    }

    double pdr=0.0;
    if(totaltx>0){
        pdr=(double)totalrx/totaltx;
    }

    std::cout << "PDR_RESULT:" << pdr << std::endl;

    Simulator::Destroy();

    return 0;
}