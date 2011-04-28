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

DistributedOptimizerContext::DistributedOptimizerContext(String projectName, String source, String destination, String managerHostName, size_t managerPort, size_t requiredCpus, size_t requiredMemory, size_t requiredTime)
: projectName(projectName), source(source), destination(destination), managerHostName(managerHostName), managerPort(managerPort), 
requiredCpus(requiredCpus), requiredMemory(requiredMemory), requiredTime(requiredTime) 
{
  (new GetFinishedExecutionTracesDaemon(this))->startThread();
} 
