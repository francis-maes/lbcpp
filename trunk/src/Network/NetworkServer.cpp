/*-----------------------------------------.---------------------------------.
| Filename: NetworkServer.cpp              | Network Server                  |
| Author  : Julien Becker                  |                                 |
| Started : 01/12/2010 18:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Network/NetworkServer.h>

using namespace lbcpp;

/** NetworkServer **/
inline void workAroundVCBugSleep(int milliseconds)
  {juce::Thread::sleep(milliseconds);}

bool NetworkServer::startServer(int port)
{
  if (!beginWaitingForSocket(port))
    return false;
  while (true)
    workAroundVCBugSleep(INT_MAX);
  return true;
}

void NetworkServer::stopServer()
  {stop();}

InterprocessConnection* NetworkServer::createConnectionObject()
  {return createNetworkClient();}
