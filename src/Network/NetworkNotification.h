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
  PushWorkUnitNotification(NetworkRequestPtr request) : request(request) {}
  PushWorkUnitNotification() {}
  
  virtual void notifyNodeNetwork(const NodeNetworkInterfacePtr& target);
  
protected:
  friend class PushWorkUnitNotificationClass;
  
  NetworkRequestPtr request;
};

class GetWorkUnitStatusNotification : public NodeNetworkNotification
{
public:
  GetWorkUnitStatusNotification(WorkUnitInformationPtr information) : information(information) {}
  GetWorkUnitStatusNotification() {}
  
  virtual void notifyNodeNetwork(const NodeNetworkInterfacePtr& target);

protected:
  friend class GetWorkUnitStatusNotificationClass;

  WorkUnitInformationPtr information;
};

class GetExecutionTraceNotification : public NodeNetworkNotification
{
public:
  GetExecutionTraceNotification(WorkUnitInformationPtr information) : information(information) {}
  GetExecutionTraceNotification() {}
  
  virtual void notifyNodeNetwork(const NodeNetworkInterfacePtr& target);
  
protected:
  friend class GetExecutionTraceNotificationClass;
  
  WorkUnitInformationPtr information;
};

}; /* namespace */
  
#endif // !LBCPP_NETWORK_NOTIFICATION_H_
