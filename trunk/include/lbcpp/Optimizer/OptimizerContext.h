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
  OptimizerContext(ExecutionContext& context, const FunctionPtr& objectiveFunction, const FunctionPtr& validationFunction = FunctionPtr());
  OptimizerContext() : context(*(ExecutionContext* )0) {}
  
  virtual void setPostEvaluationCallback(const FunctionCallbackPtr& callback);
  virtual void removePostEvaluationCallback(const FunctionCallbackPtr& callback);
  
  virtual void waitUntilAllRequestsAreProcessed() const = 0;
  virtual bool areAllRequestsProcessed() const = 0;
  virtual size_t getTimeToSleep() const = 0;
  virtual bool evaluate(const Variable& parameters) = 0;
  
  const FunctionPtr& getValidationFunction() const
    {return validationFunction;}

protected:
  friend class OptimizerContextClass;
  
  ExecutionContext& context;
  FunctionPtr objectiveFunction;
  FunctionPtr validationFunction;
};
  
typedef ReferenceCountedObjectPtr<OptimizerContext> OptimizerContextPtr;
extern ClassPtr optimizerContextClass;

extern OptimizerContextPtr synchroneousOptimizerContext(ExecutionContext& context, const FunctionPtr& objectiveFunction, const FunctionPtr& validationFunction = FunctionPtr());  
extern OptimizerContextPtr multiThreadedOptimizerContext(ExecutionContext& context, const FunctionPtr& objectiveFunction, const FunctionPtr& validationFunction = FunctionPtr(), size_t timeToSleep = 100);
extern OptimizerContextPtr distributedOptimizerContext(ExecutionContext& context, const FunctionPtr& objectiveFunction, String projectName, String source, String destination, String managerHostName, size_t managerPort, size_t requiredCpus, size_t requiredMemory, size_t requiredTimesize_t, size_t timeToSleep = 60000);  

};
#endif // !LBCPP_OPTIMIZER_CONTEXT_H_
