/*-----------------------------------------.---------------------------------.
| Filename: SynchroneousOptimizerContext.h | OptimizerContext that evaluates |
| Author  : Arnaud Schoofs                 | the requests from the Optimizer |
| Started : 03/04/2011                     | synchronously (single-thread    |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SYNCHRONEOUS_OPTIMIZER_CONTEXT_H_
# define LBCPP_SYNCHRONEOUS_OPTIMIZER_CONTEXT_H_

# include <lbcpp/Optimizer/OptimizerContext.h>

namespace lbcpp
{

class SynchroneousOptimizerContext : public OptimizerContext
{
public:
  SynchroneousOptimizerContext(const FunctionPtr& objectiveFunction) : OptimizerContext(objectiveFunction) {}
  SynchroneousOptimizerContext() {}
  
  virtual void waitUntilAllRequestsAreProcessed(ExecutionContext& context) const {}  // because evaluate is a blocking method
  virtual bool areAllRequestsProcessed(ExecutionContext& context) const
    {return true;}

  // blocking method
  virtual bool evaluate(ExecutionContext& context, const Variable& parameters) 
  { 
    inProgressEvaluation = parameters;
    Variable ret = objectiveFunction->compute(context, parameters);
    inProgressEvaluation = Variable();
    // callback is done in function evaluation !
    
    if (!ret.exists()) {
      context.errorCallback(T("No return value after function call!"));
      return false;
    }
  
    return true;
  }
  
protected:  
  friend class SynchroneousOptimizerContextClass;
  
private:
  Variable inProgressEvaluation;

};

};
#endif // !LBCPP_SYNCHRONEOUS_OPTIMIZER_CONTEXT_H_
