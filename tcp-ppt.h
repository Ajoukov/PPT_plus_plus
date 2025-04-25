/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef TCP_PPT_H
#define TCP_PPT_H

#include "ns3/tcp-dctcp.h"
#include "ns3/tcp-socket-state.h"
#include "ns3/simulator.h"

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

  void Init (Ptr<TcpSocketState> tcb) override;
  void Init (Ptr<TcpSocketState> tcb, double rtt);

  void PktsAcked (Ptr<TcpSocketState> tcb,
                  uint32_t segmentsAcked,
                  const Time& rtt) override;

  void CwndEvent (Ptr<TcpSocketState> tcb,
                  TcpSocketState::TcpCAEvent_t event) override;
  
  bool IsLcpActive() const { return m_lcpActive; }

private:

  // low-priority loop state
  bool     m_lcpActive;
  uint32_t m_lcpCwnd;
  uint32_t m_maxCwnd;
  Ptr<TcpSocketState> m_tcb;

  // exponential-decay callback: declared here, defined in tcp-ppt.cc
  void DecayLcp (Ptr<TcpSocketState> tcb);
};

} // namespace ns3
#endif /* TCP_PPT_H */
