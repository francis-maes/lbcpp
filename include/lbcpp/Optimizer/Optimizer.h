/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.h                    | Optimizers                      |
| Author  : Francis Maes, Arnaud Schoofs   |                                 |
| Started : 21/12/2010 23:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_H_
# define LBCPP_OPTIMIZER_H_

# include "../Core/Function.h"
# include "../Distribution/Distribution.h"

namespace lbcpp
{

// Function, Distribution, [Variable] -> Variable
class Optimizer : public Function
{
public:
  virtual TypePtr getRequiredFunctionType() const
    {return functionClass;}
  virtual TypePtr getRequiredAprioriType() const
    {return distributionClass(anyType);}
  virtual TypePtr getRequiredGuessType() const
    {return variableType;}
  
  virtual Variable optimize(ExecutionContext& context, const FunctionPtr& function, const DistributionPtr& apriori, const Variable& guess) const = 0;
  
  // Function
  virtual size_t getMinimumNumRequiredInputs() const 
    {return 2;}
  virtual size_t getMaximumNumRequiredInputs() const
    {return 3;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
  {
    switch (index) 
    {
      case 0:
        return getRequiredFunctionType();
      case 1:
        return getRequiredAprioriType();
      default:
        return getRequiredGuessType();
    }
  }
  
  virtual String getOutputPostFix() const
    {return T("Optimized");}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {    
    // TODO arnaud : check that initial guess and apriori distribution match ?
    if (inputVariables.size() == 3) 
      return inputVariables[2]->getType();  // type of initial guess

    return variableType;  // TODO arnaud : build type from apriori or from function ?
  }
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {jassertfalse; return Variable();}
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    FunctionPtr function = inputs[0].getObjectAndCast<Function>();
    DistributionPtr apriori = inputs[1].getObjectAndCast<Distribution>();
    return optimize(context, function, apriori, Variable()); // TODO arnaud
  }
  
};

typedef ReferenceCountedObjectPtr<Optimizer> OptimizerPtr;

extern OptimizerPtr uniformSampleAndPickBestOptimizer(size_t numSamples);
extern OptimizerPtr iterativeBracketingOptimizer(size_t numPasses, double reductionFactor, const OptimizerPtr& baseOptimizer);
extern OptimizerPtr gridEvoOptimizer();

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_H_
