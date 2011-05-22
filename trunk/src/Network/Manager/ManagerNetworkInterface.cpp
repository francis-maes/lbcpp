/*-----------------------------------------.---------------------------------.
| Filename: ManagerNetworkInterface.cpp    | Manager Network Interface       |
| Author  : Julien Becker                  |                                 |
| Started : 26/02/2011 19:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "ManagerNetworkInterface.h"
#include "ManagerNetworkNotification.h"

using namespace lbcpp;  

String ForwarderManagerNetworkInterface::pushWorkUnit(WorkUnitNetworkRequestPtr request)
{
  client->sendVariable(new PushWorkUnitNotification(request));
  String res = T("Error");
  if (!client->receiveString(15000/*1000*/, res))
    context.warningCallback(request->getSource(), T("ForwarderManagerNetworkInterface::pushWorkUnit"));
  return res;
}

bool ForwarderManagerNetworkInterface::isFinished(const String& identifier)
{
  client->sendVariable(new IsWorkUnitFinishedNotification(identifier));
  bool res = false;
  if (!client->receiveBoolean(10000, res))
    context.warningCallback(T("ForwarderManagerNetworkInterface::isFinished"), T("Timeout"));
  return res;
}

ExecutionTraceNetworkResponsePtr ForwarderManagerNetworkInterface::getExecutionTrace(const String& identifier)
{
  client->sendVariable(new GetExecutionTraceNotification(identifier));
  ExecutionTraceNetworkResponsePtr res;
  if (!client->receiveObject<ExecutionTraceNetworkResponse>(300000, res))
    context.warningCallback(T("ForwarderManagerNetworkInterface::getExecutionTrace"), T("Timeout"));
  return res;
}