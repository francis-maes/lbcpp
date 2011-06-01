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
# include <lbcpp/Network/NetworkNotification.h>

namespace lbcpp
{
class GetFinishedExecutionTracesDaemon;
  
class DistributedOptimizerContext : public OptimizerContext
{
public:
  // TODO : add validationFunction
  DistributedOptimizerContext(ExecutionContext& context, const FunctionPtr& objectiveFunction, String projectName, String source, String destination, String managerHostName, size_t managerPort, size_t requiredCpus, size_t requiredMemory, size_t requiredTime, size_t timeToSleep = 60000);
  DistributedOptimizerContext() {timeToSleep = 60000;}
  
  // overide default implementation
  // store the pointer to the callback
  // it is used by the GetFinishedExecutionTracesDaemon when a result is found
  virtual void setPostEvaluationCallback(const FunctionCallbackPtr& callback)
    {functionCallback = callback;}
  virtual void removePostEvaluationCallback(const FunctionCallbackPtr& callback);
  
  virtual bool areAllRequestsProcessed() const
    {return inProgressWUs.size() == 0;}
  
  virtual bool evaluate(const Variable& parameters)
  {
    // create a WU for this request and send it to the Manager
    NetworkClientPtr client;
    ManagerNetworkInterfacePtr interface = getNetworkInterfaceAndConnect(client);
    
    if (!interface)
      return false;
    WorkUnitPtr wu = new FunctionWorkUnit(objectiveFunction, parameters);
    String res = sendWU(wu, interface);
    
    client->sendVariable(new CloseCommunicationNotification());
    client->stopClient();
    
    if (res == T("Error"))
    {
      context.errorCallback(T("DistributedOptimizerContext::evaluate"), T("Trouble - We didn't correclty receive the acknowledgement"));
      return false;
    }
    
    {
      ScopedLock _(inProgressWUsLock);  // synchronized because it is alsa accessible by GetFinishedExecutionTracesDaemon
      inProgressWUs.push_back(std::make_pair(res, parameters));
    }
    return true;
  }
  
  
protected:  
  friend class DistributedOptimizerContextClass;
  friend class GetFinishedExecutionTracesDaemon;  // friend class so that there is no need to declare too much accessor methods
  
  String projectName;
  String source;
  String destination;
  String managerHostName;
  size_t managerPort;
  size_t requiredCpus;
  size_t requiredMemory;
  size_t requiredTime;
  
  FunctionCallbackPtr functionCallback;
  
  CriticalSection inProgressWUsLock;  /**< lock to use to acces inProgessWUs safely */
  std::vector< std::pair<String, Variable> > inProgressWUs; /**< Contains pair <id, variable> of the FunctionWorkUnit sent to the manager. */

  GetFinishedExecutionTracesDaemon* getFinishedTracesThread;  /**< Pointer to the thread that contacts the Manager to get the results. */
  
  ManagerNetworkInterfacePtr getNetworkInterfaceAndConnect(NetworkClientPtr& client) const
  {       
    client = blockingNetworkClient(context);
    if (!client->startClient(managerHostName, managerPort))
    {
      context.errorCallback(T("DistributedOptimizerContext::getNetworkInterfaceAndConnect"), T("Not connected !"));
      client = NetworkClientPtr();
      return ManagerNetworkInterfacePtr();
    }
    ManagerNetworkInterfacePtr interface = forwarderManagerNetworkInterface(context, client, source);
    client->sendVariable(ReferenceCountedObjectPtr<NetworkInterface>(interface));
    return interface;
  }
  
  String sendWU(WorkUnitPtr wu, ManagerNetworkInterfacePtr interface) const
  {    
    WorkUnitNetworkRequestPtr request = new WorkUnitNetworkRequest(context, projectName, source, destination, wu, requiredCpus, requiredMemory, requiredTime);
    return interface->pushWorkUnit(request);
  }
};
  
typedef ReferenceCountedObjectPtr<DistributedOptimizerContext> DistributedOptimizerContextPtr;

/**
 * Thread used to get the results from the Manager.
 */
class GetFinishedExecutionTracesDaemon : public Thread
{
public:
  GetFinishedExecutionTracesDaemon(const DistributedOptimizerContextPtr& optimizerContext) 
    : Thread(T("GetFinishedExecutionTracesDaemon")), optimizerContext(optimizerContext), context(optimizerContext->context) {}
  
  virtual void run()
  {    
    while (!threadShouldExit())
    {
      sleep(optimizerContext->timeToSleep); // avoid busy waiting
       
      NetworkClientPtr client;
      ManagerNetworkInterfacePtr interface = optimizerContext->getNetworkInterfaceAndConnect(client);
      if (!interface) 
        continue;
      
      // walk through inProgressWUs and check if the result is available,
      // if it is, get it
      std::vector< std::pair<String, Variable> >::iterator it;
      {
        ScopedLock _(optimizerContext->inProgressWUsLock);
        for (it = optimizerContext->inProgressWUs.begin(); it != optimizerContext->inProgressWUs.end(); )
        {
          if (interface->isFinished(it->first))
          {
            ExecutionTraceNetworkResponsePtr res = interface->getExecutionTrace(it->first);
            if (res)
            {  
              // Parse XML file to get the return value
              // FIXME : there should be an easier way to do that
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
      client->sendVariable(new CloseCommunicationNotification());
      client->stopClient();
    }
  }
  
private:
  DistributedOptimizerContextPtr optimizerContext;  /**< Pointer to the DistributedOptimizerContext associated to access inProgressWUs for instance. */
  ExecutionContext& context;
};
  
};
#endif // !LBCPP_DISTRIBUTED_OPTIMIZER_CONTEXT_H_
