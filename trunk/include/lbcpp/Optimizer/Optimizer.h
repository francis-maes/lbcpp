/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.h                    | Optimizers                      |
| Author  : Francis Maes, Arnaud Schoofs   |                                 |
| Started : 21/12/2010 23:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_H_
# define LBCPP_OPTIMIZER_H_

# include "OptimizerInput.h"

namespace lbcpp
{

// Function, DistributionClass, [Variable] -> Variable
class Optimizer : public Function
{
public:  
  virtual size_t getNumRequiredInputs() const 
    {return getMinimumNumRequiredInputs();}
  virtual size_t getMinimumNumRequiredInputs() const 
    {return 2;}
  virtual size_t getMaximumNumRequiredInputs() const
    {return 3;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
  {
    switch (index) 
    {
      case 0:
        return (TypePtr) objectiveFunctionClass;
      case 1:
        return (TypePtr) distributionClass(anyType);
      case 2:
        if (numInputs != 3) 
        {
          jassert(false);
          return anyType; // TODO arnaud
        } else
          return variableType;
      default:
        jassert(false);
        return anyType; // TODO arnaud
    }
  }
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {    
    // TODO arnaud : check initial guess and apriori distribution
    if (inputVariables.size() == 3) 
      return inputVariables[2]->getType();  // type of initial guess
    
    return variableType;  // TODO arnaud : build type from apriori or from function ?
  } 
};

typedef ReferenceCountedObjectPtr<Optimizer> OptimizerPtr;

extern OptimizerPtr uniformSampleAndPickBestOptimizer(size_t numSamples);
extern OptimizerPtr iterativeBracketingOptimizer(size_t numPasses, double reductionFactor, const OptimizerPtr& baseOptimizer);
extern OptimizerPtr gridEvoOptimizer();

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_H_
