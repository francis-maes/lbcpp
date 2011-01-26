#include "NetworkCommand.h"

namespace lbcpp
{

class ServerWorkUnit : public WorkUnit
{
public:
  ServerWorkUnit(File workUnitDirectory = File::getCurrentWorkingDirectory())
    : workUnitDirectory(workUnitDirectory), hostname(T("monster24.montefiore.ulg.ac.be")), serverName(T("unnamed")) {}
  
  virtual bool run(ExecutionContext& context)
  {
    ServerNetworkContextPtr networkContext = new SgeServerNetworkContext(getName(), hostname, 1664, workUnitDirectory);    
    networkContext->run(context);
    return true;
  }

protected:
  friend class ServerWorkUnitClass;

  File workUnitDirectory;
  String hostname;
  String serverName;
};

};