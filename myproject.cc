/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Universidad de la República - Uruguay
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Marcelo Guerrero <M.L.GuerreroViveros@student.tudelft.nl> and Renzo Arreaza <?@student.tudelft.nl>
 */

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/stats-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("wifi-ratecontrol-network");

class Statistics
{
public:
  
  void AdvancePosition (Ptr<Node> node, int stepsSize, int stepsTime, int steps);
  void RxCallback (std::string path, Ptr<const Packet> packet, const Address &from);
  void FailedPacketCallback (std::string path, Mac48Address dest);
  void RateCallback (std::string path, uint64_t rate, Mac48Address dest);
  
  uint32_t bytesTotal = 0;
  uint32_t indexTime = 1;
  int indexStep = 1;
  uint32_t failedPackets = 0;


  Gnuplot2dDataset outputDistanceThroughput{"Received Throughput"};
  Gnuplot2dDataset outputTimeRate{"Data Rate - MCS"};
  Gnuplot2dDataset outputTimePosition{"STA position"};
  Gnuplot2dDataset outputFailedPacket{"AP - Failed transmission"};
};

void
Statistics::AdvancePosition (Ptr<Node> node, int stepsSize, int stepsTime, int steps)
{

  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
  Vector pos = mobility->GetPosition();
  
  double throughput = ((bytesTotal * 8.0) / (1000000 * stepsTime));
  bytesTotal = 0;

  outputDistanceThroughput.Add (indexTime * stepsTime, throughput);
  outputTimePosition.Add (indexTime * stepsTime, pos.x);
  indexTime += 1;
  outputFailedPacket.Add (indexTime * stepsTime, failedPackets);
  failedPackets = 0;

  if (indexStep < steps)
  {

    pos.x += stepsSize;
    indexStep += 1;
    mobility->SetPosition (pos);
    Simulator::Schedule (Seconds (stepsTime), &Statistics::AdvancePosition, this, node, stepsSize, stepsTime, steps);
  }
  else
  {
    pos.x -= stepsSize;
    indexStep += 1;
    mobility->SetPosition (pos);
    Simulator::Schedule (Seconds (stepsTime), &Statistics::AdvancePosition, this, node, stepsSize, stepsTime, steps);
  }

}

void
Statistics::RxCallback (std::string path, Ptr<const Packet> packet, const Address &from)
{
  bytesTotal += packet->GetSize ();
}

void
Statistics::FailedPacketCallback (std::string path, Mac48Address dest)
{
  failedPackets += 1;
}


void Statistics::RateCallback (std::string path, uint64_t rate, Mac48Address dest)
{
  outputTimeRate.Add ((Simulator::Now ()).GetSeconds (), rate/1000000);
}


void
GetPosition (Ptr<Node> node, Gnuplot3dDataset outputPosUsers)
{
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
  Vector pos = mobility->GetPosition();
  outputPosUsers.Add(pos.x, pos.y, pos.z);
  //std::cout << pos.x << pos.y << pos.z << std::endl;
}


int main (int argc, char *argv[])
{

  int payLoadSize = 1420;
  double initialDistance = 1.0;
  
  bool shortGuardInterval = false;
  int spatialStreams = 1;
  uint32_t rtsThreshold = 65535;
  std::string apRateControl = "ns3::MinstrelHtWifiManager";
  uint32_t chWidth = 20;
  int steps = 40;

  int stepsSize = 1;
  int stepsTime = 1;

  int indexPos = 0;

  // Additional users
  bool additionalUsers = false;
  int numberUsers = 4;
  Gnuplot3dDataset outputPosUsers{"Devices"};

  CommandLine cmd;
  cmd.AddValue ("initialDistance", "Initial distance of the STA", initialDistance);
  cmd.AddValue ("shortGuardInterval", "Enable Short Guard Interval", shortGuardInterval);
  cmd.AddValue ("spatialStreams", "Number of Spatial Streams", spatialStreams);
  cmd.AddValue ("rtsThreshold", "RTS threshold", rtsThreshold);
  cmd.AddValue ("apRateControl", "Rate Control Algorithm of the AP", apRateControl);
  cmd.AddValue ("channelWidth", "Channel width of all the stations", chWidth);
  cmd.AddValue ("steps", "How many different distances to try", steps);
  cmd.AddValue ("stepsTime", "Time on each step", stepsTime);
  cmd.AddValue ("stepsSize", "Distance between steps", stepsSize);
  cmd.AddValue ("additionalUsers", "4 Additional users (default false)", additionalUsers);
  cmd.Parse (argc, argv);


  int simulationTime = stepsTime * (2 * steps - 1);

  //Create AP
  NodeContainer wifiApNode;
  wifiApNode.Create(1);

  //Create STA
  NodeContainer wifiStaNode;
 
  if (additionalUsers




) wifiStaNode.Create(1 + numberUsers);
  else wifiStaNode.Create(1);

  //Create channel helper and phy helper. Create channel.
  //Log distance model - 5.15 GHz - Reference Loss 46.6777 dB at reference distance of 1 m.
  //Propagation delay - Speed of light
  //Error model - NistErrorRateModel
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());

  //Enable pcap tracing - Radiotap and Prism tracing for 802.11
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
 
  //Enable short guard interval
  wifiPhy.Set ("ShortGuardEnabled", BooleanValue (shortGuardInterval));

  //Set number of spatial streams
  wifiPhy.Set ("Antennas", UintegerValue(spatialStreams));
  wifiPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (spatialStreams));
  wifiPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (spatialStreams));

  //Enable GreenField mode - Hardcoded
  //wifiPhy.Set ("GreenfieldEnabled", BooleanValue(true));

  //Create MAC and WiFi helper and SSID
  WifiMacHelper wifiMac;
  WifiHelper wifi;
  Ssid ssid = Ssid ("TSS1");

  //Set 802.11n standard - 5 GHz
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);

  //Configure STA node - Hardcoded Rate Control Algorithm
  wifi.SetRemoteStationManager ("ns3::MinstrelHtWifiManager", "RtsCtsThreshold", UintegerValue (rtsThreshold), "PrintStats", BooleanValue(true));

  //Disable Active Probing
  wifiMac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssid),
                   "ActiveProbing", BooleanValue(false));

  NetDeviceContainer staDevice = wifi.Install (wifiPhy, wifiMac, wifiStaNode);

  //Configure AP node 
  wifi.SetRemoteStationManager (apRateControl, "RtsCtsThreshold", UintegerValue (rtsThreshold));

  wifiMac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevice = wifi.Install (wifiPhy, wifiMac, wifiApNode);

  // Set channel width
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/ChannelWidth", UintegerValue (chWidth));
  

  // mobility.
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  positionAlloc->Add (Vector (0.0, 0.0, 3.0));
  positionAlloc->Add (Vector (initialDistance, 0.0, 0.0));

  if (additionalUsers)
  {
    for (double y = -5.0; indexPos < numberUsers; y+=10.0)
    {
      for (double x = -5.0; (indexPos < numberUsers) and (x <= 5.0); x+=10.0)
      {
        positionAlloc->Add (Vector (x, y, 0.0));
        indexPos += 1;
      }
    }
  }

  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (wifiApNode);
  mobility.Install (wifiStaNode.Get(0));

  GetPosition (wifiApNode.Get(0), outputPosUsers);
  GetPosition (wifiStaNode.Get(0), outputPosUsers);
  //Mandatory for 3D plots after x
  outputPosUsers.AddEmptyLine ();

  // Mobility for additional users
  if (additionalUsers)
  {
    for (int i = 1; i <= numberUsers; i++)
    {
      mobility.Install (wifiStaNode.Get(i));
      GetPosition (wifiStaNode.Get(i), outputPosUsers);
      outputPosUsers.AddEmptyLine ();
    }
  }

  //Simulator Schedule
  Statistics result;
  Simulator::Schedule (Seconds (0.5 + stepsTime), &Statistics::AdvancePosition, &result, wifiStaNode.Get (0), stepsSize, stepsTime, steps);
  

  /* Internet stack*/
  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNode);

  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer staNodeInterface;
  Ipv4InterfaceContainer apNodeInterface;

  apNodeInterface = address.Assign (apDevice);
  staNodeInterface = address.Assign (staDevice);

  //UDP flow Sink - AP sends packets to the STA
  uint16_t port = 10;
  PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (staNodeInterface.GetAddress(0), port));
  ApplicationContainer apps_sink = sink.Install (wifiStaNode.Get (0));

  OnOffHelper onoff ("ns3::UdpSocketFactory", InetSocketAddress (staNodeInterface.GetAddress(0), port));
  onoff.SetConstantRate (DataRate ("100Mb/s"), payLoadSize);
  onoff.SetAttribute ("StartTime", TimeValue (Seconds (0.5)));
  onoff.SetAttribute ("StopTime", TimeValue (Seconds (simulationTime)));
  ApplicationContainer apps_source = onoff.Install (wifiApNode.Get (0));

  apps_sink.Start (Seconds (0.5));
  apps_sink.Stop (Seconds (simulationTime));

  if (additionalUsers)
  {

  //UDP flow Sink - AP sends packets to additional STAs
    uint16_t port1 = 11;
    PacketSinkHelper sink1 ("ns3::UdpSocketFactory", InetSocketAddress (staNodeInterface.GetAddress(1), port1));
    ApplicationContainer apps_sink1 = sink1.Install (wifiStaNode.Get (1));

    OnOffHelper onoff1 ("ns3::UdpSocketFactory", InetSocketAddress (staNodeInterface.GetAddress(1), port1));
    onoff1.SetConstantRate (DataRate ("100Mb/s"), payLoadSize);
    onoff1.SetAttribute ("StartTime", TimeValue (Seconds (0.5)));
    onoff1.SetAttribute ("StopTime", TimeValue (Seconds (simulationTime)));
    ApplicationContainer apps_source1 = onoff1.Install (wifiApNode.Get (0));

    apps_sink1.Start (Seconds (0.5));
    apps_sink1.Stop (Seconds (simulationTime));


    uint16_t port2 = 12;
    PacketSinkHelper sink2 ("ns3::UdpSocketFactory", InetSocketAddress (staNodeInterface.GetAddress(2), port2));
    ApplicationContainer apps_sink2 = sink2.Install (wifiStaNode.Get (2));

    OnOffHelper onoff2 ("ns3::UdpSocketFactory", InetSocketAddress (staNodeInterface.GetAddress(2), port2));
    onoff2.SetConstantRate (DataRate ("100Mb/s"), payLoadSize);
    onoff2.SetAttribute ("StartTime", TimeValue (Seconds (0.5)));
    onoff2.SetAttribute ("StopTime", TimeValue (Seconds (simulationTime)));
    ApplicationContainer apps_source2 = onoff2.Install (wifiApNode.Get (0));

    apps_sink2.Start (Seconds (0.5));
    apps_sink2.Stop (Seconds (simulationTime));


    uint16_t port3 = 13;
    PacketSinkHelper sink3 ("ns3::UdpSocketFactory", InetSocketAddress (staNodeInterface.GetAddress(3), port3));
    ApplicationContainer apps_sink3 = sink3.Install (wifiStaNode.Get (3));

    OnOffHelper onoff3 ("ns3::UdpSocketFactory", InetSocketAddress (staNodeInterface.GetAddress(3), port3));
    onoff3.SetConstantRate (DataRate ("100Mb/s"), payLoadSize);
    onoff3.SetAttribute ("StartTime", TimeValue (Seconds (0.5)));
    onoff3.SetAttribute ("StopTime", TimeValue (Seconds (simulationTime)));
    ApplicationContainer apps_source3 = onoff3.Install (wifiApNode.Get (0));

    apps_sink3.Start (Seconds (0.5));
    apps_sink3.Stop (Seconds (simulationTime));


    uint16_t port4 = 14;
    PacketSinkHelper sink4 ("ns3::UdpSocketFactory", InetSocketAddress (staNodeInterface.GetAddress(4), port4));
    ApplicationContainer apps_sink4 = sink4.Install (wifiStaNode.Get (4));

    OnOffHelper onoff4 ("ns3::UdpSocketFactory", InetSocketAddress (staNodeInterface.GetAddress(4), port4));
    onoff4.SetConstantRate (DataRate ("100Mb/s"), payLoadSize);
    onoff4.SetAttribute ("StartTime", TimeValue (Seconds (0.5)));
    onoff4.SetAttribute ("StopTime", TimeValue (Seconds (simulationTime)));
    ApplicationContainer apps_source4 = onoff4.Install (wifiApNode.Get (0));

    apps_sink4.Start (Seconds (0.5));
    apps_sink4.Stop (Seconds (simulationTime));
    
  }


  //Pcap - Capture Frames
  wifiPhy.EnablePcap ("myproject", apDevice.Get (0));

  //Register packet receptions to calculate throughput
  Config::Connect ("/NodeList/1/ApplicationList/*/$ns3::PacketSink/Rx",
                   MakeCallback (&Statistics::RxCallback, &result));

  //Register every change of rate
  Config::Connect ("/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/RemoteStationManager/$" + apRateControl + "/RateChange",
                   MakeCallback (&Statistics::RateCallback, &result));

  //Register failed transmission of packets
  Config::Connect ("/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/RemoteStationManager/$" + apRateControl + "/MacTxDataFailed",
                   MakeCallback (&Statistics::FailedPacketCallback, &result));

  Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();


  //GnuPlot
  std::ofstream outfile1 ("DistanceThroughput.plt");
  Gnuplot gnuplot1 = Gnuplot ("DistanceThroughput.eps", "Throughput");
  gnuplot1.SetTerminal ("post eps color enhanced");
  gnuplot1.SetLegend ("time (sec)", "Throughput (Mb/s)");
  gnuplot1.SetTitle ("Throughput (AP to STA) vs Time");
  gnuplot1.AppendExtra ("set autoscale y");
  gnuplot1.AppendExtra ("set autoscale x");
  gnuplot1.AddDataset (result.outputDistanceThroughput);
  gnuplot1.GenerateOutput (outfile1);

  std::ofstream outfile2 ("TimeRate.plt");
  Gnuplot gnuplot2 = Gnuplot ("TimeRate.eps", "Rate");
  gnuplot2.SetTerminal ("post eps color enhanced");
  gnuplot2.SetLegend ("Time (sec)", "Rate (Mb/s)");
  gnuplot2.SetTitle ("Rate (AP) vs Time");
  gnuplot2.AppendExtra ("set autoscale y");
  gnuplot2.AppendExtra ("set autoscale x");
  gnuplot2.AddDataset (result.outputTimeRate);
  gnuplot2.GenerateOutput (outfile2);

  std::ofstream outfile3 ("TimePosition.plt");
  Gnuplot gnuplot3 = Gnuplot ("TimePosition.eps", "Position");
  gnuplot3.SetTerminal ("post eps color enhanced");
  gnuplot3.SetLegend ("Time (sec)", "Position_x (meters)");
  gnuplot3.SetTitle ("Positon_x (STA) vs Time");
  gnuplot3.AppendExtra ("set autoscale y");
  gnuplot3.AppendExtra ("set autoscale x");
  gnuplot3.AddDataset (result.outputTimePosition);
  gnuplot3.GenerateOutput (outfile3);

  std::ofstream outfile4 ("FailedPackets.plt");
  Gnuplot gnuplot4 = Gnuplot ("FailedPackets.eps", "Position");
  gnuplot4.SetTerminal ("post eps color enhanced");
  gnuplot4.SetLegend ("Time (sec)", "# Frames");
  gnuplot4.SetTitle ("Failed Frames vs Time");
  gnuplot4.AppendExtra ("set autoscale y");
  gnuplot4.AppendExtra ("set autoscale x");
  gnuplot4.AddDataset (result.outputFailedPacket);
  gnuplot4.GenerateOutput (outfile4);

  std::ofstream outfile5 ("PosDevices.plt");
  Gnuplot gnuplot5 = Gnuplot ("PosDevices.png");
  gnuplot5.SetTitle ("Position Devices");
  gnuplot5.SetTerminal ("png");
  gnuplot5.AppendExtra ("set ticslevel 0");
  gnuplot5.AppendExtra ("set xlabel \"X (m)\"");
  gnuplot5.AppendExtra ("set ylabel \"Y (m)\"");
  gnuplot5.AppendExtra ("set zlabel \"Z (m)\"");
  gnuplot5.AppendExtra ("set xrange [-15:+15]");
  gnuplot5.AppendExtra ("set yrange [-15:+15]");

  gnuplot5.AddDataset (outputPosUsers);
  gnuplot5.GenerateOutput (outfile5);

  Simulator::Destroy ();
}
