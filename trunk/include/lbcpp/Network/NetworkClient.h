/*-----------------------------------------.---------------------------------.
| Filename: NetworkClient.h                | Network Client                  |
| Author  : Julien Becker                  |                                 |
| Started : 01/12/2010 18:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NETWORK_CLIENT_H_
# define LBCPP_NETWORK_CLIENT_H_

# include <lbcpp/Core/Object.h>
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Network/WorkUnitNetworkRequest.h>

namespace lbcpp
{

/** NetworkClient **/
class NetworkClient : public Object, public InterprocessConnection
{
public:
  NetworkClient(ExecutionContext& context);

  bool startClient(const String& host, int port);
  void stopClient();

  bool sendVariable(const Variable& variable);
  virtual void variableReceived(const Variable& variable) = 0;

  ExecutionContext& getContext()
    {return context;}

  /* InterprocessConnection */
  virtual void connectionMade() {}
  virtual void connectionLost() {}

  lbcpp_UseDebuggingNewOperator
  
protected:
  enum {magicNumber = 0xdeadface};
  ExecutionContext& context;

  virtual void messageReceived(const juce::MemoryBlock& message);
};

typedef ReferenceCountedObjectPtr<NetworkClient> NetworkClientPtr;

/** ManagerNetworkClient **/
class ManagerNetworkClientCallback
{
public:
  virtual ~ManagerNetworkClientCallback() {}

  virtual void workUnitAcknowledgementReceived(size_t sourceIdentifier, const String& uniqueIdentifier) = 0;
  virtual void workUnitResultReceived(const String& uniqueIdentifier, const Variable& result) = 0;
};

typedef ManagerNetworkClientCallback* ManagerNetworkClientCallbackPtr;

class ManagerNetworkClient : public NetworkClient
{
public:
  ManagerNetworkClient(ExecutionContext& context, const ManagerNetworkClientCallbackPtr& callback)
    : NetworkClient(context), callback(callback) {}

  bool sendWorkUnit(size_t sourceIdentifier, const WorkUnitPtr& workUnit,
                    const String& projectName, const String& source, const String& destination,
                    size_t requiredCpus, size_t requiredMemory, size_t requiredTime);

protected:
  ManagerNetworkClientCallbackPtr callback;

  virtual void variableReceived(const Variable& variable);
};

typedef ReferenceCountedObjectPtr<ManagerNetworkClient> ManagerNetworkClientPtr;

/** GridNetworkClient **/
class GridNetworkClientCallback
{
public:
  virtual ~GridNetworkClientCallback() {}

  virtual void workUnitRequestsReceived(const std::vector<WorkUnitNetworkRequestPtr>& workUnitRequests) = 0;
};

typedef GridNetworkClientCallback* GridNetworkClientCallbackPtr;

class GridNetworkClient : public NetworkClient
{
public:
  GridNetworkClient(ExecutionContext& context)
    : NetworkClient(context) {}

  virtual void askForWorkUnits(const String& gridName) = 0;
  virtual void sendFinishedTraces() = 0;
  virtual void waitResponses(juce::int64 timeout) = 0;
  virtual void closeCommunication() = 0;

protected:
  GridNetworkClientCallbackPtr callback;

  virtual void variableReceived(const Variable& variable);
};

typedef ReferenceCountedObjectPtr<GridNetworkClient> GridNetworkClientPtr;

extern GridNetworkClientPtr localGridNetworkClient(ExecutionContext& context);
extern GridNetworkClientPtr sgeGridNetworkClient(ExecutionContext& context);
/** Utility **/
extern bool isValidNetworkMessage(ExecutionContext& context, const Variable& variable);

};

#endif //!LBCPP_NETWORK_CLIENT_H_
