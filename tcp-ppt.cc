/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "tcp-ppt.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/tcp-socket-state.h"
#include "ns3/dctcp-congestion-state.h"

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
  : TcpDctcp (), m_lcpActive (false), m_lcpCwnd (0),
    m_alphaMin (1.0), m_maxCwnd (SequenceNumber32 (0))
{
  NS_LOG_FUNCTION (this);
}

TcpPpt::TcpPpt (const TcpPpt& sock)
  : TcpDctcp (sock),
    m_lcpActive (sock.m_lcpActive),
    m_lcpCwnd (sock.m_lcpCwnd),
    m_alphaMin (sock.m_alphaMin),
    m_maxCwnd (sock.m_maxCwnd)
{
  NS_LOG_FUNCTION (this);
}

TcpPpt::~TcpPpt () {}

Ptr<TcpCongestionOps> TcpPpt::Fork () { return CopyObject<TcpPpt> (this); }
std::string TcpPpt::GetName () const { return "TcpPpt"; }

void TcpPpt::Init (Ptr<TcpSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);
  TcpDctcp::Init (tcb);
  m_lcpActive = false;
  m_alphaMin = 1.0;
  m_maxCwnd = tcb->m_cWnd;
}

void TcpPpt::PktsAcked (Ptr<TcpSocketState> tcb,
                        uint32_t segmentsAcked,
                        const Time& rtt)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked << rtt);
  TcpDctcp::PktsAcked (tcb, segmentsAcked, rtt);

  auto state = static_cast<DctcpCongestionState*> (tcb->m_congestionState.get ());
  m_alphaMin = std::min (m_alphaMin, state->GetAlpha ());
  if (tcb->m_cWnd > m_maxCwnd.Get ()) m_maxCwnd = tcb->m_cWnd;
}

void TcpPpt::CwndEvent (Ptr<TcpSocketState> tcb,
                        TcpSocketState::TcpCAEvent_t event)
{
  NS_LOG_FUNCTION (this << tcb << event);
  TcpDctcp::CwndEvent (tcb, event);

  if (!m_lcpActive && m_alphaMin < 1.0)
    {
      m_lcpActive = true;
      double gain = 0.5 - m_alphaMin;
      uint32_t init = static_cast<uint32_t> (gain * m_maxCwnd.Get ());
      m_lcpCwnd = std::max (1u, init);
      Simulator::Schedule (tcb->m_rtt, &TcpPpt::DecayLcp, this, tcb);
    }
}

void TcpPpt::DecayLcp (Ptr<TcpSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb << m_lcpCwnd);
  if (!m_lcpActive) return;
  m_lcpCwnd = std::max (1u, m_lcpCwnd / 2);
  if (m_lcpCwnd <= 1)
    {
      m_lcpActive = false;
      m_alphaMin  = 1.0;
    }
  else
    {
      Simulator::Schedule (tcb->m_rtt, &TcpPpt::DecayLcp, this, tcb);
    }
}

} // namespace ns3

