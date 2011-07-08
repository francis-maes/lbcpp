/*-----------------------------------------.---------------------------------.
| Filename: DistributedOptimizerContext.cpp| Implementation file of          |
| Author  : Arnaud Schoofs                 | DistributedOptimizerContext.h   |
| Started : 03/04/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "DistributedOptimizerContext.h"
using namespace lbcpp;

DistributedOptimizerContext::DistributedOptimizerContext(ExecutionContext& context, const FunctionPtr& objectiveFunction
                                                        , String projectName, String source, String destination
                                                        , String managerHostName, size_t managerPort
                                                        , size_t requiredCpus, size_t requiredMemory, size_t requiredTime, size_t timeToSleep)
  : OptimizerContext(context, objectiveFunction, FunctionPtr(), timeToSleep)
  , projectName(projectName), source(source), destinations(std::vector<String>(1, destination))
  , managerHostName(managerHostName), managerPort(managerPort)
  , requiredCpus(requiredCpus), requiredMemory(requiredMemory), requiredTime(requiredTime), nextDestination(0)
{
  getFinishedTracesThread = new GetFinishedExecutionTracesDaemon(this);
  getFinishedTracesThread->startThread();
}

DistributedOptimizerContext::DistributedOptimizerContext(ExecutionContext& context, const FunctionPtr& objectiveFunction
                                                        , String projectName, String source, const std::vector<String>& destinations
                                                        , String managerHostName, size_t managerPort
                                                        , size_t requiredCpus, size_t requiredMemory, size_t requiredTime, size_t timeToSleep)
  : OptimizerContext(context, objectiveFunction, FunctionPtr(), timeToSleep)
  , projectName(projectName), source(source), destinations(destinations)
  , managerHostName(managerHostName), managerPort(managerPort)
  , requiredCpus(requiredCpus), requiredMemory(requiredMemory), requiredTime(requiredTime), nextDestination(0)
{
  getFinishedTracesThread = new GetFinishedExecutionTracesDaemon(this);
  getFinishedTracesThread->startThread();
} 

void DistributedOptimizerContext::removePostEvaluationCallback(const FunctionCallbackPtr& callback)
{
  // stop and delete the GetFinishedExecutionTracesDaemon thread associated
  functionCallback = NULL;
  getFinishedTracesThread->signalThreadShouldExit();
  while (getFinishedTracesThread->isThreadRunning())
    Thread::sleep(100);
  delete getFinishedTracesThread;
}