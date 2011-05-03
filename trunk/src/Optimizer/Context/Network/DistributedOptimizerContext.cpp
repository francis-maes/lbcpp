/*
 *  DistributedOptimizerContext.cpp
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 28/04/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

# include "DistributedOptimizerContext.h"

using namespace lbcpp;

DistributedOptimizerContext::DistributedOptimizerContext(const FunctionPtr& objectiveFunction, String projectName, String source, String destination, String managerHostName, size_t managerPort, size_t requiredCpus, size_t requiredMemory, size_t requiredTime)
: OptimizerContext(objectiveFunction), projectName(projectName), source(source), destination(destination), managerHostName(managerHostName), managerPort(managerPort), 
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