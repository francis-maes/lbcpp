/*-----------------------------------------.---------------------------------.
| Filename: DecoratorInferenceStep.h       | Decorator inference steps       |
| Author  : Francis Maes                   |                                 |
| Started : 16/04/2010 18:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_STEP_DECORATOR_H_
# define LBCPP_INFERENCE_STEP_DECORATOR_H_

# include <lbcpp/Inference/Inference.h>
# include <lbcpp/Inference/InferenceContext.h>
# include <lbcpp/Inference/InferenceCallback.h>
# include <lbcpp/lbcpp.h> // tmp

namespace lbcpp
{

class DecoratorInferenceStep : public InferenceStep
{
public:
  DecoratorInferenceStep(const String& name, InferenceStepPtr decorated)
    : InferenceStep(name), decorated(decorated) {}
  DecoratorInferenceStep() {}
 
  /*
  ** Object
  */
  virtual String toString() const;
  virtual bool loadFromFile(const File& file);
  virtual bool saveToFile(const File& file) const;

  /*
  ** InferenceStep
  */
  virtual void accept(InferenceVisitorPtr visitor);
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);

  InferenceStepPtr getDecoratedInference() const
    {return decorated;}

protected:
  InferenceStepPtr decorated;
};

class CallbackBasedDecoratorInferenceStep : public DecoratorInferenceStep
{
public:
  CallbackBasedDecoratorInferenceStep(const String& name, InferenceStepPtr decorated, InferenceCallbackPtr callback)
    : DecoratorInferenceStep(name, decorated), callback(callback) {}
  CallbackBasedDecoratorInferenceStep() {}
 
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);

protected:
  InferenceCallbackPtr callback;
  
  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;
};

class TransferRegressionInferenceStep : public DecoratorInferenceStep
{
public:
  TransferRegressionInferenceStep(const String& name, InferenceStepPtr regressionStep, ScalarFunctionPtr transferFunction)
    : DecoratorInferenceStep(name, regressionStep), transferFunction(transferFunction) {}
  TransferRegressionInferenceStep() {}
  
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    if (supervision)
    {
      ScalarFunctionPtr loss = supervision.dynamicCast<ScalarFunction>();
      jassert(loss);
      supervision = transferFunction->composeWith(loss);
    }
    ObjectPtr result = DecoratorInferenceStep::run(context, input, supervision, returnCode);
    if (result)
    {
      ScalarPtr scalarResult = result.dynamicCast<Scalar>();
      jassert(scalarResult);
      result = new Scalar(transferFunction->compute(scalarResult->getValue()));
    }
    return result;
  }

protected:
  ScalarFunctionPtr transferFunction;

  virtual bool load(InputStream& istr)
    {return DecoratorInferenceStep::load(istr) && lbcpp::read(istr, transferFunction);}

  virtual void save(OutputStream& ostr) const
    {DecoratorInferenceStep::save(ostr); lbcpp::write(ostr, transferFunction);}
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_DECORATOR_H_
