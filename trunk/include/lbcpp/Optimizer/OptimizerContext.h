/*-----------------------------------------.---------------------------------.
| Filename: OptimizerContext.h             | Performs evaluations asked by   |
| Author  : Arnaud Schoofs                 | an Optimizer (abstract class)   |
| Started : 03/04/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
															 `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_CONTEXT_H_
# define LBCPP_OPTIMIZER_CONTEXT_H_

# include <lbcpp/Core/Function.h>

namespace lbcpp
{
  
class OptimizerContext : public Object
{
public:

  OptimizerContext(ExecutionContext& context, const FunctionPtr& objectiveFunction, const FunctionPtr& validationFunction = FunctionPtr(), size_t timeToSleep = 0);
  OptimizerContext() : context(*(ExecutionContext* )0) {}
  
  /** Set the FunctionCallback that should be called once the evaluation is done. */
  virtual void setPostEvaluationCallback(const FunctionCallbackPtr& callback);
  virtual void removePostEvaluationCallback(const FunctionCallbackPtr& callback);
  
  virtual void waitUntilAllRequestsAreProcessed() const;
  virtual bool areAllRequestsProcessed() const = 0;
  virtual size_t getTimeToSleep() const;
  
  /**
   * This function is supposed to be asynchronous, ie the result is not returned directly but through the callback.
   * @param parmeters value to use for the evaluation of objectiveFunction.
   * @return true if the operation is successful, false otherwise.
   */
  virtual bool evaluate(const Variable& parameters) = 0;
  
  const FunctionPtr& getValidationFunction() const
    {return validationFunction;}

protected:
  friend class OptimizerContextClass;
  
  ExecutionContext& context;
  FunctionPtr objectiveFunction;  /**< Function used for the evaluations. */
  FunctionPtr validationFunction;
  size_t timeToSleep;
};
  
typedef ReferenceCountedObjectPtr<OptimizerContext> OptimizerContextPtr;
extern ClassPtr optimizerContextClass;

extern OptimizerContextPtr synchroneousOptimizerContext(ExecutionContext& context, const FunctionPtr& objectiveFunction, const FunctionPtr& validationFunction = FunctionPtr());  
extern OptimizerContextPtr multiThreadedOptimizerContext(ExecutionContext& context, const FunctionPtr& objectiveFunction, const FunctionPtr& validationFunction = FunctionPtr(), size_t timeToSleep = 100);
extern OptimizerContextPtr distributedOptimizerContext(ExecutionContext& context, const FunctionPtr& objectiveFunction, String projectName, String source, String destination, String managerHostName, size_t managerPort, size_t requiredCpus, size_t requiredMemory, size_t requiredTimesize_t, size_t timeToSleep = 60000);  

};
#endif // !LBCPP_OPTIMIZER_CONTEXT_H_
