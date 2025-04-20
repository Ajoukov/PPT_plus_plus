/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef TCP_PPT_H
#define TCP_PPT_H

#include "ns3/tcp-dctcp.h"
#include "ns3/tcp-socket-state.h"
#include "ns3/sequence-number.h"

namespace ns3 {

class TcpPpt : public TcpDctcp
{
public:
  static TypeId GetTypeId (void);
  TcpPpt ();
  TcpPpt (const TcpPpt& sock);
  ~TcpPpt () override;

  std::string GetName () const override;
  Ptr<TcpCongestionOps> Fork () override;
  void Init (Ptr<TcpSocketState> tcb) override;
  void PktsAcked (Ptr<TcpSocketState> tcb,
                  uint32_t segmentsAcked,
                  const Time& rtt) override;
  void CwndEvent (Ptr<TcpSocketState> tcb,
                  TcpSocketState::TcpCAEvent_t event) override;

private:
  bool     m_lcpActive;
  uint32_t m_lcpCwnd;
  double   m_alphaMin;
  SequenceNumber32 m_maxCwnd;

  void DecayLcp (Ptr<TcpSocketState> tcb);
};

} // namespace ns3

#endif /* TCP_PPT_H */

