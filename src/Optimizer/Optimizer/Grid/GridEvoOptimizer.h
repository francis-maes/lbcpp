/*-----------------------------------------.---------------------------------.
 | Filename: GridEvoOptimizer.h             | Optimizer using Evolutionary    |
 | Author  : Arnaud Schoofs                 | Algorithm on Grid               |
 | Started : 01/03/2010 23:45               |                                 |
 `------------------------------------------/                                 |
                                            |                                 |
                                            `--------------------------------*/

#ifndef GRID_EVO_OPTIMIZER_H_
#define GRID_EVO_OPTIMIZER_H_

# include <lbcpp/Optimizer/GridOptimizer.h>
# include <lbcpp/Network/NetworkClient.h>
# include "../src/Network/Node/ManagerNode/ManagerNodeNetworkInterface.h"
# include "../../../Distribution/Builder/IndependentMultiVariateDistributionBuilder.h"

// TODO delete Ptr ?
namespace lbcpp
{

class GridEvoOptimizerState : public GridOptimizerState {
public:
  GridEvoOptimizerState() {} // TODO arnaud OK?
  GridEvoOptimizerState(IndependentMultiVariateDistributionPtr distributions) : distributions(distributions)
  {
    totalNumberGeneratedWUs = 0;
    totalNumberEvaluatedWUs = 0;
    bestVariable = Variable();
    bestScore = 0.0;
  }
  
  virtual WorkUnitPtr generateSampleWU(ExecutionContext& context) const = 0;
  
protected:
  size_t totalNumberGeneratedWUs;
  size_t totalNumberEvaluatedWUs;
  
  std::vector<String> inProgressWUs;
  std::vector<String> currentEvaluatedWUs;  // evaluated WUs not processed yet
  
  IndependentMultiVariateDistributionPtr distributions;
  
  Variable bestVariable;
  double bestScore;
  
  friend class GridEvoOptimizerStateClass;
  friend class GridEvoOptimizer;  // so that optimizer has access to state variables
  
};
typedef ReferenceCountedObjectPtr<GridEvoOptimizerState> GridEvoOptimizerStatePtr;  
extern ClassPtr gridEvoOptimizerStateClass; 
  
class GridEvoOptimizer : public GridOptimizer
{
public:
  GridEvoOptimizer() {}
  GridEvoOptimizer(size_t totalNumberWuRequested, size_t numberWuToUpdate, size_t numberWuInProgress, size_t ratioUsedForUpdate, String projectName, String source, String destination,
                   String managerHostName, size_t managerPort, size_t requiredMemory, size_t requiredTime, size_t timeToSleep) : 
  totalNumberWuRequested(totalNumberWuRequested), numberWuToUpdate(numberWuToUpdate), numberWuInProgress(numberWuInProgress), ratioUsedForUpdate(ratioUsedForUpdate),
  projectName(projectName), source(source), destination(destination), managerHostName(managerHostName), managerPort(managerPort), requiredMemory(requiredMemory), requiredTime(requiredTime), timeToSleep(timeToSleep) 
  {requiredCpus = 1;}
  
  
  
  virtual TypePtr getRequiredStateType() const
    {return gridEvoOptimizerStateClass;}
  
  virtual Variable optimize(ExecutionContext& context, const GridOptimizerStatePtr& state_, const FunctionPtr& getVariableFromTrace, const FunctionPtr& getScoreFromTrace) const
  {    
    // save initial state
    GridEvoOptimizerStatePtr state = state_.dynamicCast<GridEvoOptimizerState>();
    state->saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml")));
    
    while (state->totalNumberEvaluatedWUs < totalNumberWuRequested) 
    {
      // don't stress the manager
      juce::Thread::sleep(timeToSleep*1000);
      
      // Send WU's on network
      if (state->totalNumberGeneratedWUs < totalNumberWuRequested && state->inProgressWUs.size() < numberWuInProgress) 
      {
        // Establish network connection
        ManagerNodeNetworkInterfacePtr interface = getNetworkInterfaceAndConnect(context);
        if (!interface)
          continue;
        context.informationCallback(T("Sending WUs ..."));
        // Send WU's
        while (state->totalNumberGeneratedWUs < totalNumberWuRequested && state->inProgressWUs.size() < numberWuInProgress) 
        {
          WorkUnitPtr wu = state->generateSampleWU(context);
          String res = sendWU(context, wu, interface);
          if (res == T("Error"))
          {
            context.errorCallback(T("SendWorkUnit::run"), T("Trouble - We didn't correclty receive the acknowledgement"));
            break;
          }
          context.resultCallback(T("WorkUnitIdentifier"), res);
          state->inProgressWUs.push_back(res);
          state->totalNumberGeneratedWUs++;
        }
        interface->closeCommunication();
        
        // save state
        File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml")).copyFileTo(File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState_backup.xml")));
        state->saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml")));
        context.informationCallback(T("State file saved in : ") + File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml")).getFullPathName());
        
        continue;
      }
      
      
      // don't stress the manager
      juce::Thread::sleep(timeToSleep*1000);
      

      // handle finished WUs
      ManagerNodeNetworkInterfacePtr interface = getNetworkInterfaceAndConnect(context);
      if (!interface) 
        continue;
      
      std::vector<String>::iterator it;
      for(it = state->inProgressWUs.begin(); it != state->inProgressWUs.end(); )
      {
        if(interface->isFinished(*it))
        {
          NetworkResponsePtr res = interface->getExecutionTrace(*it);
          if (res)
          {  
            ExecutionTracePtr trace = res->getExecutionTrace(context);
            trace->saveToFile(context,File::getCurrentWorkingDirectory().getChildFile(String(*it) + T(".trace")));  // TODO arnaud : project directory ?
            state->currentEvaluatedWUs.push_back(*it);
            state->totalNumberEvaluatedWUs++; // TODO arnaud : here or when updating distri ?
            state->inProgressWUs.erase(it);
          }
          else
            ++it;
        }
        else 
          ++it;
      }
      interface->closeCommunication();
      
      // enough WUs evaluated -> update distribution (with best results)
      if (state->currentEvaluatedWUs.size() >= numberWuToUpdate) 
      {
        context.informationCallback(T("Updating state ..."));
        std::multimap<double, Variable> resultsMap; // mutlimap used to sort results by score
        std::vector<String>::iterator it;
        for(it = state->currentEvaluatedWUs.begin(); it != state->currentEvaluatedWUs.end(); it++) 
        {
          ExecutionTracePtr trace = Object::createFromFile(context, File::getCurrentWorkingDirectory().getChildFile(String(*it) + T(".trace"))).staticCast<ExecutionTrace>();
          double score = getScoreFromTrace->compute(context, trace).getDouble();
          Variable var = getVariableFromTrace->compute(context, trace);
          resultsMap.insert(std::pair<double, Variable>(score,var));
        }
        
        IndependentMultiVariateDistributionBuilderPtr distributionsBuilder = state->distributions->createBuilder();
        size_t nb = 0;
        std::multimap<double, Variable>::reverse_iterator mapIt;
        // best results : use them then delete
        for (mapIt = resultsMap.rbegin(); mapIt != resultsMap.rend() && nb < state->currentEvaluatedWUs.size()/ratioUsedForUpdate; mapIt++)
        {
          distributionsBuilder->addElement((*mapIt).second);
          nb++;
        }
        state->distributions = distributionsBuilder->build(context);
        
        if ((*(resultsMap.rbegin())).first > state->bestScore) {
          state->bestScore = (*(resultsMap.rbegin())).first;
          state->bestVariable = (*(resultsMap.rbegin())).second;
          context.informationCallback(T("New best result found : ") + state->bestVariable.toString());
        }
               
        // delete files and clear vector
        for (it = state->currentEvaluatedWUs.begin(); it != state->currentEvaluatedWUs.end(); it++) {
          File::getCurrentWorkingDirectory().getChildFile(String(*it) + T(".trace")).deleteFile();
        }
        state->currentEvaluatedWUs.clear(); // clear map
        context.informationCallback(T("State updated"));
        
        // save state
        File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml")).copyFileTo(File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState_backup.xml")));
        state->saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml")));
        context.informationCallback(T("State file saved in : ") + File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml")).getFullPathName());
      }
    }
    
    return state->bestVariable;
  }
    
protected:  
  friend class GridEvoOptimizerClass;
  
private:
  size_t totalNumberWuRequested;
  size_t numberWuToUpdate;
  size_t numberWuInProgress;
  size_t ratioUsedForUpdate;
  String projectName;
  String source;
  String destination;
  String managerHostName;
  size_t managerPort;
  size_t requiredCpus;
  size_t requiredMemory;
  size_t requiredTime;
  size_t timeToSleep; // in seconds
  
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
  
typedef ReferenceCountedObjectPtr<GridEvoOptimizer> GridEvoOptimizerPtr;

}; /* namespace lbcpp */

#endif // !GRID_EVO_OPTIMIZER_H_