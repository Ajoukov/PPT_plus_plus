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

char PPT_IS_USING_SUPERPOSITION = 0;
char PPT_IS_PRINTING_CWND_SIZES = 0;

void PrintTime() {
  std::cout << "Sim time: " << Simulator::Now().GetSeconds() << " s\n";
  Simulator::Schedule(Seconds(0.1), &PrintTime);
}

int main(int argc, char *argv[]) {
    int n_sources = 144;
    int n_dests = 144;
    int base_port = 8000;

    double sim_time = 10.0; // seconds
    double load = 0.5;
    int workload = 0;
    int transport = 0;
    double link_rate_gbps = 40e-3;
    double core_rate_gbps = 100e-3;

    CommandLine cmd;
    cmd.AddValue("sim_time",   "Simulation time (s)",            sim_time);
    cmd.AddValue("load",       "Offered traffic load (0.0-1.0)", load);
    cmd.AddValue("workload",   "0=WebSearch,1=DataMining",       workload);
    cmd.AddValue("transport",  "0=PPT,1=DCTCP,2=Cubic",          transport);
    cmd.AddValue("n_sources",  "Number of sources",              n_sources);
    cmd.AddValue("n_dests",    "Number of destinations",         n_dests);
    cmd.AddValue("print_cwnd", "Print cwnd sizes or not",        PPT_IS_PRINTING_CWND_SIZES);
    cmd.Parse(argc, argv);

    // Pareto numbers according to paper
    double alpha = 1.5;
    double xm = workload == 0 ? 533333.0 : 2.47e6;

    Config::SetDefault("ns3::TcpSocketBase::UseEcn", EnumValue(TcpSocketState::On));
    switch (transport) {
        case (0):
            Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpPpt::GetTypeId())); break;
        case (1):
            Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpDctcp::GetTypeId())); break;
        case (2):
            Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpCubic::GetTypeId())); break;
        case (3):
            Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpPpt::GetTypeId()));
            PPT_IS_USING_SUPERPOSITION = 1;
            break;

    }

    int n_servers = 144;
    int n_leaves = 9;
    int n_spines = 4;
    int n_servers_per_leaf = 16;
    NodeContainer servers; servers.Create(n_servers);
    NodeContainer leaves; leaves.Create(n_leaves);
    NodeContainer spines; spines.Create(n_spines);

    InternetStackHelper stack;
    stack.Install(servers);
    stack.Install(leaves);
    stack.Install(spines);

    PointToPointHelper edge;
    edge.SetDeviceAttribute("DataRate", StringValue(std::to_string(link_rate_gbps) + "Gbps"));
    edge.SetChannelAttribute("Delay", StringValue("0.1ms"));

    PointToPointHelper core;
    core.SetDeviceAttribute("DataRate", StringValue(std::to_string(core_rate_gbps) + "Gbps"));
    core.SetChannelAttribute("Delay", StringValue("0.5ms"));

    Ipv4AddressHelper addr;
    std::ostringstream subnet;

    // leaves end up with 2 IPs, one for server-side and one for core-side interactions
    for (int i = 0; i < n_leaves; i++) {
        for (int j = 0; j < n_spines; j++) {
            NodeContainer pair(leaves.Get(i), spines.Get(j));
            NetDeviceContainer devs = core.Install(pair);
            subnet.str(""); subnet.clear();
            subnet << "1." << j << "." << i << ".0";
            addr.SetBase(subnet.str().c_str(), "255.255.255.252");
            addr.Assign(devs);
        }
    }

    for (int i = 0; i < n_leaves; i++) {
        for (int j = 0; j < n_servers_per_leaf; j++) {
            int idx = i * n_servers_per_leaf + j;
            NodeContainer pair(servers.Get(idx), leaves.Get(i));
            NetDeviceContainer devs = edge.Install(pair);
            std::ostringstream sn;
            sn << "2."<< i << "." << j << ".0";
            addr.SetBase(sn.str().c_str(), "255.255.255.0");
            Ipv4InterfaceContainer ifc = addr.Assign(devs);
        }
    }

    NodeContainer sources;
    for (int i = 0; i < n_sources; i++)
        sources.Add(servers.Get(i));

    // we add n_sources to prevent overlap
    NodeContainer destinations;
    for (int i = 0; i < n_dests; i++)
        destinations.Add(servers.Get((i + n_sources) % n_servers));

    std::vector<InetSocketAddress> sinks;
    for (int i = 0; i < n_sources; i++) {
        Ptr<Node> dest = destinations.Get(i);
        Ptr<Ipv4> ipv4 = dest->GetObject<Ipv4>();
        Ipv4Address ip = ipv4->GetAddress(1,0).GetLocal();
        InetSocketAddress isa(ip, base_port + i);
        PacketSinkHelper ps("ns3::TcpSocketFactory", isa);
        ApplicationContainer apps = ps.Install(destinations.Get(i));
        apps.Start(Seconds(0.0));
        apps.Stop(Seconds(sim_time + 1.0));
        sinks.push_back(isa);
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Ptr<UniformRandomVariable> urv = CreateObject<UniformRandomVariable>();
    urv->SetAttribute("Min", DoubleValue(0.0));
    urv->SetAttribute("Max", DoubleValue(1.0));

    double mean_size = alpha * xm / (alpha - 1);
    double capacity = link_rate_gbps * 1e9; // bits/s
    double lambda_src = load * capacity / (n_sources * mean_size * 8.0);
    lambda_src *= 1e3;
    NS_LOG_UNCOND("Mean number of flows per source: " << lambda_src);
    Ptr<ExponentialRandomVariable> erv = CreateObject<ExponentialRandomVariable>();
    erv->SetAttribute("Mean", DoubleValue(1.0 / lambda_src));

    int port_counter = base_port + 1000; // any free range
    for (int i = 0; i < n_sources; i++) {
        for (double t = erv->GetValue(); t < sim_time; t += erv->GetValue()) {
            Simulator::Schedule(Seconds(t), [&,i,t]() {
                auto dst = sinks[urv->GetInteger(0, sinks.size() - 1)];
                BulkSendHelper bulk("ns3::TcpSocketFactory", dst);
                bulk.SetAttribute("MaxBytes",
                        UintegerValue( xm/std::pow(1.0-urv->GetValue(),1.0/alpha) ));
                bulk.SetAttribute("Local",
                        AddressValue(InetSocketAddress(Ipv4Address::GetAny(), port_counter++)));

                ApplicationContainer apps = bulk.Install(sources.Get(i));
                apps.Start(Seconds(t));
                apps.Stop(Seconds(sim_time + 1.0));
            });
        }
    }

    FlowMonitorHelper fm;
    auto monitor = fm.InstallAll();

    NS_LOG_UNCOND("Starting the simulation");

    Simulator::Schedule(Seconds(0.0), &PrintTime);

    Simulator::Stop(Seconds(sim_time + 2.0));
    Simulator::Run();

    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(fm.GetClassifier());

    double total_fct = 0;
    double total_throughput = 0;
    int n_samples = 0;

    int i = 0;
    for (auto &kv : monitor->GetFlowStats()) {
        const auto &st = kv.second;

        if (st.rxPackets == 0)
            continue;

        double t0 = st.timeFirstTxPacket.GetSeconds();
        double t1 = st.timeLastRxPacket.GetSeconds();
        if (t1 < t0) // negative means ACKs b/c numbers are reversed
            continue;

        // std::cout << "End time: " << t1 << std::endl;

        auto ft = classifier->FindFlow(kv.first);
        uint16_t port = ft.destinationPort;
        // not one of our BulkSend sinks
        if (port < base_port || port >= base_port + n_dests)
            continue;

        n_samples ++;
        total_fct += (t1 - t0) * 1000;

        total_throughput += st.rxBytes;

        std::cout << "FCT " << i++ << ": " << ((t1 - t0) * 1000) << "ms" << std::endl;
    }

    std::cout << "Total throughput: " << std::fixed << total_throughput << "bytes" << std::endl;

    // std::cout << "Average FCT: " <<
    //     (total_fct / n_samples * 1000) << "ms" << std::endl;

    Simulator::Destroy();
}
