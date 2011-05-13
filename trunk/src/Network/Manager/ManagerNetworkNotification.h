/*-----------------------------------------.---------------------------------.
| Filename: ManagerNetworkNotification.h   | Manager  Network                |
| Author  : Julien Becker                  |   Notification                  |
| Started : 26/02/2011 19:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MANAGER_NETWORK_NOTIFICATION_H_
# define LBCPP_MANAGER_NETWORK_NOTIFICATION_H_

# include <lbcpp/Network/NetworkNotification.h>

namespace lbcpp
{

class ManagerNetworkNotification : public NetworkNotification
{
public:
  virtual void notifyNetwork(const NetworkInterfacePtr& target, const NetworkClientPtr& client)
  {
    if (!client)
      return;
    notifyManagerNetwork(target, client);
  }

  virtual void notifyManagerNetwork(const ManagerNetworkInterfacePtr& target, const NetworkClientPtr& client) = 0;
};

class PushWorkUnitNotification : public ManagerNetworkNotification
{
public:
  PushWorkUnitNotification(WorkUnitNetworkRequestPtr request)
    : request(request) {}
  PushWorkUnitNotification() {}

  virtual void notifyManagerNetwork(const ManagerNetworkInterfacePtr& target, const NetworkClientPtr& client)
  {
    String res = target->pushWorkUnit(request);
    client->sendVariable(res);
  }
protected:
  friend class PushWorkUnitNotificationClass;

  WorkUnitNetworkRequestPtr request;
};

class IsWorkUnitFinishedNotification : public ManagerNetworkNotification
{
public:
  IsWorkUnitFinishedNotification(const String& identifier)
    : identifier(identifier) {}
  IsWorkUnitFinishedNotification() {}
  
  virtual void notifyManagerNetwork(const ManagerNetworkInterfacePtr& target, const NetworkClientPtr& client)
  {
    bool res = target->isFinished(identifier);
    client->sendVariable(res);
  }
  
protected:
  friend class IsWorkUnitFinishedNotificationClass;
  
  String identifier;
};

class GetExecutionTraceNotification : public ManagerNetworkNotification
{
public:
  GetExecutionTraceNotification(const String& identifier)
    : identifier(identifier) {}
  GetExecutionTraceNotification() {}
  
  virtual void notifyManagerNetwork(const ManagerNetworkInterfacePtr& target, const NetworkClientPtr& client)
  {
    ExecutionTraceNetworkResponsePtr res = target->getExecutionTrace(identifier);
    client->sendVariable(res);
  }

protected:
  friend class GetExecutionTraceNotificationClass;
  
  String identifier;
};

}; /* namespace */
  
#endif // !LBCPP_MANAGER_NETWORK_NOTIFICATION_H_
