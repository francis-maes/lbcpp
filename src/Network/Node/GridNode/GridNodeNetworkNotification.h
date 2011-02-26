/*-----------------------------------------.---------------------------------.
| Filename: GridNodeNetworkNotification.h  | Grid Node Network Notification  |
| Author  : Julien Becker                  |                                 |
| Started : 26/02/2011 19:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRID_NODE_NETWORK_NOTIFICATION_H_
# define LBCPP_GRID_NODE_NETWORK_NOTIFICATION_H_

# include "../NodeNetworkNotification.h"

namespace lbcpp
{

class GridNodeNetworkNotification : public NodeNetworkNotification
{
public:
  virtual void notifyNodeNetwork(const NodeNetworkInterfacePtr& target)
    {notifyGridNodeNetwork(target);}

  virtual void notifyGridNodeNetwork(const GridNodeNetworkInterfacePtr& target) = 0;
};

class PushWorkUnitsNotification : public GridNodeNetworkNotification
{
public:
  PushWorkUnitsNotification(ContainerPtr networkRequests) : networkRequests(networkRequests) {}
  PushWorkUnitsNotification() {}
  
  virtual void notifyGridNodeNetwork(const GridNodeNetworkInterfacePtr& target)
  {
    ContainerPtr res = target->pushWorkUnits(networkRequests);
    target->getNetworkClient()->sendVariable(res);
  }

protected:
  friend class PushWorkUnitsNotificationClass;
  
  ContainerPtr networkRequests;
};

class GetFinishedExecutionTraces : public GridNodeNetworkNotification
{
public:
  virtual void notifyGridNodeNetwork(const GridNodeNetworkInterfacePtr& target)
  {
    ContainerPtr res = target->getFinishedExecutionTraces();
    target->getNetworkClient()->sendVariable(res);
    /* Short way that avoid to use network trafic */
    bool ack = false;
    if (!target->getNetworkClient()->receiveBoolean(10000, ack) || !ack)
      return;
    target->removeExecutionTraces(res);
  }
};


};

#endif //!LBCPP_GRID_NODE_NETWORK_NOTIFICATION_H_
