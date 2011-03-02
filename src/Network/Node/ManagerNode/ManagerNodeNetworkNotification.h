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

class IsWorkUnitFinishedNotification : public ManagerNodeNetworkNotification
{
public:
  IsWorkUnitFinishedNotification(const String& identifier)
    : identifier(identifier) {}
  IsWorkUnitFinishedNotification() {}
  
  virtual void notifyManagerNodeNetwork(const ManagerNodeNetworkInterfacePtr& target)
  {
    bool res = target->isFinished(identifier);
    target->getNetworkClient()->sendVariable(res);
  }
  
protected:
  friend class IsWorkUnitFinishedNotificationClass;
  
  String identifier;
};

class GetExecutionTraceNotification : public ManagerNodeNetworkNotification
{
public:
  GetExecutionTraceNotification(const String& identifier)
    : identifier(identifier) {}
  GetExecutionTraceNotification() {}
  
  virtual void notifyManagerNodeNetwork(const ManagerNodeNetworkInterfacePtr& target)
  {
    NetworkResponsePtr res = target->getExecutionTrace(identifier);
    target->getNetworkClient()->sendVariable(res);
  }

protected:
  friend class GetExecutionTraceNotificationClass;
  
  String identifier;
};

}; /* namespace */
  
#endif // !LBCPP_MANAGER_NODE_NETWORK_NOTIFICATION_H_
