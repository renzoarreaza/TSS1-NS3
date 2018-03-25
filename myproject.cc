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
  
  void AdvancePosition (Ptr<Node> node, int stepsSize, int stepsTime);
  void RxCallback (std::string path, Ptr<const Packet> packet, const Address &from);
  void FailedPacketCallback (std::string path, Mac48Address dest);
  void RateCallback (std::string path, uint64_t rate, Mac48Address dest);
  
  uint32_t bytesTotal = 0;
  uint32_t indexTime = 1;
  uint32_t failedPackets = 0;


  Gnuplot2dDataset outputDistanceThroughput{"Received Throughput"};
  Gnuplot2dDataset outputTimeRate{"Data Rate - MCS"};
  Gnuplot2dDataset outputTimePosition{"STA position"};
  Gnuplot2dDataset outputFailedPacket{"AP - Failed transmission"};
};

void
Statistics::AdvancePosition (Ptr<Node> node, int stepsSize, int stepsTime)
{

  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
  Vector pos = mobility->GetPosition();
  
  double throughput = ((bytesTotal * 8.0) / (1000000 * stepsTime));
  bytesTotal = 0;

  outputDistanceThroughput.Add (pos.x, throughput);
  outputTimePosition.Add (indexTime * stepsTime, pos.x);
  indexTime += 1;
  outputFailedPacket.Add (pos.x, failedPackets);
  failedPackets = 0;

  pos.x += stepsSize;
  mobility->SetPosition (pos);
  Simulator::Schedule (Seconds (stepsTime), &Statistics::AdvancePosition, this, node, stepsSize, stepsTime);
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

int main (int argc, char *argv[])
{

  int payLoadSize = 1420;
  int initialDistance = 1;
  
  bool shortGuardInterval = false;
  int spatialStreams = 1;
  uint32_t rtsThreshold = 65535;
  std::string apRateControl = "ns3::MinstrelHtWifiManager";
  uint32_t chWidth = 20;
  int steps = 40;
  int stepsSize = 1;
  int stepsTime = 1;
  int simulationTime = stepsTime * steps;

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
  cmd.Parse (argc, argv);

  //Create AP
  NodeContainer wifiApNode;
  wifiApNode.Create(1);

  //Create STA
  NodeContainer wifiStaNode;
  wifiStaNode.Create(1);

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
  wifi.SetRemoteStationManager ("ns3::MinstrelHtWifiManager", "RtsCtsThreshold", UintegerValue (rtsThreshold));

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

  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (initialDistance, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (wifiApNode);
  mobility.Install (wifiStaNode);


  //Simulator Schedule
  Statistics result;
  Simulator::Schedule (Seconds (0.5 + stepsTime), &Statistics::AdvancePosition, &result, wifiStaNode.Get (0), stepsSize, stepsTime);


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
  uint16_t port = 9;
  PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (staNodeInterface.GetAddress(0), port));
  ApplicationContainer apps_sink = sink.Install (wifiStaNode.Get (0));

  OnOffHelper onoff ("ns3::UdpSocketFactory", InetSocketAddress (staNodeInterface.GetAddress(0), port));
  onoff.SetConstantRate (DataRate ("100Mb/s"), payLoadSize);
  onoff.SetAttribute ("StartTime", TimeValue (Seconds (0.5)));
  onoff.SetAttribute ("StopTime", TimeValue (Seconds (simulationTime)));
  ApplicationContainer apps_source = onoff.Install (wifiApNode.Get (0));

  apps_sink.Start (Seconds (0.5));
  apps_sink.Stop (Seconds (simulationTime));

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
  gnuplot1.SetLegend ("Distance (meters)", "Throughput (Mb/s)");
  gnuplot1.SetTitle ("Throughput (AP to STA) vs distance");
  gnuplot1.AddDataset (result.outputDistanceThroughput);
  gnuplot1.GenerateOutput (outfile1);

  std::ofstream outfile2 ("TimeRate.plt");
  Gnuplot gnuplot2 = Gnuplot ("TimeRate.eps", "Rate");
  gnuplot2.SetTerminal ("post eps color enhanced");
  gnuplot2.SetLegend ("Time (seconds)", "Rate (Mb/s)");
  gnuplot2.SetTitle ("Rate (AP) vs time");
  gnuplot2.AddDataset (result.outputTimeRate);
  gnuplot2.GenerateOutput (outfile2);

  std::ofstream outfile3 ("TimePosition.plt");
  Gnuplot gnuplot3 = Gnuplot ("TimePosition.eps", "Position");
  gnuplot3.SetTerminal ("post eps color enhanced");
  gnuplot3.SetLegend ("Time (seconds)", "Position (meters)");
  gnuplot3.SetTitle ("Positon (STA) vs time");
  gnuplot3.AddDataset (result.outputTimePosition);
  gnuplot3.GenerateOutput (outfile3);

  std::ofstream outfile4 ("FailedPackets.plt");
  Gnuplot gnuplot4 = Gnuplot ("FailedPackets.eps", "Position");
  gnuplot4.SetTerminal ("post eps color enhanced");
  gnuplot4.SetLegend ("# Frames", "Position (meters)");
  gnuplot4.SetTitle ("Failed Frames vs distance (meters)");
  gnuplot4.AddDataset (result.outputFailedPacket);
  gnuplot4.GenerateOutput (outfile4);
  Simulator::Destroy ();

}
