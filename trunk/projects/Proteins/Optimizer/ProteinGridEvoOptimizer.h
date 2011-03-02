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
    ProteinGridEvoOptimizerState() {
      distributions = new IndependentMultiVariateDistribution(numericalProteinFeaturesParametersClass);
      //EnumerationPtr enumeration = positiveIntegerEnumerationEnumeration();
      
      distributions->setSubDistribution(0, new IntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(1, new IntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(2, new IntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(3, new IntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(4, new IntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(5, new IntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(6, new IntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(7, new IntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(8, new IntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(9, new IntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(10, new BernoulliDistribution(0.5));
      distributions->setSubDistribution(11, new IntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(12, new IntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(13, new IntegerGaussianDistribution(5,1));


      //distributions->setSubDistribution(9, new UniformDistribution(0,10));
      //distributions->setSubDistribution(10, new UniformDistribution(0,10));
      Variable var = distributions->sample(RandomGenerator::getInstance());
      std::cout << var.toString() << std::endl;
    }
    
  protected:
    friend class ProteinGridEvoOptimizerStateClass;
    
  private:
    IndependentMultiVariateDistributionPtr distributions;
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
          return (TypePtr) independentMultiVariateDistributionClass(doubleType);
        default:
          jassert(false); // TODO arnaud
          return anyType;
      }
    }
    
  protected:
    virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
    
    friend class ProteinGridEvoOptimizerClass;
  };  
  

}; /* namespace lbcpp */

#endif // !PROTEIN_GRID_EVO_OPTIMIZER_H_