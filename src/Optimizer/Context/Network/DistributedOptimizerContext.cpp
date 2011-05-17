/*
 *  DistributedOptimizerContext.cpp
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 28/04/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#include "precompiled.h"
# include "DistributedOptimizerContext.h"

using namespace lbcpp;
// TODO arnaud : use validationFunction
DistributedOptimizerContext::DistributedOptimizerContext(ExecutionContext& context, const FunctionPtr& objectiveFunction, String projectName, String source, String destination, String managerHostName, size_t managerPort, size_t requiredCpus, size_t requiredMemory, size_t requiredTime, size_t timeToSleep)
: OptimizerContext(context, objectiveFunction, FunctionPtr(), timeToSleep), projectName(projectName), source(source), destination(destination), managerHostName(managerHostName), managerPort(managerPort), 
requiredCpus(requiredCpus), requiredMemory(requiredMemory), requiredTime(requiredTime)
{
  getFinishedTracesThread = new GetFinishedExecutionTracesDaemon(this);
  getFinishedTracesThread->startThread();
} 

void DistributedOptimizerContext::removePostEvaluationCallback(const FunctionCallbackPtr& callback)
{
  functionCallback = NULL;
  getFinishedTracesThread->signalThreadShouldExit();
  while (getFinishedTracesThread->isThreadRunning())
    Thread::sleep(100);
  delete getFinishedTracesThread;
}