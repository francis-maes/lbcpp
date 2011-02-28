/*-----------------------------------------.---------------------------------.
| Filename: ManagerNodeNetworkInterface.cpp| Manager Node Network Interface  |
| Author  : Julien Becker                  |                                 |
| Started : 26/02/2011 19:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

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
  if (!client->receiveString(300000, res))
    context.warningCallback(request->getSource(), T("ClientManagerNodeNetworkInterface::pushWorkUnit"));
  return res;
}
