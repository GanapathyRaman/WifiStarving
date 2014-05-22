
/* Topology
   --------

        +----+               +----+               +----+               +----+
        |AP-A|-------------->|ST-A|               |ST-B|<--------------|AP-B|
        +----+               +----+               +----+               +----+
          0                    7                    14                   21
          |--------------------|--------------------|--------------------|
                   7m                    7m                   7m


        +---------------+------------+-------------------+-------------+
        |    Entity     | Denoted as |    MAC ADDRESS    | IP Address  |
        +---------------+------------+-------------------+-------------+
	| Access Point  |    AP_A    | 00:00:00:00:00:01 | 192.168.1.1 |
        +---------------+------------+-------------------+-------------+
        | Access Point  |    AP_B    | 00:00:00:00:00:03 | 192.168.1.2 |
        +---------------+------------+-------------------+-------------+
        | Station Point |    ST_A    | 00:00:00:00:00:02 | 192.168.1.3 |
        +---------------+------------+-------------------+-------------+
        | Station Point |    ST_B    | 00:00:00:00:00:04 | 192.168.1.4 |
        +---------------+------------+-------------------+-------------+

*/


#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"



using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("1B");

int 
main (int argc, char *argv[]){
  bool verbose = true;
  CommandLine cmd; 
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.Parse (argc,argv);

  if (verbose){
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_FUNCTION); 
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_FUNCTION);
    }

  /* Node creation for Access Point and Station Nodes */ 
  NodeContainer ApNodes; /* For Access Point */  
  ApNodes.Create (2);    
  NodeContainer StNodes; /* For Station node */
  StNodes.Create (2);   
  
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
  StDevices = wifi.Install (Phy, MAC, StNodes.Get(0));  /* Set the MAC address of ST_A to 00:00:00:00:00:02 */

  
  /* Setting up the Network between AP_B and ST_B.
     Assigning the SSID to AP_B and associating ST_B with AP_B.
     Assigning MAC Address to the devices in order */
  Ssid ssidAp_B = Ssid ("ssidAp_B");
  MAC.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssidAp_B)); /* Assigning an SSID to AP_B */
  ApDevices.Add (wifi.Install (Phy, MAC, ApNodes.Get(1)));     /* Set the MAC address of AP_B to 00:00:00:00:00:03 */
  MAC.SetType ("ns3::StaWifiMac","Ssid", SsidValue (ssidAp_B),
               "ActiveProbing", BooleanValue (false));         /* Associating with SSID of AP_B*/
  StDevices.Add (wifi.Install (Phy, MAC, StNodes.Get(1)));     /* Set the MAC address of ST_B to 00:00:00:00:00:04 */


  /* Create Mobility and setting the vector position for Stations and Access Points */ 
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> StAlloc = CreateObject<ListPositionAllocator>();
  StAlloc->Add(Vector(7.0, 0.0, 0.0));  /* Placing ST_A */
  StAlloc->Add(Vector(14.0, 0.0, 0.0)); /* Placing ST_B */

  mobility.SetPositionAllocator(StAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (StNodes);

  Ptr<ListPositionAllocator> ApAlloc = CreateObject<ListPositionAllocator>();
  ApAlloc->Add(Vector(0.0, 0.0, 0.0));  /* Placing AP_A */
  ApAlloc->Add(Vector(21.0, 0.0, 0.0)); /* Placing AP_B */

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
  UdpServerHelper Server1 (10); /* Starting Server at Port 10 */

  ApplicationContainer ServerApp1 = Server1.Install (StNodes.Get (0)); /* ST_A as Server */
  ServerApp1.Start (Seconds (1.0));  /* Starting time of Server */
  ServerApp1.Stop (Seconds (100.0)); /* Stopping time of Server */

  UdpClientHelper Client1 (StInterfaces.GetAddress (0), 10); /* Binding the Client to Sever and opening Port 10 for communication with server */
  Client1.SetAttribute ("MaxPackets", UintegerValue (100000000));
  Client1.SetAttribute ("Interval", TimeValue (Seconds (0.0001)));
  Client1.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer ClientApp1 = Client1.Install (ApNodes.Get (0)); /* AP_A as Client */
  ClientApp1.Start (Seconds (2.0));  /* Starting Client after Server has started */
  ClientApp1.Stop (Seconds (100.0)); /* Stopping time of Client */

  /* Server and Client part. 
     AP_B sends data packets to ST_B */ 	
  UdpServerHelper Server2 (10); /* Starting Server at Port 10 */

  ApplicationContainer ServerApp2 = Server2.Install (StNodes.Get (1)); /* ST_B as Server */
  ServerApp2.Start (Seconds (1.0));  /* Starting time of Server */
  ServerApp2.Stop (Seconds (100.0)); /* Stopping time of Server */

  UdpClientHelper Client2 (StInterfaces.GetAddress (1), 10); /* Binding the Client to Sever and opening Port 10 for communication with server */
  Client2.SetAttribute ("MaxPackets", UintegerValue (100000000));
  Client2.SetAttribute ("Interval", TimeValue (Seconds (0.0001)));
  Client2.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer ClientApp2 = Client2.Install (ApNodes.Get (1)); /* AP_B as Client */
  ClientApp2.Start (Seconds (2.0));  /* Starting Client after Server has started */
  ClientApp2.Stop (Seconds (100.0)); /* Stopping time of Client */


  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (100.0));  /* Stimulation Stopping time */

 /* Logging packet capture in .pcap format */
  Phy.EnablePcap ("Ap_pcap", ApDevices, true);
  Phy.EnablePcap ("St_pcap", StDevices, true);

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

