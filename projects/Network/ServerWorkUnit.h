#include "NetworkCommand.h"

namespace lbcpp
{

class ServerWorkUnit : public WorkUnit
{
public:
  ServerWorkUnit(File workUnitDirectory = File::getCurrentWorkingDirectory())
    : workUnitDirectory(workUnitDirectory), hostname(T("192.168.1.3")) {}//hostname(T("monster.montefiore.ulg.ac.be")) {}
  
  virtual bool run(ExecutionContext& context)
  {
    ServerNetworkContextPtr networkContext = new SgeServerNetworkContext(T("jbecker-server-mac"), hostname, 1664, workUnitDirectory);    
    networkContext->run(context);
    return true;
  }

protected:
  friend class ServerWorkUnitClass;

  File workUnitDirectory;
  String hostname;
  std::deque<WorkUnitPtr> units;
};

};