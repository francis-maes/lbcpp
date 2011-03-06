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

// Function, Variable -> Variable
class Optimizer : public Function
{
public:
  virtual TypePtr getRequiredFunctionType() const
    {return functionClass;}
  
  virtual TypePtr getRequestedPriorKnowledgeType() const = 0;
  virtual Variable optimize(ExecutionContext& context, const FunctionPtr& function, const Variable& priorKnowledge) const = 0;
  
  // Function
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? getRequiredFunctionType() : getRequestedPriorKnowledgeType();}
  
  virtual String getOutputPostFix() const
    {return T("Optimized");}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return variableType;}  // TODO arnaud : more precise needed ?
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {jassertfalse; return Variable();}
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    return optimize(context, inputs[0].getObjectAndCast<Function>(), inputs[1]);
  }
  
};

typedef ReferenceCountedObjectPtr<Optimizer> OptimizerPtr;

extern OptimizerPtr uniformSampleAndPickBestOptimizer(size_t numSamples);
extern OptimizerPtr iterativeBracketingOptimizer(size_t numPasses, double reductionFactor, const OptimizerPtr& baseOptimizer);
extern OptimizerPtr gridEvoOptimizer();

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_H_
