/*-----------------------------------------.---------------------------------.
| Filename: ProxyFunctionBatchLearner.h    | Proxy Function Batch Learner    |
| Author  : Francis Maes                   |                                 |
| Started : 16/02/2011 22:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_BATCH_LEARNER_PROXY_FUNCTION_H_
# define LBCPP_LEARNING_BATCH_LEARNER_PROXY_FUNCTION_H_

# include <lbcpp/Learning/BatchLearner.h>

namespace lbcpp
{

class ProxyFunctionBatchLearner : public BatchLearner
{
public:
  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const ProxyFunctionPtr& function = f.staticCast<ProxyFunction>();
    const FunctionPtr& implementation = function->getImplementation();
    if (!implementation)
    {
      context.errorCallback(T("Missing implementation"));
      return false;
    }
    if (!implementation->getBatchLearner())
      return true;
    return implementation->train(context, trainingData, validationData);
  }

  lbcpp_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_BATCH_LEARNER_PROXY_FUNCTION_H_
