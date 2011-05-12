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
  DistributedOptimizerContext(ExecutionContext& context, const FunctionPtr& objectiveFunction, String projectName, String source, String destination, String managerHostName, size_t managerPort, size_t requiredCpus, size_t requiredMemory, size_t requiredTime, size_t timeToSleep = 60000);
  DistributedOptimizerContext() {timeToSleep = 60000;}
  
  virtual void setPostEvaluationCallback(const FunctionCallbackPtr& callback)
    {functionCallback = callback;}
  virtual void removePostEvaluationCallback(const FunctionCallbackPtr& callback); // TODO arnaud : code should be elsewhere ?
  
  virtual void waitUntilAllRequestsAreProcessed() const
  {
    while (inProgressWUs.size())
      Thread::sleep(timeToSleep);
  }
  
  virtual bool areAllRequestsProcessed() const
    {return inProgressWUs.size() == 0;}
  
  virtual size_t getTimeToSleep() const
    {return timeToSleep;}
  
  virtual bool evaluate(const Variable& parameters)
  {   
    ManagerNodeNetworkInterfacePtr interface = getNetworkInterfaceAndConnect();
    if (!interface)
      return false;
    WorkUnitPtr wu = new FunctionWorkUnit(objectiveFunction, parameters);
    String res = sendWU(wu, interface);
    
    if (res == T("Error"))
    {
      context.errorCallback(T("DistributedOptimizerContext::evaluate"), T("Trouble - We didn't correclty receive the acknowledgement"));
      return false;
    }
    interface->closeCommunication();
    
    {
      ScopedLock _(inProgressWUsLock);
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
  size_t timeToSleep;
  
  FunctionCallbackPtr functionCallback;
  
  CriticalSection inProgressWUsLock;
  std::vector< std::pair<String, Variable> > inProgressWUs;

  GetFinishedExecutionTracesDaemon* getFinishedTracesThread;
  
  ManagerNodeNetworkInterfacePtr getNetworkInterfaceAndConnect() const
  {       
    NetworkClientPtr client = blockingNetworkClient(context);
    if (!client->startClient(managerHostName, managerPort))
    {
      context.errorCallback(T("DistributedOptimizerContext::getNetworkInterfaceAndConnect"), T("Not connected !"));
      return NULL;
    }
    ManagerNodeNetworkInterfacePtr interface = clientManagerNodeNetworkInterface(context, client, source);
    interface->sendInterfaceClass();
    return interface;
  }
  
  String sendWU(WorkUnitPtr wu, ManagerNodeNetworkInterfacePtr interface) const
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
    : Thread(T("GetFinishedExecutionTracesDaemon")), optimizerContext(optimizerContext), context(optimizerContext->context) {}
  
  virtual void run()
  {    
    while (!threadShouldExit())
    {
      sleep(optimizerContext->timeToSleep);
       
      // handle finished WUs
      ManagerNodeNetworkInterfacePtr interface = optimizerContext->getNetworkInterfaceAndConnect();
      if (!interface) 
        continue;
      
      std::vector< std::pair<String, Variable> >::iterator it;
      {
        ScopedLock _(optimizerContext->inProgressWUsLock);
        for (it = optimizerContext->inProgressWUs.begin(); it != optimizerContext->inProgressWUs.end(); )
        {
          if (interface->isFinished(it->first))
          {
            NetworkResponsePtr res = interface->getExecutionTrace(it->first);
            if (res)
            {  
              // TODO arnaud : traiter cas où qq pas valide mieux
             
              ExecutionTracePtr trace = res->getExecutionTrace(context);
              if (!trace) {
                context.warningCallback(T("Trace of ") + it->first + T(" is not a valide ExecutionTrace"));
                optimizerContext->functionCallback->functionReturned(context, optimizerContext->objectiveFunction, &(it->second), DBL_MAX);
                optimizerContext->inProgressWUs.erase(it);
                continue;
              }
              
              ExecutionTraceNodePtr root = trace->getRootNode();
              if (!root) {
                context.warningCallback(T("Can't get root in trace of ") + it->first);
                optimizerContext->functionCallback->functionReturned(context, optimizerContext->objectiveFunction, &(it->second), DBL_MAX);
                optimizerContext->inProgressWUs.erase(it);
                continue;
              }
              
              std::vector<ExecutionTraceItemPtr> vec = root->getSubItems();
              if (vec.empty()) {
                context.warningCallback(T("Can't get subitems in trace of ") + it->first);
                optimizerContext->functionCallback->functionReturned(context, optimizerContext->objectiveFunction, &(it->second), DBL_MAX);
                optimizerContext->inProgressWUs.erase(it);
                continue;
              }
              
              ExecutionTraceNodePtr traceNode = vec[0].dynamicCast<ExecutionTraceNode>();
              if (!traceNode) {
                context.warningCallback(T("Can't get node in trace of ") + it->first);
                optimizerContext->functionCallback->functionReturned(context, optimizerContext->objectiveFunction, &(it->second), DBL_MAX);
                optimizerContext->inProgressWUs.erase(it);
                continue;
              }
              
              Variable returnValue = traceNode->getReturnValue();
              if (!returnValue.exists()) {
                context.warningCallback(T("No return value in trace of ") + it->first);
                optimizerContext->functionCallback->functionReturned(context, optimizerContext->objectiveFunction, &(it->second), DBL_MAX);
                optimizerContext->inProgressWUs.erase(it);
                continue;
              }
              
              optimizerContext->functionCallback->functionReturned(context, FunctionPtr(), &(it->second), returnValue);
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
  ExecutionContext& context;
};
  
};
#endif // !LBCPP_DISTRIBUTED_OPTIMIZER_CONTEXT_H_
