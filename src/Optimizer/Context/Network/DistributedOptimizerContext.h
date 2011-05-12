/*-----------------------------------------.---------------------------------.
| Filename: DistributedOptimizerContext.h  | OptimizerContext used to        |
| Author  : Arnaud Schoofs                 | distribute work on NIC3, BOINC, |
| Started : 03/04/2011                     | ... (asynchronous)              |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DISTRIBUTED_OPTIMIZER_CONTEXT_H_
# define LBCPP_DISTRIBUTED_OPTIMIZER_CONTEXT_H_

# include <lbcpp/Optimizer/OptimizerContext.h>
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Network/NetworkClient.h>
# include <lbcpp/Network/NetworkInterface.h>

namespace lbcpp
{
class GetFinishedExecutionTracesDaemon;
  
class DistributedOptimizerContext : public OptimizerContext
{
public:
  DistributedOptimizerContext(const FunctionPtr& objectiveFunction, String projectName, String source, String destination, String managerHostName, size_t managerPort, size_t requiredCpus, size_t requiredMemory, size_t requiredTime);
  DistributedOptimizerContext() {}
  
  virtual void setPostEvaluationCallback(const FunctionCallbackPtr& callback)
    {functionCallback = callback;}
  virtual void removePostEvaluationCallback(const FunctionCallbackPtr& callback); // TODO arnaud : code should be elsewhere ?
  
  virtual void waitUntilAllRequestsAreProcessed() const
  {
    while (inProgressWUs.size())
      Thread::sleep(60000);
  }
  
  virtual bool areAllRequestsProcessed() const
    {return inProgressWUs.size() == 0;}
  
  virtual bool evaluate(ExecutionContext& context, const Variable& parameters)
  {
    executionContext = &context;  // TODOD arnaud : if
    
    ManagerNodeNetworkInterfacePtr interface = getNetworkInterfaceAndConnect(context);
    if (!interface)
      return false;
    WorkUnitPtr wu = new FunctionWorkUnit(objectiveFunction, parameters);
    String res = sendWU(context, wu, interface);
    
    if (res == T("Error"))
    {
      context.errorCallback(T("DistributedOptimizerContext::evaluate"), T("Trouble - We didn't correclty receive the acknowledgement"));
      return false;
    }
    interface->closeCommunication();
    
    {
      ScopedLock _(lock);
      inProgressWUs.push_back(std::make_pair(res, parameters));
    }
    
    return true;
  }
  
  
protected:  
  friend class DistributedOptimizerContextClass;
  friend class GetFinishedExecutionTracesDaemon;
  
  String projectName;
  String source;
  String destination;
  String managerHostName;
  size_t managerPort;
  size_t requiredCpus;
  size_t requiredMemory;
  size_t requiredTime;
  
  FunctionCallbackPtr functionCallback;
  ExecutionContextPtr executionContext;
  
  std::vector< std::pair<String, Variable> > inProgressWUs;
  
  CriticalSection lock;
  GetFinishedExecutionTracesDaemon* getFinishedTracesThread;
  
  ManagerNodeNetworkInterfacePtr getNetworkInterfaceAndConnect(ExecutionContext& context) const
  {       
    NetworkClientPtr client = blockingNetworkClient(context);
    if (!client->startClient(managerHostName, managerPort))
    {
      context.errorCallback(T("DistributedOptimizerContext::getNetworkInterfaceAndConnect"), T("Not connected !"));
      return NULL;
    }
    //context.informationCallback(managerHostName, T("Connected !")); TODO arnaud : useless ?
    ManagerNodeNetworkInterfacePtr interface = clientManagerNodeNetworkInterface(context, client, source);
    interface->sendInterfaceClass();
    return interface;
  }
  
  String sendWU(ExecutionContext& context, WorkUnitPtr wu, ManagerNodeNetworkInterfacePtr interface) const
  {    
    NetworkRequestPtr request = new NetworkRequest(context, projectName, source, destination, wu, requiredCpus, requiredMemory, requiredTime);
    return interface->pushWorkUnit(request);
  }
};
  
typedef ReferenceCountedObjectPtr<DistributedOptimizerContext> DistributedOptimizerContextPtr;

class GetFinishedExecutionTracesDaemon : public Thread
{
public:
  GetFinishedExecutionTracesDaemon(const DistributedOptimizerContextPtr& optimizerContext) 
  : Thread(T("GetFinishedExecutionTracesDaemon")), optimizerContext(optimizerContext) {}
  
  virtual void run()
  {
    while (!threadShouldExit())
    {
      sleep(30000);
      
      // handle finished WUs
      ManagerNodeNetworkInterfacePtr interface = optimizerContext->getNetworkInterfaceAndConnect(*(optimizerContext->executionContext));  // TODO arnaud
      if (!interface) 
        continue;
      
      std::vector< std::pair<String, Variable> >::iterator it;
      {
        ScopedLock _(optimizerContext->lock);
        for (it = optimizerContext->inProgressWUs.begin(); it != optimizerContext->inProgressWUs.end(); )
        {
          if (interface->isFinished(it->first))
          {
            NetworkResponsePtr res = interface->getExecutionTrace(it->first);
            if (res)
            {  
              // TODO arnaud : traiter cas où qq pas valide
              ExecutionTracePtr trace = res->getExecutionTrace(*(optimizerContext->executionContext));
              ExecutionTraceNodePtr root = trace->getRootNode();
              std::vector<ExecutionTraceItemPtr> vec = root->getSubItems();  
              ExecutionTraceNodePtr traceNode = vec[0].dynamicCast<ExecutionTraceNode>();
              Variable returnValue = traceNode->getReturnValue();
              
              optimizerContext->functionCallback->functionReturned(*(optimizerContext->executionContext), FunctionPtr(), &(it->second), returnValue); 
              
              optimizerContext->inProgressWUs.erase(it);
            }
            else
              ++it;
          }
          else 
            ++it;
        }
      }
      interface->closeCommunication();
    }
  }
  
private:
  DistributedOptimizerContextPtr optimizerContext;
};
  
};
#endif // !LBCPP_DISTRIBUTED_OPTIMIZER_CONTEXT_H_
