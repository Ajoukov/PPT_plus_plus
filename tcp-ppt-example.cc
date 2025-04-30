#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/point-to-point-module.h"

#include "ns3/tcp-ppt.h"
#include "ns3/tcp-dctcp.h"
#include "ns3/tcp-cubic.h"

#include <cmath>
#include <vector>
#include <iostream>

using namespace ns3;

int main (int argc, char *argv[]) {
    int         num_sources         = 15;
    int         num_destinations    = 15;
    int         base_port           = 8000;

    int         full_mesh           = 0;    // false=Poisson arrivals
    double      sim_time            = 10.0; // seconds
    double      flow_rate           = 1.0;  // Hz per source (for Poisson)
    double      link_rate_mbps      = 50.0; // Mbps on CSMA
    double      link_delay_ms       = 5.0;  // ms one-way
    int         workload            = 0;
    int         transport           = 0;

    CommandLine cmd;
    cmd.AddValue ("full_mesh",    "true=full mesh, false=Poisson",  full_mesh);
    cmd.AddValue ("sim_time",     "Simulation time (s)",            sim_time);
    cmd.AddValue ("flow_rate",    "Poisson rate per source (Hz)",   flow_rate);
    cmd.AddValue ("link_rate",    "CSMA data rate (Mbps)",          link_rate_mbps);
    cmd.AddValue ("link_delay",   "CSMA delay (ms)",                link_delay_ms);
    cmd.AddValue ("workload",     "0=WebSearch,1=DataMining",       workload);
    cmd.AddValue ("transport",    "0=PPT,1=DCTCP,2=Cubic",          transport);
    cmd.Parse (argc, argv);

    // Pareto numbers according to paper
    double alpha = 1.5;
    double xm = workload == 0 ? 533333.0 : 2.47e6;

    Config::SetDefault ("ns3::TcpSocketBase::UseEcn", EnumValue(TcpSocketState::On));
    switch (transport) {
        case (0):
            Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpPpt::GetTypeId())); break;
        case (1):
            Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpDctcp::GetTypeId())); break;
        case (2):
            Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpCubic::GetTypeId())); break;
    }

    NodeContainer sources;
    sources.Create(num_sources);
    NodeContainer destinations;
    destinations.Create(num_destinations);
    NodeContainer all;
    all.Add(sources);
    all.Add(destinations);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate",
            StringValue (std::to_string(link_rate_mbps) + "Mbps"));
    csma.SetChannelAttribute("Delay",
            StringValue (std::to_string(link_delay_ms) + "ms"));
    NetDeviceContainer devices = csma.Install(all);

    // To use p2p we'll need to create individual connections for each src-dst pair (for{for{}})
    // PointToPointHelper p2p;
    // p2p.SetDeviceAttribute("DataRate",
    //         StringValue(std::to_string(link_rate_mbps) + "Mbps"));
    // p2p.SetChannelAttribute("Delay",
    //         StringValue(std::to_string(link_delay_ms) + "ms"));
    // auto devices = p2p.Install(all);

    InternetStackHelper inet;
    inet.Install(all);

    Ipv4AddressHelper addr ("10.1.1.0", "255.255.255.0");
    auto ifs = addr.Assign(devices);

    std::vector<InetSocketAddress> sinks;
    for (int i = 0; i < num_destinations; i++) {
        InetSocketAddress isa(ifs.GetAddress(num_sources + i), base_port + i);
        PacketSinkHelper ps("ns3::TcpSocketFactory", isa);
        auto app = ps.Install(destinations.Get(i));
        app.Start(Seconds(0.0));
        app.Stop(Seconds(sim_time + 1.0));
        sinks.push_back(isa);
    }

    Ptr<UniformRandomVariable> urv =
        CreateObject<UniformRandomVariable>();
    urv->SetAttribute("Min", DoubleValue(0.0));
    urv->SetAttribute("Max", DoubleValue(1.0));
    Ptr<ExponentialRandomVariable> erv =
        CreateObject<ExponentialRandomVariable>();
    erv->SetAttribute("Mean", DoubleValue(1.0 / flow_rate));

    int port_counter = base_port + num_destinations + 1000;

    for (int i = 0; i < num_sources; i++) {
        for (double t = erv->GetValue(); t < sim_time; t += erv->GetValue()) {
            Simulator::Schedule(Seconds(t), [&, i, t]() mutable {
                auto dst =
sinks[urv->GetInteger(0, num_destinations - 1)];
                auto bytes =
UintegerValue(xm / std::pow(1.0 - urv->GetValue(), 1.0/alpha));
                auto addr =
AddressValue(InetSocketAddress(Ipv4Address::GetAny(), port_counter));

                BulkSendHelper bulk("ns3::TcpSocketFactory", dst);
                bulk.SetAttribute("MaxBytes", bytes);
                bulk.SetAttribute("Local", addr);
                port_counter++;
                auto a = bulk.Install(sources.Get(i));
                a.Start(Seconds(t));
                a.Stop(Seconds(sim_time + 1.0));
            });
        }
    }
    Simulator::Stop (Seconds (sim_time + 2.0));
    Simulator::Run();
}
