/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef TCP_PPT_H
#define TCP_PPT_H

#include "ns3/tcp-dctcp.h"
#include "ns3/tcp-socket-state.h"

namespace ns3 {

class TcpPpt : public TcpDctcp
{
public:
  static TypeId GetTypeId (void);
  TcpPpt ();
  TcpPpt (const TcpPpt& sock);
  ~TcpPpt () override;

  std::string GetName () const override { return "TcpPpt"; }
  Ptr<TcpCongestionOps> Fork () override { return CopyObject<TcpPpt> (this); }

  void Init (Ptr<TcpSocketState> tcb) override
  {
    TcpDctcp::Init (tcb);
    m_tcb = tcb;
    m_lcpActive = false;
    m_maxCwnd = tcb->m_cWnd;
  }

  void PktsAcked (Ptr<TcpSocketState> tcb,
                  uint32_t segmentsAcked,
                  const Time& rtt) override
  {
    TcpDctcp::PktsAcked (tcb, segmentsAcked, rtt);
    // update historical max HCP cwnd
    if (!m_lcpActive && tcb->m_cWnd > m_maxCwnd)
    {
      m_maxCwnd = tcb->m_cWnd;
    }
  }

  void CwndEvent (Ptr<TcpSocketState> tcb,
                  TcpSocketState::TcpCAEvent_t event) override
  {
    // always run standard DCTCP behavior
    TcpDctcp::CwndEvent (tcb, event);

    // Launch LCP on first ECN indication
    if (!m_lcpActive && event == TcpSocketState::CA_EVENT_ECN_IS_CE)
    {
      m_lcpActive = true;
      // start with half of max seen HCP cwnd
      m_lcpCwnd = std::max<unsigned>(1, m_maxCwnd / 2);
      // schedule decay
      Simulator::Schedule (tcb->m_srtt, &TcpPpt::DecayLcp, this, tcb);
    }
  }

  bool IsLcpActive() const { return m_lcpActive; }

private:
  // low‑priority loop state
  bool     m_lcpActive;
  uint32_t m_lcpCwnd;
  uint32_t m_maxCwnd;

  void DecayLcp (Ptr<TcpSocketState> tcb)
  {
    // exponential window decrease: half each RTT
    m_lcpCwnd = std::max<unsigned>(1, m_lcpCwnd / 2);
    if (m_lcpCwnd <= 1)
    {
      m_lcpActive = false;
    }
    else
    {
      // schedule next decay
      Simulator::Schedule (tcb->m_srtt, &TcpPpt::DecayLcp, this, tcb);
    }
  }
};

} // namespace ns3
#endif /* TCP_PPT_H */
