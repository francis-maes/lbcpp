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

// TODO: remove the .cpp file

/** NetworkServer **/
bool NetworkServer::startServer(int port)
  {return beginWaitingForSocket(port);}

void NetworkServer::stopServer()
  {stop();}

InterprocessConnection* NetworkServer::createConnectionObject()
  {return createNetworkClient();}
