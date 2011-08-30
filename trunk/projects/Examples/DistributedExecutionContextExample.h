/*-----------------------------------------.---------------------------------.
| Filename: DistributedExecutionContex...h | Illustrates Distributed         |
| Author  : Julien Becker                  |       Execution Context         |
| Started : 23/08/2011 10:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXAMPLES_DISTRIBUTED_EXECUTION_CONTEXT_H_
# define LBCPP_EXAMPLES_DISTRIBUTED_EXECUTION_CONTEXT_H_

# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

class DumbWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    for (size_t i = 0; i < 10; ++i)
    {
      context.progressCallback(new ProgressionState(i+1, 10, T("DumbWorkUnit")));
      juce::Thread::sleep(context.getRandomGenerator()->sampleSize(1000, 5000));
    }

    return Variable(context.getRandomGenerator()->sampleSize(100), positiveIntegerType);
  }
};

class TestExecutionContextCallback : public ExecutionContextCallback
{
public:
  TestExecutionContextCallback(ExecutionContext& context)
    : context(context) {}

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
  {
    context.informationCallback(T("TestExecutionContextCallback::workUnitFinished"), result.toString());
    delete this;
  }

protected:
  ExecutionContext& context;
};

class ClientWorkUnit : public WorkUnit
{
public:
  ClientWorkUnit(String hostName = T("localhost"), size_t hostPort = 1664)
    : hostName(hostName), hostPort(hostPort) {}

  Variable run(ExecutionContext& context)
  {
    ExecutionContextPtr remoteContext = distributedExecutionContext(context, hostName, hostPort,
                                                                    T("testProject"), T("jbecker@mac"), T("jbecker@nic3"),
                                                                    fixedResourceEstimator(1, 1, 1));

    context.informationCallback(T("ClientWorkUnit::run"), T("Use of CompositeWorkUnit"));
    CompositeWorkUnitPtr workUnits = new CompositeWorkUnit(T("A composite work unit (synchronous run)"));
    for (size_t i = 0; i < 10; ++i)
      workUnits->addWorkUnit(new DumbWorkUnit());

    Variable result = remoteContext->run(workUnits);
    context.informationCallback(result.toString());

    context.informationCallback(T("ClientWorkUnit::run"), T("Use of an ExecutionContextCallback (asynchronous run)"));
    for (size_t i = 0; i < 10; ++i)
      remoteContext->pushWorkUnit(new DumbWorkUnit(), new TestExecutionContextCallback(context));
    remoteContext->waitUntilAllWorkUnitsAreDone();

    return true;
  }

protected:
  friend class ClientWorkUnitClass;

  String hostName;
  size_t hostPort;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_DISTRIBUTED_EXECUTION_CONTEXT_H_
