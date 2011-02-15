/*-----------------------------------------.---------------------------------.
| Filename: NetworkNotification.h          | Network Notification            |
| Author  : Julien Becker                  |                                 |
| Started : 01/02/2011 19:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NETWORK_NOTIFICATION_H_
# define LBCPP_NETWORK_NOTIFICATION_H_

# include <lbcpp/Network/NetworkInterface.h>
# include <lbcpp/Execution/Notification.h>

namespace lbcpp
{

class NetworkNotification : public Notification
{
public:
  virtual void notify(const ObjectPtr& target);  
  virtual void notifyNetwork(const NetworkInterfacePtr& target) = 0;
};

class NodeNetworkNotification : public NetworkNotification
{
public:
  virtual void notifyNetwork(const NetworkInterfacePtr& target);  
  virtual void notifyNodeNetwork(const NodeNetworkInterfacePtr& target) = 0;
};

class CloseCommunicationNotification : public NetworkNotification
{
public:
  virtual void notifyNetwork(const NetworkInterfacePtr& target);
};

class GetNodeNameNotification : public NodeNetworkNotification
{
public:
  virtual void notifyNodeNetwork(const NodeNetworkInterfacePtr& target);
};

class PushWorkUnitNotification : public NodeNetworkNotification
{
public:
  PushWorkUnitNotification(WorkUnitNetworkRequestPtr request) : request(request) {}
  PushWorkUnitNotification() {}
  
  virtual void notifyNodeNetwork(const NodeNetworkInterfacePtr& target);
  
protected:
  friend class PushWorkUnitNotificationClass;
  
  WorkUnitNetworkRequestPtr request;
};

class GetWorkUnitStatusNotification : public NodeNetworkNotification
{
public:
  GetWorkUnitStatusNotification(NetworkRequestPtr request) : request(request) {}
  GetWorkUnitStatusNotification() {}
  
  virtual void notifyNodeNetwork(const NodeNetworkInterfacePtr& target);

protected:
  friend class GetWorkUnitStatusNotificationClass;

  NetworkRequestPtr request;
};

class GetExecutionTraceNotification : public NodeNetworkNotification
{
public:
  GetExecutionTraceNotification(NetworkRequestPtr request) : request(request) {}
  GetExecutionTraceNotification() {}
  
  virtual void notifyNodeNetwork(const NodeNetworkInterfacePtr& target);
  
protected:
  friend class GetExecutionTraceNotificationClass;
  
  NetworkRequestPtr request;
};

}; /* namespace */
  
#endif // !LBCPP_NETWORK_NOTIFICATION_H_
