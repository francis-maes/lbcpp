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
# include "../Predictor/NumericalProteinPredictorParameters.h"
#include <map>
#include <set>
#include <lbcpp/Core/Vector.h>

namespace lbcpp
{
  class ProteinGridEvoOptimizerState : public Object {
  public:
    ProteinGridEvoOptimizerState()
      {jassert(false);} // TODO arnaud
    ProteinGridEvoOptimizerState(IndependentMultiVariateDistributionPtr distributions);

    NumericalProteinFeaturesParametersPtr sampleParameters() const;
    void generateSampleWU(ExecutionContext& context, const String& name);
    void clearBuilders();
    
    
    // TODO arnaud : implement accessors
    size_t totalNumberGeneratedWUs;
    size_t totalNumberEvaluatedWUs;
        
    std::set<String> inProgressWUs;
    std::multimap<double, String> currentEvaluatedWUs;

    IndependentMultiVariateDistributionPtr distributions;
    IndependentMultiVariateDistributionBuilderPtr distributionsBuilder;

  protected:    
    friend class ProteinGridEvoOptimizerStateClass;

  };
  typedef ReferenceCountedObjectPtr<ProteinGridEvoOptimizerState> ProteinGridEvoOptimizerStatePtr;

  
  class ProteinGridEvoOptimizer : public GridEvoOptimizer
  {
  public:
    
    virtual TypePtr getRequestedPriorKnowledgeType() const
      {return independentMultiVariateDistributionClass(variableType);}
    
    virtual Variable optimize(ExecutionContext& context, const FunctionPtr& objective, const Variable& apriori) const;
        
  protected:
    //virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
    
    friend class ProteinGridEvoOptimizerClass;
  };  
  

}; /* namespace lbcpp */

#endif // !PROTEIN_GRID_EVO_OPTIMIZER_H_