/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "tcp-ppt.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include <fstream>

extern char PPT_IS_USING_SUPERPOSITION;
extern char PPT_IS_PRINTING_CWND_SIZES;

namespace ns3 {

static std::ofstream g_lcpLog ("cwnd_sizes.log", std::ios::out);

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
  // NS_LOG_UNCOND ("[TcpPpt::Init] TcpPpt initialized");
  TcpDctcp::Init (tcb);
  m_tcb = tcb;
  tcb->lcpActive = false;
  tcb->prev_m_cWnd = 0;
  m_maxCwnd = 0;
}
void
TcpPpt::Init (Ptr<TcpSocketState> tcb, double rtt)
{
  // NS_LOG_UNCOND ("[TcpPpt::Init] TcpPpt initialized");
  TcpDctcp::Init (tcb);
  m_tcb = tcb;
  // tcb->m_lcWnd = 10;
  tcb->lcpActive = true;
  m_maxCwnd = 0;
  // tcb->m_lcWnd = m_maxCwnd;
  // NS_LOG_UNCOND ("[TcpPpt::CwndEvent] HCP activated, initial cwnd = " << tcb->m_cWnd);
  // NS_LOG_UNCOND ("[TcpPpt::CwndEvent] LCP activated, initial cwnd = " << tcb->m_lcWnd);
  // Simulator::Schedule (tcb->m_srtt, &TcpPpt::DecayLcp, this, tcb);
}

void
TcpPpt::PktsAcked (Ptr<TcpSocketState> tcb,
                   uint32_t segmentsAcked,
                   const Time& rtt)
{
  TcpDctcp::PktsAcked (tcb, segmentsAcked, rtt);
  // update historical max HCP cwnd
}

void
TcpPpt::CwndEvent (Ptr<TcpSocketState> tcb,
                   TcpSocketState::TcpCAEvent_t event)
{
  // always run standard DCTCP behavior
  TcpDctcp::CwndEvent (tcb, event);

  int prev_m_cWnd = tcb->prev_m_cWnd;
  tcb->prev_m_cWnd = tcb->m_cWnd;

  if (tcb->m_cWnd > m_maxCwnd)
    m_maxCwnd = tcb->m_cWnd;

  // don't check for LCP changes if ssThresh is INT_MAX
  // if (tcb->m_ssThresh == 4294967295) { // log
  //   double now = Simulator::Now().GetSeconds ();
  //   long id = (long)&*tcb;
  //   g_lcpLog << now
  //        << "\t" << id
  //        << "\t" << tcb->m_cWnd    // HCP cwnd
  //        << "\t" << tcb->m_lcWnd   // LCP cwnd
  //        << "\t" << m_maxCwnd
  //        << "\n";
  //   g_lcpLog.flush ();
  //   return;
  // }

  // NS_LOG_UNCOND ("[TcpPpt::Hcp] HCP Activated " << tcb->m_cWnd);
  // if (!tcb->lcpActive && tcb->m_cWnd > m_maxCwnd)

  // Launch LCP on first ECN indication
  // if (!tcb->lcpActive && event == TcpSocketState::CA_EVENT_ECN_IS_CE)

  if (PPT_IS_USING_SUPERPOSITION) { // new version
    if ((int)tcb->m_cWnd < prev_m_cWnd) {
      tcb->m_lcWnd += m_maxCwnd - tcb->m_cWnd;
      m_maxCwnd = tcb->m_cWnd;
    }
    if (!tcb->lcpActive && (int)tcb->m_cWnd < prev_m_cWnd)
      TcpPpt::TestFunc(tcb);
  } else { // old version
    if (!tcb->lcpActive && (int)tcb->m_cWnd < prev_m_cWnd) {
      tcb->m_lcWnd += m_maxCwnd - tcb->m_cWnd;
      m_maxCwnd = tcb->m_cWnd;
      TcpPpt::TestFunc(tcb);
    }
  }
  if (PPT_IS_PRINTING_CWND_SIZES) { // log
    double now = Simulator::Now().GetSeconds ();
    long id = (long)&*tcb;
    g_lcpLog << now
         << "\t" << id
         << "\t" << tcb->m_cWnd    // HCP cwnd
         << "\t" << tcb->m_lcWnd   // LCP cwnd
         << "\t" << m_maxCwnd
         << "\n";
    g_lcpLog.flush ();
  }
}

void 
TcpPpt::TestFunc(Ptr<TcpSocketState> tcb)
{
  tcb->lcpActive = true;
  // NS_LOG_UNCOND ("[TcpPpt::Lcp] LCP Activated");
  // tcb->m_lcWnd += m_maxCwnd - tcb->m_cWnd;
  // m_maxCwnd = tcb->m_cWnd;

  // NS_LOG_UNCOND ("[TcpPpt::CwndEvent] LCP, initial cwnd = " << tcb->m_lcWnd);
  // NS_LOG_UNCOND ("[TcpPpt::CwndEvent] HCP, initial cwnd = " << tcb->m_cWnd);
  // NS_LOG_UNCOND ("[TcpPpt::CwndEvent] MAX CWND, max cwnd = " << m_maxCwnd);
  // NS_LOG_UNCOND ("[TcpPpt::CwndEvent] rtt = " << tcb->m_srtt);
  // schedule decay
  Simulator::Schedule (tcb->m_srtt, &TcpPpt::DecayLcp, this, tcb);
}

void
TcpPpt::DecayLcp (Ptr<TcpSocketState> tcb)
{
  // uint32_t old = tcb->m_lcWnd;
  // exponential window decrease: half each RTT

  tcb->m_lcWnd = std::max<uint32_t> (1, tcb->m_lcWnd / 2);
  if (PPT_IS_PRINTING_CWND_SIZES) {
    double now = Simulator::Now().GetSeconds ();
    long id = (long)&*tcb;
    g_lcpLog << now
         << "\t" << id
         << "\t" << tcb->m_cWnd    // HCP cwnd
         << "\t" << tcb->m_lcWnd   // LCP cwnd
         << "\t" << m_maxCwnd
         << "\n";
    g_lcpLog.flush ();
  }

  int prev_m_cWnd = tcb->prev_m_cWnd;
  tcb->prev_m_cWnd = tcb->m_cWnd;
  if (!tcb->lcpActive && (int)tcb->m_cWnd < prev_m_cWnd)
    tcb->m_lcWnd += m_maxCwnd - tcb->m_cWnd;

  if (tcb->m_lcWnd <= 1) {
    tcb->lcpActive = false;
    m_maxCwnd = 0;
    // NS_LOG_UNCOND ("[TcpPpt::DecayLcp] LCP deactivated");
  }
  else {
    // NS_LOG_UNCOND ("[TcpPpt::DecayLcp] cwnd decayed: " << old << " -> " << tcb->m_lcWnd);
    // schedule next decay
    Simulator::Schedule (tcb->m_srtt, &TcpPpt::DecayLcp, this, tcb);
  }
}

} // namespace ns3

