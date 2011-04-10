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

// TODO arnaud : clean things to compile without network

# include <lbcpp/Optimizer/OptimizerContext.h>

#ifdef LBCPP_NETWORKING
# include <lbcpp/Network/NetworkClient.h>
# include <lbcpp/Network/NetworkInterface.h>
//# include "../src/Network/Node/ManagerNode/ManagerNodeNetworkInterface.h"
#endif

namespace lbcpp
{
/*
class DistributedOptimizerContext
{
public:
  DistributedOptimizerContext(String projectName, String source, String destination, String managerHostName, size_t managerPort, size_t requiredCpus, size_t requiredMemory, size_t requiredTime)
    : projectName(projectName), source(source), destination(destination), managerHostName(managerHostName), managerPort(managerPort), 
      requiredCpus(requiredCpus), requiredMemory(requiredMemory), requiredTime(requiredTime) {} 
  DistributedOptimizerContext() {}
  
  virtual bool evaluate(const Variable& parameters, const OptimizerStatePtr& optimizerState)
  {
#ifdef LBCPP_NETWORKING
    // TODO arnaud : defaultExecutionContext
    ManagerNodeNetworkInterfacePtr interface = getNetworkInterfaceAndConnect(defaultExecutionContext());
    if (!interface)
      return false;
        
    WorkUnitPtr wu = NULL;//generateWUFromFunctionAndParameters(getObjectiveFunction(), parameters);
    String res = sendWU(defaultExecutionContext(), wu, interface);
    
    if (res == T("Error"))
    {
      defaultExecutionContext().errorCallback(T("SendWorkUnit::run"), T("Trouble - We didn't correclty receive the acknowledgement"));
      return false;
    }
    interface->closeCommunication();
    
    inProgressWUs.push_back(res); // TODO arnaud : syncrhonized
    
    return true;
#else
    return false;
#endif
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
  
  std::vector<String> inProgressWUs;  // TODO arnaud : list
#ifdef LBCPP_NETWORKING 
  ManagerNodeNetworkInterfacePtr getNetworkInterfaceAndConnect(ExecutionContext& context) const
  {       
    NetworkClientPtr client = blockingNetworkClient(context);
    if (!client->startClient(managerHostName, managerPort))
    {
      context.errorCallback(T("SendWorkUnit::run"), T("Not connected !"));
      return NULL;
    }
    context.informationCallback(managerHostName, T("Connected !"));
    ManagerNodeNetworkInterfacePtr interface = clientManagerNodeNetworkInterface(context, client, source);
    interface->sendInterfaceClass();
    return interface;
  }
  
  String sendWU(ExecutionContext& context, WorkUnitPtr wu, ManagerNodeNetworkInterfacePtr interface) const
  {    
    NetworkRequestPtr request = new NetworkRequest(context, projectName, source, destination, wu, requiredCpus, requiredMemory, requiredTime);
    return interface->pushWorkUnit(request);
  }
#endif
  
  WorkUnitPtr generateWUFromFunctionAndParameters(const FunctionPtr& function, const Variable& parameters)
  {
    jassertfalse;
    // TODO  arnaud
    return NULL;
  }
  
};
  */
};
#endif // !LBCPP_DISTRIBUTED_OPTIMIZER_CONTEXT_H_
