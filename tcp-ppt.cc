/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "tcp-ppt.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpPpt");

TypeId
TcpPpt::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpPpt")
    .SetParent<TcpDctcp> ()
    .SetGroupName ("Internet")
    .AddConstructor<TcpPpt> ()
  ;
  return tid;
}

TcpPpt::TcpPpt ()
  : TcpDctcp (),
    // m_lcpActive (false),
    m_maxCwnd (0)
{
  NS_LOG_FUNCTION (this);
}

TcpPpt::TcpPpt (const TcpPpt& sock)
  : TcpDctcp (sock),
    // m_lcpActive (sock.m_lcpActive),
    m_maxCwnd (sock.m_maxCwnd)
{
  NS_LOG_FUNCTION (this);
}

TcpPpt::~TcpPpt ()
{
  NS_LOG_FUNCTION (this);
}

void
TcpPpt::Init (Ptr<TcpSocketState> tcb)
{
  NS_LOG_UNCOND ("[TcpPpt::Init] TcpPpt initialized");
  TcpDctcp::Init (tcb);
  m_tcb = tcb;
  tcb->lcpActive = false;
  m_maxCwnd = tcb->m_cWnd;
}
void
TcpPpt::Init (Ptr<TcpSocketState> tcb, double rtt)
{
  NS_LOG_UNCOND ("[TcpPpt::Init] TcpPpt initialized");
  TcpDctcp::Init (tcb);
  m_tcb = tcb;
  tcb->m_lcWnd = tcb->m_segmentSize * 10;
  tcb->lcpActive = true;
  m_maxCwnd = tcb->m_cWnd;
  NS_LOG_UNCOND ("[TcpPpt::CwndEvent] LCP activated, initial cwnd = " << tcb->m_lcWnd);
  Simulator::Schedule (tcb->m_srtt, &TcpPpt::DecayLcp, this, tcb);
}

void
TcpPpt::PktsAcked (Ptr<TcpSocketState> tcb,
                   uint32_t segmentsAcked,
                   const Time& rtt)
{
  TcpDctcp::PktsAcked (tcb, segmentsAcked, rtt);
  // update historical max HCP cwnd
  if (!tcb->lcpActive && tcb->m_cWnd > m_maxCwnd)
    {
      m_maxCwnd = tcb->m_cWnd;
    }
}

void
TcpPpt::CwndEvent (Ptr<TcpSocketState> tcb,
                   TcpSocketState::TcpCAEvent_t event)
{
  // always run standard DCTCP behavior
  TcpDctcp::CwndEvent (tcb, event);

  // Launch LCP on first ECN indication
  if (!tcb->lcpActive && event == TcpSocketState::CA_EVENT_ECN_IS_CE)
    {
      tcb->lcpActive = true;
      // start with half of max seen HCP cwnd
      tcb->m_lcWnd = std::max<uint32_t> (tcb->m_cWnd, m_maxCwnd - tcb->m_cWnd);
      // NS_LOG_UNCOND ("[TcpPpt::CwndEvent] LCP activated, initial cwnd = " << tcb->m_lcWnd);
      // NS_LOG_UNCOND ("[TcpPpt::CwndEvent] HCP activated, initial cwnd = " << tcb->m_cWnd);
      // schedule decay
      Simulator::Schedule (tcb->m_srtt, &TcpPpt::DecayLcp, this, tcb);
    }
}

void
TcpPpt::DecayLcp (Ptr<TcpSocketState> tcb)
{
  uint32_t old = tcb->m_lcWnd;
  // exponential window decrease: half each RTT
  tcb->m_lcWnd = std::max<uint32_t> (1, tcb->m_lcWnd / 2);
  if (tcb->m_lcWnd <= 1)
    {
      tcb->lcpActive = false;
      NS_LOG_UNCOND ("[TcpPpt::DecayLcp] LCP deactivated");
    }
  else
    {
      NS_LOG_UNCOND ("[TcpPpt::DecayLcp] cwnd decayed: " << old << " -> " << tcb->m_lcWnd);
      // schedule next decay
      Simulator::Schedule (tcb->m_srtt, &TcpPpt::DecayLcp, this, tcb);
    }
}

} // namespace ns3

