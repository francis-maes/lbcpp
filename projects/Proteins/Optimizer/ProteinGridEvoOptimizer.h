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
# include "../WorkUnit/ProteinLearner.h"
# include "../Predictor/ProteinPredictorParameters.h"
#include <map>
#include <set>
#include <lbcpp/Core/Vector.h>

namespace lbcpp
{
  class ProteinGridEvoOptimizerState : public Object {
  public:
    ProteinGridEvoOptimizerState()
      {jassert(false);} // TODO arnaud
    ProteinGridEvoOptimizerState(IndependentMultiVariateDistributionPtr distributions) : distributions(distributions) 
    {
      // TODO arnaud : if state.xml loaded
      numberGeneratedWU = 0;
      numberEvaluatedWU = 0;
    }
    
    NumericalProteinFeaturesParametersPtr sampleParameters() const;
    void generateSampleWU(ExecutionContext& context, const String& name);
    
    size_t getNumberGeneratedWU() const 
      {return numberGeneratedWU;}
    
    size_t getNumberEvaluatedWU() const 
      {return numberEvaluatedWU;}
    
    std::set<String> inProgressWUs;
    std::multimap<double, String> evaluatedWUs;

  protected:
    IndependentMultiVariateDistributionPtr distributions;
    
    size_t numberGeneratedWU;
    size_t numberEvaluatedWU;
    
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