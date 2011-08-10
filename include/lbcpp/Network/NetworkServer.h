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

class NetworkServer : public Object, public InterprocessConnectionServer
{
public:
  NetworkServer(ExecutionContext& context)
    : context(context) {}
  NetworkServer()
    : context(*(ExecutionContext*)NULL) {}
  virtual ~NetworkServer() {}

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

class XxxNetworkServer : public Object, public InterprocessConnectionServer
{
public:
  XxxNetworkServer(ExecutionContext& context)
    : context(context) {}

  bool startServer(int port);
  void stopServer();

  virtual XxxNetworkClient* createNetworkClient() = 0;

  lbcpp_UseDebuggingNewOperator

protected:
  /* InterprocessConnectionServer */
  virtual InterprocessConnection* createConnectionObject();

protected:
  enum {magicNumber = 0xdeadface};
  ExecutionContext& context;
};

typedef ReferenceCountedObjectPtr<XxxNetworkServer> XxxNetworkServerPtr;

};

#endif //!LBCPP_NETWORK_SERVER_H_
