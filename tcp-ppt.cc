/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "tcp-ppt.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/tcp-socket-state.h"
#include "ns3/tcp-dctcp.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpPpt");
NS_OBJECT_ENSURE_REGISTERED (TcpPpt);

TypeId
TcpPpt::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpPpt")
    .SetParent<TcpDctcp> ()
    .SetGroupName ("Internet")
    .AddConstructor<TcpPpt> ();
  return tid;
}

TcpPpt::TcpPpt ()
  : TcpDctcp (),
    m_lcpActive (false),
    m_lcpCwnd   (0),
    m_maxCwnd   (0)
{
  NS_LOG_FUNCTION (this);
}

TcpPpt::TcpPpt (const TcpPpt& sock)
  : TcpDctcp (sock),
    m_lcpActive (sock.m_lcpActive),
    m_lcpCwnd   (sock.m_lcpCwnd),
    m_maxCwnd   (sock.m_maxCwnd)
{
  NS_LOG_FUNCTION (this);
}

TcpPpt::~TcpPpt () {}

Ptr<TcpCongestionOps>
TcpPpt::Fork ()
{
  return CopyObject<TcpPpt> (this);
}

std::string
TcpPpt::GetName () const
{
  return "TcpPpt";
}

void
TcpPpt::Init (Ptr<TcpSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);
  TcpDctcp::Init (tcb);
  m_lcpActive = false;
  m_maxCwnd   = tcb->m_cWnd.Get ();
}

void
TcpPpt::PktsAcked (Ptr<TcpSocketState> tcb,
                   uint32_t segmentsAcked,
                   const Time& rtt)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked << rtt);
  TcpDctcp::PktsAcked (tcb, segmentsAcked, rtt);

  uint32_t cwnd = tcb->m_cWnd.Get ();
  if (cwnd > m_maxCwnd)
    {
      m_maxCwnd = cwnd;
    }
}

void
TcpPpt::CwndEvent (Ptr<TcpSocketState> tcb,
                   TcpSocketState::TcpCAEvent_t event)
{
  NS_LOG_FUNCTION (this << tcb << event);
  TcpDctcp::CwndEvent (tcb, event);

  if (!m_lcpActive && event == TcpSocketState::CA_EVENT_ECN_IS_CE)
    {
      m_lcpActive = true;
      m_lcpCwnd = std::max (1u, m_maxCwnd / 2u);
      Simulator::Schedule (tcb->m_srtt,
                           &TcpPpt::DecayLcp, this, tcb);
    }
}

void
TcpPpt::DecayLcp (Ptr<TcpSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb << m_lcpCwnd);
  if (!m_lcpActive) return;

  m_lcpCwnd = std::max (1u, m_lcpCwnd / 2u);
  if (m_lcpCwnd <= 1)
    {
      m_lcpActive = false;
    }
  else
    {
      Simulator::Schedule (tcb->m_srtt,
                           &TcpPpt::DecayLcp, this, tcb);
    }
}

} // namespace ns3

