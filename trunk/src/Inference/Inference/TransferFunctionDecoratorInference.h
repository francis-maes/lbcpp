/*-----------------------------------------.---------------------------------.
| Filename: TransferFunctionDecoratorIn...h| Applies a transfer function     |
| Author  : Francis Maes                   |  to a scalar inference          |
| Started : 05/05/2010 15:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_TRANSFER_FUNCTION_DECORATOR_H_
# define LBCPP_INFERENCE_TRANSFER_FUNCTION_DECORATOR_H_

# include <lbcpp/Inference/DecoratorInference.h>

namespace lbcpp
{

class TransferFunctionDecoratorInference : public DecoratorInference
{
public:
  TransferFunctionDecoratorInference(const String& name, InferencePtr regressionStep, ScalarFunctionPtr transferFunction)
    : DecoratorInference(name, regressionStep), transferFunction(transferFunction) {}
  TransferFunctionDecoratorInference() {}
  
  virtual std::pair<Variable, Variable> prepareSubInference(const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    if (supervision)
    {
      ScalarFunctionPtr loss = supervision.dynamicCast<ScalarFunction>();
      jassert(loss);
      return std::make_pair(input, transferFunction->composeWith(loss));
    }
    return std::make_pair(input, supervision);
  }
    
  virtual Variable finalizeSubInference(const Variable& input, const Variable& supervision, const Variable& subInferenceOutput, ReturnCode& returnCode) const
  {
    if (!subInferenceOutput)
      return Variable();
    ScalarPtr scalarResult = subInferenceOutput.dynamicCast<Scalar>();
    jassert(scalarResult);
    return ObjectPtr(new Scalar(transferFunction->compute(scalarResult->getValue())));
  }
  
protected:
  ScalarFunctionPtr transferFunction;

  virtual bool load(InputStream& istr)
    {return DecoratorInference::load(istr) && lbcpp::read(istr, transferFunction);}

  virtual void save(OutputStream& ostr) const
    {DecoratorInference::save(ostr); lbcpp::write(ostr, transferFunction);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_TRANSFER_FUNCTION_DECORATOR_H_
