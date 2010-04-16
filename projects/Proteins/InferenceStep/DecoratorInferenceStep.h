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

namespace lbcpp
{

class DecoratorInferenceStep : public InferenceStep
{
public:
  DecoratorInferenceStep(InferenceStepPtr decorated)
    : decorated(decorated) {}
 
  virtual void accept(InferenceVisitorPtr visitor)
    {if (decorated) decorated->accept(visitor);}

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
  CallbackBasedDecoratorInferenceStep(InferenceStepPtr decorated, InferenceCallbackPtr callback)
    : DecoratorInferenceStep(decorated), callback(callback) {}

protected:
  InferenceCallbackPtr callback;

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
