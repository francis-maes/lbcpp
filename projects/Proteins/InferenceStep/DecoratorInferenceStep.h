/*-----------------------------------------.---------------------------------.
| Filename: DecoratorInferenceStep.h       | Decorator inference steps       |
| Author  : Francis Maes                   |                                 |
| Started : 16/04/2010 18:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_STEP_DECORATOR_H_
# define LBCPP_INFERENCE_STEP_DECORATOR_H_

# include "InferenceStep.h"
# include "../InferenceContext/InferenceContext.h"
# include "../InferenceContext/InferenceCallback.h"

namespace lbcpp
{

class DecoratorInferenceStep : public InferenceStep
{
public:
  DecoratorInferenceStep(const String& name, InferenceStepPtr decorated)
    : InferenceStep(name), decorated(decorated) {}
  DecoratorInferenceStep() {}
 
  virtual void accept(InferenceVisitorPtr visitor)
    {if (decorated) decorated->accept(visitor);}

  virtual bool loadFromFile(const File& file)
  {
    if (!loadFromDirectory(file))
      return false;
    decorated = createFromFileAndCast<InferenceStep>(file.getChildFile(T("decorated.inference")));
    return decorated != InferenceStepPtr();
  }

  virtual bool saveToFile(const File& file) const
    {return saveToDirectory(file) && decorated->saveToFile(file.getChildFile(T("decorated.inference")));}

protected:
  InferenceStepPtr decorated;

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    if (decorated)
      return decorated->run(context, input, supervision, returnCode);
    else
    {
      returnCode = InferenceStep::errorReturnCode;
      return ObjectPtr();
    }
  }
};

class CallbackBasedDecoratorInferenceStep : public DecoratorInferenceStep
{
public:
  CallbackBasedDecoratorInferenceStep(const String& name, InferenceStepPtr decorated, InferenceCallbackPtr callback)
    : DecoratorInferenceStep(name, decorated), callback(callback) {}
  CallbackBasedDecoratorInferenceStep() {}
 
protected:
  InferenceCallbackPtr callback;
  
  virtual bool load(InputStream& istr)
    {return DecoratorInferenceStep::load(istr) && lbcpp::read(istr, callback);}

  virtual void save(OutputStream& ostr) const
    {DecoratorInferenceStep::save(ostr); lbcpp::write(ostr, callback);}

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    context->appendCallback(callback);
    ObjectPtr res = DecoratorInferenceStep::run(context, input, supervision, returnCode);
    context->removeCallback(callback);
    return res;
  }
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_DECORATOR_H_
