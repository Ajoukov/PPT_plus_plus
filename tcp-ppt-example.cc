/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/tcp-ppt.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PptExample");

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // enable ALL PPT logs and prefixes
  LogComponentEnable ("TcpPpt",    LOG_LEVEL_ALL);
  LogComponentEnable ("PptExample", LOG_LEVEL_ALL);
  LogComponentEnableAll ((LogLevel)(LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC));
  NS_LOG_INFO ("[MAIN] Starting PPT example");

  // 1) create a 2-node P2P
  NodeContainer nodes; nodes.Create (2);
  NS_LOG_INFO ("[MAIN] Created 2 nodes");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("50Mbps"));
  p2p.SetChannelAttribute("Delay",     StringValue ("5ms"));
  auto devices = p2p.Install (nodes);
  NS_LOG_INFO ("[MAIN] P2P devices installed");

  // 2) force PPT everywhere
  //    – at the TCP socket
  Config::SetDefault ("ns3::TcpSocketBase::UseEcn",
                      EnumValue (TcpSocketState::On));
  //    – pick your PPT variant
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType",
                      TypeIdValue (TcpPpt::GetTypeId ()));
  NS_LOG_INFO ("[MAIN] Configured default TCP variant = TcpPpt + ECN");

  // 3) install IP/TCP stack
  InternetStackHelper internet;
  internet.Install (nodes);
  NS_LOG_INFO ("[MAIN] Installed Internet stack");

  // 4) assign IPs
  Ipv4AddressHelper addr ("10.1.1.0", "255.255.255.0");
  auto ifs = addr.Assign (devices);
  NS_LOG_INFO ("[MAIN] Assigned IPs "
               << ifs.GetAddress (0) << " & " << ifs.GetAddress (1));

  // 5) install a RED queue‐disc with ECN marking
  TrafficControlHelper tch;
  tch.Uninstall (devices);
  tch.SetRootQueueDisc ("ns3::RedQueueDisc",
                        // mark ECN when qlen > MinTh
                        "MinTh",        DoubleValue (5),
                        "MaxTh",        DoubleValue (15),
                        "MeanPktSize",  UintegerValue (1500),
                        "UseEcn",       BooleanValue (true));
  tch.Install (devices);
  NS_LOG_INFO ("[MAIN] Installed RED+ECN queue-disc");

  // 6) sink on node 1
  constexpr uint16_t port = 9000;
  Address sinkAddr (InetSocketAddress (ifs.GetAddress (1), port));
  PacketSinkHelper sink ("ns3::TcpSocketFactory", sinkAddr);
  auto sinkApp = sink.Install (nodes.Get (1));
  sinkApp.Start (Seconds (0.0));
  sinkApp.Stop  (Seconds (10.0));
  NS_LOG_INFO ("[MAIN] Installed PacketSink on node 1");

  // 7) bulk‐send from node 0→1
  BulkSendHelper bulk ("ns3::TcpSocketFactory", sinkAddr);
  bulk.SetAttribute ("MaxBytes", UintegerValue (0)); // endless
  auto srcApp = bulk.Install (nodes.Get (0));
  srcApp.Start (Seconds (1.0));
  srcApp.Stop  (Seconds (10.0));
  NS_LOG_INFO ("[MAIN] Installed BulkSend on node 0");

  // 8) run
  NS_LOG_INFO ("[MAIN] Running simulator");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("[MAIN] Done");
  return 0;
}

