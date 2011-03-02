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
# include "../WorkUnit/ProteinLearner.h"
# include "../Predictor/ProteinPredictorParameters.h"

namespace lbcpp
{
  class ProteinGridEvoOptimizerState : public Object {
  public:
    ProteinGridEvoOptimizerState()
      {jassert(false);} // TODO arnaud
    ProteinGridEvoOptimizerState(IndependentMultiVariateDistributionPtr distributions) : distributions(distributions) {}
    
    NumericalProteinFeaturesParametersPtr sampleParameters() const;

  protected:
    IndependentMultiVariateDistributionPtr distributions;
    
    friend class ProteinGridEvoOptimizerStateClass;

  };
  typedef ReferenceCountedObjectPtr<ProteinGridEvoOptimizerState> ProteinGridEvoOptimizerStatePtr;

  
  class ProteinGridEvoOptimizer : public GridEvoOptimizer
  {
  public:
    virtual size_t getMaximumNumRequiredInputs() const
      {return 2;} // do not use initial guess
    
    virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {
      switch (index) 
      {
        case 0:
          return (TypePtr) objectiveFunctionClass;
        case 1:
          return (TypePtr) independentMultiVariateDistributionClass(variableType);
        default:
          jassert(false); // TODO arnaud
          return anyType;
      }
    }
    
  protected:
    virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
    void generateSampleWU(ExecutionContext& context, ProteinGridEvoOptimizerStatePtr state, const String& name) const;
    
    friend class ProteinGridEvoOptimizerClass;
  };  
  

}; /* namespace lbcpp */

#endif // !PROTEIN_GRID_EVO_OPTIMIZER_H_