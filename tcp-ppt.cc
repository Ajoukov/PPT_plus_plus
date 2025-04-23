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
    m_lcpActive (false),
    m_lcpCwnd (0),
    m_maxCwnd (0),
    m_tcb (0)
{
  NS_LOG_FUNCTION (this);
}

TcpPpt::TcpPpt (const TcpPpt& sock)
  : TcpDctcp (sock),
    m_lcpActive (sock.m_lcpActive),
    m_lcpCwnd (sock.m_lcpCwnd),
    m_maxCwnd (sock.m_maxCwnd),
    m_tcb (sock.m_tcb)
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
  m_lcpActive = false;
  m_maxCwnd = tcb->m_cWnd;
}

void
TcpPpt::PktsAcked (Ptr<TcpSocketState> tcb,
                   uint32_t segmentsAcked,
                   const Time& rtt)
{
  TcpDctcp::PktsAcked (tcb, segmentsAcked, rtt);
  // update historical max HCP cwnd
  if (!m_lcpActive && tcb->m_cWnd > m_maxCwnd)
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
  if (!m_lcpActive && event == TcpSocketState::CA_EVENT_ECN_IS_CE)
    {
      m_lcpActive = true;
      // start with half of max seen HCP cwnd
      m_lcpCwnd = std::max<uint32_t> (1, m_maxCwnd / 2);
      NS_LOG_UNCOND ("[TcpPpt::CwndEvent] LCP activated, initial cwnd = " << m_lcpCwnd);
      // schedule decay
      Simulator::Schedule (tcb->m_srtt, &TcpPpt::DecayLcp, this, tcb);
    }
}

void
TcpPpt::DecayLcp (Ptr<TcpSocketState> tcb)
{
  uint32_t old = m_lcpCwnd;
  // exponential window decrease: half each RTT
  m_lcpCwnd = std::max<uint32_t> (1, m_lcpCwnd / 2);
  if (m_lcpCwnd <= 1)
    {
      m_lcpActive = false;
      NS_LOG_UNCOND ("[TcpPpt::DecayLcp] LCP deactivated");
    }
  else
    {
      NS_LOG_UNCOND ("[TcpPpt::DecayLcp] cwnd decayed: " << old << " -> " << m_lcpCwnd);
      // schedule next decay
      Simulator::Schedule (tcb->m_srtt, &TcpPpt::DecayLcp, this, tcb);
    }
}

} // namespace ns3

