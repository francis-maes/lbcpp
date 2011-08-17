/*-----------------------------------------.---------------------------------.
| Filename: NetworkServer.h                | Network Server                  |
| Author  : Julien Becker                  |                                 |
| Started : 01/12/2010 18:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NETWORK_SERVER_H_
# define LBCPP_NETWORK_SERVER_H_

# include <lbcpp/Network/NetworkClient.h>
# include <lbcpp/Network/Manager.h>

namespace lbcpp
{

class NetworkServer : public Object, public InterprocessConnectionServer
{
public:
  NetworkServer(ExecutionContext& context)
    : context(context) {}

  bool startServer(int port);
  void stopServer();

  virtual NetworkClient* createNetworkClient() = 0;

  lbcpp_UseDebuggingNewOperator

protected:
  /* InterprocessConnectionServer */
  virtual InterprocessConnection* createConnectionObject();

protected:
  enum {magicNumber = 0xdeadface};
  ExecutionContext& context;
};

typedef ReferenceCountedObjectPtr<NetworkServer> NetworkServerPtr;

extern NetworkServerPtr managerNetworkServer(ExecutionContext& context, const ManagerPtr& manager);

}; /* namespace lbcpp */

#endif //!LBCPP_NETWORK_SERVER_H_
