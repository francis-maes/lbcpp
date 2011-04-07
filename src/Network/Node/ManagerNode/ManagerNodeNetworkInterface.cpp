/*-----------------------------------------.---------------------------------.
| Filename: ManagerNodeNetworkInterface.cpp| Manager Node Network Interface  |
| Author  : Julien Becker                  |                                 |
| Started : 26/02/2011 19:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "ManagerNodeNetworkInterface.h"
#include "ManagerNodeNetworkNotification.h"
using namespace lbcpp;  

void ClientManagerNodeNetworkInterface::closeCommunication()
{
  client->sendVariable(new CloseCommunicationNotification());
  client->stopClient();
  return;
}

String ClientManagerNodeNetworkInterface::pushWorkUnit(NetworkRequestPtr request)
{
  client->sendVariable(new PushWorkUnitNotification(request));
  String res = T("Error");
  if (!client->receiveString(1000, /*300000*/ res))
    context.warningCallback(request->getSource(), T("ClientManagerNodeNetworkInterface::pushWorkUnit"));
  return res;
}

bool ClientManagerNodeNetworkInterface::isFinished(const String& identifier)
{
  client->sendVariable(new IsWorkUnitFinishedNotification(identifier));
  bool res = false;
  if (!client->receiveBoolean(10000, res))
    context.warningCallback(T("ClientManagerNodeNetworkInterface::isFinished"), T("Timeout"));
  return res;
}

NetworkResponsePtr ClientManagerNodeNetworkInterface::getExecutionTrace(const String& identifier)
{
  client->sendVariable(new GetExecutionTraceNotification(identifier));
  NetworkResponsePtr res;
  if (!client->receiveObject<NetworkResponse>(300000, res))
    context.warningCallback(T("ClientManagerNodeNetworkInterface::getExecutionTrace"), T("Timeout"));
  return res;
}
