/*-----------------------------------------.---------------------------------.
| Filename: OptimizerContext.h             | Performs evaluations asked by   |
| Author  : Arnaud Schoofs                 | an Optimizer (abstract class)   |
| Started : 03/04/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
															 `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_CONTEXT_H_
# define LBCPP_OPTIMIZER_CONTEXT_H_

# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Core/Function.h>

namespace lbcpp
{
  
class OptimizerContext : public Object
{
public:
  OptimizerContext(const FunctionPtr& objectiveFunction);
  OptimizerContext() {}
  
  void setPostEvaluationCallback(const FunctionCallbackPtr& callback);
  void removePostEvaluationCallback(const FunctionCallbackPtr& callback);
  
  virtual void waitUntilAllRequestsAreProcessed(ExecutionContext& context) const = 0;
  virtual bool evaluate(ExecutionContext& context, const Variable& parameters) = 0;
  virtual bool evaluate(ExecutionContext& context, const std::vector<Variable>& parametersVector);
  
protected:
  friend class OptimizerContextClass;
  
  FunctionPtr objectiveFunction;
};
  
typedef ReferenceCountedObjectPtr<OptimizerContext> OptimizerContextPtr;
extern ClassPtr optimizerContextClass;

extern OptimizerContextPtr multiThreadedOptimizerContext(const FunctionPtr& objectiveFunction);

};
#endif // !LBCPP_OPTIMIZER_CONTEXT_H_
