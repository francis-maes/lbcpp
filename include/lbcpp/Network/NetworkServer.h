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

namespace lbcpp
{

class NetworkServer : public InterprocessConnectionServer, public Object
{
public:
  NetworkServer(ExecutionContext& context) : context(context) {}
  
  bool startServer(int port);
  void stopServer();

  NetworkClientPtr acceptClient(juce::int64 timeout);

  lbcpp_UseDebuggingNewOperator

protected:
  /* InterprocessConnectionServer */
  virtual InterprocessConnection* createConnectionObject();

protected:
  enum {magicNumber = 0xdeadface};
  ExecutionContext& context;
  std::deque<NetworkClientPtr> acceptedClients;
  CriticalSection lock;
  
  NetworkClientPtr popClient();
};

typedef ReferenceCountedObjectPtr<NetworkServer> NetworkServerPtr;

};

#endif //!LBCPP_NETWORK_SERVER_H_
