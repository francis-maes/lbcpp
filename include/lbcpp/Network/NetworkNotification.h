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
  virtual void notify(const ObjectPtr& target)
    {notifyNetwork(target);}

  virtual void notifyNetwork(const NetworkInterfacePtr& target) = 0;
};

}; /* namespace */
  
#endif // !LBCPP_NETWORK_NOTIFICATION_H_
