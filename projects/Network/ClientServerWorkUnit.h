/*******************************************************************************
 ! WARNING !   Test Zone - Do not enter !
 *******************************************************************************/

#include "NetworkCommand.h"

namespace lbcpp
{
  
class DumbWorkUnit : public WorkUnit
{
public:
  DumbWorkUnit() {}
  
  virtual Variable run(ExecutionContext& context)
  {
    for (size_t i = 0; i < 10; ++i)
    {
      context.progressCallback(new ProgressionState(i+1, 10, T("DumbWorkUnit")));
      juce::Thread::sleep(1000);
    }
    return Variable();
  }
};

class ClientWorkUnit : public WorkUnit
{
public:
  ClientWorkUnit() : clientName(T("jbecker-localhost")), hostName(T("monster24.montefiore.ulg.ac.be")), port(1664) {}
  
  virtual Variable run(ExecutionContext& context)
  {
    NetworkClientPtr client = blockingNetworkClient(context);
    
    if (!client->startClient(hostName, port))
    {
      context.errorCallback(T("ClientWorkUnit::run"), T("Not connected !"));
      return Variable();
    }

    NodeNetworkInterfacePtr interface = new ClientNodeNetworkInterface(context, client, clientName);
    interface->sendInterfaceClass();

    /* Submit jobs */
    for (size_t i = 0; i < 1; ++i)
    {
      WorkUnitNetworkRequestPtr request = new WorkUnitNetworkRequest(new DumbWorkUnit(), T("testProject"), clientName, T("LocalGridNode"));
      NetworkRequestPtr res = interface->pushWorkUnit(context, request);
      interface->getWorkUnitStatus(context, res);
    }
    
    interface->closeCommunication(context);

    return Variable();
  }

protected:
  friend class ClientWorkUnitClass;

  String clientName;
  String hostName;
  size_t port;
};

}; /* namespace lbcpp */
