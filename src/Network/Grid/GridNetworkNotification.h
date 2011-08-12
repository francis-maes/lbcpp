/*-----------------------------------------.---------------------------------.
| Filename: GridNetworkNotification.h  | Grid  Network Notification  |
| Author  : Julien Becker                  |                                 |
| Started : 26/02/2011 19:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRID_NETWORK_NOTIFICATION_H_
# define LBCPP_GRID_NETWORK_NOTIFICATION_H_
/*
# include <lbcpp/Network/NetworkNotification.h>

namespace lbcpp
{

class GridNetworkNotification : public NetworkNotification
{
public:
  virtual void notifyNetwork(const NetworkInterfacePtr& target, const NetworkClientPtr& client)
  {
    if (!client)
      return;
    notifyGridNetwork(target, client);
  }

  virtual void notifyGridNetwork(const GridNetworkInterfacePtr& target, const NetworkClientPtr& client) = 0;
};

class PushWorkUnitsNotification : public GridNetworkNotification
{
public:
  PushWorkUnitsNotification(ContainerPtr networkRequests) : networkRequests(networkRequests) {}
  PushWorkUnitsNotification() {}
  
  virtual void notifyGridNetwork(const GridNetworkInterfacePtr& target, const NetworkClientPtr& client)
  {
    ContainerPtr res = target->pushWorkUnits(networkRequests);
    client->sendVariable(res);
  }

protected:
  friend class PushWorkUnitsNotificationClass;
  
  ContainerPtr networkRequests;
};

class GetFinishedExecutionTraces : public GridNetworkNotification
{
public:
  virtual void notifyGridNetwork(const GridNetworkInterfacePtr& target, const NetworkClientPtr& client)
  {
    ContainerPtr res = target->getFinishedExecutionTraces();
    if (!client->sendVariable(res))
    {
      target->getContext().warningCallback(T("GetFinishedExecutionTraces"), T("Trace not sent"));
      return;
    }
    // Short way that avoid to use network trafic
    bool ack = false;
    if (!client->receiveBoolean(3000, ack) || !ack)
      return;
    target->removeExecutionTraces(res);
  }
};


};
*/
#endif //!LBCPP_GRID_NETWORK_NOTIFICATION_H_
