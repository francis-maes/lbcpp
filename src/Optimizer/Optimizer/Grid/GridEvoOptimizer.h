/*-----------------------------------------.---------------------------------.
| Filename: GridEvoOptimizer.h             | Optimizer using Evolutionary    |
| Author  : Arnaud Schoofs                 | Algorithm on Grid               |
| Started : 01/03/2010 23:45               | (deprecated)                    |
`------------------------------------------/                                 |
                               |                                             |
															 `--------------------------------------------*/

// TODO arnaud : clean things up to compile wihtout network

#ifndef GRID_EVO_OPTIMIZER_H_
#define GRID_EVO_OPTIMIZER_H_

# include <lbcpp/Optimizer/GridOptimizer.h>

#ifdef LBCPP_NETWORKING
# include <lbcpp/Network/NetworkClient.h>
# include <lbcpp/Network/NetworkInterface.h>
#endif

# include "../../../Distribution/Builder/IndependentMultiVariateDistributionBuilder.h"

namespace lbcpp
{

class GridEvoOptimizerState : public GridOptimizerState
{
public:
  GridEvoOptimizerState(IndependentMultiVariateDistributionPtr distributions)
    : distributions(distributions)
  {
    totalNumberGeneratedWUs = 0;
    totalNumberEvaluatedWUs = 0;
    bestVariable = Variable();
    bestScore = 0.0;
  }
  GridEvoOptimizerState() {}
  
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
  friend class EvoOptimizer;  //TODO arnaud : change this after retructuration
};

typedef ReferenceCountedObjectPtr<GridEvoOptimizerState> GridEvoOptimizerStatePtr;  
extern ClassPtr gridEvoOptimizerStateClass; 
  

// TODO arnaud : put this class in antoher file
/**
 * Class used to run a WorkUnit as RunWorkUnit.cpp does
 */
class RunWorkUnit : public WorkUnit
{
public:
  RunWorkUnit(const File& f) : f(f) {}
  RunWorkUnit() {}
  
  virtual Variable run(ExecutionContext& context)
  {
#ifdef LBCPP_NETWORKING
    ExecutionContextPtr newContext = singleThreadedExecutionContext(File::getCurrentWorkingDirectory());  // one thread per WUs
    
    // execution trace
    ExecutionCallbackPtr makeTraceCallback;
    ExecutionTracePtr trace = new ExecutionTrace(newContext->toString());
    makeTraceCallback = makeTraceExecutionCallback(trace);
    newContext->appendCallback(makeTraceCallback);
    
    // run work unit from file
    WorkUnitPtr wu = Object::createFromFile(*newContext, f).staticCast<WorkUnit>();
    newContext->run(wu);
    
    // save execution trace
    newContext->removeCallback(makeTraceCallback);
    trace->saveToFile(*newContext, File::getCurrentWorkingDirectory().getChildFile(f.getFileNameWithoutExtension() + T(".trace")));
#endif   
    return 0; // TODO arnaud
  }
  
protected:
  friend class RunWorkUnitClass;
  
private:
  File f;
};
  
// TODO arnaud : progressionCallback !  
  
// TODO arnaud : put this class in antoher file  
class EvoOptimizer : public GridOptimizer   // TODO arnaud : change Optimizer interface to use Optimizer instead of GridOptimizer as mother class
{
public:
  EvoOptimizer(size_t totalNumberWuRequested, size_t numberWuToUpdate, size_t numberWuInProgress, size_t ratioUsedForUpdate, size_t timeToSleep, size_t updateFactor)
    : totalNumberWuRequested(totalNumberWuRequested), numberWuToUpdate(numberWuToUpdate), numberWuInProgress(numberWuInProgress),
      ratioUsedForUpdate(ratioUsedForUpdate), timeToSleep(timeToSleep), updateFactor(updateFactor) {}

  EvoOptimizer() {}
  
  virtual TypePtr getRequiredStateType() const
    {return gridEvoOptimizerStateClass;}
  
  virtual Variable optimize(ExecutionContext& context, const GridOptimizerStatePtr& state_, const FunctionPtr& getVariableFromTrace, const FunctionPtr& getScoreFromTrace) const
  {    
    // save initial state
    GridEvoOptimizerStatePtr state = state_.dynamicCast<GridEvoOptimizerState>();
    state->saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(T("EvoOptimizerState.xml")));  // TODO arnaud : file name as args ?
    
    // multi-threads execution to run the WUs
    ExecutionContextPtr newContext = multiThreadedExecutionContext((size_t)juce::SystemStats::getNumCpus());
    
    // restart inProgress WUs from state
    std::vector<String>::iterator it2;
    for(it2 = state->inProgressWUs.begin(); it2 != state->inProgressWUs.end(); it2++)  // restart WUs
    {
      WorkUnitPtr wu = new RunWorkUnit(File::getCurrentWorkingDirectory().getChildFile(*it2 + T(".workUnit")));
      newContext->pushWorkUnit(wu);
    }
    
    // main loop
    while (state->totalNumberEvaluatedWUs < totalNumberWuRequested) 
    {
      
      // WU generation loop
      size_t nb = 0;
      while (state->totalNumberGeneratedWUs < totalNumberWuRequested && state->inProgressWUs.size() < numberWuInProgress) 
      {
        WorkUnitPtr wu = state->generateSampleWU(context);
        wu->saveToFile(context,File::getCurrentWorkingDirectory().getChildFile(String((int) state->totalNumberGeneratedWUs) + T(".workUnit"))); 
        wu = new RunWorkUnit(File::getCurrentWorkingDirectory().getChildFile(String((int) state->totalNumberGeneratedWUs) + T(".workUnit")));
        newContext->pushWorkUnit(wu);
        state->inProgressWUs.push_back(String((int) state->totalNumberGeneratedWUs));
        state->totalNumberGeneratedWUs++;
        nb++;
      }
      context.informationCallback(T("WUs generation: ") + String((int) nb) + T(" WU(s) generated"));
      
      
      // save state
      if (nb > 0) 
      {
        File::getCurrentWorkingDirectory().getChildFile(T("EvoOptimizerState.xml")).copyFileTo(File::getCurrentWorkingDirectory().getChildFile(T("EvoOptimizerState_backup.xml")));
        state->saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(T("EvoOptimizerState.xml")));
        context.informationCallback(T("State file saved in : ") + File::getCurrentWorkingDirectory().getChildFile(T("EvoOptimizerState.xml")).getFullPathName());
      }
      
      
      // don't do busy waiting
      Thread::sleep(timeToSleep*1000);

      // check for finished results
      std::vector<String>::iterator it;
      for (it = state->inProgressWUs.begin(); it != state->inProgressWUs.end(); )
      {
        File f = File::getCurrentWorkingDirectory().getChildFile(*it + T(".trace"));
        if (f.existsAsFile())
        {
          state->currentEvaluatedWUs.push_back(*it);
          state->totalNumberEvaluatedWUs++;
          File::getCurrentWorkingDirectory().getChildFile(*it + T(".workUnit")).deleteFile();
          context.informationCallback(T("WU ") + *it + T(" finished"));
          state->inProgressWUs.erase(it);
        }
        else 
          ++it;
      }
      
      File::getCurrentWorkingDirectory().getChildFile(T("EvoOptimizerState.xml")).copyFileTo(File::getCurrentWorkingDirectory().getChildFile(T("EvoOptimizerState_backup.xml")));
      state->saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(T("EvoOptimizerState.xml")));
      context.informationCallback(T("State file saved in : ") + File::getCurrentWorkingDirectory().getChildFile(T("EvoOptimizerState.xml")).getFullPathName());
      
      // enough WUs evaluated -> update distribution (with best results)
      if (state->currentEvaluatedWUs.size() >= numberWuToUpdate || (state->totalNumberGeneratedWUs == totalNumberWuRequested && state->inProgressWUs.size() == 0)) 
      {
        context.informationCallback(T("Updating state ..."));
        
        // get (and sort) : score -> variable from trace files
        std::multimap<double, Variable> resultsMap; // mutlimap used to sort results by score
        std::vector<String>::iterator it;
        for (it = state->currentEvaluatedWUs.begin(); it != state->currentEvaluatedWUs.end(); it++) 
        {
          ExecutionTracePtr trace = Object::createFromFile(context, File::getCurrentWorkingDirectory().getChildFile(*it + T(".trace"))).staticCast<ExecutionTrace>();
          double score = getScoreFromTrace->compute(context, trace).getDouble();
          Variable var = getVariableFromTrace->compute(context, trace);
          resultsMap.insert(std::pair<double, Variable>(score,var));
        }
        
        // use best results to build new distri
        IndependentMultiVariateDistributionBuilderPtr distributionsBuilder = state->distributions->createBuilder();
        nb = 0;
        std::multimap<double, Variable>::reverse_iterator mapIt;
        for (mapIt = resultsMap.rbegin(); mapIt != resultsMap.rend() && nb < state->currentEvaluatedWUs.size()/ratioUsedForUpdate; mapIt++)
        {
          distributionsBuilder->addElement((*mapIt).second);  // TODO arnaud : maybe use all results and use weight
          nb++;
        }
        IndependentMultiVariateDistributionPtr newDistri = distributionsBuilder->build(context);
        distributionsBuilder->clear();
        distributionsBuilder->addDistribution(state->distributions);  // old distri
        for (size_t i = 0; i < updateFactor; ++i)
          distributionsBuilder->addDistribution(newDistri);
        state->distributions = distributionsBuilder->build(context); 
                
        // update best score
        if ((*(resultsMap.rbegin())).first > state->bestScore)
        {
          state->bestScore = (*(resultsMap.rbegin())).first;
          state->bestVariable = (*(resultsMap.rbegin())).second;
          context.informationCallback(T("New best result found : ") + state->bestVariable.toString() + T(" ( ") + String(state->bestScore) + T(" )"));
        }
        
        // clear
        state->currentEvaluatedWUs.clear();
        context.informationCallback(T("State updated"));
        
        // save state
        File::getCurrentWorkingDirectory().getChildFile(T("EvoOptimizerState.xml")).copyFileTo(File::getCurrentWorkingDirectory().getChildFile(T("EvoOptimizerState_backup.xml")));
        state->saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(T("EvoOptimizerState.xml")));
        context.informationCallback(T("State file saved in : ") + File::getCurrentWorkingDirectory().getChildFile(T("EvoOptimizerState.xml")).getFullPathName());
      }
    }
    
    return state->bestVariable;
  }
  
protected:  
  friend class EvoOptimizerClass;
  
private:
  size_t totalNumberWuRequested;
  size_t numberWuToUpdate;
  size_t numberWuInProgress;
  size_t ratioUsedForUpdate;
  size_t timeToSleep; // in seconds
  size_t updateFactor;
  
};
  typedef ReferenceCountedObjectPtr<EvoOptimizer> EvoOptimizerPtr;

  
class GridEvoOptimizer : public GridOptimizer
{
public:
  GridEvoOptimizer() {}
  GridEvoOptimizer(size_t totalNumberWuRequested, size_t numberWuToUpdate, size_t numberWuInProgress, size_t ratioUsedForUpdate, String projectName, String source, String destination,
                   String managerHostName, size_t managerPort, size_t requiredMemory, size_t requiredTime, size_t timeToSleep, size_t updateFactor)
    : totalNumberWuRequested(totalNumberWuRequested), numberWuToUpdate(numberWuToUpdate), numberWuInProgress(numberWuInProgress), ratioUsedForUpdate(ratioUsedForUpdate),
      projectName(projectName), source(source), destination(destination), managerHostName(managerHostName), managerPort(managerPort), requiredMemory(requiredMemory),
      requiredTime(requiredTime), timeToSleep(timeToSleep), updateFactor(updateFactor) 
  {
    requiredCpus = 1;
  } // TODO arnaud : edit for NIC3
  // TODO arnaud : check nbevalute < nbinprogress
  
  virtual TypePtr getRequiredStateType() const
    {return gridEvoOptimizerStateClass;}
  
  virtual Variable optimize(ExecutionContext& context, const GridOptimizerStatePtr& state_, const FunctionPtr& getVariableFromTrace, const FunctionPtr& getScoreFromTrace) const
  {   
#ifdef LBCPP_NETWORKING
    // save initial state
    GridEvoOptimizerStatePtr state = state_.dynamicCast<GridEvoOptimizerState>();
    state->saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml")));
    
    while (state->totalNumberEvaluatedWUs < totalNumberWuRequested) 
    {      
      // Send WU's on network
      if (state->totalNumberGeneratedWUs < totalNumberWuRequested && state->inProgressWUs.size() < numberWuInProgress) 
      {
        // Establish network connection
        ManagerNodeNetworkInterfacePtr interface = getNetworkInterfaceAndConnect(context);
        if (!interface)
          continue;
        context.informationCallback(T("Sending WUs ..."));
        // Send WU's
        size_t nb = 0;
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
          nb++;
        }
        interface->closeCommunication();
        context.informationCallback(T("WUs generation: ") + String((int) nb) + T(" WU(s) generated"));

        
        // save state
        if (nb > 0) 
        {
          File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml")).copyFileTo(File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState_backup.xml")));
          state->saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml")));
          context.informationCallback(T("State file saved in : ") + File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml")).getFullPathName());
        }
      }
      
      
      // don't stress the manager
      juce::Thread::sleep(timeToSleep*1000);
      

      // handle finished WUs
      ManagerNodeNetworkInterfacePtr interface = getNetworkInterfaceAndConnect(context);
      if (!interface) 
        continue;
      
      std::vector<String>::iterator it;
      for (it = state->inProgressWUs.begin(); it != state->inProgressWUs.end(); )
      {
        if (interface->isFinished(*it))
        {
          NetworkResponsePtr res = interface->getExecutionTrace(*it);
          if (res)
          {  
            ExecutionTracePtr trace = res->getExecutionTrace(context);
            trace->saveToFile(context,File::getCurrentWorkingDirectory().getChildFile(String(*it) + T(".trace")));  // TODO arnaud : project directory ?
            state->currentEvaluatedWUs.push_back(*it);
            state->totalNumberEvaluatedWUs++;
            context.informationCallback(T("WU ") + *it + T(" finished"));
            state->inProgressWUs.erase(it);
          }
          else
            ++it;
        }
        else 
          ++it;
      }
      interface->closeCommunication();
      
      File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml")).copyFileTo(File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState_backup.xml")));
      state->saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml")));
      context.informationCallback(T("State file saved in : ") + File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml")).getFullPathName());
      
      // enough WUs evaluated -> update distribution (with best results)
      if (state->currentEvaluatedWUs.size() >= numberWuToUpdate || (state->totalNumberGeneratedWUs == totalNumberWuRequested && state->inProgressWUs.size() == 0)) 
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
          distributionsBuilder->addElement((*mapIt).second);  // TODO arnaud : maybe use all results and use weight
          nb++;
        }
        
        IndependentMultiVariateDistributionPtr newDistri = distributionsBuilder->build(context);
        distributionsBuilder->clear();
        distributionsBuilder->addDistribution(state->distributions);  // old distri
        for (size_t i = 0; i < updateFactor; ++i)
          distributionsBuilder->addDistribution(newDistri);
        state->distributions = distributionsBuilder->build(context);        
        
        if ((*(resultsMap.rbegin())).first > state->bestScore) {
          state->bestScore = (*(resultsMap.rbegin())).first;
          state->bestVariable = (*(resultsMap.rbegin())).second;
          context.informationCallback(T("New best result found : ") + state->bestVariable.toString() + T(" ( ") + String(state->bestScore) + T(" )"));
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
#else
    return Variable();
#endif
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
  size_t updateFactor;
  
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
  
};
  
typedef ReferenceCountedObjectPtr<GridEvoOptimizer> GridEvoOptimizerPtr;

}; /* namespace lbcpp */

#endif // !GRID_EVO_OPTIMIZER_H_
