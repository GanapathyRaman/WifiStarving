/* Topology
   --------

  --- 3.5        +----+ 
   |             |AP-A|
   |            /+----+\
   |           /   |     \
   |          /    |       \
   |         /     |         \
   |        /      |           \
   |       +----+  +----+       +----+
   |       |ST-1|  |ST-2| ..... |ST-N|
  --- 0    +----+  +----+       +----+



          0                            7          
         |-----------------------------|
                      7m                    


        +---------------+------------+-------------------+-------------+
        |    Entity     | Denoted as |    MAC ADDRESS    | IP Address  |
        +---------------+------------+-------------------+-------------+
	| Access Point  |    AP_A    | 00:00:00:00:00:01 | 192.168.1.1 |
        +---------------+------------+-------------------+-------------+
        | Station Point |    ST_1    | 00:00:00:00:00:02 | 192.168.1.2 |
        +---------------+------------+-------------------+-------------+
        | Station Point |    ST_2    | 00:00:00:00:00:03 | 192.168.1.3 |
        +---------------+------------+-------------------+-------------+
                                         :
                                         :
                                         :
        +---------------+------------+-----------------------+-----------------+
        | Station Point |    ST_N    | 00:00:00:00:00:0(N+1) | 192.168.1.(N+1) |
        +---------------+------------+-----------------------+-----------------+

*/


#include <iostream>
#include <string.h>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"
#include "ns3/gnuplot.h"


using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("3A");


int 
main (int argc, char *argv[]){
  bool verbose = true;
  uint32_t i = 0;
  uint32_t nSt = 2;
  CommandLine cmd; 
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue("nSt", "Number of Stations", nSt);
  cmd.Parse (argc,argv);

  if (verbose){
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_FUNCTION); 
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_FUNCTION);
    }

  /* Node creation for Access Point and Station Nodes */ 
  NodeContainer ApNodes; /* For Access Point */  
  ApNodes.Create (1);    
  NodeContainer StNodes; /* For Station node */
  StNodes.Create (nSt);   
  
  /* Creating Channel and Phy */ 
  YansWifiChannelHelper Channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper Phy = YansWifiPhyHelper::Default (); 

  /* Setting maximum transmission range to 10m. 
     and Stations are within the range of coverage */
  Channel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(10.0)); 
  Phy.SetChannel (Channel.Create ()); 
  
  /* Creating Wifi helper setting the standard to 802.11b.
     Setting the datamode and control mode to DsssRate11Mbps */ 
  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("0")); 
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue("DsssRate11Mbps"), "ControlMode",StringValue("DsssRate11Mbps")); 

  /* Creating non-QOS Wifi MAC layers */
  NqosWifiMacHelper MAC = NqosWifiMacHelper::Default ();

 /* Creating NetDevice Containers */
  NetDeviceContainer ApDevices; 
  NetDeviceContainer StDevices;
  
  /* Setting up the Network between AP_A and ST_A.
     Assigning the SSID to AP_A and associating ST_A with AP_A.
     Assigning MAC Address to the devices in order */
  Ssid ssidAp_A = Ssid ("ssidAp_A");
  MAC.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssidAp_A)); /* Assigning an SSID to AP_A */
  ApDevices = wifi.Install (Phy, MAC, ApNodes.Get(0));         /* Set the MAC address of AP_A to 00:00:00:00:00:01 */

    MAC.SetType ("ns3::StaWifiMac","Ssid", SsidValue (ssidAp_A),
                 "ActiveProbing", BooleanValue (false));  /* Associating with SSID of AP_A */
    StDevices = wifi.Install (Phy, MAC, StNodes.Get(0));  /* Set the MAC address of ST_A to 00:00:00:00:00:0i */

    for(i=1; i<nSt; i++)
    {
     MAC.SetType ("ns3::StaWifiMac","Ssid", SsidValue (ssidAp_A),
                 "ActiveProbing", BooleanValue (false));  /* Associating with SSID of AP_A */
     StDevices.Add (wifi.Install (Phy, MAC, StNodes.Get(i)));
    }

  /* Create Mobility and setting the vector position for Stations and Access Points */ 
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> StAlloc = CreateObject<ListPositionAllocator>();
  double Pos = 0.0;
  for(i=0; i<nSt; i++)
  {
    StAlloc->Add(Vector(Pos, 0.0, 0.0));  /* Placing ST_i */
    Pos = Pos + (7.0/nSt);
  }

  mobility.SetPositionAllocator(StAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (StNodes);

  Ptr<ListPositionAllocator> ApAlloc = CreateObject<ListPositionAllocator>();
  ApAlloc->Add(Vector(3.5, 3.5, 0.0));  /* Placing AP_A */

  mobility.SetPositionAllocator(ApAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (ApNodes);
  
  /* Protocol configuration */ 
  InternetStackHelper stack;
  stack.Install (ApNodes);
  stack.Install (StNodes);
 
  /* Setting the base address to 192.168.1.0 */
  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");
 
  /* Setting IP Address to AP's and Stations */ 
  Ipv4InterfaceContainer ApInterfaces;
  ApInterfaces.Add (address.Assign (ApDevices));
  Ipv4InterfaceContainer StInterfaces;
  StInterfaces.Add (address.Assign (StDevices)); 
 
  
  /* Server and Client part. 
     AP_A sends data packets to ST_A */ 
  UdpServerHelper Server (10); /* Starting Server at Port 10 */

  ApplicationContainer ServerApp = Server.Install (ApNodes.Get (0)); /* ST_A as Server */
  ServerApp.Start (Seconds (1.0));  /* Starting time of Server */
  ServerApp.Stop (Seconds (100.0)); /* Stopping time of Server */

  /* Client */

  for(i=0; i<nSt; i++)
  {
    UdpClientHelper Client (ApInterfaces.GetAddress (0), 10); /* Binding the Client to Sever and opening Port 10 for communication with server */
    Client.SetAttribute ("MaxPackets", UintegerValue (100000000));
    Client.SetAttribute ("Interval", TimeValue (Seconds (0.0001)));
    Client.SetAttribute ("PacketSize", UintegerValue (1024));

    ApplicationContainer ClientApp = Client.Install (StNodes.Get (i)); /* ST_i as Client */
    ClientApp.Start (Seconds (2.0));  /* Starting Client after Server has started */
    ClientApp.Stop (Seconds (100.0)); /* Stopping time of Client */
  }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (100.0));  /* Stimulation Stopping time */
  
  static char wrdir[300];
  static char wrdirAp[300];
  static char wrdirSt[300];

  sprintf(wrdir,"pcap/3A/%d",nSt);
  
  sprintf(wrdirAp,"%s/Ap_pcap",wrdir);
  sprintf(wrdirSt,"%s/St_pcap",wrdir);
  

  /* Logging packet capture in .pcap format */
  Phy.EnablePcap (wrdirAp, ApDevices, true);
  Phy.EnablePcap (wrdirSt, StDevices, true);


  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

