#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/tcp-westwood.h"
#include <iostream>

NS_LOG_COMPONENT_DEFINE ("group35");

using namespace ns3;

Ptr<PacketSink> sink;                         
uint64_t lastTotalRx = 0; 
uint32_t tcpSegmentSize = 1000; 
double simulationTime = 50.0;

void calculate_avg_bandwidths()
{
  FILE* fp = fopen("stats.txt","r");
  char line[1024];
  uint32_t rtsctsack_size = 14;
  uint32_t tcpack_size = 74;
  uint32_t cts_size = 14;
  uint32_t rts_size = 20;
  uint32_t data_size = tcpSegmentSize + 34;

  std::string rtsctsack = "Acknowledgment";
  std::string tcpack = "ack";
  std::string rts = "Request-To-Send";
  std::string cts = "Clear-To-Send";
  std::string seq = "seq";

  uint32_t rtsctsack_count = 0;
  uint32_t tcpack_count = 0;
  uint32_t rts_count = 0;
  uint32_t cts_count = 0;
  uint32_t data_count = 0;

  /*Reading from the file and incrementing the appropriate counters*/
  while(fgets(line, 1024, fp))
  {
    std::string s = line;
    if(s.find(rtsctsack) != std::string::npos) rtsctsack_count++;
    else if(s.find(tcpack) != std::string::npos and s.find(seq) == std::string::npos) tcpack_count++;
    else if(s.find(rts) != std::string::npos)  rts_count++;
    else if(s.find(cts) != std::string::npos)  cts_count++;
    else if(s.find(seq) != std::string::npos)  data_count++;
  }

  /*Calculation of various bandwidths with the data obtained from the file*/
  double rts_bandwidth = (rts_size*rts_count*8/simulationTime)/1000;
  double cts_bandwidth = (cts_size*cts_count*8/simulationTime)/1000;
  double rtsctsack_bandwidth = (rtsctsack_size*rts_count*8/simulationTime)/1000;
  double tcpack_bandwidth = (tcpack_size*tcpack_count*8/simulationTime)/1000;
  double data_bandwidth = (data_size*tcpack_count*8/simulationTime)/1000;

  printf("Average bandwidth spent in transmitting RTS : %f\n",rts_bandwidth );
  printf("Average bandwidth spent in transmitting CTS : %f\n",cts_bandwidth );
  printf("Average bandwidth spent in transmitting RTS-CTS ACKs : %f\n",rtsctsack_bandwidth );
  printf("Average bandwidth spent in transmitting TCP ACKs : %f\n",tcpack_bandwidth );
  printf("Average bandwidth spent in transmitting TCP segments : %f\n",data_bandwidth ); 

  /*Writing stats to .txt file for plotting graph output*/
  FILE *FilePointer;
  FilePointer = fopen("statsdump.txt","a");
  fprintf(FilePointer,"%lf:",rts_bandwidth);
  fprintf(FilePointer,"%lf:",cts_bandwidth);
  fprintf(FilePointer,"%lf:",rtsctsack_bandwidth);
  fprintf(FilePointer,"%lf:",tcpack_bandwidth);
  fprintf(FilePointer,"%lf:",data_bandwidth);
  fclose(FilePointer); 
}

uint32_t MacTxDropCount = 0, PhyTxDropCount = 0, PhyRxDropCount = 0;

void MacTxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  MacTxDropCount++;
}

void PhyTxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  PhyTxDropCount++;
}
void PhyRxDrop(Ptr<const Packet> p)
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
void GetStats(FlowMonitorHelper &flowHelper,Ptr<FlowMonitor> &flowMonitor)
{
 	/* Check for lost packets */
	flowMonitor->CheckForLostPackets ();  
	/*Check flow stats */
	/* Get stats for transmitted,received packets */
	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier ());
	FlowMonitor::FlowStatsContainer stats = flowMonitor->GetFlowStats ();
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
	std::cout << "\nAverage TCP throughtput obtained: " << averageThroughput << " Mbit/s" << std::endl;
	FILE *FilePointer;
	FilePointer = fopen("statsdump.txt","a");
	fprintf(FilePointer,"%lf:",averageThroughput);
	fclose(FilePointer);

	flowMonitor->SerializeToXmlFile("group35.xml", true, true);
}
void Simulator_80211(uint32_t RtsCtsThreshold)
{

  std::string datarate = "200kbps";           //default taken as 200Kbps
  bool pcapTracing = true;                // PCAP Tracing is enabled or not

  /* Setting the RTS/CTS threshold */
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue(RtsCtsThreshold));
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("1500"));

  /* Set TCP segment size */
  /*Value given in question */
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (tcpSegmentSize));

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);

  /*Channel Set up*/
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  /*the propogation loss follows random distribution*/
  wifiChannel.AddPropagationLoss ("ns3::RandomPropagationLossModel");

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
  OnOffHelper clientAppHelper ("ns3::TcpSocketFactory", (InetSocketAddress (AccessPointInterface.GetAddress (0), 9)));
  clientAppHelper.SetAttribute ("PacketSize", UintegerValue (tcpSegmentSize));
  clientAppHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientAppHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientAppHelper.SetAttribute ("DataRate", DataRateValue (DataRate (datarate)));
  ApplicationContainer clientApp = clientAppHelper.Install (clientNodes);

  double start_time = rand()%5 + 1;
  serverApp.Start (Seconds (start_time));
  clientApp.Start (Seconds (start_time+1));

  if(pcapTracing)
  {
    wifiPhy.EnablePcap ("Server", serverDevice);
    wifiPhy.EnablePcap ("Station", clientDevice);
  }

  // Trace Collisions
  Config::ConnectWithoutContext("ns3::WifiNetDevice/Mac/MacTxDrop", MakeCallback(&MacTxDrop));
  Config::ConnectWithoutContext("ns3::WifiNetDevice/Phy/PhyRxDrop", MakeCallback(&PhyRxDrop));
  Config::ConnectWithoutContext("ns3::WifiNetDevice/Phy/PhyTxDrop", MakeCallback(&PhyTxDrop));	
  
  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();

  

  /* Start Simulation */
  Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();
  GetStats(flowHelper,flowMonitor);  

  Simulator::Destroy ();
}

int main()
{
  std::string tcpVariant; //default TcpWestwood
    std::cout<<"Enter TCP variant TcpWestwood/TcpHybla\n";
    std::cin>>tcpVariant;
    tcpVariant = "ns3::"+tcpVariant;
  	setTCPVariant(tcpVariant);
  	FILE *FilePointer;
  	FilePointer = fopen("statsdump.txt","w");
  	fclose(FilePointer);

  	uint32_t rtsCtsThresholds[] = {0,256,512,1024};
  	for(int i = 0; i < 4; i++)
  	{
	    std::cout<<"Running simulation with RTS threshold : "<<rtsCtsThresholds[i]<<"\n";
	    Simulator_80211(rtsCtsThresholds[i]);
	    system("tcpdump -nn -tt -r Server-1-0.pcap > stats.txt");
	    calculate_avg_bandwidths();
	    int collisions = MacTxDropCount+PhyTxDropCount+PhyRxDropCount;
	    std::cout<<"Total number of packets lost due to collisions : "<<collisions<<"\n";
	    FilePointer = fopen("statsdump.txt","a");
	    fprintf(FilePointer,"%d\n",collisions);
	    fclose(FilePointer);

	    std::cout<<"---------------------------------------------------------------"<<"\n";
	 }
	 system("python3 ./scratch/plot_graph.py");

  return 0;
}

