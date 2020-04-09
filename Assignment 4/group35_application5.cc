#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/tcp-westwood.h"
#include <iostream>


NS_LOG_COMPONENT_DEFINE ("group35");
Ptr<PacketSink> sink;                         
uint64_t lastTotalRx = 0; 
uint32_t tcpSegmentSize = 1000; 
double simulationTime = 50.0;

uint32_t MacTxDropCount = 0, PhyTxDropCount = 0, PhyRxDropCount = 0;

void
MacTxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  MacTxDropCount++;
}

void
PhyTxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  PhyTxDropCount++;
}
void
PhyRxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  PhyRxDropCount++;
}

void setTCPVariant(std::string tcpVariant)
{
  if (tcpVariant.compare ("ns3::TcpWestwood") == 0)
    { 
      // TcpWestwoodPlus is not an actual TypeId name; we need TcpWestwood here
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
      // the default protocol type in ns3::TcpWestwood is WESTWOOD
      Config::SetDefault ("ns3::TcpWestwood::ProtocolType", EnumValue (TcpWestwood::WESTWOOD));
    }
  else if(tcpVariant.compare("ns3::TcpHybla") == 0)
  {
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpHybla::GetTypeId()));
  }
  else
    {
      TypeId tcpTid;
      NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe (tcpVariant, &tcpTid), "TypeId " << tcpVariant << " not found");
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName (tcpVariant)));
    }
}

void Simulator_80211(uint32_t RtsCtsThreshold)
{

  std::string datarate = "200";           //default taken as 100Mbps
  std::string tcpVariant = "TcpWestwood"; //default TcpWestwood
  std::string FragmentationThreshold = "2000";
  bool pcapTracing = true;                // PCAP Tracing is enabled or not
  std::cout<<"Enter TCP variant TcpWestwood/TcpHybla\n";
  std::cin>>tcpVariant;
  setTCPVariant(tcpVariant);



  /* Setting the RTS/CTS threshold */
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue(RtsCtsThreshold));
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", FragmentationThreshold);

  /* Set TCP segment size */
  /*Value given in question */
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (tcpSegmentSize));

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);

  /*Channel Set up*/
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  /*follows the equation of isotropic antenna */
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");

  /* Setup Physical Layer */
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  wifiPhy.SetErrorRateModel ("ns3::YansErrorRateModel");
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                    "DataMode", StringValue ("HtMcs7"),
                    "ControlMode", StringValue ("HtMcs0"));

  NodeContainer nodes;
  nodes.Create (3);
  Ptr<Node> serverNode = nodes.Get (1);
  NodeContainer clientNodes;
  clientNodes = NodeContainer(nodes.Get(0),nodes.Get(2));

  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");

  /*Install server device*/
  NetDeviceContainer serverDevice;
  serverDevice = wifi.Install (wifiPhy, wifiMac, serverNode);

  /*Install client device*/
  NetDeviceContainer clientDevice;
  clientDevice = wifi.Install (wifiPhy, wifiMac, clientNodes);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (200.0, 0.0, 0.0));
  positionAlloc->Add (Vector (400.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  /*setting up protocol stack on client and server devices */
  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  /* Assigning IP adress starting from 12.0.0.0 */
  address.SetBase ("12.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer AccessPointInterface;
  AccessPointInterface = address.Assign (serverDevice);
  Ipv4InterfaceContainer StationInterface;
  StationInterface = address.Assign (clientDevice);

   /* Install TCP Receiver on the access point */
  PacketSinkHelper serverUtil ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 9));
  ApplicationContainer serverApp = serverUtil.Install (serverNode);
  sink = StaticCast<PacketSink> (serverApp.Get (0));

  /* Install TCP/UDP Transmitter on the station */
  OnOffHelper onOffHelper ("ns3::TcpSocketFactory", (InetSocketAddress (AccessPointInterface.GetAddress (0), 9)));
  onOffHelper.SetAttribute ("PacketSize", UintegerValue (tcpSegmentSize));
  onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onOffHelper.SetAttribute ("DataRate", DataRateValue (DataRate (datarate)));
  ApplicationContainer clientApp = onOffHelper.Install (clientNodes);

  double start_time = rand()%5 + 1;
  serverApp.Start (Seconds (start_time));
  clientApp.Start (Seconds (start_time+1));

  if(pcapTracing)
  {
  	wifiPhy.EnablePcap ("Server", serverDevice);
    wifiPhy.EnablePcap ("Station", clientDevice);
  }

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  // Trace Collisions
  Config::ConnectWithoutContext("ns3::WifiNetDevice/Mac/MacTxDrop", MakeCallback(&MacTxDrop));
  Config::ConnectWithoutContext("ns3::WifiNetDevice/Phy/PhyRxDrop", MakeCallback(&PhyRxDrop));
  Config::ConnectWithoutContext("ns3::WifiNetDevice/Phy/PhyTxDrop", MakeCallback(&PhyTxDrop));

  /* Start Simulation */
  Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();
  

  monitor->CheckForLostPackets ();

  /*Check flow stats */
  /* Get stats for transmitted,received packets */
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); i++)
  {
  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
  std::cout << "Flow " << i->first - 2 << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
  std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
  std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
  std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds()) / 1000/1000 << " Mbps\n";
  std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
  std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
  std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds()) / 1000/1000 << " Mbps\n";
  }

  double averageThroughput = ((sink->GetTotalRx () * 8) / (1e6  * simulationTime));
  std::cout << "\nAverage throughtput: " << averageThroughput << " Mbit/s" << std::endl;

  monitor->SerializeToXmlFile("group35.xml", true, true);

  Simulator::Destroy ();
}

int main()
{
	uint32_t thresholds[] = {0,256,512,1024};
	for(int i=0;i<4; i++)
	{
		uint32_t rtsCtsThreshold = thresholds[i];
		std::cout<<"Results with RTS Threshold : "<<rtsCtsThreshold<<std::endl;
		run(rtsCtsThreshold);
		system("tcpdump -nn -tt -r Server-1-0.pcap > server.pcap");
		calculateAnalytics();
		std::cout<<"Collisions : "<<MaTxDropCount+PhyTxDropCount+PhyRxDropCount<<std::endl;
		std::cout<<"---------------------------------------------------------------"<<std::endl;
	}
	return 0;
}
