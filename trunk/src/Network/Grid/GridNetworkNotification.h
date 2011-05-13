/*-----------------------------------------.---------------------------------.
| Filename: GridNetworkNotification.h  | Grid  Network Notification  |
| Author  : Julien Becker                  |                                 |
| Started : 26/02/2011 19:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRID_NETWORK_NOTIFICATION_H_
# define LBCPP_GRID_NETWORK_NOTIFICATION_H_

# include <lbcpp/Network/NetworkNotification.h>

namespace lbcpp
{

class GridNetworkNotification : public NetworkNotification
{
public:
  virtual void notifyNetwork(const NetworkInterfacePtr& target)
    {notifyGridNetwork(target);}

  virtual void notifyGridNetwork(const GridNetworkInterfacePtr& target) = 0;
};

class PushWorkUnitsNotification : public GridNetworkNotification
{
public:
  PushWorkUnitsNotification(ContainerPtr networkRequests) : networkRequests(networkRequests) {}
  PushWorkUnitsNotification() {}
  
  virtual void notifyGridNetwork(const GridNetworkInterfacePtr& target)
  {
    ContainerPtr res = target->pushWorkUnits(networkRequests);
    target->getNetworkClient()->sendVariable(res);
  }

protected:
  friend class PushWorkUnitsNotificationClass;
  
  ContainerPtr networkRequests;
};

class GetFinishedExecutionTraces : public GridNetworkNotification
{
public:
  virtual void notifyGridNetwork(const GridNetworkInterfacePtr& target)
  {
    ContainerPtr res = target->getFinishedExecutionTraces();
    if (!target->getNetworkClient()->sendVariable(res))
    {
      target->getContext().warningCallback(T("GetFinishedExecutionTraces"), T("Trace not sent"));
      return;
    }
    /* Short way that avoid to use network trafic */
    bool ack = false;
    if (!target->getNetworkClient()->receiveBoolean(300000, ack) || !ack)
      return;
    target->removeExecutionTraces(res);
  }
};


};

#endif //!LBCPP_GRID_NETWORK_NOTIFICATION_H_
