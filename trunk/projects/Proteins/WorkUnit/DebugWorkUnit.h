/*-----------------------------------------.---------------------------------.
| Filename: DebugWorkUnit.h                | WorkUnit used for debug purpose |
| Author  : Arnaud Schoofs                 |                                 |
| Started : 05/04/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

// TODO arnaud : useless file ?

#ifndef LBCPP_DEBUG_WORK_UNIT_H_
# define LBCPP_DEBUG_WORK_UNIT_H_

# include <lbcpp/lbcpp.h>
# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Optimizer/OptimizerContext.h>
# include <lbcpp/Optimizer/OptimizerState.h>
# include "../../../src/Distribution/Builder/GaussianDistributionBuilder.h"

# include "../WorkUnit/ProteinLearner.h"

namespace lbcpp
{
class DebugWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context) 
  {
    return Variable();
    
    
    // variables used by DistributedOptimizerContext
    String projectName(T("DebugNetwork2"));
    String source(T("arnaud@monster24"));
    String destination(T("boincadm@boinc.run"));
    String managerHostName(T("localhost"));
    size_t managerPort = 1664;
    size_t requiredMemory = 1;
    size_t requiredCpus = 1;
    size_t requiredTime = 1;
    
    // initial distribution
    IndependentMultiVariateDistributionPtr distributions = new IndependentMultiVariateDistribution(numericalProteinFeaturesParametersClass);      
    distributions->setSubDistribution(0, new PositiveIntegerGaussianDistribution(1,9));
    distributions->setSubDistribution(1, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(2, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(3, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(4, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(5, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(6, new PositiveIntegerGaussianDistribution(2,9));
    distributions->setSubDistribution(7, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(8, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(9, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(10, new BernoulliDistribution(0.5));
    distributions->setSubDistribution(11, new PositiveIntegerGaussianDistribution(15,4));
    distributions->setSubDistribution(12, new PositiveIntegerGaussianDistribution(15,100));
    distributions->setSubDistribution(13, new PositiveIntegerGaussianDistribution(50,225));
    
    // Optimizer
    OptimizerPtr optimizer = asyncEDAOptimizer(15, 1000, 300, 1500, 15);
    OptimizerContextPtr optimizerContext = distributedOptimizerContext(context, new ProteinLearnerObjectiveFunction(), projectName, source, destination, managerHostName, managerPort, requiredCpus, requiredMemory, requiredTime, 150000);
    DistributionBasedOptimizerStatePtr optimizerState = new DistributionBasedOptimizerState();
    optimizerState->setDistribution(distributions);
    return optimizer->compute(context, optimizerContext, optimizerState);    
    
    /*ProteinGridEvoOptimizerStatePtr state = Object::createFromFile(context, File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml"))).staticCast<ProteinGridEvoOptimizerState>();
    foo(context, state);
    
    std::cout << numericalProteinPredictorParameters()->toString() << std::endl;
    
    return Variable();
  }
  
private:
  void foo(ExecutionContext& context, const GridOptimizerStatePtr& state_)
  {
    GridEvoOptimizerStatePtr state = state_.dynamicCast<GridEvoOptimizerState>();
    state->saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml")));*/
  }
};

};


#endif //!LBCPP_DEBUG_WORK_UNIT_H_
