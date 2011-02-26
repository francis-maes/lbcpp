/*-----------------------------------------.---------------------------------.
| Filename: ManagerNodeNetworkNotifica...h | Manager Node Network            |
| Author  : Julien Becker                  |   Notification                  |
| Started : 26/02/2011 19:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MANAGER_NODE_NETWORK_NOTIFICATION_H_
# define LBCPP_MANAGER_NODE_NETWORK_NOTIFICATION_H_

# include "../NodeNetworkNotification.h"

namespace lbcpp
{

class ManagerNodeNetworkNotification : public NodeNetworkNotification
{
public:
  virtual void notifyNodeNetwork(const NodeNetworkInterfacePtr& target)
    {notifyManagerNodeNetwork(target);}

  virtual void notifyManagerNodeNetwork(const ManagerNodeNetworkInterfacePtr& target) = 0;
};

class PushWorkUnitNotification : public ManagerNodeNetworkNotification
{
public:
  PushWorkUnitNotification(NetworkRequestPtr request)
    : request(request) {}
  PushWorkUnitNotification() {}

  virtual void notifyManagerNodeNetwork(const ManagerNodeNetworkInterfacePtr& target)
  {
    String res = target->pushWorkUnit(request);
    target->getNetworkClient()->sendVariable(res);
  }
protected:
  friend class PushWorkUnitNotificationClass;

  NetworkRequestPtr request;
};

}; /* namespace */
  
#endif // !LBCPP_MANAGER_NODE_NETWORK_NOTIFICATION_H_
