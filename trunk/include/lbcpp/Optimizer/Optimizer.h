/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.h                    | Optimizers                      |
| Author  : Francis Maes, Arnaud Schoofs   |                                 |
| Started : 21/12/2010 23:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_H_
# define LBCPP_OPTIMIZER_H_

# include <lbcpp/Optimizer/OptimizerContext.h>
# include <lbcpp/Optimizer/OptimizerState.h>

namespace lbcpp
{

class Optimizer : public Function
{
public:
  virtual TypePtr getRequiredContextType() const
    {return optimizerContextClass;}
  
  virtual TypePtr getRequiredStateType() const
    {return optimizerStateClass;}
  
  virtual Variable optimize(ExecutionContext& context, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const = 0;
  
  // Function
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? getRequiredContextType() : getRequiredStateType();}

  virtual String getOutputPostFix() const
    {return T("Optimized");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return variableType;}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    OptimizerContextPtr optimizerContext = inputs[0].getObjectAndCast<OptimizerContext>();
    OptimizerStatePtr optimizerState = inputs[1].getObjectAndCast<OptimizerState>();
    optimizerContext->setPostEvaluationCallback((FunctionCallbackPtr) optimizerState.get());  // TODO arnaud : casting OK ?
    Variable output = optimize(context, optimizerContext, optimizerState);
    optimizerContext->removePostEvaluationCallback((FunctionCallbackPtr) optimizerState.get()); // TODO arnaud : casting OK ?
    return output;
  }
};

typedef ReferenceCountedObjectPtr<Optimizer> OptimizerPtr;

// Function, Variable -> Variable
/*class OptimizerOld : public Function
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
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return optimize(context, inputs[0].getObjectAndCast<Function>(), inputs[1]);}
};

typedef ReferenceCountedObjectPtr<OptimizerOld> OptimizerOldPtr;

extern OptimizerOldPtr uniformSampleAndPickBestOptimizer(size_t numSamples);
extern OptimizerOldPtr iterativeBracketingOptimizer(size_t numPasses, double reductionFactor, const OptimizerOldPtr& baseOptimizer);
extern OptimizerOldPtr gridEvoOptimizer();*/

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_H_
