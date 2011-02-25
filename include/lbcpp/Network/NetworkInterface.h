/*-----------------------------------------.---------------------------------.
| Filename: NetworkInterface.h             | Network Interface               |
| Author  : Julien Becker                  |                                 |
| Started : 01/02/2011 19:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NETWORK_INTERFACE_H_
# define LBCPP_NETWORK_INTERFACE_H_

# include <lbcpp/Network/NetworkClient.h>

namespace lbcpp
{

class NetworkInterface : public Object
{
public:
  NetworkInterface(ExecutionContext& context)
    : context(context) {}
  NetworkInterface(ExecutionContext& context, NetworkClientPtr client)
    : context(context), client(client) {}
  NetworkInterface()
    : context(*(ExecutionContext*)NULL) {}

  void setContext(ExecutionContext& context)
    {this->context = context;}

  ExecutionContext& getContext() const
    {return context;}

  void setNetworkClient(NetworkClientPtr client)
    {this->client = client;}

  NetworkClientPtr getNetworkClient() const
    {return client;}

  virtual void sendInterfaceClass()
    {client->sendVariable(getClassName());}

  virtual void closeCommunication(ExecutionContext& context)
    {client->stopClient();}

protected:
  friend class NetworkInterfaceClass;

  ExecutionContext& context;
  NetworkClientPtr client;
};

typedef ReferenceCountedObjectPtr<NetworkInterface> NetworkInterfacePtr;

}; /* namespace */

#endif // !LBCPP_NETWORK_INTERFACE_H_
