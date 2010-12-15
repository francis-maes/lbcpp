
#include <lbcpp/lbcpp.h>
#include "ClientServerWorkUnit.h"

namespace lbcpp
{

class SgeExecutionContext : public ExecutionContext
{
public:
  SgeExecutionContext(const File& workUnitDirectory = File::getCurrentWorkingDirectory())
  : workUnitDirectory(workUnitDirectory), lastFileID(0) {}
  
  virtual bool isMultiThread() const
  {return false;}
  
  virtual bool isCanceled() const
  {return false;}
  
  virtual bool isPaused() const
  {return false;}
  
  virtual bool run(const CompositeWorkUnitPtr& workUnits)
  {return run((WorkUnitPtr)workUnits);}
  
  virtual void pushWorkUnit(const WorkUnitPtr& workUnit)
  {
    /* Save work unit */
    File workUnitFile = workUnitDirectory.getChildFile(String(getFileID()) + T(".workUnit"));
    if (workUnitFile.exists())
    {
      warningCallback(T("SgeExecutionContext::pushWorkUnit"), T("File ") + workUnitFile.getFileName().quoted() + T(" already exists"));
      workUnitFile.deleteFile();
    }
    workUnit->saveToFile(*this, workUnitFile);
  }
  
  lbcpp_UseDebuggingNewOperator
  
protected:
  File workUnitDirectory;
  juce::int64 lastFileID;
  
  juce::int64 getFileID()
  {
    juce::int64 res = Time::currentTimeMillis();
    if (res == lastFileID)
    {
      juce::Thread::sleep(1);
      res = Time::currentTimeMillis();
    }
    lastFileID = res;
    return res;
  }
};

};

using namespace lbcpp;

namespace lbcpp
{
  extern LibraryPtr proteinLibrary;
  extern LibraryPtr programLibrary;
};

int main(int argc, char** argv)
{
  lbcpp::initialize(argv[0]);
  ExecutionContextPtr context = new SgeExecutionContext(File(T("/u/jbecker/.WorkUnit/Waiting")));
  lbcpp::importLibrary(proteinLibrary);
  lbcpp::importLibrary(programLibrary);
  
  int exitCode;
  {
    exitCode = WorkUnit::main(*context, new ClientWorkUnit(), argc, argv);
  }

  lbcpp::deinitialize();
  return exitCode;
}
