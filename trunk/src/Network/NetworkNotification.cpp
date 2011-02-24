/*-----------------------------------------.---------------------------------.
| Filename: NetworkNotification.h          | Network Notification            |
| Author  : Julien Becker                  |                                 |
| Started : 01/02/2011 19:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "NetworkNotification.h"

using namespace lbcpp;

/*
 ** NetworkNotification
 */
void NetworkNotification::notify(const ObjectPtr& target)
  {notifyNetwork(target);}

void NodeNetworkNotification::notifyNetwork(const NetworkInterfacePtr& target)
  {notifyNodeNetwork(target);}

void CloseCommunicationNotification::notifyNetwork(const NetworkInterfacePtr& target)
  {target->closeCommunication(target->getContext());}

void GetNodeNameNotification::notifyNodeNetwork(const NodeNetworkInterfacePtr& target)
  {target->getNetworkClient()->sendVariable(Variable(target->getNodeName(target->getContext()), stringType));}


void PushWorkUnitNotification::notifyNodeNetwork(const NodeNetworkInterfacePtr& target)
{
  WorkUnitInformationPtr res = target->pushWorkUnit(target->getContext(), request);
  target->getNetworkClient()->sendVariable(res);
}

void GetWorkUnitStatusNotification::notifyNodeNetwork(const NodeNetworkInterfacePtr& target)
{
  int res = target->getWorkUnitStatus(target->getContext(), information);
  target->getNetworkClient()->sendVariable(Variable(res, integerType));
}

void GetExecutionTraceNotification::notifyNodeNetwork(const NodeNetworkInterfacePtr& target)
{
  NetworkResponsePtr res = target->getExecutionTrace(target->getContext(), information);
  target->getNetworkClient()->sendVariable(res);
}
