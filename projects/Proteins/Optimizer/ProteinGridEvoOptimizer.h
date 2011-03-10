/*-----------------------------------------.---------------------------------.
 | Filename: ProteinGridEvoOptimizer.h      | Optimizer using Evolutionary    |
 | Author  : Arnaud Schoofs                 | Algorithm on Grid for Protein   |
 | Started : 01/03/2010 23:45               | project                         |
 `------------------------------------------/                                 |
                                            |                                 |
                                            `--------------------------------*/

#ifndef PROTEIN_GRID_EVO_OPTIMIZER_H_
#define PROTEIN_GRID_EVO_OPTIMIZER_H_

# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Core/Enumeration.h>
# include "../../../src/Optimizer/Optimizer/GridEvoOptimizer.h"
# include <lbcpp/Distribution/MultiVariateDistribution.h>
# include <lbcpp/Distribution/ContinuousDistribution.h>
# include <lbcpp/Distribution/DistributionBuilder.h>
# include <lbcpp/Distribution/Distribution.h>
# include "../../src/Distribution/Builder/GaussianDistributionBuilder.h"
# include "../../src/Distribution/Builder/BernoulliDistributionBuilder.h"
# include "../../src/Distribution/Builder/IndependentMultiVariateDistributionBuilder.h"
# include "../WorkUnit/ProteinLearner.h"
# include "../Predictor/ProteinPredictorParameters.h"
# include "../Evaluator/ProteinEvaluator.h"
# include "../Predictor/NumericalProteinPredictorParameters.h"
#include <map>
#include <set>
#include <lbcpp/Core/Vector.h>
#include <lbcpp/Network/NetworkClient.h>
#include "../src/Network/Node/ManagerNode/ManagerNodeNetworkInterface.h"

namespace lbcpp
{
  class ProteinGridEvoOptimizerState : public GridEvoOptimizerState {
  public:
    ProteinGridEvoOptimizerState()
      {jassert(false);} // TODO arnaud
    ProteinGridEvoOptimizerState(IndependentMultiVariateDistributionPtr distributions, IndependentMultiVariateDistributionBuilderPtr distributionsBuilder) :
    GridEvoOptimizerState(distributions, distributionsBuilder) {}
    
    virtual WorkUnitPtr generateSampleWU(ExecutionContext& context) const;
    
  protected:    
    friend class ProteinGridEvoOptimizerStateClass;

  };
  typedef ReferenceCountedObjectPtr<ProteinGridEvoOptimizerState> ProteinGridEvoOptimizerStatePtr;

  
  class ProteinGridEvoOptimizer : public GridEvoOptimizer
  {
  public:
    
    virtual TypePtr getRequestedPriorKnowledgeType() const
      {return independentMultiVariateDistributionClass(variableType);}
            
  protected:    
    virtual GridEvoOptimizerStatePtr loadState() const {jassertfalse; return NULL;}  //not implemented
    virtual bool saveState() const {jassertfalse; return false;}  //not implemented
    virtual double getScoreFromTrace(ExecutionTracePtr trace) const;
    virtual Variable getVariableFromTrace(ExecutionTracePtr trace) const;
    
    friend class ProteinGridEvoOptimizerClass;
  };  
  
  typedef ReferenceCountedObjectPtr<ProteinGridEvoOptimizer> ProteinGridEvoOptimizerPtr;


}; /* namespace lbcpp */

#endif // !PROTEIN_GRID_EVO_OPTIMIZER_H_