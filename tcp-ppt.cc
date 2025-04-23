#include "tcp-ppt.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/tcp-socket-state.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpPpt");
NS_OBJECT_ENSURE_REGISTERED (TcpPpt);

TypeId
TcpPpt::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpPpt")
    .SetParent<TcpDctcp>()
    .AddConstructor<TcpPpt>()
    ;
  return tid;
}

TcpPpt::TcpPpt ()
  : TcpDctcp(), m_lcpActive(false), m_lcpCwnd(0), m_maxCwnd(0)
{}

TcpPpt::TcpPpt (const TcpPpt& sock)
  : TcpDctcp(sock)
  , m_lcpActive(sock.m_lcpActive)
  , m_lcpCwnd(sock.m_lcpCwnd)
  , m_maxCwnd(sock.m_maxCwnd)
{}

TcpPpt::~TcpPpt () = default;

} // namespace ns3
