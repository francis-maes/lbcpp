/*-----------------------------------------.---------------------------------.
| Filename: SynchroneousOptimizerContext.h | OptimizerContext that evaluates |
| Author  : Arnaud Schoofs                 | the requests from the Optimizer |
| Started : 03/04/2011                     | synchronously (single-thread)   |
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
  SynchroneousOptimizerContext(ExecutionContext& context, const FunctionPtr& objectiveFunction, const FunctionPtr& validationFunction)
    : OptimizerContext(context, objectiveFunction, validationFunction, 0) {}
  SynchroneousOptimizerContext() {}
  
  virtual bool isSynchroneous() const
    {return true;}

  // evaluate is a blocking method
  virtual bool areAllRequestsProcessed() const
    {return true;}

  // blocking method
  virtual bool evaluate(const Variable& parameters) 
  { 
    Variable ret = objectiveFunction->compute(context, parameters);
    // callback is done in function evaluation !
    
    if (!ret.exists()) {
      context.errorCallback(T("No return value after function call!"));
      return false;
    }
    return true;
  }
  
protected:  
  friend class SynchroneousOptimizerContextClass;
};

};
#endif // !LBCPP_SYNCHRONEOUS_OPTIMIZER_CONTEXT_H_
