/*-----------------------------------------.---------------------------------.
| Filename: NodeNetworkInterface.h         | Node Network Interface          |
| Author  : Julien Becker                  |                                 |
| Started : 24/02/2011 23:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NODE_NETWORK_INTERFACE_H_
# define LBCPP_NODE_NETWORK_INTERFACE_H_

# include <lbcpp/Network/NetworkInterface.h>

namespace lbcpp
{

class NodeNetworkInterface : public NetworkInterface
{
public:
  NodeNetworkInterface(ExecutionContext& context, const String& nodeName = String::empty)
    : NetworkInterface(context), nodeName(nodeName) {}
  NodeNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName = String::empty)
    : NetworkInterface(context, client), nodeName(nodeName) {}
  NodeNetworkInterface() {}

  String getNodeName() const
    {return nodeName;}

protected:
  friend class NodeNetworkInterfaceClass;
  
  String nodeName;
};

typedef ReferenceCountedObjectPtr<NodeNetworkInterface> NodeNetworkInterfacePtr;

}; /* namespace */

#endif // !LBCPP_NODE_NETWORK_INTERFACE_H_
