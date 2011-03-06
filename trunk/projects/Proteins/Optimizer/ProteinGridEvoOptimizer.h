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
    virtual TypePtr getRequiredAprioriType() const
      {return independentMultiVariateDistributionClass(variableType);}  // TODO arnaud : continuousDistributionClass ?
    
    virtual size_t getMaximumNumRequiredInputs() const
      {return 2;} // do not use initial guess
    
    virtual Variable optimize(ExecutionContext& context, const FunctionPtr& function, const DistributionPtr& apriori, const Variable& guess) const;
    
  protected:
    //virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
    
    friend class ProteinGridEvoOptimizerClass;
  };  
  

}; /* namespace lbcpp */

#endif // !PROTEIN_GRID_EVO_OPTIMIZER_H_