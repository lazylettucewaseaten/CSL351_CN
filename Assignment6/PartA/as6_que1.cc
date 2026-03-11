#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
using namespace ns3;

int main(int argc, char* argv[]){
    Time::SetResolution(Time::NS);

    NodeContainer termi;
    termi.Create(8);

    NodeContainer routers;
    routers.Create(3);

    
    PointToPointHelper alinks;
    alinks.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    alinks.SetChannelAttribute("Delay", StringValue("2ms"));

    PointToPointHelper rout0_1;
    rout0_1.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    rout0_1.SetChannelAttribute("Delay", StringValue("5ms"));

    PointToPointHelper rout0_2;
    rout0_2.SetDeviceAttribute("DataRate", StringValue("3Mbps"));
    rout0_2.SetChannelAttribute("Delay", StringValue("10ms"));


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

    InternetStackHelper stack;
    stack.Install(termi);
    stack.Install(routers);

    Ipv4AddressHelper address;

    Ipv4InterfaceContainer acsinterfaces;
    Ipv4InterfaceContainer rout0_1_interfaces;
    Ipv4InterfaceContainer rout0_2_interfaces;

    for (uint32_t i=0;i<acsdevice.GetN();i+=2)
    {
        std::ostringstream subnet;
        subnet<<"10.1."<<(i/2+1)<<".0";

        address.SetBase(Ipv4Address(subnet.str().c_str()), "255.255.255.0");

        acsinterfaces.Add(address.Assign(acsdevice.Get(i)));
        address.Assign(acsdevice.Get(i + 1));
    }

    address.SetBase("10.1.10.0","255.255.255.0");
    rout0_1_interfaces = address.Assign(d_rout0_1);

    address.SetBase("10.1.20.0","255.255.255.0");
    rout0_2_interfaces = address.Assign(d_r0r2);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Ptr<OutputStreamWrapper> routingStream =
    Create<OutputStreamWrapper>("routing-tables.txt", std::ios::out);

    routers.Get(0)->GetObject<Ipv4>()->GetRoutingProtocol()->PrintRoutingTable(routingStream);

    routers.Get(1)->GetObject<Ipv4>()->GetRoutingProtocol()->PrintRoutingTable(routingStream);

    routers.Get(2)->GetObject<Ipv4>()->GetRoutingProtocol()->PrintRoutingTable(routingStream);

    alinks.EnablePcapAll("partA-access");
    rout0_1.EnablePcapAll("partA-rout0_1");
    rout0_2.EnablePcapAll("partA-r0r2");

    uint16_t port1 = 8770;
    Address sinkaddr1(InetSocketAddress(acsinterfaces.GetAddress(7), port1));

    PacketSinkHelper sink1("ns3::TcpSocketFactory", sinkaddr1);
    ApplicationContainer sinkapp1 = sink1.Install(termi.Get(4));
    sinkapp1.Start(Seconds(1.0));
    sinkapp1.Stop(Seconds(10.0));

    OnOffHelper client1("ns3::TcpSocketFactory", sinkaddr1);
    client1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    client1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    client1.SetAttribute("DataRate", DataRateValue(DataRate("2Mbps")));
    client1.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer app1 = client1.Install(termi.Get(7));
    app1.Start(Seconds(2.0));
    app1.Stop(Seconds(10.0));

    uint16_t port2 = 8780;
    Address sinkaddr2(InetSocketAddress(acsinterfaces.GetAddress(5), port2));



    PacketSinkHelper sink2("ns3::UdpSocketFactory", sinkaddr2);
    ApplicationContainer sinkapp2 = sink2.Install(termi.Get(5));
    sinkapp2.Start(Seconds(1.0));
    sinkapp2.Stop(Seconds(10.0));

    OnOffHelper client2("ns3::UdpSocketFactory", sinkaddr2);
    client2.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    client2.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    client2.SetAttribute("DataRate", DataRateValue(DataRate("2Mbps")));
    client2.SetAttribute("PacketSize", UintegerValue(1024));
    
    

    ApplicationContainer app2 = client2.Install(termi.Get(1));
    app2.Start(Seconds(2.0));
    app2.Stop(Seconds(10.0));

    uint16_t port3=8790;
    
    
    Address sinkaddr3(InetSocketAddress(acsinterfaces.GetAddress(6), port3));

    PacketSinkHelper sink3("ns3::TcpSocketFactory", sinkaddr3);
    ApplicationContainer sinkapp3 = sink3.Install(termi.Get(6));
    sinkapp3.Start(Seconds(1.0));
    sinkapp3.Stop(Seconds(10.0));



    OnOffHelper client3("ns3::TcpSocketFactory", sinkaddr3);
    client3.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    client3.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    client3.SetAttribute("DataRate", DataRateValue(DataRate("1Mbps")));
    client3.SetAttribute("PacketSize", UintegerValue(1024));
    
    

    ApplicationContainer app3 = client3.Install(termi.Get(2));
    app3.Start(Seconds(2.0));
    app3.Stop(Seconds(10.0));

    uint16_t port4 = 8800;
    Address sinkaddr4(InetSocketAddress(acsinterfaces.GetAddress(3), port4));

    
    
    PacketSinkHelper sink4("ns3::TcpSocketFactory", sinkaddr4);
    ApplicationContainer sinkapp4 = sink4.Install(termi.Get(3));
    
    sinkapp4.Start(Seconds(1.0));
    
    sinkapp4.Stop(Seconds(10.0));

    OnOffHelper client4("ns3::TcpSocketFactory", sinkaddr4);
    client4.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    client4.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    client4.SetAttribute("DataRate", DataRateValue(DataRate("3Mbps")));
    client4.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer app4=client4.Install(termi.Get(0));
    app4.Start(Seconds(2.0));
    
    
    app4.Stop(Seconds(10.0));

   
    FlowMonitorHelper monitorflow;
    Ptr<FlowMonitor> monitor=monitorflow.InstallAll();

 
    AnimationInterface anim("partA.xml");

    
    Simulator::Stop(Seconds(10.0));
    Simulator::Run();


   

    monitor->CheckForLostPackets();

    Ptr<Ipv4FlowClassifier> classifier=DynamicCast<Ipv4FlowClassifier>(monitorflow.GetClassifier());

    std::map<FlowId, FlowMonitor::FlowStats> stats=monitor->GetFlowStats();
    
    
    std::ofstream outFile;
    outFile.open("flowresults.dat");
    outFile << "FlowID Throughput(Kbps) Goodput(Kbps) LostPackets\n";

    for(const auto& flow:stats){

        FlowId flowid=flow.first;
        FlowMonitor::FlowStats fstats=flow.second;


        Ipv4FlowClassifier::FiveTuple t=classifier->FindFlow(flowid);
        if(t.destinationPort==8770|| t.destinationPort == 8780 || t.destinationPort == 8790|| t.destinationPort==8800){
            
            
            double duration=fstats.timeLastRxPacket.GetSeconds() - fstats.timeFirstTxPacket.GetSeconds();
            if(duration<=0) continue;
            double throughput=(fstats.rxBytes*8.0)/duration/1024;
            double payloadSize;
            if(t.destinationPort == 8770 || t.destinationPort == 8790 || t.destinationPort == 8800) {
                payloadSize=984; 
            }
            else{
                payloadSize=996;
            }
            double goodput=(fstats.rxPackets*payloadSize*8.0)/duration/1024;
            uint32_t lostPackets=fstats.lostPackets;

            std::cout<<"flow id: "<<flowid<<std::endl;
            
            
            std::cout<<"soruce: "<<t.sourceAddress<<" -> destination: " << t.destinationAddress << std::endl;
            std::cout<<"throughput: "<<throughput<<" Kbps" << std::endl;
            std::cout<<"goodput: "<<goodput<<" Kbps" << std::endl;
            std::cout<<"lost packets: "<<lostPackets << std::endl;
            std::cout<<"============================" << std::endl;

            outFile<<flowid<<" "<<throughput<<" "<<goodput<<" "<<lostPackets << "\n";
        }
    }

    

    outFile.close();
    Simulator::Destroy();



    return 0;
}

