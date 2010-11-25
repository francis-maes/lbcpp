/*-----------------------------------------.---------------------------------.
| Filename: TransferFunctionDecoratorIn...h| Applies a transfer function     |
| Author  : Francis Maes                   |  to a scalar inference          |
| Started : 05/05/2010 15:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_INFERENCE_TRANSFER_FUNCTION_H_
# define LBCPP_NUMERICAL_LEARNING_INFERENCE_TRANSFER_FUNCTION_H_

# include <lbcpp/Inference/DecoratorInference.h>
# include <lbcpp/NumericalLearning/NumericalLearning.h>

namespace lbcpp
{

class TransferFunctionDecoratorInference : public StaticDecoratorInference
{
public:
  TransferFunctionDecoratorInference(const String& name, InferencePtr regressionStep, ScalarFunctionPtr transferFunction)
    : StaticDecoratorInference(name, regressionStep), transferFunction(transferFunction) {}
  TransferFunctionDecoratorInference() {}
  
  virtual DecoratorInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    DecoratorInferenceStatePtr res = new DecoratorInferenceState(input, supervision);
    if (supervision.exists())
    {
      ScalarFunctionPtr loss = supervision.dynamicCast<ScalarFunction>();
      jassert(loss);
      res->setSubInference(decorated, input, transferFunction->composeWith(loss));
    }
    else
      res->setSubInference(decorated, input, Variable());
    return res;
  }

  virtual std::pair<Variable, Variable> prepareSubInference(const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    if (supervision.exists())
    {
      ScalarFunctionPtr loss = supervision.dynamicCast<ScalarFunction>();
      jassert(loss);
      return std::make_pair(input, transferFunction->composeWith(loss));
    }
    return std::make_pair(input, supervision);
  }
   
  virtual Variable finalizeInference(ExecutionContext& context, const DecoratorInferenceStatePtr& finalState, ReturnCode& returnCode)
  {
    Variable subOutput = finalState->getSubOutput();
    return subOutput.exists() ? Variable(transferFunction->compute(subOutput.getDouble())) : Variable();
  }
  
protected:
  friend class TransferFunctionDecoratorInferenceClass;

  ScalarFunctionPtr transferFunction;
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_INFERENCE_TRANSFER_FUNCTION_H_
