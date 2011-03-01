/*-----------------------------------------.---------------------------------.
| Filename: NodeNetworkNotification.h      | Node Network Notification       |
| Author  : Julien Becker                  |                                 |
| Started : 01/02/2011 19:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NODE_NETWORK_NOTIFICATION_H_
# define LBCPP_NODE_NETWORK_NOTIFICATION_H_

# include <lbcpp/Network/NetworkNotification.h>
# include "NodeNetworkInterface.h"

namespace lbcpp
{

class NodeNetworkNotification : public NetworkNotification
{
public:
  virtual void notifyNetwork(const NetworkInterfacePtr& target)
    {notifyNodeNetwork(target);}

  virtual void notifyNodeNetwork(const NodeNetworkInterfacePtr& target) = 0;
};

}; /* namespace */
  
#endif // !LBCPP_NODE_NETWORK_NOTIFICATION_H_
