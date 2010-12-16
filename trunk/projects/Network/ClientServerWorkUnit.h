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
  
  virtual bool run(ExecutionContext& context)
  {
    for (size_t i = 0; i < 10; ++i)
    {
      context.progressCallback(i+1, 10, T("DumbWorkUnit"));
      juce::Thread::sleep(1000);
    }
    return true;
  }
};

class ClientWorkUnit : public WorkUnit
{
public:
  ClientWorkUnit() : hostname(T("192.168.1.3")) {}//hostname(T("monster.montefiore.ulg.ac.be")) {}
  
  virtual bool run(ExecutionContext& context)
  {
    ClientNetworkContextPtr networkContext = new ClientNetworkContext(T("jbecker-client-mac"), hostname, 1664);

    /* Submit jobs */
    for (size_t i = 0; i < 3; ++i)
      networkContext->pushWorkUnit(new DumbWorkUnit());

    networkContext->run(context);
    return true;
  }

protected:
  friend class ClientWorkUnitClass;

  String hostname;
};

}; /* namespace lbcpp */
