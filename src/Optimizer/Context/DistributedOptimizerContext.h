/*
 *  DistributedOptimizerContext.h
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 3/04/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef LBCPP_DISTRIBUTED_OPTIMIZER_CONTEXT_H_
# define LBCPP_DISTRIBUTED_OPTIMIZER_CONTEXT_H_

# include <lbcpp/Optimizer/OptimizerContext.h>
# include <lbcpp/Network/NetworkClient.h>
# include "../src/Network/Node/ManagerNode/ManagerNodeNetworkInterface.h"

namespace lbcpp
{

class DistributedOptimizerContext : public OptimizerContext
{
public:
  DistributedOptimizerContext(String projectName, String source, String destination, String managerHostName, size_t managerPort, size_t requiredCpus, size_t requiredMemory, size_t requiredTime)
    : projectName(projectName), source(source), destination(destination), managerHostName(managerHostName), managerPort(managerPort), 
      requiredCpus(requiredCpus), requiredMemory(requiredMemory), requiredTime(requiredTime) {} 
  DistributedOptimizerContext() {}
  
  virtual juce::int64 evaluate(const Variable& parameters)
  {
    /*ManagerNodeNetworkInterfacePtr interface = getNetworkInterfaceAndConnect(context);
    if (!interface)
      continue;
    WorkUnitPtr wu = state->generateSampleWU(context);
    String res = sendWU(context, wu, interface);
    
    if (res == T("Error"))
    {
      context.errorCallback(T("SendWorkUnit::run"), T("Trouble - We didn't correclty receive the acknowledgement"));
      break;
    }
    interface->closeCommunication();*/
    return 0;
  }
  
protected:  
  friend class DistributedOptimizerContextClass;
  
private:
  String projectName;
  String source;
  String destination;
  String managerHostName;
  size_t managerPort;
  size_t requiredCpus;
  size_t requiredMemory;
  size_t requiredTime;
  
  ManagerNodeNetworkInterfacePtr getNetworkInterfaceAndConnect(ExecutionContext& context) const
  {       
    NetworkClientPtr client = blockingNetworkClient(context);
    if (!client->startClient(managerHostName, managerPort))
    {
      context.errorCallback(T("SendWorkUnit::run"), T("Not connected !"));
      return NULL;
    }
    context.informationCallback(managerHostName, T("Connected !"));
    ManagerNodeNetworkInterfacePtr interface = new ClientManagerNodeNetworkInterface(context, client, source);
    interface->sendInterfaceClass();
    return interface;
  }
  
  String sendWU(ExecutionContext& context, WorkUnitPtr wu, ManagerNodeNetworkInterfacePtr interface) const
  {    
    NetworkRequestPtr request = new NetworkRequest(context, projectName, source, destination, wu, requiredCpus, requiredMemory, requiredTime);
    return interface->pushWorkUnit(request);
  }
  
};
  
};
#endif // !LBCPP_DISTRIBUTED_OPTIMIZER_CONTEXT_H_