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

int main(int argc, char *argv[])
{
  CommandLine cmd; cmd.Parse(argc,argv);
  LogComponentEnable("TcpPpt", LOG_LEVEL_INFO);
  // turn *this* example’s own INFO logs on
  LogComponentEnable ("PptExample", LOG_LEVEL_INFO);
  // also turn on TcpPpt’s detailed debug/function logs
  LogComponentEnable ("TcpPpt",   LOG_LEVEL_FUNCTION);
  LogComponentEnable ("TcpPpt",   LOG_LEVEL_DEBUG);

  // NS_LOG_COMPONENT_DEFINE ("PptExample");
  NS_LOG_INFO ("Starting PPT example");

  NodeContainer nodes; nodes.Create(2);
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("50Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("5ms"));
  auto devs = p2p.Install(nodes);

  InternetStackHelper internet;
  internet.Install(nodes);
  Ipv4AddressHelper addr("10.1.1.0","255.255.255.0");
  auto ifs = addr.Assign(devs);

  // Use PPT everywhere
  Config::Set ("/NodeList/*/$ns3::TcpL4Protocol/SocketType",
               TypeIdValue (TcpPpt::GetTypeId()));

  // Strict‑priority queue: band 0 = high, band 1 = low
  TrafficControlHelper tch;
  // Tear down _any_ existing root‑queue‑disc on those devices:
  tch.Uninstall (devs);
  

  tch.SetRootQueueDisc ("ns3::FqCoDelQueueDisc");
  // // Now set up exactly one root qdisc, with the exact type+attributes you want:
  // tch.SetRootQueueDisc ("ns3::DctcpQueueDisc",
  //                       "MeanPktSize", UintegerValue (1500),
  //                       "MaxSize", StringValue ("20KiB"));
  // // …or whatever qdisc and parameters PPT needs…
  
  // And finally, install that single root‑qdisc onto all of `devs` at once:
  tch.Install (devs);


  // Sink on node 1
  uint16_t port = 9000;
  Address sinkAddr(InetSocketAddress(ifs.GetAddress(1), port));
  PacketSinkHelper sink("ns3::TcpSocketFactory", sinkAddr);
  auto sinkApp = sink.Install(nodes.Get(1));
  sinkApp.Start(Seconds(0.0)); sinkApp.Stop(Seconds(10.0));

  // Bulk send from node 0→1
  BulkSendHelper bulk("ns3::TcpSocketFactory", sinkAddr);
  bulk.SetAttribute("MaxBytes", UintegerValue(0));
  auto srcApp = bulk.Install(nodes.Get(0));
  srcApp.Start(Seconds(1.0)); srcApp.Stop(Seconds(10.0));

  Simulator::Run();
  Simulator::Destroy();
  return 0;
}

