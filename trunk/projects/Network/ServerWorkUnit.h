#include "NetworkCommand.h"

namespace lbcpp
{

class ServerWorkUnit : public WorkUnit
{
public:
  ServerWorkUnit() : hostname(T("192.168.1.3")){}//hostname(T("monster.montefiore.ulg.ac.be")) {}
  
  virtual bool run(ExecutionContext& context)
  {
    NetworkContextPtr networkContext = new NetworkContext(T("jbecker-server-mac"), hostname, 1664);    
    networkContext->run(context);
    return true;
  }
  
protected:
  friend class ServerWorkUnitClass;

  String hostname;
  std::deque<WorkUnitPtr> units;
};

};